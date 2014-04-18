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

#include <mpv/client.h>

#include "dbus.h"
#include "library.h"
#include "player.h"
#include "config.h"
#include "printf.h"

enum player_status {
	STARTING,
	IDLE,
	PLAYING,
	PAUSED,
	STOPPED,
};

static pthread_t   player_thr;
static mpv_handle *player_ctx;

static enum player_status player_status;

static enum replaygain player_replaygain = PLAYER_REPLAYGAIN_TRACK;
static bool player_loop = false;

static int64_t playlist_pos = -1;

int64_t player_playlist_count(void);
int64_t player_playlist_position(void);

#define player_check_error(MSG, RC)				\
	if (RC < 0)						\
		fail_printf(MSG ": %s", mpv_error_string(RC));

static void player_print_metadata(void);
static void player_print_playlist_status(void);

static void *player_start_thread(void *ptr) {
	mpv_handle *ctx = ptr;

	player_status = STARTING;

	while (1) {
		enum player_status prev_status = player_status;

		mpv_event *event = mpv_wait_event(ctx, 10000);
		debug_printf("event: %s", mpv_event_name(event -> event_id));

		switch (event -> event_id) {
			case MPV_EVENT_IDLE: {
				player_status = IDLE;

				if ((prev_status == STARTING) ||
				    (prev_status == STOPPED))
					break;

				dbus_emit_signal(STATUS_CHANGED);

				player_playback_play();
				break;
			}

			case MPV_EVENT_PAUSE: {
				player_status = PAUSED;
				dbus_emit_signal(STATUS_CHANGED);
				break;
			}

			case MPV_EVENT_UNPAUSE: {
				player_status = PLAYING;
				dbus_emit_signal(STATUS_CHANGED);
				break;
			}

			case MPV_EVENT_START_FILE: {
				playlist_pos = player_playlist_position();
				dbus_emit_signal(TRACK_CHANGED);

				player_print_playlist_status();
				break;
			}

			case MPV_EVENT_END_FILE:
				break;

			case MPV_EVENT_METADATA_UPDATE: {
				player_print_metadata();
				break;
			}

			case MPV_EVENT_LOG_MESSAGE: {
				mpv_event_log_message *log = event -> data;

				err_printf(
					"%s: %s: %s",
					log -> level,
					log -> prefix,
					log -> text
				);

				break;
			}

			default:
				break;
		}

		if (event -> event_id == MPV_EVENT_SHUTDOWN)
			break;
	}

	return NULL;
}

void player_init(void) {
	int rc;
	player_ctx = mpv_create();

	if (player_ctx == NULL)
		fail_printf("Could not create player");

	rc = mpv_set_option_string(player_ctx, "no-video", "");
	player_check_error("Could not set no-video", rc);

	rc = mpv_set_option_string(player_ctx, "no-softvol", "");
	player_check_error("Could not set no-softvol", rc);

	mpv_request_log_messages(player_ctx, "error");

	switch (cfg.rgain) {
		case PLAYER_REPLAYGAIN_TRACK:
			rc = mpv_set_option_string(
				player_ctx, "af",
				"@replaygain_track:volume=replaygain-track"
			);
			break;

		case PLAYER_REPLAYGAIN_ALBUM:
			rc = mpv_set_option_string(
				player_ctx, "af",
				"@replaygain_album:volume=replaygain-album"
			);
			break;

		case PLAYER_REPLAYGAIN_NONE:
			rc = MPV_ERROR_SUCCESS;
			break;
	}

	player_check_error("Could not set replaygain", rc);

	rc = mpv_initialize(player_ctx);
	player_check_error("Could not initialize player", rc);

	pthread_create(&player_thr, NULL, player_start_thread, player_ctx);
}

