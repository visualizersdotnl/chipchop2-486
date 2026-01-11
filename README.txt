

PC MS-DOS (486DX2-66) port of Chip Chop #16 by Desire (Amiga music disk).
All tools & libraries included!

This project is built in MS-DOS.

Have at least 8MB of RAM at your disposal to build (4MB to run). 
No problem of course if you're using DOSBox or any VM.

Plan:
- Set up with VSCode (see issue list / WIP)
- Get a proper debug experience (preferably remote) working
- Maybe another project?

For now, at least in Linux and OSX alike (workflow should adapt to Windows in a jiffy)
- Make sure DOSBox installed (0.74 or later)
- Launch into VSCode or something that can/will do the same for you, if you must
- Open his folder
- Open a local terminal window, and type 'dosbox --conf dosbox-0.74-linux.conf' to get the DOSBox environment started
- From there on out you're off to the races: read the outdated manual below
- Be a hero and implemented replay for those .AHX modules :-) 

I'm not at all very satisfied with the implementation of this port, but it has some interesting and proven/stable code
in there that you can build upon.

There are a few niceties worth noting:
- I dive into the illustrious ModeX (only to demonstrate that it can be quite cumbersome compared to plain old chunky 320x200 'mode 13').
- Yes, there is a fully embedded install of OpenWatcom ready to go with all bells and whistles.
- A working MIDAS build for easy audio.
- Go see what lives in '/bonus' <3

What would be nice to have:
- A VESA library (you know, those we all made ourselves and were a major PITA when trying to get that new demoscene release to run).
- An Ad-Lib (is that the correct marketing-approved spelling?) player of sorts for that glorious PC BBS intro sound.
- More tools of the 1990s MS-DOS trade that I can't recall right now.


** First time and you'd like to build the music disk?

Head over to /disk/tracks and type 'wmake' then 'build' to build the track archive.

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

Tools:
- OpenWatcom
- Netwide Assembler
- UPX by Markus Oberhumer

All original music disk fonts, images & tracks are property of their respective author(s) as of 11/01/2026.

Have fun!
