set(VK_LAYER_NAME VK_LAYER_${PROJECT_NAME})
string(TOUPPER ${PROJECT_NAME} VK_LAYER_TOGGLE)
set(VK_LAYER_DESCRIPTION "${PROJECT_NAME} Vulkan layer")

set(VKLAYER_MANIFEST_LOCATION /usr/share/vulkan/implicit_layer.d)

target_sources(${PROJECT_NAME}
PRIVATE 
    Layer.c++
)

find_package(Vulkan REQUIRED)

target_link_libraries(${PROJECT_NAME}
PRIVATE
    Vulkan::Vulkan
)

OutputName(${PROJECT_NAME} VK_LAYER_LIB)
set(VK_LAYER_LIB ${CMAKE_BINARY_DIR}/${VK_LAYER_LIB})

Configure(VkLayer.json ../${VKLAYER_MANIFEST_LOCATION}/${PROJECT_NAME}.json)

add_subdirectory(Launcher)
