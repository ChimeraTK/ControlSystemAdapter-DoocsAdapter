#define the dependeny locations here
DOOCSROOT = $(HOME)/doocs.git/doocs

# to define DOOCSROOT as an absolute path
include $(DOOCSROOT)/$(DOOCSARCH)/DEFINEDOOCSROOT

# to define the arch dependend things
include $(DOOCSROOT)/$(DOOCSARCH)/CONFIG

#Check that the ControlSystemAdapter-config script is in your path
CPPFLAGS += $(shell ControlSystemAdapter-config --cppflags)
ControlSystemAdapter_LDFLAGS = $(shell ControlSystemAdapter-config --ldflags)

DOOCS_Adapter_SRC_DIR = ../..

OBJDIR = $(DOOCSROOT)/$(DOOCSARCH)/obj/library/common/DoocsAdapter
SRCDIR = $(PWD)

TEST_SOURCES = $(shell ls src/*.cpp)
TEST_EXECUTABLES = $(notdir $(basename $(TEST_SOURCES)))
TEST_TARGETS = $(addprefix $(OBJDIR)/tests/, $(TEST_EXECUTABLES))

#SOURCEOBJ = $(OBJDIR)/

#SOURCEHFILES = $(SRCDIR)/eq_cosade.h $(SRCDIR)/IndependentControlCore.h

#ALLPROGS = $(OBJDIR)/cosade_server 

CPPFLAGS += -Wall -Wextra -Wshadow -pedantic -Wuninitialized $(CPP_DEBUG_FLAGS)
CPPFLAGS += -I../include -I../example -Iinclude -isystem/local/lib/include
LDFLAGS += -lboost_thread

LDFLAGS += -lboost_system -lEqServer -lDOOCSapi 
#link the adapter with runpath, so it is found at execution time
LDFLAGS += -L$(OBJDIR) -lDoocsAdapter -Wl,-rpath=$(OBJDIR),--enable-new-dtags $(ControlSystemAdapter_LDFLAGS)

include ../CPP_DEBUG_FLAGS.CONFIG

all: $(TEST_TARGETS)

$(OBJDIR)/.depend_tests depend:
		@if [ ! -f $(OBJDIR) ] ; then \
		  echo ---------- create dir $(OBJDIR) --------------; \
		  mkdir -p $(OBJDIR) ; \
		fi
		for i in $(SRCDIR)/src/*.cpp ;do $(CC) $(CPPFLAGS) -MM $$i ;done > $(OBJDIR)/.depend_tests
		chmod g+w $(OBJDIR)/.depend*

include $(OBJDIR)/.depend_tests

#.cpp.o:
#	@if [ ! -f $(OBJDIR)/tests ] ; then \
#	  echo ---------- creating directory $(OBJDIR)/tests --------------; \
#	  mkdir -p $(OBJDIR)/tests ; \
#	fi
#	echo This is the implicit rule cpp for $<
#	$(CC) $(CCFLAGS) $(CPPFLAGS) -c $< -o$(OBJDIR)/tests/$@

#%:  $(OBJDIR)/tests/%.o
#	echo This is the implicit rule o for $<
#	$(LINK.cc) $< -o$(OBJDIR)/tests/$@ $(LDFLAGS) 

$(OBJDIR)/tests/%: src/%.cpp
	@if [ ! -f $(OBJDIR)/tests ] ; then \
	  echo ---------- creating directory $(OBJDIR)/tests --------------; \
	  mkdir -p $(OBJDIR)/tests ; \
	fi
	echo This is the implicit rule cpp for $<
	$(CC) $(CCFLAGS) $(CPPFLAGS) $< $(LDFLAGS) -o$@

clean:
	rm -f $(SOURCEOBJ) $(OBJDIR)/tests/* *.gcda *.gcno $(OBJDIR)/.depend*

test: $(OBJDIR)/CTestTestfile.cmake
	(cd $(OBJDIR); ctest)

$(OBJDIR)/CTestTestfile.cmake: CTestTestfile.cmake.in
	cat $< | sed "{s|@__OBJDIR__@|$(OBJDIR)|}" > $@
