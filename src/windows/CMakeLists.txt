project(psychokinesis)

cmake_minimum_required(VERSION 2.8)

SET(MINGW32_APTH /home/complier/mingw32)
INCLUDE_DIRECTORIES(${MINGW32_APTH}/include)
LINK_DIRECTORIES(${MINGW32_APTH}/lib)

if(MINGW)
  set(CMAKE_RC_COMPILER_INIT windres)
  ENABLE_LANGUAGE(RC)
  SET(CMAKE_RC_COMPILE_OBJECT
    "<CMAKE_RC_COMPILER> --codepage=936 -O coff -i <SOURCE> -o <OBJECT>")
endif(MINGW)

SET(SRC_LIST api/api_communication.cpp
	api/api_download.cpp
	adapter/adapter_communication.cpp
	adapter/adapter_download.cpp
	process/control.cpp
	process/ui_control.cpp
	ui/encoding_changer.cpp
	ui/api_message.cpp
	ui/frame_window.cpp
	ui/psychokinesis.cpp
	ui/psychokinesis.rc
	)
	
if(NOT CMAKE_BUILD_TYPE)
	SET(CMAKE_BUILD_TYPE Debug)
endif()
if(${CMAKE_BUILD_TYPE} MATCHES Debug)
	ADD_DEFINITIONS("-Wall -DPSYCHOKINESIS_DEBUG")
endif(${CMAKE_BUILD_TYPE} MATCHES Debug)

SET(CMAKE_CXX_FLAGS "-fpermissive")
SET(CMAKE_EXE_LINKER_FLAGS "-static-libgcc -static-libstdc++ -static -s -Wl,--subsystem,windows")

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
TARGET_LINK_LIBRARIES(psychokinesis boost_filesystem)
TARGET_LINK_LIBRARIES(psychokinesis boost_serialization)
TARGET_LINK_LIBRARIES(psychokinesis boost_thread_win32)
TARGET_LINK_LIBRARIES(psychokinesis expat)
TARGET_LINK_LIBRARIES(psychokinesis ${MINGW32_APTH}/lib/libUIlib.dll.a)
TARGET_LINK_LIBRARIES(psychokinesis ole32)