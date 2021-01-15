
#ifndef MIDIMSG_H
#define MIDIMSG_H

typedef unsigned char Byte;

#define MIDIMSG_NOTE_OFF 0x80
#define MIDIMSG_NOTE_ON 0x90
#define MIDIMSG_KEY_PRESSURE 0xa0
#define MIDIMSG_PARAMETER 0xb0
#define MIDIMSG_PROGRAM 0xc0
#define MIDIMSG_CHANNEL_PRESSURE 0xd0
#define MIDIMSG_PITCH_WHEEL 0xe0

#ifndef True
#define True  1
#define False 0
#endif

int  midimsgGetMessageType(Byte *message);
void midimsgSetMessageType(Byte *message_inout, int message_type);

int  midimsgGetChannel(Byte *message);
void midimsgSetChannel(Byte *message_inout, int channel);

int  midimsgGetPitch(Byte *message);
void midimsgSetPitch(Byte *message_inout, int pitch);

int  midimsgGetVelocity(Byte *message);
void midimsgSetVelocity(Byte *message_inout, int velocity);

int  midimsgGetParameterNumber(Byte *message);
void midimsgSetParameterNumber(Byte *message_inout, int number);

int  midimsgGetParameterValue(Byte *message);
void midimsgSetParameterValue(Byte *message_inout, int value);

int  midimsgGetPitchWheelValue(Byte *message);

void midimsgRead(int fd, Byte *message_return);
void midimsgWrite(int fd, Byte *message_return);

#endif

