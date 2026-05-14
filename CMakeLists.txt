cmake_minimum_required(VERSION 3.5)

project(SoundsView VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets LinguistTools Core )
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets LinguistTools)
add_subdirectory(QtAwesome)
include_directories(QtAwesome)

# QHotkeys eliminado — usa XGrabKey (no funciona en Wayland)
# En su lugar: SIGUSR1 desde niri config

set(TS_FILES SoundsView_es_PE.ts)

set(PROJECT_SOURCES
        main.cpp
        mainwindow.cpp
        mainwindow.h
        mainwindow.ui
        ${TS_FILES}
)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    add_executable(SoundsView
        ${PROJECT_SOURCES}
        circulartrianglebutton.h circulartrianglebutton.cpp
        voiceroulette.h voiceroulette.cpp
        polygonbutton.h polygonbutton.cpp
        datamodel.h datamodel.cpp
        dataview.h dataview.cpp
        datacontroller.h datacontroller.cpp
        iconmanager.h iconmanager.cpp
        utils.h utils.cpp
        cirlebutoneditconvert.h cirlebutoneditconvert.cpp
        circularbutton.h circularbutton.cpp
        images.qrc
        structdata.h structdata.cpp


    )
# Define target properties for Android with Qt 6 as:
#    set_property(TARGET SoundsView APPEND PROPERTY QT_ANDROID_PACKAGE_SOURCE_DIR
#                 ${CMAKE_CURRENT_SOURCE_DIR}/android)
# For more information, see https://doc.qt.io/qt-6/qt-add-executable.html#target-creation

    qt_create_translation(QM_FILES ${CMAKE_SOURCE_DIR} ${TS_FILES})
else()
    if(ANDROID)
        add_library(SoundsView SHARED
            ${PROJECT_SOURCES}
        )
# Define properties for Android with Qt 5 after find_package() calls as:
#    set(ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/android")
    else()
        add_executable(SoundsView
            ${PROJECT_SOURCES}
        )
    endif()

    qt5_create_translation(QM_FILES ${CMAKE_SOURCE_DIR} ${TS_FILES})
endif()
target_link_libraries(SoundsView PRIVATE QtAwesome Qt${QT_VERSION_MAJOR}::Widgets)

# Qt for iOS sets MACOSX_BUNDLE_GUI_IDENTIFIER automatically since Qt 6.1.
# If you are developing for iOS or macOS you should consider setting an
# explicit, fixed bundle identifier manually though.
if(${QT_VERSION} VERSION_LESS 6.1.0)
  set(BUNDLE_ID_OPTION MACOSX_BUNDLE_GUI_IDENTIFIER com.example.SoundsView)
endif()
set_target_properties(SoundsView PROPERTIES
    ${BUNDLE_ID_OPTION}
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

include(GNUInstallDirs)

install(TARGETS SoundsView
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

# qt_finalize_executable skipped — using add_executable instead of qt_add_executable
# to avoid QML import scanner (version mismatch in Qt installation)
