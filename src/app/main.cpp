/***************************************************************************
                            main.cpp  -  description
                              -------------------
              begin                : Fri Jun 21 10:48:28 AKDT 2002
              copyright            : (C) 2002 by Gary E.Sherman
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

//qt includes
#include <QBitmap>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QFont>
#include <QFontDatabase>
#include <QPixmap>
#include <QLocale>
#include <QSettings>
#include <QSplashScreen>
#include <QString>
#include <QStringList>
#include <QStyle>
#if QT_VERSION < 0x050000
#include <QPlastiqueStyle>
#endif
#include <QTranslator>
#include <QImageReader>
#include <QMessageBox>

#include "qgscustomization.h"
#include "qgsfontutils.h"
#include "qgspluginregistry.h"
#include "qgsmessagelog.h"
#include "qgspythonrunner.h"

#include <cstdio>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

#ifdef WIN32
// Open files in binary mode
#include <fcntl.h> /*  _O_BINARY */
#include <windows.h>
#include <dbghelp.h>
#include <time.h>
#ifdef MSVC
#undef _fmode
int _fmode = _O_BINARY;
#else
// Only do this if we are not building on windows with msvc.
// Recommended method for doing this with msvc is with a call to _set_fmode
// which is the first thing we do in main().
// Similarly, with MinGW set _fmode in main().
#endif  //_MSC_VER
#else
#include <getopt.h>
#endif

#ifdef Q_OS_MACX
#include <ApplicationServices/ApplicationServices.h>
#if MAC_OS_X_VERSION_MAX_ALLOWED < 1050
typedef SInt32 SRefCon;
#endif
// For setting the maximum open files limit higher
#include <sys/resource.h>
#include <limits.h>
#endif

#include "qgisapp.h"
#include "qgsmapcanvas.h"
#include "qgsapplication.h"
#include <qgsconfig.h>
#include <qgscustomization.h>
#include <qgsversion.h>
#include "qgsexception.h"
#include "qgsproject.h"
#include "qgsrectangle.h"
#include "qgslogger.h"

#if ((defined(linux) || defined(__linux__)) && !defined(ANDROID)) || defined(__FreeBSD__)
#include <unistd.h>
#include <execinfo.h>
#include <signal.h>
#include <sys/wait.h>
#include <errno.h>
#endif

/** print usage text
 */
void usage( std::string const & appName )
{
  std::cerr << "QGIS - " << VERSION << " '" << RELEASE_NAME << "' ("
            << QGSVERSION << ")\n"
            << "QGIS is a user friendly Open Source Geographic Information System.\n"
            << "Usage: " << appName <<  " [OPTION] [FILE]\n"
            << "  OPTION:\n"
            << "\t[--snapshot filename]\temit snapshot of loaded datasets to given file\n"
            << "\t[--width width]\twidth of snapshot to emit\n"
            << "\t[--height height]\theight of snapshot to emit\n"
            << "\t[--lang language]\tuse language for interface text\n"
            << "\t[--project projectfile]\tload the given QGIS project\n"
            << "\t[--extent xmin,ymin,xmax,ymax]\tset initial map extent\n"
            << "\t[--nologo]\thide splash screen\n"
            << "\t[--noplugins]\tdon't restore plugins on startup\n"
            << "\t[--nocustomization]\tdon't apply GUI customization\n"
            << "\t[--customizationfile]\tuse the given ini file as GUI customization\n"
            << "\t[--optionspath path]\tuse the given QSettings path\n"
            << "\t[--configpath path]\tuse the given path for all user configuration\n"
            << "\t[--code path]\trun the given python file on load\n"
            << "\t[--defaultui]\tstart by resetting user ui settings to default\n"
            << "\t[--help]\t\tthis text\n\n"
            << "  FILE:\n"
            << "    Files specified on the command line can include rasters,\n"
            << "    vectors, and QGIS project files (.qgs): \n"
            << "     1. Rasters - supported formats include GeoTiff, DEM \n"
            << "        and others supported by GDAL\n"
            << "     2. Vectors - supported formats include ESRI Shapefiles\n"
            << "        and others supported by OGR and PostgreSQL layers using\n"
            << "        the PostGIS extension\n"  ; // OK


} // usage()


/////////////////////////////////////////////////////////////////
// Command line options 'behaviour' flag setup
////////////////////////////////////////////////////////////////

// These two are global so that they can be set by the OpenDocuments
// AppleEvent handler as well as by the main routine argv processing

// This behaviour will cause QGIS to autoload a project
static QString myProjectFileName = "";

// This is the 'leftover' arguments collection
static QStringList myFileList;


