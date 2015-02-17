groovectl(1) -- groovy music player client
==========================================

## SYNOPSIS

`groovectl COMMAND [ARGS]`

## DESCRIPTION

**groovectl** is a command-line client for grooved. It communicates with the
grooved daemon via the DBus session bus.

## COMMANDS

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

`stop`

&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;
Stop playback and clear tracklist.

`add TRACK`

&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;
Append tracks to the player's tracklist. Tracks can be either files on the
filesystem or other kinds of streams (e.g. HTTP streams, Youtube videos, ...).

`load [--append] FILE`

&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;
Load a playlist file. The tracklist will be replaced with the content of the
given playlist and playback stopped, unless `--append` is used.

`save FILE`

&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;
Save the tracklist to a playlist file.

`goto INDEX`

&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;
Skip to a specific track, identified by its index, in the tracklist.

`rm INDEX`

&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;
Remove a track identified by its index in the tracklist, from the tracklist. The
special value `-1` corresponds to the current track.

`ls`

&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;
Show tracklist (the current track is marked with '*').

`status`

&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;
Show the status of the player.

`seek SECONDS`

&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;
Seek by the given amount of seconds relative to the current position. A negative
value seeks backwards.

`loop track|list|none|force`

&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;
Set the player's loop mode.

`quit`

&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;
Shutdown the player.

## AUTHOR ##

Alessandro Ghedini <alessandro@ghedini.me>

## COPYRIGHT ##

Copyright (C) 2014 Alessandro Ghedini <alessandro@ghedini.me>

This program is released under the 2 clause BSD license.
