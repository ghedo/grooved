.. _grooved(1):

grooved
=======

SYNOPSIS
--------

.. program:: grooved

**grooved [options]**

DESCRIPTION
-----------

**grooved** is a stupidly simple music player that runs as a daemon instead of
showing a fancy GUI. I'm kinda lazy, so instead or re-implementing some of the
features that make up a music player, I just used stuff someone else already
built. For example, grooved uses mpv to reproduce audio instead of implementing
its own half-working audio decoder and player, and piggybacks on tools like
beets instead of implementing yet another music database. Life gets so much
better once you let other people do the hard work.

OPTIONS
-------

.. option:: -c, --config=<file>

Specify the configuration file. (default: `~/.config/grooved/config.ini`)

.. option:: --list-outputs

List supported outputs.

.. option:: -V, --verbose

Enable verbose log messages.

CONFIG
------

Here is a list of valid configuration options:

.. option:: library=<path>

The path to beets' database.

.. option:: notify=<yes|no>

Send desktop notifications on track change.

.. option:: verbose=<yes|no>

Whether to enable verbose output. (optional)

In addition to these options, grooved will accept any option supported by
mpv itself (e.g. `ao`, `cache`, `gapless-audio`, `ytdl`, ...) but note that
not all values make sense as part of grooved's configuration.

AUTHOR
------

Alessandro Ghedini <alessandro@ghedini.me>

COPYRIGHT
---------

Copyright (C) 2014 Alessandro Ghedini <alessandro@ghedini.me>

This program is released under the 2 clause BSD license.
