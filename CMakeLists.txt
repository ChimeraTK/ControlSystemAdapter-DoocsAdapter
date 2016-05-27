PROJECT(ControlSystemAdapter-DoocsAdapter)
cmake_minimum_required(VERSION 2.8)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake/Modules)

set(${PROJECT_NAME}_MAJOR_VERSION 00)
set(${PROJECT_NAME}_MINOR_VERSION 00)
set(${PROJECT_NAME}_PATCH_VERSION 00)
set(${PROJECT_NAME}_VERSION ${${PROJECT_NAME}_MAJOR_VERSION}.${${PROJECT_NAME}_MINOR_VERSION}.${${PROJECT_NAME}_PATCH_VERSION})
set(${PROJECT_NAME}_SOVERSION ${${PROJECT_NAME}_MAJOR_VERSION}.${${PROJECT_NAME}_MINOR_VERSION})

#Moderate version of the compiler flags
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -fPIC -Wextra -ansi -Wuninitialized")

#use -DCMAKE_BUILD_TYPE=Debug in your cmake command to turn on the coverage option
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -O0 --coverage")

# linker flags
set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -Wl,--export-dynamic")

FIND_PACKAGE(ControlSystemAdapter REQUIRED)
include_directories(SYSTEM ${ControlSystemAdapter_INCLUDE_DIRS})
link_directories(${ControlSystemAdapter_LIBRARY_DIRS})

FIND_PACKAGE(DOOCS 18.10.5 COMPONENTS server REQUIRED)
include_directories(SYSTEM ${DOOCS_INCLUDE_DIRS})
link_directories(${DOOCS_LIBRARY_DIRS})

FIND_PACKAGE(mtca4u-doocsServerTestHelper REQUIRED)
include_directories(SYSTEM ${mtca4u-doocsServerTestHelper_INCLUDE_DIRS})
link_directories(${mtca4u-doocsServerTestHelper_LIBRARY_DIRS})

FIND_PACKAGE(Boost COMPONENTS thread REQUIRED)

#For some reason the system library has to be mentioned explicitly to the linker.
#This is strange because the thead library has it as a dependency.
FIND_PACKAGE(Boost COMPONENTS thread system unit_test_framework)
IF(Boost_UNIT_TEST_FRAMEWORK_FOUND)
  set(TESTING_IS_ENABLED "true")
  ENABLE_TESTING()
ELSE(Boost_UNIT_TEST_FRAMEWORK_FOUND)
  message("Boost unit_test_framework, disabling testing")
ENDIF(Boost_UNIT_TEST_FRAMEWORK_FOUND)


include_directories(${CMAKE_SOURCE_DIR}/include ${CMAKE_SOURCE_DIR}/tests/include ${CMAKE_SOURCE_DIR}/example)
set(${PROJECT_NAME}_INCLUDE_DIRS ${${PROJECT_NAME}_INCLUDE_DIRS} ${CMAKE_SOURCE_DIR}/include/)

aux_source_directory(${CMAKE_SOURCE_DIR}/src library_sources)

# Create the executables for automated unit testing.
# Currently we assume that they are all tests contained in one file, so
# each file gives a new executable. This section has to be adapted if this should change.
if(TESTING_IS_ENABLED)

  # loop over all sources files in execultables_src directory
  aux_source_directory(${CMAKE_SOURCE_DIR}/tests/src testExecutables)
  foreach( testExecutableSrcFile ${testExecutables} ${referenceExecutables})
    # NAME_WE means the base name without path and (longest) extension
    get_filename_component(excutableName ${testExecutableSrcFile} NAME_WE)
    # each test includes a DOOCS server
    add_executable(${excutableName} ${testExecutableSrcFile})
    set_target_properties(${excutableName} PROPERTIES COMPILE_FLAGS "${ControlSystemAdapter_CXX_FLAGS} ${DOOCS_CXX_FLAGS}")
    set_target_properties(${excutableName} PROPERTIES LINK_FLAGS ${ControlSystemAdapter_LINK_FLAGS} ${DOOCS_LINK_FLAGS})
    target_link_libraries(${excutableName} ${mtca4u-doocsServerTestHelper_LIBRARIES} ${DOOCS_LIBRARIES}  ${ControlSystemAdapter_LIBRARIES} ${Boost_LIBRARIES} ${PROJECT_NAME}) 
    add_test(${excutableName} ${excutableName})
  endforeach( testExecutableSrcFile )

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

