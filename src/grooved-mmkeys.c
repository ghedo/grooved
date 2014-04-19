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
#include <stdbool.h>

#define XLIB_ILLEGAL_ACCESS
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/XKBlib.h>
#include <X11/XF86keysym.h>

#include <glib.h>

#include "dbus-common.h"
#include "dbus-service.h"
#include "printf.h"

static Display *dpy;
static GroovedPlayer *proxy;

static void x11_init(void);
static void x11_destroy(void);

static gboolean x11_loop_fd_prepare(GSource *source, int *timeout);
static gboolean x11_loop_fd_check(GSource * source);
static gboolean x11_loop_fd_dispatch(GSource *source, GSourceFunc cb, void *ptr);

static void x11_grab_key(Display *dpy, const KeySym keysym);
static void x11_handle_key(Display *dpy, XEvent ev);

static void x11_setup_error_handler(void);
static void x11_teardown_error_handler(void);

int main(int argc, char *argv[]) {
	GMainLoop *loop;
	GError *err = NULL;

	proxy = grooved_player_proxy_new_for_bus_sync(
		G_BUS_TYPE_SESSION, G_DBUS_PROXY_FLAGS_NONE,
		GROOVED_DBUS_NAME, GROOVED_DBUS_PLAYER_PATH, NULL, &err
	);

	x11_init();

	loop = g_main_loop_new(NULL, FALSE);

	g_main_loop_run(loop);

	g_main_destroy(loop);

	x11_destroy();
}

static void x11_init(void) {
	KeySym *map, *iter;
	int i, key_min, key_max, key_num;

	GSource *x11_source;

	dpy = XOpenDisplay(0);

	GPollFD *dpy_pollfd = malloc(sizeof(GPollFD));

	GSourceFuncs *x11_source_funcs = calloc(1, sizeof(GSourceFuncs));

	dpy_pollfd -> fd = dpy -> fd;
	dpy_pollfd -> events = G_IO_IN | G_IO_HUP | G_IO_ERR;
	dpy_pollfd -> revents = G_IO_IN | G_IO_HUP | G_IO_ERR;

	x11_source_funcs -> prepare  = x11_loop_fd_prepare;
	x11_source_funcs -> check    = x11_loop_fd_check;
	x11_source_funcs -> dispatch = x11_loop_fd_dispatch;

	XDisplayKeycodes(dpy, &key_min, &key_max);
	map = XGetKeyboardMapping(dpy, key_min, (key_max - key_min + 1), &key_num);

	iter = map;

	x11_setup_error_handler();

	for (i = key_min; i <= key_max; i++) {
		int j;

		for (j = 0; j <= (key_num - 1); j++, iter++) {
			switch (iter[j]) {
				case XF86XK_AudioPlay:
					x11_grab_key(dpy, XF86XK_AudioPlay);
					break;
				case XF86XK_AudioStop:
					x11_grab_key(dpy, XF86XK_AudioStop);
					break;
				case XF86XK_AudioPrev:
					x11_grab_key(dpy, XF86XK_AudioPrev);
					break;
				case XF86XK_AudioNext:
					x11_grab_key(dpy, XF86XK_AudioNext);
					break;
				case XF86XK_AudioRepeat:
					x11_grab_key(dpy, XF86XK_AudioRepeat);
					break;
				case XF86XK_AudioRandomPlay:
					x11_grab_key(dpy, XF86XK_AudioRandomPlay);
					break;
			}
		}
	}

	x11_teardown_error_handler();

	XFree(map);

	XSelectInput(dpy, DefaultRootWindow(dpy), KeyPressMask);

	x11_source = g_source_new(x11_source_funcs, sizeof(GSource));
	g_source_add_poll(x11_source, dpy_pollfd);

	g_source_attach(x11_source, NULL);
}

static void x11_destroy(void) {
	XCloseDisplay(dpy);
}

static gboolean x11_loop_fd_prepare(GSource *source, int *timeout) {
	*timeout = -1;

	return FALSE;
}

static gboolean x11_loop_fd_check(GSource *source) {
	return XPending(dpy) > 0;
}

static gboolean x11_loop_fd_dispatch(GSource *source, GSourceFunc cb, void *ptr) {
	while (XPending(dpy) > 0) {
		XEvent ev;
		XNextEvent(dpy, &ev);

		switch (ev.type) {
			case KeyPress:
				x11_handle_key(dpy, ev);
				break;
		}
	}

	return TRUE;
}

static void x11_grab_key(Display *dpy, const KeySym keysym) {
	Window root = DefaultRootWindow(dpy);
	KeyCode keycode = XKeysymToKeycode(dpy, keysym);

	XGrabKey(dpy, keycode, AnyModifier, root, True, GrabModeAsync, GrabModeAsync);
}

static void x11_handle_key(Display *dpy, XEvent ev) {
	GError *err = NULL;

	switch (XkbKeycodeToKeysym(dpy, ev.xkey.keycode, 0, 0)) {
		case XF86XK_AudioPlay:
			grooved_player_call_toggle_sync(proxy, NULL, &err);
			break;
		case XF86XK_AudioNext:
			grooved_player_call_next_sync(proxy, NULL, &err);
			break;
		case XF86XK_AudioPrev:
			grooved_player_call_prev_sync(proxy, NULL, &err);
			break;
		case XF86XK_AudioStop:
			grooved_player_call_stop_sync(proxy, NULL, &err);
			break;
	}

	if (err != NULL)
		err_printf("%s", err -> message);
}

static int GrabXErrorHandler(Display *dpy, XErrorEvent *ev) {
	char err_buf[4096];
	XGetErrorText(dpy, ev -> error_code, err_buf, 4096);

	if (ev -> error_code != BadAccess)
		fail_printf("X error: %s", err_buf);

	return 0;
}

static void x11_setup_error_handler(void) {
	XFlush(dpy);
	XSetErrorHandler(GrabXErrorHandler);
}

static void x11_teardown_error_handler(void) {
	XFlush(dpy);
	XSync(dpy, false);
	XSetErrorHandler(NULL);
}
