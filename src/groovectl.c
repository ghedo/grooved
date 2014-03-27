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
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <termios.h>

#include <inttypes.h>

#include <sys/select.h>

#include <glyr/glyr.h>

#include "dbus.h"
#include "dbus-common.h"
#include "printf.h"

static void cmd_status(GroovedPlayer *proxy);
static void cmd_play(GroovedPlayer *proxy);
static void cmd_pause(GroovedPlayer *proxy);
static void cmd_toggle(GroovedPlayer *proxy);
static void cmd_stop(GroovedPlayer *proxy);
static void cmd_next(GroovedPlayer *proxy);
static void cmd_prev(GroovedPlayer *proxy);
static void cmd_seek(GroovedPlayer *proxy, const char *secs);
static void cmd_add(GroovedPlayer *proxy, const char *path);
static void cmd_rgain(GroovedPlayer *proxy, const char *mode);
static void cmd_loop(GroovedPlayer *proxy, const char *mode);
static void cmd_lyrics(GroovedPlayer *proxy);
static void cmd_interactive(GroovedPlayer *proxy);
static void cmd_quit(GroovedPlayer *proxy);

static inline void help();

int main(int argc, char *argv[]) {
	GError *err = NULL;

	GroovedPlayer *proxy = grooved_player_proxy_new_for_bus_sync(
		G_BUS_TYPE_SESSION, G_DBUS_PROXY_FLAGS_NONE,
		GROOVED_DBUS_NAME, GROOVED_DBUS_PLAYER_PATH, NULL, &err
	);

	if (argc < 2)
		cmd_interactive(proxy);
	else if (strcmp("status", argv[1]) == 0)
		cmd_status(proxy);
	else if (strcmp("play", argv[1]) == 0)
		cmd_play(proxy);
	else if (strcmp("pause", argv[1]) == 0)
		cmd_pause(proxy);
	else if (strcmp("toggle", argv[1]) == 0)
		cmd_toggle(proxy);
	else if (strcmp("stop", argv[1]) == 0)
		cmd_stop(proxy);
	else if (strcmp("next", argv[1]) == 0)
		cmd_next(proxy);
	else if (strcmp("prev", argv[1]) == 0)
		cmd_prev(proxy);
	else if (strcmp("seek", argv[1]) == 0) {
		if ((argc < 3))
			fail_printf("Invalid seek value");

		cmd_seek(proxy, argv[2]);
	} else if (strcmp("add", argv[1]) == 0) {
		int i;

		for (i = 2; i < argc; i++) {
			char *path;

			if (access(argv[i], F_OK) == 0)
				path = realpath(argv[i], NULL);
			else
				path = strdup(argv[i]);

			cmd_add(proxy, path);

			printf("Added track '%s'\n", path);
		}
	} else if (strcmp("rgain", argv[1]) == 0) {
		if ((argc < 3) ||
		    (strcmp("track", argv[2]) &&
		     strcmp("album", argv[2]) &&
		     strcmp("none", argv[2])))
			fail_printf("Invalid replaygain mode");

		cmd_rgain(proxy, argv[2]);
	} else if (strcmp("loop", argv[1]) == 0) {
		if ((argc < 3) ||
		    (strcmp("on", argv[2]) &&
		     strcmp("off", argv[2])))
			fail_printf("Invalid replaygain mode");

		cmd_loop(proxy, argv[2]);
	} else if (strcmp("lyrics", argv[1]) == 0)
		cmd_lyrics(proxy);
	else if (strcmp("quit", argv[1]) == 0)
		cmd_quit(proxy);
	else if (strcmp("help", argv[1]) == 0)
		help();
	else if (strcmp("introspect", argv[1]) == 0)
		puts(GROOVED_DBUS_INTROSPECTION);
	else {
		err_printf("Invalid command '%s'", argv[1]);
		help();
	}

	g_object_unref(proxy);

	return 0;
}

