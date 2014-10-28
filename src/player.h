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

#include <stdint.h>
#include <stdbool.h>

#include <gio/gio.h>

enum loop {
	PLAYER_LOOP_TRACK,
	PLAYER_LOOP_LIST,
	PLAYER_LOOP_NONE
};

extern void player_init(void);

extern const char *player_error_string(int error);

extern void player_make_list(GVariantBuilder *list);
extern void player_make_metadata(GVariantBuilder *metadata);

extern char *player_make_media_title(void);

extern int player_playback_play(void);
extern int player_playback_pause(void);
extern int player_playback_toggle(void);
extern int player_playback_stop(void);

extern int player_playback_seek(int64_t secs);

extern int player_playback_loop(enum loop mode);

extern char *player_playback_status_string(void);
extern char *player_loop_status_string(void);

extern char *player_playback_track_path(void);
extern double player_playback_track_length(void);
extern double player_playback_track_position_time(void);
extern double player_playback_track_position_percent(void);

extern int player_playlist_append_file(const char *path, bool play);
extern int player_playlist_append_list(const char *path);
extern int player_playlist_goto_index(int64_t index);
extern int player_playlist_remove_index(int64_t index);
extern int player_playlist_next(void);
extern int player_playlist_prev(void);

extern int64_t player_playlist_count(void);
extern int64_t player_playlist_position(void);
