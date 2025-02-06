# Main ########################################################################

cmake_minimum_required(VERSION 3.22)

project(Normalize VERSION 0.1)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# JUCE ########################################################################

add_subdirectory(submodules/juce)

# juce_set_vst2_sdk_path(...)

juce_add_gui_app(Normalize
    # ICON_BIG ...                      # ICON_* arguments specify a path to an
    # ICON_SMALL ...                    #   image file to use as an icon
    COMPANY_NAME "Csaba Koza"           # Specify the name of the app's author
    PRODUCT_NAME "Normalize"
    BUNDLE_ID "com.Csaba_Koza.Normalize")

target_include_directories(Normalize PRIVATE 
                           ${CMAKE_SOURCE_DIR}/src)

target_sources(Normalize PRIVATE
    src/Main.cpp
    src/MainComponent.cpp
    
    src/processor/MainProcessor.cpp
    src/processor/LKFSProcessor.cpp
    src/processor/FilterProcessor.cpp
    src/processor/FileHandler.cpp
    
    src/gui/FileList.cpp
    
    src/util/Logger.cpp
    src/util/CircularArray.h
    )

target_compile_definitions(Normalize PRIVATE
    JUCE_WEB_BROWSER=0  # If you remove this, add `NEEDS_WEB_BROWSER TRUE` to the `juce_add_gui_app` call
    JUCE_USE_CURL=0     # If you remove this, add `NEEDS_CURL TRUE` to the `juce_add_gui_app` call
    JUCE_APPLICATION_NAME_STRING="$<TARGET_PROPERTY:Normalize,JUCE_PRODUCT_NAME>"
    JUCE_APPLICATION_VERSION_STRING="$<TARGET_PROPERTY:Normalize,JUCE_VERSION>"
    JUCE_USE_MP3AUDIOFORMAT
)

target_compile_features(Normalize PUBLIC cxx_std_20)


# juce_add_binary_data(GuiAppData SOURCES ...)

target_link_libraries(Normalize PRIVATE
    # GuiAppData
    juce::juce_core
    juce::juce_audio_basics
    juce::juce_audio_formats
    juce::juce_gui_basics
)

# Google Test #################################################################

add_subdirectory(submodules/googletest)

set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

enable_testing()

add_executable(
    TestBench
    tests/TestRunner.cc
)

add_dependencies(TestBench Normalize)

target_link_libraries(TestBench GTest::gtest)

include(GoogleTest)
gtest_discover_tests(TestBench)
