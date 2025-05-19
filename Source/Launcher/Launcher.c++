#include <cstdlib>
#ifdef _WIN32
#  include <process.h>
#else
#  include <unistd.h>
#endif

import std;
using namespace std;

void
Launch(span< char * > const &args)
{
  auto *const cmd = args.at(0);

#ifdef _WIN32
  auto const result = _spawnvp(_P_OVERLAY, cmd, args.data());
  if (result != -1) {
    return;
  }
#else
  auto const result = execvp(cmd, args.data());
#endif

  cerr << format("Failed to launch {}\n", cmd);
  exit(EXIT_FAILURE);
}

void
PrintHelp(string_view const &cmd)
{
  cout << format(
    R"(Help for {0}
Usage: {0} <command> [args...]

Example:
  {0} vkgears
)",
    cmd);
}

int
main(int argc, char *argv[])
{
  string_view    cmd    = argv[0];
  size_t const   offset = 1;
  span< char * > args{argv + offset, argc - offset};

  if (!args.empty()) {
    Launch(args);
  }

  PrintHelp(cmd);
}
