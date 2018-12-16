# grooved Makefile
# Copyright (C) 2014 Alessandro Ghedini <alessandro@ghedini.me>
# This file is released under the 2 clause BSD license, see COPYING

man:
	sphinx-build -c docs/ -b man docs/ docs/man

html:
	sphinx-build -c docs/ -b html docs/ docs/html
