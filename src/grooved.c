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
#include <getopt.h>

#include <glib.h>

#include "dbus.h"
#include "library.h"
#include "player.h"
#include "config.h"
#include "printf.h"

GMainLoop *loop;

static inline void help();

static struct option long_opts[] = {
	{ "config",	required_argument,	0, 'c' },
	{ "verbose",	no_argument,		0, 'V' },
	{ "help",	no_argument,		0, 'h' },
	{ 0, 0, 0, 0 }
};

int main(int argc, char *argv[]) {
	int opts, i, rc;
	bool verbose = false;
	char *config_file = NULL;

	while ((opts = getopt_long(argc, argv, "c:Vh", long_opts, &i)) != -1) {
		switch (opts) {
			case 'c':
				config_file = strdup(optarg);
				break;

			case 'V':
				verbose = true;
				break;

			case 'h':
				help();
				exit(0);
		}
	}

	cfg.verbose = verbose;

	if (config_file == NULL) {
		rc = asprintf(&config_file, "%s/.config/grooved/config.ini",
							getenv("HOME"));
		if (rc < 0) fail_printf("Could not allocate memory");
	}

	if (!access(config_file, F_OK))
		cfg_parse(config_file);
	else
		err_printf("Config file not found, skipping");

	free(config_file);

	library_open();

	dbus_init();

	player_init();

	loop = g_main_loop_new(NULL, FALSE);

	g_main_loop_run(loop);

	g_main_destroy(loop);

	player_destroy();

	dbus_destroy();

	library_close();

	return 0;
}

static inline void help() {
	#define CMD_HELP(CMDL, CMDS, MSG) printf("  %s, %s      \t%s.\n", COLOR_YELLOW CMDS, CMDL COLOR_OFF, MSG);

	printf(COLOR_RED "Usage: " COLOR_OFF);
	printf(COLOR_GREEN "grooved " COLOR_OFF);
	puts("[OPTIONS]\n");

	puts(COLOR_RED " Options:" COLOR_OFF);

	CMD_HELP("--config",	"-c",	"Specify the configuration file");
	CMD_HELP("--verbose",	"-V",	"Enable verbose log messages");

	puts("");
}
