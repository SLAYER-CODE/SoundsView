cmake_minimum_required(VERSION 3.5)

project(SoundsView VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets Core)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets)
add_subdirectory(lib/QtAwesome)

# Qt6 Multimedia and TextToSpeech modules at 6.10.2 have ABI mismatch with 6.11.0 core.
# Using system audio tools (paplay/pw-play/ffplay) and speech-dispatcher via QProcess instead.

set(PROJECT_SOURCES
    src/main.cpp
)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    add_executable(SoundsView
        ${PROJECT_SOURCES}
        src/ui/voiceroulette.h src/ui/voiceroulette.cpp
        src/ui/polygonbutton.h src/ui/polygonbutton.cpp
        src/ui/circulartrianglebutton.h src/ui/circulartrianglebutton.cpp
        src/ui/cirlebutoneditconvert.h src/ui/cirlebutoneditconvert.cpp
        src/ui/circularbutton.h src/ui/circularbutton.cpp
        src/ui/structdata.h src/ui/structdata.cpp
        src/common/iconmanager.h src/common/iconmanager.cpp
        src/common/utils.h src/common/utils.cpp
        src/core/audiomanager.h src/core/audiomanager.cpp
        src/core/soundmanager.h src/core/soundmanager.cpp
        resources/images.qrc
    )
endif()

target_include_directories(SoundsView PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/src/ui
    ${CMAKE_CURRENT_SOURCE_DIR}/src/common
    ${CMAKE_CURRENT_SOURCE_DIR}/src/core
    ${CMAKE_CURRENT_SOURCE_DIR}/lib/QtAwesome
)

target_link_libraries(SoundsView PRIVATE QtAwesome Qt${QT_VERSION_MAJOR}::Widgets)

# Qt6 Multimedia link omitted due to ABI mismatch (6.10.2 vs 6.11.0)
# Audio playback uses QProcess + system tools instead.

include(GNUInstallDirs)

install(TARGETS SoundsView
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIN}
)
