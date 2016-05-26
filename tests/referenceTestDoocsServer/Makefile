#define the dependeny locations here
DOOCSROOT = $(HOME)/doocs.git/doocs

# to define DOOCSROOT as an absolute path
include $(DOOCSROOT)/$(DOOCSARCH)/DEFINEDOOCSROOT

# to define the arch dependend things
include $(DOOCSROOT)/$(DOOCSARCH)/CONFIG

#Check that the ControlSystemAdapter-config script is in your path
CPPFLAGS += $(shell ControlSystemAdapter-config --cppflags)
LDFLAGS += $(shell ControlSystemAdapter-config --ldflags)

#DOOCS_Adapter_SRC_DIR = ../..

OBJDIR = $(DOOCSROOT)/$(DOOCSARCH)/obj/server/test/referenceTestDoocsServer
SRCDIR = $(PWD)
ADAPTER_OBJDIR = $(DOOCSROOT)/$(DOOCSARCH)/obj/library/common/DoocsAdapter

SOURCEOBJ = $(OBJDIR)/referenceTestDoocsServer.o 
SOURCEHFILES =  
ALLPROGS = $(OBJDIR)/referenceTestDoocsServer

include ../../CPP_DEBUG_FLAGS.CONFIG

CPPFLAGS += -Wall -Wextra -Wshadow -pedantic -Wuninitialized $(CPP_DEBUG_FLAGS)
CPPFLAGS += -I../../include -Iinclude -isystem/local/lib/include
LDFLAGS += -lboost_thread -lboost_system
#link the adapter with runpath, so it is found at execution time
LDFLAGS += -L$(ADAPTER_OBJDIR) -lDoocsAdapter -Wl,-rpath=$(ADAPTER_OBJDIR),--enable-new-dtags

all: $(ALLPROGS)

$(OBJDIR)/.depend depend:
		@if [ ! -f $(OBJDIR) ] ; then \
		  echo ---------- create dir $(OBJDIR) --------------; \
		  mkdir -p $(OBJDIR) ; \
		fi
		for i in $(SRCDIR)/*.cc ;do $(CCDEP) $$i ;done > $(OBJDIR)/.depend_temp
		cat $(OBJDIR)/.depend_temp | sed -e "/:/s/^/\$$\(OBJDIR\)\//g" > $(OBJDIR)/.depend
		chmod g+w $(OBJDIR)/.depend*

include $(OBJDIR)/.depend

$(OBJDIR)/referenceTestDoocsServer: $(SOURCEOBJ)	
		$(LINK.cc) \
		-o $@ $(SOURCEOBJ) \
		           -lEqServer -lDOOCSapi \
			   $(LDFLAGS) $(LDLIBS)
		@chmod g+w $@
		@echo "---------------- $@ done---------------"

clean:
	rm -f $(SOURCEOBJ) $(OBJDIR)/*.o $(OBJDIR)/referenceTestDoocsServer $(OBJDIR)/.depend* *.gcda *.gcno

test: $(OBJDIR)/CTestTestfile.cmake
	(cd $(OBJDIR); ctest)

$(OBJDIR)/CTestTestfile.cmake: CTestTestfile.cmake.in
	cat $< | sed "{s|@__OBJDIR__@|$(OBJDIR)|}" > $@
