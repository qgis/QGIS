#include <windows.h>
#include <io.h>

#include <sstream>
#include <iostream>
#include <fstream>
#include <list>
#include <memory>

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
  basename.resize( basename.length() - 4 );
  return basename;
}


int CALLBACK WinMain( HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow )
{
  std::string basename( moduleExeBaseName() );

  if ( getenv( "OSGEO4W_ROOT" ) )
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
      catch ( std::ifstream::failure e )
      {
        std::cerr << "could read environment variable list " << basename + ".vars" << " [" << e.what() << "]" << std::endl;
        return EXIT_FAILURE;
      }

      try
      {
        std::ofstream file;
        file.open( envfile, std::ifstream::out );

        for ( std::list<std::string>::const_iterator it = vars.begin();  it != vars.end(); ++it )
        {
          if ( getenv( it->c_str() ) )
            file << *it << "=" << getenv( it->c_str() ) << std::endl;
        }
      }
      catch ( std::ifstream::failure e )
      {
        std::cerr << "could not write environment file " << basename + ".env" << " [" << e.what() << "]" << std::endl;
        return EXIT_FAILURE;
      }
    }

    if ( __argc == 2 && strcmp( __argv[1], "--exit" ) == 0 )
    {
      return EXIT_SUCCESS;
    }
  }
  else
  {
    try
    {
      std::ifstream file;
      file.open( basename + ".env" );

      std::string var;
      while ( std::getline( file, var ) )
      {
        if ( _putenv( var.c_str() ) < 0 )
        {
          std::cerr << "could not set environment variable:" << var << std::endl;
          return EXIT_FAILURE;
        }
      }
    }
    catch ( std::ifstream::failure e )
    {
      std::cerr << "could not read environment file " << basename + ".env" << " [" << e.what() << "]" << std::endl;
      return EXIT_FAILURE;
    }
  }

#ifdef _MSC_VER
  HINSTANCE hGetProcIDDLL = LoadLibrary( "qgis_app.dll" );
#else
  // MinGW
  HINSTANCE hGetProcIDDLL = LoadLibrary( "libqgis_app.dll" );
#endif

  if ( !hGetProcIDDLL )
  {
    std::cerr << "Could not load the qgis_app.dll" << std::endl;
    return EXIT_FAILURE;
  }

  int ( *realmain )( int, char *[] ) = ( int ( * )( int, char *[] ) ) GetProcAddress( hGetProcIDDLL, "main" );
  if ( !realmain )
  {
    std::cerr << "could not locate main function in qgis_app.dll" << std::endl;
    return EXIT_FAILURE;
  }

  return realmain( __argc, __argv );
}
