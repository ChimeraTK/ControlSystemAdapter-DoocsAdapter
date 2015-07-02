PROJECT_NAME=DoocsAdapter

DOOCSROOT = $(HOME)/doocs.git/doocs
# to define DOOCSROOT as an absolute path
include $(DOOCSROOT)/$(DOOCSARCH)/DEFINEDOOCSROOT

# to define the arch dependend things
include $(DOOCSROOT)/$(DOOCSARCH)/CONFIG

#include /usr/share/mtca4u/ControlSystemAdapter.CONFIG
include /home/killenb/mtca4u/Ubuntu-14.04-x86_64/TESTING/ControlSystemAdapter/TESTING/ControlSystemAdapter.CONFIG

CPPFLAGS += $(ControlSystemAdapter_INCLUDE_FLAGS)
LDFLAGS += $(ControlSystemAdapter_LIB_FLAGS) $(ControlSystemAdapter_RUNPATH_FLAGS)

OBJDIR = $(DOOCSROOT)/$(DOOCSARCH)/obj/library/common/$(PROJECT_NAME)
SRCDIR = $(PWD)

OBJ_SOURCES = $(shell ls src/*.cc)

CPPFLAGS += -I$(SRCDIR)/include

all: $(OBJDIR)/lib${PROJECT_NAME}.so

.PHONY: $(OBJDIR)/.depend

$(OBJDIR)/.depend:
	@if [ ! -f $(OBJDIR) ] ; then \
	  mkdir -p $(OBJDIR) ; \
	fi
	for i in $(SRCDIR)/src/*.cc ;do  $(CC) $(CPPFLAGS) -MM $$i ;done > $(OBJDIR)/.depend
	chmod g+w $(OBJDIR)/.depend*

include $(OBJDIR)/.depend

$(OBJDIR)/lib${PROJECT_NAME}.so: $(OBJ_SOURCES)	
		$(LINK.cc) $(LDFLAGS) -shared -fPIC -o $@ $(OBJ_SOURCES)\
			   $(LDLIBS)
		# -lEqServer -lDOOCSapi \
		chmod g+w $@

clean:
	rm -f $(SOURCEOBJ) $(OBJDIR)/*.o *.ps $(OBJDIR)/lib${PROJECT_NAME}.so $(OBJDIR)/.depend*
