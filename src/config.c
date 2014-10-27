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

#include <stdlib.h>
#include <string.h>

#include <inih/ini.h>

#include "config.h"
#include "player.h"
#include "printf.h"
#include "util.h"

struct config cfg = {
	.library = "/invalid",
	.verbose = false,
	.gapless = NULL,
	.filters = NULL,
	.output  = NULL,
	.cache   = NULL,
	.scripts = NULL,
};

static int config_cb(void *, const char *, const char *, const char *);

void cfg_parse(const char *file) {
	if (ini_parse(file, config_cb, &cfg) < 0)
		fail_printf("Could not load config file '%s'", file);
}

static bool cfg_decode_bool(const char *key, const char *val) {
	if (strcmp("yes", val) == 0)
		return true;
	else if (strcmp("no", val) == 0)
		return false;
	else
		fail_printf("Invalid value for option '%s'", key);

	return false; /* __builtin_unreachable(); */
}

static int config_cb(void *argp, const char *section,
                     const char *key, const char *val) {
	int rc;
	struct config *cfg = argp;

	if (strcmp(section, "default") == 0) {
		if (strcmp(key, "library") == 0) {
			char *path = realpath(val, NULL);

			if (!path)
				sysf_printf("Invalid value for option '%s'", key);

			cfg -> library = path;
		} else if (strcmp(key, "verbose") == 0) {
			cfg -> verbose = cfg_decode_bool(key, val);
		} else if (strcmp(key, "gapless") == 0) {
			cfg -> gapless = strdup(val);
		} else if (strcmp(key, "filter") == 0) {
			if (cfg -> filters != NULL) {
				_free_ char *tmp = cfg -> filters;

				rc = asprintf(&cfg -> filters, "%s,%s",
				              cfg -> filters, val);
				if (rc < 0) fail_printf("OOM");
			} else {
				cfg -> filters = strdup(val);
			}
		} else if (strcmp(key, "script") == 0) {
			if (cfg -> scripts != NULL) {
				_free_ char *tmp = cfg -> scripts;

				rc = asprintf(&cfg -> scripts, "%s,%s",
				              cfg -> scripts, val);
				if (rc < 0) fail_printf("OOM");
			} else {
				cfg -> scripts = strdup(val);
			}
		} else if (strcmp(key, "output") == 0) {
			cfg -> output = strdup(val);
		} else if (strcmp(key, "cache") == 0) {
			cfg -> cache = strdup(val);
		} else
			fail_printf("Invalid option '%s'", key);
	} else
		fail_printf("Invalid section '%s'", section);

	return 1;
}
