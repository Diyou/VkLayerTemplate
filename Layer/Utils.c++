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

// Enable generic pointer types for format()
/*export template< typename T > struct formatter< T *, char >
{
  constexpr static char        default_presentation = 'x';
  constexpr static string_view supported            = "xX";
  char                         presentation         = default_presentation;

  constexpr auto
  parse(format_parse_context &ctx)
  {
    auto const *iter = ctx.begin();

    if (iter != ctx.end() && *iter != '}') {
      char const chr = *iter++;
      presentation   = supported.contains(chr) ? chr : default_presentation;
    }

    // Ignore any extra characters after the format specifier
    while (iter != ctx.end() && *iter != '}') {
      ++iter;
    }

    return iter;
  }

  auto
  format(T *ptr, format_context &ctx) const
  {
    auto value = reinterpret_cast< uintptr_t >(ptr);

    if (presentation == 'X') {
      return format_to(ctx.out(), "0x{:X}", value);
    }

    return format_to(ctx.out(), "0x{:x}", value);
  }
};*/

constexpr string_view empty_view;

constexpr string_view
strip_left(string_view view, string_view match)
{
  size_t pos = view.rfind(match);
  return pos == string_view::npos ? empty_view
                                  : view.substr(pos + match.size());
}

constexpr string_view
strip_first_of(string_view view, string_view match)
{
  size_t pos = view.find_first_of(match);
  return pos == string_view::npos ? empty_view : view.substr(0, pos);
}

template< auto T >
constexpr auto
GetFunctionView()
{
  string_view needle = Compiler::GCC ? "[with auto T = " : "[T = &";
  string_view signature =
    strip_left(source_location::current().function_name(), needle);
  string_view stripped_namespace = strip_left(signature, "::");
  signature = stripped_namespace.empty() ? signature : stripped_namespace;
  return strip_first_of(signature, ";]>");
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

template< size_t N >
constexpr auto
to_array(string_view view)
{
  array< char, N + 1 > buffer{};
  copy(view.begin(), view.end(), buffer.begin());
  return buffer;
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
