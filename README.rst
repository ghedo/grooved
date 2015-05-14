grooved
=======

**grooved** is a stupidly simple music player that runs as a daemon instead of
showing a fancy GUI. I'm kinda lazy, so instead or re-implementing some of the
features that make up a music player, I just used stuff someone else already
built. For example, grooved uses mpv_ to reproduce audio instead of implementing
its own half-working audio decoder and player, and piggybacks on tools like
beets_ instead of implementing yet another music database. Life gets so much
better once you let other people do the hard work.

grooved's operation is very simple: there's a single tracklist (or track queue,
or call-it-however-you-want) which can be filled manually by the user by adding
any kind of track (local files, network streams, YouTube videos, ..., you can
throw pretty much anything at it), which will be played in order. Once the
player reaches the end of the list, depending on the configuration it either
stops, loops to the beginning of the list, or picks a random track from the user
library and appends it to the list. And that's just about it. If you need
anything more complex, you can build a custom client on top of grooved's DBus
interface.

The project is composed of the following components:

* grooved_: the music player daemon.
* groovectl_: a command-line client to grooved.

.. _mpv: http://mpv.io/
.. _beets: http://beets.radbox.org/

Getting Started
---------------

First off, you may want to `install and configure beets`_, unless you haven't
done so already. grooved will use the music library created by beets to pick
random tracks to play. If no library is available you'll have to provide the
tracks to play to grooved manually.

If you want grooved to use your music library, you'll also have to configure
grooved itself. Here's a simple configuration example:

.. code-block:: ini

   [default]
   library = ~/data/musiclibrary.blb ; path to beets' database

Save that in the file *~/.config/grooved/config.ini* (see the manual__ for more
information about configuration options).

That's it for the setup, now start `grooved`:

.. code-block:: bash

   $ grooved

You can now control it using the *groovectl* command:

.. code-block:: bash

   $ groovectl toggle                           # toggle playback
   $ groovectl status                           # print the player's current status
   $ groovectl add file.mp3                     # add files to the tracklist
   $ groovectl next                             # skip to next track
   $ groovectl add http://example.com/stream    # add network stream to tracklist
   $ groovectl quit                             # terminate grooved

You can also use beets to search for songs in your library and add them to the
grooved tracklist as follows:

.. code-block:: bash

   $ beet ls -p song title | groovectl add -

You can use beets' `query syntax`_  for more advanced queries to
the database.

Alternatively you can use beets' `play plugin`_ to automatically add the files
(note that at least beets v1.3.7 is required for this). First, `enable the play
plugin`_ and add the following section to your configuration file:

.. code-block:: yaml

   play:
       command: groovectl --append load

then use the `beet play` command to search for tracks:

.. code-block:: bash

   $ beet play song title

the matching tracks will be automatically added to grooved's tracklist. You can
use beets' `query syntax`_ for more advanced queries to the database with the
`play` command as well.

See grooved_ and groovectl_ for more information.

.. _`install and configure beets`: http://beets.readthedocs.org/en/latest/guides/main.html
.. _`play plugin`: http://beets.readthedocs.org/en/latest/plugins/play.html
.. _`query syntax`: http://beets.readthedocs.org/en/latest/reference/query.html
.. _`enable the play plugin`: http://beets.readthedocs.org/en/latest/plugins/index.html#using-plugins
__ grooved_

Dependencies
------------

* `libmpv`

Building
--------

grooved is distributed as source code. Build with:

.. code-block:: bash

   $ make

Copyright
---------

Copyright (C) 2014 Alessandro Ghedini <alessandro@ghedini.me>

See COPYING_ for the license.

.. _grooved: http://ghedo.github.io/grooved/grooved.html
.. _groovectl: http://ghedo.github.io/grooved/groovectl.html
.. _COPYING: https://github.com/ghedo/pflask/tree/master/COPYING