/* Test to determine if this program was started on Mac OS X by double-clicking
 * the application bundle rather then from a command line. If clicked, argv[1]
 * contains a process serial number in the form -psn_0_1234567. Don't process
 * the command line arguments in this case because argv[1] confuses the processing.
 */
bool bundleclicked( int argc, char *argv[] )
{
  return ( argc > 1 && memcmp( argv[1], "-psn_", 5 ) == 0 );
}

void myPrint( const char *fmt, ... )
{
  va_list ap;
  va_start( ap, fmt );
#if defined(Q_OS_WIN)
  char buffer[1024];
  vsnprintf( buffer, sizeof buffer, fmt, ap );
  OutputDebugString( buffer );
#else
  vfprintf( stderr, fmt, ap );
#endif
  va_end( ap );
}

static void dumpBacktrace( unsigned int depth )
{
  if ( depth == 0 )
    depth = 20;

#if ((defined(linux) || defined(__linux__)) && !defined(ANDROID)) || defined(__FreeBSD__)
  int stderr_fd = -1;
  if ( access( "/usr/bin/c++filt", X_OK ) < 0 )
  {
    myPrint( "Stacktrace (c++filt NOT FOUND):\n" );
  }
  else
  {
    int fd[2];

    if ( pipe( fd ) == 0 && fork() == 0 )
    {
      close( STDIN_FILENO ); // close stdin

      // stdin from pipe
      if ( dup( fd[0] ) != STDIN_FILENO )
      {
        QgsDebugMsg( "dup to stdin failed" );
      }

      close( fd[1] );        // close writing end
      execl( "/usr/bin/c++filt", "c++filt", ( char * ) 0 );
      perror( "could not start c++filt" );
      exit( 1 );
    }

    myPrint( "Stacktrace (piped through c++filt):\n" );
    stderr_fd = dup( STDERR_FILENO );
    close( fd[0] );          // close reading end
    close( STDERR_FILENO );  // close stderr

    // stderr to pipe
    if ( dup( fd[1] ) != STDERR_FILENO )
    {
      QgsDebugMsg( "dup to stderr failed" );
    }

    close( fd[1] );          // close duped pipe
  }

  void **buffer = new void *[ depth ];
  int nptrs = backtrace( buffer, depth );
  backtrace_symbols_fd( buffer, nptrs, STDERR_FILENO );
  delete [] buffer;
  if ( stderr_fd >= 0 )
  {
    int status;
    close( STDERR_FILENO );
    if ( dup( stderr_fd ) != STDERR_FILENO )
    {
      QgsDebugMsg( "dup to stderr failed" );
    }
    close( stderr_fd );
    wait( &status );
  }
#elif defined(Q_OS_WIN)
  void **buffer = new void *[ depth ];

  SymSetOptions( SYMOPT_DEFERRED_LOADS | SYMOPT_INCLUDE_32BIT_MODULES | SYMOPT_UNDNAME );
  SymInitialize( GetCurrentProcess(), "http://msdl.microsoft.com/download/symbols;http://download.osgeo.org/osgeo4w/symstore", TRUE );

  unsigned short nFrames = CaptureStackBackTrace( 1, depth, buffer, NULL );
  SYMBOL_INFO *symbol = ( SYMBOL_INFO * ) qgsMalloc( sizeof( SYMBOL_INFO ) + 256 );
  symbol->MaxNameLen = 255;
  symbol->SizeOfStruct = sizeof( SYMBOL_INFO );

  for ( int i = 0; i < nFrames; i++ )
  {
    SymFromAddr( GetCurrentProcess(), ( DWORD64 )( buffer[ i ] ), 0, symbol );
    symbol->Name[ 255 ] = 0;
    myPrint( "%d: %s [%x]\n", i, symbol->Name, symbol->Address );
  }

  qgsFree( symbol );
#endif
}

