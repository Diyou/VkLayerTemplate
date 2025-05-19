#include <cstdlib>
#include <unistd.h>

import std;
using namespace std;

void
Launch(span< char * > const &args)
{
  execvp(args.at(0), args.data());
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
    return EXIT_SUCCESS;
  }

  PrintHelp(cmd);
}
