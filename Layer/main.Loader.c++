#include <vulkan/vk_layer.h>

import std;
import Utils;

using namespace std;

VKAPI_ATTR PFN_vkGetInstanceProcAddr VKAPI_CALL GetInstanceProcAddr = nullptr;
VKAPI_ATTR PFN_vkGetDeviceProcAddr VKAPI_CALL   GetDeviceProcAddr   = nullptr;

template< auto Function >
constexpr auto
GetNextInstanceProcAddr(VkInstance instance)
{
  using VkFunction    = decltype(Function);
  constexpr auto name = GetFunctionName< Function >();

  return VkFunction(GetInstanceProcAddr(instance, name.data()));
}

template< auto Function >
constexpr auto
GetNextDeviceProcAddr(VkDevice device)
{
  using VkFunction    = decltype(Function);
  constexpr auto name = GetFunctionName< Function >();

  return VkFunction(GetDeviceProcAddr(device, name.data()));
}

namespace Layer {

struct InstanceFunctions
{
  static constexpr string_view vkCreateInstance = "vkCreateInstance";
  static constexpr string_view vkCreateDevice   = "vkCreateDevice";
};

struct DeviceFunctions
{};

template< typename T, typename... Ts >
concept is_one_of = (same_as< T, Ts > || ...);

template< typename T >
  requires is_one_of< T, VkInstanceCreateInfo, VkDeviceCreateInfo >
auto const *
FindLayerLink(T const *info)
{
  using return_t = conditional_t<
    is_same_v< T, VkInstanceCreateInfo >,
    VkLayerInstanceCreateInfo,
    VkLayerDeviceCreateInfo > const *;

  auto const loader = is_same_v< T, VkInstanceCreateInfo >
                      ? VK_STRUCTURE_TYPE_LOADER_INSTANCE_CREATE_INFO
                      : VK_STRUCTURE_TYPE_LOADER_DEVICE_CREATE_INFO;

  for (auto *i = return_t(info->pNext); i != nullptr;) [[likely]] {
    if (i->sType == loader && i->function == VK_LAYER_LINK_INFO) [[unlikely]] {
      return i;
    }
    i = return_t(i->pNext);
  }
  return return_t(nullptr);
}

VKAPI_ATTR VkResult VKAPI_CALL
vkCreateInstance(
  VkInstanceCreateInfo const  *pCreateInfo,
  VkAllocationCallbacks const *pAllocator,
  VkInstance                  *pInstance)
{
  if (GetInstanceProcAddr == nullptr) [[unlikely]] {
    auto const *link = FindLayerLink(pCreateInfo);

    if (link == nullptr || link->u.pLayerInfo == nullptr) [[unlikely]] {
      return VK_ERROR_INITIALIZATION_FAILED;
    }

    GetInstanceProcAddr = link->u.pLayerInfo->pfnNextGetInstanceProcAddr;
  }

  static auto const next =
    GetNextInstanceProcAddr< vkCreateInstance >(*pInstance);

  return next(pCreateInfo, pAllocator, pInstance);
}

VKAPI_ATTR VkResult VKAPI_CALL
vkCreateDevice(
  VkPhysicalDevice             physicalDevice,
  VkDeviceCreateInfo const    *pCreateInfo,
  VkAllocationCallbacks const *pAllocator,
  VkDevice                    *pDevice)
{
  if (GetDeviceProcAddr == nullptr) [[unlikely]] {
    auto const *link = FindLayerLink(pCreateInfo);

    if (link == nullptr || link->u.pLayerInfo == nullptr) [[unlikely]] {
      return VK_ERROR_INITIALIZATION_FAILED;
    }

    GetDeviceProcAddr = link->u.pLayerInfo->pfnNextGetDeviceProcAddr;
  }

  static auto const next =
    GetNextInstanceProcAddr< vkCreateDevice >(VK_NULL_HANDLE);

  return next(physicalDevice, pCreateInfo, pAllocator, pDevice);
}

#ifndef NDEBUG
#  define LOG(next)                                                         \
    cout << format("[{}]:\t{}(0x{:x})\n", __func__, pName, uintptr_t(next))
#else
#  define LOG(next)
#endif

VKAPI_ATTR PFN_vkVoidFunction VKAPI_CALL
vkGetInstanceProcAddr(VkInstance pInstance, char const *pName)
{
  if (InstanceFunctions::vkCreateInstance == pName) {
    return PFN_vkVoidFunction(vkCreateInstance);
  }
  if (InstanceFunctions::vkCreateDevice == pName) {
    return PFN_vkVoidFunction(vkCreateDevice);
  }

  auto const next = GetInstanceProcAddr(pInstance, pName);
  LOG(next);
  return next;
}

VKAPI_ATTR PFN_vkVoidFunction VKAPI_CALL
vkGetDeviceProcAddr(VkDevice pDevice, char const *pName)
{
  auto const next = GetDeviceProcAddr(pDevice, pName);
  LOG(next);
  return next;
}
}

extern "C" VKAPI_ATTR VkResult VKAPI_CALL
vkNegotiateLoaderLayerInterfaceVersion(
  VkNegotiateLayerInterface *pVersionStruct)
{
  if (pVersionStruct == nullptr) [[unlikely]] {
    return VK_ERROR_INITIALIZATION_FAILED;
  }

  if (pVersionStruct->loaderLayerInterfaceVersion > 3) [[unlikely]] {
    cout << format(
      "High Loader LayerInterfaceVersion: {}\n",
      pVersionStruct->loaderLayerInterfaceVersion);
  }
  pVersionStruct->loaderLayerInterfaceVersion = 2;

  pVersionStruct->pfnGetInstanceProcAddr      = Layer::vkGetInstanceProcAddr;
  pVersionStruct->pfnGetDeviceProcAddr        = Layer::vkGetDeviceProcAddr;
  return VK_SUCCESS;
}