#if (defined(linux) && !defined(ANDROID)) || defined(__FreeBSD__)
void qgisCrash( int signal )
{
  fprintf( stderr, "QGIS died on signal %d", signal );

  if ( access( "/usr/bin/gdb", X_OK ) == 0 )
  {
    // take full stacktrace using gdb
    // http://stackoverflow.com/questions/3151779/how-its-better-to-invoke-gdb-from-program-to-print-its-stacktrace
    // unfortunately, this is not so simple. the proper method is way more OS-specific
    // than this code would suggest, see http://stackoverflow.com/a/1024937

    char exename[512];
#if defined(__FreeBSD__)
    int len = readlink( "/proc/curproc/file", exename, sizeof( exename ) - 1 );
#else
    int len = readlink( "/proc/self/exe", exename, sizeof( exename ) - 1 );
#endif
    if ( len < 0 )
    {
      myPrint( "Could not read link (%d:%s)\n", errno, strerror( errno ) );
    }
    else
    {
      exename[ len ] = 0;

      char pidstr[32];
      snprintf( pidstr, sizeof pidstr, "--pid=%d", getpid() );

      int gdbpid = fork();
      if ( gdbpid == 0 )
      {
        // attach, backtrace and continue
        execl( "/usr/bin/gdb", "gdb", "-q", "-batch", "-n", pidstr, "-ex", "thread", "-ex", "bt full", exename, NULL );
        perror( "cannot exec gdb" );
        exit( 1 );
      }
      else if ( gdbpid >= 0 )
      {
        int status;
        waitpid( gdbpid, &status, 0 );
        myPrint( "gdb returned %d\n", status );
      }
      else
      {
        myPrint( "Cannot fork (%d:%s)\n", errno, strerror( errno ) );
        dumpBacktrace( 256 );
      }
    }
  }

  abort();
}
#endif

/*
 * Hook into the qWarning/qFatal mechanism so that we can channel messages
 * from libpng to the user.
 *
 * Some JPL WMS images tend to overload the libpng 1.2.2 implementation
 * somehow (especially when zoomed in)
 * and it would be useful for the user to know why their picture turned up blank
 *
 * Based on qInstallMsgHandler example code in the Qt documentation.
 *
 */
void myMessageOutput( QtMsgType type, const char *msg )
{
  switch ( type )
  {
    case QtDebugMsg:
      myPrint( "%s\n", msg );
      if ( strncmp( msg, "Backtrace", 9 ) == 0 )
        dumpBacktrace( atoi( msg + 9 ) );
      break;
    case QtCriticalMsg:
      myPrint( "Critical: %s\n", msg );
      break;
    case QtWarningMsg:
      myPrint( "Warning: %s\n", msg );

#ifdef QGISDEBUG
      // Print all warnings except setNamedColor.
      // Only seems to happen on windows
      if ( 0 != strncmp( msg, "QColor::setNamedColor: Unknown color name 'param", 48 ) )
      {
        // TODO: Verify this code in action.
        dumpBacktrace( 20 );
        QgsMessageLog::logMessage( msg, "Qt" );
      }
#endif

      // TODO: Verify this code in action.
      if ( 0 == strncmp( msg, "libpng error:", 13 ) )
      {
        // Let the user know
        QgsMessageLog::logMessage( msg, "libpng" );
      }

      break;
    case QtFatalMsg:
    {
      myPrint( "Fatal: %s\n", msg );
#if (defined(linux) && !defined(ANDROID)) || defined(__FreeBSD__)
      qgisCrash( -1 );
#else
      dumpBacktrace( 256 );
      abort();                    // deliberately dump core
#endif
    }
  }
}

