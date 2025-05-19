#include <cassert>
#include <vulkan/vk_layer.h>

import std;

PFN_vkGetInstanceProcAddr GetInstanceProcAddr = nullptr;
PFN_vkGetDeviceProcAddr   GetDeviceProcAddr   = nullptr;

namespace Layer {
using namespace std;

VKAPI_ATTR VkResult VKAPI_CALL
vkCreateInstance(
  VkInstanceCreateInfo const  *pCreateInfo,
  VkAllocationCallbacks const *pAllocator,
  VkInstance                  *pInstance)
{
  cout << "---Creating Instance---\n";
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
  cout << "---Creating Device---\n";
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

  static auto const next = reinterpret_cast< PFN_vkCreateDevice >(
    GetInstanceProcAddr(VK_NULL_HANDLE, __func__));
  return next(physicalDevice, pCreateInfo, pAllocator, pDevice);
}

inline PFN_vkVoidFunction
Overlay(string_view pName)
{
  if ("vkCreateInstance" == pName) {
    return reinterpret_cast< PFN_vkVoidFunction >(vkCreateInstance);
  }
  if ("vkCreateDevice" == pName) {
    return reinterpret_cast< PFN_vkVoidFunction >(vkCreateDevice);
  }
  return nullptr;
}

VKAPI_ATTR PFN_vkVoidFunction VKAPI_CALL
vkGetInstanceProcAddr(VkInstance pInstance, char const *pName)
{
  auto const overlayed = Overlay(pName);
  if (overlayed != nullptr) {
    return overlayed;
  }

  auto const next = GetInstanceProcAddr(pInstance, pName);
  cout << format("[GIPA]:\t{}(0x{:x})\n", pName, uintptr_t(next));
  return next;
}

VKAPI_ATTR PFN_vkVoidFunction VKAPI_CALL
vkGetDeviceProcAddr(VkDevice pDevice, char const *pName)
{
  auto const overlayed = Overlay(pName);
  if (overlayed != nullptr) {
    return overlayed;
  }

  auto const next = GetDeviceProcAddr(pDevice, pName);
  cout << format("[GDPA]:\t{}(0x{:x})\n", pName, uintptr_t(next));
  return next;
}

}

extern "C"
{
  VKAPI_ATTR VkResult VKAPI_CALL
  vkNegotiateLoaderLayerInterfaceVersion(
    VkNegotiateLayerInterface *pVersionStruct)
  {
    pVersionStruct->pfnGetInstanceProcAddr = Layer::vkGetInstanceProcAddr;
    pVersionStruct->pfnGetDeviceProcAddr   = Layer::vkGetDeviceProcAddr;

    return VK_SUCCESS;
  }
}
