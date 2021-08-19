
/*
	Chip Chop #16 (or #2 in the modern age).
	The MS-DOS 486 port by Space Operator / Megahawks INC.

	Original Amiga 500 version by Tim/TBL.

	---------------------------------------------------------------------------------------------------------------------------------------------------------------

	NEWER NOTES (19/08/2021):

	I'm going to rush this to the finish line, ugly or not... -> Done!
	I'll have it tested on a real 486, which I do not have anymore..

	- Clean up!
	- Private repository!
	- C2P: storing data differently is the entire key to success!

	---------------------------------------------------------------------------------------------------------------------------------------------------------------

	NEW NOTES:

	I could try to make this look presentable but there's so much wrong with the approach to most problems
	that it's probably not worth it.

	Best just keep it as a toy to play around with when you've had a few drinks.
	For a serious attempt at a music disk on 486 code a new one!

	---------------------------------------------------------------------------------------------------------------------------------------------------------------

	General notes:
	- This compiles with native OpenWatcom, which I've supplied in the repository.
	- Code style is a blend of 1990s and semi-modern: yes classes, no ASSERT.
	- This is one monstrous file, and yes, I should have split it up.

	- Compiler is babied (verbose expression) to try and be sure nothing ugly happens.
	  + I often use #define in places you'd nowadays expect 'const <type> kConstant'.
	  + Watcom does not accept inline assembler in class member functions, something I figured out a little late.

	- There are plenty hardcoded values around (especially in the parts), so remember:	
	  + The 320x240 resolution.
	  + The 320/4, so 80, plane width (ModeX).
	  + That we have 256 colors (but use far less).
	  + Color #0 (kBorder) is always reserved (black).
	  + Draw functions postfixed with an 'X' have special constraints.
	  + VRAM overdraw or non-sequential writes are avoided like the plague.
	  + Timing is handled in seconds (floating point) and converted to integers (mostly 0-63, 6-bit precision).

	To-do:

	- Priority:
	  + Solve the "ensure first two frames are consistent" problem.
	  + Read up on those latches (Abrash) and check what it can do for me (I have 1-2 full buffers left for off-screen storage) -> Not necessary now.
	  + Fix multiple shades for separate font draws.
	  + Analyze generated code -> Maybe, some day.
	  + Fix argument length issue in WMAKE -> What?
	  + Reduce memory footprint by not keeping the entire track archive in memory on load -> Doesn't matter, you'll peak there anyway.
	  + Speed up C2P strategies -> See above.
	
	- AHX replay.
	- Abbreviate track names so they fit within the arrows (look at Tim's version) -> ?
	- Some functions have 320 pixels wide constraints (keep most for speed, add non-restricted when needed).

	Possible optimizations:
	- Use IDA & DOSBox debugger (under Windows) to analyze.
	- Palette updates can be done partially (which covers most cases where it could matter).
	- In heavy parts, see what can be stored in and blitted from off-screen VRAM.
	- Sacrifice RAM for precalculation.
	- Use the ASM hammer.

	Bugs and issues:
	- GUS crashes (in DOSBox at least) still there?
	- Flicker bug on return to text mode (both in DOSBox OSX and PC): FIX!

	Keep an eye on:
	- Memory req. and statistics (printed on exit in DEVELOPMENT_MODE).
	- File size.
	- Flicker artifacts:
	  + Keep kTotalColors as small as possible (defined just below).
	  + Keep palette color #0 black (kBorder).
	  + No palette fiddling outside of the VBLANK period.
	- FIXMEs.

	Eventually:
	- Supply DOS extender along with release.
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

// Undef. to load from/as release content.
#define DEVELOPMENT_MODE 

// Def. to dump graphics to embedded data container:
// - Required to build a version that runs without DEVELOPMENT_MODE.
// - Only works in DEVELOPMENT_MODE.
// #define DUMP_C_DATA 

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

// Megabytes of RAM used by OS environment, extender, packer (UPX adds approx. 200KB) & executable.
#define kMegsEnvAndEXE 2

// Megabytes of RAM required total.
#define kMegsRequired 4

// Fixed border color.
#define kBorder 0                
#define kColorOffs (kBorder+1) // First available color.

// Total amount of colors used (limits the DAC upload during VBLANK, important).
#define kTotalColors 70

// Temporary file to dump & load for MIDAS.
#define kModuleTemp "CC2TEMP"

// MiniLZO uncompress buffer (temporarily allocated while loading modules).
// Shoud be about the size of the largest one (reported by track datafile builder).
#define kLargestModuleSize 237065

// --------------------------------------------------------------------------------------------------------------------
//
// "SetLastError"
//
// If text is set it will be displayed on exit along with return code 1.
//
// --------------------------------------------------------------------------------------------------------------------

static char s_lastErr[256] = { 0 };

// --------------------------------------------------------------------------------------------------------------------
//
// System functions.
//
// --------------------------------------------------------------------------------------------------------------------

// Byte port I/O aliases.
#define outpb outp
#define inpb  inp

// Disable all interrupts.
void CLI();
#pragma aux CLI = "cli" modify nomemory exact []

// Enable all interrupts.
void STI();
#pragma aux STI = "sti" modify nomemory exact []

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

	const size_t bytesFree = *(reinterpret_cast<size_t *>(info));
	return bytesFree;
}

// --------------------------------------------------------------------------------------------------------------------
//
// Some floating point math (for timing).
//
// --------------------------------------------------------------------------------------------------------------------

__inline float saturatef(float value)
{
	if (value < 0.f) value = 0.f;
	if (value > 1.f) value = 1.f;
	return value;
}

__inline float lerpf(float A, float B, float T)
{
	return A*(1.f-T) + B*T;
}

__inline float smoothstepf(float A, float B, float T)
{
	const float X = saturatef(T);
	return lerpf(A, B, X*X*X * (X*(X*6.f - 15.f) + 10.f));
}

__inline float rampf(float A, float B, float T)
{
	const float X = T*T;
	return lerpf(A, B, X);
}

// Convert floating point value [0, 1] (clamped) to 6-bit unsigned integer [0, 63].
// Fits exactly for palette and audio fading.
__inline unsigned int fto6(float value)
{
	value = 63.f*saturatef(value);
	return (unsigned) value;
}

// --------------------------------------------------------------------------------------------------------------------
//
// Simple file I/O.
//
// --------------------------------------------------------------------------------------------------------------------

static uint8_t *ReadFile(const char *path, bool binary, size_t &numBytesRead)
{
	FILE *hFile = fopen(path, (true == binary) ? "rb" : "r");
	if (NULL != hFile)
	{
		fseek(hFile, 0L, SEEK_END);
		const size_t numBytes = ftell(hFile);
		fseek(hFile, 0L, SEEK_SET);

		uint8_t *pData = new uint8_t[numBytes];	
		numBytesRead = fread(pData, sizeof(char), numBytes, hFile);

		// Since I don't have a predetermined size any output from fread() is acceptable,
		// and I'm assuming it will give me exactly what I'm just told through ftell().
		fclose(hFile);

		return pData;
	}

	sprintf(s_lastErr, "Can not load file: %s", path);
	
	numBytesRead = 0;
	return NULL;
}

static bool WriteFile(const char *path, bool binary, const void *pSrc, size_t numBytes)
{
	FILE *hFile = fopen(path, (binary) ? "wb" : "w");
	if (NULL != hFile)
	{
		const size_t numBytesWritten = fwrite(pSrc, sizeof(char), numBytes, hFile);
		fclose(hFile);
		return numBytesWritten == numBytes;			
	}

	sprintf(s_lastErr, "Can not write %u bytes to file: %s", numBytes, path);
	return false;
}

__inline const char *StripPath(const char *path)
{
	// Assumes we're always stripping 'graphics\' (raw graphics data path, FIXME).
	return path+9;
}

// --------------------------------------------------------------------------------------------------------------------
//
// VGA I/O ports (VP_) & registers (VR_).
//
// Proper documentation on this can be found here: 
// - http://www.osdever.net/FreeVGA/vga/portidx.htm (all about VGA).
// - http://www.drdobbs.com/parallel/graphics-programming-black-book/184404919 (Michael Abrash's black book).
//
// --------------------------------------------------------------------------------------------------------------------

// CRT controller:
#define VP_CRTC_ADDR 0x3d4
#define VP_CRTC_DATA 0x3d5	

// Sequencer:
#define VP_SEQ_ADDR 0x3c4	     
#define VP_SEQ_DATA 0x3c5 

#define VR_SEQ_MAP_MASK    0x02
#define VR_SEQ_MEMORY_MODE 0x04

// Misc. (single register, write only):
#define VP_MISC_READ_DATA  0x3cc
#define VP_MISC_WRITE_DATA 0x3c2

// DAC (256 colors 6-bit RGB palette):
#define VP_DAC_READ_ADDR   0x3c7
#define VP_DAC_WRITE_ADDR  0x3c8
#define VP_DAC_DATA        0x3c9

// Input status (read) & feature control (write).
#define VP_STATUS_ADDR  0x3da
#define VP_FEATURE_ADDR 0x3da

uint8_t *s_pVRAM = (uint8_t *) 0xa0000;

__inline void VGA_WaitForVBLANK()
{
	// First wait for VBLANK to end, ideally this won't take ages.
	while (0 != (inpb(VP_STATUS_ADDR) & 0x08)) {}

	// And now wait for a new VBLANK to begin!
	while (0 == (inpb(VP_STATUS_ADDR) & 0x08)) {}
}

static void VGA_UploadPalette(const uint8_t *pPalette)
{
	__asm
	{
		xor eax, eax
		mov esi, [pPalette]
		mov dx, VP_DAC_WRITE_ADDR
		mov ecx, kTotalColors*3 // 768
		out dx, al
		inc dx // VP_DAC_DATA
		rep outsb
	}
}

// --------------------------------------------------------------------------------------------------------------------
//
// VGA 320x240 (planar, ModeX).
//
// Important:
// - Each page (or buffer if you will) is 320*240 bytes, *but*, you're writing to 80*240 planes each covering 1 pixel.
// - Use VGA_ModeX_SetPlanes() or VGA_ModeX_SetPlane() to direct your writes to any combination of the 4 planes.
// - Handle page-flipping yourself (in this case, implemented in the section below).
//
// --------------------------------------------------------------------------------------------------------------------

// ModeX resolution constants.
#define kResX 320
#define kResY 240
#define kPlaneW (kResX/4)
#define kPlaneSize (kPlaneW*kResY)
#define kPageSize kPlaneSize

// VRAM buffer pointers.
uint8_t *g_pWrite;
const uint8_t *g_pFront;

// Local palette.
// Flag as dirty to upload for next frame (only uploads kTotalColors, see above).
static bool s_paletteDirty;
static uint8_t s_palette[768];

// Offset is per 4 pixels to address 256KB (kPageSize).
__inline void VGA_ModeX_SetWritePage(unsigned int offset)
{
	g_pWrite = s_pVRAM+offset;
}

// Offset is per 4 pixels to address 256KB (kPageSize).
__inline void VGA_ModeX_SetFrontPage(unsigned int offset)
{
	g_pFront = s_pVRAM+offset;

	outpw(VP_CRTC_ADDR, offset & 0xff00 | 0x0c);
	outpw(VP_CRTC_ADDR, offset &   0xff | 0x0d);
}

// Set plane write mask (4-bit).
__inline void VGA_ModeX_SetPlanes(unsigned int mask)
{
	outpw(VP_SEQ_ADDR, mask<<8 | VR_SEQ_MAP_MASK);
}

// Select a single plane for write.
__inline void VGA_ModeX_SetPlane(unsigned int iX)
{
	iX &= 0x3;
	VGA_ModeX_SetPlanes(1<<iX);
}

// Clear current page.
__inline void VGA_ModeX_Clear(unsigned int color = 0)
{
	VGA_ModeX_SetPlanes(0x0f);
	memset(g_pWrite, color, kPageSize);
}

// 0x3  = Standard DOS text mode.
// 0x13 = MCGA.
static void SetVideoMode(unsigned int mode)
{
	__asm
	{
		mov eax, [mode]
		int 10h
	}
}

// 320*240, planar, 4 buffers (4*64KB) unlocked.
// Setup implemented after Michael Abrash's example in his Dr. Dobbs article.
static void SetVideoModeX() 
{
	// We'll be modifying MCGA mode.
	SetVideoMode(0x13);

	// Disable Chain-4 mode, enabling 256KB memory with sequential access.
	outpb(VP_SEQ_ADDR, VR_SEQ_MEMORY_MODE);
	outpb(VP_SEQ_DATA, 0x06);

	// Order sequencer to synchronously clear & halt.
	outpw(VP_SEQ_ADDR, 0x0100);

	// Select 25MHz clock / 60Hz refresh (neg. sync. polarity).
	outpb(VP_MISC_WRITE_DATA, 0xe3);

	// Restart sequencer.
	outpw(VP_SEQ_ADDR, 0x0300);

	// Turn off CRTC write protect.
	outpb(VP_CRTC_ADDR, 0x11);
	const uint8_t vSyncEnd = inpb(VP_CRTC_DATA);
	outpb(VP_CRTC_DATA, vSyncEnd & 0x7f);

	// Registers to modify for our mode, directly taken from Abrash's setup code.
	// Want to know exactly what? Check the links I supplied.
	// I spent more than 5 minutes, then realized it wasn't 1995 anymore.
	outpw(VP_CRTC_ADDR, 0x0d06); // Vertical total.
	outpw(VP_CRTC_ADDR, 0x3e07); // Overflow bits (bit 8 of vertical counts).
	outpw(VP_CRTC_ADDR, 0x4109); // Cell height (double scan).
	outpw(VP_CRTC_ADDR, 0xea10); // VSYNC start.
	outpw(VP_CRTC_ADDR, 0xac11); // VSYNC end + write protect. 
	outpw(VP_CRTC_ADDR, 0xdf12); // Vertical displayed.
	outpw(VP_CRTC_ADDR, 0x0014); // Turn off DWORD mode.
	outpw(VP_CRTC_ADDR, 0xe715); // VBLANK start.
	outpw(VP_CRTC_ADDR, 0x0616); // VBLANK end.
	outpw(VP_CRTC_ADDR, 0xe317); // Turn on byte mode.

	// Clear VRAM.
	outpw(VP_SEQ_ADDR, 0x0f<<8 | VR_SEQ_MAP_MASK);
	memset(s_pVRAM, 0, 65536);

	// Erase local palette.
	memset(s_palette, 0, 768);

	// Wait for VBLANK, then:
	// - Upload palette.
	// - Select write & display page.
	VGA_WaitForVBLANK();

	VGA_UploadPalette(s_palette);
	s_paletteDirty = false;

	VGA_ModeX_SetWritePage(0);
	VGA_ModeX_SetFrontPage(kPageSize);
}

// --------------------------------------------------------------------------------------------------------------------
//
// Audio & VGA: MIDAS timer callbacks.
//
// This is a reliable (60Hz) timing mechanism, tied to VGA sync.
//
// MIDAS offers 3 callbacks, but I've so far found that just using the first one works best (no artifacts).
// This would be different if I "locked" all volatile variables in the first call, but this way it's just easier.
//
// VGA ModeX functions are used to implement page-flipping and palette update.
//
// Timer g_runTime is *always* updated at 60Hz, so use it.
// Also, be sure to explicitly indicate when a frame (page) is ready to be shown!
// 
// --------------------------------------------------------------------------------------------------------------------

#define kTimeStep (1.f/60.f) // 60Hz.

volatile unsigned int g_frameCount = 0;
volatile float g_runTime = 0.f;

volatile int g_frameIssued = -1;

#ifdef DEVELOPMENT_MODE
static unsigned int s_totalFramesDropped = 0;
#endif

static void MIDAS_CALL MIDAS_PreVR()
{	
	// Make a local copy of volatile, then:
	const int frameIssuedCopy = g_frameIssued;
	if (frameIssuedCopy >= 0)
	{
		{
			// Made it: increase frame counter.
			const unsigned int frameCount = ++g_frameCount;

			if (true == s_paletteDirty)
			{
				VGA_UploadPalette(s_palette);
				s_paletteDirty = false;
			}

			if (frameIssuedCopy > 0)
			{
				// Double buffering.
				static const uint16_t pages[2] = { 0, kPageSize };
				const unsigned int iPage = frameCount & 1;
				VGA_ModeX_SetWritePage(pages[iPage]);
				VGA_ModeX_SetFrontPage(pages[!iPage]);
			}

			g_frameIssued = -1;
		}
	}
#ifdef DEVELOPMENT_MODE
	else
	{
		// Frame dropped.
		++s_totalFramesDropped;
	}
#endif

	// Time always goes on.
	g_runTime += kTimeStep;
}

// static void MIDAS_CALL MIDAS_ImmVR() {}
// static void MIDAS_CALL MIDAS_InVR() {}

// Frame done: do not swap buffer (unchanged) but check for palette update(s).
__inline void MIDAS_ModeX_Cycle() { g_frameIssued = 0; }

// Frame done: swap buffer & check for palette update(s).
__inline void MIDAS_ModeX_Flip()  { g_frameIssued = 1; }

// --------------------------------------------------------------------------------------------------------------------
//
// Audio.
//
// --------------------------------------------------------------------------------------------------------------------

// Track list.
#include "tracks/tracks.h"

static BOOL s_midasStarted = FALSE;
static MIDASmodule s_modules[kNumTracks] = { NULL };

// Grabbed after switching to VGA mode.
// Our ModeX is supposed to be exactly 60Hz but especially in emulators the exact rate deviates.
static unsigned int s_midasRefresh = -1;

// Current module play instance.
static MIDASmodulePlayHandle s_modulePlay = 0;

// Set this to a path or anything to supply context to SetLastMIDASError().
static char s_midasErrorContext[256] = { 0 };

static void SetLastMIDASError()
{
	const int errNo = MIDASgetLastError();
	if (0 != errNo)
	{
		sprintf(s_lastErr, "MIDAS error (%s): %s", s_midasErrorContext, MIDASgetErrorMessage(errNo));
	}
}

static MIDASmodule Audio_LoadModule(const char *name)
{
	MIDASmodule module = MIDASloadModule(kModuleTemp);
	if (NULL == module)
	{
		strcpy(s_midasErrorContext, name);
		SetLastMIDASError();
		return NULL;
	}

	return module;
}

// Call before switching display mode; starts MIDAS and handles user setup.
static bool Audio_Config()
{
	s_midasStarted = MIDASstartup();
	if (FALSE == s_midasStarted)
	{
		SetLastMIDASError();
		return false;
	}

	// Present MIDAS configuration screen.
	if (FALSE == MIDASconfig())
	{
		if (0 == MIDASgetLastError())
			sprintf(s_lastErr, "MIDAS configuration cancelled by user.");
		else
			SetLastMIDASError();

		return false;
	}

	return true;
}

// Call after switching to VGA display mode.
static bool Audio_Start()
{
	// This should return approx. 60Hz (like our ModeX).
	// By the (old) text I'm told to do this *before* what follows.
	s_midasRefresh = MIDASgetDisplayRefreshRate();

	if (FALSE == MIDASinit())
	{
		SetLastMIDASError();
		return false;
	}

	// Approx. 70% separated, as recommended by Triace.
	if (FALSE == MIDASsetOption(MIDAS_OPTION_DEFAULT_STEREO_SEPARATION, 45)) 
	{
		SetLastMIDASError();
		return false;
	}

	// Initialize MiniLZO (de)compressor.
    if (lzo_init() != LZO_E_OK)
    {
    	sprintf(s_lastErr, "Can't initialize MiniLZO.");
    	return false;
    }

    // Allocate uncompress buffer.
	uint8_t *pUnc = new uint8_t[kLargestModuleSize];

	// Here we'll uncompress and load from the archive.
	// It's a hack that writes a temporary file to disk, but I wanted to keep MIDAS (and was too lazy to rebuild it).
	// FIXME: it's an easy fix to save memory here by loading the table and seeking through the archive.
	size_t numBytesRead;
	uint8_t *pModArch = ReadFile("tracks.dat", true, numBytesRead);

	if (NULL != pModArch)
	{
		// Module data is preceded by a table.
		struct Table
		{
			uint32_t offset;
			uint32_t size;
		};

		const Table *pTab = reinterpret_cast<Table *>(pModArch);

		// Load all modules into RAM.
		for (size_t iModule = 0; iModule < kNumTracks; ++iModule)
		{
			// Grab name, offset & size.
			const char *modContext = s_tracks[(iModule*3)+2];
			const uint32_t offset = pTab[iModule].offset;
			const uint32_t size = pTab[iModule].size;

			// Uncompress.
			lzo_uint uncSize;
			const int result = lzo1x_decompress(pModArch+offset, size, pUnc, &uncSize, NULL);
			if (LZO_E_OK != result)
			{
				sprintf(s_lastErr, "Error decompressing module (%s) from 'tracks.dat'.", modContext);
				
				delete[] pUnc;
				delete[] pModArch;
				return false;
			}

			// Write to temporary file.
			if (true == WriteFile(kModuleTemp, true, pUnc, uncSize))
			{
				// Then load.
				MIDASmodule module;
				module = Audio_LoadModule(modContext);
				if (NULL == module)
				{
					delete[] pUnc;
					delete[] pModArch;
					return false;
				}

				s_modules[iModule] = module;
			}
		}
	}
	else
	{
		// Archive isn't there?
		return false;
	}

	// Ditch temporary buffers.
	delete[] pUnc;
	delete[] pModArch;

	// Remove temp. file.
	remove(kModuleTemp);

	// Set timer (tied with VGA) callbacks.
	if (FALSE == MIDASsetTimerCallbacks(s_midasRefresh, TRUE, &MIDAS_PreVR, NULL, NULL))
//	if (FALSE == MIDASsetTimerCallbacks(s_midasRefresh, TRUE, &MIDAS_PreVR, &MIDAS_ImmVR, &MIDAS_InVR))
	{
		SetLastMIDASError();
		return false;
	}

	// Start Accolade track (TD2, 19/8/2021).
	s_modulePlay = MIDASplayModule(s_modules[kNumTracks-1], TRUE);
	if (0 == s_modulePlay)
	{
		SetLastMIDASError();
		return false;
	}

	return true;
}

// Call before returning switching back video mode.
static void Audio_Stop()
{
	if (0 != s_modulePlay)
	{
		MIDASsetMusicVolume(s_modulePlay, 0); // Just to be safe.
		MIDASstopModule(s_modulePlay);
		s_modulePlay = 0;
	}

	// Remove timer callbacks.
	MIDASremoveTimerCallbacks();
}

// Call before exit.
static void Audio_Release()
{
	for (size_t iModule = 0; iModule < kNumTracks-1; ++iModule)
	{
		if (NULL != s_modules[iModule])
			MIDASfreeModule(s_modules[iModule]);
	}

	// Remove temp. file.
//	remove(kModuleTemp);

	if (TRUE == s_midasStarted)
		MIDASclose();
}

static void Audio_SetVolume(unsigned int volume)
{
	// MIDAS full volume is 64 instead of 63, but I use a 6-bit range for this (FIXME).
	MIDASsetMusicVolume(s_modulePlay, volume+1);
}

static void Audio_SelectTrack(unsigned int iTrack)
{
	if (0 != s_modulePlay)
	{
		MIDASstopModule(s_modulePlay);
	}

	s_modulePlay = MIDASplayModule(s_modules[iTrack], TRUE);
}

// --------------------------------------------------------------------------------------------------------------------
//
// Data-to-C conversion, for all things (to be) embedded in the executable.
//
// --------------------------------------------------------------------------------------------------------------------

#if defined(DEVELOPMENT_MODE) && defined(DUMP_C_DATA)

#include <iomanip>
#include <fstream>

// This class builds header files (BIN2H-style) for assets and adds them to a global container (DATA.CPP+H).
// This is used to convert any (graphics) data during development to be included inside the executable for release.
class DataCPP
{
public:
	DataCPP()
	{
		printf("[DUMP_C_DATA] Opening new DATA.CPP+H for write.\n");

		m_C.open("data.cpp");
		m_C << std::endl;
		m_C << "// ** GENERATED **" << std::endl;
		m_C << "#include <sys/types.h>" << std::endl;
		m_C << std::endl;

		m_H.open("data.h");
		m_H << std::endl;
		m_H << "// ** GENERATED **" << std::endl;
		m_H << "#pragma once" << std::endl;
		m_H << std::endl;
	}

	~DataCPP()
	{
		m_C.close();
		m_H.close();

		printf("[DUMP_C_DATA] Closed new DATA.CPP+H.\n");
	}

private:
	// Add a local include to a header containing data (BIN2H-style) to the CPP.
	void AddBin(const char *headerPath)
	{
		m_C << "#include \"" << headerPath << '"' << std::endl;
	}

	// Add an external ref. to header.
	void AddRef(const char *declaration)
	{
		m_H << "extern " << declaration << ";" << std::endl;
	}

	// Add a #define to header.
	void AddDef(const char *define)
	{
		m_H << "#define " << define << std::endl;
	}

	// Add arbitrary line to header.
	void AddLine(const char *line)
	{
		m_H << line << std::endl;
	}

	static void Bin2H(std::ofstream &stream, const char *declaration, const uint8_t *pData, size_t size)
	{
		stream << declaration << " = {";

		stream << std::hex << std::setfill('0');
		{
			for (size_t iByte = 0; iByte < size; ++iByte)
			{
				if (0 == iByte % 24) // Space indent every 24 bytes.
				{
					stream << std::endl;
					stream << "   ";
				}

				stream << "0x" << std::setw(2) << (unsigned) pData[iByte];
				if (iByte < size-1) stream << ", ";
				else stream << std::endl;
			}
		}
		stream << std::dec;

		stream << "};" << std::endl;
		stream << std::endl;
	}

public:
	// - Generates separate BIN2H-style header in asset's path.
	// - Adds it to the global container.
	void AddBitmap(const char *path, const char *name, const uint8_t *pBitmap, size_t bitmapSize, const uint8_t *pPalette, size_t palSize, const char *instance)
	{
		char headerPath[256];
		sprintf(headerPath, "%s.h", path);

		std::ofstream header;
		header.open(headerPath);

		// Header.
		header << std::endl;
		header << "// Generated for bitmap: " << path << std::endl;
		header << "#pragma once" << std::endl;
		header << std::endl;

		// Dump binaries.
		char declaration[256];
		sprintf(declaration, "uint8_t g_bitmap_%s[]", name);
		Bin2H(header, declaration, pBitmap, bitmapSize);
//		AddRef(declaration);

		if (NULL != pPalette)
		{
			sprintf(declaration, "uint8_t g_palette_%s[]", name);
			Bin2H(header, declaration, pPalette, palSize);
//			AddRef(declaration);
		}

		// Add (global) instance.
		AddLine(instance);
		AddLine(""); // And blank one to separate.

		header.close();

		// Add to container.
		AddBin(headerPath);
	}

private:
	std::ofstream m_C, m_H;
} static s_dataCPP;

#endif // defined(DEVELOPMENT_MODE) && defined(DUMP_C_DATA)

// --------------------------------------------------------------------------------------------------------------------
//
// VGA palette conversion & fading.
//
// --------------------------------------------------------------------------------------------------------------------

// 8-bit to 6-bit (DAC).
static void PAL_826(uint8_t *pComponents, unsigned int palSize)
{
	for (unsigned int iComp = 0; iComp < palSize; ++iComp)
	{
		// Simply drop 2 bits.
		*pComponents++ = *pComponents >> 2;
	}
}

// Calculate delta values (64) to fade from or to a base color (supply as 6-bit RGB).
static int *PAL_CalculateFadeTableFrom(const uint8_t *pSrc, unsigned int palSize, const uint8_t *pRGB)
{
	int *pTable = new int[palSize];

	for (unsigned int iComp = 0; iComp < palSize; ++iComp)
	{
		const int toTint = pRGB[iComp%3];
		const int tint = *pSrc++;
		const int delta = ((tint-toTint) << 6) >> 6;
		pTable[iComp] = delta;
	}

	return pTable;
}

static int *PAL_CalculateFadeTableFromBlack(const uint8_t *pSrc, unsigned int palSize)
{
	const uint8_t black[3] = { 0, 0, 0 };
	return PAL_CalculateFadeTableFrom(pSrc, palSize, black);
}

static int *PAL_CalculateFadeTable(const uint8_t *pSrc, unsigned int palSize, const uint8_t *pRGB)
{
	if (NULL == pRGB)
		return PAL_CalculateFadeTableFromBlack(pSrc, palSize);
	else
		return PAL_CalculateFadeTableFrom(pSrc, palSize, pRGB);
}

// Opacity is a 6-bit value, 63 = 1.
__inline void PAL_Fade(uint8_t *pDest, const uint8_t *pSrc, const int *pTable, unsigned int palSize, unsigned int opacity)
{
	while (palSize--)
	{
		*pDest++ = (*pSrc++ + (*pTable++ * opacity)) >> 6;
	}
}

// --------------------------------------------------------------------------------------------------------------------
//
// Raw 8-bit planar image container (.ACT color table & raw, exported by Photoshop CS6).
// Width must be a multiple of 4 for conversion to planar.
//
// --------------------------------------------------------------------------------------------------------------------

class Image
{
public:
	// Constructor to load from memory (release).
	Image(uint8_t *pPalette, uint8_t *pImage,
	      unsigned int xRes, unsigned int yRes, unsigned int numColors, unsigned int palOffs) : 
	      m_path(""), m_xRes(xRes), m_yRes(yRes), m_palSize(numColors*3), m_palOffs(palOffs)
,	      m_pPalette(pPalette), m_pImage(pImage)
,	      m_xRes4(xRes>>2), m_planeSize((xRes>>2)*yRes)
,	      m_pFadeTab(PAL_CalculateFadeTable(m_pPalette, m_palSize, NULL))
	{
		// Expects processed data (DEVELOPMENT_MODE).
	}

#ifdef DEVELOPMENT_MODE
	// Constructor to load from disk (dev.).
	// Pass -1 as numColors to skip loading a palette!
	Image(const char *path, unsigned int xRes, unsigned int yRes, unsigned int numColors, unsigned int palOffs) : 
	      m_path(path), m_xRes(xRes), m_yRes(yRes), m_palSize(numColors*3), m_palOffs(palOffs + kColorOffs)
,	      m_pPalette(NULL), m_pImage(NULL)
,	      m_xRes4(xRes>>2), m_planeSize((xRes>>2)*yRes)
,	      m_pFadeTab(NULL)
	{
		// Load immediately, check later.
		Load();
	}
#endif

	~Image()
	{
		delete[] m_pPalette;
		delete[] m_pImage;
		delete[] m_pFadeTab;
	}

#ifdef DEVELOPMENT_MODE

private:
	bool Load()
	{
		if ( 0 != (m_xRes & 0x3) )
		{
			sprintf(s_lastErr, "Image (%s) width (%u) is not a multiple of 4!\n Required for conversion to planar format.", m_path, m_xRes);
			return false;
		}

		char palPath[256], imgPath[256];
		sprintf(palPath, "%s.act", m_path);
		sprintf(imgPath, "%s.raw", m_path);

		size_t palSize;
		m_pPalette = ReadFile(palPath, true, palSize);
		if (NULL == m_pPalette)
			return false;

		// Cut down palette & calculate fade table (black).
		PAL_826(m_pPalette, m_palSize);
		m_pFadeTab = PAL_CalculateFadeTable(m_pPalette, m_palSize, NULL);

		size_t imgSize;
		m_pImage = ReadFile(imgPath, true, imgSize);
		if (NULL == m_pImage)
			return false;

		// Check if raw image size matches resolution (8BPP).
		if (m_xRes*m_yRes != imgSize)
		{
			sprintf(s_lastErr, "Image (%s) does not match resolution: %u*%u.", m_path, m_xRes, m_yRes);
			return false;
		}

		// Convert to ModeX planar.
		uint8_t *pChunky = m_pImage;
		uint8_t *pPlanar = new uint8_t[imgSize];
	
		unsigned int planarLineOffs = 0;	
		for (unsigned int iY = 0; iY < m_yRes; ++iY)
		{
			for (unsigned int iX = 0; iX < m_xRes; ++iX)
			{
				const unsigned int plane = iX & 0x3;
				const unsigned int planarIdx = plane*m_planeSize + planarLineOffs + (iX>>2);
				pPlanar[planarIdx] = *pChunky++ + m_palOffs; // Add palette offset.
			}

			planarLineOffs += m_xRes4;
		}

		// Swap chunky for planar.	
		delete[] m_pImage;
		m_pImage = pPlanar;

#ifdef DUMP_C_DATA
		// Add to executable data.
		char instance[256];
		const char *name = StripPath(m_path);
		sprintf(instance, "Image %s(g_palette_%s, g_bitmap_%s, %u, %u, %u, %u);", name, name, name, m_xRes, m_yRes, m_palSize/3, m_palOffs);
		s_dataCPP.AddBitmap(m_path, name, m_pImage, imgSize, m_pPalette, m_palSize, instance);
#endif

		// Done!
		printf("Raw image loaded and converted: %s\n", m_path);
		return true;
	}
#endif // DEVELOPMENT_MODE

public:
	void SetPalette(unsigned int opacity) const
	{
		if (NULL != m_pPalette)
		{
			uint8_t *pDest = s_palette + m_palOffs*3;			
			if (opacity == 63)
				memcpy(pDest, m_pPalette, m_palSize);
			else
				PAL_Fade(pDest, m_pPalette, m_pFadeTab, m_palSize, opacity);

			s_paletteDirty = true;
		}
	}

	// X-case: assumes m_xRes is 320 (kResX).
	// FIXME: optimize.
	void DrawX(uint8_t *pVRAM, unsigned int yOffs) const
	{
		const size_t planeOffs = yOffs*kPlaneW;
		pVRAM += planeOffs;
		
		const uint8_t *pPlane = m_pImage;

		// FIXME: assembler, unrolled.	
		VGA_ModeX_SetPlane(0);		
		memcpy(pVRAM, pPlane, m_planeSize);
		pPlane += m_planeSize;
		VGA_ModeX_SetPlane(1);
		memcpy(pVRAM, pPlane, m_planeSize);
		pPlane += m_planeSize;
		VGA_ModeX_SetPlane(2);
		memcpy(pVRAM, pPlane, m_planeSize);
		pPlane += m_planeSize;
		VGA_ModeX_SetPlane(3);
		memcpy(pVRAM, pPlane, m_planeSize);
	}

	void DrawX_SinglePlane(uint8_t *pVRAM, unsigned int yOffs, unsigned int iPlane) const
	{
		const size_t planeOffs = yOffs*kPlaneW;
		pVRAM += planeOffs;
		
		const uint8_t *pPlane = m_pImage;

		VGA_ModeX_SetPlane(iPlane);
		memcpy(pVRAM, pPlane + m_planeSize*iPlane, m_planeSize);		
	}

	unsigned int GetWidth()     const { return m_xRes;      }
	unsigned int GetHeight()    const { return m_yRes;      }
	unsigned int GetNumColors() const { return m_palSize/3; }
	unsigned int GetPalOffs()   const { return m_palOffs;   }

	const uint8_t *GetPalette() const { return m_pPalette;  }
	const uint8_t *GetBitmap()  const { return m_pImage;    }

	unsigned int GetXRes4()     const { return m_xRes4;     }
	unsigned int GetPlaneSize() const { return m_planeSize; }

private:
	const char *m_path;
	const unsigned int m_xRes, m_yRes;
	const unsigned int m_palSize;
	const unsigned int m_palOffs;

	uint8_t *m_pPalette;
	uint8_t *m_pImage;
	int *m_pFadeTab;

	// Planar-specific sizes.
	const unsigned int m_xRes4;
	const unsigned int m_planeSize;
};

// --------------------------------------------------------------------------------------------------------------------
//
// Raw 8-bit font container (.ACT color table & raw, exported by Photoshop CS6).
// Chunky mode, use C2P to convert.
//
// Assumes that: 
// - All characters are on a single line, so yRes is the font's height.
// - The first character is ASCII number 32 (i.e. the space).
//
// --------------------------------------------------------------------------------------------------------------------

class Font
{
public:
	// Constructor to load from memory (release).
	Font(uint8_t *pPalette, uint8_t *pBitmap,
	     unsigned int xRes, unsigned int yRes, unsigned int charWidth,
	     unsigned int numColors, unsigned int palOffs) : 
	     m_path(""), m_xRes(xRes), m_yRes(yRes), m_charWidth(charWidth)
,	     m_palSize(numColors*3), m_palOffs(palOffs)
,	     m_pPalette(pPalette), m_pBitmap(pBitmap)
,	     m_pFadeTab(PAL_CalculateFadeTable(m_pPalette, m_palSize, NULL))
	{
		// Expects processed data (DEVELOPMENT_MODE).
	}

#ifdef DEVELOPMENT_MODE
	// Constructor to load from disk (dev.).
	Font(const char *path, 
	     unsigned int xRes, unsigned int yRes, unsigned int charWidth,
	     unsigned int numColors, unsigned int palOffs) : 
	     m_path(path), m_xRes(xRes), m_yRes(yRes), m_charWidth(charWidth)
,	     m_palSize(numColors*3), m_palOffs(palOffs + kColorOffs)
,	     m_pPalette(NULL), m_pBitmap(NULL)
,	     m_pFadeTab(NULL)
	{
		// Load immediately, check later.
		Load();
	}
#endif

	~Font()
	{
		delete[] m_pPalette;
		delete[] m_pBitmap;
		delete[] m_pFadeTab;
	}

#ifdef DEVELOPMENT_MODE

private:
	bool Load()
	{
		if ( 0 != (m_xRes & 0x3) )
		{
			printf("WARNING: font (%s) width (%u) is not a multiple of 4.\n", m_path, m_xRes);
			printf("Doesn't mean it's not 16 pixels wide, maybe exported it drunk.\n");
		}

		char palPath[256], imgPath[256];
		sprintf(palPath, "%s.act", m_path);
		sprintf(imgPath, "%s.raw", m_path);

		size_t palSize;
		m_pPalette = ReadFile(palPath, true, palSize);
		if (NULL == m_pPalette)
			return false;

		// Cut down palettte & calculate fade table (black).
		PAL_826(m_pPalette, m_palSize);
		m_pFadeTab = PAL_CalculateFadeTable(m_pPalette, m_palSize, NULL);

		size_t imgSize;
		m_pBitmap = ReadFile(imgPath, true, imgSize);
		if (NULL == m_pBitmap)
			return false;

		// Check if raw image size matches resolution (8BPP).
		if (m_xRes*m_yRes != imgSize)
		{
			sprintf(s_lastErr, "Font bitmap (%s) does not match resolution: %u*%u.", m_path, m_xRes, m_yRes);
			return false;
		}

		// Add palette offset to pixels.
		for (size_t iPixel = 0; iPixel < imgSize; ++iPixel)
			m_pBitmap[iPixel] += m_palOffs;

#ifdef DUMP_C_DATA
		char instance[256];
		const char *name = StripPath(m_path);
		sprintf(instance, "Font %s(g_palette_%s, g_bitmap_%s, %u, %u, %u, %u, %u);", name, name, name, m_xRes, m_yRes, m_charWidth, m_palSize/3, m_palOffs);
		s_dataCPP.AddBitmap(m_path, name, m_pBitmap, imgSize, m_pPalette, m_palSize, instance);
#endif

		// Done!
		printf("Raw font loaded: %s\n", m_path);
		return true;
	}

#endif // DEVELOPMENT_MODE

public:
	void SetPalette(unsigned int opacity) const
	{
		if (NULL != m_pPalette)
		{
			uint8_t *pDest = s_palette + m_palOffs*3;		
			if (opacity == 63)
				memcpy(pDest, m_pPalette, m_palSize);
			else
				PAL_Fade(pDest, m_pPalette, m_pFadeTab, m_palSize, opacity);

			s_paletteDirty = true;
		}
	}

private:
	unsigned int GetCharOffs(char character) const
	{
		character -= ' '; // Or: ASCII 32
		return character*m_charWidth;
	}

public:
	// Draw single line to chunky buffer.
	// Test function, don't use it.
	void DrawLine(uint8_t *pDest, unsigned int destWidth, const char *text) const
	{
		const int yRange = m_yRes*destWidth;

		while (*text)
		{
			const char character = *text++;
			const unsigned int charOffs = GetCharOffs(character);
			uint8_t *pSrc = m_pBitmap + charOffs;

			for (int yOffs = 0; yOffs < yRange; yOffs += destWidth)
			{
				memcpy(pDest+yOffs, pSrc, m_charWidth);
				pSrc += m_xRes;
			}

			pDest += m_charWidth;
		}
	}

	// Draw single line to chunky buffer.
	// X-case: character width 16, dest. width 320.
	// FIXME: optimize further (assembly).
	void DrawLineX(uint8_t *pDest, const char *text) const
	{
		unsigned int yRange = m_yRes<<6;
		yRange += yRange<<2;

		while (*text)
		{
			const char character = *text++;
			const unsigned int charOffs = (character-32) << 4;
			uint8_t *pSrc = m_pBitmap + charOffs;

			for (int yOffs = 0; yOffs < yRange; yOffs += 320)
			{
				memcpy(pDest+yOffs, pSrc, m_charWidth);
				pSrc += m_xRes;
			}

			pDest += 16;
		}
	}

	unsigned int GetLineWidth(const char *text) const
	{
		return strlen(text)*m_charWidth;
	}

	unsigned int GetWidth()     const { return m_xRes;      }
	unsigned int GetHeight()    const { return m_yRes;      }
	unsigned int GetCharWidth() const { return m_charWidth; }
	unsigned int GetNumColors() const { return m_palSize/3; }
	unsigned int GetPalOffs()   const { return m_palOffs;   }

private:
	const char *m_path;
	const unsigned int m_xRes, m_yRes;
	const unsigned int m_charWidth;
	const unsigned int m_palSize;
	const unsigned int m_palOffs;

	uint8_t *m_pPalette;
	uint8_t *m_pBitmap;
	int *m_pFadeTab;
};

// --------------------------------------------------------------------------------------------------------------------
//
// C2P.
//
// --------------------------------------------------------------------------------------------------------------------

class C2P
{
public:
	C2P(unsigned int width, unsigned int height) : 
	    m_pChunky(new uint8_t[width*height])
,	    m_xRes(width), m_xRes4(width>>2), m_yRes(height)
,	    m_planeSize((width>>2)*height)
	{
//		Clear(kBorder);
	}

	~C2P()
	{
		delete[] m_pChunky;
	}

/*
	// X-case: assumes source and dest. are 320 pixels wide.
	// FIXME: optimize, if used at all
	void BlitToRAMX(uint8_t *pDest, unsigned int yOffs) const
	{
		// Planar offset.
		const size_t topLeft = yOffs*kPlaneW;
		pDest += topLeft;

		const uint32_t *pChunky = reinterpret_cast<uint32_t *>(m_pChunky);
		uint8_t *pPlanar = pDest;
	
		unsigned int numPixels4 = m_planeSize;
		while (numPixels4--)
		{
			uint32_t pixels = *pChunky++;

			*pPlanar = pixels;
			pixels >>= 8;

			unsigned int planeOffs = m_planeSize;
			pPlanar[planeOffs] = pixels;
			pixels >>= 8;

			planeOffs <<= 1;
			pPlanar[planeOffs] = pixels;
			pixels >>= 8;

			planeOffs += m_planeSize;
			pPlanar[planeOffs] = pixels;

			++pPlanar;
		}
	}
*/

	// X-case: assumes m_xRes is 320 pixels.
	// FIXME	
	void BlitToVRAMX(uint8_t *pVRAM, unsigned int yOffs) const
	{
		// Planar offset.
		const size_t topLeft = yOffs*kPlaneW;
		pVRAM += topLeft;

		// Convert to planar.
		const uint8_t *pChunky = m_pChunky;
		uint8_t *pPlanar = pVRAM;

		// We'll be doing this by the column (assuming, dangerously, that this outpb() is far more expensive than accessing small parts of chunky memory by column)
		outpb(VP_SEQ_ADDR, VR_SEQ_MAP_MASK);

		for (unsigned int iX = 0; iX < m_xRes; ++iX)
		{
			const unsigned int bit = iX & 3;
			outpb(VP_SEQ_DATA, 1<<bit);

			const uint8_t *pChunkyCol = pChunky;
			uint8_t *pPlanarCol = pPlanar;

			for (unsigned int iY = 0; iY < m_yRes; ++iY)
			{
				*pPlanarCol = *pChunkyCol;

				pPlanarCol += kPlaneW;
				pChunkyCol += m_xRes;
			}

			++pChunky; // Next column

			if (bit == 0x3) 
				++pPlanar; // Next colum after we've done each plane
		}
	}

	// X_R-case: assumes m_xRes is 160 pixels.
	// FIXME
	void BlitToVRAMX_R(uint8_t *pVRAM, unsigned int yOffs) const
	{
		// Planar offset.
		const size_t topLeft = yOffs*kPlaneW;
		pVRAM += topLeft;

		// Convert to planar.
		const uint8_t *pChunky = m_pChunky;
		uint8_t *pPlanar = pVRAM;

		// We'll be doing this by the column (assuming, dangerously, that this outpb() is far more expensive than accessing small parts of chunky memory by column)
		outpb(VP_SEQ_ADDR, VR_SEQ_MAP_MASK);

		pPlanar += kPlaneW/2;
		pChunky += m_xRes>>1;

		for (unsigned int iX = 160; iX < m_xRes; ++iX)
		{
			const unsigned int bit = iX & 3;
			outpb(VP_SEQ_DATA, 1<<bit);

			const uint8_t *pChunkyCol = pChunky;
			uint8_t *pPlanarCol = pPlanar;

			for (unsigned int iY = 0; iY < m_yRes; ++iY)
			{
				*pPlanarCol = *pChunkyCol;

				pPlanarCol += kPlaneW;
				pChunkyCol += m_xRes;
			}

			++pChunky; // Next column

			if (bit == 0x3) 
				++pPlanar; // Next colum after we've done each plane
		}
	}

	void Clear(unsigned int color)
	{
		memset(m_pChunky, color, m_planeSize<<2);
	}

	uint8_t *GetChunky() { return m_pChunky; }
	size_t GetImgSize() const { return m_planeSize<<2; }

