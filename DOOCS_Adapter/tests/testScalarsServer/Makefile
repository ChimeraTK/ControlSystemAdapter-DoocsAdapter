#define the dependeny locations here
DOOCSROOT = $(HOME)/doocs.git/doocs
ControlSystemAdapter_DIR = $(HOME)/mtca4u/Ubuntu-14.04-x86_64/TESTING/ControlSystemAdapter/TESTING/
DOOCS_Adapter_DIR = /home/killenb/DOOCS_Adapter_redesign/DOOCS_Adapter

# to define DOOCSROOT as an absolute path
include $(DOOCSROOT)/$(DOOCSARCH)/DEFINEDOOCSROOT

SERVER_NAME = test_scalars_server
OBJDIR = $(DOOCSROOT)/$(DOOCSARCH)/obj/server/test/$(SERVER_NAME)
SRCDIR = $(PWD)

# to define the arch dependend things
include $(DOOCSROOT)/$(DOOCSARCH)/CONFIG

SOURCEOBJ = $(OBJDIR)/TestScalarsEqFct.o \
	    $(OBJDIR)/test_scalars_server.o

SOURCEHFILES = $(SRCDIR)/TestScalarsEqFct.h

ALLPROGS = $(OBJDIR)/$(SERVER_NAME)

CPPFLAGS += -Wall -Wextra -Wshadow -pedantic -Wuninitialized
CPPFLAGS += -I$(ControlSystemAdapter_DIR)/include
CPPFLAGS += -I$(DOOCS_Adapter_DIR)/include 
#LDFLAGS += -L/space/killenb/DOOCS_Adapter_trunk/DOOCS_Adapter/build/ -lDOOCSControlsSystemAdapter
LDFLAGS += -L$(ControlSystemAdapter_DIR)/lib -lControlSystemAdapter

echo:
	echo $(CPPFLAGS)

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

$(OBJDIR)/$(SERVER_NAME):    $(SOURCEOBJ)
		$(LINK.cc) $(LDFLAGS) \
		-o $(OBJDIR)/$(SERVER_NAME) $(SOURCEOBJ) \
		           -lEqServer -lDOOCSapi \
			   $(LDLIBS) 
		@chmod g+w $(OBJDIR)/$(SERVER_NAME)
		@echo "---------------- $(OBJDIR)/$(SERVER_NAME) done---------------"

static $(OBJDIR)/static_$(SERVER_NAME):    $(SOURCEOBJ)	
		$(LINK.cc.static) $(LDFLAGS) -o $(OBJDIR)/static_$(SERVER_NAME) $(SOURCEOBJ) \
		           -lEqServer -lDOOCSapi \
			   $(LDLIBS) 
		@chmod g+w $(OBJDIR)/static_$(SERVER_NAME)
		@echo "----------------$(OBJDIR)/static_$(SERVER_NAME) done---------------"

clean:
	rm -f $(SOURCEOBJ) $(OBJDIR)/*.o *.ps $(OBJDIR)/$(SERVER_NAME) $(OBJDIR)/.depend*
