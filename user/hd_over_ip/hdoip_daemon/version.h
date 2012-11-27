#ifndef VERSION_H_
#define VERSION_H_

/* [31 .. 16] = decimal number; [15 .. 0] = after comma number */
#define VERSION_SOFTWARE        0x00020002
#define VERSION_MAJOR		((VERSION_SOFTWARE >> 16) & 0xFFFF)
#define VERSION_MINOR		(VERSION_SOFTWARE & 0xFFFF)

// version tag (max. 50 letters)
#define VERSION_TAG             "audio-video delay"

#endif /* VERSION_H_ */
