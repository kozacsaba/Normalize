cmake_minimum_required(VERSION 3.22)

project(Normalize VERSION 0.1)

set(CMAKE_CXX_STANDARD 20)

add_subdirectory(submodules/juce)
add_subdirectory(submodules/googletest)

# juce_set_vst2_sdk_path(...)

juce_add_gui_app(Normalize
    # ICON_BIG ...                      # ICON_* arguments specify a path to an image file to use as an icon
    # ICON_SMALL ...
    COMPANY_NAME "Csaba Koza"           # Specify the name of the app's author
    PRODUCT_NAME "Audio File Normalizer App")

target_sources(Normalize PRIVATE
    src/Main.cpp
    src/MainComponent.cpp
    
    src/processor/MainProcessor.cpp
    src/processor/LKFSProcessor.cpp
    
    src/gui/FileList.cpp
    
    src/util/Logger.cpp
    )

target_compile_definitions(Normalize PRIVATE
    JUCE_WEB_BROWSER=0  # If you remove this, add `NEEDS_WEB_BROWSER TRUE` to the `juce_add_gui_app` call
    JUCE_USE_CURL=0     # If you remove this, add `NEEDS_CURL TRUE` to the `juce_add_gui_app` call
    JUCE_APPLICATION_NAME_STRING="$<TARGET_PROPERTY:Normalize,JUCE_PRODUCT_NAME>"
    JUCE_APPLICATION_VERSION_STRING="$<TARGET_PROPERTY:Normalize,JUCE_VERSION>")

# juce_add_binary_data(GuiAppData SOURCES ...)

target_link_libraries(Normalize PRIVATE
    # GuiAppData
    juce::juce_core
    juce::juce_audio_formats
    juce::juce_gui_extra
)
