
C  = wcc386
CC = wpp386
CL = wlink

# MIDAS paths.
MIDASDIR = midas
LIBDIR = $(MIDASDIR)\lib\dos\wcretail
MIDASINCS = $(MIDASDIR)\include
MIDASLIB = $(LIBDIR)\midas.lib

!ifdef debug

# Debug compile (in order):
# - Flat memory model.
# - Disable all optimization.
# - Level 3 (highest) debug information.
# - Inline FPU intrinsics (where available, otherwise emulate).
CC_FLAGS = -mfi -od -d3 -fpi -i$(MIDASINCS) 

!else

# Profile compile (in order):
# - Flat memory model.
# - Debug information.
# - 486 register call profile.
# - Enable branch prediction.
# - Expand inline.
# - Enable "expensive" optimizations.
# - Expand intrinsics inline.
# - Loop unrolling.
# - Inline math.
# - Reorder instructions.
# - Favor execution time.
# - Inline FPU intrinsics (where available, otherwise emulate).
# CC_FLAGS = -mfi -d3 -4r -ob -oe32 -oh -oi -ol+ -om -or -ot -fpi -i$(MIDASINCS)

# Release compile (in order):
# - Place constants in code segment.
# - Flat memory model.
# - No debug information.
# - 486 register call profile.
# - Enable branch prediction.
# - Expand inline.
# - Enable "expensive" optimizations.
# - Expand intrinsics inline.
# - Loop unrolling.
# - Inline math.
# - Reorder instructions.
# - Favor execution time.
# - Inline FPU intrinsics (where available, otherwise emulate).
CC_FLAGS = -zc -mfi -d0 -4r -ob -oe32 -oh -oi -ol+ -om -or -ot -fpi -i$(MIDASINCS)

!endif

TARGET = mdisk.exe
OBJLIST = main.obj minilzo\minilzo.obj

$(TARGET) : $(OBJLIST) $(MIDASLIB)
	$(CL) name $(TARGET) system pmodew library $(MIDASLIB) file main,minilzo\minilzo
#	wlink NAME mdisk.exe SYSTEM pmodew LIBRARY $(MIDASLIB) FILE main,minilzo OPTION map

main.obj : main.cpp tracks\tracks.h
	$(CC) main.cpp $(CC_FLAGS)

# Build Oberhumer's MiniLZO.
minilzo\minilzo.obj : minilzo\minilzo.c
	cd minilzo
	wmake
	cd ..

# Remove all executables, maps and generated.
clean : .symbolic
	rm -f *.obj *.map *.exe

# Pack using UPX (maximum compression).
pack : .symbolic
	upx -9 -v mdisk.exe
	ren mdisk.exe cchop2.exe
