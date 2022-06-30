PROJECT(ChimeraTK-ControlSystemAdapter-DoocsAdapter)
cmake_minimum_required(VERSION 2.8)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake/Modules)

set(${PROJECT_NAME}_MAJOR_VERSION 01)
set(${PROJECT_NAME}_MINOR_VERSION 05)
set(${PROJECT_NAME}_PATCH_VERSION 02)
include(cmake/set_version_numbers.cmake)

include(cmake/set_default_build_to_release.cmake)
include(cmake/set_default_flags.cmake)

# linker flags
set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -Wl,--export-dynamic")

include(cmake/add_dependency.cmake)
include(cmake/enable_code_coverage_report.cmake)

add_dependency(ChimeraTK-ControlSystemAdapter 02.04 REQUIRED)
add_dependency(doocs-server-test-helper 01.05 REQUIRED)
add_dependency(DOOCS 22.05.0 COMPONENTS server REQUIRED)

FIND_PACKAGE(Boost COMPONENTS thread REQUIRED)

# libXML++ is used for parsing/writing XML files
FIND_PACKAGE(PkgConfig REQUIRED)
set(LIBXML++_VERSION "libxml++-2.6")
PKG_CHECK_MODULES(LibXML++ REQUIRED ${LIBXML++_VERSION})
PKG_CHECK_MODULES(glib REQUIRED glib-2.0)
include_directories(SYSTEM "${LibXML++_INCLUDEDIR}/${LIBXML++_VERSION}" ${glib_INCLUDE_DIRS} ${LibXML++_INCLUDE_DIRS})

#For some reason the system library has to be mentioned explicitly to the linker.
#This is strange because the thead library has it as a dependency.
option(BUILD_TESTS "Build test programs" ON)
IF(BUILD_TESTS)
  FIND_PACKAGE(Boost COMPONENTS unit_test_framework REQUIRED)
  ENABLE_TESTING()
ENDIF()


include_directories(${CMAKE_SOURCE_DIR}/include ${CMAKE_SOURCE_DIR}/tests/include ${CMAKE_SOURCE_DIR}/example)
set(${PROJECT_NAME}_INCLUDE_DIRS ${${PROJECT_NAME}_INCLUDE_DIRS} ${CMAKE_SOURCE_DIR}/include/)

aux_source_directory(${CMAKE_SOURCE_DIR}/src library_sources)
file(GLOB headers "${CMAKE_CURRENT_SOURCE_DIR}/${subdir}/include/*.h")
set(library_sources ${library_sources} ${headers})

