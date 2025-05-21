set(VK_LAYER_NAME VK_LAYER_${PROJECT_NAME})
string(TOUPPER ${PROJECT_NAME} VK_LAYER_TOGGLE)
set(VK_LAYER_DESCRIPTION "${PROJECT_NAME} Vulkan layer")

set(VKLAYER_MANIFEST_LOCATION /usr/share/vulkan/implicit_layer.d)

DLL(${PROJECT_NAME} main.Loader.c++)
target_sources(${PROJECT_NAME}
PUBLIC
    FILE_SET module_files
    TYPE CXX_MODULES
    FILES
        Utils.c++
)

find_package(Vulkan REQUIRED)
target_link_libraries(${PROJECT_NAME} Vulkan::Vulkan)

OutputName(${PROJECT_NAME} VK_LAYER_LIB)
file(RELATIVE_PATH RELATIVE_SOURCE_PATH ${CMAKE_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR})

set(VK_LAYER_LIB ${CMAKE_BINARY_DIR}/${RELATIVE_SOURCE_PATH}/${VK_LAYER_LIB})

Configure(Manifest.json ../${VKLAYER_MANIFEST_LOCATION}/${PROJECT_NAME}.json)
