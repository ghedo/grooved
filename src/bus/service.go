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

package bus

import "fmt"

import "github.com/godbus/dbus"
import "github.com/godbus/dbus/introspect"
import "github.com/godbus/dbus/prop"

import "player"

const bus_introspection = `
<?xml version="1.0" encoding="UTF-8"?>
<node>
  <interface name="io.github.ghedo.grooved.Player">
    <property name="PlaybackStatus" type="s" access="read">
    </property>

    <property name="LoopStatus" type="s" access="readwrite">
    </property>

    <property name="TrackMetadata" type="a{ss}" access="read">
    </property>

    <property name="TrackPath" type="s" access="read">
    </property>

    <property name="TrackLength" type="d" access="read">
    </property>

    <method name="TrackPosition">
      <arg direction="out" name="position" type="d"/>
      <arg direction="out" name="percent" type="d"/>
    </method>

    <property name="TrackTitle" type="s" access="read">
    </property>

    <method name="Play">
    </method>

    <method name="Pause">
    </method>

    <method name="Toggle">
    </method>

    <method name="Next">
    </method>

    <method name="Prev">
    </method>

    <method name="Stop">
    </method>

    <method name="Seek">
      <arg direction="in" name="seconds" type="x"/>
    </method>

    <method name="List">
      <arg direction="out" name="files" type="as"/>
      <arg direction="out" name="count" type="x"/>
      <arg direction="out" name="position" type="x"/>
    </method>

    <method name="AddTrack">
      <arg direction="in" name="path" type="s"/>
    </method>

    <method name="AddList">
      <arg direction="in" name="path" type="s"/>
    </method>

    <method name="GotoTrack">
      <arg direction="in" name="index" type="t"/>
    </method>

    <method name="RemoveTrack">
      <arg direction="in" name="index" type="x"/>
    </method>

    <method name="Quit">
    </method>
  </interface>

  <interface name="org.freedesktop.DBus.Introspectable">
    <method name="Introspect">
      <arg name="out" direction="out" type="s"/>
    </method>
  </interface>

  <interface name="org.freedesktop.DBus.Properties">
    <method name="Get">
      <arg name="interface" direction="in" type="s"/>
      <arg name="property" direction="in" type="s"/>
      <arg name="value" direction="out" type="v"/>
    </method>
    <method name="GetAll">
      <arg name="interface" direction="in" type="s"/>
      <arg name="props" direction="out" type="a{sv}"/>
    </method>
    <method name="Set">
      <arg name="interface" direction="in" type="s"/>
      <arg name="property" direction="in" type="s"/>
      <arg name="value" direction="in" type="v"/>
    </method>
    <signal name="PropertiesChanged">
      <arg name="interface" type="s"/>
      <arg name="changed_properties" type="a{sv}"/>
      <arg name="invalidates_properties" type="as"/>
    </signal>
  </interface>
</node>`;

const bus_name = "io.github.ghedo.grooved";
const bus_path = "/io/github/ghedo/grooved";
const bus_interface_player = "io.github.ghedo.grooved.Player";
const bus_interface_introspect = "org.freedesktop.DBus.Introspectable";

var bus *Bus;

type Bus struct {
	player *player.Player;
	props  *prop.Properties;
};

func (b *Bus) TrackPosition() (float64, float64, *dbus.Error) {
	time, _ := b.player.GetTrackPosition(false);
	percent, _ := b.player.GetTrackPosition(true);

	return time, percent, nil;
}

func (b *Bus) Play() *dbus.Error {
	b.player.Play();
	return nil;
}

func (b *Bus) Pause() *dbus.Error {
	b.player.Pause();
	return nil;
}

func (b *Bus) Toggle() *dbus.Error {
	b.player.Toggle();
	return nil;
}

func (b *Bus) Next() *dbus.Error {
	b.player.Next();
	return nil;
}

func (b *Bus) Prev() *dbus.Error {
	b.player.Prev();
	return nil;
}

func (b *Bus) Stop() *dbus.Error {
	b.player.Stop();
	return nil;
}

func (b *Bus) Seek(seconds int64) *dbus.Error {
	b.player.Seek(seconds);
	return nil;
}

func (b *Bus) List() ([]string, int64, int64, *dbus.Error) {
	files, count, pos, _ := b.player.List();
	return files, count, pos, nil;
}

func (b *Bus) AddTrack(path string) *dbus.Error {
	b.player.AddTrack(path, false);
	return nil;
}

func (b *Bus) AddList(path string) *dbus.Error {
	b.player.AddList(path);
	return nil;
}

func (b *Bus) GotoTrack(index uint64) *dbus.Error {
	b.player.GotoTrack(int64(index));
	return nil;
}

func (b *Bus) RemoveTrack(index int64) *dbus.Error {
	b.player.RemoveTrack(int64(index));
	return nil;
}

func (b *Bus) Quit() *dbus.Error {
	b.player.Quit();
	return nil;
}

func HandleStatusChange() {
	status := bus.player.Status.String();
	bus.props.SetMust(bus_interface_player, "PlaybackStatus", status);
}

func HandleTrackChange() {
	metadata, _ := bus.player.GetTrackMetadata();
	bus.props.SetMust(bus_interface_player, "TrackMetadata", metadata);

	path, _ := bus.player.GetTrackPath();
	bus.props.SetMust(bus_interface_player, "TrackPath", path);

	length, _ := bus.player.GetTrackLength();
	bus.props.SetMust(bus_interface_player, "TrackLength", length);

	title, _ := bus.player.GetTrackTitle();
	bus.props.SetMust(bus_interface_player, "TrackTitle", title);
}

func SetLoopStatus(c *prop.Change) *dbus.Error {
	bus.player.SetLoopStatus(c.Value.(string));
	return nil;
}

func Run(pl *player.Player) error {
	conn, err := dbus.SessionBus();
	if err != nil {
		return fmt.Errorf("Could not get session bus: %s", err);
	}

	reply, err := conn.RequestName(bus_name, dbus.NameFlagDoNotQueue);
	if err != nil {
		return fmt.Errorf("Could not request name: %s", err);
	}

	if reply != dbus.RequestNameReplyPrimaryOwner {
		return fmt.Errorf("Name already take");
	}

	bus_props_spec := map[string]map[string]*prop.Prop{
		bus_interface_player: {
			"PlaybackStatus": {
				player.StatusStarting.String(), false,
				prop.EmitTrue, nil,
			},

			"LoopStatus": {
				"none", true, prop.EmitTrue, SetLoopStatus,
			},

			"TrackMetadata": {
				map[string]string{}, false, prop.EmitTrue, nil,
			},

			"TrackPath": {
				"", false, prop.EmitTrue, nil,
			},

			"TrackLength": {
				float64(0), false, prop.EmitTrue, nil,
			},

			"TrackTitle": {
				"", false, prop.EmitTrue, nil,
			},
		},
	};

	bus = new(Bus);

	conn.Export(bus, bus_path, bus_interface_player);

	bus.player = pl;
	bus.props  = prop.New(conn, bus_path, bus_props_spec);

	pl.HandleStatusChange = HandleStatusChange;
	pl.HandleTrackChange = HandleTrackChange;

	introspect := introspect.Introspectable(bus_introspection);
	conn.Export(introspect, bus_path, bus_interface_introspect);

	return nil;
}
