cmake_minimum_required(VERSION 4.0.0 FATAL_ERROR)

include(.cmake/Setup.cmake)

project(VkLayerTemplate)

MODULE(${PROJECT_NAME})

add_subdirectory(Source)
