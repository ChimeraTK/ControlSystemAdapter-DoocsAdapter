PROJECT(ChimeraTK-ControlSystemAdapter-DoocsAdapter)
cmake_minimum_required(VERSION 3.16)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake/Modules)

set(${PROJECT_NAME}_MAJOR_VERSION 01)
set(${PROJECT_NAME}_MINOR_VERSION 07)
set(${PROJECT_NAME}_PATCH_VERSION 01)
include(cmake/set_version_numbers.cmake)

include(cmake/set_default_build_to_release.cmake)
include(cmake/set_default_flags.cmake)

include(cmake/format_options.cmake)
# global linker flags. Note, this is not exported.
set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -Wl,--export-dynamic")

include(cmake/enable_code_coverage_report.cmake)
include(cmake/enable_code_style_check.cmake)

FIND_PACKAGE(ChimeraTK-ControlSystemAdapter 02.05 REQUIRED)

FIND_PACKAGE(doocs-server-test-helper 01.05 REQUIRED)

FIND_PACKAGE(DOOCS 22.05.0 COMPONENTS server REQUIRED)

FIND_PACKAGE(Boost COMPONENTS thread REQUIRED)

FIND_PACKAGE(PkgConfig REQUIRED)
# note, libxml++ is already used in ControlSystemAdapter but set as PRIVATE. 
# It makes sinse to keep versions in sync.
set(LIBXML++_VERSION "libxml++-2.6")
PKG_CHECK_MODULES(LibXML++ REQUIRED IMPORTED_TARGET ${LIBXML++_VERSION})
PKG_CHECK_MODULES(glib REQUIRED IMPORTED_TARGET glib-2.0)

#For some reason the system library has to be mentioned explicitly to the linker.
#This is strange because the thead library has it as a dependency.
option(BUILD_TESTS "Build test programs" ON)
IF(BUILD_TESTS)
  FIND_PACKAGE(Boost COMPONENTS unit_test_framework REQUIRED)
  ENABLE_TESTING()
ENDIF()
  
aux_source_directory(${CMAKE_SOURCE_DIR}/src library_sources)
file(GLOB headers "${CMAKE_CURRENT_SOURCE_DIR}/${subdir}/include/*.h")
set(library_sources ${library_sources} ${headers})

# first define C++ library, will be referenced by tests
add_library(${PROJECT_NAME} SHARED ${library_sources} )
target_include_directories(${PROJECT_NAME} PUBLIC 
    "$<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include>"
    # relative to ${CMAKE_INSTALL_PREFIX} but don't explicitly mention it
    "$<INSTALL_INTERFACE:include>")
set_target_properties(${PROJECT_NAME} PROPERTIES VERSION ${${PROJECT_NAME}_FULL_LIBRARY_VERSION} SOVERSION ${${PROJECT_NAME}_SOVERSION})
# with imported targets, CXX flags etc are pulled in automatically
target_link_libraries(${PROJECT_NAME} 
    PRIVATE PkgConfig::LibXML++
    # public seems to be necessary otherwise weird linker errors downstream
    PUBLIC DOOCS::server 
    # we make this public because of implicitly carried DeviceAccess compile flags, needed e.g. for tests
    PUBLIC ChimeraTK::ChimeraTK-ControlSystemAdapter)

# do not remove runtime paths of the library when installing (helps for unsually located implicit dependencies)
set_property(TARGET ${PROJECT_NAME} PROPERTY INSTALL_RPATH_USE_LINK_PATH TRUE)

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
    target_include_directories(${excutableName} PRIVATE ${CMAKE_SOURCE_DIR}/tests/include)
    target_link_options(${excutableName} PRIVATE "-Wl,--enable-new-dtags")
    target_link_libraries(${excutableName} 
        PRIVATE ${PROJECT_NAME} ChimeraTK::doocs-server-test-helper
        PRIVATE PkgConfig::LibXML++ PkgConfig::glib)
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

# build the reference test doocs server
add_executable(referenceTestDoocsServer ${CMAKE_SOURCE_DIR}/tests/referenceTestDoocsServer/referenceTestDoocsServer.cc)
target_link_libraries(referenceTestDoocsServer 
    PRIVATE ${PROJECT_NAME} ChimeraTK::ChimeraTK-ControlSystemAdapter ${Boost_LIBRARIES} 
    PRIVATE PkgConfig::LibXML++ PkgConfig::glib)

FILE( COPY ${CMAKE_SOURCE_DIR}/tests/referenceTestDoocsServer/referenceTestDoocsServer.conf DESTINATION ${PROJECT_BINARY_DIR})

# TODO - remove again when done with testing: 
# install referenceTestDoocsServer for testing of rpath
set_property(TARGET referenceTestDoocsServer PROPERTY INSTALL_RPATH_USE_LINK_PATH TRUE)
install(TARGETS referenceTestDoocsServer
    RUNTIME DESTINATION bin)

#Install the library and the executables
install( TARGETS ${PROJECT_NAME}
         EXPORT ${PROJECT_NAME}Targets
         RUNTIME DESTINATION bin
         LIBRARY DESTINATION lib )

#all include files go into include/PROJECT_NAME
#The exclusion of ${PROJECT_NAME} prevents the recursive installation of the files just being installed.
#The original headers are in include/*.h, the installed ones in include/PROJECT_NAME/*.h.
install( DIRECTORY ${CMAKE_SOURCE_DIR}/include/ DESTINATION include/${PROJECT_NAME}
         FILES_MATCHING PATTERN "*.h"
         PATTERN ".svn" EXCLUDE
         PATTERN "${PROJECT_NAME}" EXCLUDE )

# generate cmake config so other projects can find this library
# we support our cmake EXPORTS as imported targets
set(PROVIDES_EXPORTED_TARGETS 1)
# we need the public dependencies so create_cmake_config_files can find them as implicit dependencies
list(APPEND ${PROJECT_NAME}_PUBLIC_DEPENDENCIES "DOOCS Components server;ChimeraTK-ControlSystemAdapter")
include(${CMAKE_SOURCE_DIR}/cmake/create_cmake_config_files.cmake)
