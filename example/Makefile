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

OBJDIR = $(DOOCSROOT)/$(DOOCSARCH)/obj/server/test/cosade
SRCDIR = $(PWD)
ADAPTER_OBJDIR = $(DOOCSROOT)/$(DOOCSARCH)/obj/library/common/DoocsAdapter

SOURCEOBJ = $(OBJDIR)/cosade_server.o 
SOURCEHFILES =  
ALLPROGS = $(OBJDIR)/cosade_server 

include ../CPP_DEBUG_FLAGS.CONFIG

CPPFLAGS += -Wall -Wextra -Wshadow -pedantic -Wuninitialized $(CPP_DEBUG_FLAGS)
CPPFLAGS += -I../include -I../example -Iinclude -isystem/local/lib/include
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

$(OBJDIR)/cosade_server: $(SOURCEOBJ)	
		$(LINK.cc) \
		-o $(OBJDIR)/cosade_server $(SOURCEOBJ) \
		           -lEqServer -lDOOCSapi \
			   $(LDFLAGS) $(LDLIBS)
		@chmod g+w $(OBJDIR)/cosade_server
		@echo "---------------- $(OBJDIR)/cosade_server done---------------"

static $(OBJDIR)/static_cosade_server:    $(SOURCEOBJ)	
		$(LINK.cc.static) $(LDFLAGS) -o $(OBJDIR)/static_cosade_server $(SOURCEOBJ) \
		           -lEqServer -lDOOCSapi \
			   $(LDLIBS) 
		@chmod g+w $(OBJDIR)/static_cosade_server
		@echo "----------------$(OBJDIR)/static_cosade_server done---------------"

clean:
	rm -f $(SOURCEOBJ) $(OBJDIR)/*.o $(OBJDIR)/cosade_server $(OBJDIR)/.depend* *.gcda *.gcno

test: $(OBJDIR)/CTestTestfile.cmake
	(cd $(OBJDIR); ctest)

$(OBJDIR)/CTestTestfile.cmake: CTestTestfile.cmake.in
	cat $< | sed "{s|@__OBJDIR__@|$(OBJDIR)|}" > $@