# Create the executables for automated unit testing.
# Currently we assume that they are all tests contained in one file, so
# each file gives a new executable. This section has to be adapted if this should change.
if(BUILD_TESTS)

  # loop over all sources files in execultables_src directory
  aux_source_directory(${CMAKE_SOURCE_DIR}/tests/src testExecutables)
  foreach( testExecutableSrcFile ${testExecutables} ${referenceExecutables})
    # NAME_WE means the base name without path and (longest) extension
    get_filename_component(excutableName ${testExecutableSrcFile} NAME_WE)
    # each test includes a DOOCS server
    add_executable(${excutableName} ${testExecutableSrcFile})
    set_target_properties(${excutableName} PROPERTIES COMPILE_FLAGS "${ChimeraTK-ControlSystemAdapter_CXX_FLAGS} ${DOOCS_CXX_FLAGS} ${doocs-server-test-helper_CXX_FLAGS}")
    set_target_properties(${excutableName} PROPERTIES LINK_FLAGS "-Wl,-rpath=${CMAKE_BINARY_DIR},--enable-new-dtags ${ChimeraTK-ControlSystemAdapter_LINKER_FLAGS} ${DOOCS_LINKER_FLAGS} ${doocs-server-test-helper_LINKER_FLAGS}")
    target_link_libraries(${excutableName} ${PROJECT_NAME} ${doocs-server-test-helper_LIBRARIES})
    add_test(${excutableName} ${excutableName})
  endforeach( testExecutableSrcFile )

  file(GLOB XML_FILES variable  ${CMAKE_SOURCE_DIR}/tests/variableTreeXml/*.xml ${CMAKE_SOURCE_DIR}/tests/variableTreeXml/intentionallyBroken/*.xml)
  file(COPY ${XML_FILES} DESTINATION ${PROJECT_BINARY_DIR}/variableTreeXml)

  file(GLOB TEST_XML_FILES variable  ${CMAKE_SOURCE_DIR}/tests/*.xml)
  file(COPY ${TEST_XML_FILES} DESTINATION ${PROJECT_BINARY_DIR})

  file(GLOB TEST_CONF_FILES variable  ${CMAKE_SOURCE_DIR}/tests/*.conf)
  file(COPY ${TEST_CONF_FILES} DESTINATION ${PROJECT_BINARY_DIR})

endif()

include(cmake/enable_doxygen_documentation.cmake)

# C++ library
add_library(${PROJECT_NAME} SHARED ${library_sources} )
set_target_properties(${PROJECT_NAME} PROPERTIES VERSION ${${PROJECT_NAME}_FULL_LIBRARY_VERSION} SOVERSION ${${PROJECT_NAME}_SOVERSION})
set_target_properties(${PROJECT_NAME} PROPERTIES COMPILE_FLAGS "${ChimeraTK-ControlSystemAdapter_CXX_FLAGS} ${DOOCS_CXX_FLAGS}")
target_link_libraries(${PROJECT_NAME} ${ChimeraTK-ControlSystemAdapter_LIBRARIES} ${DOOCS_LIBRARIES})

# do not remove runtime path of the library when installing
set_property(TARGET ${PROJECT_NAME} PROPERTY INSTALL_RPATH_USE_LINK_PATH TRUE)

# build the reference test doocs server
add_executable(referenceTestDoocsServer ${CMAKE_SOURCE_DIR}/tests/referenceTestDoocsServer/referenceTestDoocsServer.cc)
set_target_properties(referenceTestDoocsServer PROPERTIES COMPILE_FLAGS "${ChimeraTK-ControlSystemAdapter_CXX_FLAGS} ${DOOCS_CXX_FLAGS}")
set_target_properties(referenceTestDoocsServer PROPERTIES LINK_FLAGS "${ChimeraTK-ControlSystemAdapter_LINKER_FLAGS} ${DOOCS_LINKER_FLAGS}")
target_link_libraries(referenceTestDoocsServer ${PROJECT_NAME} ${DOOCS_LIBRARIES} ${ChimeraTK-ControlSystemAdapter_LIBRARIES} ${Boost_LIBRARIES} )
FILE( COPY ${CMAKE_SOURCE_DIR}/tests/referenceTestDoocsServer/referenceTestDoocsServer.conf DESTINATION ${PROJECT_BINARY_DIR})


#Install the library and the executables
install( TARGETS ${PROJECT_NAME}
         RUNTIME DESTINATION bin
             LIBRARY DESTINATION lib )

#all include files go into include/PROJECT_NAME
#The exclusion of ${PROJECT_NAME} prevents the recursive installation of the files just being installed.
#The original headers are in include/*.h, the installed ones in include/PROJECT_NAME/*.h.
install( DIRECTORY ${${PROJECT_NAME}_INCLUDE_DIRS} DESTINATION include/${PROJECT_NAME}
         FILES_MATCHING PATTERN "*.h"
         PATTERN ".svn" EXCLUDE
         PATTERN "${PROJECT_NAME}" EXCLUDE )

# generate cmake config so other projects can find this library
set(${PROJECT_NAME}_INCLUDE_DIRS "${CMAKE_INSTALL_PREFIX}/include ${ChimeraTK-ControlSystemAdapter_INCLUDE_DIRS}")
set(${PROJECT_NAME}_LIBRARIES "${ChimeraTK-ControlSystemAdapter_LIBRARIES} ${DOOCS_LIBRARIES}")
set(${PROJECT_NAME}_LIBRARY_DIRS "${CMAKE_INSTALL_PREFIX}/lib ${ChimeraTK-ControlSystemAdapter_LIBRARY_DIRS} ${DOOCS_LIBRARY_DIRS}")
set(${PROJECT_NAME}_CXX_FLAGS "${ChimeraTK-ControlSystemAdapter_CXX_FLAGS}")
set(${PROJECT_NAME}_LINKER_FLAGS "${ChimeraTK-ControlSystemAdapter_LINKER_FLAGS} ${DOOCS_LINKER_FLAGS}")
include(${CMAKE_SOURCE_DIR}/cmake/create_cmake_config_files.cmake)

