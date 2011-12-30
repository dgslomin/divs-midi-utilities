
import MidiFile

def sharp(diatonic): return diatonic + 0.5
def flat(diatonic): return diatonic - 0.5
def high(diatonic): return diatonic + 8
def low(diatonic): return diatonic - 8

class Turtle:
	def __init__(self, midi_file = None):
		if midi_file == None:
			self.midi_file = MidiFile.MidiFile()
		else:
			self.midi_file = midi_file

		self.base_note = 63
		self.velocity = 63
		self.beats_per_measure = 4.0
		self.time = 0.0
		self.track = 1
		self.channel = 0

	def n(self, beats = 1, *args):
		for diatonic in args:
			{1: 0, 1.5: 1, 2: 2, 2.5: 3, 3: 4, 4: 5, 4.5: 6, }
			(0, 1, 2, 3, 4, 4, 5, 7, 9, 11)[int(round((diatonic - 1.0) * 2.0))]
			self.base_note + diatonic
			pass

	def base(self, base_note):
		self.base_note = base_note

	def vel(self, velocity):
		self.velocity = velocity

	def beats(self, beats_per_measure):
		self.beats_per_measure = beats_per_measure

	def time(self, time):
		self.time = time

	def chan(self, channel):
		self.channel = channel

	def track(self, track):
		self.track = track

	self.ticks_per_measure

