
/*
	Chip Chop #16
	The MS-DOS 486DX port by Space Operator / Megahawks INC.
	Original Amiga 500 version by Tim/TBL.

	- This compiles with native OpenWatcom, which I've supplied in the repository.
	- Compiler is old, so I babied it a little to aid code generation.

	To do:
	- Play *all* tracks flawlessly (including AHX), only then proceed to graphics.
	- AHX code: https://github.com/pete-gordon/hivelytracker/tree/master/Replayer_Windows
*/

#include <sys/types.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
// #include <math.h>
// #include <conio.h>

#include "midas/include/midasdll.h"
#include "minilzo/minilzo.h"
#include "tracks\tracks.h"

#include "VGA.h"

// --------------------------------------------------------------------------------------------------------------------
//
// 'SetLastError()'
//
// If text is set on exit, it will be displayed and code 1 returned.
// This strategy works because practically any error in this disk is fatal.
//
// --------------------------------------------------------------------------------------------------------------------

static char s_lastErr[256] = { 0 };

// --------------------------------------------------------------------------------------------------------------------
//
// Misc. low-level stuff.
//
// --------------------------------------------------------------------------------------------------------------------

unsigned inp(unsigned port);
#pragma aux inp = "xor eax, eax", "in al,dx" parm [edx] value [eax] modify nomemory exact [eax];

void outp(unsigned port, unsigned value);
#pragma aux outp = "out dx,al" parm [edx] [eax] modify nomemory exact [];

void outpw(unsigned port, unsigned value);
#pragma aux outpw = "out dx,ax" parm [edx] [eax] modify nomemory exact [];

void _disable();
#pragma aux _disable = "cli" modify nomemory exact [];

void _enable();
#pragma aux _enable = "sti" modify nomemory exact [];

void SetDisplayMode(unsigned mode);
#pragma aux SetDisplayMode = \
	"mov ah, 0", \
	"mov al, 3", \
	"int 10h"    \
	parm [eax] modify nomemory exact [eax];

static size_t GetFreeMem()
{
	// First 4 bytes (gauranteed): amount of free memory.
	// The rest are optional DPMI fields (-1 means unavailable).
	uint8_t info[48];

	__asm
	{
		lea edi, [info]
		mov ax, 0500h
		int 31h
	}

	const size_t bytesFree = *(reinterpret_cast<size_t*>(info));
	return bytesFree;
}

// Keyboard codes.
#define KEY_ESC        27
#define KEY_ENTER      13
#define KEY_SPACE      32
#define KEY_SCAN_UP    72
#define KEY_SCAN_DOWN  80
#define KEY_SCAN_LEFT  75
#define KEY_SCAN_RIGHT 77

__inline bool IsScanCode(int key)
{
	return 0 == key || 224 == key;
}

// --------------------------------------------------------------------------------------------------------------------
//
// Video (320x240 8-bit Mode X).
//
// - This is a planar mode. 
// - VRAM is accessed through 4 80*240 planes covering every 1st, 2nd, 3rd and 4th pixel.
// - Unchained mode grants access to the full 256KB of VRAM, or 4 pages total.
// 
// For more information on this use Google or Abrash's black book.
// Or this: http://www.osdever.net/FreeVGA/vga/vga.htm
// 
// --------------------------------------------------------------------------------------------------------------------

// Resolution, sizes.
#define kResX   320
#define kResY   240
#define kPlaneX (kResX/4)
#define kPlaneY kResY
#define kPlane  (kPlaneX*kPlaneY)
#define kPage   kPlane

uint8_t* const g_pVRAM = (uint8_t *) 0xa0000;
uint8_t* g_pBack;
const uint8_t* g_pFront;

static void WaitVBL()
{
	while (0 != (inp(VGA_STATUS) & 0x08)) {} // Wait for VBLANK end.
	while (0 == (inp(VGA_STATUS) & 0x08)) {} // Wait for VBLANK begin.
}

// inline void SetFront

static void SetModeX()
{
	// We'll be modifying good old chunky 320x200.
	SetDisplayMode(0x13);

	// ...
	outp(SEQ_INDEX, SEQ_MEMORY_MODE);
	outp(SEQ_DATA, 0x06);

	// 
}

// --------------------------------------------------------------------------------------------------------------------
//
// Audio.
//
// --------------------------------------------------------------------------------------------------------------------

static BOOL s_midasStarted = FALSE;
static MIDASmodule s_modules[kNumTracks] = { NULL }

static void Audio_SetLastError()
{
	const int errNo = MIDASgetLastError();
	if (0 != errNo)
	{
		sprintf(s_lastErr, "[MIDAS error] %s", MIDASgetErrorMessage(errNo));
	}
}

// Used by Audio_Create().
static bool Audio_LoadTracks(const char* path)
{
	// Initialize (de)compressor.
    if (lzo_init() != LZO_E_OK)
    {
    	sprintf(s_lastErr, "Can't initialize MiniLZO.");
    	return false;
    }

    return true;
}

static bool Audio_Create()
{
	s_midasStarted = MIDASstartup();
	if (FALSE == s_midasStarted)
	{
		Audio_SetLastError();
		return false;
	}

	// Present MIDAS configuration screen.
	if (FALSE == MIDASconfig())
	{
		if (0 != MIDASgetLastError())
			Audio_SetLastError();

		return false;
	}

	// Approx. 70% separated, as recommended by Triace.
	if (FALSE == MIDASsetOption(MIDAS_OPTION_DEFAULT_STEREO_SEPARATION, 45)) 
	{
		Audio_SetLastError();
		return false;
	}

	if (false == Audio_LoadTracks("tracks.dat"))
		return false;

	return true;
}

static void Audio_Destroy()
{
	if (TRUE == s_midasStarted)
		MIDASclose();
}

// --------------------------------------------------------------------------------------------------------------------
//
// Main.
//
// --------------------------------------------------------------------------------------------------------------------

int main(int argC, char **argV)
{
	// Seed rand().
	srand(time(NULL));

	if (true == Audio_Create())
	{
	}

	Audio_Destroy();

	// Error?
	if (0 != strlen(s_lastErr))
	{
		printf(s_lastErr);
		printf("\n");
		return 1;
	}

	// Goodbye!
	printf("    ___   ____ ____    __    ____   ____         \n");
	printf("____\\  \\_/_  //   /___.\\_)._/_  /_./_  /____  \n");
	printf("\\    _  | '_/ \\___    |   | _/    | '_/    /   \n");
	printf(" \\   \\  |  \\   | /    |   | \\     |  \\    / \n");
	printf(" /______|______|______|___|__\\____|_______\     \n");
	printf("+-diP----------------------------------aSL-+     \n");
	printf("\n");
	printf("Chip Chop #16 by DESiRE.\n");
	printf("MS-DOS 486DX port by MEGAHAWKS INC.\n");
	printf("\n");
	printf("Remember kids: DOS does it better, and Accolade lives!\n");

	return 0;
}