private:
	uint8_t *m_pChunky;

	const unsigned int m_xRes, m_xRes4, m_yRes;
	const unsigned int m_planeSize;
};

// --------------------------------------------------------------------------------------------------------------------
//
// Global graphics (embedded on release).
//
// --------------------------------------------------------------------------------------------------------------------

#if defined(DEVELOPMENT_MODE)

/*
	Load all (unconverted) graphics here!
	They can be written to disk for use in non-dev. mode by enabling a toggle on top (DUMP_C_DATA).

	Loading is performed automatically and checked in main().
	
	Rules:
	- Name each instance after the filename without extension(s).
	- Keep an eye on per part palette use.
	- You have kTotalColors-1 (defined on top, #0 is reserved for kBorder).
*/

// Accolade part:
Image accolade("graphics\\accolade", 320, 240, 64, 0);

// Credits part:
Image crd_logo("graphics\\crd_logo", 320, 136, 32, 0);
Font crd_font("graphics\\crd_font", 960, 16, 16, 8, 32);

// Menu part:
Image mnu_logo("graphics\\mnu_logo", 320, 122, 16, 0);
Image mnu_grp("graphics\\mnu_grp", 320, 122, 16, 16);

Font mnu_font("graphics\\mnu_font", 1024, 9, 16, 2, 16+16);
Font mnu_fnt2("graphics\\mnu_fnt2", 1024, 9, 16, 2, 16+16+2);
// Font mnu_arr("graphics\\mnu_arr", 32, 32, 16, 7, 16+16+2);

