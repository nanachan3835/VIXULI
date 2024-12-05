# The following lines of boilerplate have to be in your project's
# CMakeLists in this exact order for cmake to work correctly
cmake_minimum_required(VERSION 3.16)

include($ENV{IDF_PATH}/tools/cmake/project.cmake)

# uncomment the 2 following lines to switch to test mode
# add_compile_definitions(TESTING)
# set(EXTRA_COMPONENT_DIRS "test")

project(Test)
