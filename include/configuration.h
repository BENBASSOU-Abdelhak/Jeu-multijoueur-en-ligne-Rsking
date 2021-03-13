#ifndef INCLUDE_CONFIGURATION_H
#define INCLUDE_CONFIGURATION_H

// Configuration file overwritten by CMake

// program version
#define VERSION_MAJOR 0
#define VERSION_MINOR 1

#define PROG_NAME "Risking"
#define mk_str(s) #s
#define PROG_FULLNAME PROG_NAME mk_str(VERSION_MAJOR) "." mk_str(VERSION_MINOR)

#endif
