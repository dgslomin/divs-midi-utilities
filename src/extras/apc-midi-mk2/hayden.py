
import argparse
import time
import rtmidi

arg_parser = argparse.ArgumentParser(prog="hayden")
arg_parser.add_argument("--out", required=True)
args = arg_parser.parse_args()

apc_midi_in = rtmidi.MidiIn(name="hayden")

for (port_number, port_name) in enumerate(apc_midi_in.get_ports()):
	if port_name.startswith("APC mini mk2"):
		apc_midi_in.open_port(port_number)
		break
else:
	raise Exception("APC mini mk2 MIDI input port not found")

apc_midi_out = rtmidi.MidiOut(name="hayden")

for (port_number, port_name) in enumerate(apc_midi_out.get_ports()):
	if port_name.startswith("APC mini mk2"):
		apc_midi_out.open_port(port_number)
		break
else:
	raise Exception("APC mini mk2 MIDI output port not found")

midi_out = rtmidi.MidiOut(name="hayden")

for (port_number, port_name) in enumerate(midi_out.get_ports()):
	if str(port_number) == args.out or port_name.startswith(args.out):
		midi_out.open_port(port_number)
		break
else:
	raise Exception("MIDI output port not found")

lowest_note = 34

def get_pad_coordinates(pad_number):
	return (pad_number % 8, pad_number // 8)

def get_pad_number(x, y):
	return (y * 8) + x

def get_note(x, y):
	global lowest_note
	return lowest_note + (y * 5) + (x * 2)

def update_leds():
	for y in range(8):
		for x in range(8):
			pad_number = get_pad_number(x, y)
			note = get_note(x, y)

			if note < 0 or note > 127:
				brightness = 0x90
				color = 0
			elif note == 60:
				brightness = 0x96
				color = 24
			elif [True, False, True, False, True, True, False, True, False, True, False, True][note % 12]:
				brightness = 0x92
				color = 24
			else:
				brightness = 0x91
				color = 4

			apc_midi_out.send_message([brightness, pad_number, color])

def clear_leds():
	for y in range(8):
		for x in range(8):
			pad_number = get_pad_number(x, y)
			apc_midi_out.send_message([0x90, pad_number, 0])

update_leds()

def handle_midi_input(midi_event, data):
	global lowest_note

	(midi_message, delta_time) = midi_event

	if midi_message[0] == 0x80:
		if midi_message[1] < 0x64:
			(x, y) = get_pad_coordinates(midi_message[1])
			note = get_note(x, y)

			if note >= 0 and note < 128:
				midi_out.send_message([0x80, note, 127])

	elif midi_message[0] == 0x90:
		if midi_message[1] < 0x64:
			(x, y) = get_pad_coordinates(midi_message[1])
			note = get_note(x, y)

			if note >= 0 and note < 128:
				midi_out.send_message([0x90, note, 127])

		elif midi_message[1] == 0x68:
			lowest_note -= 5
			update_leds()
		elif midi_message[1] == 0x69:
			lowest_note += 5
			update_leds()
		elif midi_message[1] == 0x6A:
			lowest_note += 2
			update_leds()
		elif midi_message[1] == 0x6B:
			lowest_note -= 2
			update_leds()

apc_midi_in.set_callback(handle_midi_input)

try:
	while True:
		time.sleep(1)

except KeyboardInterrupt:
	pass

clear_leds()

