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

#include <string.h>

#include <sqlite3.h>

#include "player.h"
#include "config.h"
#include "printf.h"

static sqlite3 *db = NULL;

static int single_path_cb(void *argp, int argc, char **argv, char **column);

void library_open(void) {
	if (sqlite3_open(cfg.library, &db) != SQLITE_OK) {
		err_printf("Could not open database: %s", sqlite3_errmsg(db));
		sqlite3_close(db);
		db = NULL;
	}
}

void library_close(void) {
	sqlite3_close(db);
}

char *library_random(void) {
	char *err;
	char *path = NULL;

	char *q = "SELECT path FROM items ORDER BY RANDOM() LIMIT 1";

	if (db == NULL)
		return NULL;

	if (sqlite3_exec(db, q, single_path_cb, &path, &err) != SQLITE_OK) {
		err_printf("SQL error: %s", err);
		sqlite3_free(err);
		return NULL;
	}

	return path;
}

static int single_path_cb(void *argp, int argc, char **argv, char **column) {
	char **str = argp;
	*str = strdup(argv[0]);
	return 0;
}
