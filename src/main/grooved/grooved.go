/*
 * Groovy music player daemon.
 *
 * Copyright (c) 2014, Alessandro Ghedini
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS
 * IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

package main

import "log"

import "github.com/docopt/docopt-go"
import "github.com/vaughan0/go-ini"

import "bus"
import "player"
import "util"

func main() {
	log.SetFlags(0);

	usage := `Usage: grooved [options]

Options:
  -c <file>, --config <file>    Configuration file [default: ~/.config/grooved/config.ini].
  -V, --verbose                 Enable verbose log messages [default: false].
  -h, --help                    Show the program's help message and exit.`

	args, err := docopt.Parse(usage, nil, true, "", false)
	if err != nil {
		log.Fatalf("Invalid arguments: %s", err);
	}

	cfg_file, err := util.ExpandUser(args["--config"].(string));
	if err != nil {
		log.Fatalf("Error expanding home directory: %s", err);
	}

	cfg, err := ini.LoadFile(cfg_file);
	if err != nil {
		log.Fatalf("Error loading config file: %s", err);
	}

	player, err := player.Run(cfg);
	if err != nil {
		log.Fatalf("Error creating player: %s", err);
	}

	err = bus.Run(player);
	if err != nil {
		log.Fatalf("Error creating dbus service: %s", err);
	}

	player.Wait.Wait();
}