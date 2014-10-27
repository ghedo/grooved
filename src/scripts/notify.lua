--
-- Groovy music player daemon.
--
-- Copyright (c) 2014, Alessandro Ghedini
-- All rights reserved.
--
-- Redistribution and use in source and binary forms, with or without
-- modification, are permitted provided that the following conditions are
-- met:
--
--     * Redistributions of source code must retain the above copyright
--       notice, this list of conditions and the following disclaimer.
--
--     * Redistributions in binary form must reproduce the above copyright
--       notice, this list of conditions and the following disclaimer in the
--       documentation and/or other materials provided with the distribution.
--
-- THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS
-- IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
-- THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
-- PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR
-- CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
-- EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
-- PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
-- PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
-- LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
-- NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
-- SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

local lgi  = require 'lgi'
local Gio  = lgi.require 'Gio'
local GLib = lgi.require 'GLib'

function notify()
	local bus      = Gio.bus_get_sync(Gio.BusType.SESSION)
	local builder  = GLib.VariantBuilder(GLib.VariantType.TUPLE)
	local metadata = mp.get_property_native('metadata')

	local title  = metadata['title'] or mp.get_property_native('media-title')
	local artist = metadata['artist']

	if artist ~= nil then
		msg = artist .. ' - ' .. title
	else
		msg = title
	end

	builder:add_value(GLib.Variant('s', 'grooved'))
	builder:add_value(GLib.Variant('u', 1))
	builder:add_value(GLib.Variant('s', 'media-playback-start'))
	builder:add_value(GLib.Variant('s', 'Now Playing:'))
	builder:add_value(GLib.Variant('s', msg))
	builder:add_value(GLib.VariantBuilder(GLib.VariantType.STRING_ARRAY):_end())
	builder:add_value(GLib.VariantBuilder(GLib.VariantType.new('a{sv}')):_end())
	builder:add_value(GLib.Variant('i', -1))

	local var, err = bus:call_sync(
		'org.freedesktop.Notifications',
		'/org/freedesktop/Notifications',
		'org.freedesktop.Notifications',
		'Notify',
		builder:_end(),
		nil,
		Gio.DBusConnectionFlags.NONE,
		-1
	)
end

mp.register_event("metadata-update", notify)
