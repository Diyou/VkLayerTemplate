module;
#ifdef _MSC_VER
#  define CURRENT_FUNCTION_SIGNATURE __FUNCSIG__
#else
#  define CURRENT_FUNCTION_SIGNATURE __PRETTY_FUNCTION__
#endif
export module Utils;

import std;
using namespace std;

export struct Compiler
{
#if defined(__GNUC__) && !defined(__clang__)
  constexpr static bool GCC   = true;
  constexpr static bool CLANG = false;
#else
  constexpr static bool GCC   = false;
  constexpr static bool CLANG = true;
#endif
#if defined(_MSC_VER)
  constexpr static bool MSVC = true;
#else
  constexpr static bool MSVC = false;
#endif
};

constexpr string_view
strip_namespace(string_view name)
{
  string_view needle = "::";
  size_t      pos    = name.rfind(needle);
  return pos == string_view::npos ? name : name.substr(pos + needle.size());
}

template< size_t N >
constexpr array< char, N + 1 >
to_array(string_view view)
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
  string_view needle    = Compiler::GCC ? "with auto T = " : "&";
  string_view signature = CURRENT_FUNCTION_SIGNATURE;
  size_t      pos       = signature.rfind(needle);
  if (pos == string_view::npos) {
    return string_view{};
  }
  signature = signature.substr(pos + needle.size());
  pos       = signature.find_first_of(";]>");
  if (pos == string_view::npos) {
    return string_view{};
  }
  return strip_namespace(signature.substr(0, pos));
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
  constexpr auto buffer = to_array< name.size() >(name);

  static_assert(
    !name.empty() && compare(name.data(), buffer.data(), name.size()),
    "Function name extraction failed");

  return buffer;
}
