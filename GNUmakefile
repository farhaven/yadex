#
#	Makefile for Yadex
#	Copyright © André Majorel 1998-2003.
#	AYM 1998-06-10
#

# ATTENTION : GNU MAKE IS REQUIRED ! This makefile uses pattern
# rules, addprefix, addsuffix, etc. It's not named "GNUmakefile"
# for nothing.

########################################################################
#
#	Definitions that only hackers
#	might want to change
#
########################################################################

# The name of the directory where objects and
# binaries are put. I include the output of
# "uname -a" to make it easier for me to build
# Yadex for different platforms from the same
# source tree.
SYSTEM := $(shell echo `uname -n`_`uname -a | cksum` | tr -dc '[:alnum:]._-')
OBJDIR             = obj/0
DOBJDIR            = dobj/0
OBJPHYSDIR         = obj/$(SYSTEM)
DOBJPHYSDIR        = dobj/$(SYSTEM)
OBJDIR_ATCLIB      = $(OBJDIR)/atclib
DOBJDIR_ATCLIB     = $(DOBJDIR)/atclib
OBJPHYSDIR_ATCLIB  = $(OBJPHYSDIR)/atclib
DOBJPHYSDIR_ATCLIB = $(DOBJPHYSDIR)/atclib
OBJDIR_COMPAT      = $(OBJDIR)/compat
DOBJDIR_COMPAT     = $(DOBJDIR)/compat
OBJPHYSDIR_COMPAT  = $(OBJPHYSDIR)/compat
DOBJPHYSDIR_COMPAT = $(DOBJPHYSDIR)/compat

# Create all directories and make symlinks to
# config.cc and config.h. Doing it at the start
# makes things much simpler later on.
DUMMY := $(shell							\
	mkdir -p $(OBJPHYSDIR)  $(OBJPHYSDIR_ATCLIB) $(OBJPHYSDIR_COMPAT);			\
	mkdir -p $(DOBJPHYSDIR) $(DOBJPHYSDIR_ATCLIB) $(DOBJPHYSDIR_COMPAT);			\
	[ ! -h $(OBJDIR)  ] || rm $(OBJDIR);				\
	[ ! -h $(DOBJDIR) ] || rm $(DOBJDIR);				\
	ln -s $(SYSTEM) $(OBJDIR);					\
	ln -s $(SYSTEM) $(DOBJDIR);					\
	)

include $(OBJDIR)/Makefile.config

########################################################################
#
#	Definitions that end users
#	might want to change
#
########################################################################

# Which OS ?
OS := $(shell uname -s | tr A-Z a-z)

# Where your X11 libraries and headers reside.
# Current rule:
# - AIX has them in /usr/lpp/X11/{lib,include},
# - Solaris has them in /usr/openwin/{lib,include},
# - all other unices in /usr/X11R6/{lib,include}.
ifeq ($(findstring $(OS), aix), $(OS))
  X11LIBDIR     = /usr/lpp/X11/lib
  X11INCLUDEDIR = /usr/lpp/X11/include
else
  ifeq ($(findstring $(OS), solaris sunos), $(OS))
    X11LIBDIR     = /usr/openwin/lib
    X11INCLUDEDIR = /usr/openwin/include
  else
    X11LIBDIR     = /usr/X11R6/lib
    X11INCLUDEDIR = /usr/X11R6/include
  endif
endif

# $(CC) and $(CXX) are the C and C++ compiler respectively. They're
# normally autodetected by ./configure and passed to make through
# obj/0/Makefile.config.
#CC  =
#CXX =

# Options used when compiling Atclib.
CFLAGS  = -pedantic
CFLAGS += -Wall
CFLAGS += -Werror
CFLAGS += -Wno-variadic-macros

# Options used when compiling and linking Yadex.
# ld is invoked through the C++ compiler so
# LDFLAGS should not contain options that mean
# something to the C++ compiler.
CXXFLAGS  = -std=c++11
CXXFLAGS += -pedantic
CXXFLAGS += -Wall
CXXFLAGS += -Werror
CXXFLAGS += -Wno-variadic-macros
#CXXFLAGS += -DWHITE_BACKGROUND
LDFLAGS  = -g

