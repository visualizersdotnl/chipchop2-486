/*      IT.H
 *
 * Impulse Tracker Module format structures
 *
 * $Id: it.h,v 1.5 1997/07/31 10:56:42 pekangas Exp $
 *
 * Copyright 1996,1997 Housemarque, Inc
 *
 * This file is part of MIDAS Digital Audio System, and may only be
 * used, modified and distributed under the terms of the MIDAS
 * Digital Audio System license, "license.txt". By continuing to use,
 * modify or distribute this file you indicate that you have
 * read the license and understand and accept it fully.
*/


#ifndef __IT_H
#define __IT_H


/****************************************************************************\
*       enum itFlags
*       ------------
* Description:  Flag bits for different flags in IT modules
\****************************************************************************/

enum itFlags
{
    itStereo = 1,                       /* module is supposed to be stereo */
    itVol0Optimization = 2,             /* no mixing occurs if the volume at
					   mixing time is 0 (obsolete) */
    itInstruments = 4,                  /* use instruments */
    itLinearSlides = 8,                 /* linear slides */
    itOldEffects = 16,                 /* old effects on */

    itSongMessage = 1                   /* song message attached */
};


/****************************************************************************\
*       struct itPattern
*       ----------------
* Description:  Impulse Tracker Module pattern header
\****************************************************************************/

typedef struct
{
    U16         pattDataSize;           /* pattern size */
    U16         numRows;                /* rows in pattern */
    U32         reserved;
    U8          data[EMPTYARRAY];       /* packed pattern data */
} itPattern;


/*
                             Impulse Pattern Format

        0   1   2   3   4   5   6   7   8   9   A   B   C   D   E   F
      +-------+-------+---+---+---+---+-------------------------------+
0000: |Length | Rows  | x | x | x | x | Packed data................   |
      +---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+

      Length:   Length of packed pattern, not including the 8 byte header
                Note that the pattern + the 8 byte header will ALWAYS
                be less than 64k
      Rows:     Number of rows in this pattern (Ranges from 32->200)

      Patterns are unpacked by the following pseudocode... (this may look
      horrible, but in practise, it's just as conveninent as the S3M
      pattern format for playback (but not for display))

      GetNextChannelMarker:
        Read byte into channelvariable.
        if(channelvariable = 0) then end of row
        Channel = (channelvariable-1) & 63              ; Channel is 0 based.
        if(channelvariable & 128) then read byte into maskvariable
          else maskvariable = previousmaskvariable for current channel

        if(maskvariable & 1), then read note. (byte value)
                // Note ranges from 0->119 (C-0 -> B-9)
                // 255 = note off, 254 = notecut

        if(maskvariable & 2), then read instrument (byte value)
                // Instrument ranges from 1->99

        if(maskvariable & 4), then read volume/panning (byte value)
                // Volume ranges from 0->64
                // Panning ranges from 0->64, mapped onto 128->192

        if(maskvariable & 8), then read command (byte value) and commandvalue
                // Valid ranges from 0->31 (0=no effect, 1=A, 2=B, 3=C, etc.)

        if(maskvariable & 16), then note = lastnote for channel
        if(maskvariable & 32), then instrument = lastinstrument for channel
        if(maskvariable & 64), then volume/pan = lastvolume/pan for channel
        if(maskvariable & 128), then {
                command = lastcommand for channel and
                commandvalue = lastcommandvalue for channel
        }
        Goto GetNextChannelMarker

	*/

/****************************************************************************\
*       struct itHeader
*       ---------------
* Description:  Impulse Tracker Module file header
\****************************************************************************/

typedef struct
{
    char        signature[4];           /* "IMPM" */
    char        name[26];               /* module name */
    U16         reserved;
    U16         songLength;             /* number of orders */
    U16         numInsts;               /* number of instruments */
    U16         numSamples;             /* number of samples */
    U16         numPatts;               /* number of patterns */
    U16         trackerVersion;         /* version of the tracker this file
					   was created with */
    U16         compatibleVersion;      /* version this module is compatible
					   with */
    U16         flags;                  /* see itFlags enum above */
    U16         special;                /* song attachment see itFlags above */
    U8          globalVolume;           /* 0-128 */
    U8          mixingVolume;           /* 0-128 */
    U8          speed;
    U8          tempo;
    U8          panSeparation;          /* 0-128, 128 = max. separation */
    U8          zero;
    U16         msgLength;
    U32         msgOffset;
    U32         reserved2;
    U8          chanPan[64];
    U8          chanVol[64];
    U8          orders[EMPTYARRAY];
} itHeader;

