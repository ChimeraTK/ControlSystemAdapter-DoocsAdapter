cmake_minimum_required (VERSION 2.6)
project (DOOCS_Adapter)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -pedantic -Wextra -Wshadow -Wuninitialized")

#use -DCMAKE_BUILD_TYPE=Debug in your cmake command to turn on the coverage option
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -O0 --coverage")

FIND_PACKAGE(ControlSystemAdapter REQUIRED)
include_directories(${ControlSystemAdapter_INCLUDE_DIRS})

# --- boost ---
FIND_PACKAGE(Boost REQUIRED)
include_directories(${Boost_INCLUDE_DIR})

# --- boost unit_test ---
FIND_PACKAGE(Boost COMPONENTS unit_test_framework REQUIRED)
IF(Boost_UNIT_TEST_FRAMEWORK_FOUND)
    include_directories(${Boost_INCLUDE_DIR})
    ENABLE_TESTING()
    set(TESTING_IS_ENABLED "true")
    message(status "** Boost Include: ${Boost_INCLUDE_DIR}")
    message(status "** Boost Libraries: ${Boost_LIBRARY_DIRS}")
    message(status "** Boost Libraries: ${Boost_LIBRARIES}")
ELSE(Boost_UNIT_TEST_FRAMEWORK_FOUND)
    message("Boost unit_test_framework not found, disabling testing")
ENDIF(Boost_UNIT_TEST_FRAMEWORK_FOUND)

include_directories(${CMAKE_SOURCE_DIR}/include)
aux_source_directory(${CMAKE_SOURCE_DIR}/src library_sources)

#Currently we assume that they are all tests contained in one file, so
#each file gives a new executable. This section has to be adapted if this should change.
if(TESTING_IS_ENABLED)
    include_directories(${CMAKE_SOURCE_DIR}/tests/include)
    aux_source_directory(${CMAKE_SOURCE_DIR}/tests/src testSources)
    foreach( testSourceFile ${testSources})
        #NAME_WE means the base name without path and (longest) extension
        get_filename_component(excutableName ${testSourceFile} NAME_WE)
        add_executable(${excutableName} ${testSourceFile})
        target_link_libraries(${excutableName} ${ControlSystemAdapter_LIBRARIES})
        add_test(${excutableName} ${excutableName})
    endforeach( testSourceFile )

    #The make coverage command is only available in debug mode
    IF(CMAKE_BUILD_TYPE STREQUAL "Debug")
      configure_file(cmake/Makefile.coverage.in
	${PROJECT_BINARY_DIR}/Makefile.coverage @ONLY)
      add_custom_target(coverage
	make -f Makefile.coverage
	WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
	COMMENT "Generating test coverage documentation" VERBATIM
	)
    ENDIF(CMAKE_BUILD_TYPE STREQUAL "Debug")

endif(TESTING_IS_ENABLED)

#Generate the Makefile for the COntrol System ADapter Example (cosade),
#depending on the source code of the DOOCS_Adapter.
#FIXME: Should be done for the installed version of the DOOOCS_Adapter, but is not critical at the moment
# as it is header only not (which will change)
message("CMAKE_SOURCE_DIR  ${CMAKE_SOURCE_DIR}")
configure_file(cmake/Makefile.cosade.in ${CMAKE_SOURCE_DIR}/example/Makefile @ONLY)

#Generate the Makefile for the DOOCS test server (dpvatestsrv),
#depending on the source code of the DOOCS_Adapter.
configure_file(cmake/Makefile.dpvatestsrv.in ${CMAKE_SOURCE_DIR}/tests/dpvatestsrv/Makefile @ONLY)

