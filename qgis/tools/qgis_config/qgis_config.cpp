/***************************************************************************
  qgis_config.cpp
  Display information about the installed version of QGIS.
  This information can be used to configure applications that use
  the QGIS library.

  begin                : 2004-03-22
  copyright            : (C) 2004 by Gary Sherman
  email                : sherman at mrcc.com

***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
/*  $Id$ */

#include <iostream>
#include <getopt.h>

/**
 * Show usage of qgis_config when user supplies --help or -h as an argument
 */
void usage()
{
  std::cout << 
    "qgis_config provides information about the installed version of QGIS.\n\n" <<
    "Usage:\n " <<
    "  qgis_config OPTION...\n\n" <<
    "Options:\n" <<
    "   --bindir              print destination of executable(s)\n" <<
    "   --cflags              print the compiler flags that are necessary to\n" <<
    "                         compile a plug-inshow location of C++ header files\n" << 
    "   --libs                print the linker flags that are necessary to link a\n" <<
    "                         plug-in\n" <<
    "   --plugindir           print the path where the plugins are installed\n" <<
    "   --help                show this help, then exit\n"
    << std::endl; 
}

/**
 * Main function to display various information about the configured
 * and installed version of QGIS. This information is based on parameters
 * supplied when configuring QGIS prior to building.
 */
int main(int argc, char **argv)
{
  int optionChar;
  // options structure
  static struct option long_options[] =
  {
    /* These options set a flag. */
    {"help", no_argument, 0, 'h'},
    /* These options don't set a flag.
     *  We distinguish them by their indices. */
    {"prefix", no_argument, 0, 'p'},
    {"bindir",     no_argument, 0, 'b'},
    {"cflags",  no_argument, 0, 'c'},
    {"libs",  no_argument, 0, 'l'},
    {"plugindir",  no_argument, 0, 'w'},
    {0, 0, 0, 0}
  };
  // If no argument is given, show hint
  if(argc == 1)
  {
    std::cout << "qgis_config: argument required" << std::endl; 
    std::cout << "Try \"qgis_config --help\" for more information." << std::endl; 
  }else
  {
    // One or more arguments supplied
    while (1)
    {
      /* getopt_long stores the option index here. */
      int option_index = 0;

      optionChar = getopt_long (argc, argv, "pbclw",
          long_options, &option_index);

      /* Detect the end of the options. */
      if (optionChar == -1)
        break;

      switch (optionChar)
      {
        case 'p':
          std::cout << PREFIX << std::endl; 
          break;

        case 'b':
          std::cout << BIN_DIR << std::endl; 
          break;

        case 'c':
          std::cout << "-I" << INCLUDE_DIR << " ";
					std::cout << "-I" << INCLUDE_DIR << "/qgis" << std::endl; 
          break;

        case 'l':
          std::cout << "-L" << LIB_DIR << " ";
					std::cout << " -lqgis" << std::endl; 
          break;

        case 'w':
          std::cout << PLUGIN_DIR << std::endl; 
          break;

        case 'h':
          usage();
          return 1;   
          break;

        default:
          std::cout << "Try \"qgis_config --help\" for more information." 
            << std::endl; 
          return 1;  
      }
    }
  }
  // return success
  return 0;
}