/*
                             Impulse Header Layout

        0   1   2   3   4   5   6   7   8   9   A   B   C   D   E   F
      +---+---+---+---+-----------------------------------------------+
0000: |'I'|'M'|'P'|'M'| Song Name, max 26 characters, includes NULL   |
      +---+---+---+---+---------------------------------------+---+---+
0010: |.......................................................| x | x |
      +-------+-------+-------+-------+-------+-------+-------+---+---+
0020: |OrdNum |InsNum |SmpNum |PatNum | Cwt/v | Cmwt  | Flags |Special|
      +---+---+---+---+---+---+-------+-------+-------+---+---+---+---+
0030: |GV |MV |IS |IT |Sep|(0)|MsgLgth|Message Offset | x | x | x | x |
      +---+---+---+---+---+---+-------+---------------+---+---+---+---+
0040: | Chnl Pan (64 bytes)...........................................|
      +---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+

      +---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+
0080: | Chnl Vol (64 bytes)...........................................|
      +---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+

      +---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+
00C0: | Orders, Length = OrdNum                                       |
      +---------------------------------------------------------------+
xxxx: | 'Long' Offset of instruments, Length = InsNum*4 (1)           |
      +---------------------------------------------------------------+
xxxx: | 'Long' Offset of samples headers, Length = SmpNum*4 (2)       |
      +---------------------------------------------------------------+
xxxx: | 'Long' Offset of patterns, Length = PatNum*4 (3)              |
      +---------------------------------------------------------------+

      (1) Offset = 00C0h+OrdNum
      (2) Offset = 00C0h+OrdNum+InsNum*4
      (3) Offset = 00C0h+OrdNum+InsNum*4+SmpNum*4

        Note that if the (long) offset to a pattern = 0, then the
        pattern is assumed to be a 64 row empty pattern.

      Cwt:      Created with tracker.
                 Impulse Tracker y.xx = 0yxxh
      Cmwt:     Compatible with tracker with version greater than value.
                 (ie. format version)
      OrdNum:   Number of orders in song.
      InsNum:   Number of instruments in song
      SmpNum:   Number of samples in song
      PatNum:   Number of patterns in song
      Flags:    Bit 0: On = Stereo, Off = Mono
                Bit 1: Vol0MixOptimizations - If on, no mixing occurs if
                       the volume at mixing time is 0 (redundant v1.04+)
                Bit 2: On = Use instruments, Off = Use samples.
                Bit 3: On = Linear slides, Off = Amiga slides.
                Bit 4: On = Old Effects, Off = IT Effects
                        Differences:
                       - Vibrato is updated EVERY frame in IT mode, whereas
                          it is updated every non-row frame in other formats.
                          Also, it is two times deeper with Old Effects ON
                       - Command Oxx will set the sample offset to the END
                         of a sample instead of ignoring the command under
                         old effects mode.
                       - (More to come, probably)

      Special:  Bit 0: On = song message attached.
                       Song message:
                        Stored at offset given by "Message Offset" field.
                        Length = MsgLgth.
                        NewLine = 0Dh (13 dec)
                        EndOfMsg = 0

                       Note: v1.04+ of IT may have song messages of up to
                             8kb included.

                Bits 1-15: Undefined.

      GV:       Global volume. (0->128) All volumes are adjusted by this
      MV:       Mix volume (0->128) During mixing, this value controls
                the magnitude of the wave being mixed.
      IS:       Initial Speed of song.
      IT:       Initial Tempo of song
      Sep:      Panning separation between channels (0->128, 128 is max sep.)
      Chnl Vol: Volume for each channel. Ranges from 0->64
      Chnl Pan: Each byte contains a panning value for a channel. Ranges from
                 0 (absolute left) to 64 (absolute right). 32 = central pan,
                 100 = Surround sound.
                 +128 = disabled channel (notes will not be played, but note
                                          that effects in muted channels are
                                          still processed)
      Orders:   This is the order in which the patterns are played.
                 Valid values are from 0->199.
                 255 = "---", End of song marker
                 254 = "+++", Skip to next order

*/


