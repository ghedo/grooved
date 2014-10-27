/*
 * Groovy music player daemon.
 *
 * Copyright (c) 2014, Alessandro Ghedini
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS
 * IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

#include <inttypes.h>

#include <glib.h>

#include <mpv/client.h>

#include "config.h"
#include "dbus-service.h"
#include "library.h"
#include "player.h"
#include "printf.h"
#include "util.h"

typedef struct {
	GSource  src;
	gpointer fd;
} GPlayerSource;

enum player_status {
	STARTING,
	IDLE,
	PLAYING,
	PAUSED,
	STOPPED,
};

static mpv_handle *player_ctx;

static enum player_status player_status;

static enum loop player_loop = PLAYER_LOOP_NONE;

static int64_t playlist_pos = -1;

#define player_check_error(MSG, RC)				\
	if (RC < 0)						\
		fail_printf(MSG ": %s", mpv_error_string(RC));

static gboolean player_loop_fd_prepare(GSource *src, int *timeout);
static gboolean player_loop_fd_check(GSource *src);
static gboolean player_loop_fd_dispatch(GSource *src, GSourceFunc cb, void *p);
static void     player_loop_fd_finalize(GSource *src);

static GSourceFuncs player_funcs = {
	player_loop_fd_prepare,
	player_loop_fd_check,
	player_loop_fd_dispatch,
	player_loop_fd_finalize
};

static void player_print_playlist_status(void);

void player_init(void) {
	int rc;

	GPlayerSource *player_src = NULL;

	player_status = STARTING;

	player_ctx = mpv_create();

	if (player_ctx == NULL)
		fail_printf("Could not create player");

	rc = mpv_set_option_string(player_ctx, "no-config", "");
	player_check_error("Could not set no-config", rc);

	rc = mpv_set_option_string(player_ctx, "no-video", "");
	player_check_error("Could not set no-video", rc);

	rc = mpv_set_option_string(player_ctx, "no-sub", "");
	player_check_error("Could not set no-sub", rc);

	rc = mpv_set_option_string(player_ctx, "no-softvol", "");
	player_check_error("Could not set no-softvol", rc);

	if (cfg.gapless != NULL) {
		rc = mpv_set_option_string(player_ctx, "gapless-audio", cfg.gapless);
		player_check_error("Could not enable gapless audio", rc);
	}

	if (cfg.filters != NULL) {
		rc = mpv_set_option_string(player_ctx, "af", cfg.filters);
		player_check_error("Could not set filters", rc);
	}

	if (cfg.output != NULL) {
		rc = mpv_set_option_string(player_ctx, "ao", cfg.output);
		player_check_error("Could not set output", rc);
	}

	if (cfg.cache != NULL) {
		rc = mpv_set_option_string(player_ctx, "cache", cfg.cache);
		player_check_error("Could not set cache", rc);
	}

	if (cfg.scripts != NULL) {
		rc = mpv_set_option_string(player_ctx, "lua", cfg.scripts);
		player_check_error("Could not set scripts", rc);
	}

	mpv_request_log_messages(player_ctx, "warn");

	player_src = (GPlayerSource *) g_source_new(
		&player_funcs, sizeof(GPlayerSource)
	);

	player_src -> fd  = g_source_add_unix_fd(
		(GSource *) player_src,
		mpv_get_wakeup_pipe(player_ctx),
		G_IO_IN | G_IO_HUP | G_IO_ERR
	);

	g_source_attach((GSource *) player_src, NULL);

	rc = mpv_initialize(player_ctx);
	player_check_error("Could not initialize player", rc);
}

const char *player_error_string(int error) {
	return mpv_error_string(error);
}

void player_make_list(GVariantBuilder *list) {
	int i, j;

	int64_t count;
	mpv_node playlist;

	GVariantBuilder *files = g_variant_builder_new(G_VARIANT_TYPE("as"));

	mpv_get_property(player_ctx, "playlist", MPV_FORMAT_NODE, &playlist);
	mpv_get_property(player_ctx, "playlist-count", MPV_FORMAT_INT64, &count);

	if (playlist.format == MPV_FORMAT_NODE_ARRAY) {
		for (i = 0; i < playlist.u.list -> num; i++) {
			mpv_node *current = &playlist.u.list -> values[i];

			if (current -> format != MPV_FORMAT_NODE_MAP)
				continue;

			for (j = 0; j < current -> u.list -> num; j++) {
				mpv_node *val = &current -> u.list -> values[j];

				if (val -> format != MPV_FORMAT_STRING)
					continue;

				g_variant_builder_add(
					files, "s", val -> u.string
				);
			}

		}
	}

	g_variant_builder_add_value(list, g_variant_builder_end(files));
	g_variant_builder_add(list, "x", count);
	g_variant_builder_add(list, "x", playlist_pos);

	mpv_free_node_contents(&playlist);
}

void player_make_metadata(GVariantBuilder *meta) {
	int i;

	mpv_node metadata;

	mpv_get_property(player_ctx, "metadata", MPV_FORMAT_NODE, &metadata);

	if (metadata.format == MPV_FORMAT_NODE_MAP) {
		for (i = 0; i < metadata.u.list -> num; i++) {
			char *key = metadata.u.list -> keys[i];
			char *val = metadata.u.list -> values[i].u.string;

			g_variant_builder_add(meta, "{ss}", key, val);
		}
	}

	mpv_free_node_contents(&metadata);
}

int player_playback_start(void) {
	int rc;

	_free_ char *path = NULL;

	int64_t pos   = playlist_pos;
	int64_t count = player_playlist_count();

	if ((pos == (count - 1) && (path = library_random())))
		player_playlist_append_file(path);

	++pos;

	rc = mpv_set_property(
		player_ctx, "playlist-pos", MPV_FORMAT_INT64, &pos
	);
	if (rc < 0) return rc;

	player_status = PLAYING;

	return 0;
}

int player_playback_play(void) {
	int rc;

	switch (player_status) {
		case STARTING:
		case PLAYING:
			break;

		case IDLE:
		case STOPPED:
			player_playback_start();

		case PAUSED:
			rc = mpv_set_property_string(player_ctx, "pause", "no");
			if (rc < 0) return rc;
			break;
	}

	return 0;
}

int player_playback_pause(void) {
	int rc;

	switch (player_status) {
		case STARTING:
		case IDLE:
		case PAUSED:
		case STOPPED:
			break;

		case PLAYING:
			rc = mpv_set_property_string(player_ctx, "pause", "yes");
			if (rc < 0) return rc;
			break;
	}

	return 0;
}

int player_playback_toggle(void) {
	int rc;

	switch (player_status) {
		case STARTING:
			break;

		case IDLE:
		case PAUSED:
		case STOPPED:
			rc = player_playback_play();
			if (rc < 0) return rc;
			break;

		case PLAYING:
			rc = player_playback_pause();
			if (rc < 0) return rc;
			break;
	}

	return 0;
}

int player_playback_stop(void) {
	int rc = 0;
	enum player_status prev_status = player_status;
	const char *cmd_clear[]  = { "playlist_clear", NULL };

	player_status = STOPPED;

	switch (prev_status) {
		case STARTING:
		case STOPPED:
			break;

		case IDLE:
		case PLAYING:
		case PAUSED:
			rc = mpv_command(player_ctx, cmd_clear);
			if (rc < 0) return rc;

			rc = player_playlist_remove_index(-1);
			if (rc < 0) return rc;

			break;
	}

	return 0;
}

int player_playback_seek(int64_t secs) {
	int rc;
	char secs_arg[9];
	const char *cmd[] = { "seek", secs_arg, NULL };

	sprintf(secs_arg, "%" PRId64, secs);

	rc = mpv_command(player_ctx, cmd);
	if (rc < 0) return rc;

	return 0;
}

int player_playback_loop(enum loop mode) {
	int rc;

	switch (mode) {
		case PLAYER_LOOP_TRACK:
			player_playback_loop(PLAYER_LOOP_NONE);

			rc = mpv_set_property_string(
				player_ctx, "loop-file", "yes"
			);
			if (rc < 0) return rc;
			break;

		case PLAYER_LOOP_LIST:
			player_playback_loop(PLAYER_LOOP_NONE);

			rc = mpv_set_property_string(player_ctx, "loop", "inf");
			if (rc < 0) return rc;
			break;

		case PLAYER_LOOP_NONE:
			rc = mpv_set_property_string(
				player_ctx, "loop-file", "no"
			);
			if (rc < 0) return rc;

			rc = mpv_set_property_string(player_ctx, "loop", "no");
			if (rc < 0) return rc;
			break;
	}

	player_loop = mode;

	return 0;
}

char *player_playback_status_string(void) {
	switch (player_status) {
		case STARTING:
			return "starting";

		case IDLE:
			return "idle";

		case PLAYING:
			return "play";

		case PAUSED:
			return "pause";

		case STOPPED:
			return "stop";
	}

	return NULL;
}

char *player_loop_status_string(void) {
	switch (player_loop) {
		case PLAYER_LOOP_TRACK:
			return "track";

		case PLAYER_LOOP_LIST:
			return "list";

		case PLAYER_LOOP_NONE:
			return "none";
	}

	return NULL;
}

int player_get_property_double(char *name, double *value) {
	int rc;

	rc = mpv_get_property(player_ctx, name, MPV_FORMAT_DOUBLE, value);
	if (rc < 0) return rc;

	return 0;
}

int player_get_property_int64(char *name, int64_t *value) {
	int rc;

	rc = mpv_get_property(player_ctx, name, MPV_FORMAT_INT64, value);
	if (rc < 0) return rc;

	return 0;
}

int player_get_property_string(char *name, char **value) {
	char *mvalue = mpv_get_property_string(player_ctx, name);
	*value       = (mvalue != NULL) ? strdup(mvalue) : strdup("");

	mpv_free(mvalue);

	return 0;
}

double player_playback_track_length(void) {
	double length = 0.0;
	player_get_property_double("length", &length);
	return length;
}

char *player_playback_track_path(void) {
	char *path = NULL;
	player_get_property_string("path", &path);

	return path;
}

double player_playback_track_position_time(void) {
	double pos = 0.0;
	player_get_property_double("time-pos", &pos);
	return pos;
}

double player_playback_track_position_percent(void) {
	double pos = 0.0;
	player_get_property_double("percent-pos", &pos);
	return pos;
}

int64_t player_playlist_count(void) {
	int64_t count = -1;
	player_get_property_int64("playlist-count", &count);
	return count;
}

int64_t player_playlist_position(void) {
	int64_t pos = -1;
	player_get_property_int64("playlist-pos", &pos);
	return pos;
}

int player_playlist_append_file(const char *path) {
	int rc;
	const char *cmd[] = { "loadfile", path, "append", NULL };

	rc = mpv_command(player_ctx, cmd);
	if (rc < 0) return rc;

	return 0;
}

int player_playlist_append_list(const char *path) {
	int rc;
	const char *cmd[] = { "loadlist", path, "append", NULL };

	rc = mpv_command(player_ctx, cmd);
	if (rc < 0) return rc;

	return 0;
}

int player_playlist_goto_index(int64_t index) {
	int rc;

	rc = mpv_set_property(
		player_ctx, "playlist-pos", MPV_FORMAT_INT64, &index
	);
	if (rc < 0) return rc;

	playlist_pos = player_playlist_position();

	return 0;
}

int player_playlist_remove_index(int64_t index) {
	int rc;

	_free_ char *index_str = NULL;

	if (index == -1)
		rc = asprintf(&index_str, "%s", "current");
	else
		rc = asprintf(&index_str, "%" PRId64, index);

	if (rc < 0) fail_printf("OOM");

	const char *cmd[] = { "playlist_remove", index_str, NULL };

	rc = mpv_command(player_ctx, cmd);
	if (rc < 0) return rc;

	playlist_pos = player_playlist_position();

	return 0;
}

int player_playlist_next(void) {
	int rc;
	const char *cmd[] = { "playlist_next", "force", NULL };

	rc = mpv_command(player_ctx, cmd);
	if (rc < 0) return rc;

	return 0;
}

int player_playlist_prev(void) {
	int rc;
	const char *cmd[] = { "playlist_prev", "weak", NULL };

	rc = mpv_command(player_ctx, cmd);
	if (rc < 0) return rc;

	return 0;
}

static void player_print_playlist_status(void) {
	char *path = mpv_get_property_string(player_ctx, "path");

	int64_t count = player_playlist_count();
	int64_t pos   = player_playlist_position();

	debug_printf("position: %d, count: %d, path: %s", pos + 1, count, path);

	mpv_free(path);
}

static gboolean player_loop_fd_prepare(GSource *src, int *timeout) {
	*timeout = -1;

	return FALSE;
}

static gboolean player_loop_fd_check(GSource *src) {
	GIOCondition cond = g_source_query_unix_fd(src, ((GPlayerSource *) src) -> fd);

	flush_pipe(mpv_get_wakeup_pipe(player_ctx));

	return (cond > 0);
}

static gboolean player_loop_fd_dispatch(GSource *src, GSourceFunc cb, void *p) {
	enum player_status prev_status = player_status;

	mpv_event *event = mpv_wait_event(player_ctx, 0);
	debug_printf("event: %s", mpv_event_name(event -> event_id));

	switch (event -> event_id) {
		case MPV_EVENT_IDLE: {
			player_status = IDLE;

			if (prev_status == STARTING)
				break;

			dbus_handle_event(STATUS_CHANGED);

			if (prev_status != STOPPED)
				player_playback_play();
			else
				dbus_handle_event(TRACK_CHANGED);

			break;
		}

		case MPV_EVENT_PAUSE: {
			player_status = PAUSED;
			dbus_handle_event(STATUS_CHANGED);
			break;
		}

		case MPV_EVENT_UNPAUSE: {
			player_status = PLAYING;
			dbus_handle_event(STATUS_CHANGED);
			break;
		}

		case MPV_EVENT_START_FILE: {
			playlist_pos = player_playlist_position();

			player_print_playlist_status();
			break;
		}

		case MPV_EVENT_END_FILE:
			break;

		case MPV_EVENT_METADATA_UPDATE: {
			dbus_handle_event(TRACK_CHANGED);
			break;
		}

		case MPV_EVENT_LOG_MESSAGE: {
			mpv_event_log_message *log = event -> data;

			size_t       log_len = strlen(log -> text);
			_free_ char *log_str = strdup(log -> text);

			if (!log_str)
				break;

			/* TODO: buffer the log if it doesn't end with a \n */
			if (log_str[log_len - 1] == '\n')
				log_str[log_len - 1] = '\0';

			err_printf(
				"%s: %s: %s",
				log -> level,
				log -> prefix,
				log_str
			);

			break;
		}

		case MPV_EVENT_NONE:
			return TRUE;

		default:
			break;
	}

	return player_loop_fd_dispatch(src, cb, p);
}

static void player_loop_fd_finalize(GSource *src) {
	mpv_terminate_destroy(player_ctx);
}
