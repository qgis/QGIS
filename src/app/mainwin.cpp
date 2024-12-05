/***************************************************************************
    mainwin.cpp
    ---------------------
    begin                : February 2017
    copyright            : (C) 2017 by Juergen E. Fischer
    email                : jef at norbit dot de
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include <windows.h>
#include <io.h>

#include <sstream>
#include <iostream>
#include <fstream>
#include <list>
#include <memory>

void showError( std::string message, std::string title )
{
  std::string newmessage = "Oops, looks like an error loading QGIS \n\n Details: \n\n" + message;
  MessageBox(
    NULL,
    newmessage.c_str(),
    title.c_str(),
    MB_ICONERROR | MB_OK
  );
  std::cerr << message << std::endl;
}

std::string moduleExeBaseName( void )
{
  DWORD l = MAX_PATH;
  std::unique_ptr<char> filepath;
  for ( ;; )
  {
    filepath.reset( new char[l] );
    if ( GetModuleFileName( nullptr, filepath.get(), l ) < l )
      break;

    l += MAX_PATH;
  }

  std::string basename( filepath.get() );
  return basename;
}

int CALLBACK WinMain( HINSTANCE /*hInstance*/, HINSTANCE /*hPrevInstance*/, LPSTR /*lpCmdLine*/, int /*nCmdShow*/ )
{
  std::string exename( moduleExeBaseName() );
  std::string basename( exename.substr( 0, exename.size() - 4 ) );

  if ( getenv( "OSGEO4W_ROOT" ) && __argc == 2 && strcmp( __argv[1], "--postinstall" ) == 0 )
  {
    std::string envfile( basename + ".env" );

    // write or update environment file
    if ( _access( envfile.c_str(), 0 ) < 0 || _access( envfile.c_str(), 2 ) == 0 )
    {
      std::list<std::string> vars;

      try
      {
        std::ifstream varfile;
        varfile.open( basename + ".vars" );

        std::string var;
        while ( std::getline( varfile, var ) )
        {
          vars.push_back( var );
        }

        varfile.close();
      }
      catch ( std::ifstream::failure &e )
      {
        std::string message = "Could not read environment variable list " + basename + ".vars" + " [" + e.what() + "]";
        showError( message, "Error loading QGIS" );
        return EXIT_FAILURE;
      }

      try
      {
        std::ofstream file;
        file.open( envfile, std::ifstream::out );

        for ( std::list<std::string>::const_iterator it = vars.begin(); it != vars.end(); ++it )
        {
          if ( getenv( it->c_str() ) )
            file << *it << "=" << getenv( it->c_str() ) << std::endl;
        }
      }
      catch ( std::ifstream::failure &e )
      {
        std::string message = "Could not write environment file " + basename + ".env" + " [" + e.what() + "]";
        showError( message, "Error loading QGIS" );
        return EXIT_FAILURE;
      }
    }

    return EXIT_SUCCESS;
  }

  try
  {
    std::ifstream file;
    file.open( basename + ".env" );

    std::string var;
    while ( std::getline( file, var ) )
    {
      if ( _putenv( var.c_str() ) < 0 )
      {
        std::string message = "Could not set environment variable:" + var;
        showError( message, "Error loading QGIS" );
        return EXIT_FAILURE;
      }
    }
  }
  catch ( std::ifstream::failure &e )
  {
    std::string message = "Could not read environment file " + basename + ".env" + " [" + e.what() + "]";
    showError( message, "Error loading QGIS" );
    return EXIT_FAILURE;
  }

#ifndef _MSC_VER // MinGW
#pragma GCC diagnostic ignored "-Wcast-function-type"
#endif
  HINSTANCE hKernelDLL = LoadLibrary( "kernel32.dll" );
  BOOL ( *SetDefaultDllDirectories )( DWORD ) = hKernelDLL ? reinterpret_cast<BOOL ( * )( DWORD )>( GetProcAddress( hKernelDLL, "SetDefaultDllDirectories" ) ) : 0;
  DLL_DIRECTORY_COOKIE ( *AddDllDirectory )( PCWSTR ) = hKernelDLL ? reinterpret_cast<DLL_DIRECTORY_COOKIE ( * )( PCWSTR )>( GetProcAddress( hKernelDLL, "AddDllDirectory" ) ) : 0;
#ifndef _MSC_VER // MinGW
#pragma GCC diagnostic pop
#endif

  if ( SetDefaultDllDirectories && AddDllDirectory )
  {
    SetDefaultDllDirectories( LOAD_LIBRARY_SEARCH_DEFAULT_DIRS );

    wchar_t windir[MAX_PATH];
    GetWindowsDirectoryW( windir, MAX_PATH );
    wchar_t systemdir[MAX_PATH];
    GetSystemDirectoryW( systemdir, MAX_PATH );

    wchar_t *path = wcsdup( _wgetenv( L"PATH" ) );

#ifdef _UCRT
    for ( wchar_t *p = wcstok( path, L";", nullptr ); p; p = wcstok( NULL, L";", nullptr ) )
#else
    for ( wchar_t *p = wcstok( path, L";" ); p; p = wcstok( NULL, L";" ) )
#endif
    {
      if ( wcsicmp( p, windir ) == 0 )
        continue;
      if ( wcsicmp( p, systemdir ) == 0 )
        continue;
      AddDllDirectory( p );
    }

    free( path );
  }


#ifdef _MSC_VER
  HINSTANCE hGetProcIDDLL = LoadLibrary( "qgis_app.dll" );
#else
  // MinGW
  HINSTANCE hGetProcIDDLL = LoadLibrary( "libqgis_app.dll" );
#endif

  if ( !hGetProcIDDLL )
  {
    DWORD error = GetLastError();
    LPTSTR errorText = NULL;

    FormatMessage(
      FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_IGNORE_INSERTS,
      NULL,
      error,
      MAKELANGID( LANG_NEUTRAL, SUBLANG_DEFAULT ),
      ( LPTSTR ) &errorText,
      0,
      NULL
    );

    std::string message = "Could not load qgis_app.dll \n Windows Error: " + std::string( errorText )
                          + "\n Help: \n\n Check " + basename + ".env for correct environment paths";
    showError( message, "Error loading QGIS" );

    LocalFree( errorText );
    errorText = NULL;
    return EXIT_FAILURE;
  }

#ifndef _MSC_VER // MinGW
#pragma GCC diagnostic ignored "-Wcast-function-type"
#endif
  int ( *realmain )( int, char *[] ) = ( int ( * )( int, char *[] ) ) GetProcAddress( hGetProcIDDLL, "main" );
#ifndef _MSC_VER // MinGW
#pragma GCC diagnostic pop
#endif

  if ( !realmain )
  {
    showError( "Could not locate main function in qgis_app.dll", "Error loading QGIS" );
    return EXIT_FAILURE;
  }

  return realmain( __argc, __argv );
}