// Greetings part:
Image grt_girl("graphics\\grt_girl", 320, 240, 32, 0);
Font grt_font("graphics\\grt_font", 1090, 16, 16, 24, 32);

#else // !defined(DEVELOPMENT_MODE)

/*
	Embedded (converted) graphics data.
	Generate by defining DUMP_C_DATA & DEVELOPMENT_MODE.
*/

#include "data.cpp"
#include "data.h"

#endif // defined(DEVELOPMENT_MODE)

// --------------------------------------------------------------------------------------------------------------------
//
// Part base class.
//
// A simple and static model (just a minimal description):
// - 1. Part fades in, uninterrupted.
// - 2. Part cycles and either ends on it's own accord or due to a keystroke.
// - 3. Part fades out, uninterrupted.
//
// Base takes care of the little state machine and timer offset.
//
// --------------------------------------------------------------------------------------------------------------------

class Part
{
public:
	Part()
	{
		m_section = IDLE;
	}

	virtual ~Part() {}

private:
	float m_tSection;

protected:

	// Called once directly before FadeIn().
	// Use it to initialize variables and maybe start a new track.
	virtual void Prepare() = 0;

	// Fade in, return true when done.
	virtual bool FadeIn(float time) = 0;

	// Main cycle, return true when done.
	virtual bool Main(float time, int keyPressed) = 0;

