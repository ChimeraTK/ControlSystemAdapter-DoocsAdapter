PROJECT_NAME=DoocsAdapter

DOOCSROOT = $(HOME)/doocs.git/doocs
# to define DOOCSROOT as an absolute path
include $(DOOCSROOT)/$(DOOCSARCH)/DEFINEDOOCSROOT

# to define the arch dependend things
include $(DOOCSROOT)/$(DOOCSARCH)/CONFIG

#Check that the ControlSystemAdapter-config script is in your path
CPPFLAGS += $(shell ControlSystemAdapter-config --cppflags)
LDFLAGS += $(shell ControlSystemAdapter-config --ldflags)

OBJDIR = $(DOOCSROOT)/$(DOOCSARCH)/obj/library/common/$(PROJECT_NAME)
SRCDIR = $(PWD)

OBJ_SOURCES = $(shell ls src/*.cc)

CPPFLAGS += -Wall -Wextra -Wshadow -pedantic -Wuninitialized $(CPP_DEBUG_FLAGS)
CPPFLAGS += -I$(SRCDIR)/include
#Treat DOOCS, which is in /local/lib as system library and suppress compiler warnings for these files
CPPFLAGS += -isystem/local/lib/include

TARGET_OBJECTS =  $(OBJDIR)/lib$(PROJECT_NAME).so

.PHONY: $(OBJDIR)/.depend tests example

all: CPP_DEBUG_FLAGS.CONFIG $(TARGET_OBJECTS) tests example

CPP_DEBUG_FLAGS.CONFIG:
	touch CPP_DEBUG_FLAGS.CONFIG

include CPP_DEBUG_FLAGS.CONFIG

# The debug only works if you remove the OPTIMIZED variable from the Doocs CONFIG.
# This sets the optimisation to -O2 and you cannot create a coverage report.
enable_debug:
	echo "CPP_DEBUG_FLAGS = -g -O0 --coverage" > CPP_DEBUG_FLAGS.CONFIG
	make clean

disable_debug:
	echo > CPP_DEBUG_FLAGS.CONFIG
	make clean

$(OBJDIR)/.depend:
	@if [ ! -f $(OBJDIR) ] ; then \
	  mkdir -p $(OBJDIR) ; \
	fi
	for i in $(SRCDIR)/src/*.cc ;do  $(CC) $(CPPFLAGS) -MM $$i ;done > $(OBJDIR)/.depend
	chmod g+w $(OBJDIR)/.depend*
	echo depend done!

include $(OBJDIR)/.depend

$(OBJDIR)/lib${PROJECT_NAME}.so: $(OBJ_SOURCES)	
	$(LINK.cc) $(LDFLAGS) -shared -fPIC -o $@ $(OBJ_SOURCES)\
	 $(LDLIBS) -lEqServer -lDOOCSapi
	chmod g+w $@

clean:
	echo This is clean!	
	rm -f $(SOURCEOBJ) $(OBJDIR)/*.o *.ps $(OBJDIR)/lib${PROJECT_NAME}.so $(OBJDIR)/.depend*
	rm -rf *.gcda *.gcno tests/*.gcda tests/*.gcno
	(cd tests; make clean)
	(cd example; make clean)

test: tests
	(cd tests; make test)

tests: $(TARGET_OBJECTS)
	(cd tests; make)

example: $(TARGET_OBJECTS)
	(cd example; make)

coverage:
	rm -f `find . -name "*\.gcda"`
	(cd tests; make test)
	lcov --capture --directory . --output-file $(OBJDIR)/coverage_all.info
	#lcov capture also includes external stuff like glibc, boost etc.
	#only extract the reports for this project
	lcov --extract $(OBJDIR)/coverage_all.info "$(SRCDIR)*" -o $(OBJDIR)/coverage.info
	genhtml $(OBJDIR)/coverage.info --output-directory $(OBJDIR)/coverage_html
	@echo Coverage written to $(OBJDIR)/coverage_html
