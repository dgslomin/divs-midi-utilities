
cmake_minimum_required(VERSION 3.16.0)

project(Seqer VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 COMPONENTS Widgets REQUIRED)

add_executable(Seqer
	main.cpp
	window.cpp
	../../../../../midifile/midifile.c
	resources.qrc
)

target_link_libraries(Seqer PRIVATE Qt6::Widgets)