# Options used to compile and link the debugging
# targets. Not used by normal end-user targets.
# Unlike CFLAGS, CXXFLAGS and LDFLAGS, assume
# GCC/EGCS.
DCFLAGS		= $(CFLAGS) -g
DCXXFLAGS	= $(CXXFLAGS) -g

DLDFLAGS	= -g

########################################################################
#
#	Definitions that only hackers
#	might want to change
#
########################################################################

MAKEFILE = GNUmakefile
VERSION := $(shell cat VERSION)
VERPREV := $(shell test -f VERPREV && cat VERPREV)

# All the modules of Yadex without path or extension.
MODULES_YADEX =								\
	acolours	aym		bench		bitvec			\
	cfgfile		checks		colour		\
	config		credits		\
	decorate 										\
	dependcy	dialog		disppic		drawmap		\
	edisplay	editgrid	editlev		editloop	\
	editobj		editsave	endian		editzoom	\
	entry		entry2		events		flats		\
	game		gcolour1	gcolour2	gcolour3	\
	geom		gfx		gfx2		gfx3			\
	gotoobj		help1		help2		highlt		\
	img		imgscale	imgspect	infobar			\
	input		l_align		l_centre	l_flags		\
	l_misc		l_prop		l_unlink	l_vertices	\
	levels		lists		locate		lumpdir		\
	macro		menubar		menu		\
	mkpalette	names \
	objects		objinfo		oldmenus	palview		\
	patchdir	pic2img		prefer		s_centre	\
	s_door		s_lift		s_linedefs	s_merge		\
	s_misc		s_prop		s_slice		s_split		\
	s_swapf		s_vertices	sanity				    \
	selbox		selectn		selpath		selrect		\
	serialnum	spritdir	sticker		\
 	r_render	r_images							\
	t_centre	t_flags		t_prop		t_spin		\
	textures	things		trace		v_centre	\
	v_merge		v_polyg		vectext		verbmsg		\
	version		wadfile		wadlist		wadnamec	\
	wadres		wads		wads2		warn		\
	windim		x_centre	x_exchng	x_hover		\
	x_mirror	x_rotate	x11			xref		\
	yadex		ytime

# Compatibility modules
MODULES_COMPAT =
ifneq "$(HAVE_STRL)" "1"
	MODULES_COMPAT += strlcpy strlcat
endif
ifneq "$(HAVE_ARC4RANDOM)" "1"
	MODULES_COMPAT += arc4random
endif

# All the modules of Atclib without path or extension.
MODULES_ATCLIB =							\
	al_adigits	al_aerrno	al_astrerror	al_fana		\
	al_fnature	al_lateol	al_lcount	al_lcreate	\
	al_ldelete	al_ldiscard	al_lgetpos	al_linsert	\
	al_linsertl	al_llength	al_lpeek	al_lpeekl	\
	al_lpoke	al_lpokel	al_lptr		al_lread	\
	al_lreadl	al_lrewind	al_lseek	\
	al_lstep	al_ltell	al_lwrite	al_lwritel	\
	al_sapc		al_scpslower	\
	al_sisnum	al_strolc

# The source files of Yadex and Atclib
SRC_YADEX  = $(addprefix src/,     $(addsuffix .cc, $(MODULES_YADEX)))
SRC_COMPAT = $(addprefix compat/,  $(addsuffix .c,  $(MODULES_COMPAT)))
SRC_ATCLIB = $(addprefix atclib/,  $(addsuffix .c,  $(MODULES_ATCLIB)))