/****************************************************************************\
*       struct itEnvelope
*       -----------------
* Description:  Impulse Tracker Module envelope
\****************************************************************************/

typedef struct
{
    U8          vol;
    U16         tick;
}itEnvNode;

typedef struct
{
    U8          flags;
    U8          num;
    U8          loopStart;
    U8          loopEnd;
    U8          susLoopStart;
    U8          susLoopEnd;
    itEnvNode   nodes[25];
} itEnvelope;


/*                              Envelope layout

        Envelopes: 3 structures, first for volume (130h), second for
                   panning (182h), third for pitch (1D4h).

                   Each is structured as such:

        0   1   2   3   4   5   6   7   8   9   A   B   C   D   E
      +---+---+---+---+---+---+-----------------------------------+---+
xxxx: |Flg|Num|LpB|LpE|SLB|SLE| Node points, 25 sets, 75 bytes....| x |
      +---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+

        Flg: Bit 0: Envelope on/off, 1 = on, 0 = off
             Bit 1: Loop on/off, 1 = on, 0 = off
             Bit 2: SusLoop on/off, 1 = on, 0 = off

        Num = Number of node points

        LpB = Loop beginning            SLB = Sustain loop beginning
        LpE = Loop end                  SLE = Sustain loop end

        Node point = 1 byte for y-value
                        (0->64 for vol, -32->+32 for panning or pitch)
                     1 word (2 bytes) for tick number (0->999)

        Total length of an instrument is 547 bytes, but 554 bytes are
        written, just to simplify the loading of the old format. (Hence
        there are 7 'wasted' bytes per instrument)

*/


/****************************************************************************\
*       struct itOldInstHeader
*       ----------------------
* Description:  Old (cmwt < 200h) Impulse Tracker Module instrument header
\****************************************************************************/

typedef struct
{
    char        signature[4];           /* "IMPI" */
    char        DOSfileName[12];
    U8          zero;
    U8          flags;
    U8          volLoopStart;
    U8          volLoopEnd;
    U8          susLoopStart;
    U8          susLoopEnd;
    U16         reserved;
    U16         fadeOut;
    U8          NNA;
    U8          DNC;
    U16         trackerVersion;
    U8          numSamples;
    U8          reserved2;
    char        instName[26];
    char        reserved3[6];
    U8          noteSampleTable[240];
    U8          volEnvelope[200];
    U8          nodePoints[2*25];
} itOldInstHeader;


/*
                  Old Impulse Instrument Format (cmwt < 200h)

        0   1   2   3   4   5   6   7   8   9   A   B   C   D   E   F
      +---+---+---+---+-----------------------------------------------+
0000: |'I'|'M'|'P'|'I'| DOS FileName (12345678.123)                   |
      +---+---+---+---+---+---+---+---+-------+---+---+-------+---+---+
0010: |00h|Flg|VLS|VLE|SLS|SLE| x | x |FadeOut|NNA|DNC|TrkVers|NoS| x |
      +---+---+---+---+---+---+---+---+-------+---+---+-------+---+---+
0020: | Instrument Name, max 26 bytes, includes NUL...................|
      +---------------------------------------+---+---+---+---+---+---+
0030: |.......................................| x | x | x | x | x | x |
      +---------------------------------------+---+---+---+---+---+---+
0040: | Note-Sample/Keyboard Table, Length = 240 bytes................|
      +---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+

      +---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+
0130: | Volume envelope (200 bytes)...................................|
      +---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+

                                      +---+---+---+---+---+---+---+---+
01F8:                                 | Node points (25x2 bytes)......|
                                      +-------------------------------+

        Total length of old instrument header is 554 bytes.

      Flg:      Bit 0. On = Use volume envelope
                Bit 1. On = Use volume loop
                Bit 2. On = Use sustain volume loop
      VLS:      Volume loop start (node number)
      VLE:      Volume loop end (node number)
      SLS:      Sustain loop start (node number)
      SLE:      Sustain loop end (node number)
      FadeOut:  Ranges between 0 and 64, but the fadeout "Count" is 512.
                See the Last section on how this works.
                Fade applied when:
                1) Note fade NNA is selected and triggered (by another note)
                2) Note off NNA is selected with no volume envelope
                   or volume envelope loop
                3) Volume envelope end is reached

      DNC:      Duplicate note check (0 = Off, 1 = On)
      NNA:      New note action:
                        0 = Note cut
                        1 = Note continue
                        2 = Note off
                        3 = Note fade

      TrkVers:  Tracker version used to save the instrument. This is only
                used in the instrument files.
      NoS:      Number of samples associated with instrument. This is only
                used in the instrument files.

      Note-Sample/Keyboard Table.
       Each note of the instrument is first converted to a sample number
       and a note (C-0 -> B-9). These are stored as note/sample pairs
       (note first, range 0->119 for C-0 to B-9, sample ranges from
       1-99, 0=no sample)

       Volume envelope: Values from 0->64, 0FFh indicating end of envelope.
       (after which note fade applies)

      Node data: Tick THEN magnitude
*/

