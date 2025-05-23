#include <cstdlib>
#ifdef _WIN32
#  include <process.h>
#else
#  include <unistd.h>
#endif

import std;
using namespace std;
namespace fs = filesystem;

void
Launch(span< char * > const &args)
{
  char *const cmd = *args.begin();

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
PrintHelp(fs::path const &cmd)
{
  cout << format(
    R"(Help for {0}
Usage: {0} <command> [args...]

Example:
  {0} vkgears
)",
    cmd.filename().c_str());
}

int
main(int argc, char *argv[])
{
  span< char * > args{argv, static_cast< size_t >(argc)};

  if (args.size() <= 1) {
    PrintHelp(*args.begin());
    exit(EXIT_FAILURE);
  }

  Launch(args.subspan(1));
}