int main( int argc, char *argv[] )
{
#ifdef Q_OS_MACX
  // Increase file resource limits (i.e., number of allowed open files)
  // (from code provided by Larry Biehl, Purdue University, USA, from 'MultiSpec' project)
  // This is generally 256 for the soft limit on Mac
  // NOTE: setrlimit() must come *before* initialization of stdio strings,
  //       e.g. before any debug messages, or setrlimit() gets ignored
  // see: http://stackoverflow.com/a/17726104/2865523
  struct rlimit rescLimit;
  if ( getrlimit( RLIMIT_NOFILE, &rescLimit ) == 0 )
  {
    rlim_t oldSoft( rescLimit.rlim_cur );
    rlim_t oldHard( rescLimit.rlim_max );
#ifdef OPEN_MAX
    rlim_t newSoft( OPEN_MAX );
    rlim_t newHard( std::min( oldHard, newSoft ) );
#else
    rlim_t newSoft( 4096 );
    rlim_t newHard( std::min(( rlim_t )8192, oldHard ) );
#endif
    if ( rescLimit.rlim_cur < newSoft )
    {
      rescLimit.rlim_cur = newSoft;
      rescLimit.rlim_max = newHard;

      if ( setrlimit( RLIMIT_NOFILE, &rescLimit ) == 0 )
      {
        QgsDebugMsg( QString( "Mac RLIMIT_NOFILE Soft/Hard NEW: %1 / %2" )
                     .arg( rescLimit.rlim_cur ).arg( rescLimit.rlim_max ) );
      }
    }
    QgsDebugMsg( QString( "Mac RLIMIT_NOFILE Soft/Hard ORIG: %1 / %2" )
                 .arg( oldSoft ).arg( oldHard ) );
  }
#endif

  QgsDebugMsg( QString( "Starting qgis main" ) );
#ifdef WIN32  // Windows
#ifdef _MSC_VER
  _set_fmode( _O_BINARY );
#else //MinGW
  _fmode = _O_BINARY;
#endif  // _MSC_VER
#endif  // WIN32

  // Set up the custom qWarning/qDebug custom handler
#ifndef ANDROID
  qInstallMsgHandler( myMessageOutput );
#endif

#if (defined(linux) && !defined(ANDROID)) || defined(__FreeBSD__)
  signal( SIGQUIT, qgisCrash );
  signal( SIGILL, qgisCrash );
  signal( SIGFPE, qgisCrash );
  signal( SIGSEGV, qgisCrash );
  signal( SIGBUS, qgisCrash );
  signal( SIGSYS, qgisCrash );
  signal( SIGTRAP, qgisCrash );
  signal( SIGXCPU, qgisCrash );
  signal( SIGXFSZ, qgisCrash );
#endif

#ifdef Q_OS_WIN
  SetUnhandledExceptionFilter( QgisApp::qgisCrashDump );
#endif

  // initialize random number seed
  srand( time( NULL ) );

  /////////////////////////////////////////////////////////////////
  // Command line options 'behaviour' flag setup
  ////////////////////////////////////////////////////////////////

  //
  // Parse the command line arguments, looking to see if the user has asked for any
  // special behaviours. Any remaining non command arguments will be kept aside to
  // be passed as a list of layers and / or a project that should be loaded.
  //

  // This behaviour is used to load the app, snapshot the map,
  // save the image to disk and then exit
  QString mySnapshotFileName = "";
  int mySnapshotWidth = 800;
  int mySnapshotHeight = 600;

  bool myHideSplash = false;
#if defined(ANDROID)
  QgsDebugMsg( QString( "Android: Splash hidden" ) );
  myHideSplash = true;
#endif

  bool myRestoreDefaultWindowState = false;
  bool myRestorePlugins = true;
  bool myCustomization = true;

  // This behaviour will set initial extent of map canvas, but only if
  // there are no command line arguments. This gives a usable map
  // extent when qgis starts with no layers loaded. When layers are
  // loaded, we let the layers define the initial extent.
  QString myInitialExtent = "";
  if ( argc == 1 )
    myInitialExtent = "-1,-1,1,1";

  // This behaviour will allow you to force the use of a translation file
  // which is useful for testing
  QString myTranslationCode;

  // The user can specify a path which will override the default path of custom
  // user settings (~/.qgis) and it will be used for QSettings INI file
  QString configpath;
  QString optionpath;

  QString pythonfile;

  QString customizationfile;

#if defined(ANDROID)
  QgsDebugMsg( QString( "Android: All params stripped" ) );// Param %1" ).arg( argv[0] ) );
  //put all QGIS settings in the same place
  configpath = QgsApplication::qgisSettingsDirPath();
  QgsDebugMsg( QString( "Android: configpath set to %1" ).arg( configpath ) );
#endif

  QStringList args;

  if ( !bundleclicked( argc, argv ) )
  {
    // Build a local QCoreApplication from arguments. This way, arguments are correctly parsed from their native locale
    // It will use QString::fromLocal8Bit( argv ) under Unix and GetCommandLine() under Windows.
    QCoreApplication coreApp( argc, argv );
    args = QCoreApplication::arguments();

    for ( int i = 1; i < args.size(); ++i )
    {
      QString arg = args[i];

      if ( arg == "--help" || arg == "-?" )
      {
        usage( args[0].toStdString() );
        return 2;
      }
      else if ( arg == "--nologo" || arg == "-n" )
      {
        myHideSplash = true;
      }
      else if ( arg == "--noplugins" || arg == "-P" )
      {
        myRestorePlugins = false;
      }
      else if ( arg == "--nocustomization" || arg == "-C" )
      {
        myCustomization = false;
      }
      else if ( i + 1 < argc && ( arg == "--snapshot" || arg == "-s" ) )
      {
        mySnapshotFileName = QDir::toNativeSeparators( QFileInfo( args[++i] ).absoluteFilePath() );
      }
      else if ( i + 1 < argc && ( arg == "--width" || arg == "-w" ) )
      {
        mySnapshotWidth = QString( args[++i] ).toInt();
      }
      else if ( i + 1 < argc && ( arg == "--height" || arg == "-h" ) )
      {
        mySnapshotHeight = QString( args[++i] ).toInt();
      }
      else if ( i + 1 < argc && ( arg == "--lang" || arg == "-l" ) )
      {
        myTranslationCode = args[++i];
      }
      else if ( i + 1 < argc && ( arg == "--project" || arg == "-p" ) )
      {
        myProjectFileName = QDir::toNativeSeparators( QFileInfo( args[++i] ).absoluteFilePath() );
      }
      else if ( i + 1 < argc && ( arg == "--extent" || arg == "-e" ) )
      {
        myInitialExtent = args[++i];
      }
      else if ( i + 1 < argc && ( arg == "--optionspath" || arg == "-o" ) )
      {
        optionpath = QDir::toNativeSeparators( QDir( args[++i] ).absolutePath() );
      }
      else if ( i + 1 < argc && ( arg == "--configpath" || arg == "-c" ) )
      {
        configpath = QDir::toNativeSeparators( QDir( args[++i] ).absolutePath() );
      }
      else if ( i + 1 < argc && ( arg == "--code" || arg == "-f" ) )
      {
        pythonfile = QDir::toNativeSeparators( QFileInfo( args[++i] ).absoluteFilePath() );
      }
      else if ( i + 1 < argc && ( arg == "--customizationfile" || arg == "-z" ) )
      {
        customizationfile = QDir::toNativeSeparators( QFileInfo( args[++i] ).absoluteFilePath() );
      }
      else if ( arg == "--defaultui" || arg == "-d" )
      {
        myRestoreDefaultWindowState = true;
      }
      else
      {
        myFileList.append( QDir::toNativeSeparators( QFileInfo( args[i] ).absoluteFilePath() ) );
      }
    }
  }

  /////////////////////////////////////////////////////////////////////
  // If no --project was specified, parse the args to look for a     //
  // .qgs file and set myProjectFileName to it. This allows loading  //
  // of a project file by clicking on it in various desktop managers //
  // where an appropriate mime-type has been set up.                 //
  /////////////////////////////////////////////////////////////////////
  if ( myProjectFileName.isEmpty() )
  {
    // check for a .qgs
    for ( int i = 0; i < args.size(); i++ )
    {
      QString arg = QDir::toNativeSeparators( QFileInfo( args[i] ).absoluteFilePath() );
      if ( arg.contains( ".qgs" ) )
      {
        myProjectFileName = arg;
        break;
      }
    }
  }


  /////////////////////////////////////////////////////////////////////
  // Now we have the handlers for the different behaviours...
  ////////////////////////////////////////////////////////////////////

  /////////////////////////////////////////////////////////////////////
  // Initialise the application and the translation stuff
  /////////////////////////////////////////////////////////////////////

#ifdef Q_OS_UNIX
  bool myUseGuiFlag = getenv( "DISPLAY" ) != 0;
#else
  bool myUseGuiFlag = true;
#endif
  if ( !myUseGuiFlag )
  {
    std::cerr << QObject::tr(
                "QGIS starting in non-interactive mode not supported.\n"
                "You are seeing this message most likely because you "
                "have no DISPLAY environment variable set.\n"
              ).toUtf8().constData();
    exit( 1 ); //exit for now until a version of qgis is capabable of running non interactive
  }

  if ( !optionpath.isEmpty() || !configpath.isEmpty() )
  {
    // tell QSettings to use INI format and save the file in custom config path
    QSettings::setDefaultFormat( QSettings::IniFormat );
    QSettings::setPath( QSettings::IniFormat, QSettings::UserScope, optionpath.isEmpty() ? configpath : optionpath );
  }

  // GUI customization is enabled according to settings (loaded when instance is created)
  // we force disabled here if --nocustomization argument is used
  if ( !myCustomization )
  {
    QgsCustomization::instance()->setEnabled( false );
  }

  QgsApplication myApp( argc, argv, myUseGuiFlag, configpath );

// (if Windows/Mac, use icon from resource)
#if !defined(Q_OS_WIN) && !defined(Q_OS_MAC)
  myApp.setWindowIcon( QIcon( QgsApplication::iconsPath() + "qgis-icon-60x60.png" ) );
#endif

  //
  // Set up the QSettings environment must be done after qapp is created
  QCoreApplication::setOrganizationName( QgsApplication::QGIS_ORGANIZATION_NAME );
  QCoreApplication::setOrganizationDomain( QgsApplication::QGIS_ORGANIZATION_DOMAIN );
  QCoreApplication::setApplicationName( QgsApplication::QGIS_APPLICATION_NAME );
  QCoreApplication::setAttribute( Qt::AA_DontShowIconsInMenus, false );

  QSettings* customizationsettings;
  if ( !optionpath.isEmpty() || !configpath.isEmpty() )
  {
    // tell QSettings to use INI format and save the file in custom config path
    QSettings::setDefaultFormat( QSettings::IniFormat );
    QString path = optionpath.isEmpty() ? configpath : optionpath;
    QSettings::setPath( QSettings::IniFormat, QSettings::UserScope, path );
    customizationsettings = new QSettings( QSettings::IniFormat, QSettings::UserScope, "QGIS", "QGISCUSTOMIZATION2" );
  }
  else
  {
    customizationsettings = new QSettings( "QGIS", "QGISCUSTOMIZATION2" );
  }

  // Using the customizationfile option always overrides the option and config path options.
  if ( !customizationfile.isEmpty() )
  {
    customizationsettings = new QSettings( customizationfile, QSettings::IniFormat );
    QgsCustomization::instance()->setEnabled( true );
  }

  // Load and set possible default customization, must be done afterQgsApplication init and QSettings ( QCoreApplication ) init
  QgsCustomization::instance()->setSettings( customizationsettings );
  QgsCustomization::instance()->loadDefault();

#ifdef Q_OS_MACX
  // If the GDAL plugins are bundled with the application and GDAL_DRIVER_PATH
  // is not already defined, use the GDAL plugins in the application bundle.
  QString gdalPlugins( QCoreApplication::applicationDirPath().append( "/lib/gdalplugins" ) );
  if ( QFile::exists( gdalPlugins ) && !getenv( "GDAL_DRIVER_PATH" ) )
  {
    setenv( "GDAL_DRIVER_PATH", gdalPlugins.toUtf8(), 1 );
  }
#endif

  QSettings mySettings;

  // update any saved setting for older themes to new default 'gis' theme (2013-04-15)
  if ( mySettings.contains( "/Themes" ) )
  {
    QString theme = mySettings.value( "/Themes", "default" ).toString();
    if ( theme == QString( "gis" )
         || theme == QString( "classic" )
         || theme == QString( "nkids" ) )
    {
      mySettings.setValue( "/Themes", QString( "default" ) );
    }
  }


  // custom environment variables
  QMap<QString, QString> systemEnvVars = QgsApplication::systemEnvVars();
  bool useCustomVars = mySettings.value( "qgis/customEnvVarsUse", QVariant( false ) ).toBool();
  if ( useCustomVars )
  {
    QStringList customVarsList = mySettings.value( "qgis/customEnvVars", "" ).toStringList();
    if ( !customVarsList.isEmpty() )
    {
      foreach ( const QString &varStr, customVarsList )
      {
        int pos = varStr.indexOf( QLatin1Char( '|' ) );
        if ( pos == -1 )
          continue;
        QString envVarApply = varStr.left( pos );
        QString varStrNameValue = varStr.mid( pos + 1 );
        pos = varStrNameValue.indexOf( QLatin1Char( '=' ) );
        if ( pos == -1 )
          continue;
        QString envVarName = varStrNameValue.left( pos );
        QString envVarValue = varStrNameValue.mid( pos + 1 );

        if ( systemEnvVars.contains( envVarName ) )
        {
          if ( envVarApply == "prepend" )
          {
            envVarValue += systemEnvVars.value( envVarName );
          }
          else if ( envVarApply == "append" )
          {
            envVarValue = systemEnvVars.value( envVarName ) + envVarValue;
          }
        }

        if ( systemEnvVars.contains( envVarName ) && envVarApply == "unset" )
        {
#ifdef Q_OS_WIN
          putenv( envVarName.toUtf8().constData() );
#else
          unsetenv( envVarName.toUtf8().constData() );
#endif
        }
        else
        {
#ifdef Q_OS_WIN
          if ( envVarApply != "undefined" || !getenv( envVarName.toUtf8().constData() ) )
            putenv( QString( "%1=%2" ).arg( envVarName ).arg( envVarValue ).toUtf8().constData() );
#else
          setenv( envVarName.toUtf8().constData(), envVarValue.toUtf8().constData(), envVarApply == "undefined" ? 0 : 1 );
#endif
        }
      }
    }
  }

#ifdef QGISDEBUG
  QgsFontUtils::loadStandardTestFonts( QStringList() << "Roman" << "Bold" );
#endif

  // Set the application style.  If it's not set QT will use the platform style except on Windows
  // as it looks really ugly so we use QPlastiqueStyle.
  QString style = mySettings.value( "/qgis/style" ).toString();
  if ( !style.isNull() )
    QApplication::setStyle( style );
#ifdef Q_OS_WIN
#if QT_VERSION < 0x050000
  else
    QApplication::setStyle( new QPlastiqueStyle );
#endif
#endif

  /* Translation file for QGIS.
   */
  QString i18nPath = QgsApplication::i18nPath();
  QString myUserLocale = mySettings.value( "locale/userLocale", "" ).toString();
  bool myLocaleOverrideFlag = mySettings.value( "locale/overrideFlag", false ).toBool();
  QString myLocale;
  //
  // Priority of translation is:
  //  - command line
  //  - user secified in options dialog (with group checked on)
  //  - system locale
  //
  //  When specifying from the command line it will change the user
  //  specified user locale
  //
  if ( !myTranslationCode.isNull() && !myTranslationCode.isEmpty() )
  {
    mySettings.setValue( "locale/userLocale", myTranslationCode );
  }
  else
  {
    if ( !myLocaleOverrideFlag || myUserLocale.isEmpty() )
    {
      myTranslationCode = QLocale::system().name();
      //setting the locale/userLocale when the --lang= option is not set will allow third party
      //plugins to always use the same locale as the QGIS, otherwise they can be out of sync
      mySettings.setValue( "locale/userLocale", myTranslationCode );
    }
    else
    {
      myTranslationCode = myUserLocale;
    }
  }

  QTranslator qgistor( 0 );
  QTranslator qttor( 0 );
  if ( myTranslationCode != "C" )
  {
    if ( qgistor.load( QString( "qgis_" ) + myTranslationCode, i18nPath ) )
    {
      myApp.installTranslator( &qgistor );
    }
    else
    {
      qWarning( "loading of qgis translation failed [%s]", QString( "%1/qgis_%2" ).arg( i18nPath ).arg( myTranslationCode ).toLocal8Bit().constData() );
    }

    /* Translation file for Qt.
     * The strings from the QMenuBar context section are used by Qt/Mac to shift
     * the About, Preferences and Quit items to the Mac Application menu.
     * These items must be translated identically in both qt_ and qgis_ files.
     */
    if ( qttor.load( QString( "qt_" ) + myTranslationCode, QLibraryInfo::location( QLibraryInfo::TranslationsPath ) ) )
    {
      myApp.installTranslator( &qttor );
    }
    else
    {
      qWarning( "loading of qt translation failed [%s]", QString( "%1/qt_%2" ).arg( QLibraryInfo::location( QLibraryInfo::TranslationsPath ) ).arg( myTranslationCode ).toLocal8Bit().constData() );
    }
  }

  // For non static builds on mac and win (static builds are not supported)
  // we need to be sure we can find the qt image
  // plugins. In mac be sure to look in the
  // application bundle...
#ifdef Q_OS_WIN
  QCoreApplication::addLibraryPath( QApplication::applicationDirPath()
                                    + QDir::separator() + "qtplugins" );
#endif
#ifdef Q_OS_MACX
  // IMPORTANT: do before Qt uses any plugins, e.g. before loading splash screen
  QString  myPath( QCoreApplication::applicationDirPath().append( "/../PlugIns" ) );
  // Check if it contains a standard Qt-specific plugin subdirectory
  if ( !QFile::exists( myPath + "/imageformats" ) )
  {
    // We are either running from build dir bundle, or launching binary directly.
    // Use system Qt plugins, since they are not bundled.
    // An app bundled with QGIS_MACAPP_BUNDLE=0 will still have Plugins/qgis in it
    myPath = QT_PLUGINS_DIR;
  }

  // First clear the plugin search paths so we can be sure only plugins we define
  // are being used. Note: this strips QgsApplication::pluginPath()
  QStringList myPathList;
  QCoreApplication::setLibraryPaths( myPathList );

  QgsDebugMsg( QString( "Adding Mac QGIS and Qt plugins dirs to search path: %1" ).arg( myPath ) );
  QCoreApplication::addLibraryPath( QgsApplication::pluginPath() );
  QCoreApplication::addLibraryPath( myPath );
#endif

  //set up splash screen
  QString mySplashPath( QgsCustomization::instance()->splashPath() );
  QPixmap myPixmap( mySplashPath + QString( "splash.png" ) );
  QSplashScreen *mypSplash = new QSplashScreen( myPixmap );
  if ( mySettings.value( "/qgis/hideSplash" ).toBool() || myHideSplash )
  {
    //splash screen hidden
  }
  else
  {
    //for win and linux we can just automask and png transparency areas will be used
    mypSplash->setMask( myPixmap.mask() );
    mypSplash->show();
  }

  // optionally restore default window state
  // use restoreDefaultWindowState setting only if NOT using command line (then it is set already)
  if ( myRestoreDefaultWindowState || mySettings.value( "/qgis/restoreDefaultWindowState", false ).toBool() )
  {
    QgsDebugMsg( "Resetting /UI/state settings!" );
    mySettings.remove( "/UI/state" );
    mySettings.remove( "/qgis/restoreDefaultWindowState" );
  }

  // set max. thread count
  // this should be done in QgsApplication::init() but it doesn't know the settings dir.
  QgsApplication::setMaxThreads( QSettings().value( "/qgis/max_threads", -1 ).toInt() );

  QgisApp *qgis = new QgisApp( mypSplash, myRestorePlugins ); // "QgisApp" used to find canonical instance
  qgis->setObjectName( "QgisApp" );

  myApp.connect(
    &myApp, SIGNAL( preNotify( QObject *, QEvent *, bool * ) ),
    //qgis, SLOT( preNotify( QObject *, QEvent *))
    QgsCustomization::instance(), SLOT( preNotify( QObject *, QEvent *, bool * ) )
  );

  /////////////////////////////////////////////////////////////////////
  // Load a project file if one was specified
  /////////////////////////////////////////////////////////////////////
  if ( ! myProjectFileName.isEmpty() )
  {
    qgis->openProject( myProjectFileName );
  }

  /////////////////////////////////////////////////////////////////////
  // autoload any file names that were passed in on the command line
  /////////////////////////////////////////////////////////////////////
  QgsDebugMsg( QString( "Number of files in myFileList: %1" ).arg( myFileList.count() ) );
  for ( QStringList::Iterator myIterator = myFileList.begin(); myIterator != myFileList.end(); ++myIterator )
  {
    QgsDebugMsg( QString( "Trying to load file : %1" ).arg(( *myIterator ) ) );
    QString myLayerName = *myIterator;
    // don't load anything with a .qgs extension - these are project files
    if ( !myLayerName.contains( ".qgs" ) )
    {
      qgis->openLayer( myLayerName );
    }
  }


  /////////////////////////////////////////////////////////////////////
  // Set initial extent if requested
  /////////////////////////////////////////////////////////////////////
  if ( ! myInitialExtent.isEmpty() )
  {
    double coords[4];
    int pos, posOld = 0;
    bool ok = true;

    // XXX is it necessary to switch to "C" locale?

    // parse values from string
    // extent is defined by string "xmin,ymin,xmax,ymax"
    for ( int i = 0; i < 3; i++ )
    {
      // find comma and get coordinate
      pos = myInitialExtent.indexOf( ',', posOld );
      if ( pos == -1 )
      {
        ok = false;
        break;
      }

      coords[i] = QString( myInitialExtent.mid( posOld, pos - posOld ) ).toDouble( &ok );
      if ( !ok )
        break;

      posOld = pos + 1;
    }

    // parse last coordinate
    if ( ok )
      coords[3] = QString( myInitialExtent.mid( posOld ) ).toDouble( &ok );

    if ( !ok )
    {
      QgsDebugMsg( "Error while parsing initial extent!" );
    }
    else
    {
      // set extent from parsed values
      QgsRectangle rect( coords[0], coords[1], coords[2], coords[3] );
      qgis->setExtent( rect );
      if ( qgis->mapCanvas() )
      {
        qgis->mapCanvas()->refresh();
      }
    }
  }

  if ( !pythonfile.isEmpty() )
  {
#ifdef Q_OS_WIN
    //replace backslashes with forward slashes
    pythonfile.replace( "\\", "/" );
#endif
    QgsPythonRunner::run( QString( "execfile('%1')" ).arg( pythonfile ) );
  }

  /////////////////////////////////`////////////////////////////////////
  // Take a snapshot of the map view then exit if snapshot mode requested
  /////////////////////////////////////////////////////////////////////
  if ( mySnapshotFileName != "" )
  {

    /*You must have at least one paintEvent() delivered for the window to be
      rendered properly.

      It looks like you don't run the event loop in non-interactive mode, so the
      event is never occuring.

      To achieve this without running the event loop: show the window, then call
      qApp->processEvents(), grab the pixmap, save it, hide the window and exit.
      */
    //qgis->show();
    myApp.processEvents();
    QPixmap * myQPixmap = new QPixmap( mySnapshotWidth, mySnapshotHeight );
    myQPixmap->fill();
    qgis->saveMapAsImage( mySnapshotFileName, myQPixmap );
    myApp.processEvents();
    qgis->hide();

    QgsPluginRegistry::instance()->unloadAll();

    return 1;
  }

  /////////////////////////////////////////////////////////////////////
  // Continue on to interactive gui...
  /////////////////////////////////////////////////////////////////////
  qgis->show();
  myApp.connect( &myApp, SIGNAL( lastWindowClosed() ), &myApp, SLOT( quit() ) );

  mypSplash->finish( qgis );
  delete mypSplash;

  qgis->completeInitialization();

#if defined(ANDROID)
  // fix for Qt Ministro hiding app's menubar in favor of native Android menus
  qgis->menuBar()->setNativeMenuBar( false );
  qgis->menuBar()->setVisible( true );
#endif

  int retval = myApp.exec();
  delete qgis;
  return retval;
}
