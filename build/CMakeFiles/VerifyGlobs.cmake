# CMAKE generated file: DO NOT EDIT!
# Generated by CMake Version 3.27
cmake_policy(SET CMP0009 NEW)

# sources at cmake/CMakeSources.cmake:5 (file)
file(GLOB_RECURSE NEW_GLOB LIST_DIRECTORIES false "/home/miguelyermo/Material/TFG/t-basteiro/src/*.cpp")
set(OLD_GLOB
  "/home/miguelyermo/Material/TFG/t-basteiro/src/TimeWatcher.cpp"
  "/home/miguelyermo/Material/TFG/t-basteiro/src/geometry.cpp"
  "/home/miguelyermo/Material/TFG/t-basteiro/src/handlers.cpp"
  "/home/miguelyermo/Material/TFG/t-basteiro/src/main.cpp"
  "/home/miguelyermo/Material/TFG/t-basteiro/src/main_options.cpp"
  "/home/miguelyermo/Material/TFG/t-basteiro/src/octree.cpp"
  "/home/miguelyermo/Material/TFG/t-basteiro/src/plane.cpp"
  "/home/miguelyermo/Material/TFG/t-basteiro/src/readers/FileReaderFactory.cpp"
  "/home/miguelyermo/Material/TFG/t-basteiro/src/readers/LasFileReader.cpp"
  "/home/miguelyermo/Material/TFG/t-basteiro/src/readers/TxtFileReader.cpp"
  )
if(NOT "${NEW_GLOB}" STREQUAL "${OLD_GLOB}")
  message("-- GLOB mismatch!")
  file(TOUCH_NOCREATE "/home/miguelyermo/Material/TFG/t-basteiro/build/CMakeFiles/cmake.verify_globs")
endif()
