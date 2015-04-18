cmake_minimum_required(VERSION 2.8)

set(CMAKE_TOOLCHAIN_FILE $ENV{ARDUINO_CMAKE}/cmake/ArduinoToolchain.cmake)
set(ARDUINO_1_5 true)

project(cbmkeyboard C CXX)

link_directories(${ARDUINO_SDK_PATH}/hardware/arduino/avr/libraries)
link_directories(${ARDUINO_SDK_PATH}/libraries)
link_directories($ENV{ARDUINO_LIBRARIES})

generate_arduino_firmware(
	cbmkeyboard
	SRCS cbmkeyboard.cpp
	BOARD uno
	PROGRAMMER dragon)
