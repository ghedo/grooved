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

#include <inttypes.h>

#include <glib.h>
#include <gio/gio.h>
#include <glyr/glyr.h>

#include "dbus-grooved.h"
#include "dbus-service.h"
#include "printf.h"
#include "util.h"

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

	double pos, percent;

	const char *state = grooved_player_get_playback_status(proxy);
	const char *loop  = grooved_player_get_loop_status(proxy);
	const char *title = grooved_player_get_track_title(proxy);

	double len  = grooved_player_get_track_length(proxy);
	GVariant *metadata = grooved_player_get_track_metadata(proxy);

	grooved_player_call_track_position_sync(proxy, &pos, &percent, NULL, &err);
	if (err != NULL)
		fail_printf("%s", err -> message);

	GVariantIter iter;

	char *key, *val;

	printf("Title: %s\n", title);

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

	printf("loop: %s\n", loop);
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

CMD_HANDLE(ls) {
	GError *err = NULL;

	char **files;
	int64_t i, count, pos;

	grooved_player_call_list_sync(proxy, &files, &count, &pos, NULL, &err);

	if (err != NULL)
		fail_printf("%s", err -> message);

	for (i = 0; i < count; i++)
		printf("%c %3" PRId64 ":%s\n", (pos == i) ? '*' : ' ', i, files[i]);
}

CMD_HANDLE(save) {
	GError *err = NULL;

	char **files;
	int64_t i, count, pos;

	FILE *list = NULL;

	if ((argc < 3))
		fail_printf("Invalid output file");

	list = fopen(argv[2], "w");
	if (list == NULL)
		sysf_printf("Invalid output file");

	grooved_player_call_list_sync(proxy, &files, &count, &pos, NULL, &err);

	if (err != NULL)
		fail_printf("%s", err -> message);

	fprintf(list, "[playlist]\nNumberOfEntries=%" PRId64 "\n", count);

	for (i = 0; i < count; i++)
		fprintf(list, "File%" PRId64 "=%s\n", i, files[i]);

	fclose(list);
}

CMD_HANDLE(load) {
	int i = 2;

	GError *err = NULL;

	bool dont_stop = false;
	_free_ char *path = NULL;

	if ((argc < 3))
		fail_printf("Invalid playlist file");

	if (strcmp(argv[i], "--append") == 0) {
		dont_stop = true;
		i++;
	}

	path = realpath(argv[i], NULL);
	if (path == NULL)
		sysf_printf("Invalid playlist file");

	if (!dont_stop) {
		grooved_player_call_stop_sync(proxy, NULL, &err);

		if (err != NULL)
			fail_printf("%s", err -> message);
	}

	grooved_player_call_add_list_sync(proxy, path, NULL, &err);

	if (err != NULL)
		fail_printf("%s", err -> message);
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

static void add_track(GroovedPlayer *proxy, char *input) {
	GError *err = NULL;

	_free_ char *entry = realpath(input, NULL);
	if (entry == NULL)
		entry = strdup(input);

	if (entry == NULL)
		fail_printf("OOM");

	grooved_player_call_add_track_sync(proxy, entry, NULL, &err);

	if (err != NULL)
		fail_printf("%s", err -> message);

	printf("Added track '%s'\n", entry);
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

CMD_HANDLE(goto) {
	uint64_t index;
	GError *err = NULL;

	if ((argc < 3) || (sscanf(argv[2], "%" PRIu64, &index) <= 0))
		fail_printf("Invalid track index");

	grooved_player_call_goto_track_sync(proxy, index, NULL, &err);

	if (err != NULL)
		fail_printf("%s", err -> message);
}

CMD_HANDLE(rm) {
	int64_t index;
	GError *err = NULL;

	if ((argc < 3) || (sscanf(argv[2], "%" PRId64, &index) <= 0))
		fail_printf("Invalid track index");

	grooved_player_call_remove_track_sync(proxy, index, NULL, &err);

	if (err != NULL)
		fail_printf("%s", err -> message);
}

CMD_HANDLE(loop) {
	if ((argc < 3) ||
	    (strcmp("track", argv[2]) &&
	     strcmp("list", argv[2]) &&
	     strcmp("none", argv[2])))
		fail_printf("Invalid loop mode");

	grooved_player_set_loop_status(proxy, argv[2]);
}

CMD_HANDLE(lyrics) {
	GVariantIter iter;

	GlyrQuery q;
	GLYR_ERROR glyr_err;
	GlyrMemCache *cache;

	char *key, *val;
	GVariant *metadata;

	char *title = NULL, *artist = NULL, *album = NULL;

	glyr_init();

	glyr_query_init(&q);

	glyr_opt_type(&q, GLYR_GET_LYRICS);

	metadata = grooved_player_get_track_metadata(proxy);

	g_variant_iter_init(&iter, metadata);
	while (g_variant_iter_loop(&iter, "{ss}", &key, &val)) {
		if (!title && strcasecmp("title", key) == 0)
			title = strdup(val);
		else if (!artist && strcasecmp("artist", key) == 0)
			artist = strdup(val);
		else if (!album && strcasecmp("album", key) == 0)
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
	{ "goto",   cmd_goto,   "Skip to a specific track in the tracklist" },
	{ "last",   cmd_last,   "Stop playback after currently playing track" },
	{ "load",   cmd_load,   "Load a playlist file" },
	{ "loop",   cmd_loop,   "Set the player's loop mode" },
	{ "ls",     cmd_ls,     "Show tracklist" },
	{ "lyrics", cmd_lyrics, "Download and show lyrics for the currently playing track" },
	{ "next",   cmd_next,   "Skip to next track" },
	{ "pause",  cmd_pause,  "Pause the player" },
	{ "play",   cmd_play,   "Unpause the player" },
	{ "prev",   cmd_prev,   "Skip to previous track" },
	{ "quit",   cmd_quit,   "Shutdown the player" },
	{ "rm",     cmd_rm,     "Remove a track from the tracklist" },
	{ "save",   cmd_save,   "Save the tracklist to a playlist file" },
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
		GROOVED_DBUS_NAME, GROOVED_DBUS_PATH, NULL, &err
	);

	if (err != NULL)
		fail_printf("%s", err -> message);

	for (i = 0; argv[1] && i < sizeof(cmds) / sizeof(cmds[0]); i++) {
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
