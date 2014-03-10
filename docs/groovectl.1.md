groovectl(1) -- groovy music player client
==========================================

## SYNOPSIS

`groovectl COMMAND [ARGS]`

## DESCRIPTION

**groovectl** is a command-line client for grooved. It communicates with the
groved daemon via the DBus session bus.

## COMMANDS

`status`

&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;
Show the status of the player.

`play`

&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;
Unpause the player.

`pause`

&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;
Pause the player.

`toggle`

&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;
Toggle the player's pause status.

`next`

&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;
Skip to the next track.

`prev`

&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;
Skip to the previous track.

`add TRACK [, TRACK ...]`

&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;
Append tracks to the player's tracklist. Tracks can be either files on the
filesystem or other kinds of streams (e.g. HTTP streams).

`rgain track|album|none`

&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;
Set the player's replaygain mode.

`loop on|off`

&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;
Set the player's loop mode.

`lyrics`

&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;
Download and show lyrics for the currently playing track.

`quit`

&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;
Shutdown the player.

## AUTHOR ##

Alessandro Ghedini <alessandro@ghedini.me>

## COPYRIGHT ##

Copyright (C) 2014 Alessandro Ghedini <alessandro@ghedini.me>

This program is released under the 2 clause BSD license.

