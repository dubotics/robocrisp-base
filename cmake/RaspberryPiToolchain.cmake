# this one is important
set(CMAKE_SYSTEM_NAME Linux)
#this one not so much
set(CMAKE_SYSTEM_VERSION 1)

set(CMAKE_LIBRARY_ARCHITECTURE "arm-linux-gnueabihf")
set(PLATFORM "arm-linux-gnueabihf")
if(NOT TOOLCHAIN_PREFIX)
  set(TOOLCHAIN_PREFIX "")
endif(NOT TOOLCHAIN_PREFIX)

set(CMAKE_C_COMPILER "${PLATFORM}-gcc")
set(CMAKE_CXX_COMPILER "${PLATFORM}-g++")

# search for programs in the build host directories
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM BOTH)

# for libraries and headers in the target directories
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)

add_definitions(
  "--sysroot=${CMAKE_SYSTEM_PREFIX_PATH}"
  "-march=armv6zk"
  "-mcpu=arm1176jzf-s"
  "-mfloat-abi=hard"
  "-mfpu=vfp"
  "-marm"
  )
