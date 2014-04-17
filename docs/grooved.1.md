grooved(1) -- groovy music player daemon
========================================

## SYNOPSIS

`grooved [OPTIONS]`

## DESCRIPTION

**grooved** is a stupidly simple music player that runs as a daemon instead of
showing a fancy GUI. I'm kinda lazy, so instead or re-implementing some of the
features that make up a music player, I just used stuff someone else already
built. For example, grooved uses mpv to reproduce audio instead of implementing
its own half-working audio decoder and player, and piggybacks on tools like
beets instead of implementing yet another music database. Life gets so much
better once you let other people do the hard work.

## OPTIONS

`-c, --config=<file>`

&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;
Specify the configuration file (default: `~/.config/grooved/config.ini`).

`-V, --verbose`

&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;
Enable verbose log messages.

## CONFIG

Here is an example configuration that illustrates the available options:

```
[default]
library = ~/data/musiclibrary.blb    ; path to beets' database
verbose = off                        ; enable/disable verbose output

[player]
replaygain = none                    ; optionally enable replaygain, can be
```                                     ; "track", "album" or "none"

## AUTHOR ##

Alessandro Ghedini <alessandro@ghedini.me>

## COPYRIGHT ##

Copyright (C) 2014 Alessandro Ghedini <alessandro@ghedini.me>

This program is released under the 2 clause BSD license.
