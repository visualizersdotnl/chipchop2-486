
CC = wpp386

!ifdef debug

# Debug compile (in order):
# - Flat memory model.
# - Disable all optimization.
# - Level 3 (highest) debug information.
# - Inline FPU intrinsics (where available, otherwise emulate).
CC_FLAGS = -mfi -od -d3 -fpi -i$(MIDASINCS) 

!else

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
CC_FLAGS = -zc -mfi -d0 -4r -ob -oe32 -oh -oi -ol+ -om -or -ot -fpi

!endif

minilzo.obj : minilzo.c
	$(CC) minilzo.c $(CC_FLAGS)

# Remove all executables, maps and generated.
clean : .symbolic
	rm -f *.obj
