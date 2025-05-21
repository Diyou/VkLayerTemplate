module;

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
  copy(view.begin(), view.end(), buffer.begin());
  return buffer;
}

template< auto T >
constexpr auto
GetFunctionView()
{
  string_view needle    = Compiler::GCC ? "[with auto T = " : "[T = &";
  string_view signature = source_location::current().function_name();
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
compare(string_view const left, char const *right)
{
  for (auto const *i = left.begin(); i != left.end(); ++i) {
    if (*i != right[distance(left.begin(), i)]) {
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
    !name.empty() && compare(name, buffer.data()),
    "Function name extraction failed");

  return buffer;
}
