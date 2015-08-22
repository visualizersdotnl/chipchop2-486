
** First build? **

Project is built in DOS.

1. Have at least 16MB of RAM at your disposal to build (4MB to run). No problem of course if you're using DOSBox.
2. Before building and running the disk itself, go to /src/tracks and type 'wmake' then 'build' to build track archive.
3. Now back to /src, then type 'wmake'.

... Now, read on :)

PC MS-DOS (target: ~486DX2-66) port of Chip Chop #16 by Desire (Amiga music disk).
All tools (except DOSBox) included!

Content that's ours falls under the Creative Commons license version 4.

DOSBox instructions for Windows:
- Install 0.74 (at least).
- Launch with local .conf file: DOSBox.exe -conf /.../chipchop2-486/dosbox-0.74.conf
- Match mount path at bottom of .conf file.

I supplied a shortcut that should work if you, like most people, have DOSBox installed on your C drive.

DOSBox instructions for OSX:
- Install 0.74 (at least).
- Launch with local .conf file: open -a DOSBox --args -conf ~/.../chipchop2-486/dosbox-0.74-osx.conf
- Match mount path at bottom of .conf file.

To build (to mdisk.exe), simply type:
- Release: 'wmake'
- Debug: 'wmake debug=1'
- Clean: 'wmake clean'
- Pack: 'wmake pack', renames mdisk.exe to cchop2.exe!

Some files and folders are UPPER CASE. 
Looks a bit messy, it's because of DOS.

Have fun!


Credits:
- Amiga code: Tim.
- PC code: Superplek.
- Graphics: Hammerfist, Alien, Lowlife.
- 486 Accolade modules by Triace.
- Special thanks to Metin Seven for fixing up the Accolade logo for me.

Third party:
- PMODE/W by good old Tran.
- UPX and MiniLZO by Markus Oberhumer & those other guys.

All fonts, images & tracks are property of their respective author(s).

