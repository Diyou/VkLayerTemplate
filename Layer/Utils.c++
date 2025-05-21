module;
#ifdef _MSC_VER
#  define CURRENT_FUNCTION_SIGNATURE __FUNCSIG__
#else
#  define CURRENT_FUNCTION_SIGNATURE __PRETTY_FUNCTION__
#endif
export module Utils;

import std;
using namespace std;

template< size_t N >
constexpr array< char, N + 1 >
ToArray(string_view view)
{
  array< char, N + 1 > buffer{};
  for (size_t i = 0; i < N && i < view.size(); ++i) {
    buffer[i] = view[i];
  }
  return buffer;
}

template< auto T >
constexpr auto
GetFunctionView()
{
#if defined(__clang__) || defined(_MSC_VER)
  string_view needle = "&";
#else
  string_view needle = "with auto T = ";
#endif
  string_view signature = CURRENT_FUNCTION_SIGNATURE;
  size_t      leftpos   = signature.rfind(needle);
  size_t      offset    = leftpos + needle.size();
  size_t      rightpos  = signature.find_first_of(";]>", offset);
  if (leftpos == string_view::npos || rightpos == string_view::npos) {
    return string_view{};
  }
  string_view full_name        = signature.substr(offset, rightpos - offset);

  string_view namespace_needle = "::";
  size_t      ns_pos           = full_name.rfind(namespace_needle);

  return ns_pos == string_view::npos
         ? full_name
         : full_name.substr(ns_pos + namespace_needle.size());
}

constexpr bool
compare(char const *left, char const *right, size_t const count)
{
  if (left == nullptr || right == nullptr) {
    return false;
  }
  for (size_t i = 0; i < count; i++) {
    if (left[i] != right[i]) {
      return false;
    }
  }
  return true;
}

export template< auto Function >
constexpr auto
GetFunctionName()
{
  constexpr auto name   = GetFunctionView< Function >();
  constexpr auto buffer = ToArray< name.size() >(name);

  static_assert(
    !name.empty() && compare(name.data(), buffer.data(), name.size()),
    "Function name extraction failed");

  return buffer;
}
