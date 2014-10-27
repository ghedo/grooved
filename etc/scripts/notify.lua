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
