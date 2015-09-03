
/* 
	VGA IO ports & registers.
	Reference: http://www.osdever.net/FreeVGA/vga/vga.htm
*/

#pragma once

// Sequencer.
#define SEQ_INDEX 0x3c4	     
#define SEQ_DATA  0x3c5 

// CRTC.
#define CRTC_INDEX 0x3d4
#define CRTC_DATA  0x3d5

// Palette.
#define DAC_READ_INDEX  0x3c7
#define DAC_WRITE_INDEX 0x3c8
#define DAC_DATA        0x3c9

#define VGA_STATUS          0x3da // Status register (read).
#define VGA_FEATURE_CONTROL 0x3da // Feature control (write).
