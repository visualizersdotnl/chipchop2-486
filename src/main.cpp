
/*
	Chip Chop #16 .
	The MS-DOS 486DX port by Space Operator / Megahawks INC.
	Original Amiga 500 version by Tim/TBL.

	Oldschool C-style rewrite (see OLD.CPP for previous ambitious C++ misfire).

	General notes:
	- This compiles with native OpenWatcom, which I've supplied in the repository.
	- Code style is a blend of 1990s and semi-modern: yes classes, no ASSERT.
	- This is one monstrous file, and yes, I should have split it up.
	- Compiler is babied a little for optimal code generation.

	To do:
	- Play *all* tracks flawlessly (including AHX), only then proceed to graphics.
	- AHX code: https://github.com/pete-gordon/hivelytracker/tree/master/Replayer_Windows
*/

// CRT:
#include <sys/types.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include <conio.h>

// Libs:
#include "midas/include/midasdll.h"
#include "minilzo/minilzo.h"

// Tracklist.
#include "tracks\tracks.h"

// --------------------------------------------------------------------------------------------------------------------
//
// 'SetLastError()'
//
// If text is set it will be displayed on exit along with return code 1.
//
// --------------------------------------------------------------------------------------------------------------------

static char s_lastErr[256] = { 0 };

// --------------------------------------------------------------------------------------------------------------------
//
// Audio (using MIDAS sound system).
//
// --------------------------------------------------------------------------------------------------------------------

static BOOL s_midasStarted = FALSE;
static MIDASmodule s_modules[kNumTracks] = { NULL }; // FIXME: tracks will become modules & streams (AHX).

// Set this to a path or anything to supply context to SetLastMIDASError().
static char s_midasErrorContext[256] = { 0 };

// Used by other functions to set meaningful MIDAS error message.
static void Audio_SetLastError()
{
	const int errNo = MIDASgetLastError();
	if (0 != errNo)
	{
		sprintf(s_lastErr, "MIDAS error (%s): %s", s_midasErrorContext, MIDASgetErrorMessage(errNo));
	}
}

// Used by Audio_Create().
static bool Audio_LoadTracks(const char *path)
{
	// Initialize MiniLZO (de)compressor.
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
		if (0 == MIDASgetLastError())
			sprintf(s_lastErr, "MIDAS configuration cancelled by user.");
		else
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
