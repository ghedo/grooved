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

#include "dbus.h"
#include "grooved.h"
#include "player.h"
#include "printf.h"

static unsigned int owner_id;
static GDBusConnection *dbus_conn;


void on_method_call(GDBusConnection *conn, const char *sender, const char *path,
		    const char *ifname, const char *method, GVariant *args,
		    GDBusMethodInvocation *invocation, void *p) {

	if (g_strcmp0(method, "Status") == 0) {
		GVariantBuilder *status = g_variant_builder_new(
			G_VARIANT_TYPE("(ssddda{ss}sb)")
		);

		player_status(status);

		g_dbus_method_invocation_return_value(
			invocation, g_variant_builder_end(status)
		);
	} else if (g_strcmp0(method, "Play") == 0) {
		player_playback_play();

		g_dbus_method_invocation_return_value(invocation, NULL);
	} else if (g_strcmp0(method, "Pause") == 0) {
		player_playback_pause();

		g_dbus_method_invocation_return_value(invocation, NULL);
	} else if (g_strcmp0(method, "Toggle") == 0) {
		player_playback_toggle();

		g_dbus_method_invocation_return_value(invocation, NULL);
	} else if (g_strcmp0(method, "Next") == 0) {
		player_playlist_next();

		g_dbus_method_invocation_return_value(invocation, NULL);
	} else if (g_strcmp0(method, "Prev") == 0) {
		player_playlist_prev();

		g_dbus_method_invocation_return_value(invocation, NULL);
	} else if (g_strcmp0(method, "Seek") == 0) {
		int64_t secs;

		g_variant_get(args, "(x)", &secs);
		player_playback_seek(secs);

		g_dbus_method_invocation_return_value(invocation, NULL);
	} else if (g_strcmp0(method, "AddTrack") == 0) {
		const char *path;

		g_variant_get(args, "(s)", &path);
		player_playlist_append_file(path);

		g_dbus_method_invocation_return_value(invocation, NULL);
	} else if (g_strcmp0(method, "AddList") == 0) {
		const char *path;

		g_variant_get(args, "(s)", &path);
		player_playlist_append_list(path);

		g_dbus_method_invocation_return_value(invocation, NULL);
	} else if (g_strcmp0(method, "Replaygain") == 0) {
		const char *mode;

		g_variant_get(args, "(s)", &mode);

		if (g_strcmp0(mode, "track") == 0) {
			player_playback_replaygain(PLAYER_REPLAYGAIN_TRACK);
			g_dbus_method_invocation_return_value(invocation, NULL);
		} else if (g_strcmp0(mode, "album") == 0) {
			player_playback_replaygain(PLAYER_REPLAYGAIN_ALBUM);
			g_dbus_method_invocation_return_value(invocation, NULL);
		} else if (g_strcmp0(mode, "none") == 0) {
			player_playback_replaygain(PLAYER_REPLAYGAIN_NONE);
			g_dbus_method_invocation_return_value(invocation, NULL);
		} else {
			g_dbus_method_invocation_return_dbus_error(
				invocation, DBUS_ERROR_INVALID_ARGS,
				"Invalid Replaygain mode"
			);
		}
	} else if (g_strcmp0(method, "Loop") == 0) {
		bool enable;

		g_variant_get(args, "(b)", &enable);
		player_playback_loop(enable);

		g_dbus_method_invocation_return_value(invocation, NULL);
	} else if (g_strcmp0(method, "Quit") == 0) {
		g_main_loop_quit(loop);

		g_dbus_method_invocation_return_value(invocation, NULL);
	} else {
		g_dbus_method_invocation_return_dbus_error(
			invocation, DBUS_ERROR_UNKNOWN_METHOD, ""
		);
	}

	g_dbus_connection_flush(conn, NULL, NULL, NULL);
}

static const GDBusInterfaceVTable player_vtable = {
	.method_call = on_method_call
};

static void on_bus_acquired(GDBusConnection *conn, const char *name, void *p) {
	unsigned int registration_id;

	GDBusNodeInfo *intro = g_dbus_node_info_new_for_xml(intro_xml, NULL);

	registration_id = g_dbus_connection_register_object(
		conn, GROOVED_DBUS_PLAYER_PATH, intro -> interfaces[0],
		&player_vtable, NULL, NULL, NULL
	);

	if (registration_id <= 0)
		fail_printf("Could not register DBus object");
}

static void on_name_acquired(GDBusConnection *conn, const char *name, void *p) {
	dbus_conn = conn;
}

static void on_name_lost(GDBusConnection *conn, const char *name, void *ptr) {
	fail_printf("Lost DBus name");
}

void dbus_init(void) {
	owner_id = g_bus_own_name(
		G_BUS_TYPE_SESSION, GROOVED_DBUS_NAME, G_BUS_NAME_OWNER_FLAGS_NONE,
		on_bus_acquired, on_name_acquired, on_name_lost, NULL, NULL
	);
}

void dbus_destroy(void) {
	g_bus_unown_name(owner_id);
}
