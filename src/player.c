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

#include "library.h"
#include "player.h"
#include "config.h"
#include "printf.h"

static pthread_t   player_thr;
static mpv_handle *player_ctx;

static bool player_is_started = false;
static bool player_is_idle    = true;

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

	while (1) {
		mpv_event *event = mpv_wait_event(ctx, 10000);
		debug_printf("event: %s", mpv_event_name(event -> event_id));

		switch (event -> event_id) {
			case MPV_EVENT_IDLE: {
				player_is_idle = true;

				if (!player_is_started) {
					player_is_started = true;
					break;
				}

				player_playback_play();

				break;
			}

			case MPV_EVENT_START_FILE: {
				playlist_pos = player_playlist_position();

				player_print_playlist_status();
				player_print_metadata();

				break;
			}

			case MPV_EVENT_END_FILE: {
				debug_printf("prev position: %d\n",
					     playlist_pos + 1);
				player_print_playlist_status();

				break;
			}

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

	rc = mpv_set_option_string(player_ctx, "no-video", "yes");
	player_check_error("Could not set no-video", rc);

	rc = mpv_set_option_string(player_ctx, "no-softvol", "yes");
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

void player_status(GVariantBuilder *status) {
	int i;

	mpv_node metadata;

	GVariantBuilder *meta_build;
	double length = 0, position = 0, percent = 0;

	char *state = mpv_get_property_string(player_ctx, "pause");
	char *path  = mpv_get_property_string(player_ctx, "path");

	mpv_get_property(player_ctx, "length", MPV_FORMAT_DOUBLE, &length);
	mpv_get_property(player_ctx, "time-pos", MPV_FORMAT_DOUBLE, &position);
	mpv_get_property(player_ctx, "percent-pos", MPV_FORMAT_DOUBLE, &percent);

	mpv_get_property(player_ctx, "metadata", MPV_FORMAT_NODE, &metadata);

	if (player_is_idle)
		g_variant_builder_add(status, "s", "idle");
	else if (state && strcmp(state, "no") == 0)
		g_variant_builder_add(status, "s", "play");
	else if (state && strcmp(state, "yes") == 0)
		g_variant_builder_add(status, "s", "pause");

	g_variant_builder_add(status, "s", path);

	g_variant_builder_add(status, "d", length);
	g_variant_builder_add(status, "d", position);
	g_variant_builder_add(status, "d", percent);

	meta_build = g_variant_builder_new(G_VARIANT_TYPE("a{ss}"));

	if (metadata.format == MPV_FORMAT_NODE_ARRAY) {
		for (i = 0; i < metadata.u.list -> num; i += 2) {
			char *key = metadata.u.list -> values[i].u.string;
			char *val = metadata.u.list -> values[i + 1].u.string;

			g_variant_builder_add(meta_build, "{ss}", key, val);
		}
	}

	g_variant_builder_add_value(status, g_variant_builder_end(meta_build));

	mpv_free_node_contents(&metadata);

	g_variant_builder_add(status, "s", player_playback_replaygain_tostr());

	g_variant_builder_add(status, "b", player_loop);

	mpv_free(state);
	mpv_free(path);
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

	player_is_idle = false;
}

void player_playback_play(void) {
	int rc;

	if (player_is_idle) {
		player_playback_start();
		return;
	}

	rc = mpv_set_property_string(player_ctx, "pause", "no");
	player_check_error("Could not unpause", rc);
}

void player_playback_pause(void) {
	int rc;

	rc = mpv_set_property_string(player_ctx, "pause", "yes");
	player_check_error("Could not pause", rc);
}

void player_playback_toggle(void) {
	char *val = mpv_get_property_string(player_ctx, "pause");

	if (player_is_idle || (val && (strcmp("yes", val) == 0)))
		player_playback_play();
	else if (val && (strcmp("no", val) == 0))
		player_playback_pause();
	else
		err_printf("Could not get player state");

	mpv_free(val);
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

	if (metadata.format != MPV_FORMAT_NODE_ARRAY)
		err_printf("No metadata");

	debug_printf("tags:");
	for (i = 0; i < metadata.u.list -> num; i += 2) {
		char *key = metadata.u.list -> values[i].u.string;
		char *val = metadata.u.list -> values[i + 1].u.string;

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
