#*      makefile
#*
#* Make script, used just for cleanup
#*
#* $Id: makefile,v 1.7 1997/07/31 10:56:27 pekangas Exp $
#*
#* Copyright 1996,1997 Housemarque Inc.
#*
#* This file is part of MIDAS Digital Audio System, and may only be
#* used, modified and distributed under the terms of the MIDAS
#* Digital Audio System license, "license.txt". By continuing to use,
#* modify or distribute this file you indicate that you have
#* read the license and understand and accept it fully.
#*

# Include build options:
BLDOPTSDIR=../src/bldopts
include $(BLDOPTSDIR)/bldopts.mak


# Default target:
all :
	$(REM) Targets available here: clean, distclean, cvsclean

# Cleanup:
clean distclean cvsclean :
	$(CLEANALL)
	$(MAKE) -C dsound $@
	$(MAKE) -C midpnt $@
	$(MAKE) -C midp_linux $@
	$(MAKE) -C midp_dos $@
	$(MAKE) -C dos $@
	$(MAKE) -C common $@
	$(MAKE) -C delphi $@



.PHONY : clean distclean cvsclean
