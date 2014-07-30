# the name of the target operating system
SET(CMAKE_SYSTEM_NAME Windows)
SET(COMPILER_PREFIX "i686-w64-mingw32")
find_program(CMAKE_RC_COMPILER NAMES ${COMPILER_PREFIX}-windres)
find_program(CMAKE_C_COMPILER NAMES ${COMPILER_PREFIX}-gcc)
find_program(CMAKE_CXX_COMPILER NAMES ${COMPILER_PREFIX}-g++)
