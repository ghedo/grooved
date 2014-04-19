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

#include <libnotify/notify.h>

#include "dbus.h"
#include "dbus-common.h"
#include "printf.h"
#include "util.h"

static GroovedPlayer *proxy;

static void notify_on_track_changed(GroovedPlayer *obj, void *data);
static void notify(NotifyNotification *n, GVariant *metadata);

int main(int argc, char *argv[]) {
	GError *err = NULL;

	NotifyNotification *n = NULL;
	GMainLoop *loop = g_main_loop_new(NULL, FALSE);

	proxy = grooved_player_proxy_new_for_bus_sync(
		G_BUS_TYPE_SESSION, G_DBUS_PROXY_FLAGS_NONE,
		GROOVED_DBUS_NAME, GROOVED_DBUS_PLAYER_PATH, NULL, &err
	);

	notify_init("grooved");

	n = notify_notification_new("", "", "");

	g_signal_connect(proxy, "track_changed",
	                 G_CALLBACK(notify_on_track_changed), n);

	g_main_loop_run(loop);

	g_main_destroy(loop);

	return 0;
}

static void notify_on_track_changed(GroovedPlayer *obj, void *data) {
	GError *err = NULL;
	GVariant *metadata = NULL;
	NotifyNotification *n = data;

	grooved_player_call_status_sync(
		proxy, NULL, NULL, NULL, NULL, NULL,
		&metadata, NULL, NULL, &err
	);

	notify(n, metadata);
}

static void notify(NotifyNotification *n, GVariant *metadata) {
	GVariantIter iter;

	char *key, *val;
	_free_ char *body = NULL, *title = NULL, *artist = NULL;

	g_variant_iter_init(&iter, metadata);
	while (g_variant_iter_loop(&iter, "{ss}", &key, &val)) {
		if (strcasecmp(key, "icy-title") == 0)
			title = strdup(val);

		if (strcasecmp(key, "title") == 0)
			title = strdup(val);

		if (strcasecmp(key, "artist") == 0)
			artist = strdup(val);
	}

	if (title)
		body = strdup(title);

	if (artist) {
		char *tmp = body;

		asprintf(&body, "%s - %s", body, artist);

		if (tmp)
			free(tmp);
	}

	notify_notification_close(n, NULL);

	notify_notification_update(n, body, NULL, "media-playback-start");
	notify_notification_show(n, NULL);

	g_variant_unref(metadata);
}
