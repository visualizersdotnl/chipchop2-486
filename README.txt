
** OUTDATED, PLEASE UPDATE **

PC MS-DOS (486DX2-66) port of Chip Chop #16 by Desire (Amiga music disk).
All tools & libraries included!

This project is built in MS-DOS.

Have at least 8MB of RAM at your disposal to build (4MB to run). 
No problem of course if you're using DOSBox or any VM.

DOSBox instructions for Windows:
- Install 0.74 or later.
- Modify mount path at bottom of dosbox-0.74-win32.conf
- Launch with .conf file: DOSBox.exe -conf dosbox-0.74-win32.conf

DOSBox instructions for Linux & OSX:
- Install 0.74 or later.
- Modify mount path at bottom of dosbox-0.74-linux.conf
- Launch with .conf file: open -a DOSBox --args -conf dosbox-0.74-linux.conf

** First time? **
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

Third party:
- PMODE/W by Tran
- MiniLZO by Markus Oberhumer
- MIDAS Digital Audio System by Housemarque Inc.

Tools:
- OpenWatcom
- Netwide Assembler
- UPX by Markus Oberhumer

All fonts, images & tracks are property of their respective author(s).

Have fun!
