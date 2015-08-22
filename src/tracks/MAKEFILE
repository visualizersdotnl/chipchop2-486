
CC = wpp386

# Debug compile:
# - Flat memory model.
CC_FLAGS = -mfi

build.exe : build.obj ..\minilzo\minilzo.obj
	wlink NAME build.exe SYSTEM pmodew FILE build,..\minilzo\minilzo

build.obj : build.cpp tracks.h
	$(CC) build.cpp $(CC_FLAGS)

# Build Oberhumer's MiniLZO.
..\minilzo\minilzo.obj : ..\minilzo\minilzo.c
	cd ..\minilzo
	wmake
	cd ..\tracks

# Remove all executables, maps and generated.
clean : .symbolic
	rm -f *.obj *.map *.exe
