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

#define dbus_check_error(I, RC)						\
	if (rc < 0) {							\
		g_dbus_method_invocation_return_dbus_error(		\
			I, DBUS_ERROR_FAILED, player_error_string(RC)	\
		);							\
		return TRUE;						\
	}

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

static void on_track_changed(void) {
	GVariantBuilder *meta = g_variant_builder_new(G_VARIANT_TYPE("a{ss}"));
	player_make_metadata(meta);
	grooved_player_set_track_metadata(iface, g_variant_builder_end(meta));

	char *path = player_playback_track_path();
	grooved_player_set_track_path(iface, path);
	free(path);

	double length = player_playback_track_length();
	grooved_player_set_track_length(iface, length);
}

static void on_status_changed(void) {
	char *status = player_playback_status_string();
	grooved_player_set_playback_status(iface, status);
}

void dbus_handle_event(enum dbus_event sig) {
	switch (sig) {
		case STATUS_CHANGED:
			on_status_changed();
			break;

		case TRACK_CHANGED:
			on_track_changed();
			break;
	}
}

gboolean on_add_list(GroovedPlayer *obj, GDBusMethodInvocation *invocation,
                     const char *arg_path) {
	int rc = player_playlist_append_list(arg_path);
	dbus_check_error(invocation, rc);

	g_dbus_method_invocation_return_value(invocation, NULL);

	return TRUE;
}

gboolean on_add_track(GroovedPlayer *obj, GDBusMethodInvocation *invocation,
                      const char *arg_path) {
	int rc = player_playlist_append_file(arg_path);
	dbus_check_error(invocation, rc);

	g_dbus_method_invocation_return_value(invocation, NULL);

	return TRUE;
}

gboolean on_goto_track(GroovedPlayer *obj, GDBusMethodInvocation *invocation,
                       guint64 arg_index) {
	int rc = player_playlist_goto_index(arg_index);
	dbus_check_error(invocation, rc);

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
	int rc;

	if (g_strcmp0(arg_mode, "track") == 0) {
		rc = player_playback_loop(PLAYER_LOOP_TRACK);
	} else if (g_strcmp0(arg_mode, "list") == 0) {
		rc = player_playback_loop(PLAYER_LOOP_LIST);
	} else if (g_strcmp0(arg_mode, "none") == 0) {
		rc = player_playback_loop(PLAYER_LOOP_NONE);
	} else {
		g_dbus_method_invocation_return_dbus_error(
			invocation, DBUS_ERROR_INVALID_ARGS,
			"Invalid loop mode"
		);

		goto exit;
	}
	dbus_check_error(invocation, rc);

	g_dbus_method_invocation_return_value(invocation, NULL);

	char *loop = player_loop_status_string();
	grooved_player_set_loop_status(iface, loop);

exit:
	return TRUE;
}

gboolean on_next(GroovedPlayer *obj, GDBusMethodInvocation *invocation) {
	int rc = player_playlist_next();
	dbus_check_error(invocation, rc);

	g_dbus_method_invocation_return_value(invocation, NULL);

	return TRUE;
}

gboolean on_pause(GroovedPlayer *obj, GDBusMethodInvocation *invocation) {
	int rc = player_playback_pause();
	dbus_check_error(invocation, rc);

	g_dbus_method_invocation_return_value(invocation, NULL);

	return TRUE;
}

gboolean on_play(GroovedPlayer *obj, GDBusMethodInvocation *invocation) {
	int rc = player_playback_play();
	dbus_check_error(invocation, rc);

	g_dbus_method_invocation_return_value(invocation, NULL);

	return TRUE;
}

gboolean on_prev(GroovedPlayer *obj, GDBusMethodInvocation *invocation) {
	int rc = player_playlist_prev();
	dbus_check_error(invocation, rc);

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
	int rc = player_playlist_remove_index(arg_index);
	dbus_check_error(invocation, rc);

	g_dbus_method_invocation_return_value(invocation, NULL);

	return TRUE;
}

gboolean on_seek(GroovedPlayer *obj, GDBusMethodInvocation *invocation,
                 int64_t arg_seconds) {
	int rc = player_playback_seek(arg_seconds);
	dbus_check_error(invocation, rc);

	g_dbus_method_invocation_return_value(invocation, NULL);

	return TRUE;
}

gboolean on_stop(GroovedPlayer *obj, GDBusMethodInvocation *invocation) {
	int rc = player_playback_stop();
	dbus_check_error(invocation, rc);

	g_dbus_method_invocation_return_value(invocation, NULL);

	return TRUE;
}

gboolean on_track_position(GroovedPlayer *obj, GDBusMethodInvocation *invocation) {
	double time    = player_playback_track_position_time();
	double percent = player_playback_track_position_percent();

	grooved_player_complete_track_position(obj, invocation, time, percent);

	return TRUE;
}

gboolean on_toggle(GroovedPlayer *obj, GDBusMethodInvocation *invocation) {
	int rc = player_playback_toggle();
	dbus_check_error(invocation, rc);

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
		{ "handle-add-list",        G_CALLBACK(on_add_list) },
		{ "handle-add-track",       G_CALLBACK(on_add_track) },
		{ "handle-goto-track",      G_CALLBACK(on_goto_track) },
		{ "handle-list",            G_CALLBACK(on_list) },
		{ "handle-set-loop-status", G_CALLBACK(on_loop) },
		{ "handle-next",            G_CALLBACK(on_next) },
		{ "handle-pause",           G_CALLBACK(on_pause) },
		{ "handle-play",            G_CALLBACK(on_play) },
		{ "handle-prev",            G_CALLBACK(on_prev) },
		{ "handle-quit",            G_CALLBACK(on_quit) },
		{ "handle-remove-track",    G_CALLBACK(on_remove_track) },
		{ "handle-seek",            G_CALLBACK(on_seek) },
		{ "handle-stop",            G_CALLBACK(on_stop) },
		{ "handle-track-position",  G_CALLBACK(on_track_position) },
		{ "handle-toggle",          G_CALLBACK(on_toggle) },
	};

	for (i = 0; i < sizeof(cbs) / sizeof(cbs[0]); i++)
		g_signal_connect(iface, cbs[i].name, cbs[i].callback, NULL);

	g_dbus_interface_skeleton_export(
		G_DBUS_INTERFACE_SKELETON(iface),
		conn, GROOVED_DBUS_PLAYER_PATH, &err
	);

	if (err != NULL)
		fail_printf("%s", err -> message);

	char *status = player_playback_status_string();
	grooved_player_set_playback_status(iface, status);

	char *loop = player_loop_status_string();
	grooved_player_set_loop_status(iface, loop);
}

static void on_name_acquired(GDBusConnection *conn, const char *name, void *p) {
}

static void on_name_lost(GDBusConnection *conn, const char *name, void *ptr) {
	fail_printf("Lost DBus name");
}
