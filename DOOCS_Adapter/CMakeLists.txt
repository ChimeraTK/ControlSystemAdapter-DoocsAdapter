cmake_minimum_required (VERSION 2.6)
project (DOOCS_Adapter)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -pedantic -Wextra -Wshadow -Wuninitialized")

# --- boost ---
FIND_PACKAGE(Boost REQUIRED)
include_directories(${Boost_INCLUDE_DIRS})

FIND_PACKAGE(Boost COMPONENTS unit_test_framework REQUIRED)
message(status "** Boost Include: ${Boost_INCLUDE_DIR}")
message(status "** Boost Libraries: ${Boost_LIBRARY_DIRS}")
message(status "** Boost Libraries: ${Boost_LIBRARIES}")

INCLUDE_DIRECTORIES(${Boost_INCLUDE_DIR})
LINK_DIRECTORIES(${Boost_LIBRARY_DIRS})



include_directories(${CMAKE_SOURCE_DIR}/include)
aux_source_directory(${CMAKE_SOURCE_DIR}/src library_sources)


include_directories(${CMAKE_SOURCE_DIR}/tests/include)
aux_source_directory(${CMAKE_SOURCE_DIR}/tests/src testSources)
foreach( testSourceFile ${testSources})
    #NAME_WE means the base name without path and (longest) extension
    get_filename_component(excutableName ${testSourceFile} NAME_WE)
    add_executable(${excutableName} ${testSourceFile})
endforeach( testSourceFile )