static void cmd_status(GroovedPlayer *proxy) {
	GError *err = NULL;

	gboolean loop;
	char *state, *replaygain, *path;
	double len, pos, percent;
	GVariant *metadata;

	grooved_player_call_status_sync(
		proxy, &state, &path, &len, &pos, &percent,
		&metadata, &replaygain, &loop, NULL, &err
	);

	if (err != NULL)
		fail_printf("%s", err -> message);

	GVariantIter iter;

	char *key, *val;

	puts("Tags:");

	g_variant_iter_init(&iter, metadata);
	while (g_variant_iter_loop(&iter, "{ss}", &key, &val))
		printf(" %s: %s\n", key, val);

	int pos_min = pos / 60;
	int pos_sec = (int) pos % 60;

	int len_min = len / 60;
	int len_sec = (int) len % 60;

	printf(
		"[%s]   %d:%02d/%d:%02d   (%d%%)\n",
		state, pos_min, pos_sec, len_min, len_sec, (int) percent
	);

	printf(
		"replaygain: %s   loop: %s\n",
		replaygain, loop ? "yes" : "no"
	);
}

static void cmd_play(GroovedPlayer *proxy) {
	GError *err = NULL;
	grooved_player_call_play_sync(proxy, NULL, &err);

	if (err != NULL)
		fail_printf("%s", err -> message);
}

static void cmd_pause(GroovedPlayer *proxy) {
	GError *err = NULL;
	grooved_player_call_pause_sync(proxy, NULL, &err);

	if (err != NULL)
		fail_printf("%s", err -> message);
}

static void cmd_toggle(GroovedPlayer *proxy) {
	GError *err = NULL;
	grooved_player_call_toggle_sync(proxy, NULL, &err);

	if (err != NULL)
		fail_printf("%s", err -> message);
}

static void cmd_stop(GroovedPlayer *proxy) {
	GError *err = NULL;
	grooved_player_call_stop_sync(proxy, NULL, &err);

	if (err != NULL)
		fail_printf("%s", err -> message);
}

static void cmd_next(GroovedPlayer *proxy) {
	GError *err = NULL;
	grooved_player_call_next_sync(proxy, NULL, &err);

	if (err != NULL)
		fail_printf("%s", err -> message);
}

static void cmd_prev(GroovedPlayer *proxy) {
	GError *err = NULL;
	grooved_player_call_prev_sync(proxy, NULL, &err);

	if (err != NULL)
		fail_printf("%s", err -> message);
}

static void cmd_seek(GroovedPlayer *proxy, const char *secs) {
	int rc;
	int64_t seconds;
	GError *err = NULL;

	rc = sscanf(secs, "%" PRId64, &seconds);
	if (rc != 1)
		fail_printf("Invalid seek value");

	grooved_player_call_seek_sync(proxy, seconds, NULL, &err);

	if (err != NULL)
		fail_printf("%s", err -> message);
}

static void cmd_add(GroovedPlayer *proxy, const char *path) {
	GError *err = NULL;
	grooved_player_call_add_track_sync(proxy, path, NULL, &err);

	if (err != NULL)
		fail_printf("%s", err -> message);
}

static void cmd_rgain(GroovedPlayer *proxy, const char *mode) {
	GError *err = NULL;
	grooved_player_call_replaygain_sync(proxy, mode, NULL, &err);

	if (err != NULL)
		fail_printf("%s", err -> message);
}

static void cmd_loop(GroovedPlayer *proxy, const char *mode) {
	bool enable;
	GError *err = NULL;

	if (strcmp("on", mode) == 0)
		enable = true;
	else if (strcmp("off", mode) == 0)
		enable = false;
	else
		fail_printf("Invalid value '%s'", mode);

	grooved_player_call_loop_sync(proxy, enable, NULL, &err);

	if (err != NULL)
		fail_printf("%s", err -> message);
}

