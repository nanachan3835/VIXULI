# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION 3.5)

file(MAKE_DIRECTORY
  "/home/nanachanuwu/ESP/esp-idf/components/bootloader/subproject"
  "/home/nanachanuwu/CODE_LEARN_CODE_BLOCK/VXL/hello_world/build/bootloader"
  "/home/nanachanuwu/CODE_LEARN_CODE_BLOCK/VXL/hello_world/build/bootloader-prefix"
  "/home/nanachanuwu/CODE_LEARN_CODE_BLOCK/VXL/hello_world/build/bootloader-prefix/tmp"
  "/home/nanachanuwu/CODE_LEARN_CODE_BLOCK/VXL/hello_world/build/bootloader-prefix/src/bootloader-stamp"
  "/home/nanachanuwu/CODE_LEARN_CODE_BLOCK/VXL/hello_world/build/bootloader-prefix/src"
  "/home/nanachanuwu/CODE_LEARN_CODE_BLOCK/VXL/hello_world/build/bootloader-prefix/src/bootloader-stamp"
)

set(configSubDirs )
foreach(subDir IN LISTS configSubDirs)
    file(MAKE_DIRECTORY "/home/nanachanuwu/CODE_LEARN_CODE_BLOCK/VXL/hello_world/build/bootloader-prefix/src/bootloader-stamp/${subDir}")
endforeach()
if(cfgdir)
  file(MAKE_DIRECTORY "/home/nanachanuwu/CODE_LEARN_CODE_BLOCK/VXL/hello_world/build/bootloader-prefix/src/bootloader-stamp${cfgdir}") # cfgdir has leading slash
endif()