# The headers of Yadex and Atclib
HEADERS_YADEX  := $(wildcard src/*.h)
HEADERS_ATCLIB = atclib/atclib.h
HEADERS_COMPAT = compat/compat.h

# All the source files, including the headers.
SRC = $(filter-out src/config.cc, $(SRC_YADEX))				\
      $(filter-out src/config.h, $(HEADERS_YADEX))			\
      $(SRC_ATCLIB) $(HEADERS_ATCLIB)	\
      $(SRC_COMPAT) $(HEADERS_COMPAT)

# The files on which youngest is run.
SRC_NON_GEN = $(filter-out src/credits.cc src/prefix.cc src/version.cc, $(SRC))

# The object files
OBJ_CONFIG  =# $(OBJDIR)/config.o
DOBJ_CONFIG =# $(DOBJDIR)/config.o
OBJ_YADEX   = $(addprefix $(OBJDIR)/,  $(addsuffix .o, $(MODULES_YADEX)))
DOBJ_YADEX  = $(addprefix $(DOBJDIR)/, $(addsuffix .o, $(MODULES_YADEX)))
OBJ_ATCLIB  = $(addprefix $(OBJDIR_ATCLIB)/,  $(addsuffix .o,$(MODULES_ATCLIB)))
DOBJ_ATCLIB = $(addprefix $(DOBJDIR_ATCLIB)/, $(addsuffix .o,$(MODULES_ATCLIB)))
OBJ_COMPAT  = $(addprefix $(OBJDIR_COMPAT)/,  $(addsuffix .o,$(MODULES_COMPAT)))
DOBJ_COMPAT = $(addprefix $(DOBJDIR_COMPAT)/, $(addsuffix .o,$(MODULES_COMPAT)))

# The game definition files.
YGD = $(addprefix ygd/,							\
	doom.ygd	doom02.ygd	doom04.ygd	doom05.ygd	\
	doom2.ygd	doompr.ygd	heretic.ygd	hexen.ygd	\
	strife.ygd	strife10.ygd	zdoom.ygd)

# Files that are used with scripts/process to
# generate files that go in the doc/ directory
# and are NOT included in the archive.
DOC2_SRC_HTML =				\
	docsrc/advanced.html		\
	docsrc/contact.html		\
	docsrc/credits.html		\
	docsrc/deu_diffs.html		\
	docsrc/editing_docs.html	\
	docsrc/faq.html			\
	docsrc/feedback.html		\
	docsrc/getting_started.html	\
	docsrc/hackers_guide.html	\
	docsrc/help.html		\
	docsrc/index.html		\
	docsrc/keeping_up.html		\
	docsrc/legal.html		\
	docsrc/packagers_guide.html	\
	docsrc/palette.html		\
	docsrc/preview.html		\
	docsrc/reporting.html		\
	docsrc/tips.html		\
	docsrc/trivia.html		\
	docsrc/trouble.html		\
	docsrc/users_guide.html		\
	docsrc/wad_specs.html		\
	docsrc/ygd.html

DOC2_SRC_MISC =				\
	docsrc/yadex.6			\
#	docsrc/yadex.lsm\

# Files that must be put in the distribution
# archive. Most (but not all) are generated from
# $(DOC1_SRC_*) into the base directory.
DOC1 = FAQ readme.md

# Files that go in the doc/ directory and must
# NOT be put in the distribution archive. Most
# are either generated from $(DOC2_SRC_*) or
# symlinked for docsrc/*.png.
DOC2 = $(addprefix doc/, $(PIX) $(notdir $(DOC2_SRC_HTML) $(DOC2_SRC_MISC)))

# Misc. other files that must be put in the
# distribution archive.
MISC_FILES =								\
	cache/copyright.man						\
	cache/copyright.txt						\
	cache/pixlist							\
	cache/srcdate							\
	cache/uptodate							\
	configure							\
	docsrc/copyright						\
	CHANGES								\
	COPYING								\
	COPYING.LIB							\
	GNUmakefile							\
	Makefile							\
	TODO								\
	VERSION								\
	yadex.cfg							\
	yadex.dep

# The images used in the HTML doc. FIXME: "<img"
# and "src=" have to be on the same line. These
# are symlinked into doc/ when $(DOC2) is made.
PIX = $(shell cat cache/pixlist)

# The script files.
SCRIPTS = $(addprefix scripts/,	\
	copyright		\
	ftime.1			\
	ftime.c			\
	mkinstalldirs		\
	notexist.c		\
	process			\
	youngest)

# The patches
PATCHES = $(addprefix patches/,	\
	README			\
	1.5.0_gcc27.diff)

# All files that must be put in the distribution archive.
ARC_FILES = $(sort $(DOC1) $(DOC2_SRC_HTML) $(DOC2_SRC_MISC)\
	$(MISC_FILES) $(addprefix docsrc/, $(PIX)) $(SCRIPTS) $(SRC) $(YGD)\
	$(PATCHES))

# The "root" directory of the archives. The
# basename of the archives is also based on this.
ARCHIVE := yadex-$(VERSION)
ARCPREV := yadex-$(VERPREV)
ARCDIFF := yadex-$(VERSION).diff

# Cosmetic
CFLAGS    := $(strip $(CFLAGS))
DCFLAGS   := $(strip $(DCFLAGS))
CXXFLAGS  := $(strip $(CXXFLAGS))
DCXXFLAGS := $(strip $(DCXXFLAGS))
LDFLAGS   := $(strip $(LDFLAGS))
DLDFLAGS  := $(strip $(DLDFLAGS))


########################################################################
#
#	Targets for
#	end users.
#
########################################################################

.PHONY: all
all: doc yadex.dep yadex $(YGD)

.PHONY: yadex
yadex: $(OBJDIR)/yadex

$(OBJDIR)/yadex: $(OBJ_CONFIG) $(OBJ_YADEX) $(OBJ_ATCLIB) $(OBJ_COMPAT) $(MAKEFILE)
	@echo "** Linking Yadex"
	$(CXX) $(OBJ_CONFIG) $(OBJ_YADEX) $(OBJ_ATCLIB) $(OBJ_COMPAT) -o $@		\
	  -L$(X11LIBDIR) -lX11 -lm -lc $(LDFLAGS)

.PHONY: test
test:
	$(OBJDIR)/yadex $(A)

.PHONY: install
install:
	@scripts/mkinstalldirs $(BINDIR)
	@scripts/mkinstalldirs $(ETCDIR)
	@scripts/mkinstalldirs $(MANDIR)
	@scripts/mkinstalldirs $(MANDIR)/man6
	@scripts/mkinstalldirs $(SHAREDIR)
	install -m 755 $(OBJDIR)/yadex $(BINDIR)/yadex
	install -m 644 doc/yadex.6 $(MANDIR)/man6/yadex.6
	$(foreach y,$(YGD),install -D -m 644 $(y) $(SHAREDIR);)
	install -D -m 644 yadex.cfg $(ETCDIR)
	@echo "---------------------------------------------------------------"
	@echo "  Yadex is now installed."
	@echo
	@echo "  Before you run it, enter the paths to your iwads in"
	@echo "  $(ETCDIR)/yadex.cfg or ~/.yadex/yadex.cfg."
	@echo "  When you're done, type \"yadex\" to start."
	@echo "  If you're confused, take a look at doc/index.html."
	@echo
	@echo "  Happy editing !"
	@echo "---------------------------------------------------------------"

.PHONY: clean
clean:
	rm -f $(OBJ_CONFIG) $(OBJ_YADEX) $(OBJ_ATCLIB) $(OBJ_COMPAT) $(OBJDIR)/yadex
	rm -f $(DOBJ_CONFIG) $(DOBJ_YADEX) $(DOBJ_ATCLIB) $(OBJ_COMPAT) $(DOBJDIR)/yadex
	rm -f $(OBJDIR)/ftime
	rm -f $(OBJDIR)/notexist
	rm -f $(OBJDIR)
	rm -f $(DOBJDIR)
	rm -rf doc

.PHONY: dclean
dclean:
	rm -rf $(DOBJPHYSDIR)
	rm -f $(DOBJDIR)

.PHONY: doc
doc: cache/pixlist docdirs $(DOC1) doc2

# Have to put it separately because evaluation
# of $(DOC2) requires cache/pixlist to exist.
.PHONY: doc2
doc2: $(DOC2)

.PHONY: help
help:
	@echo User targets:
	@echo "make [all]           Build everything"
	@echo "make yadex           Build Yadex"
	@echo "make test [A=args]   Test Yadex"
	@echo "make install         Install everything"
	@echo "make showconf        Show current configuration"
	@echo
	@echo Hacker targets:
	@echo "make dall            Build debug version of everything"
	@echo "make dyadex          Build debug version of Yadex"
	@echo "make dtest [A=args]  Test debug version of Yadex"
	@echo "make dg              Run debug version of Yadex through gdb"
	@echo "make dd              Run debug version of Yadex through ddd"
	@echo "make doc             Update doc"
	@echo "make man             View man page with man"
	@echo "make dvi             View man page with xdvi"
	@echo "make ps              View man page with gv"
	@echo "make dist            Create distribution archive"
	@echo "make save            Create backup archive"


########################################################################
#
#	Targets meant for
#	hackers only.
#
########################################################################

# d: Compile and run
.PHONY: d
d: dyadex dtest

.PHONY: save
save:
	tar -cjvf yadex-$$(date '+%Y%m%d').tar.bz2			\
		--exclude "*.wad"					\
		--exclude "*.zip"					\
		--exclude "core"					\
		--exclude "dos/*"					\
		--exclude "obj"						\
		--exclude "dobj"					\
		--exclude "old/*"					\
		--exclude "*~"						\
		--exclude "*.bak"					\
		--exclude "web/arc"					\
		--exclude yadex-$$(date '+%Y%m%d').tar.bz2		\
		.

.PHONY: dall
dall: yadex.dep dyadex $(YGD)

.PHONY: dyadex
dyadex: $(DOBJDIR)/yadex
	
$(DOBJDIR)/yadex: $(DOBJ_CONFIG) $(DOBJ_YADEX) $(DOBJ_ATCLIB) $(DOBJ_COMPAT) $(MAKEFILE)
	@echo "** Linking Yadex"
	$(CXX) $(DOBJ_CONFIG) $(DOBJ_YADEX) $(DOBJ_ATCLIB) $(DOBJ_COMPAT) -o $@	\
	  -L$(X11LIBDIR) -lX11 -lm -lc $(DLDFLAGS)

.PHONY: dtest
dtest:
	$(DOBJDIR)/yadex $(A)
	gprof $(DOBJDIR)/yadex >gprof.out

.PHONY: dg
dg:
	gdb $(DOBJDIR)/yadex
	
.PHONY: dd
dd:
	ddd $(DOBJDIR)/yadex

.PHONY: asm
asm: $(addprefix $(OBJDIR)/, $(addsuffix .S, $(MODULES_YADEX)))

# Generate the distribution archives. Requires GNU tar,
# GNU cp, gzip and optionally bzip2 (if distbz2 is
# uncommented).
.PHONY: dist
dist: changes distimage distgz distdiff #distbz2
	@echo "** Removing distribution image tree $(ARCHIVE)"
	rm -r $(ARCHIVE)

.PHONY: distimage
distimage: all $(ARC_FILES)
	@echo "** Creating distribution image tree $(ARCHIVE)"
	rm -rf $(ARCHIVE)
	scripts/mkinstalldirs $(ARCHIVE)
	@tar -cf - $(ARC_FILES) | (cd $(ARCHIVE); tar -xf -)

.PHONY: distgz
distgz: distimage
	@echo "** Creating tar.gz distribution"
	tar -czf $(ARCHIVE).tar.gz $(ARCHIVE)

.PHONY: distbz2
distbz2: distimage
	@echo "** Creating .tar.bz2 distribution"
	tar -cIf $(ARCHIVE).tar.bz2 $(ARCHIVE)

.PHONY: distdiff
TMP0    = $$HOME/tmp
TMPPREV = $(TMP0)/$(ARCPREV)
TMPCURR = $(TMP0)/$(ARCHIVE)
distdiff:
	@echo "** Building the diff distribution"
	@echo "Creating the diff"
	rm -rf $(TMPPREV) $(TMPCURR) $(TMPDIFF)
	mkdir -p $(TMP0)
	tar -xzf                  $(ARCHIVE).tar.gz -C $(TMP0)
	tar -xzf ../yadex-arc/pub/$(ARCPREV).tar.gz -C $(TMP0)
	scripts/process docsrc/README.diff >$(TMP0)/$(ARCDIFF)
	echo >>$(TMP0)/$(ARCDIFF)
	cd $(TMP0) && (diff -uaNr $(ARCPREV) $(ARCHIVE) >>$(ARCDIFF) || true)
	@# KLUDGE - On my system, just "! grep" makes make choke
	true; ! grep "^Binary files .* and .* differ" $(TMP0)/$(ARCDIFF)
	gzip -f $(TMP0)/$(ARCDIFF)
	@echo "Verifying the diff"
	cd $(TMPPREV) && gzip -d <../$(ARCDIFF).gz | patch -p1
	@# FIXME remove -N after 1.6 is done, it's there because
	@# uptodate has been moved between 1.5 and 1.6 and since
	@# it's empty it remains in $(ARCPREV).
	cd $(TMP0) && diff -rP $(ARCHIVE) $(ARCPREV)
	mv $(TMP0)/$(ARCDIFF).gz .
	@echo "Cleaning up"
	cd $(TMP0) && rm -rf $(ARCPREV)
	cd $(TMP0) && rm -rf $(ARCHIVE)

.PHONY: showconf
showconf:
	@echo "ARCHIVE            \"$(ARCHIVE)\""
	@echo "BINDIR             \"$(BINDIR)\""
	@echo "CC                 \"$(CC)\""
	@echo "CFLAGS             \"$(CFLAGS)\""
	@echo "CXX                \"$(CXX)\""
	@echo "CXXFLAGS           \"$(CXXFLAGS)\""
	@echo "DCFLAGS            \"$(DCFLAGS)\""
	@echo "DCXXFLAGS          \"$(DCXXFLAGS)\""
	@echo "DLDFLAGS           \"$(DLDFLAGS)\""
	@echo "ETCDIR             \"$(ETCDIR)\""
	@echo "ETCDIRNV           \"$(ETCDIRNV)\""
	@echo "HAVE_STRL          \"$(HAVE_STRL)\""
	@echo "LDFLAGS            \"$(LDFLAGS)\""
	@echo "MANDIR             \"$(MANDIR)\""
	@echo "OS                 \"$(OS)\""
	@echo "PREFIX             \"$(PREFIX)\""
	@echo "SHAREDIR           \"$(SHAREDIR)\""
	@echo "SHAREDIRNV         \"$(SHAREDIRNV)\""
	@echo "SHELL              \"$(SHELL)\""
	@echo "SYSTEM             \"$(SYSTEM)\""
	@echo "VERSION            \"$(VERSION)\""
	@echo "X11INCLUDEDIR      \"$(X11INCLUDEDIR)\""
	@echo "X11LIBDIR          \"$(X11LIBDIR)\""
	@echo "CXX --version      \"`$(CXX) --version`\""
	@echo "CC --version       \"`$(CC) --version`\""
	@echo "shell              \"$$SHELL\""
	@echo "uname              \"`uname`\""


########################################################################
#
#	Internal targets, not meant
#	to be invoked directly
#
########################################################################

# If Makefile.config or config.h don't exist, give a hint...
$(OBJDIR)/Makefile.config:
$(OBJDIR)/config.h:
	@echo "Sorry guv'nor, but... did you run ./configure ?" >&2
	@false

$(OBJDIR)/files_etc.man: $(OBJDIR)/config.etc $(MAKEFILE)
	sed 's/%v/$(VERSION)/g; s,.*,.B &/yadex.cfg,' $< >$@

$(OBJDIR)/files_share.man: $(OBJDIR)/config.share $(MAKEFILE)
	sed 's/%v/$(VERSION)/g; s,.*,.BI &/ game .ygd,' $< >$@

# Dependencies of the modules of Yadex
# -Y is here to prevent the inclusion of dependencies on
# /usr/include/*.h etc. As a side-effect, it generates many
# warnings, hence "2>/dev/null".
#
# The purpose of the awk script is to transform this input :
#
#   src/foo.o: src/whatever.h
#
# into this output :
#
#   obj/0/foo.o: src/whatever.h
#   dobj/0/foo.o: src/whatever.h
#
# Note: the modules of Atclib are not scanned as they all
# depend on $(HEADERS_ATCLIB) and nothing else.

yadex.dep: $(SRC_NON_GEN) src/config.h
	@echo "Generating $@"
	@makedepend -f- -Y -Iatclib $(SRC_NON_GEN) 2>/dev/null	\
		| awk 'sub (/^src/, "") == 1 {				\
				print "'$(OBJDIR)'" $$0;		\
				print "'$(DOBJDIR)'" $$0;		\
				next;					\
			}' >$@

cache/copyright.man: $(MAKEFILE) scripts/copyright docsrc/copyright
	scripts/copyright -m docsrc/copyright >$@

cache/copyright.txt: $(MAKEFILE) scripts/copyright docsrc/copyright
	scripts/copyright -t docsrc/copyright | sed 's/^./    &/' >$@

# The YYYY-MM-DD date indicated in the parentheses after the
# version number is the mtime of the most recent source file
# (where "being a source file" is defined as "being listed in
# $(SRC_NON_GEN)"). That string is the output of a perl script,
# scripts/youngest. Since perl is not necessarily installed on
# all machines, we cache that string in the file cache/srcdate
# and include that file in the distribution archive. If we
# didn't do that, people who don't have perl would be unable to
# build Yadex.
#
# Conceptually, cache/srcdate depends on $(SRC_NON_GEN) and
# doc/*.html depend on cache/srcdate. However, we can't write the
# makefile that way because if we did, that would cause two
# problems. Firstly every time a source file is changed,
# scripts/youngest would be ran, most of the time for nothing
# since its output is always the same, unless it's never been
# run today. Secondly, cache/srcdate being just generated, it's
# more recent than the content of the doc/ directory. The result
# would be that the entire doc/ directory would be rebuilt every
# time a single source file is changed, which is guaranteed to
# have an unnerving effect on the hacker at the keyboard.
#
# Part of the solution is to systematically force the mtime of
# cache/srcdate to 00:00, today. Thus, cache/srcdate always looks
# older than the content of the doc/ directory, unless it's not
# been refreshed yet today.
#
# But that's not enough because then cache/srcdate also looks
# always older than the source files it depends on, and thus
# make attempts to regenerate it every time make is invoked at
# all, which would render the very existence of cache/srcdate
# useless. That's why we have another file, cache/uptodate, that
# we touch to keep track of the time when we last generated
# cache/srcdate.
#
# If there was a such thing as _date-only_ dependencies, I could
# get away with just this :
#
# cache/srcdate: scripts/youngest
# cache/srcdate <date_dependency_operator> $(SRC_NON_GEN)
#         if perl -v >/dev/null 2>&1; then\
#           scripts/youngest >$@;\
#         else\
#           blah...
# doc/*.html <date_dependency_operator> cache/srcdate
#         blah...
#
# That would save two calls to "touch", one intermediary
# dependency (cache/uptodate) and a lot of obfuscation.
cache/srcdate: cache/uptodate

cache/uptodate: scripts/youngest $(SRC_NON_GEN)
	@mkdir -p cache
	@if perl -v >/dev/null 2>&1; then				\
	  echo Generating cache/srcdate;				\
	  scripts/youngest $(SRC_NON_GEN) >cache/srcdate;		\
	  touch -t `date '+%m%d'`0000 cache/srcdate;			\
	elif [ -r cache/srcdate ]; then					\
	  echo Perl not available. Keeping old cache/srcdate;		\
	else								\
	  echo Perl not available. Creating bogus cache/srcdate;	\
	  date '+%Y-%m-%d' >cache/srcdate;				\
	fi
	@touch $@;

# To compile the modules of Yadex
# (normal and debugging versions)
include yadex.dep

# It's simpler to copy config.cc into src/ than to have a
# compilation rule for just one file.
src/config.cc: $(OBJDIR)/config.cc
	cp -p $< $@

src/config.h: $(OBJDIR)/config.h
	cp -p $< $@

$(OBJDIR)/%.o: src/%.cc
	$(CXX) -c -Iatclib -I$(X11INCLUDEDIR) $(CXXFLAGS) $< -o $@

$(DOBJDIR)/%.o: src/%.cc
	$(CXX) -c -Iatclib -I$(X11INCLUDEDIR) $(DCXXFLAGS) $< -o $@

# To compile the modules of Atclib
# (normal and debugging versions)
$(OBJDIR_ATCLIB)/%.o: atclib/%.c $(HEADERS_ATCLIB)
	$(CC) -c $(CFLAGS) $< -o $@

$(DOBJDIR_ATCLIB)/%.o: atclib/%.c $(HEADERS_ATCLIB)
	$(CC) -c $(DCFLAGS) $< -o $@

# To compile the modules of Atclib
# (normal and debugging versions)
$(OBJDIR_COMPAT)/%.o: compat/%.c $(HEADERS_COMPAT)
	$(CC) -c $(CFLAGS) $< -o $@

$(DOBJDIR_COMPAT)/%.o: compat/%.c $(HEADERS_COMPAT)
	$(CC) -c $(DCFLAGS) $< -o $@

# To see the generated assembly code
# for the modules of Yadex
$(OBJDIR)/%.S: src/%.cc $(MAKEFILE)
	$(CXX) $(CXXFLAGS) -S -fverbose-asm -Iatclib -I$(X11INCLUDEDIR)\
	  $< -o $@

# A source file containing the credits
src/credits.cc: $(MAKEFILE) docsrc/copyright scripts/copyright
	@echo Generating $@
	@echo '// DO NOT EDIT -- generated from docsrc/copyright' >$@
	scripts/copyright -c docsrc/copyright >>$@

# A source file containing just the date of the
# most recent source file and the version number
# (found in ./VERSION)
src/version.cc: $(SRC_NON_GEN) VERSION cache/srcdate $(MAKEFILE)
	@echo Generating $@
	@printf '// DO NOT EDIT -- generated from VERSION\n\n' >$@
	@printf "extern const char *const yadex_source_date = \"%s\";\n" \
		`cat cache/srcdate` >>$@
	@printf "extern const char *const yadex_version = \"%s\";\n" 	\
		"$(VERSION)" >>$@


# -------- Doc-related stuff --------

docdirs:
	@if [ ! -d doc ]; then mkdir doc; fi

cache/pixlist: $(DOC2_SRC_HTML)
	@echo Generating $@
	@mkdir -p cache
	@if perl -v >/dev/null 2>/dev/null; then			\
	  perl -ne '@l = m/<img\s[^>]*src="?([^\s">]+)/io;		\
	    print "@l\n" if @l;' $(DOC2_SRC_HTML) | sort | uniq >$@;	\
	elif [ -f $@ ]; then						\
	  echo "Sorry, you need Perl to refresh $@. Keeping old $@.";	\
	else								\
	  echo "Sorry, you need Perl to create $@. Creating empty $@.";	\
	  touch $@;							\
	fi

events.html: ev evhtml
	evhtml -- -n $< >$@

events.txt: events.html
	lynx -dump $< >$@

changes/changes.html: changes/*.log log2html RELEASE
	./log2html -- -r `cat RELEASE` -- $$(ls -r changes/*.log) >$@
	
# changes - update the changelog
.PHONY: changes
changes: changes/changes.html
	w3m -dump -cols 72 $< >CHANGES

# cns - view the changelog with Netscape
.PHONY: cns
cns:
	netscape -remote "openURL(file:$$(pwd)/changes/changes.html,new-window)"

# clynx - view the changelog with Lynx
.PHONY: clynx
clynx:
	lynx changes/changes.html

# cless - view the changelog with less
.PHONY: cless
cless:
	less CHANGES

# man - view the man page with man
.PHONY: man
man: doc/yadex.6
	man -l $^

# dvi - view the man page with xdvi
.PHONY: dvi
dvi: doc/yadex.dvi
	xdvi $^ 

# ps - view the man page with gv
.PHONY: ps
ps: doc/yadex.ps
	gv $^

# Use docsrc/faq.html and not directly
# doc/faq.html because we don't want FAQ to be
# remade at first build time.
FAQ: docsrc/faq.html
	scripts/process $< >cache/faq.html
	links -width 72 -dump cache/faq.html >$@
	rm cache/faq.html

doc/yadex.dvi: doc/yadex.6
	groff -Tdvi -man $^ >$@

doc/yadex.ps: doc/yadex.6
	groff -Tps -man $^ >$@
	

# Generate the doc by filtering them through scripts/process
PROCESS =				\
	VERSION				\
	cache/copyright.man		\
	cache/copyright.txt		\
	cache/srcdate			\
	scripts/process			\
	$(OBJDIR)/ftime			\
	$(OBJDIR)/files_etc.man		\
	$(OBJDIR)/files_share.man	\
	$(OBJDIR)/notexist

doc/yadex.6: docsrc/yadex.6 $(PROCESS)
	@echo Generating $@
	@scripts/process $< >$@

doc/README: docsrc/README.doc $(PROCESS)
	@echo Generating $@
	@scripts/process $< >$@

%: docsrc/% $(PROCESS)
	@echo Generating $@
	@scripts/process $< >$@

doc/%.html: docsrc/%.html $(PROCESS)
	@echo Generating $@
	@scripts/process $< >$@

# The images are just symlinked from docsrc/ to doc/
doc/%.png: docsrc/%.png
	@rm -f $@
	@ln -s ../$< $@

$(OBJDIR)/ftime: scripts/ftime.c
	$(CC) $< -o $@

$(OBJDIR)/install: scripts/install.c
	$(CC) $< -o $@

$(OBJDIR)/notexist: scripts/notexist.c
	$(CC) $< -o $@


