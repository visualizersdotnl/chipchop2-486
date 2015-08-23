
** First time? **

This project is built in (MS-)DOS.

1. Have at least 8MB of RAM at your disposal to build (4MB to run). No problem of course if you're using DOSBox or a VM.
2. Before building and running the disk itself, go to /src/tracks and type 'wmake' then 'build' to build track archive.
3. Now back to /src, then type 'wmake'.

... Now read on :)

PC MS-DOS (target: ~486DX2-66) port of Chip Chop #16 by Desire (Amiga music disk).
All tools (except DOSBox) & libraries included!

DOSBox instructions for Windows:
- Install 0.74 or later.
- Modify mount path at bottom of dosbox-0.74.conf
- Launch with .conf file: DOSBox.exe -conf <path>/dosbox-0.74.conf

I supplied a shortcut that should work if you, like most people, have DOSBox installed on your C drive.

DOSBox instructions for OSX:
- Install 0.74 or later.
- Modify mount path at bottom of dosbox-0.74-osx.conf
- Launch with .conf file: open -a DOSBox --args -conf ~/<path>/dosbox-0.74-osx.conf

To build (mdisk.exe), simply type:
- Release: 'wmake'
- Debug: 'wmake debug=1'
- Clean: 'wmake clean'
- Pack: 'wmake pack' (renames mdisk.exe to cchop2.exe!)

Some files are UPPER CASE, this is because they were created in DOS.
Looks a bit messy, yes.

Credits:
- Amiga code: Tim.
- PC code: Superplek.
- Graphics: Hammerfist, Alien, Lowlife.
- 486 Accolade modules by Triace.
- Special thanks to Metin Seven for fixing up the Accolade logo for me.

Third party:
- PMODE/W by Tran.
- MiniLZO by Markus Oberhumer.
- MIDAS Digital Audio System by Housemarque Inc.

Tools:
- OpenWatcom.
- Netwide Assembler.
- UPX by Markus Oberhumer.

All fonts, images & tracks are property of their respective author(s).

Have fun!
