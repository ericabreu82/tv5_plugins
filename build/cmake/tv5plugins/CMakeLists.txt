if(WIN32)
  add_definitions(-DTV5PLUGINSDLL)
endif()

set(TV5PLG_ABSOLUTE_ROOT_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../../..)

include_directories(
  ${TV5PLG_ABSOLUTE_ROOT_DIR}/src
  ${terralib_INCLUDE_DIRS}
  ${Boost_INCLUDE_DIR}
)

file(GLOB TV5PLG_SRC_FILES ${TV5PLG_ABSOLUTE_ROOT_DIR}/src/tv5plugins/*.cpp)
file(GLOB TV5PLG_HDR_FILES ${TV5PLG_ABSOLUTE_ROOT_DIR}/src/tv5plugins/*.h)

source_group("Source Files"  FILES ${TV5PLG_SRC_FILES})
source_group("Header Files"  FILES ${TV5PLG_HDR_FILES})

#tile generator
file(GLOB TV5PLG_TILE_SRC_FILES ${TV5PLG_ABSOLUTE_ROOT_DIR}/src/tv5plugins/tileGenerator/*.cpp)
file(GLOB TV5PLG_TILE_HDR_FILES ${TV5PLG_ABSOLUTE_ROOT_DIR}/src/tv5plugins/tileGenerator/*.h)

source_group("Source Files\\tileGenerator"  FILES ${TV5PLG_TILE_SRC_FILES})
source_group("Header Files\\tileGenerator"  FILES ${TV5PLG_TILE_HDR_FILES})

file(GLOB TV5PLG_TILE_CORE_SRC_FILES ${TV5PLG_ABSOLUTE_ROOT_DIR}/src/tv5plugins/tileGenerator/core/*.cpp)
file(GLOB TV5PLG_TILE_CORE_HDR_FILES ${TV5PLG_ABSOLUTE_ROOT_DIR}/src/tv5plugins/tileGenerator/core/*.h)

source_group("Source Files\\tileGenerator\\core"  FILES ${TV5PLG_TILE_CORE_SRC_FILES})
source_group("Header Files\\tileGenerator\\core"  FILES ${TV5PLG_TILE_CORE_HDR_FILES})

file(GLOB TV5PLG_TILE_QT_SRC_FILES ${TV5PLG_ABSOLUTE_ROOT_DIR}/src/tv5plugins/tileGenerator/qt/*.cpp)
file(GLOB TV5PLG_TILE_QT_HDR_FILES ${TV5PLG_ABSOLUTE_ROOT_DIR}/src/tv5plugins/tileGenerator/qt/*.h)

source_group("Source Files\\tileGenerator\\qt"  FILES ${TV5PLG_TILE_QT_SRC_FILES})
source_group("Header Files\\tileGenerator\\qt"  FILES ${TV5PLG_TILE_QT_HDR_FILES})

file(GLOB TV5PLG_TILE_UI_FILES  ${TV5PLG_ABSOLUTE_ROOT_DIR}/src/tv5plugins/tileGenerator/qt/ui/*.ui)

source_group("UI Files\\tileGenerator"  FILES ${TV5PLG_TILE_UI_FILES})

#forest monitor
file(GLOB TV5PLG_FOREST_SRC_FILES ${TV5PLG_ABSOLUTE_ROOT_DIR}/src/tv5plugins/forestMonitor/*.cpp)
file(GLOB TV5PLG_FOREST_HDR_FILES ${TV5PLG_ABSOLUTE_ROOT_DIR}/src/tv5plugins/forestMonitor/*.h)

source_group("Source Files\\forestMonitor"  FILES ${TV5PLG_FOREST_SRC_FILES})
source_group("Header Files\\forestMonitor"  FILES ${TV5PLG_FOREST_HDR_FILES})

file(GLOB TV5PLG_FOREST_CORE_SRC_FILES ${TV5PLG_ABSOLUTE_ROOT_DIR}/src/tv5plugins/forestMonitor/core/*.cpp)
file(GLOB TV5PLG_FOREST_CORE_HDR_FILES ${TV5PLG_ABSOLUTE_ROOT_DIR}/src/tv5plugins/forestMonitor/core/*.h)

source_group("Source Files\\forestMonitor\\core"  FILES ${TV5PLG_FOREST_CORE_SRC_FILES})
source_group("Header Files\\forestMonitor\\core"  FILES ${TV5PLG_FOREST_CORE_HDR_FILES})

file(GLOB TV5PLG_FOREST_QT_SRC_FILES ${TV5PLG_ABSOLUTE_ROOT_DIR}/src/tv5plugins/forestMonitor/qt/*.cpp)
file(GLOB TV5PLG_FOREST_QT_HDR_FILES ${TV5PLG_ABSOLUTE_ROOT_DIR}/src/tv5plugins/forestMonitor/qt/*.h)

source_group("Source Files\\forestMonitor\\qt"  FILES ${TV5PLG_FOREST_QT_SRC_FILES})
source_group("Header Files\\forestMonitor\\qt"  FILES ${TV5PLG_FOREST_QT_HDR_FILES})

file(GLOB TV5PLG_FOREST_TOOLS_QT_SRC_FILES ${TV5PLG_ABSOLUTE_ROOT_DIR}/src/tv5plugins/forestMonitor/qt/tools/*.cpp)
file(GLOB TV5PLG_FOREST_TOOLS_QT_HDR_FILES ${TV5PLG_ABSOLUTE_ROOT_DIR}/src/tv5plugins/forestMonitor/qt/tools/*.h)

source_group("Source Files\\forestMonitor\\qt\\tools"  FILES ${TV5PLG_FOREST_TOOLS_QT_SRC_FILES})
source_group("Header Files\\forestMonitor\\qt\\tools"  FILES ${TV5PLG_FOREST_TOOLS_QT_HDR_FILES})

file(GLOB TV5PLG_FOREST_UI_FILES  ${TV5PLG_ABSOLUTE_ROOT_DIR}/src/tv5plugins/forestMonitor/qt/ui/*.ui)

source_group("UI Files\\forestMonitor"  FILES ${TV5PLG_FOREST_UI_FILES})

#photo index
file(GLOB TV5PLG_PHOTOINDEX_SRC_FILES ${TV5PLG_ABSOLUTE_ROOT_DIR}/src/tv5plugins/photoIndex/*.cpp)
file(GLOB TV5PLG_PHOTOINDEX_HDR_FILES ${TV5PLG_ABSOLUTE_ROOT_DIR}/src/tv5plugins/photoIndex/*.h)

source_group("Source Files\\photoIndex"  FILES ${TV5PLG_PHOTOINDEX_SRC_FILES})
source_group("Header Files\\photoIndex"  FILES ${TV5PLG_PHOTOINDEX_HDR_FILES})

file(GLOB TV5PLG_PHOTOINDEX_CORE_SRC_FILES ${TV5PLG_ABSOLUTE_ROOT_DIR}/src/tv5plugins/photoIndex/core/*.cpp)
file(GLOB TV5PLG_PHOTOINDEX_CORE_HDR_FILES ${TV5PLG_ABSOLUTE_ROOT_DIR}/src/tv5plugins/photoIndex/core/*.h)

source_group("Source Files\\photoIndex\\core"  FILES ${TV5PLG_PHOTOINDEX_CORE_SRC_FILES})
source_group("Header Files\\photoIndex\\core"  FILES ${TV5PLG_PHOTOINDEX_CORE_HDR_FILES})

file(GLOB TV5PLG_PHOTOINDEX_QT_SRC_FILES ${TV5PLG_ABSOLUTE_ROOT_DIR}/src/tv5plugins/photoIndex/qt/*.cpp)
file(GLOB TV5PLG_PHOTOINDEX_QT_HDR_FILES ${TV5PLG_ABSOLUTE_ROOT_DIR}/src/tv5plugins/photoIndex/qt/*.h)

source_group("Source Files\\photoIndex\\qt"  FILES ${TV5PLG_PHOTOINDEX_QT_SRC_FILES})
source_group("Header Files\\photoIndex\\qt"  FILES ${TV5PLG_PHOTOINDEX_QT_HDR_FILES})

file(GLOB TV5PLG_PHOTOINDEX_UI_FILES  ${TV5PLG_ABSOLUTE_ROOT_DIR}/src/tv5plugins/photoIndex/qt/ui/*.ui)

source_group("UI Files\\photoIndex"  FILES ${TV5PLG_PHOTOINDEX_UI_FILES})

# uic'ing
QT5_WRAP_UI(TV5PLG_GEN_HDR_FILES ${TV5PLG_TILE_UI_FILES}
								   ${TV5PLG_FOREST_UI_FILES}
								   ${TV5PLG_PHOTOINDEX_UI_FILES})

set(TV5PLG_FILES ${TV5PLG_SRC_FILES} 
				   ${TV5PLG_HDR_FILES} 
				   ${TV5PLG_TILE_SRC_FILES} 
				   ${TV5PLG_TILE_HDR_FILES} 
				   ${TV5PLG_TILE_CORE_SRC_FILES} 
				   ${TV5PLG_TILE_CORE_HDR_FILES}
				   ${TV5PLG_TILE_QT_SRC_FILES} 
				   ${TV5PLG_TILE_QT_HDR_FILES}
				   ${TV5PLG_FOREST_SRC_FILES} 
				   ${TV5PLG_FOREST_HDR_FILES} 
				   ${TV5PLG_FOREST_CORE_SRC_FILES} 
				   ${TV5PLG_FOREST_CORE_HDR_FILES}
				   ${TV5PLG_FOREST_QT_SRC_FILES} 
				   ${TV5PLG_FOREST_QT_HDR_FILES}
				   ${TV5PLG_FOREST_TOOLS_QT_SRC_FILES}
				   ${TV5PLG_FOREST_TOOLS_QT_HDR_FILES}
				   ${TV5PLG_PHOTOINDEX_SRC_FILES} 
				   ${TV5PLG_PHOTOINDEX_HDR_FILES} 
				   ${TV5PLG_PHOTOINDEX_CORE_SRC_FILES} 
				   ${TV5PLG_PHOTOINDEX_CORE_HDR_FILES}
				   ${TV5PLG_PHOTOINDEX_QT_SRC_FILES} 
				   ${TV5PLG_PHOTOINDEX_QT_HDR_FILES}
				   ${TV5PLG_GEN_HDR_FILES})
				   
add_library(tv5_3rdparty_plugins SHARED ${TV5PLG_FILES})

target_link_libraries(tv5_3rdparty_plugins terralib_mod_plugin terralib_mod_qt_apf)

qt5_use_modules(tv5_3rdparty_plugins Widgets)

add_custom_command(
      TARGET tv5_3rdparty_plugins
      POST_BUILD
      COMMAND ${CMAKE_COMMAND} -E copy "${TV5PLG_ABSOLUTE_ROOT_DIR}/share/terralib/plugins/te.qt.tv5plugins.teplg" "${terralib_DIR}/../../../share/terralib/plugins"
	  COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_BINARY_DIR}/bin/Debug/tv5_3rdparty_plugins.dll" "${terralib_DIR}/../../../bin"
      
      COMMENT "Copying plugin files..."
      VERBATIM
    )

install(TARGETS tv5_3rdparty_plugins
        RUNTIME DESTINATION bin
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib)
