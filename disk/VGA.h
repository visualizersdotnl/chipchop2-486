
/* 
	VGA IO ports & registers.

	For most of these the procedure is: select a register by writing to the component's index
	port (e.g. SEQ_INDEX), then read or write that register on the corresponding data port.

	Often you can select and write at once (16-bit out) as the ports are neighbours.

	Reference: http://www.osdever.net/FreeVGA/vga/vga.htm
*/

#pragma once

// Sequencer.
#define SEQ_INDEX 0x3c4	     
#define SEQ_DATA  0x3c5 

#define SEQ_MAP_MASK    0x02
#define SEQ_MEMORY_MODE 0x04

// CRTC.
#define CRTC_INDEX 0x3d4
#define CRTC_DATA  0x3d5

// Palette.
#define DAC_READ_INDEX  0x3c7
#define DAC_WRITE_INDEX 0x3c8
#define DAC_DATA        0x3c9

#define VGA_STATUS          0x3da // Read: status register.
#define VGA_FEATURE_CONTROL 0x3da // Write: feature control.
