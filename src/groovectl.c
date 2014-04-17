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

#include <glib.h>
#include <gio/gio.h>
#include <glyr/glyr.h>

#include "dbus.h"
#include "dbus-common.h"
#include "printf.h"

typedef void (*cmd_handle)(GroovedPlayer *proxy, int argc, char *argv[]);

#define CMD_HANDLE(NAME) \
	void cmd_##NAME(GroovedPlayer *proxy, int argc, char *argv[])

struct handle_cmd {
	const char *name;
	cmd_handle fn;
	const char *desc;
};

static inline void help(void);

CMD_HANDLE(status) {
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

CMD_HANDLE(play) {
	GError *err = NULL;
	grooved_player_call_play_sync(proxy, NULL, &err);

	if (err != NULL)
		fail_printf("%s", err -> message);
}

CMD_HANDLE(pause) {
	GError *err = NULL;
	grooved_player_call_pause_sync(proxy, NULL, &err);

	if (err != NULL)
		fail_printf("%s", err -> message);
}

CMD_HANDLE(toggle) {
	GError *err = NULL;
	grooved_player_call_toggle_sync(proxy, NULL, &err);

	if (err != NULL)
		fail_printf("%s", err -> message);
}

CMD_HANDLE(stop) {
	GError *err = NULL;
	grooved_player_call_stop_sync(proxy, NULL, &err);

	if (err != NULL)
		fail_printf("%s", err -> message);
}

CMD_HANDLE(next) {
	GError *err = NULL;
	grooved_player_call_next_sync(proxy, NULL, &err);

	if (err != NULL)
		fail_printf("%s", err -> message);
}

CMD_HANDLE(prev) {
	GError *err = NULL;
	grooved_player_call_prev_sync(proxy, NULL, &err);

	if (err != NULL)
		fail_printf("%s", err -> message);
}

void stop_on_track_changed(GroovedPlayer *obj, void *data) {
	GMainLoop *loop = data;

	cmd_stop(obj, 0, NULL);

	g_main_loop_quit(loop);
}

CMD_HANDLE(last) {
	GMainLoop *loop = g_main_loop_new(NULL, FALSE);

	g_signal_connect(proxy, "track_changed",
	                 G_CALLBACK(stop_on_track_changed), loop);

	g_main_loop_run(loop);
}

CMD_HANDLE(list) {
	GError *err = NULL;

	char **files;
	int64_t i, count, pos;

	grooved_player_call_list_sync(proxy, &files, &count, &pos, NULL, &err);

	if (err != NULL)
		fail_printf("%s", err -> message);

	for (i = 0; i < count; i++)
		printf("%c %s\n", (pos == i) ? '*' : ' ', files[i]);
}

CMD_HANDLE(seek) {
	int rc;
	int64_t seconds;
	GError *err = NULL;

	if ((argc < 3))
		fail_printf("Invalid seek value");

	rc = sscanf(argv[2], "%" PRId64, &seconds);
	if (rc != 1)
		fail_printf("Invalid seek value");

	grooved_player_call_seek_sync(proxy, seconds, NULL, &err);

	if (err != NULL)
		fail_printf("%s", err -> message);
}

static void add_track(GroovedPlayer *proxy, char *path) {
	GError *err = NULL;

	if (access(path, F_OK) == 0)
		path = realpath(path, NULL);
	else
		path = strdup(path);

	grooved_player_call_add_track_sync(proxy, path, NULL, &err);

	if (err != NULL)
		fail_printf("%s", err -> message);

	printf("Added track '%s'\n", path);
}

CMD_HANDLE(add) {
	int i;

	if (strcmp(argv[2], "-") == 0) {
		char *path = NULL;
		size_t rc, n = 0;

		while ((rc = getline(&path, &n, stdin)) != -1) {
			path[rc - 1] = '\0';
			add_track(proxy, path);
		}
	} else {
		for (i = 2; i < argc; i++)
			add_track(proxy, argv[i]);
	}
}

CMD_HANDLE(rgain) {
	GError *err = NULL;

	if ((argc < 3) ||
	    (strcmp("track", argv[2]) &&
	     strcmp("album", argv[2]) &&
	     strcmp("none", argv[2])))
		fail_printf("Invalid replaygain mode");

	grooved_player_call_replaygain_sync(proxy, argv[2], NULL, &err);

	if (err != NULL)
		fail_printf("%s", err -> message);
}

CMD_HANDLE(loop) {
	bool enable;
	GError *err = NULL;

	if (argc < 3)
		fail_printf("Missing loop mode");

	if (strcmp("on", argv[2]) == 0)
		enable = true;
	else if (strcmp("off", argv[2]) == 0)
		enable = false;
	else
		fail_printf("Invalid loop mode '%s'", argv[2]);

	grooved_player_call_loop_sync(proxy, enable, NULL, &err);

	if (err != NULL)
		fail_printf("%s", err -> message);
}

CMD_HANDLE(lyrics) {
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

CMD_HANDLE(interactive) {
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
				char *seek_arg[] = { NULL, NULL, "0" };

				switch (ch) {
					case 65: /* up */
						seek_arg[2] = "15";
						break;

					case 66: /* down */
						seek_arg[2] = "-15";
						break;

					case 67: /* right */
						seek_arg[2] = "5";
						break;

					case 68: /* left */
						seek_arg[2] = "-5";
						break;

					default:
						break;
				}

				cmd_seek(proxy, 3, seek_arg);
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

CMD_HANDLE(quit) {
	GError *err = NULL;
	grooved_player_call_quit_sync(proxy, NULL, &err);

	if (err != NULL)
		fail_printf("%s", err -> message);

	puts("Bye");
}

CMD_HANDLE(help) {
	help();
}

struct handle_cmd cmds[] = {
	{ "add",    cmd_add,    "Append tracks to the player's tracklist" },
	{ "help",   cmd_help,   "Show this help" },
	{ "last",   cmd_last,   "Stop playback after currently playing track" },
	{ "list",   cmd_list,   "Show tracklist" },
	{ "loop",   cmd_loop,   "Set the player's loop mode" },
	{ "lyrics", cmd_lyrics, "Download and show lyrics for the currently playing track" },
	{ "next",   cmd_next,   "Skip to next track" },
	{ "pause",  cmd_pause,  "Pause the player" },
	{ "play",   cmd_play,   "Unpause the player" },
	{ "prev",   cmd_prev,   "Skip to previous track" },
	{ "quit",   cmd_quit,   "Shutdown the player" },
	{ "rgain",  cmd_rgain,  "Set the player's replaygain mode" },
	{ "seek",   cmd_seek,   "Seek by a relative amount of seconds" },
	{ "status", cmd_status, "Show the status of the player" },
	{ "stop",   cmd_stop,   "Stop playback and clear tracklist" },
	{ "toggle", cmd_toggle, "Toggle the player's pause status" },
};

int main(int argc, char *argv[]) {
	int i;
	bool match = false;
	GError *err = NULL;

	GroovedPlayer *proxy = grooved_player_proxy_new_for_bus_sync(
		G_BUS_TYPE_SESSION, G_DBUS_PROXY_FLAGS_NONE,
		GROOVED_DBUS_NAME, GROOVED_DBUS_PLAYER_PATH, NULL, &err
	);

	if (argc < 2)
		cmd_interactive(proxy, argc, argv);

	for (i = 0; i < sizeof(cmds) / sizeof(cmds[0]); i++) {
		if (strcmp(cmds[i].name, argv[1]) == 0) {
			cmds[i].fn(proxy, argc, argv);
			match = true;
			break;
		}
	}

	if (!match) {
		err_printf("Invalid command '%s'", argv[1]);
		cmd_help(NULL, 0, NULL);
	}

	g_object_unref(proxy);

	return 0;
}

static inline void help(void) {
	#define CMD_HELP(CMDL, MSG) printf("  %s%s%s      \t%s.\n", COLOR_YELLOW, CMDL, COLOR_OFF, MSG);
	int i;

	printf(COLOR_RED "Usage: " COLOR_OFF);
	printf(COLOR_GREEN "groovectl " COLOR_OFF);
	puts("COMMAND [ARGS]\n");

	puts(COLOR_RED " Commands:" COLOR_OFF);

	for (i = 0; i < sizeof(cmds) / sizeof(cmds[0]); i++)
		CMD_HELP(cmds[i].name, cmds[i].desc);

	puts("");
}
