#----------------------------------------------------------------
# Generated CMake target import file for configuration "Debug".
#----------------------------------------------------------------

# Commands may need to know the format version.
set(CMAKE_IMPORT_FILE_VERSION 1)

# Import target "QtAwesome::QtAwesome" for configuration "Debug"
set_property(TARGET QtAwesome::QtAwesome APPEND PROPERTY IMPORTED_CONFIGURATIONS DEBUG)
set_target_properties(QtAwesome::QtAwesome PROPERTIES
  IMPORTED_LINK_INTERFACE_LANGUAGES_DEBUG "CXX"
  IMPORTED_LOCATION_DEBUG "${_IMPORT_PREFIX}/lib/libQtAwesome.a"
  )

list(APPEND _cmake_import_check_targets QtAwesome::QtAwesome )
list(APPEND _cmake_import_check_files_for_QtAwesome::QtAwesome "${_IMPORT_PREFIX}/lib/libQtAwesome.a" )

# Commands beyond this point should not need to know the version.
set(CMAKE_IMPORT_FILE_VERSION)
