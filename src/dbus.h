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

#define GROOVED_DBUS_NAME             "io.github.ghedo.grooved"
#define GROOVED_DBUS_PLAYER_INTERFACE "io.github.ghedo.grooved.Player"
#define GROOVED_DBUS_PLAYER_PATH      "/io/github/ghedo/grooved/Player"

#define GROOVED_DBUS_INTROSPECTION					\
"<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"				\
"<node name=\"" GROOVED_DBUS_PLAYER_PATH "\">\n"			\
"  <interface name=\"" GROOVED_DBUS_PLAYER_INTERFACE "\">\n"		\
									\
"    <method name=\"Status\">"						\
"      <arg direction=\"out\" name=\"state\" type=\"s\"/>\n"		\
"      <arg direction=\"out\" name=\"path\" type=\"s\"/>\n"		\
									\
"      <arg direction=\"out\" name=\"length\" type=\"d\"/>\n"		\
"      <arg direction=\"out\" name=\"position\" type=\"d\"/>\n"		\
"      <arg direction=\"out\" name=\"percent\" type=\"d\"/>\n"		\
									\
"      <arg direction=\"out\" name=\"metadata\" type=\"a{ss}\"/>\n"	\
									\
"      <arg direction=\"out\" name=\"replaygain\" type=\"s\"/>\n"	\
"      <arg direction=\"out\" name=\"loop\" type=\"b\"/>\n"		\
"    </method>\n"							\
									\
"    <method name=\"Play\">\n"						\
"    </method>\n"							\
									\
"    <method name=\"Pause\">\n"						\
"    </method>\n"							\
									\
"    <method name=\"Toggle\">\n"					\
"    </method>\n"							\
									\
"    <method name=\"Next\">\n"						\
"    </method>\n"							\
									\
"    <method name=\"Prev\">\n"						\
"    </method>\n"							\
									\
"    <method name=\"Stop\">\n"						\
"    </method>\n"							\
									\
"    <method name=\"Seek\">\n"						\
"      <arg direction=\"in\" name=\"seconds\" type=\"x\"/>\n"		\
"    </method>\n"							\
									\
"    <method name=\"List\">\n"						\
"      <arg direction=\"out\" name=\"files\" type=\"as\"/>\n"		\
"      <arg direction=\"out\" name=\"count\" type=\"x\"/>\n"		\
"      <arg direction=\"out\" name=\"position\" type=\"x\"/>\n"		\
"    </method>\n"							\
									\
"    <method name=\"AddTrack\">\n"					\
"      <arg direction=\"in\" name=\"path\" type=\"s\"/>\n"		\
"    </method>\n"							\
									\
"    <method name=\"AddList\">\n"					\
"      <arg direction=\"in\" name=\"path\" type=\"s\"/>\n"		\
"    </method>\n"							\
									\
"    <method name=\"RemoveTrack\">\n"					\
"      <arg direction=\"in\" name=\"index\" type=\"x\"/>\n"		\
"    </method>\n"							\
									\
"    <method name=\"Replaygain\">\n"					\
"      <arg direction=\"in\" name=\"mode\" type=\"s\"/>\n"		\
"    </method>\n"							\
									\
"    <method name=\"Loop\">\n"						\
"      <arg direction=\"in\" name=\"enable\" type=\"b\"/>\n"		\
"    </method>\n"							\
									\
"    <method name=\"Quit\">\n"						\
"    </method>\n"							\
									\
"    <signal name=\"StatusChanged\">\n"					\
"    </signal>\n"							\
									\
"    <signal name=\"TrackChanged\">\n"					\
"    </signal>\n"							\
									\
"    <signal name=\"TrackAdded\">\n"					\
"    </signal>\n"							\
									\
"    <signal name=\"OptionChanged\">\n"					\
"    </signal>\n"							\
									\
"  </interface>\n"							\
"</node>"

enum dbus_signal {
	STATUS_CHANGED,
	TRACK_CHANGED,
	TRACK_ADDED,
	OPTION_CHANGED,
};

extern void dbus_init(void);
extern void dbus_destroy(void);

extern void dbus_emit_signal(enum dbus_signal sig);