/****************************************************************************\
*       struct itInstHeader
*       -------------------
* Description:  Impulse Tracker Module instrument header
\****************************************************************************/

typedef struct
{
    char        signature[4];           /* "IMPI" */
    char        DOSfileName[12];
    U8          zero;
    U8          NNA;
    U8          DCT;
    U8          DCA;
    U16         fadeOut;
    U8          PPS;
    U8          PPC;
    U8          GbV;
    U8          DfP;
    U8          RV;
    U8          RP;
    U16         trackerVersion;
    U8          numSamples;
    U8          reserved2;
    char        instName[26];
    char        reserved3[6];
    U8          noteSampleTable[240];
    itEnvelope  volEnvelope;
    itEnvelope  panEnvelope;
    itEnvelope  pitchEnvelope;
} itInstHeader;

/*
                           Impulse Instrument Format

        0   1   2   3   4   5   6   7   8   9   A   B   C   D   E   F
      +---+---+---+---+-----------------------------------------------+
0000: |'I'|'M'|'P'|'I'| DOS FileName (12345678.123)                   |
      +---+---+---+---+-------+---+---+---+---+---+---+-------+---+---+
0010: |00h|NNA|DCT|DCA|FadeOut|PPS|PPC|GbV|DfP|RV |RP |TrkVers|NoS| x |
      +---+---+---+---+-------+---+---+---+---+---+---+-------+---+---+
0020: | Instrument Name, max 26 bytes, includes NUL...................|
      +---------------------------------------+---+---+---+---+---+---+
0030: |.......................................| x | x | x | x | x | x |
      +---------------------------------------+---+---+---+---+---+---+
0040: | Note-Sample/Keyboard Table, Length = 240 bytes................|
      +---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+

      +---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+
0130: | Envelopes.....................................................|
      +---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+

        NNA = New Note Action
                0 = Cut                 1 = Continue
                2 = Note off            3 = Note fade

        DCT = Duplicate Check Type
                0 = Off                 1 = Note
                2 = Sample              3 = Instrument

        DNA: Duplicate Check Action
                0 = Cut
                1 = Note Off
                2 = Note fade

      FadeOut:  Ranges between 0 and 128, but the fadeout "Count" is 512.
                See the Last section on how this works.
                Fade applied when:
                1) Note fade NNA is selected and triggered (by another note)
                2) Note off NNA is selected with no volume envelope
                   or volume envelope loop
                3) Volume envelope end is reached

        PPS: Pitch-Pan separation, range 0->64, &128 => Don't use
        PPC: Pitch-Pan center: C-0 to B-9 represented as 0->119 inclusive

        GbV: Global Volume, 0->128
        DfP: Default Pan, 0->64, &128 => Don't use
        RV: Random volume variation
        RP: Random panning variation (not implemented yet)

      TrkVers:  Tracker version used to save the instrument. This is only
                used in the instrument files.
      NoS:      Number of samples associated with instrument. This is only
                used in the instrument files.

      Note-Sample/Keyboard Table.
       Each note of the instrument is first converted to a sample number
       and a note (C-0 -> B-9). These are stored as note/sample pairs
       (note first, range 0->119 for C-0 to B-9, sample ranges from
       1-99, 0=no sample)

*/

/****************************************************************************\
*       struct itSample
*       ---------------
* Description:  Impulse Tracker Module sample header
\****************************************************************************/

