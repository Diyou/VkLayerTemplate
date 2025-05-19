cmake_minimum_required(VERSION 4.0.0 FATAL_ERROR)

include(.cmake/Setup.cmake)

project(VkLayerTemplate)

add_subdirectory(Layer)
add_subdirectory(Launcher)
