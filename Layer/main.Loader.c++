#include <cassert>
#include <vulkan/vk_layer.h>

import std;

VKAPI_ATTR PFN_vkGetInstanceProcAddr VKAPI_CALL GetInstanceProcAddr = nullptr;
VKAPI_ATTR PFN_vkGetDeviceProcAddr VKAPI_CALL   GetDeviceProcAddr   = nullptr;

namespace Layer {
using namespace std;

struct VulkanFunctions
{
  static inline string_view vkCreateInstance = "vkCreateInstance";
  static inline string_view vkCreateDevice   = "vkCreateDevice";
};

VKAPI_ATTR VkResult VKAPI_CALL
vkCreateInstance(
  VkInstanceCreateInfo const  *pCreateInfo,
  VkAllocationCallbacks const *pAllocator,
  VkInstance                  *pInstance)
{
  if (GetInstanceProcAddr != nullptr) {
    goto skiploop;
  }
  for (auto *i = (VkLayerInstanceCreateInfo *)pCreateInfo->pNext; i != nullptr;
       i       = (VkLayerInstanceCreateInfo *)i->pNext)
  {
    if (
      i->sType == VK_STRUCTURE_TYPE_LOADER_INSTANCE_CREATE_INFO
      && i->function == VK_LAYER_LINK_INFO)
    {
      GetInstanceProcAddr = i->u.pLayerInfo->pfnNextGetInstanceProcAddr;
    }
  }
  assert(GetInstanceProcAddr != nullptr);
skiploop:

  static auto const next = reinterpret_cast< PFN_vkCreateInstance >(
    GetInstanceProcAddr(*pInstance, __func__));

  return next(pCreateInfo, pAllocator, pInstance);
}

VKAPI_ATTR VkResult VKAPI_CALL
vkCreateDevice(
  VkPhysicalDevice             physicalDevice,
  VkDeviceCreateInfo const    *pCreateInfo,
  VkAllocationCallbacks const *pAllocator,
  VkDevice                    *pDevice)
{
  if (GetDeviceProcAddr != nullptr) {
    goto skiploop;
  }
  for (auto *i = (VkLayerDeviceCreateInfo *)pCreateInfo->pNext; i != nullptr;
       i       = (VkLayerDeviceCreateInfo *)i->pNext)
  {
    if (
      i->sType == VK_STRUCTURE_TYPE_LOADER_DEVICE_CREATE_INFO
      && i->function == VK_LAYER_LINK_INFO)
    {
      GetDeviceProcAddr = i->u.pLayerInfo->pfnNextGetDeviceProcAddr;
    }
  }
  assert(GetDeviceProcAddr != nullptr);
skiploop:

  static auto const next = reinterpret_cast< PFN_vkCreateDevice >(
    GetInstanceProcAddr(VK_NULL_HANDLE, __func__));
  return next(physicalDevice, pCreateInfo, pAllocator, pDevice);
}

VKAPI_ATTR PFN_vkVoidFunction VKAPI_CALL
vkGetInstanceProcAddr(VkInstance pInstance, char const *pName)
{
  if (VulkanFunctions::vkCreateInstance == pName) {
    return reinterpret_cast< PFN_vkVoidFunction >(vkCreateInstance);
  }
  if (VulkanFunctions::vkCreateDevice == pName) {
    return reinterpret_cast< PFN_vkVoidFunction >(vkCreateDevice);
  }

  auto const next = GetInstanceProcAddr(pInstance, pName);
  cout << format("[{}]:\t{}(0x{:x})\n", __func__, pName, uintptr_t(next));
  return next;
}

VKAPI_ATTR PFN_vkVoidFunction VKAPI_CALL
vkGetDeviceProcAddr(VkDevice pDevice, char const *pName)
{
  auto const next = GetDeviceProcAddr(pDevice, pName);
  cout << format("[{}]:\t{}(0x{:x})\n", __func__, pName, uintptr_t(next));
  return next;
}
}

extern "C" VKAPI_ATTR VkResult VKAPI_CALL
vkNegotiateLoaderLayerInterfaceVersion(
  VkNegotiateLayerInterface *pVersionStruct)
{
  pVersionStruct->pfnGetInstanceProcAddr = Layer::vkGetInstanceProcAddr;
  pVersionStruct->pfnGetDeviceProcAddr   = Layer::vkGetDeviceProcAddr;

  return VK_SUCCESS;
}
