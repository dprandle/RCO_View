cmake_minimum_required(VERSION 3.10.0)
project(RCO_View)

foreach(path ${CMAKE_PREFIX_PATH})
  message("Prefix Path/s: " ${path})
endforeach(path)

set(TARGET_NAME RCO_View)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS ON)

find_package(Qt5Core REQUIRED)
find_package(Qt5Widgets REQUIRED)
find_package(Qt5Gui REQUIRED)
find_package(Qt5SerialPort REQUIRED)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

# Set the source directories
set(RCO_VIEW_SRC_DIR ${CMAKE_SOURCE_DIR}/src)
set(RCO_VIEW_INCLUDE_DIR ${CMAKE_SOURCE_DIR}/include)
set(UI_INCLUDE_DIR ${CMAKE_SOURCE_DIR}/ui)
set(MOC_INCLUDE_DIR ${CMAKE_SOURCE_DIR}/build/${CMAKE_BUILD_TYPE}/${TARGET_NAME}_autogen/include)

set(CMAKE_AUTOUIC_SEARCH_PATHS ${UI_INCLUDE_DIR})
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# Set the src files for the project
file(GLOB SRC_FILES 
  "${RCO_VIEW_SRC_DIR}/*.cpp"
  )

qt5_add_resources(SRC_RES_FILE ${CMAKE_SOURCE_DIR}/icons/resources.qrc)

# Set project includes dir
include_directories(
  ${RCO_VIEW_INCLUDE_DIR}
  ${MOC_INCLUDE_DIR}
  # ${Qt5Core_INCLUDE_DIRS}
  # ${Qt5Widgets_INCLUDE_DIRS}
  # ${Qt5Gui_INCLUDE_DIRS}
)

add_definitions(${Qt5Core_DEFINITIONS})
add_definitions(${Qt5Widgets_DEFINITIONS})
add_definitions(${Qt5Gui_DEFINITIONS})
add_definitions(${Qt5SerialPort_DEFINITIONS})
add_compile_definitions(${Qt5Core_COMPILE_DEFINITIONS})
add_compile_definitions(${Qt5Widgets_COMPILE_DEFINITIONS})
add_compile_definitions(${Qt5Gui_COMPILE_DEFINITIONS})
add_compile_definitions(${Qt5SerialPort_COMPILE_DEFINITIONS})

add_definitions(-DSPDLOG_ACTIVE_LEVEL=SPDLOG_LEVEL_TRACE)

if (${CMAKE_BUILD_TYPE} STREQUAL Debug)
  add_definitions(-DDEBUG_VERSION)
else()
  add_definitions(-DRELEASE_VERSION)
  add_definitions(-DQT_NO_DEBUG)
endif()


if (WIN32)
elseif(APPLE)
  set(CMAKE_MACOSX_RPATH 1)
  set_source_files_properties(${CMAKE_SOURCE_DIR}/icons/rco_view.icns PROPERTIES MACOSX_PACKAGE_LOCATION "Resources")
  link_directories(/usr/local/lib)
  include_directories(/usr/local/include)
  add_executable(${TARGET_NAME} MACOSX_BUNDLE ${SRC_FILES} ${SRC_RES_FILE} ${CMAKE_SOURCE_DIR}/icons/rco_view.icns)
  set_target_properties(${TARGET_NAME} PROPERTIES MACOSX_BUNDLE_INFO_PLIST ${CMAKE_SOURCE_DIR}/cmake/${TARGET_NAME}.plist)
else()
endif()

if (${CMAKE_BUILD_TYPE} STREQUAL Release)
    target_compile_options(${TARGET_NAME} PRIVATE -Werror -Wall -Wextra)
endif()

target_link_libraries(${TARGET_NAME}
  Qt5::Core
  Qt5::Widgets
  Qt5::Gui
  Qt5::SerialPort
  spdlog
  )