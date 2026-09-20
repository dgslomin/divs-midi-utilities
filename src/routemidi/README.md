routemidi 1 "Oct 2022" routemidi "User Manual"
==================================================

# NAME
routemidi - connect, copy, merge, split streams from MIDI ports; establish virtual MIDI ports/busses


# SYNOPSIS
routemidi [ --bus | --in <port> | --out <port> | --virtual-in <name> | --virtual-out <name> | --channel <input bus number> <input channel number> <output bus number> <output channel number> ] ...


# DESCRIPTION
routemidi lets you define multiple busses, each of which reads from multiple input ports,
merges the streams together, and copies the result to multiple output ports.
Optionally MIDI channel numbers may be used for more complex routing
including the rewriting of the channel number, e.g. incoming messages for
MIDI channel 3 on bus 1 may be routed to bus 2 with MIDI channel changed to 5.

You can route channels freely between busses. You can also establish
virtual ports for other applications to connect to later. This allows you to provide a single port
name to configure in an application whose other side you can connect to different sources without
having to reconfigure the application. Another use is the ability to provide a
writeable port (e.g. for `aplaymidi(1)`) for an application that only supports reading from a MIDI port.

# GENERAL OPERATION
Once started, the program will keep running until terminated. It will set up at least 1 `bus`, more if the
`--bus` option is used. Each bus has a number of inputs (`--in` and `--virtual-in`) and a number
of outputs (`--out` and `--virtual-out`). Number of inputs and number of outputs are independent.

Whenever a MIDI message arrives on any input, it will be copied to ALL outputs on the corresponding bus.

If the `--channel` option is not used, the corresponding output bus is the same as the bus on whose input
the message was received. In this case the MIDI message is copied verbatim. If the `--channel` option is used and
the MIDI message is a channel message (such as Note On), the corresponding output bus is changed according to
the `--channel` option's `<output bus number>` and the message's channel field is changed to
`<output channel number>` before copying the message to all output ports on the corresponding output bus.

# OPTIONS
`--help`               Print usage and a list of detected ports, then exit.

`--bus`                Create a new bus. The position of this option on the command line is important.
                     All options following `--bus` on the command line will affect the new bus. `--bus` can be used
                     multiple times.
                     The first bus with number 0 is always created automatically, the bus created with the 1st
                     `--bus` option will have number 1.

`--in <port>`          `<port>` is the number or name of an existing MIDI port that will be read from, such as
                     the port on which key presses from a MIDI keyboard will come in. All MIDI messages appearing
                     on this port will be copied to all `--output` and `--virtual-output` ports of the corresponding
                     bus.

`--out <port>`         `<port>` is the number or name of an existing MIDI port that will be written to, such as
                     the port on which a synthesizer accepts notes to be played. All MIDI messages appearing
                     on any input port from the corresponding bus will be copied to <port>.

`--virtual-in <name>`  Creates a port called `<name>` on the current bus that can be written to, e.g. by
                     `aplaymidi(1)`. Note that <name> is not the identifier you would use to access the port
                     in an application like `aplaymidi(1)`.

`--virtual-out <name>` Creates a port called `<name>` on the current bus to which all inputs will be copied as if it
                     was a `--out` port. You can configure this port in an application that reads MIDI events, such
                     as `arecordmidi(1)`. Note that <name> is not the identifier you would use to access the port
                     in an application like `arecordmidi(1)`.

`--channel <input bus number> <input channel number> <output bus number> <output channel number>`
                     Establish a mapping based on MIDI channel number. Whenever a MIDI channel message
                     for channel `<input channel number>` (range 0..15) arrives on bus `<input bus number>`
                     (the default bus created by `routemidi` has number 0, the 1st bus created by `--bus`
                     has number 1), it will have its channel number changed to `<output bus number>`
                     and it will be copied to all outputs of bus `<output channel number>`.
                     The default mapping for bus B, channel C is to copy to the same bus B with channel
                     number C unchanged.

# EXAMPLES
```
routemidi --virtual-in WriteMIDI --virtual-out ReadMIDI
```

Sets up a simple bus with 1 input and 1 output that are not connected to
any existing MIDI devices or programs. While seemingly trivial this
is one of most useful applications of `routemidi(1)`. You can configure your
applications to read their MIDI data from the `ReadMIDI` port and connect
e.g. your MIDI instruments to WriteMIDI using `routemidi(1)` or
`aconnect(1)`.


# SEE ALSO
 aconnect(1),aplaymidi(1),arecordmidi(1)
