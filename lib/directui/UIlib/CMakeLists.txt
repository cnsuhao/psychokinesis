project(UIlib)

cmake_minimum_required(VERSION 2.8)

INCLUDE_DIRECTORIES(/home/complier/mingw32/include)
LINK_DIRECTORIES(/home/complier/mingw32/lib)

SET(SRC_LIST UILabel.cpp
			UIManager.cpp
			UIAnim.cpp
			UIActiveX.cpp
			UITool.cpp
			UIBlue.cpp
			UIDlgBuilder.cpp
			UIList.cpp
			UIDecoration.cpp
			UIEdit.cpp
			UITab.cpp
			UICombo.cpp
			UIlib.cpp
			UIPanel.cpp
			UIContainer.cpp
			StdAfx.cpp
			UIBase.cpp
			UIMarkup.cpp
			UIButton.cpp
	)
	
if(NOT CMAKE_BUILD_TYPE)
	SET(CMAKE_BUILD_TYPE Release)
endif()

ADD_DEFINITIONS("-DUILIB_EXPORTS")
SET(CMAKE_SHARED_LINKER_FLAGS "-static")
SET(CMAKE_CXX_FLAGS "-fpermissive")

ADD_LIBRARY(UIlib SHARED ${SRC_LIST})

TARGET_LINK_LIBRARIES(UIlib user32)
TARGET_LINK_LIBRARIES(UIlib gdi32)
TARGET_LINK_LIBRARIES(UIlib comctl32)
TARGET_LINK_LIBRARIES(UIlib uuid)
TARGET_LINK_LIBRARIES(UIlib d3d9)
TARGET_LINK_LIBRARIES(UIlib winmm)