typedef struct
{
    char        signature[4];           /* "IMPS" */
    char        DOSfileName[12];
    U8          zero;
    U8          GvL;
    U8          flags;
    U8          volume;
    char        sampleName[26];
    U16         convert;
    U32         length;
    U32         loopStart;
    U32         loopEnd;
    U32         C5speed;
    U32         susLoopStart;
    U32         susLoopEnd;
    U32         samplePointer;
    U8          vibSpeed;
    U8          vibDepth;
    U8          vibRate;
    U8          vibType;
} itSampleHeader;

/*
                             Impulse Sample Format

        0   1   2   3   4   5   6   7   8   9   A   B   C   D   E   F
      +---+---+---+---+-----------------------------------------------+
0000: |'I'|'M'|'P'|'S'| DOS Filename (12345678.123)                   |
      +---+---+---+---+-----------------------------------------------+
0010: |00h|GvL|Flg|Vol| Sample Name, max 26 bytes, includes NUL.......|
      +---+---+---+---+---------------------------------------+---+---+
0020: |.......................................................|Convert|
      +---------------+---------------+---------------+-------+---+---+
0030: | Length        | Loop Begin    | Loop End      | C5Speed       |
      +---------------+---------------+---------------+---+---+---+---+
0040: | SusLoop Begin | SusLoop End   | SamplePointer |ViS|ViD|ViR|ViT|
      +---------------+---------------+---------------+---+---+---+---+

The cache file has the following pieces of information added on:

        0   1   2   3   4   5   6   7   8   9   A   B   C   D   E   F
      +---------------+-------+-------+---+---------------------------+
0050: | File Size     | Date  | Time  |Fmt|...........................|
      +---------------+-------+-------+---+---------------------------+

        Fmt. 0 = unchecked. 1 = directory, 2 = it sample, 3 = st sample


      GvL:      Global volume for instrument, ranges from 0->64
      Flg:      Bit 0. On = sample associated with header.
                Bit 1. On = 16 bit, Off = 8 bit.
                Bit 2. On = stereo, Off = mono. Stereo samples not supported yet
                Bit 3. Reserved
                Bit 4. On = Use loop
                Bit 5. On = Use sustain loop
                Bit 6. On = Ping Pong loop, Off = Forwards loop
                Bit 7. On = Ping Pong Sustain loop, Off = Forwards Sustain loop
      Vol:      Default volume for instrument

      Length:   Length of sample in no. of samples NOT no. of bytes
      LoopBeg:  Start of loop (no of samples in, not bytes)
      Loop End: Sample no. AFTER end of loop
      C5Speed:  Number of bytes a second for C-5 (ranges from 0->9999999)
      SusLBeg:  Start of sustain loop
      SusLEnd:  Sample no. AFTER end of sustain loop

      SmpPoint: 'Long' Offset of sample in file.

      ViS:      Vibrato Speed, ranges from 0->64
      ViD:      Vibrato Depth, ranges from 0->64
      ViT:      Vibrato waveform type.
                        0=Sine wave
                        1=Ramp down
                        2=Square wave
                        3=Random (speed is irrelevant)
      ViR:      Vibrato Rate, rate at which vibrato is applied (0->64)

        The depth of the vibrato at any point is worked out in the following
        way:
          Every processing cycle, the following occurs:
                1) Mov AX, [SomeVariableNameRelatingToVibrato]
                2) Add AL, Rate
                3) AdC AH, 0
                4) AH contains the depth of the vibrato as a fine-linear slide.
                5) Mov [SomeVariableNameRelatingToVibrato], AX  ; For the next
                                                                ; cycle.

        For those that don't understand assembly, then the depth is
        basically the running-sum of the rate divided by 256.

        Sample vibrato uses a table 256-bytes long

   Convert:
        Bit 0:
         Off: Samples are unsigned   } IT 2.02 and above use signed samples
          On: Samples are signed     } IT 2.01 and below use unsigned samples
        Bit 1:
         Off: Intel lo-hi byte order for 16-bit samples    } Safe to ignore
         On: Motorola hi-lo byte order for 16-bit samples  } these values
        Bit 2:                                             } for now...
         Off: Samples are stored as PCM values             }
          On: Samples are stored as Delta values           }
        Bit 3:
          On: Samples are stored as byte delta values      
        

	 */


