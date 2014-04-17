groovectl(1) -- groovy music player client
==========================================

## SYNOPSIS

`groovectl COMMAND [ARGS]`

## DESCRIPTION

**groovectl** is a command-line client for grooved. It communicates with the
grooved daemon via the DBus session bus.

## COMMANDS

`add TRACK [, TRACK ...]`

&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;
Append tracks to the player's tracklist. Tracks can be either files on the
filesystem or other kinds of streams (e.g. HTTP streams).

If the first track argument is "-", track names will be read from STDIN (one
track per line).

`last`

&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;
Stop playback after the currently playing track has ended.

`list`

&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;
Show tracklist (the current track is marked with '*').

`loop on|off`

&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;
Set the player's loop mode.

`lyrics`

&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;
Download and show lyrics for the currently playing track.

`next`

&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;
Skip to the next track.

`pause`

&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;
Pause the player.

`play`

&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;
Unpause the player.

`prev`

&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;
Skip to the previous track.

`quit`

&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;
Shutdown the player.

`rgain track|album|none`

&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;
Set the player's replaygain mode.

`seek SECONDS`

&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;
Seek by the given amount of seconds, relative to the current position. A
negative value seeks backwards.

`status`

&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;
Show the status of the player.

`toggle`

&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;
Toggle the player's pause status.

`stop`

&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;
Stop playback and clear tracklist.

## AUTHOR ##

Alessandro Ghedini <alessandro@ghedini.me>

## COPYRIGHT ##

Copyright (C) 2014 Alessandro Ghedini <alessandro@ghedini.me>

This program is released under the 2 clause BSD license.

