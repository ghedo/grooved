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

package player

// #cgo pkg-config: mpv
// #include <mpv/client.h>
import "C"

import "fmt"
import "log"
import "strconv"
import "sync"

import "github.com/vaughan0/go-ini"

import "library"
import "notify"
import "util"

type Event byte
type Status byte

const (
	StatusPlaying Status = iota
	StatusPaused
	StatusStopped
)

func (s Status) String() string {
	switch s {
	case StatusPlaying:
		return "play"

	case StatusPaused:
		return "pause"

	case StatusStopped:
		return "stop"
	}

	return "invalid"
}

type Player struct {
	handle *C.mpv_handle
	Status Status

	library string
	notify  bool
	started bool

	HandleStatusChange func()
	HandleTrackChange  func()
	HandleTracksChange func()

	Wait sync.WaitGroup
}

func (p *Player) ChangeStatus(status Status) {
	p.Status = status
	p.HandleStatusChange()
}

func (p *Player) Play() error {
	switch p.Status {
	case StatusPlaying:
		return nil

	case StatusStopped:
		count, err := p.GetProperty("playlist-count")
		if err == nil && count.(int64) > 0 {
			return p.GotoTrack(0)
		}

		return p.AddTrack("", true)

	case StatusPaused:
		return p.Command([]string{"cycle", "pause"})
	}

	return fmt.Errorf("Invalid player state")
}

func (p *Player) Pause() error {
	switch p.Status {
	case StatusPaused, StatusStopped:
		return nil

	case StatusPlaying:
		return p.Command([]string{"cycle", "pause"})
	}

	return fmt.Errorf("Invalid player state")
}

func (p *Player) Toggle() error {
	switch p.Status {
	case StatusPaused, StatusStopped:
		return p.Play()

	case StatusPlaying:
		return p.Pause()
	}

	return fmt.Errorf("Invalid player state")
}

func (p *Player) Next() error {
	return p.Command([]string{"playlist_next", "force"})
}

func (p *Player) Prev() error {
	return p.Command([]string{"playlist_prev", "weak"})
}

func (p *Player) Stop() error {
	err := p.Command([]string{"stop"})

	p.ChangeStatus(StatusStopped)
	p.HandleTrackChange()

	return err
}

func (p *Player) Seek(seconds int64) error {
	secs := strconv.FormatInt(seconds, 10)
	return p.Command([]string{"seek", secs})
}

func (p *Player) List() ([]string, error) {
	playlist, err := p.GetProperty("playlist")
	if err != nil {
		return nil, nil
	}

	var files []string

	for _, entry := range playlist.([]interface{}) {
		if entry == nil {
			continue
		}

		entry_map := entry.(map[string]interface{})

		files = append(files, entry_map["filename"].(string))
	}

	return files, nil
}

func (p *Player) AddTrack(path string, play bool) error {
	var mode string

	if play {
		mode = "append-play"
	} else {
		mode = "append"
	}

	if path == "" {
		var err error

		path, err = library.Random(p.library)
		if err != nil {
			return fmt.Errorf("Could not get random track: %s", err)
		}
	}

	return p.Command([]string{"loadfile", path, mode})
}

func (p *Player) AddList(path string) error {
	return p.Command([]string{"loadlist", path, "append"})
}

func (p *Player) GotoTrack(index int64) error {
	return p.SetProperty("playlist-pos", index)
}

func (p *Player) RemoveTrack(index int64) error {
	var track string

	if index < 0 {
		track = "current"
	} else {
		track = strconv.FormatInt(index, 10)
	}

	return p.Command([]string{"playlist_remove", track})
}

func (p *Player) Quit() error {
	return p.Command([]string{"quit"})
}

func (p *Player) GetTrackMetadata() (map[string]string, error) {
	metadata, err := p.GetProperty("metadata")
	if err != nil {
		return nil, err
	}

	metadata_str := map[string]string{}

	for key, val := range metadata.(map[string]interface{}) {
		metadata_str[key] = val.(string)
	}

	return metadata_str, err
}

func (p *Player) GetTrackLength() (float64, error) {
	length, err := p.GetProperty("length")
	if err != nil {
		return 0.0, err
	}

	return length.(float64), nil
}

func (p *Player) GetTrackPath() (string, error) {
	path, err := p.GetProperty("path")
	if err != nil {
		return "", err
	}

	return path.(string), nil
}

func (p *Player) GetTrackPosition(percent bool) (float64, error) {
	var err error
	var pos interface{}

	if !percent {
		pos, err = p.GetProperty("time-pos")
	} else {
		pos, err = p.GetProperty("percent-pos")
	}

	if err != nil {
		return 0.0, err
	}

	return pos.(float64), nil
}

func (p *Player) GetTrackTitle() (string, error) {
	title, err := p.GetProperty("media-title")
	if err != nil {
		return "", nil
	}

	metadata, err := p.GetTrackMetadata()
	if err != nil {
		return title.(string), nil
	}

	artist := metadata["artist"]
	if artist == "" {
		artist = metadata["ARTIST"]
	}

	if artist != "" {
		return fmt.Sprintf("%s - %s", artist, title.(string)), nil
	}

	return title.(string), nil
}

