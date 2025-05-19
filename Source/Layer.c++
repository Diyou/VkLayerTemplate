#include <vulkan/vk_layer.h>

import std;

namespace TEMPLATE {
using namespace std;

struct Instance;
unordered_map< VkInstance, Instance > InstanceMap;

struct Instance
{
  VkInstanceCreateInfo const pCreateInfo;
  PFN_vkGetInstanceProcAddr  GetInstanceProcAddr = nullptr;

  Instance(VkInstanceCreateInfo const *pCreateInfo)
  : pCreateInfo{*pCreateInfo}
  {
    for (auto *i = (VkLayerInstanceCreateInfo *)pCreateInfo->pNext;
         i != nullptr;
         i = (VkLayerInstanceCreateInfo *)i->pNext)
    {
      if (
        i->sType == VK_STRUCTURE_TYPE_LOADER_INSTANCE_CREATE_INFO
        && i->function == VK_LAYER_LINK_INFO)
      {
        GetInstanceProcAddr = i->u.pLayerInfo->pfnNextGetInstanceProcAddr;
        i->u.pLayerInfo     = i->u.pLayerInfo->pNext;
        break;
      }
    }
  }

  static decltype(InstanceMap.begin())
  Find(VkInstance pInstance)
  {
    return InstanceMap.find(pInstance);
  }
};

struct Device;
extern unordered_map< VkDevice, Device > DeviceMap;
extern mutex                             DeviceLock;

struct Device
{
  VkDeviceCreateInfo const  pCreateInfo;
  PFN_vkGetInstanceProcAddr GetInstanceProcAddr = nullptr;
  PFN_vkGetDeviceProcAddr   GetDeviceProcAddr   = nullptr;

  Device(VkDeviceCreateInfo const *pCreateInfo)
  : pCreateInfo{*pCreateInfo}
  {
    for (auto *i = (VkLayerDeviceCreateInfo *)pCreateInfo->pNext; i != nullptr;
         i       = (VkLayerDeviceCreateInfo *)i->pNext)
    {
      if (
        i->sType == VK_STRUCTURE_TYPE_LOADER_DEVICE_CREATE_INFO
        && i->function == VK_LAYER_LINK_INFO)
      {
        GetInstanceProcAddr = i->u.pLayerInfo->pfnNextGetInstanceProcAddr;
        GetDeviceProcAddr   = i->u.pLayerInfo->pfnNextGetDeviceProcAddr;
        i->u.pLayerInfo     = i->u.pLayerInfo->pNext;
        break;
      }
    }
  }

  static decltype(DeviceMap.begin())
  Find(VkDevice pDevice)
  {
    return DeviceMap.find(pDevice);
  }
};

VKAPI_ATTR VkResult VKAPI_CALL
vkCreateInstance(
  VkInstanceCreateInfo const  *pCreateInfo,
  VkAllocationCallbacks const *pAllocator,
  VkInstance                  *pInstance)
{
  auto const &[i, emplaced] = InstanceMap.emplace(*pInstance, pCreateInfo);
  auto const &[p, instance] = *i;

  if (instance.GetInstanceProcAddr == nullptr) {
    return VK_ERROR_INITIALIZATION_FAILED;
  }

  auto const next =
    (PFN_vkCreateInstance)instance.GetInstanceProcAddr(*pInstance, __func__);

  auto const result = next(pCreateInfo, pAllocator, pInstance);
  return result;
}

VKAPI_ATTR void VKAPI_CALL
vkDestroyInstance(VkInstance pInstance, VkAllocationCallbacks const *pAllocator)
{
  auto const iterator       = Instance::Find(pInstance);
  auto const &[p, instance] = *iterator;

  auto const next =
    (PFN_vkDestroyInstance)instance.GetInstanceProcAddr(pInstance, __func__);
  InstanceMap.erase(iterator);

  next(pInstance, pAllocator);
}

VKAPI_ATTR VkResult VKAPI_CALL
vkCreateDevice(
  VkPhysicalDevice             physicalDevice,
  VkDeviceCreateInfo const    *pCreateInfo,
  VkAllocationCallbacks const *pAllocator,
  VkDevice                    *pDevice)
{
  lock_guard< mutex > lock(DeviceLock);

  auto const &[i, emplaced] = DeviceMap.emplace(*pDevice, pCreateInfo);
  auto const &[p, device]   = *i;

  if (device.GetDeviceProcAddr == nullptr) {
    return VK_ERROR_INITIALIZATION_FAILED;
  }

  auto const next =
    (PFN_vkCreateDevice)device.GetDeviceProcAddr(*pDevice, __func__);

  auto const result = next(physicalDevice, pCreateInfo, pAllocator, pDevice);
  return result;
}

VKAPI_ATTR void VKAPI_CALL
vkDestroyDevice(VkDevice pDevice, VkAllocationCallbacks const *pAllocator)
{
  lock_guard< mutex > lock(DeviceLock);

  auto const          iterator = Device::Find(pDevice);
  auto const &[p, device]      = *iterator;

  auto const next =
    (PFN_vkDestroyDevice)device.GetDeviceProcAddr(pDevice, __func__);
  DeviceMap.erase(iterator);

  next(pDevice, pAllocator);
}

VKAPI_ATTR PFN_vkVoidFunction VKAPI_CALL
vkGetInstanceProcAddr(VkInstance pInstance, string const &pName)
{
  if (pName == "vkCreateInstance") {
    return (PFN_vkVoidFunction)vkCreateInstance;
  }
  if (pName == "vkDestroyInstance") {
    return (PFN_vkVoidFunction)vkDestroyInstance;
  }

  auto const iter = Instance::Find(pInstance);
  if (iter != InstanceMap.end()) {
    auto const &[p, instance] = *iter;
    return instance.GetInstanceProcAddr(pInstance, pName.c_str());
  }

  return nullptr;
}

VKAPI_ATTR PFN_vkVoidFunction VKAPI_CALL
vkGetDeviceProcAddr(VkDevice pDevice, string const &pName)
{
  lock_guard< mutex > lock(DeviceLock);

  if (pName == "vkCreateDevice") {
    return (PFN_vkVoidFunction)vkCreateDevice;
  }
  if (pName == "vkDestroyDevice") {
    return (PFN_vkVoidFunction)vkDestroyDevice;
  }

  auto const iter = Device::Find(pDevice);
  if (iter != DeviceMap.end()) {
    auto const &[p, device] = *iter;
    return device.GetDeviceProcAddr(pDevice, pName.c_str());
  }

  return nullptr;
}
}

extern "C"
{
  VKAPI_ATTR PFN_vkVoidFunction VKAPI_CALL
  VKLAYER_TEMPLATE_vkGetInstanceProcAddr(VkInstance instance, char const *pName)
  {
    return TEMPLATE::vkGetInstanceProcAddr(instance, pName);
  }

  VKAPI_ATTR PFN_vkVoidFunction VKAPI_CALL
  VKLAYER_TEMPLATE_vkGetDeviceProcAddr(VkDevice device, char const *pName)
  {
    return TEMPLATE::vkGetDeviceProcAddr(device, pName);
  }
}