/*                                  Mathematics

Abbreviations:
 FV = Final Volume (Ranges from 0 to 128). In versions 1.04+, mixed output
      devices are reduced further to a range from 0 to 64 due to lack of
      memory.
 Vol = Volume at which note is to be played. (Ranges from 0 to 64)
 SV = Sample Volume (Ranges from 0 to 64)
 CV = Channel Volume (Ranges from 0 to 64)
 GV = Global Volume (Ranges from 0 to 128)
 VEV = Volume Envelope Value (Ranges from 0 to 64)

In Sample mode, the following calculation is done:
 FV = Vol * SV * CV * GV / 262144               ; Note that 262144 = 2^18
                                                ; So bit shifting can be done.

In Instrument mode the following procedure is used:

 1) Update volume envelope value. Check for loops / end of envelope.
 2) If end of volume envelope (ie. position >= 200 or VEV = 0FFh), then turn
        on note fade.
 3) If notefade is on, then NoteFadeComponent (NFC) = NFC - FadeOut
        ; NFC should be initialised to 512 when a note is played.
 4) FV = Vol * SV * CV * GV * VEV * NFC / 8589934592
                                                ; Note that 8589934592 = 2^33

Linear slides work like this:
 Final frequency = Original frequency * 2^(SlideValue/768)

(I used a lookup table for the multipliers here)

For command Exx, SlideValue = -4*EffectValue
For command EEx, SlideValue = -EffectValue
For command Fxx, SlideValue = 4*EffectValue
For command FEx, SlideValue = EffectValue

Note that sample vibrato always uses Linear slides.

Notes about effects (as compared to other module formats)

C               This is now in *HEX*. (Used to be in decimal in ST3)
E/F/G/H/U       You need to check whether the song uses Amiga/Linear slides.
H/U             Vibrato in Impulse Tracker is two times finer than in
                any other tracker and is updated EVERY tick.
                If "Old Effects" is *ON*, then the vibrato is played in the
                normal manner (every non-row tick and normal depth)
E/F/G           These commands ALL share the same memory.
Oxx             Offsets to samples are to the 'xx00th' SAMPLE. (ie. for
                16 bit samples, the offset is xx00h*2)
                Oxx past the sample end will be ignored, unless "Old Effects"
                is ON, in which case the Oxx will play from the end of the
                sample.
Yxy             This uses a table 4 times larger (hence 4 times slower) than
                vibrato or tremelo. If the waveform is set to random, then
                the 'speed' part of the command is interpreted as a delay.

If you read through this document and there are ANY points which you have
troubles with (and have to try out), then let me know - because someone
else will have the same questions - and I'd like to make this DOC as easy
to understand as possible.

                                  General Info

The player in Impulse Tracker 'allocates' channels to notes whenever they
are *PLAYED*. In sample mode, the allocation is simple:
                        Virtual Channel = 'Host' channel

In instrument mode, the following procedure is used:

    Check if channel is already playing ---Yes--> set 'background' flag on.
                |                                 'Trigger' NNA. If NNA=cut,
                No                                then use this virtual
                |                                 channel.
                |                                          |
                |<------------------ else -----------------/
                |
                v
    Search and find the first non-active virtual channel.
                |
    Non-active channel found? ----Yes----> Use this for playback.
                |
                No
                |
                v
   Search through and find the channel of lowest volume that is in the
   'background' (ie. no longer controlled directly)
                |
   Background channel found? ----Yes----> Use this for playback.
                |
                No
                |
                v
   Return error - the note is *NOT* allocated a channel, and hence is not
   played.

   Actually, this is quite a simple process... just that it's another of
   those 'hassles' to have to write...

*/



#endif


/*
 * $Log: it.h,v $
 * Revision 1.5  1997/07/31 10:56:42  pekangas
 * Renamed from MIDAS Sound System to MIDAS Digital Audio System
 *
 * Revision 1.4  1997/06/30 10:59:51  jpaana
 * Major rewrite on some parts for better IT-support
 *
 * Revision 1.3  1997/06/11 03:47:37  jpaana
 * Updated to IT 2.11
 *
 * Revision 1.2  1997/06/11 00:03:00  jpaana
 * CVS fixes
 *
 *
*/



