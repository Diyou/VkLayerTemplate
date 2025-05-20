#include <cassert>
#include <vulkan/vk_layer.h>

import std;

VKAPI_ATTR PFN_vkGetInstanceProcAddr VKAPI_CALL GetInstanceProcAddr = nullptr;
VKAPI_ATTR PFN_vkGetDeviceProcAddr VKAPI_CALL   GetDeviceProcAddr   = nullptr;

namespace Layer {
using namespace std;

struct VulkanFunctions
{
  static constexpr string_view vkCreateInstance = "vkCreateInstance";
  static constexpr string_view vkCreateDevice   = "vkCreateDevice";
};

enum class LoaderCreateInfo : uint8_t
{
  INSTANCE = VK_STRUCTURE_TYPE_LOADER_INSTANCE_CREATE_INFO,
  DEVICE   = VK_STRUCTURE_TYPE_LOADER_DEVICE_CREATE_INFO
};

bool
operator==(VkStructureType left, LoaderCreateInfo right)
{
  return left == VkStructureType(right);
}

bool
operator==(LoaderCreateInfo left, VkStructureType right)
{
  return operator==(right, left);
}

template< typename T >
concept LayerCreateInfo = same_as< T, VkLayerInstanceCreateInfo >
                       || same_as< T, VkLayerDeviceCreateInfo >;

template< LayerCreateInfo Info >
Info const *
FindLayerLink(Info const *info, LoaderCreateInfo const loader)
{
  using const_Info = Info const *;
  auto *next       = const_Info(info->pNext);
  while (next != nullptr) {
    if (next->sType == loader && next->function == VK_LAYER_LINK_INFO) {
      return next;
    }
    next = const_Info(next->pNext);
  }
  return nullptr;
}

template< typename T >
concept CreateInfo =
  same_as< T, VkInstanceCreateInfo > || same_as< T, VkDeviceCreateInfo >;

template< CreateInfo T >
VkResult
Initialize(T const *info)
{
  if constexpr (is_same_v< T, VkInstanceCreateInfo >) {
    if (GetInstanceProcAddr != nullptr) [[likely]] {
      return VK_SUCCESS;
    }
    auto const *loader = FindLayerLink(
      (VkLayerInstanceCreateInfo *)info, LoaderCreateInfo::INSTANCE);

    if (loader == nullptr) [[unlikely]] {
      return VK_ERROR_INITIALIZATION_FAILED;
    }

    GetInstanceProcAddr = loader->u.pLayerInfo->pfnNextGetInstanceProcAddr;
  }
  else if constexpr (is_same_v< T, VkDeviceCreateInfo >) {
    if (GetDeviceProcAddr != nullptr) [[likely]] {
      return VK_SUCCESS;
    }
    auto const *loader =
      FindLayerLink((VkLayerDeviceCreateInfo *)info, LoaderCreateInfo::DEVICE);
    if (loader == nullptr) [[unlikely]] {
      return VK_ERROR_INITIALIZATION_FAILED;
    }

    GetDeviceProcAddr = loader->u.pLayerInfo->pfnNextGetDeviceProcAddr;
  }
  return VK_SUCCESS;
}

VKAPI_ATTR VkResult VKAPI_CALL
vkCreateInstance(
  VkInstanceCreateInfo const  *pCreateInfo,
  VkAllocationCallbacks const *pAllocator,
  VkInstance                  *pInstance)
{
  auto const result = Initialize(pCreateInfo);
  if (result != VK_SUCCESS) [[unlikely]] {
    return result;
  }

  static auto const next = PFN_vkCreateInstance(
    GetInstanceProcAddr(*pInstance, VulkanFunctions::vkCreateInstance.data()));

  return next(pCreateInfo, pAllocator, pInstance);
}

VKAPI_ATTR VkResult VKAPI_CALL
vkCreateDevice(
  VkPhysicalDevice             physicalDevice,
  VkDeviceCreateInfo const    *pCreateInfo,
  VkAllocationCallbacks const *pAllocator,
  VkDevice                    *pDevice)
{
  auto const result = Initialize(pCreateInfo);
  if (result != VK_SUCCESS) [[unlikely]] {
    return result;
  }

  static auto const next = PFN_vkCreateDevice(GetInstanceProcAddr(
    VK_NULL_HANDLE, VulkanFunctions::vkCreateDevice.data()));

  return next(physicalDevice, pCreateInfo, pAllocator, pDevice);
}

VKAPI_ATTR PFN_vkVoidFunction VKAPI_CALL
vkGetInstanceProcAddr(VkInstance pInstance, char const *pName)
{
  if (VulkanFunctions::vkCreateInstance == pName) {
    return PFN_vkVoidFunction(vkCreateInstance);
  }
  if (VulkanFunctions::vkCreateDevice == pName) {
    return PFN_vkVoidFunction(vkCreateDevice);
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