	// Fade out, return true when done.
	virtual bool FadeOut(float time) = 0;

public:
	// Returns true when ready to exit (preferrably neatly faded out).
	bool Render(float time, int keyPressed)
	{
		switch (m_section)
		{
		case IDLE:
			// Reset and proceed to FADE_IN directly.
			Prepare();
			m_section = FADE_IN;
			m_tSection = time;

		case FADE_IN:
			if (true == FadeIn(time-m_tSection))
			{
				// Proceed to MAIN.
				m_section = MAIN;
				m_tSection = time;
			}

			break;

		case MAIN:
			if (true == Main(time-m_tSection, keyPressed))
			{
				// Proceed to FADE_OUT.
				m_section = FADE_OUT;
				m_tSection = time;
			} 

			break;

		case FADE_OUT:
			if (true == FadeOut(time-m_tSection)) 
			{
				// Return state to IDLE.
				m_section = IDLE;

				// And we're done.
				return true;
			}
		}

		return false;
	}

private:
	enum Section
	{
		IDLE,
		FADE_IN,
		MAIN,
		FADE_OUT
	} m_section;
};

// --------------------------------------------------------------------------------------------------------------------
//
// Accolade bumper.
//
// --------------------------------------------------------------------------------------------------------------------

class AccoladeIntro : public Part
{
public:
	AccoladeIntro() : Part() {}
	~AccoladeIntro() {}

