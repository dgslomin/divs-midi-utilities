
cmake_minimum_required(VERSION 3.1.0)

project(Seqer VERSION 1.0.0 LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

if(CMAKE_VERSION VERSION_LESS "3.7.0")
	set(CMAKE_INCLUDE_CURRENT_DIR ON)
endif()

find_package(Qt5 COMPONENTS Widgets REQUIRED)

add_executable(Seqer colors.cpp inspector-sidebar.cpp lane.cpp main.cpp note-lane.cpp sequence.cpp time-ruler.cpp window.cpp ../../midifile/midifile.c resources.qrc)

target_include_directories(Seqer PRIVATE ../../midifile)

target_link_libraries(Seqer PRIVATE Qt5::Widgets)

