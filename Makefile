# grooved Makefile
# Copyright (C) 2014 Alessandro Ghedini <alessandro@ghedini.me>
# This file is released under the 2 clause BSD license, see COPYING

export GOPATH:=$(GOPATH):$(CURDIR)

BUILDTAGS=debug

all: grooved groovectl

grooved: deps
	go install -tags '$(BUILDTAGS)' main/grooved

groovectl:
	go install -tags '$(BUILDTAGS)' main/groovectl

deps:
	go get -tags '$(BUILDTAGS)' -d -v main/grooved

man: docs/grooved.1.md docs/groovectl.1.md
	ronn -r $?

html: docs/grooved.1.md docs/groovectl.1.md
	ronn -h $?

release-all: BUILDTAGS=release
release-all: all

clean:
	go clean -i main/grooved main/groovectl bus library player

.PHONY: all grooved deps clean