static void cmd_lyrics(GroovedPlayer *proxy) {
	GVariantIter iter;
	GError *err = NULL;

	GlyrQuery q;
	GLYR_ERROR glyr_err;
	GlyrMemCache *cache;

	char *key, *val;
	GVariant *metadata;

	char *title = NULL, *artist = NULL, *album = NULL;

	glyr_init();

	glyr_query_init(&q);

	glyr_opt_type(&q, GLYR_GET_LYRICS);

	grooved_player_call_status_sync(
		proxy, NULL, NULL, NULL, NULL, NULL,
		&metadata, NULL, NULL, NULL, &err
	);

	if (err != NULL)
		fail_printf("%s", err -> message);

	g_variant_iter_init(&iter, metadata);
	while (g_variant_iter_loop(&iter, "{ss}", &key, &val)) {
		if (strcasecmp("title", key) == 0)
			title = strdup(val);
		else if (strcasecmp("artist", key) == 0)
			artist = strdup(val);
		else if (strcasecmp("album", key) == 0)
			album = strdup(val);
	}

	if (!title)
		fail_printf("Could not find the title of the current song");

	if (title)
		glyr_opt_title(&q,  (char *) title);

	if (artist)
		glyr_opt_artist(&q, (char *) artist);

	if (album)
		glyr_opt_album(&q,  (char *) album);

	cache = glyr_get(&q, &glyr_err, NULL);

	if (cache == NULL) {
		fail_printf(
			"No lyrics found for the song '%s' of '%s'",
			title, artist
		);

		return;
	}

	glyr_cache_write(cache, "stdout");

	glyr_free_list(cache);
	glyr_cleanup();
}

static void cmd_interactive(GroovedPlayer *proxy) {
	fd_set rfds;
	struct timeval tv;

	GError *err = NULL;

	char *state;
	double len, pos, percent;

	struct termios tio_new;
	tcgetattr(0, &tio_new);

	tio_new.c_lflag &= ~(ICANON|ECHO); /* Clear ICANON and ECHO. */
	tio_new.c_cc[VMIN] = 1;
	tio_new.c_cc[VTIME] = 0;
	tcsetattr(0,TCSANOW,&tio_new);

	FD_ZERO(&rfds);

	do {
		if (FD_ISSET(0, &rfds)) {
			if ((getchar() == 27) && (getchar() == 91)) {
				int ch = getchar();

				switch (ch) {
					case 65: /* up */
						cmd_seek(proxy, "15");
						break;

					case 66: /* down */
						cmd_seek(proxy, "-15");
						break;

					case 67: /* right */
						cmd_seek(proxy, "5");
						break;

					case 68: /* left */
						cmd_seek(proxy, "-5");
						break;

					default:
						break;
				}
			}
		}

		grooved_player_call_status_sync(
			proxy, &state, NULL, &len, &pos, &percent,
			NULL, NULL, NULL, NULL, &err
		);

		int pos_min = pos / 60;
		int pos_sec = (int) pos % 60;

		int len_min = len / 60;
		int len_sec = (int) len % 60;

		printf(
			"\r[%s]   %d:%02d/%d:%02d   (%d%%)",
			state, pos_min, pos_sec, len_min, len_sec, (int) percent
		);
		fflush(stdout);

		tv.tv_sec = 0;
		tv.tv_usec = 100000;

		FD_SET(0, &rfds);
	} while (select(1, &rfds, NULL, NULL, &tv) >= 0);
}

static void cmd_quit(GroovedPlayer *proxy) {
	GError *err = NULL;
	grooved_player_call_quit_sync(proxy, NULL, &err);

	if (err != NULL)
		fail_printf("%s", err -> message);

	puts("Bye");
}

static inline void help() {
	#define CMD_HELP(CMDL, MSG) printf("  %s      \t%s.\n", COLOR_YELLOW CMDL COLOR_OFF, MSG);

	printf(COLOR_RED "Usage: " COLOR_OFF);
	printf(COLOR_GREEN "groovectl " COLOR_OFF);
	puts("COMMAND [ARGS]\n");

	puts(COLOR_RED " Commands:" COLOR_OFF);

	CMD_HELP("status",	"Show the status of the player");
	CMD_HELP("play",	"Unpause the player");
	CMD_HELP("pause",	"Pause the player");
	CMD_HELP("toggle",	"Toggle the player's pause status");
	CMD_HELP("next",	"Skip to next track");
	CMD_HELP("prev",	"Skip to previous track");
	CMD_HELP("add",		"Append tracks to the player's tracklist");
	CMD_HELP("rgain",	"Set the player's replaygain mode");
	CMD_HELP("loop",	"Set the player's loop mode");
	CMD_HELP("lyrics",	"Download and show lyrics for the currently playing track");
	CMD_HELP("quit",	"Shutdown the player");

	puts("");
}
