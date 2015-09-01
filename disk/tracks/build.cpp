
/*
	Little tool to compress all tracks into a single archive (using MiniLZO)
	- Layout: a table with offset & size (both uint32_t) pairs followed by compressed data.
	- Largest uncompressed track size is shown on exit; use it to allocate the MiniLZO uncompress buffer.

	Problem: leaks if it quits halfway.
*/

// CRT:
#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

// MiniLZO.
#include "../minilzo/minilzo.h"

// MiniLZO work memory (ugly macro lifted from example).
#define HEAP_ALLOC(varName, numBytes) lzo_align_t __LZO_MMODEL varName [ ((numBytes) + (sizeof(lzo_align_t) - 1)) / sizeof(lzo_align_t) ]
static HEAP_ALLOC(lzoWorkMem, LZO1X_1_MEM_COMPRESS);

// HACK: just allocate a megabyte for the LZO dest. buffer, should always be enough.
static uint8_t lzoBuf[1024*1000];

// Tracklist.
#include "tracks.h"

// Dest. file.
#define kDestPath "..\\tracks.dat"

static size_t GetFileSize(const char *filename)
{
	struct stat status;
	if (0 == stat(filename, &status))
		return status.st_size;
	else
		return -1;
}

static uint8_t *ReadFile(const char *path, bool binary, size_t &numBytesRead)
{
	FILE *hFile = fopen(path, (true == binary) ? "rb" : "r");
	if (NULL != hFile)
	{
		fseek(hFile, 0L, SEEK_END);
		const size_t numBytes = ftell(hFile);
		fseek(hFile, 0L, SEEK_SET);

		uint8_t *pData = new uint8_t[numBytes];			
		numBytesRead = fread(pData, 1, numBytes, hFile);

		// Since I don't have a predetermined size any output from fread() is acceptable,
		// and I'm assuming it will give me exactly what I'm just told through ftell().
		fclose(hFile);

		return pData;
	}
	
	numBytesRead = 0;
	return NULL;
}

int main(int argC, char **argV)
{
	// Initialize compressor.
    if (lzo_init() != LZO_E_OK)
    {
    	printf("Can't initialize MiniLZO.\n");
    	return 1;
    }

    // Table (header).
	uint32_t offsTab[kNumTracks*2];
	const size_t tabSize = kNumTracks*2*sizeof(uint32_t);

	size_t largestUncSize = 0;
	size_t rawDataSize = 0;
	size_t curOffs = tabSize;

	// Go!
	FILE *hArch = fopen(kDestPath, "wb");
	if (NULL != hArch)
	{
		for (size_t iTrack = 0; iTrack < kNumTracks; ++iTrack)
		{
			const size_t iList = iTrack*3;
			
			const char *path   = s_tracks[iList+0];
			const char *artist = s_tracks[iList+2];
			const char *name   = s_tracks[iList+1];

			printf("%s (%u): '%s' by %s\n", path, iTrack, name, artist);

			// Load track.
			size_t trackSize;
			uint8_t *pData = ReadFile(path, true, trackSize);
			if (NULL == pData)
			{
				printf("Error: can't read file.\n");
				exit(1);
			}

			// Keep track of the largest uncompressed size.
			if (trackSize > largestUncSize)
				largestUncSize = trackSize;

			rawDataSize += trackSize;

			// Compress it.
			lzo_uint lzoTrackSize;
			const int lzoRes = lzo1x_1_compress(pData, trackSize, lzoBuf, &lzoTrackSize, lzoWorkMem);
			if (LZO_E_OK != lzoRes || lzoTrackSize >= trackSize)
			{
				delete[] pData;
				printf("Error: can't compress file.\n");
				exit(1);
			}

			// Update table.
			const unsigned int iTab = iTrack<<1;
			offsTab[iTab+0] = curOffs;
			offsTab[iTab+1] = lzoTrackSize;

			printf("Offset: %u, Size (unc.): %u, Compressed: %u\n", curOffs, trackSize, lzoTrackSize);

			// Write compresed data (unchecked).
			fseek(hArch, curOffs, SEEK_SET);
			const size_t numBytesWritten = fwrite(lzoBuf, 1, lzoTrackSize, hArch);

			curOffs += lzoTrackSize;

			// Free uncompresed track.
			delete[] pData;
		}

		// Write table in the front (unchecked).
		fseek(hArch, 0, SEEK_SET);
		const size_t numBytesWritten = fwrite(offsTab, 1, tabSize, hArch);

		// Done!
		fclose(hArch);

		printf("Done, written %u tracks to: %s\n", kNumTracks, kDestPath);
		printf("Total uncompressed data: %u KB.\n", rawDataSize/1024);
		printf("Largest track (unc.): %u bytes.\n", largestUncSize);
		
		return 0;
	}

	printf("Can't open for write: %s\n", kDestPath);
	return 1;
}
