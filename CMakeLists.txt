cmake_minimum_required (VERSION 3.16)
project(expirationTracker DESCRIPTION "Track expiration dates" LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)

set(CXXFLAGS  "-Wall")
set(CMAKE_CXX_FLAGS "${CXXFLAGS}")

find_package (glog 0.4.0 REQUIRED)
find_package(SDL2 REQUIRED)
find_package(SDL2_image REQUIRED)
find_package(SDL2_ttf REQUIRED)
find_package(OpenCV REQUIRED)
find_package(SQLite3 REQUIRED)

include_directories(${SDL2_INCLUDE_DIRS} ${SDL2_IMAGE_INCLUDE_DIRS})

add_executable (expirationTracker 
  main.cpp
  food_item.cpp
  
  # Display process after forked from main
  display/src/display_entry.cpp

  # Handler for messages from other processes
  display/src/external_handler.cpp

  # Database operations on food items
  display/src/sql_food.cpp

  # Entry into SDL after forked from displayEntry
  display/src/sdl2/sdl_entry.cpp

  # GUI 
  display/src/sdl2/display.cpp
  display/src/sdl2/sdl_debug.cpp

  # GUI Elements
  display/src/sdl2/elements/element.cpp
  display/src/sdl2/elements/button.cpp
  display/src/sdl2/elements/text.cpp
  display/src/sdl2/elements/panel.cpp
  display/src/sdl2/elements/scroll_box.cpp
  display/src/sdl2/elements/number_setting.cpp

  # GUI States
  display/src/sdl2/states/state.cpp
  display/src/sdl2/states/main_menu.cpp
  display/src/sdl2/states/scanning.cpp
  display/src/sdl2/states/pause_menu.cpp
  display/src/sdl2/states/item_list.cpp

  # Vision process after forked from main
  vision/src/vision_pipe.cpp
  vision/src/handleOCR.cpp
  vision/src/handleOBJ.cpp
  vision/src/validateDetection.cpp
  vision/src/analyzeImages.cpp
  vision/src/helperFunctions.cpp

  #Hardware process after forked from main
  hardware/src/hardware_entry.cpp
  hardware/src/io.cpp
  hardware/src/controls/controls.cpp
  hardware/src/controls/cameras.cpp
  #hardware/src/controls/motors.cpp
  #hardware/src/controls/weight.cpp
)

target_include_directories(expirationTracker PRIVATE ${OpenCV_INCLUDE_DIRS})

target_link_libraries(expirationTracker
  glog::glog
  ${SDL2_LIBRARIES} SDL2_image
  SDL2_ttf
  ${OpenCV_LIBS}
  sqlite3
)
