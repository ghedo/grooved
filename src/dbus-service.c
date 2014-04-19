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

#include <glib.h>
#include <gio/gio.h>
#include <dbus/dbus.h>

#include "dbus-common.h"
#include "dbus-service.h"
#include "grooved.h"
#include "player.h"
#include "printf.h"

GroovedPlayer *iface = NULL;

static unsigned int owner_id;

static void on_bus_acquired(GDBusConnection *conn, const char *name, void *p);
static void on_name_acquired(GDBusConnection *conn, const char *name, void *p);
static void on_name_lost(GDBusConnection *conn, const char *name, void *ptr);

void dbus_init(void) {
	owner_id = g_bus_own_name(
		G_BUS_TYPE_SESSION, GROOVED_DBUS_NAME, G_BUS_NAME_OWNER_FLAGS_NONE,
		on_bus_acquired, on_name_acquired, on_name_lost, NULL, NULL
	);
}

void dbus_destroy(void) {
	g_bus_unown_name(owner_id);
}

void dbus_emit_signal(enum dbus_signal sig) {
	switch (sig) {
		case STATUS_CHANGED:
			grooved_player_emit_status_changed(iface);
			break;

		case TRACK_CHANGED:
			grooved_player_emit_track_changed(iface);
			break;

		case TRACK_ADDED:
			grooved_player_emit_track_added(iface);
			break;

		case OPTION_CHANGED:
			grooved_player_emit_option_changed(iface);
			break;
	}
}

gboolean on_add_list(GroovedPlayer *obj, GDBusMethodInvocation *invocation,
                     const char *arg_path) {
	player_playlist_append_list(arg_path);

	g_dbus_method_invocation_return_value(invocation, NULL);

	return TRUE;
}

gboolean on_add_track(GroovedPlayer *obj, GDBusMethodInvocation *invocation,
                      const char *arg_path) {
	player_playlist_append_file(arg_path);

	g_dbus_method_invocation_return_value(invocation, NULL);

	return TRUE;
}

gboolean on_list(GroovedPlayer *obj, GDBusMethodInvocation *invocation) {
	GVariantBuilder *list = g_variant_builder_new(G_VARIANT_TYPE("(asxx)"));

	player_make_list(list);

	g_dbus_method_invocation_return_value(
		invocation, g_variant_builder_end(list)
	);

	return TRUE;
}

gboolean on_loop(GroovedPlayer *obj, GDBusMethodInvocation *invocation,
                 const char *arg_mode) {
	if (g_strcmp0(arg_mode, "track") == 0) {
		player_playback_loop(PLAYER_LOOP_TRACK);
	} else if (g_strcmp0(arg_mode, "list") == 0) {
		player_playback_loop(PLAYER_LOOP_LIST);
	} else if (g_strcmp0(arg_mode, "none") == 0) {
		player_playback_loop(PLAYER_LOOP_NONE);
	} else {
		g_dbus_method_invocation_return_dbus_error(
			invocation, DBUS_ERROR_INVALID_ARGS,
			"Invalid loop mode"
		);

		goto exit;
	}

	g_dbus_method_invocation_return_value(invocation, NULL);

exit:
	return TRUE;
}

gboolean on_next(GroovedPlayer *obj, GDBusMethodInvocation *invocation) {
	player_playlist_next();

	g_dbus_method_invocation_return_value(invocation, NULL);

	return TRUE;
}

gboolean on_pause(GroovedPlayer *obj, GDBusMethodInvocation *invocation) {
	player_playback_pause();

	g_dbus_method_invocation_return_value(invocation, NULL);

	return TRUE;
}

gboolean on_play(GroovedPlayer *obj, GDBusMethodInvocation *invocation) {
	player_playback_play();

	g_dbus_method_invocation_return_value(invocation, NULL);

	return TRUE;
}

gboolean on_prev(GroovedPlayer *obj, GDBusMethodInvocation *invocation) {
	player_playlist_prev();

	g_dbus_method_invocation_return_value(invocation, NULL);

	return TRUE;
}

gboolean on_quit(GroovedPlayer *obj, GDBusMethodInvocation *invocation) {
	g_main_loop_quit(loop);

	g_dbus_method_invocation_return_value(invocation, NULL);

	return TRUE;
}

gboolean on_remove_track(GroovedPlayer *obj, GDBusMethodInvocation *invocation,
                         gint64 arg_index) {
	player_playlist_remove_index(arg_index);

	g_dbus_method_invocation_return_value(invocation, NULL);

	return TRUE;
}

gboolean on_seek(GroovedPlayer *obj, GDBusMethodInvocation *invocation,
                 int64_t arg_seconds) {
	player_playback_seek(arg_seconds);

	g_dbus_method_invocation_return_value(invocation, NULL);

	return TRUE;
}

gboolean on_status(GroovedPlayer *obj, GDBusMethodInvocation *invocation) {
	GVariantBuilder *status = g_variant_builder_new(
		G_VARIANT_TYPE("(ssddda{ss}s)")
	);

	player_make_status(status);

	g_dbus_method_invocation_return_value(
		invocation, g_variant_builder_end(status)
	);

	return TRUE;
}

gboolean on_stop(GroovedPlayer *obj, GDBusMethodInvocation *invocation) {
	player_playback_stop();

	g_dbus_method_invocation_return_value(invocation, NULL);

	return TRUE;
}

gboolean on_toggle(GroovedPlayer *obj, GDBusMethodInvocation *invocation) {
	player_playback_toggle();

	g_dbus_method_invocation_return_value(invocation, NULL);

	return TRUE;
}

struct handle_signal {
	const char *name;
	GCallback callback;
};

static void on_bus_acquired(GDBusConnection *conn, const char *name, void *p) {
	int i;
	GError *err = NULL;

	iface = grooved_player_skeleton_new();

	struct handle_signal cbs[] = {
		{ "handle-add-list",   G_CALLBACK(on_add_list) },
		{ "handle-add-track",  G_CALLBACK(on_add_track) },
		{ "handle-list",       G_CALLBACK(on_list) },
		{ "handle-loop",       G_CALLBACK(on_loop) },
		{ "handle-next",       G_CALLBACK(on_next) },
		{ "handle-pause",      G_CALLBACK(on_pause) },
		{ "handle-play",       G_CALLBACK(on_play) },
		{ "handle-prev",       G_CALLBACK(on_prev) },
		{ "handle-quit",       G_CALLBACK(on_quit) },
		{ "handle-remove-track",G_CALLBACK(on_remove_track) },
		{ "handle-seek",       G_CALLBACK(on_seek) },
		{ "handle-status",     G_CALLBACK(on_status) },
		{ "handle-stop",       G_CALLBACK(on_stop) },
		{ "handle-toggle",     G_CALLBACK(on_toggle) },
	};

	for (i = 0; i < sizeof(cbs) / sizeof(cbs[0]); i++)
		g_signal_connect(iface, cbs[i].name, cbs[i].callback, NULL);

	g_dbus_interface_skeleton_export(
		G_DBUS_INTERFACE_SKELETON(iface),
		conn, GROOVED_DBUS_PLAYER_PATH, &err
	);

	if (err != NULL)
		fail_printf("%s", err -> message);
}

static void on_name_acquired(GDBusConnection *conn, const char *name, void *p) {
}

static void on_name_lost(GDBusConnection *conn, const char *name, void *ptr) {
	fail_printf("Lost DBus name");
}