	/* virtual */ void Prepare() 
	{
		// Appropriate track is started by Audio_Start().
	}

	/* virtual */ bool FadeIn(float time)
	{		
		const unsigned int iFade = fto6(time*2.f); 

		accolade.SetPalette(iFade);

		// Plane transition.
		// Only possibly screws up if we miss 4 frames at one specific time.
		const uint8_t *pPlanar = accolade.GetBitmap();
		const unsigned int iPage = iFade>>4;
		VGA_ModeX_SetPlane(iPage);
		uint8_t *pDest = g_pWrite;
		const size_t imgOffs = iPage*kPlaneSize;
		memcpy(pDest, pPlanar + imgOffs, kPlaneSize);

		MIDAS_ModeX_Flip();

		return time >= 0.5f; // 0.25f;
	}

	/* virtual */ bool Main(float time, int keyPressed)
	{
		// FIXME: look at the generated code, may answer your question about Watcom & constants.
		const float duration = 6.f;
		const float animTime = 1.25f;
		const float dashFadeTime = 0.5f;
		const float animOffs = (duration-animTime)*0.5f;
		const float dashFadeOutOffs = animOffs+animTime;

		// Hack.
		#define DASH_RED_COL (kTotalColors-1)
		#define DASH_RED_PAL_IDX ((kTotalColors-1)*3)

		if (time < animOffs+dashFadeTime)
		{
			// Fade in dash.
			// Hack: just modifying R, assuming the rest is still 0.
			const unsigned int red = fto6((time-animOffs)*2.f);
			s_palette[DASH_RED_PAL_IDX] = red;
			s_paletteDirty = true;
		}

		if (time >= dashFadeOutOffs && time < dashFadeOutOffs+dashFadeTime)
		{
			// Fade out dash.
			// Hack: just modifying R, assuming the rest is still 0.
			const unsigned int red = 63 - fto6((time-dashFadeOutOffs)*2.f);
			s_palette[DASH_RED_PAL_IDX] = red;
			s_paletteDirty = true;
		}

		if (time >= animOffs && time < dashFadeOutOffs+dashFadeTime)
		{
			// Draw dash.
			int laserX;
			if (time >= dashFadeOutOffs)
				laserX = 80-8;
			else
			{
				const float fLaserX = (time-animOffs)/animTime;
				laserX = fLaserX*(80.f-8.f);
			}

			const unsigned int laserOffsY = 320/4 * 91;
			uint8_t *pDest = g_pWrite + laserOffsY;

			// 4 pixels at once.
			VGA_ModeX_SetPlanes(0x0f);

			// Clear entire dash trail (important if frames skip).
			for (int iX = 0; iX < laserX; ++iX)
			{
				uint8_t *pTrail = pDest+iX;
				for (unsigned int iY = 0; iY < 4; ++iY)
				{
					*((uint32_t *) pTrail) = 0; // 4*4.
					pTrail += kPlaneW;
				}
			}

			// New dash.
			uint8_t *pDash = pDest + laserX;
			for (unsigned int iY = 0; iY < 4; ++iY)
			{
				*((uint32_t *) pDash) = DASH_RED_COL*0x01010101; // 4*4.
				pDash += kPlaneW;
			}

		}

		MIDAS_ModeX_Flip();
		
		// Can't skip this one.		
		return time >= duration;
	}

