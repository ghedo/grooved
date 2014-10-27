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
Specify the configuration file. (default: `~/.config/grooved/config.ini`)

`-V, --verbose`

&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;
Enable verbose log messages.

## CONFIG

Here is a list of valid configuration options:

`library=<path>`

&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;
The path to beets' database.

`verbose=<yes|no>`

&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;
Whether to enable verbose output. (optional)

`gapless=<yes|no|weak>`

&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;
Whether to play consecutive audio files with no silence or disruption at the
point of file change. (optional)

`replaygain=<track|album>`

&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;
Adjust volume gain according to ReplayGain tags. (optional)

`filter=<filter>`

&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;
Filter audio with the given filter. Can be specified multiple times, in which
case filters get chained one after the other. See mpv(1) for a list of supported
filters and options. (optional)

`output=<output>`

&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;
The output audio driver to use. See mpv(1) for a list of suported audio outputs
and options. (optional)

`cache=<kBytes|no|auto>`

&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;
The size of the cache in kilobytes. (optional)

## AUTHOR ##

Alessandro Ghedini <alessandro@ghedini.me>

## COPYRIGHT ##

Copyright (C) 2014 Alessandro Ghedini <alessandro@ghedini.me>

This program is released under the 2 clause BSD license.
