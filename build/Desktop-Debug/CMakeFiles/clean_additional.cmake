# Additional clean files
cmake_minimum_required(VERSION 3.16)

if("${CONFIG}" STREQUAL "" OR "${CONFIG}" STREQUAL "Debug")
  file(REMOVE_RECURSE
  "CMakeFiles/DesktopFileCreator_autogen.dir/AutogenUsed.txt"
  "CMakeFiles/DesktopFileCreator_autogen.dir/ParseCache.txt"
  "DesktopFileCreator_autogen"
  )
endif()