	/* virtual */ bool FadeOut(float time)
	{
		const unsigned int iFade = fto6(time);

		accolade.SetPalette(63-iFade);

		if (iFade < 16)
		{
			// Plane transition (first 16 frames).
			const uint8_t *pPlanar = accolade.GetBitmap();
			const unsigned int iPage = iFade>>2;
			VGA_ModeX_SetPlane(0x3-iPage);
			uint8_t *pDest = g_pWrite;
			memset(pDest, 0, kPlaneW*120);
			
			MIDAS_ModeX_Flip();
		}
		else
			MIDAS_ModeX_Cycle();

		return time >= 1.f;
	}

private:
};

// --------------------------------------------------------------------------------------------------------------------
//
// Credits (not really).
//
// --------------------------------------------------------------------------------------------------------------------

class Credits : public Part
{
public:
	Credits() : Part(), m_credC2P(320, 32) 
	{
		m_credC2P.Clear(kBorder);
		{
			uint8_t *pChunky = m_credC2P.GetChunky();
			unsigned int lineOffs;
			const char *action = "WRECK DAT KEYBOARD";
			lineOffs  = 0;
			lineOffs += (320-crd_font.GetLineWidth(action))>>1;
			crd_font.DrawLineX(pChunky+lineOffs, action);
		}

	}

