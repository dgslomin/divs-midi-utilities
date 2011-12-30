
xmidiqwerty - an unusual utility/toy for X windows
Copyright 1998-2006 by David Slomin, All Rights Reserved.
Released under the BSD licence.

Chording computer keyboards have been available for a number of years.  
They are commonly used by people who have some sort of physical disability
in one or both of their hands, and by users of wearable computers.  The
concept behind chording keyboards is fairly simple: using less keys than
a conventional qwerty (or Dvorak) keyboard, you can still generate as many
or more keystrokes by allowing the user to press more than one key at a
time (not just the conventional Shift and Control keys).  Because the
number of combinations is factorially proportional to the number of keys, 
there tend to be many extra possible "keystrokes" which the user is allowed to 
map to arbitrary words or phrases instead of single characters.  This can allow
for a greater total typematic rate.

My contribution to this idea is to use a conventional MIDI keyboard for
this purpose instead of a custom computer keyboard.  I reasoned that
playing chords on a piano requires the identical coordination and motor
skills as using a chording keyboard.  Why not let pianists put their skills
to use?  A fairly large number of people already have MIDI keyboards connected
to their computers.

The program runs as a simple daemon that augments your normal qwerty (or 
Dvorak) keyboard rather than as a low-level driver that would replace it.  
This should make it much easier to install and use, as well as more portable.
It is implemented using the X-windows standard (but seldom used) synthetic 
keystroke feature.  (Try using the standard xev utility to see the difference 
between synthetic keystrokes and ones that come from the real keyboard.)

Unfortunately this program is incomplete, but does give you a preliminary
idea of what I had in mind.  The 26 notes starting with middle C map linearly
to the letters of the alphabet.  More infrastructure needed before chording
can work, and the .xmidiqwertyrc parser is yet to come.

