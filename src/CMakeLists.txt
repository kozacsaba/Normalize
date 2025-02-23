# Source Library ###############################################################

message(STATUS "### Configuring Source Library ###")

add_library(Source)

target_sources(Source PRIVATE
    Main.cpp
    MainComponent.cpp

    processor/MainProcessor.cpp
    processor/LKFSProcessor.cpp
    processor/FilterProcessor.cpp
    processor/FileHandler.cpp

    gui/FileList.cpp
)

target_include_directories(Source 
    PUBLIC 
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${JUCE_SOURCE_DIR}/modules
)

# juce_add_binary_data(GuiAppData SOURCES ...)

target_link_libraries(Source
        # GuiAppData
        juce::juce_core
        juce::juce_data_structures
        juce::juce_events
        juce::juce_graphics
        juce::juce_audio_basics
        juce::juce_audio_formats
        juce::juce_gui_basics
        juce::juce_recommended_config_flags
        juce::juce_recommended_warning_flags
        juce::juce_recommended_lto_flags
    )

target_compile_features(Source PUBLIC cxx_std_20)

target_compile_definitions(Source PUBLIC
    JUCE_WEB_BROWSER=0  # If you remove this, add `NEEDS_WEB_BROWSER TRUE` to the `juce_add_gui_app` call
    JUCE_USE_CURL=0     # If you remove this, add `NEEDS_CURL TRUE` to the `juce_add_gui_app` call
    JUCE_APPLICATION_NAME_STRING="${PREDEF_PROJECT_NAME}"
    JUCE_APPLICATION_VERSION_STRING="${PREDEF_PROJECT_VERSION}"
    JUCE_USE_MP3AUDIOFORMAT
)

# App ##########################################################################

message(STATUS "### Configuring Application ###")

project("${PREDEF_PROJECT_NAME}App" VERSION ${PREDEF_PROJECT_VERSION})

# juce_set_vst2_sdk_path(...)

juce_add_gui_app(${PROJECT_NAME}
    # ICON_BIG ...                      # ICON_* arguments specify a path to an
    # ICON_SMALL ...                    #   image file to use as an icon
    COMPANY_NAME "Csaba Koza"           # Specify the name of the app's author
    PRODUCT_NAME ${PREDEF_PROJECT_NAME}
    BUNDLE_ID "com.Csaba_Koza.Normalize")

target_link_libraries(${PROJECT_NAME} PUBLIC Source)

#target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