	~Credits() {}

private:

	void SetPalettes(unsigned int iFade)
	{
		crd_logo.SetPalette(iFade);
		crd_font.SetPalette(iFade);
//		mnu_font.SetPalette(iFade);
	}

	void Draw(float time)
	{
		VGA_ModeX_Clear(); // You can move this to Prepare() if you decide to just clear a piece of VRAM (where the bouncey thing is)

		crd_logo.DrawX(g_pWrite, 15);

		float whatever = smoothstepf(0.f, 32.f, fmod(time, 1.f));
		if (whatever >= 16.f) whatever = 16.f - (whatever-16.f);

		const int DYP = -8 + int(whatever);
		m_credC2P.BlitToVRAMX(g_pWrite, 200+DYP);

		MIDAS_ModeX_Flip();
	}

public:
	/* virtual */ void Prepare() 
	{
	}

	/* virtual */ bool FadeIn(float time)
	{
		const unsigned int iFade = fto6(time*2.f);

		SetPalettes(iFade);

		Draw(0.f);

		return time >= 0.5f;
	}

	/* virtual */ bool Main(float time, int keyPressed)
	{
		Draw(time);

		// Skippable by any key.
		return -1 != keyPressed;
	}

	/* virtual */ bool FadeOut(float time)
	{
		const unsigned int iFade = 63-fto6(time);

		// Fade out track.
		Audio_SetVolume(iFade);

		SetPalettes(iFade);
		Draw(time);
//		MIDAS_ModeX_Cycle();

		return time >= 1.f;
	}

private:
	C2P m_credC2P;
};

// --------------------------------------------------------------------------------------------------------------------
//
// PART: menu.
//
// --------------------------------------------------------------------------------------------------------------------

class Menu : public Part
{
public:
	Menu() : Part(), m_menuC2P(320, 96), m_tMenuAnimOffs(0.f) {}
	~Menu() {}

private:
	void SetPalettes(unsigned int iFade)
	{
		mnu_logo.SetPalette(iFade);
		mnu_grp.SetPalette(iFade);
		mnu_font.SetPalette(iFade);
		mnu_fnt2.SetPalette((iFade>50)?50:iFade);
	}

	// Draw entire information block.
	void DrawInfo(unsigned iFade, float time)
	{
		// Get artist & name.
		const size_t iList = m_iTrackSel*3;
//		const char *modFilename = s_tracks[iList+0];
		const char *modArtist = s_tracks[iList+1];
		const char *modName = s_tracks[iList+2];

		m_menuC2P.Clear(kBorder);
		{
			uint8_t *pChunky = m_menuC2P.GetChunky();

			unsigned int lineOffs;

			const char *action = NULL;
			if (m_iTrackPlaying == m_iTrackSel)
			{
				// Ugly.. but it works
				static char status[256];
				
				MIDASplayStatus playStatus;
				MIDASgetPlayStatus(s_modulePlay, &playStatus);

				// Good enough for now
				sprintf(status, "#%02d PTN %02d ROW %02d", playStatus.position, playStatus.pattern, playStatus.row);

				action = status;
			}
			else
			{
				const char *flicker = fmod(time, 0.624f) > 0.214f ? "HIT SPACE TO PLAY" : ""; // FIXME: constants
				action = flicker;
			}

			lineOffs  = 320 * ((44-9)/2 + 8);
			lineOffs += (320-mnu_font.GetLineWidth(action))>>1;
			mnu_font.DrawLineX(pChunky+lineOffs, action);

			lineOffs  = 320 * (44+16);
			lineOffs += (320-mnu_font.GetLineWidth(modArtist))>>1;
			mnu_font.DrawLineX(pChunky+lineOffs, modArtist);

			lineOffs  = 320 * (44+32);
			lineOffs += (320-mnu_font.GetLineWidth(modName))>>1;
			mnu_fnt2.DrawLineX(pChunky+lineOffs, modName);
		}
		m_menuC2P.BlitToVRAMX(g_pWrite, 128);
	}

	void Draw(unsigned iFade, float time)
	{
		// Draw logo.
		if (0 != m_iLogo)
			mnu_logo.DrawX(g_pWrite, 0);
		else
			mnu_grp.DrawX(g_pWrite, 0);

		// Clear middle bar.
		VGA_ModeX_SetPlanes(0x0f);
		uint32_t *pDest = reinterpret_cast<uint32_t *>(g_pWrite + 80*122);
		for (int iClear = 0; iClear < (80*6)/4; ++iClear)
			*pDest++ = kBorder;

		// Draw player bar (C2P).
		DrawInfo(iFade, time);

		// Clear bottom bar (FIXME: scroller).
		VGA_ModeX_SetPlanes(0x0f);
		pDest = reinterpret_cast<uint32_t *>(g_pWrite + 80*222);
		for (int iClear = 0; iClear < (80*16)/4; ++iClear)
			*pDest++ = kBorder;

		MIDAS_ModeX_Flip();
	}

public:
	/* virtual */ void Prepare()
	{
		// First frame: reset state.
		m_iTrackPlaying = m_iTrackSel = kBeachTrack;
		m_iLogo = 0;
		m_state = kInput;

		// Play track.
		Audio_SelectTrack(m_iTrackSel);
	}

	/* virtual */ bool FadeIn(float time)
	{
		const unsigned int iFade = fto6(time);

		SetPalettes(iFade);
		Draw(iFade, 0.f);

		return time >= 1.f;
	}

