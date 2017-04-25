/***************************************************************************
  qgsstacktrace - %{Cpp:License:ClassName}

 ---------------------
 begin                : 30.3.2017
 copyright            : (C) 2017 by Matthias Kuhn
 email                : matthias@opengis.ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsstacktrace.h"
#include <stdlib.h>
#include <execinfo.h>
#include <cxxabi.h>

#ifdef QGISDEBUG
#ifdef Q_OS_LINUX
QStringList QgsStacktrace::trace( unsigned int maxFrames )
{
  QStringList stack;

  // storage array for stack trace address data
  void *addrlist[maxFrames + 1];

  // retrieve current stack addresses
  int addrlen = backtrace( addrlist, sizeof( addrlist ) / sizeof( void * ) );

  if ( addrlen == 0 )
  {
    stack.append( QString() );
    return stack;
  }

  // resolve addresses into strings containing "filename(function+address)",
  // this array must be free()-ed
  char **symbollist = backtrace_symbols( addrlist, addrlen );

  // allocate string which will be filled with the demangled function name
  size_t funcnamesize = 256;
  char *funcname = ( char * )malloc( funcnamesize );

  // iterate over the returned symbol lines. skip the first, it is the
  // address of this function.
  for ( int i = 1; i < addrlen; i++ )
  {
    char *beginName = 0;
    char *beginOffset = 0;
    char *endOffset = 0;

    // find parentheses and +address offset surrounding the mangled name:
    // ./module(function+0x15c) [0x8048a6d]
    for ( char *p = symbollist[i]; *p; ++p )
    {
      if ( *p == '(' )
        beginName = p;
      else if ( *p == '+' )
        beginOffset = p;
      else if ( *p == ')' && beginOffset )
      {
        endOffset = p;
        break;
      }
    }

    if ( beginName && beginOffset && endOffset
         && beginName < beginOffset )
    {
      *beginName++ = '\0';
      *beginOffset++ = '\0';
      *endOffset = '\0';

      // mangled name is now in [begin_name, begin_offset) and caller
      // offset in [begin_offset, end_offset). now apply
      // __cxa_demangle():

      int status;
      char *ret = abi::__cxa_demangle( beginName,
                                       funcname, &funcnamesize, &status );
      if ( status == 0 )
      {
        funcname = ret; // use possibly realloc()-ed string
        stack.append( QString( "%1 : %2+%3" ).arg( symbollist[i], funcname, beginOffset ) );
      }
      else
      {
        // demangling failed. Output function name as a C function with
        // no arguments.
        stack.append( QString( "%1 : %2()+%3\n" ).arg( symbollist[i], beginName, beginOffset ) );
      }
    }
    else
    {
      // couldn't parse the line? print the whole line.
      stack.append( QString( "%1\n" ).arg( symbollist[i] ) );
    }
  }

  free( funcname );
  free( symbollist );
  return stack;
}
#else
QStringList QgsStacktrace::trace( unsigned int maxFrames )
{
  return QStringList() << QStringLiteral( "Stack trace is not implemnted for your platform." );
}
#endif // OS detection

#endif // QGISDEBUG
