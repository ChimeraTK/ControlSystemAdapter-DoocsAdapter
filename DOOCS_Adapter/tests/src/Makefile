#define the dependeny locations here
DOOCSROOT = $(HOME)/doocs.git/doocs

# to define DOOCSROOT as an absolute path
include $(DOOCSROOT)/$(DOOCSARCH)/DEFINEDOOCSROOT

# to define the arch dependend things
include $(DOOCSROOT)/$(DOOCSARCH)/CONFIG

#include /usr/share/mtca4u/ControlSystemAdapter.CONFIG
include /home/killenb/mtca4u/Ubuntu-14.04-x86_64/TESTING/ControlSystemAdapter/TESTING/ControlSystemAdapter.CONFIG

CPPFLAGS += $(ControlSystemAdapter_INCLUDE_FLAGS)
LDFLAGS += $(ControlSystemAdapter_LIB_FLAGS) $(ControlSystemAdapter_RUNPATH_FLAGS)

DOOCS_Adapter_SRC_DIR = ../..

OBJDIR = $(DOOCSROOT)/$(DOOCSARCH)/obj/library/common/DoocsAdapter
SRCDIR = $(PWD)

TEST_SOURCES = $(shell (cd src; ls *.cpp))
TEST_EXECUTABLES = $(basename $(TEST_SOURCES))

SOURCEOBJ = $(OBJDIR)/

SOURCEHFILES = $(SRCDIR)/eq_cosade.h $(SRCDIR)/IndependentControlCore.h

ALLPROGS = $(OBJDIR)/cosade_server 

CPPFLAGS += -Wall -Wextra -Wshadow -pedantic -Wuninitialized
CPPFLAGS += -I $(ControlSystemAdapter_DIR)/include
CPPFLAGS += -I $(DOOCS_Adapter_DIR)/include 
#LDFLAGS += -L /space/killenb/DOOCS_Adapter_trunk/DOOCS_Adapter/build/ -lDOOCSControlsSystemAdapter

echo:
	echo TEST_SORCES: $(TEST_EXECUTABLES)

all: $(ALLPROGS)

$(OBJDIR)/.depend_tests depend:
		@if [ ! -f $(OBJDIR) ] ; then \
		  echo ---------- create dir $(OBJDIR) --------------; \
		  mkdir -p $(OBJDIR) ; \
		fi
		for i in $(SRCDIR)/*.cpp ;do $(CCDEP) $$i ;done > $(OBJDIR)/.depend_tests
		chmod g+w $(OBJDIR)/.depend*

include $(OBJDIR)/.depend_tests

$(OBJDIR)/cosade_server:    $(SOURCEOBJ)	
		$(LINK.cc) $(LDFLAGS) \
		-o $(OBJDIR)/cosade_server $(SOURCEOBJ) \
		           -lEqServer -lTTFapi \
			   $(LDLIBS) 
		@chmod g+w $(OBJDIR)/cosade_server
		@echo "---------------- $(OBJDIR)/cosade_server done---------------"

static $(OBJDIR)/static_cosade_server:    $(SOURCEOBJ)	
		$(LINK.cc.static) $(LDFLAGS) -o $(OBJDIR)/static_cosade_server $(SOURCEOBJ) \
		           -lEqServer -lTTFapi \
			   $(LDLIBS) 
		@chmod g+w $(OBJDIR)/static_cosade_server
		@echo "----------------$(OBJDIR)/static_cosade_server done---------------"

clean:
	rm -f $(SOURCEOBJ) $(OBJDIR)/*.o *.ps $(OBJDIR)/cosade_server $(OBJDIR)/.depend*
