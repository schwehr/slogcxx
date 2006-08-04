/*!
  \file
  \brief The simplest possible example of logging without generating warnings.
 */
#include <slogcxx.h>

int
main(int argc, char *argv[]) {
  Slog log;
  log << "argc " << argc << endl;
  log << "argv[0] " << argv[0] << endl;
  return (EXIT_SUCCESS);
}
