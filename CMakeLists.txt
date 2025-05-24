cmake_minimum_required(VERSION 4.0.0 FATAL_ERROR)

add_subdirectory(.cmake)

project(VkLayerTemplate)

add_subdirectory(Layer)
add_subdirectory(Launcher)