void player_make_status(GVariantBuilder *status) {
	int i;

	mpv_node metadata;

	GVariantBuilder *meta_build;
	double length = 0, position = 0, percent = 0;

	char *path  = mpv_get_property_string(player_ctx, "path");

	mpv_get_property(player_ctx, "length", MPV_FORMAT_DOUBLE, &length);
	mpv_get_property(player_ctx, "time-pos", MPV_FORMAT_DOUBLE, &position);
	mpv_get_property(player_ctx, "percent-pos", MPV_FORMAT_DOUBLE, &percent);

	mpv_get_property(player_ctx, "metadata", MPV_FORMAT_NODE, &metadata);

	switch (player_status) {
		case IDLE:
			g_variant_builder_add(status, "s", "idle");
			break;

		case PLAYING:
			g_variant_builder_add(status, "s", "play");
			break;

		case PAUSED:
			g_variant_builder_add(status, "s", "pause");
			break;

		default:
			err_printf("Invalid state");
			break;
	}

	g_variant_builder_add(status, "s", path);

	g_variant_builder_add(status, "d", length);
	g_variant_builder_add(status, "d", position);
	g_variant_builder_add(status, "d", percent);

	meta_build = g_variant_builder_new(G_VARIANT_TYPE("a{ss}"));

	if (metadata.format == MPV_FORMAT_NODE_MAP) {
		for (i = 0; i < metadata.u.list -> num; i++) {
			char *key = metadata.u.list -> keys[i];
			char *val = metadata.u.list -> values[i].u.string;

			g_variant_builder_add(meta_build, "{ss}", key, val);
		}
	}

	g_variant_builder_add_value(status, g_variant_builder_end(meta_build));

	mpv_free_node_contents(&metadata);

	g_variant_builder_add(status, "s", player_playback_replaygain_tostr());

	g_variant_builder_add(status, "b", player_loop);

	mpv_free(path);
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

void player_playback_start(void) {
	int rc;

	char *path = NULL;

	int64_t pos   = playlist_pos;
	int64_t count = player_playlist_count();

	if ((pos == (count - 1) && (path = library_random())))
		player_playlist_append_file(path);

	++pos;

	rc = mpv_set_property(
		player_ctx, "playlist-pos", MPV_FORMAT_INT64, &pos
	);

	if (rc < 0) {
		err_printf("Could not set playlist position: %s",
		           mpv_error_string(rc));
		return;
	}

	player_status = PLAYING;
}

void player_playback_play(void) {
	int rc;

	switch (player_status) {
		case IDLE:
			player_playback_start();

		case PLAYING:
		case PAUSED:
			rc = mpv_set_property_string(player_ctx, "pause", "no");
			player_check_error("Could not unpause", rc);
			break;

		default:
			err_printf("Invalid state");
			break;
	}
}

void player_playback_pause(void) {
	int rc;

	switch (player_status) {
		case PLAYING:
		case PAUSED:
			rc = mpv_set_property_string(player_ctx, "pause", "yes");
			player_check_error("Could not pause", rc);
			break;

		default:
			err_printf("Invalid state");
			break;
	}
}

void player_playback_toggle(void) {
	switch (player_status) {
		case IDLE:
		case PAUSED:
			player_playback_play();
			break;

		case PLAYING:
			player_playback_pause();
			break;

		default:
			err_printf("Invalid state");
			break;
	}
}

void player_playback_stop(void) {
	int rc;
	const char *cmd_clear[]  = { "playlist_clear", NULL };
	const char *cmd_remove[] = { "playlist_remove", "current", NULL };

	switch (player_status) {
		case PLAYING:
		case PAUSED:
			rc = mpv_command(player_ctx, cmd_clear);
			player_check_error("Could not stop", rc);

			rc = mpv_command(player_ctx, cmd_remove);
			player_check_error("Could not stop", rc);

			playlist_pos = -1;

			break;

		default:
			break;
	}

	player_status = STOPPED;
}

void player_playback_seek(int64_t secs) {
	int rc;
	char secs_arg[9];
	const char *cmd[] = { "seek", secs_arg, NULL };

	sprintf(secs_arg, "%" PRId64, secs);

	rc = mpv_command(player_ctx, cmd);
	player_check_error("Could not seek", rc);
}

void player_playback_replaygain(enum replaygain mode) {
	int rc;
	const char *cmd[] = { "af", NULL, NULL, NULL };

	switch (mode) {
		case PLAYER_REPLAYGAIN_TRACK:
			player_playback_replaygain(PLAYER_REPLAYGAIN_NONE);

			cmd[1] = "add";
			cmd[2] = "@replaygain_track:volume=replaygain-track";
			break;

		case PLAYER_REPLAYGAIN_ALBUM:
			player_playback_replaygain(PLAYER_REPLAYGAIN_NONE);

			cmd[1] = "add";
			cmd[2] = "@replaygain_album:volume=replaygain-album";
			break;

		case PLAYER_REPLAYGAIN_NONE:
			cmd[1] = "del";
			cmd[2] = "@replaygain_track,@replaygain_album";
			break;
	}

	player_replaygain = mode;

	rc = mpv_command(player_ctx, cmd);
	player_check_error("Could not change af chain", rc);
}

char *player_playback_replaygain_tostr(void) {
	switch (player_replaygain) {
		case PLAYER_REPLAYGAIN_TRACK:
			return "track";

		case PLAYER_REPLAYGAIN_ALBUM:
			return "album";

		case PLAYER_REPLAYGAIN_NONE:
			return "none";
			break;
	}

	return NULL;
}

void player_playback_loop(bool enable) {
	int rc;

	player_loop = enable;

	rc = mpv_set_property_string(player_ctx, "loop", enable ? "inf" : "no");
	player_check_error("Could not set loop", rc);
}

int64_t player_playlist_count(void) {
	int rc;
	int64_t count = -1;

	rc = mpv_get_property(player_ctx, "playlist-count", MPV_FORMAT_INT64, &count);
	player_check_error("Could not get playlist count", rc);

	return count;
}

int64_t player_playlist_position(void) {
	int64_t pos = -1;

	int rc = mpv_get_property(
		player_ctx, "playlist-pos", MPV_FORMAT_INT64, &pos
	);

	if (rc < 0)
		return -1;

	return pos;
}

void player_playlist_append_file(const char *path) {
	int rc;
	const char *cmd[] = { "loadfile", path, "append", NULL };

	rc = mpv_command(player_ctx, cmd);
	player_check_error("Could not load file", rc);
}

void player_playlist_append_list(const char *path) {
	int rc;
	const char *cmd[] = { "loadlist", path, "append", NULL };

	rc = mpv_command(player_ctx, cmd);
	player_check_error("Could not load file", rc);
}

void player_playlist_remove_index(int64_t index) {
	int rc;
	char *index_str = NULL;

	rc = asprintf(&index_str, "%" PRId64, index);
	if (rc < 0) fail_printf("OOM");

	const char *cmd[] = { "playlist_remove", index_str, NULL };

	rc = mpv_command(player_ctx, cmd);
	player_check_error("Could not load file", rc);

	playlist_pos = player_playlist_position();

	free(index_str);
}

void player_playlist_next(void) {
	int rc;
	const char *cmd[] = { "playlist_next", "force", NULL };

	rc = mpv_command(player_ctx, cmd);
	player_check_error("Could not skip to the next playlist entry", rc);
}

void player_playlist_prev(void) {
	int rc;
	const char *cmd[] = { "playlist_prev", "weak", NULL };

	rc = mpv_command(player_ctx, cmd);
	player_check_error("Could not skip to the prev playlist entry", rc);
}

void player_destroy(void) {
	int rc;
	const char *cmd[] = { "quit", NULL };

	rc = mpv_command(player_ctx, cmd);
	player_check_error("Could not quit player", rc);

	pthread_join(player_thr, NULL);

	mpv_destroy(player_ctx);
}

static void player_print_metadata(void) {
	int rc, i;
	mpv_node metadata;

	rc = mpv_get_property(player_ctx, "metadata", MPV_FORMAT_NODE, &metadata);
	if (rc != MPV_ERROR_SUCCESS)
		return;

	if (metadata.format != MPV_FORMAT_NODE_MAP)
		err_printf("No metadata");

	debug_printf("tags:");
	for (i = 0; i < metadata.u.list -> num; i++) {
		char *key = metadata.u.list -> keys[i];
		char *val = metadata.u.list -> values[i].u.string;

		debug_printf(" %s: %s", key, val);
	}

	mpv_free_node_contents(&metadata);
}

static void player_print_playlist_status(void) {
	char *path = mpv_get_property_string(player_ctx, "path");

	int64_t count = player_playlist_count();
	int64_t pos   = player_playlist_position();

	debug_printf("position: %d, count: %d, path: %s", pos + 1, count, path);

	mpv_free(path);
}
