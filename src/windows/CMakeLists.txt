project(psychokinesis)

cmake_minimum_required(VERSION 2.8)

INCLUDE_DIRECTORIES(/home/complier/mingw32/include)
LINK_DIRECTORIES(/home/complier/mingw32/lib)

SET(SRC_LIST api/api_communication.cpp
	api/api_download.cpp
	adapter/adapter_communication.cpp
	adapter/adapter_download.cpp
	process/control.cpp
	)
	
if(NOT CMAKE_BUILD_TYPE)
	SET(CMAKE_BUILD_TYPE Debug)
endif()

ADD_DEFINITIONS("-Wall -DPSYCHOKINESIS_DEBUG")
SET(CMAKE_EXE_LINKER_FLAGS "-static-libgcc -static-libstdc++ -static")

ADD_EXECUTABLE(psychokinesis ${SRC_LIST})

TARGET_LINK_LIBRARIES(psychokinesis aria2)
TARGET_LINK_LIBRARIES(psychokinesis gloox)
TARGET_LINK_LIBRARIES(psychokinesis z)
TARGET_LINK_LIBRARIES(psychokinesis dnsapi)
TARGET_LINK_LIBRARIES(psychokinesis crypt32)
TARGET_LINK_LIBRARIES(psychokinesis secur32)
TARGET_LINK_LIBRARIES(psychokinesis ws2_32)
TARGET_LINK_LIBRARIES(psychokinesis wsock32)
TARGET_LINK_LIBRARIES(psychokinesis gdi32)
TARGET_LINK_LIBRARIES(psychokinesis winmm)
TARGET_LINK_LIBRARIES(psychokinesis iphlpapi)
TARGET_LINK_LIBRARIES(psychokinesis psapi)
TARGET_LINK_LIBRARIES(psychokinesis boost_system)
TARGET_LINK_LIBRARIES(psychokinesis boost_thread_win32)
TARGET_LINK_LIBRARIES(psychokinesis expat)
