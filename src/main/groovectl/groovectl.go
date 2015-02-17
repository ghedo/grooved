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

package main

import "log"
import "path/filepath"
import "strconv"
import "time"
import "os"

import "github.com/docopt/docopt-go"
import "github.com/godbus/dbus"

const bus_name = "io.github.ghedo.grooved"
const bus_path = "/io/github/ghedo/grooved"
const bus_interface = "io.github.ghedo.grooved.Player"

func main() {
	log.SetFlags(0)

	usage := `groovectl [options] <command> [args...]

Usage:
  groovectl play
  groovectl pause
  groovectl toggle
  groovectl next
  groovectl prev
  groovectl stop
  groovectl add <track> ...
  groovectl load [--append] <file>
  groovectl save <file>
  groovectl goto <index>
  groovectl rm <index>
  groovectl ls
  groovectl status
  groovectl seek <seconds>
  groovectl loop (none|track|list|force)
  groovectl quit

Commands:
  play                           Unpause the player.
  pause                          Pause the player.
  toggle                         Toggle the player's pause status.
  next                           Skip to next track.
  prev                           Skip to previous track.
  stop                           Stop playback and clear tracklist.
  add <track> ...                Append tracks to the player's tracklist.
  load [--append] <file>         Load a playlist file.
  save <file>                    Save the tracklist to a playlist file.
  goto <index>                   Skip to a specific track in the tracklist.
  rm <index>                     Remove a track from the tracklist.
  ls                             Show the tracklist.
  status                         Show the status of the player.
  seek <seconds>                 Seek by a relative amount of seconds.
  loop (none|track|list|force)   Set the player's loop mode.
  quit                           Shutdown the player.

Options:
  -h, --help                         Show the program's help message and exit.`

	args, err := docopt.Parse(usage, nil, true, "", false)
	if err != nil {
		log.Fatalf("Invalid arguments: %s", err)
	}

	conn, err := dbus.SessionBus()
	if err != nil {
		log.Fatalf("Could not get session bus: %s", err)
	}

	obj := conn.Object(bus_name, bus_path)

	var call *dbus.Call

	switch {
	case args["play"].(bool) == true:
		call = obj.Call(bus_interface + ".Play", 0)

	case args["pause"].(bool) == true:
		call = obj.Call(bus_interface + ".Pause", 0)

	case args["toggle"].(bool) == true:
		call = obj.Call(bus_interface + ".Toggle", 0)

	case args["next"].(bool) == true:
		call = obj.Call(bus_interface + ".Next", 0)

	case args["prev"].(bool) == true:
		call = obj.Call(bus_interface + ".Prev", 0)

	case args["stop"].(bool) == true:
		call = obj.Call(bus_interface + ".Stop", 0)

	case args["add"].(bool) == true:
		for _, track := range args["<track>"].([]string) {
			if _, err := os.Stat(track); err == nil {
				track, _ = filepath.Abs(track)
			}

			call = obj.Call(bus_interface + ".AddTrack", 0, track)
		}

	case args["load"].(bool) == true:
		file := args["<file>"].(string)

		if !args["--append"].(bool) {
			call = obj.Call(bus_interface + ".Stop", 0)
		}

		call = obj.Call(bus_interface + ".AddList", 0, file)

	case args["save"].(bool) == true:
		log.Fatalf("Not implemented")

	case args["goto"].(bool) == true:
		index, err := strconv.ParseUint(args["<index>"].(string), 10, 64)
		if err != nil {
			log.Fatalf("Could not parse arg: %s", err)
		}

		call = obj.Call(bus_interface + ".GotoTrack", 0, index)

	case args["rm"].(bool) == true:
		index, err := strconv.ParseInt(args["<index>"].(string), 10, 64)
		if err != nil {
			log.Fatalf("Could not parse arg: %s", err)
		}

		call = obj.Call(bus_interface + ".RemoveTrack", 0, index)

	case args["ls"].(bool) == true:
		PrintList(obj)
		os.Exit(0)

	case args["status"].(bool) == true:
		PrintStatus(obj)
		os.Exit(0)

	case args["seek"].(bool) == true:
		secs, err := strconv.ParseInt(args["<seconds>"].(string), 10, 64)
		if err != nil {
			log.Fatalf("Could not parse arg: %s", err)
		}

		call = obj.Call(bus_interface + ".Seek", 0, secs)

	case args["loop"].(bool) == true:
		var mode string

		switch {
		case args["none"].(bool):
			mode = "none"

		case args["track"].(bool):
			mode = "track"

		case args["list"].(bool):
			mode = "list"

		case args["force"].(bool):
			mode = "force"
		}

		call = obj.Call("org.freedesktop.DBus.Properties.Set", 0,
                                bus_interface, "LoopStatus",
                                dbus.MakeVariant(mode))

	case args["quit"].(bool) == true:
		call = obj.Call(bus_interface + ".Quit", 0)
	}

	if call.Err != nil {
		log.Fatalf("Error calling method: %s", call.Err)
	}
}

func PrintList(obj *dbus.Object) {
	files, err := obj.GetProperty(bus_interface + ".Tracks")
	if err != nil {
		log.Fatalf("Could not retrieve property: %s", err)
	}

	current, _ := obj.GetProperty(bus_interface + ".TrackPath")

	for i, file := range files.Value().([]string) {
		var prefix string

		if current.Value() != nil && file == current.Value().(string) {
			prefix = "*"
		} else {
			prefix = " "
		}

		log.Printf("%s %3d:%s\n", prefix, i, file)
	}
}

func PrintStatus(obj *dbus.Object) {
	title, err := obj.GetProperty(bus_interface + ".TrackTitle")
	if err != nil {
		log.Fatalf("Could not retrieve property: %s", err)
	}

	state, err := obj.GetProperty(bus_interface + ".PlaybackStatus")
	if err != nil {
		log.Fatalf("Could not retrieve property: %s", err)
	}

	metadata, err := obj.GetProperty(bus_interface + ".TrackMetadata")
	if err != nil {
		log.Fatalf("Could not retrieve property: %s", err)
	}

	length, err := obj.GetProperty(bus_interface + ".TrackLength")
	if err != nil {
		log.Fatalf("Could not retrieve property: %s", err)
	}

	var pos, percent float64
	obj.Call(bus_interface + ".TrackPosition", 0).Store(&pos, &percent)

	loop, err := obj.GetProperty(bus_interface + ".LoopStatus")
	if err != nil {
		log.Fatalf("Could not retrieve property: %s", err)
	}

	log.Printf("Title: %s\n", title.Value().(string))

	log.Printf("Tags:\n")

	for key, val := range metadata.Value().(map[string]string) {
		log.Printf(" %s: %s\n", key, val)
	}

	pos_time := time.Duration(int64(pos)) * time.Second
	len_time := time.Duration(int64(length.Value().(float64))) * time.Second

	log.Printf(
		"[%s]   %s/%s   (%.f%%)\n",
		state.Value().(string), pos_time.String(), len_time.String(),
		percent,
	)

	log.Printf("loop: %s\n", loop.Value().(string))
}