	/* virtual */ bool Main(float time, int keyPressed)
	{
		unsigned iFade = 63;

		if (kInput == m_state)
		{
			switch (keyPressed)
			{
			case KEY_SCAN_LEFT:
				if (m_iTrackSel > 0) --m_iTrackSel;
				else m_iTrackSel = kNumDiskTracks-1;
				m_state = kSelectLeft;
				m_tTrans = time;
				break;

			case KEY_SCAN_RIGHT:
				if (m_iTrackSel < kNumDiskTracks-1) ++m_iTrackSel;
				else m_iTrackSel = 0;
				m_state = kSelectRight;
				m_tTrans = time;
				break;

			case KEY_SPACE:
			case KEY_ENTER: // Begrudingly I also accept this
				if (m_iTrackSel != m_iTrackPlaying)
				{
					m_state = kPlay;
					m_tTrans = time;
				}

				break;

			default:
				break;
			}
		}

		switch (m_state)
		{
		case kSelectLeft:
			{
				const float tDelta = time-m_tTrans;
				iFade = fto6(tDelta*8.f);
				
				if (tDelta >= 0.125f)
				{			
					m_state = kInput;
					m_tMenuAnimOffs = time;
				}
			}

			break;

		case kSelectRight:
			{
				const float tDelta = time-m_tTrans;
				iFade = fto6(tDelta*8.f);

				if (tDelta >= 0.125f)
				{			
					m_state = kInput;
					m_tMenuAnimOffs = time;
				}
			}

			break;

		// Fade out all relevant things and switch over to, it's opposite..
		case kPlay:
			{
				const float tDelta = time-m_tTrans;
				iFade = fto6(tDelta*2.f);

				// Fade out current track.
				Audio_SetVolume(63-iFade);

				// Fade out logo (just both now, FIXME).
				mnu_logo.SetPalette(63-iFade);
				mnu_grp.SetPalette(63-iFade);

				if (tDelta >= 0.5f)
				{
					// Start new track.
					Audio_SelectTrack(m_iTrackSel);
					m_iTrackPlaying = m_iTrackSel;

					// Swap logo
					m_iLogo ^= 1;

					// Transition back
					m_tTrans = time;
					m_state = kPlay2;
				}
			}

			break;

		// Fading back 
		case kPlay2:
			{
				const float tDelta = time-m_tTrans;
				iFade = fto6(tDelta*2.f);

				// Fade in logo (just both now, FIXME).
				mnu_logo.SetPalette(iFade);
				mnu_grp.SetPalette(iFade);

				if (tDelta >= 0.5f)
				{
					m_state = kInput;
				}
			}

			break;

		default:
			break;
		}

		Draw(iFade, time);

		// Exit only by escape, and not during a transition.
		return KEY_ESC == keyPressed && kInput == m_state;
	}

	/* virtual */ bool FadeOut(float time)
	{
		const unsigned int iFade = 63-fto6(time);

		// Fade track out, next part has it's own music.
		Audio_SetVolume(iFade);

		// FIXME: this stops any possible animation.
		SetPalettes(iFade);
		MIDAS_ModeX_Cycle();

		return time >= 1.f;
	}

private:
	C2P m_menuC2P;

	unsigned int m_iTrackPlaying;
	unsigned int m_iTrackSel;
	unsigned int m_iLogo;

	enum State
	{
		kInput,
		kSelectLeft,
		kSelectRight,
		kPlay, kPlay2
	} m_state;

	float m_tTrans;
	float m_tMenuAnimOffs;
};

// --------------------------------------------------------------------------------------------------------------------
//
// PART: greetings.
//
// --------------------------------------------------------------------------------------------------------------------

class Greetings : public Part
{
public:
	Greetings() : Part(), m_greetC2P(320, 200)
	{
		// FIXME: later expand to draw entire C2P blocks ready to blit and fade? -> Also move this to constructor!
		m_greetC2P.Clear(1);
		{
			uint8_t *pChunky = m_greetC2P.GetChunky();

			unsigned int lineOffs;

			const unsigned int yMul = grt_font.GetHeight() + 2;

			const char *greetings[11] = { "TPB", "TBL", "COCOON", "FAIRLIGHT", "DESIRE", "LINEOUT", "EFC", "SATORI", "...", " ", "PRESS ESC."};
			for (int iGreet = 0; iGreet < 11; ++iGreet)
			{
				lineOffs  = 160 + iGreet*yMul*320;
				lineOffs += (160-crd_font.GetLineWidth(greetings[iGreet]))>>1;
				grt_font.DrawLineX(pChunky+lineOffs, greetings[iGreet]);
			}
		}
	}

	~Greetings() {}

	/* virtual */ void Prepare() 
	{
		Audio_SelectTrack(kNumTracks-2);
	}

	/* virtual */ bool FadeIn(float time)
	{
//		VGA_ModeX_Clear();

		const unsigned int iFade = fto6(time);

		grt_girl.SetPalette(iFade);
		grt_font.SetPalette(iFade);

		grt_girl.DrawX(g_pWrite, 0);

		unsigned int offset = iFade>>2;
		m_greetC2P.BlitToVRAMX_R(g_pWrite, offset);

		MIDAS_ModeX_Flip();

		return time >= 1.f;
	}

	/* virtual */ bool Main(float time, int keyPressed)
	{
		MIDAS_ModeX_Cycle();

		// Exit only by escape
		return KEY_ESC == keyPressed;
	}

	/* virtual */ bool FadeOut(float time)
	{
		const unsigned int iFade = 63-fto6(time);

		Audio_SetVolume(iFade);

		grt_girl.SetPalette(iFade);
		grt_font.SetPalette(iFade);
		MIDAS_ModeX_Cycle();

		return time >= 1.f;
	}

private:
	C2P m_greetC2P;
};

// --------------------------------------------------------------------------------------------------------------------
//
// Main.
//
// --------------------------------------------------------------------------------------------------------------------

int main(int argC, char **argV)
{
	const size_t freeMemStart = GetFreeMem();
	size_t freeMemPostLoad = 0;

	printf ("Memory free on startup: %uKB (%uMB)\n", freeMemStart>>10, freeMemStart/(1024*1000));
	if (freeMemStart < 1024*1000*(kMegsRequired-kMegsEnvAndEXE))
	{
		printf("Insufficient: I need at least %uMB RAM.\n", kMegsRequired);
		return 1;
	}

	bool graphicsLoaded = false;

#ifdef DEVELOPMENT_MODE

	// If last error is set, loading failed.
	if (0 == strlen(s_lastErr))
	{
		graphicsLoaded = true;
	}

	// Show results.
	printf("[DEV] Press any key to continue...\n");
	while (!kbhit()) {} getch();

#else

	// Converted graphics (must be dumped using DUMP_C_DATA first!) all have a global instance ready.
	graphicsLoaded = true;

#endif

	// Semi-properly seed rand().
	srand(time(NULL));

	if (true == graphicsLoaded)
	{
		// Parts.
		AccoladeIntro accoladeIntro;
		Credits credits;
		Menu menu;
		Greetings greetings;

		// Flow.
		Part *flow[] =
		{
//			&accoladeIntro,
			&credits,
//			&menu,
			&greetings,
			NULL
		};

		unsigned int iPart = 0;

		// Now let's get started.
		if (true == Audio_Config())
		{
			// Set ModeX, then give it a some time to accomodate the display switch.
			SetVideoModeX();

			for (int iDelay = 0; iDelay < 60; ++iDelay)
				VGA_WaitForVBLANK();

			// Start music & go!
			if (true == Audio_Start())
			{
				// Experience teaches the DPMI statistics may not have been fully updated at this point.
				freeMemPostLoad = GetFreeMem();

				while (1)
				{
					Part *pPart = flow[iPart];
					if (NULL == pPart)
						break;

					// Check for input.
					int key = -1;
					if (kbhit())
					{
						key = getch();

						// Get scan code if applicable.
						if (true == IsScanCode(key))
						{
							key = getch();
						}
					}

					// Render.
					if (true == pPart->Render(g_runTime, key))
						++iPart;

					// Wait until frame is processed.
					unsigned int prevFrame = g_frameCount;
					while (prevFrame == g_frameCount) {}
				}
			}

			// Stop (any) audio & back to text mode.
			Audio_Stop();
			SetVideoMode(0x3); // FIXME: this flickers in DOSBox, check on real hardware.

#ifdef DEVELOPMENT_MODE
			// Statistics.
			if (-1 != s_midasRefresh)
				printf("[DEV] s_midasRefresh (VGA): %u\n", s_midasRefresh);

			printf("[DEV] freeMemStart: %uKB, freeMemPostLoad: %uKB, Used: %uKB\n", freeMemStart>>10, freeMemPostLoad>>10, (freeMemStart-freeMemPostLoad)>>10);
			printf("[DEV] s_midasRefresh: %u g_frameCount: %u runTime %4.2f\n", s_midasRefresh, g_frameCount, g_runTime);
			printf("[DEV] s_totalFramesDropped: %u\n", s_totalFramesDropped);
#endif
		}
	}

	Audio_Release();

	if (0 != strlen(s_lastErr))
	{
		printf(s_lastErr);
		printf("\n");
		return 1;
	}

	// The stuff nobody reads.
	printf("    ___   ____ ____    __    ____   ____         \n");
	printf("____\\  \\_/_  //   /___.\\_)._/_  /_./_  /____  \n");
	printf("\\    _  | '_/ \\___    |   | _/    | '_/    /   \n");
	printf(" \\   \\  |  \\   | /    |   | \\     |  \\    / \n");
	printf(" /______|______|______|___|__\\____|_______\     \n");
	printf("+-diP----------------------------------aSL-+     \n");
	printf("\n");
	printf("Chip Chop(ped) #16 by DESiRE.\n");
	printf("MS-DOS 486-DX port & bonus tunes by MEGAHAWKS INC.\n");
	printf("\n");
	printf("Remember kids: DOS does it better, and Accolade lives!\n");

	return 0;
}
