# Main ########################################################################

cmake_minimum_required(VERSION 3.22)

set(PROJECT_ROOT ${CMAKE_CURRENT_LIST_DIR})
set(PREDEF_PROJECT_NAME "Normalize")
set(PREDEF_PROJECT_VERSION 0.1)

project("${PREDEF_PROJECT_NAME}_Project" VERSION ${PREDEF_PROJECT_VERSION})

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-comment")

# ZLib ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

message(STATUS ">>>>> Configuring ZLib")

set(ZLIB_INSTALL OFF CACHE BOOL "" FORCE)
set(ZLIB_BUILD_TESTING OFF CACHE BOOL "" FORCE)
set(ZLIB_BUILD_SHARED OFF CACHE BOOL "" FORCE)
set(ZLIB_BUILD_STATIC ON CACHE BOOL "" FORCE)

add_subdirectory(submodules/zlib)
set(ZLIB_BIN "${PROJECT_ROOT}/build/${CMAKE_BUILD_TYPE}/submodules/zlib")
set(ZLIB_SRC "${PROJECT_ROOT}/submodules/zlib")
message(STATUS "ZLin Binary path set to: ${ZLIB_BIN}")
message(STATUS "ZLib Source path set to: ${ZLIB_SRC}")

message(STATUS "<<<<< Done configuring ZLib")

# taglib ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

message(STATUS ">>>>> Configuring TagLib")

set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
set(BUILD_TESTING OFF CACHE BOOL "" FORCE)
set(WITH_ZLIB ON CACHE BOOL "" FORCE)
set(ZLIB_ROOT "${ZLIB_SRC}" CACHE STRING "" FORCE)
set(ZLIB_INCLUDE_DIR "${ZLIB_SRC}" CACHE STRING "" FORCE)
set(ZLIB_LIBRARY "${ZLIB_BIN}" CACHE STRING "" FORCE)

set(WITH_APE OFF CACHE BOOL "" FORCE)
set(WITH_ASF OFF CACHE BOOL "" FORCE)
set(WITH_DSF OFF CACHE BOOL "" FORCE)
set(WITH_MOD OFF CACHE BOOL "" FORCE)
set(WITH_MP4 OFF CACHE BOOL "" FORCE)
set(WITH_SHORTEN OFF CACHE BOOL "" FORCE)
set(WITH_TRUEAUDIO OFF CACHE BOOL "" FORCE)
set(WITH_VORBIS OFF CACHE BOOL "" FORCE)

add_subdirectory(submodules/taglib)
set(TAGLIB_ROOT "${PROJECT_ROOT}/submodules/taglib")

message(STATUS "<<<<< Done configuring TagLib")

# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

add_subdirectory(submodules/mp3gain)
add_subdirectory(submodules/juce)
add_subdirectory(submodules/googletest)

add_subdirectory(src)
add_subdirectory(tests)
