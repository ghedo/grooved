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

#include "player.h"
#include "config.h"
#include "printf.h"

struct config cfg = {
	.library = "/invalid",
	.verbose = false,
	.filters = NULL,
};

static int config_cb(void *, const char *, const char *, const char *);

void cfg_parse(const char *file) {
	if (ini_parse(file, config_cb, &cfg) < 0)
		fail_printf("Could not load config file '%s'", file);
}

static int config_cb(void *argp, const char *section,
                     const char *key, const char *val) {
	int rc;
	struct config *cfg = argp;

	if (strcmp(section, "default") == 0) {
		if (strcmp(key, "library") == 0) {
			char *path = realpath(val, NULL);

			if (!path)
				sysf_printf("Invalid library path");

			cfg -> library = path;
		} else if (strcmp(key, "verbose") == 0) {
			if (strcmp("on", val) == 0)
				cfg -> verbose = true;
			else if (strcmp("off", val) == 0)
				cfg -> verbose = false;
			else
				fail_printf("Invalid verbose value");
		} else if (strcmp(key, "filter") == 0) {
			if (cfg -> filters != NULL) {
				char *tmp = cfg -> filters;

				rc = asprintf(&cfg -> filters, "%s,%s",
				              cfg -> filters, val);
				if (rc < 0) fail_printf("OOM");

				free(tmp);
			} else {
				cfg -> filters = strdup(val);
			}
		} else
			fail_printf("Invalid config '%s'", key);
	} else
		fail_printf("Invalid section '%s'", section);

	return 1;
}
