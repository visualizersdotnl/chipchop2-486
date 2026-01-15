
PC MS-DOS (486DX2-66) port of Chip Chop #16 by Desire (Amiga music disk).

# IMPORTANT: currently working on AHX (Amiga-specific) module replay, so things are "in shambles".

All tools & libraries included!

This project is built in MS-DOS.

Have at least 8MB of RAM at your disposal to build (4MB to run, approx.).
No problem of course if you're using DOSBox or any VM.


For now, at least in Linux and OSX alike:
- Make sure DOSBox installed (0.74 or later), DOSBox-X if you want the debugger.
  + Tip for OSX users: 'sudo ln -s /Applications/DOSBox.app/Contents/MacOS/DOSBox(-X) /usr/local/bin/dosbox(-x)'
- Launch into VSCode or something that can/will do the same for you, if you must
- Open this folder, then open up a terminal (inside VSCode)
- Run 'startenv' script
- From there on out you're off to the races: read the outdated manual below

Shouldn't be much different for Windows though I haven't used that setup for a while so it'll need a little love.
As a bonus this is also usable on a *real* MS-DOS machine.

I'm not at all very satisfied with the implementation of this port, but it has some interesting and proven/stable code in there.
I'll toy with it now and then and see where it goes (if it goes anywhere).

There are a few niceties worth noting:
- I dive into the illustrious ModeX (only to demonstrate that it can be quite cumbersome compared to plain old chunky 320x200 'mode 13').
- Yes, there is a fully embedded install of OpenWatcom ready to go with all bells and whistles.
- A working MIDAS build for easy audio.
- Go see what lives in '/bonus' <3

What would be nice to have:
- Check out possible use of a GDB server in DOSBox (https://github.com/hezi/dosbox-x-gdb)
- A VESA library (you know, those we all made ourselves and were a major PITA when trying to get that new demoscene release to run).
  + Plus UniVBE :-)
- An Ad-Lib (is that the correct marketing-approved spelling?) player of sorts for that glorious PC BBS intro sound.
- More tools of the 1990s MS-DOS trade that I can't recall right now.


** First time and you'd like to build the music disk?

Head over to /disk/tracks and type 'wmake' then 'build' to build the track archive.

Then open up /main.cpp and scroll down until you hit DEVELOPMENT_MODE, read about it, undef. it if it isn't...
... and perhaps consider DUMP_C_DATA as well, then thereafter you can run w/o DEVELOPMENT_MODE.

See, the disk itself parses the graphics data, processes it, and spits it out into a source file for easy
linkage. It is crude and should have been done using a separate build tool, yes, I agree.

Tracks *are* processed and compressed into a nifty little archive like they should.

To build the disk (mdisk.exe), simply type:
- Release: 'wmake'
-   Debug: 'wmake debug=1'
-   Clean: 'wmake clean'
-    Pack: 'wmake pack' (renames mdisk.exe to cchop2.exe)

Credits:
- Amiga code: Tim
- PC code: Superplek
- Graphics: Hammerfist, Alien, Lowlife
- 486 Accolade modules: Triace
- Special thanks to Metin Seven for fixing up the Accolade logo for me.


** Credit(s)

Third party:
- PMODE/W by Tran
- MiniLZO by Markus Oberhumer
- MIDAS Digital Audio System by Housemarque Inc.

Tools courtesy of:
- Watcom (courtesy of OpenWatcom)
- Netwide Assembler
- UPX by Markus Oberhumer

All original music disk fonts, images & tracks are property of their respective author(s) as of 11/01/2026.

Have fun!
