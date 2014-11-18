# grooved Makefile
# Copyright (C) 2014 Alessandro Ghedini <alessandro@ghedini.me>
# This file is released under the 2 clause BSD license, see COPYING

export GOPATH:=$(CURDIR):$(GOPATH)

BUILDTAGS=debug

all: grooved groovectl

grooved:
	go get -tags '$(BUILDTAGS)' -d -v main/grooved
	go install -tags '$(BUILDTAGS)' main/grooved

groovectl:
	go get -tags '$(BUILDTAGS)' -d -v main/groovectl
	go install -tags '$(BUILDTAGS)' main/groovectl

man: docs/grooved.1.md docs/groovectl.1.md
	ronn -r $?

html: docs/grooved.1.md docs/groovectl.1.md
	ronn -h $?

release-all: BUILDTAGS=release
release-all: all

clean:
	go clean -i main/grooved main/groovectl bus library player

.PHONY: all grooved deps clean
