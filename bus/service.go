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

import "github.com/ghedo/grooved/player"

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

    <property name="Tracks" type="as" access="read">
    </property>

    <property name="Volume" type="d" access="readwrite">
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
</node>`

const bus_name = "io.github.ghedo.grooved"
const bus_path = "/io/github/ghedo/grooved"
const bus_interface_player = "io.github.ghedo.grooved.Player"
const bus_interface_introspect = "org.freedesktop.DBus.Introspectable"

var bus *Bus

type Bus struct {
    player *player.Player
    props  *prop.Properties
}

func (b *Bus) TrackPosition() (float64, float64, *dbus.Error) {
    time, err := b.player.GetTrackPosition(false)
    if err != nil {
        return 0, 0, dbus.NewError("io.github.ghedo.grooved.Error",
                                   []interface{}{err.Error()})
    }

    percent, err := b.player.GetTrackPosition(true)
    if err != nil {
        return 0, 0, dbus.NewError("io.github.ghedo.grooved.Error",
                                   []interface{}{err.Error()})
    }

    return time, percent, nil
}

func (b *Bus) Play() *dbus.Error {
    err := b.player.Play()
    if err != nil {
        return dbus.NewError("io.github.ghedo.grooved.Error",
                             []interface{}{err.Error()})
    }

    return nil
}

func (b *Bus) Pause() *dbus.Error {
    err := b.player.Pause()
    if err != nil {
        return dbus.NewError("io.github.ghedo.grooved.Error",
                             []interface{}{err.Error()})
    }

    return nil
}

func (b *Bus) Toggle() *dbus.Error {
    err := b.player.Toggle()
    if err != nil {
        return dbus.NewError("io.github.ghedo.grooved.Error",
                             []interface{}{err.Error()})
    }

    return nil
}

func (b *Bus) Next() *dbus.Error {
    err := b.player.Next()
    if err != nil {
        return dbus.NewError("io.github.ghedo.grooved.Error",
                             []interface{}{err.Error()})
    }

    return nil
}

func (b *Bus) Prev() *dbus.Error {
    err := b.player.Prev()
    if err != nil {
        return dbus.NewError("io.github.ghedo.grooved.Error",
                             []interface{}{err.Error()})
    }

    return nil
}

func (b *Bus) Stop() *dbus.Error {
    err := b.player.Stop()
    if err != nil {
        return dbus.NewError("io.github.ghedo.grooved.Error",
                             []interface{}{err.Error()})
    }

    return nil
}

func (b *Bus) Seek(seconds int64) *dbus.Error {
    err := b.player.Seek(seconds)
    if err != nil {
        return dbus.NewError("io.github.ghedo.grooved.Error",
                             []interface{}{err.Error()})
    }

    return nil
}

func (b *Bus) AddTrack(path string) *dbus.Error {
    err := b.player.AddTrack(path, false)
    if err != nil {
        return dbus.NewError("io.github.ghedo.grooved.Error",
                             []interface{}{err.Error()})
    }

    return nil
}

func (b *Bus) AddList(path string) *dbus.Error {
    err := b.player.AddList(path)
    if err != nil {
        return dbus.NewError("io.github.ghedo.grooved.Error",
                             []interface{}{err.Error()})
    }

    return nil
}

func (b *Bus) GotoTrack(index uint64) *dbus.Error {
    err := b.player.GotoTrack(int64(index))
    if err != nil {
        return dbus.NewError("io.github.ghedo.grooved.Error",
                             []interface{}{err.Error()})
    }

    return nil
}

func (b *Bus) RemoveTrack(index int64) *dbus.Error {
    err := b.player.RemoveTrack(int64(index))
    if err != nil {
        return dbus.NewError("io.github.ghedo.grooved.Error",
                             []interface{}{err.Error()})
    }

    return nil
}

func (b *Bus) Quit() *dbus.Error {
    err := b.player.Quit()
    if err != nil {
        return dbus.NewError("io.github.ghedo.grooved.Error",
                             []interface{}{err.Error()})
    }

    return nil
}

func HandleStatusChange() {
    status := bus.player.Status.String()
    bus.props.SetMust(bus_interface_player, "PlaybackStatus", status)
}

func HandleTrackChange() {
    metadata, _ := bus.player.GetTrackMetadata()
    bus.props.SetMust(bus_interface_player, "TrackMetadata", metadata)

    path, _ := bus.player.GetTrackPath()
    bus.props.SetMust(bus_interface_player, "TrackPath", path)

    length, _ := bus.player.GetTrackLength()
    bus.props.SetMust(bus_interface_player, "TrackLength", length)

    title, _ := bus.player.GetTrackTitle()
    bus.props.SetMust(bus_interface_player, "TrackTitle", title)
}

func HandleTracksChange() {
    files, _ := bus.player.List()
    bus.props.SetMust(bus_interface_player, "Tracks", files)
}

func HandleVolumeChange() {
    vol, _ := bus.player.GetProperty("volume")
    bus.props.SetMust(bus_interface_player, "Volume", vol.(float64))
}

func SetLoopStatus(c *prop.Change) *dbus.Error {
    bus.player.SetLoopStatus(c.Value.(string))
    return nil
}

func SetVolume(c *prop.Change) *dbus.Error {
    err := bus.player.SetProperty("volume", c.Value)
    if err != nil {
        return dbus.NewError("io.github.ghedo.grooved.Error",
                             []interface{}{err.Error()})
    }

    return nil
}

func Run(p *player.Player) error {
    conn, err := dbus.SessionBus()
    if err != nil {
        return fmt.Errorf("Could not get session bus: %s", err)
    }

    reply, err := conn.RequestName(bus_name, dbus.NameFlagDoNotQueue)
    if err != nil {
        return fmt.Errorf("Could not request name: %s", err)
    }

    if reply != dbus.RequestNameReplyPrimaryOwner {
        return fmt.Errorf("Name already take")
    }

    bus_props_spec := map[string]map[string]*prop.Prop{
        bus_interface_player: {
            "PlaybackStatus": {
                p.Status.String(), false,
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

            "Tracks": {
                []string{}, false, prop.EmitTrue, nil,
            },

            "Volume": {
                0.0, true, prop.EmitTrue, SetVolume,
            },
        },
    }

    bus = &Bus{
        player: p,
        props:  prop.New(conn, bus_path, bus_props_spec),
    }

    conn.Export(bus, bus_path, bus_interface_player)

    p.HandleStatusChange = HandleStatusChange
    p.HandleTrackChange  = HandleTrackChange
    p.HandleTracksChange = HandleTracksChange
    p.HandleVolumeChange = HandleVolumeChange

    introspect := introspect.Introspectable(bus_introspection)
    conn.Export(introspect, bus_path, bus_interface_introspect)

    return nil
}
