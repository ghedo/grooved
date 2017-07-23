.. _groovectl(1):

groovectl
=========

SYNOPSIS
--------

.. program:: groovectl

**groovectl command [args]**

DESCRIPTION
-----------

**groovectl** is a command-line client for grooved. It communicates with the
grooved daemon via the DBus session bus.

COMMANDS
--------

.. option:: play

Unpause the player.

.. option:: pause

Pause the player.

.. option:: toggle

Toggle the player's pause status.

.. option:: next

Skip to the next track.

.. option:: prev

Skip to the previous track.

.. option:: stop

Stop playback and clear tracklist.

.. option:: add TRACK

Append tracks to the player's tracklist. Tracks can be either files on the
filesystem or other kinds of streams (e.g. HTTP streams, Youtube videos, ...).

.. option:: [--append] load FILE

Load a playlist file. The tracklist will be replaced with the content of the
given playlist and playback stopped, unless `--append` is used.

.. option:: save FILE

Save the tracklist to a playlist file.

.. option:: goto INDEX

Skip to a specific track, identified by its index, in the tracklist.

.. option:: rm INDEX

Remove a track identified by its index in the tracklist, from the tracklist. The
special value `-1` corresponds to the current track.

.. option:: ls

Show tracklist (the current track is marked with '*').

.. option:: status

Show the status of the player.

.. option:: seek SECONDS

Seek by the given amount of seconds relative to the current position. A negative
value seeks backwards.

.. option:: loop track|list|none|force

Set the player's loop mode.

.. option:: quit

Shutdown the player.

AUTHOR
------

Alessandro Ghedini <alessandro@ghedini.me>

COPYRIGHT
---------

Copyright (C) 2014 Alessandro Ghedini <alessandro@ghedini.me>

This program is released under the 2 clause BSD license.
