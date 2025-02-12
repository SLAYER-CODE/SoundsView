# Additional clean files
cmake_minimum_required(VERSION 3.16)

if("${CONFIG}" STREQUAL "" OR "${CONFIG}" STREQUAL "Debug")
  file(REMOVE_RECURSE
  "CMakeFiles/SoundsView_autogen.dir/AutogenUsed.txt"
  "CMakeFiles/SoundsView_autogen.dir/ParseCache.txt"
  "QHotkeys/CMakeFiles/QHotkeys_autogen.dir/AutogenUsed.txt"
  "QHotkeys/CMakeFiles/QHotkeys_autogen.dir/ParseCache.txt"
  "QHotkeys/QHotkeys_autogen"
  "QtAwesome/CMakeFiles/QtAwesome_autogen.dir/AutogenUsed.txt"
  "QtAwesome/CMakeFiles/QtAwesome_autogen.dir/ParseCache.txt"
  "QtAwesome/QtAwesome_autogen"
  "SoundsView_autogen"
  )
endif()