func (p *Player) SetLoopStatus(mode string) error {
	switch mode {
	case "none":
		p.SetProperty("loop-file", false)
		p.SetProperty("loop", "no")

	case "track":
		p.SetLoopStatus("none")
		p.SetProperty("loop-file", true)

	case "list":
		p.SetLoopStatus("none")
		p.SetProperty("loop", "inf")

	case "force":
		p.SetLoopStatus("none")
		p.SetProperty("loop", "force")

	default:
		return fmt.Errorf("Invalid mode")
	}

	return nil
}

func (p *Player) GetOutputList() ([]string, error) {
	outputs, err := p.GetProperty("option-info/ao/choices")
	if err != nil {
		return nil, err
	}

	outs := []string{}
	for _, output := range outputs.([]interface{}) {
		outs = append(outs, output.(string))
	}

	return outs, nil
}

func Init(cfg ini.File) (*Player, error) {
	p := &Player{
		Status:  StatusStopped,
		started: false,
	}

	p.handle = C.mpv_create()
	if p.handle == nil {
		return nil, fmt.Errorf("Could not create player")
	}

	err := p.SetOptionString("no-config", "")
	if err != nil {
		return nil, fmt.Errorf("Could not set option 'no-config': %s", err)
	}

	err = p.SetOptionString("no-video", "")
	if err != nil {
		return nil, fmt.Errorf("Could not set option 'no-video': %s", err)
	}

	err = p.SetOptionString("no-sub", "")
	if err != nil {
		return nil, fmt.Errorf("Could not set option 'no-sub': %s", err)
	}

	err = p.SetOptionString("no-softvol", "")
	if err != nil {
		return nil, fmt.Errorf("Could not set option 'no-softvol': %s", err)
	}

	if cfg["default"]["cache"] != "" {
		p.SetOptionString("cache", cfg["default"]["cache"])
	}

	if cfg["default"]["gapless"] != "" {
		p.SetOptionString("gapless-audio", cfg["default"]["gapless"])
	}

	p.library, _ = util.ExpandUser(cfg["default"]["library"])

	if cfg["default"]["notify"] == "yes" {
		p.notify = true
	} else {
		p.notify = false
	}

	if cfg["default"]["replaygain"] != "" {
		rgain_af := fmt.Sprintf("volume=replaygain-%s", cfg["default"]["replaygain"])
		if cfg["default"]["filters"] != "" {
			cfg["default"]["filters"] += "," + rgain_af
		} else {
			cfg["default"]["filters"] = rgain_af
		}
	}

	if cfg["default"]["output"] != "" {
		p.SetOptionString("ao", cfg["default"]["output"])
	}

	if cfg["default"]["ytdl"] != "" {
		p.SetOptionString("ytdl", cfg["default"]["ytdl"])
	}

	if cfg["default"]["filters"] != "" {
		p.SetOptionString("af", cfg["default"]["filters"])
	}

	if cfg["default"]["scripts"] != "" {
		p.SetOptionString("lua", cfg["default"]["scripts"])
	}

	C.mpv_request_log_messages(p.handle, C.CString("warn"))

	mp_err := C.mpv_initialize(p.handle)
	if mp_err != 0 {
		return nil, ErrorString(mp_err)
	}

	return p, nil
}

func (p *Player) Run() error {
	p.Wait.Add(1)

	go p.EventLoop()

	return nil
}

func (p *Player) HandlePauseChange() {
	if !p.started {
		return
	}

	pause, err := p.GetProperty("pause")
	if err != nil {
		return
	}

	if pause.(bool) {
		p.ChangeStatus(StatusPaused)
	} else {
		p.ChangeStatus(StatusPlaying)
	}
}

func (p *Player) HandleMetadataChange() {
	if !p.started {
		return
	}

	if p.notify {
		msg, _ := p.GetTrackTitle()
		notify.Notify("Now Playing:", msg, "media-playback-start")
	}

	p.HandleTrackChange()
}

func (p *Player) EventLoop() {
	p.ObserveProperty("pause",    FormatFlag)
	p.ObserveProperty("metadata", FormatNode)
	p.ObserveProperty("playlist", FormatNode)

	for {
		ev := C.mpv_wait_event(p.handle, -1)
		ev_name := C.GoString(C.mpv_event_name(ev.event_id))

		switch ev_name {
		case "idle":
			if p.Status == StatusStopped {
				break
			}

			err := p.AddTrack("", true)
			if err != nil {
				log.Println("Could not add track: %s", err)
				p.ChangeStatus(StatusStopped)
			}

		case "start-file":
			if !p.started {
				p.started = true
				p.ChangeStatus(StatusPlaying)
			}

		case "property-change":
			prop := (*C.mpv_event_property)(ev.data)
			prop_name := C.GoString(prop.name)

			if prop.format == FormatNone {
				break
			}

			switch prop_name {
			case "pause":
				p.HandlePauseChange()

			case "metadata":
				p.HandleMetadataChange()

			case "playlist":
				p.HandleTracksChange()
			}

		case "log-message":
			mp_log := (*C.mpv_event_log_message)(ev.data)
			log.Printf("%s: %s: %s",
			           C.GoString(mp_log.level),
			           C.GoString(mp_log.prefix),
			           C.GoString(mp_log.text))

		case "shutdown":
			p.Wait.Done()
			return
		}
	}
}
