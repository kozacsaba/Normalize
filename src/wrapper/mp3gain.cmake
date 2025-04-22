# mp3gain ######################################################################

message(STATUS "### Configuring mp3gain library ###")

set(MP3SRC "${PROJECT_ROOT}/submodules/mp3gain/mp3gain")
message(STATUS "mp3gain library source directory set to ${MP3SRC}")

add_library(mp3gain STATIC)

target_sources(mp3gain PUBLIC
    ${MP3SRC}/mp3gain.c
    ${MP3SRC}/apetag.c
    ${MP3SRC}/rg_error.c
    ${MP3SRC}/gain_analysis.c
    ${MP3SRC}/id3tag.c
    #${MP3SRC}/replaygaindll.c
    
    ${MP3SRC}/mpglibDBL/common.c
    ${MP3SRC}/mpglibDBL/dct64_i386.c
    ${MP3SRC}/mpglibDBL/decode_i386.c
    ${MP3SRC}/mpglibDBL/interface.c
    ${MP3SRC}/mpglibDBL/layer1.c
    ${MP3SRC}/mpglibDBL/layer2.c
    ${MP3SRC}/mpglibDBL/layer3.c
    ${MP3SRC}/mpglibDBL/tabinit.c
)

target_compile_options(mp3gain PRIVATE
    -Wall
)

# this should not always have WIN32 defined, only on windows

target_compile_definitions(mp3gain PUBLIC
    HAVE_MEMCPY
    WIN32
)

# target_link_libraries(mp3gain PUBLIC m)

target_include_directories(mp3gain PUBLIC
    ${MP3SRC}/..
)