# C++ library
add_library(${PROJECT_NAME} SHARED ${library_sources} )
set_target_properties(${PROJECT_NAME} PROPERTIES VERSION ${${PROJECT_NAME}_VERSION} SOVERSION ${${PROJECT_NAME}_SOVERSION})
set_target_properties(${PROJECT_NAME} PROPERTIES COMPILE_FLAGS "${ControlSystemAdapter_CXX_FLAGS} ${DOOCS_CXX_FLAGS}")
target_link_libraries(${PROJECT_NAME} ${ControlSystemAdapter_LIBRARIES} ${DOOCS_LIBRARIES} ${Boost_LIBRARIES})

# build the reference test doocs server
add_executable(referenceTestDoocsServer ${CMAKE_SOURCE_DIR}/tests/referenceTestDoocsServer/referenceTestDoocsServer.cc)
set_target_properties(referenceTestDoocsServer PROPERTIES COMPILE_FLAGS "${ControlSystemAdapter_CXX_FLAGS} ${DOOCS_CXX_FLAGS}")
set_target_properties(referenceTestDoocsServer PROPERTIES LINK_FLAGS ${ControlSystemAdapter_LINK_FLAGS} ${DOOCS_LINK_FLAGS})
target_link_libraries(referenceTestDoocsServer ${mtca4u-doocsServerTestHelper_LIBRARIES} ${DOOCS_LIBRARIES} ${ControlSystemAdapter_LIBRARIES} ${Boost_LIBRARIES} ${PROJECT_NAME}) 
FILE( COPY ${CMAKE_SOURCE_DIR}/tests/referenceTestDoocsServer/referenceTestDoocsServer.conf DESTINATION ${PROJECT_BINARY_DIR})

#configure a header file which contains the version information for use in C++
#configure_file(cmake/version.h.in "${PROJECT_BINARY_DIR}/version.h" @ONLY)

# add a target to generate API documentation with Doxygen
find_package(Doxygen)
if(DOXYGEN_FOUND)
  configure_file(${CMAKE_CURRENT_SOURCE_DIR}/cmake/Doxyfile.in.in ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile.in @ONLY)
  configure_file(${CMAKE_CURRENT_BINARY_DIR}/Doxyfile.in ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile @ONLY)
  add_custom_target(doc ALL
    ${DOXYGEN_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    COMMENT "Generating API documentation with Doxygen" VERBATIM
    )
  install(DIRECTORY ${CMAKE_BINARY_DIR}/doc DESTINATION .)
else(DOXYGEN_FOUND)
  message("Doxygen not found, documentation will not be build.")
endif(DOXYGEN_FOUND)


#Install the library and the executables
install(TARGETS ${PROJECT_NAME}
            RUNTIME DESTINATION bin
	    LIBRARY DESTINATION lib
  )

#all include files go into include/PROJECT_NAME
#The exclusion of ${PROJECT_NAME} prevents the recursive installation of the files just being installed.
#The original headers are in include/*.h, the installed ones in include/PROJECT_NAME/*.h.
install(DIRECTORY ${${PROJECT_NAME}_INCLUDE_DIRS} DESTINATION include/${PROJECT_NAME}
          FILES_MATCHING PATTERN "*.h"
	  PATTERN ".svn" EXCLUDE
	  PATTERN "${PROJECT_NAME}" EXCLUDE
	  )

# generate cmake config so other projects can find this library
set(${PROJECT_NAME}_INCLUDE_DIRS "${CMAKE_INSTALL_PREFIX}/include ${ControlSystemAdapter_INCLUDE_DIRS}")
set(${PROJECT_NAME}_LIBRARIES "${PROJECT_NAME} ${ControlSystemAdapter_LIBRARIES}")
set(${PROJECT_NAME}_LIBRARY_DIRS "${CMAKE_INSTALL_PREFIX}/lib ${ControlSystemAdapter_LIBRARY_DIRS}")
set(${PROJECT_NAME}_CXX_FLAGS "${ControlSystemAdapter_CXX_FLAGS}")
set(${PROJECT_NAME}_LINK_FLAGS "${ControlSystemAdapter_LINK_FLAGS}")
include(${CMAKE_SOURCE_DIR}/cmake/create_cmake_config_files.cmake)

# The instructions to create a debian package are loaded from a separate file
#include(${CMAKE_SOURCE_DIR}/cmake/prepare_debian_package.cmake)