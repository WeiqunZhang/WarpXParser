AMREX_HOME ?= ../amrex

DEBUG	= FALSE

DIM	= 3

COMP    = gnu

USE_MPI   = FALSE
USE_OMP   = FALSE

TINY_PROFILE = TRUE

include $(AMREX_HOME)/Tools/GNUMake/Make.defs

include ./Make.package
include $(AMREX_HOME)/Src/Base/Make.package

include $(AMREX_HOME)/Tools/GNUMake/Make.rules


WP_TMPFILES = wp_parser.lex.h wp_parser.lex.c wp_parser.tab.h wp_parser.tab.c

wp_parser.lex.h wp_parser.lex.c wp_parser.tab.h wp_parser.tab.c: wp_parser.l wp_parser.y wp_parser_y.h wp_parser_y.c
	@sleep 2
	bison -d wp_parser.y
	flex -o wp_parser.lex.c --header-file=wp_parser.lex.h wp_parser.l

clean::
	$(RM) $(WP_TMPFILES)

