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
#include <QPixmap>
#include <QLocale>
#include <QSettings>
#include <QSplashScreen>
#include <QString>
#include <QStringList>
#include <QStyle>
#include <QPlastiqueStyle>
#include <QTranslator>
#include <QImageReader>
#include <QMessageBox>

#include "qgscustomization.h"
#include "qgspluginregistry.h"
#include "qgsmessagelog.h"
#include "qgspythonrunner.h"

#include <cstdio>
#include <stdio.h>
#include <stdlib.h>

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

#if defined(linux) && !defined(ANDROID)
#include <unistd.h>
#include <execinfo.h>
#include <signal.h>
#endif

// (if Windows/Mac, use icon from resource)
#if !defined(Q_WS_WIN) && !defined(Q_WS_MAC)
#include "../../images/themes/default/qgis.xpm" // Linux
#include <QIcon>
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

#ifdef Q_OS_WIN
LONG WINAPI qgisCrashDump( struct _EXCEPTION_POINTERS *ExceptionInfo )
{
  QString dumpName = QDir::toNativeSeparators(
                       QString( "%1\\qgis-%2-%3-%4-%5.dmp" )
                       .arg( QDir::tempPath() )
                       .arg( QDateTime::currentDateTime().toString( "yyyyMMdd-hhmmss" ) )
                       .arg( GetCurrentProcessId() )
                       .arg( GetCurrentThreadId() )
                       .arg( QGis::QGIS_DEV_VERSION )
                     );

  QString msg;
  HANDLE hDumpFile = CreateFile( dumpName.toLocal8Bit(), GENERIC_READ | GENERIC_WRITE, FILE_SHARE_WRITE | FILE_SHARE_READ, 0, CREATE_ALWAYS, 0, 0 );
  if ( hDumpFile != INVALID_HANDLE_VALUE )
  {
    MINIDUMP_EXCEPTION_INFORMATION ExpParam;
    ExpParam.ThreadId = GetCurrentThreadId();
    ExpParam.ExceptionPointers = ExceptionInfo;
    ExpParam.ClientPointers = TRUE;

    if ( MiniDumpWriteDump( GetCurrentProcess(), GetCurrentProcessId(), hDumpFile, MiniDumpWithDataSegs, ExceptionInfo ? &ExpParam : NULL, NULL, NULL ) )
    {
      msg = QObject::tr( "minidump written to %1" ).arg( dumpName );
    }
    else
    {
      msg = QObject::tr( "writing of minidump to %1 failed (%2)" ).arg( dumpName ).arg( GetLastError(), 0, 16 );
    }

    CloseHandle( hDumpFile );
  }
  else
  {
    msg = QObject::tr( "creation of minidump to %1 failed (%2)" ).arg( dumpName ).arg( GetLastError(), 0, 16 );
  }

  QMessageBox::critical( 0, QObject::tr( "Crash dumped" ), msg );

  return EXCEPTION_EXECUTE_HANDLER;
}
#endif

#if defined(linux) && !defined(ANDROID)
void qgisCrash( int signal )
{
  qFatal( "QGIS died on signal %d", signal );
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
      fprintf( stderr, "Debug: %s\n", msg );
      break;
    case QtCriticalMsg:
      fprintf( stderr, "Critical: %s\n", msg );
      break;
    case QtWarningMsg:
      fprintf( stderr, "Warning: %s\n", msg );

#ifdef QGISDEBUG
      if ( 0 == strncmp( msg, "Object::", 8 )
           || 0 == strncmp( msg, "QWidget::", 9 )
           || 0 == strncmp( msg, "QPainter::", 10 ) )
      {
#if 0
#if defined(linux) && !defined(ANDROID)
        fprintf( stderr, "Stacktrace (run through c++filt):\n" );
        void *buffer[256];
        int nptrs = backtrace( buffer, sizeof( buffer ) / sizeof( *buffer ) );
        backtrace_symbols_fd( buffer, nptrs, STDERR_FILENO );
#endif
#endif
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
      fprintf( stderr, "Fatal: %s\n", msg );
#if defined(linux) && !defined(ANDROID)
      ( void ) write( STDERR_FILENO, "Stacktrace (run through c++filt):\n", 34 );
      void *buffer[256];
      int nptrs = backtrace( buffer, sizeof( buffer ) / sizeof( *buffer ) );
      backtrace_symbols_fd( buffer, nptrs, STDERR_FILENO );
#endif
      abort();                    // deliberately core dump
    }
  }
}

int main( int argc, char *argv[] )
{
  QgsDebugMsg( QString( "Starting qgis main" ) );
#ifdef WIN32  // Windows
#ifdef _MSC_VER
  _set_fmode( _O_BINARY );
#else //MinGW
  _fmode = _O_BINARY;
#endif  // _MSC_VER
#endif  // WIN32

#if defined(linux) && !defined(ANDROID)
  // Set up the custom qWarning/qDebug custom handler
  qInstallMsgHandler( myMessageOutput );

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
  SetUnhandledExceptionFilter( qgisCrashDump );
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
  configpath = QgsApplication::qgisSettingsPath();
  QgsDebugMsg( QString( "Android: configpath set to %1" ).arg( configpath ) );
#elif defined(Q_WS_WIN)
  for ( int i = 1; i < argc; i++ )
  {
    QString arg = argv[i];

    if ( arg == "--help" || arg == "-?" )
    {
      usage( argv[0] );
      return 2;
    }
    else if ( arg == "-nologo" || arg == "-n" )
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
      mySnapshotFileName = QDir::convertSeparators( QFileInfo( QFile::decodeName( argv[++i] ) ).absoluteFilePath() );
    }
    else if ( i + 1 < argc && ( arg == "--width" || arg == "-w" ) )
    {
      mySnapshotWidth = QString( argv[++i] ).toInt();
    }
    else if ( i + 1 < argc && ( arg == "--height" || arg == "-h" ) )
    {
      mySnapshotHeight = QString( argv[++i] ).toInt();
    }
    else if ( i + 1 < argc && ( arg == "--lang" || arg == "-l" ) )
    {
      myTranslationCode = argv[++i];
    }
    else if ( i + 1 < argc && ( arg == "--project" || arg == "-p" ) )
    {
      myProjectFileName = QDir::convertSeparators( QFileInfo( QFile::decodeName( argv[++i] ) ).absoluteFilePath() );
    }
    else if ( i + 1 < argc && ( arg == "--extent" || arg == "-e" ) )
    {
      myInitialExtent = argv[++i];
    }
    else if ( i + 1 < argc && ( arg == "--optionspath" || arg == "-o" ) )
    {
      optionpath = argv[++i];
    }
    else if ( i + 1 < argc && ( arg == "--configpath" || arg == "-c" ) )
    {
      configpath = argv[++i];
    }
    else if ( i + 1 < argc && ( arg == "--code" || arg == "-f" ) )
    {
      pythonfile = argv[++i];
    }
    else if ( i + 1 < argc && ( arg == "--customizationfile" || arg == "-z" ) )
    {
      customizationfile = argv[++i];
    }
    else
    {
      myFileList.append( QDir::convertSeparators( QFileInfo( QFile::decodeName( argv[i] ) ).absoluteFilePath() ) );
    }
  }
#else
  if ( !bundleclicked( argc, argv ) )
  {

    ////////////////////////////////////////////////////////////////
    // Use the GNU Getopts utility to parse cli arguments
    // Invokes ctor `GetOpt (int argc, char **argv,  char *optstring);'
    ///////////////////////////////////////////////////////////////
    int optionChar;
    while ( 1 )
    {
      static struct option long_options[] =
      {
        /* These options set a flag. */
        {"help", no_argument, 0, '?'},
        {"nologo", no_argument, 0, 'n'},
        {"noplugins", no_argument, 0, 'P'},
        {"nocustomization", no_argument, 0, 'C'},
        /* These options don't set a flag.
         *  We distinguish them by their indices. */
        {"snapshot", required_argument, 0, 's'},
        {"width",    required_argument, 0, 'w'},
        {"height",   required_argument, 0, 'h'},
        {"lang",     required_argument, 0, 'l'},
        {"project",  required_argument, 0, 'p'},
        {"extent",   required_argument, 0, 'e'},
        {"optionspath", required_argument, 0, 'o'},
        {"configpath", required_argument, 0, 'c'},
        {"customizationfile", required_argument, 0, 'z'},
        {"code", required_argument, 0, 'f'},
        {"android", required_argument, 0, 'a'},
        {0, 0, 0, 0}
      };

      /* getopt_long stores the option index here. */
      int option_index = 0;

      optionChar = getopt_long( argc, argv, "swhlpeoc",
                                long_options, &option_index );
      QgsDebugMsg( QString( "Qgis main Debug" ) + optionChar );
      /* Detect the end of the options. */
      if ( optionChar == -1 )
        break;

      switch ( optionChar )
      {
        case 0:
          /* If this option set a flag, do nothing else now. */
          if ( long_options[option_index].flag != 0 )
            break;
          printf( "option %s", long_options[option_index].name );
          if ( optarg )
            printf( " with arg %s", optarg );
          printf( "\n" );
          break;

        case 's':
          mySnapshotFileName = QDir::convertSeparators( QFileInfo( QFile::decodeName( optarg ) ).absoluteFilePath() );
          break;

        case 'w':
          mySnapshotWidth = QString( optarg ).toInt();
          break;

        case 'h':
          mySnapshotHeight = QString( optarg ).toInt();
          break;

        case 'n':
          myHideSplash = true;
          break;

        case 'l':
          myTranslationCode = optarg;
          break;

        case 'p':
          myProjectFileName = QDir::convertSeparators( QFileInfo( QFile::decodeName( optarg ) ).absoluteFilePath() );
          break;

        case 'P':
          myRestorePlugins = false;
          break;

        case 'C':
          myCustomization = false;
          break;

        case 'e':
          myInitialExtent = optarg;
          break;

        case 'o':
          optionpath = optarg;
          break;

        case 'c':
          configpath = optarg;
          break;

        case 'f':
          pythonfile = optarg;
          break;

        case 'z':
          customizationfile = optarg;
          break;

        case '?':
          usage( argv[0] );
          return 2;   // XXX need standard exit codes
          break;

        default:
          QgsDebugMsg( QString( "%1: getopt returned character code %2" ).arg( argv[0] ).arg( optionChar ) );
          return 1;   // XXX need standard exit codes
      }
    }

    // Add any remaining args to the file list - we will attempt to load them
    // as layers in the map view further down....
    QgsDebugMsg( QString( "Files specified on command line: %1" ).arg( optind ) );
    if ( optind < argc )
    {
      while ( optind < argc )
      {
#ifdef QGISDEBUG
        int idx = optind;
        QgsDebugMsg( QString( "%1: %2" ).arg( idx ).arg( argv[idx] ) );
#endif
        myFileList.append( QDir::convertSeparators( QFileInfo( QFile::decodeName( argv[optind++] ) ).absoluteFilePath() ) );
      }
    }
  }
#endif


  /////////////////////////////////////////////////////////////////////
  // Now we have the handlers for the different behaviours...
  ////////////////////////////////////////////////////////////////////

  /////////////////////////////////////////////////////////////////////
  // Initialise the application and the translation stuff
  /////////////////////////////////////////////////////////////////////

#ifdef Q_WS_X11
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
#if !defined(Q_WS_WIN) && !defined(Q_WS_MAC)
  myApp.setWindowIcon( QPixmap( qgis_xpm ) );        // Linux
#endif

  //
  // Set up the QSettings environment must be done after qapp is created
  QCoreApplication::setOrganizationName( "QGIS" );
  QCoreApplication::setOrganizationDomain( "qgis.org" );
  QCoreApplication::setApplicationName( "QGIS2" );
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
#ifdef Q_WS_WIN
          putenv( envVarName.toUtf8().constData() );
#else
          unsetenv( envVarName.toUtf8().constData() );
#endif
        }
        else
        {
#ifdef Q_WS_WIN
          if ( envVarApply != "undefined" || !getenv( envVarName.toUtf8().constData() ) )
            putenv( QString( "%1=%2" ).arg( envVarName ).arg( envVarValue ).toUtf8().constData() );
#else
          setenv( envVarName.toUtf8().constData(), envVarValue.toUtf8().constData(), envVarApply == "undefined" ? 0 : 1 );
#endif
        }
      }
    }
  }

  // Set the application style.  If it's not set QT will use the platform style except on Windows
  // as it looks really ugly so we use QPlastiqueStyle.
  QString style = mySettings.value( "/qgis/style" ).toString();
  if ( !style.isNull() )
    QApplication::setStyle( style );
#ifdef Q_WS_WIN
  else
    QApplication::setStyle( new QPlastiqueStyle );
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

#ifdef QGISDEBUG
// QgsDebugMsg(QString("Setting translation to %1/qgis_%2").arg(i18nPath).arg(myTranslationCode));
#endif
  QTranslator qgistor( 0 );
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
  QTranslator qttor( 0 );
  if ( qttor.load( QString( "qt_" ) + myTranslationCode, QLibraryInfo::location( QLibraryInfo::TranslationsPath ) ) )
  {
    myApp.installTranslator( &qttor );
  }
  else
  {
    qWarning( "loading of qt translation failed [%s]", QString( "%1/qt_%2" ).arg( QLibraryInfo::location( QLibraryInfo::TranslationsPath ) ).arg( myTranslationCode ).toLocal8Bit().constData() );
  }

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

  // For non static builds on mac and win (static builds are not supported)
  // we need to be sure we can find the qt image
  // plugins. In mac be sure to look in the
  // application bundle...
#ifdef Q_WS_WIN
  QCoreApplication::addLibraryPath( QApplication::applicationDirPath()
                                    + QDir::separator() + "qtplugins" );
#endif
#ifdef Q_OS_MACX
  //qDebug("Adding qt image plugins to plugin search path...");
  CFURLRef myBundleRef = CFBundleCopyBundleURL( CFBundleGetMainBundle() );
  CFStringRef myMacPath = CFURLCopyFileSystemPath( myBundleRef, kCFURLPOSIXPathStyle );
  const char *mypPathPtr = CFStringGetCStringPtr( myMacPath, CFStringGetSystemEncoding() );
  CFRelease( myBundleRef );
  CFRelease( myMacPath );
  QString myPath( mypPathPtr );
  // if we are not in a bundle assume that the app is built
  // as a non bundle app and that image plugins will be
  // in system Qt frameworks. If the app is a bundle
  // lets try to set the qt plugin search path...
  QFileInfo myInfo( myPath );
  if ( myInfo.isBundle() )
  {
    // First clear the plugin search paths so we can be sure
    // only plugins from the bundle are being used
    QStringList myPathList;
    QCoreApplication::setLibraryPaths( myPathList );
    // Now set the paths inside the bundle
    myPath += "/Contents/Plugins";
    QCoreApplication::addLibraryPath( myPath );
    if ( QgsApplication::isRunningFromBuildDir() )
    {
      QCoreApplication::addLibraryPath( QTPLUGINSDIR );
    }
    //next two lines should not be needed, testing only
    //QCoreApplication::addLibraryPath( myPath + "/imageformats" );
    //QCoreApplication::addLibraryPath( myPath + "/sqldrivers" );
    //foreach (myPath, myApp.libraryPaths())
    //{
    //qDebug("Path:" + myPath.toLocal8Bit());
    //}
    //qDebug( "Added %s to plugin search path", qPrintable( myPath ) );
    //QList<QByteArray> myFormats = QImageReader::supportedImageFormats();
    //for ( int x = 0; x < myFormats.count(); ++x ) {
    //  qDebug("Format: " + myFormats[x]);
    //}
  }
#endif

  QgisApp *qgis = new QgisApp( mypSplash, myRestorePlugins ); // "QgisApp" used to find canonical instance
  qgis->setObjectName( "QgisApp" );

  myApp.connect(
    &myApp, SIGNAL( preNotify( QObject *, QEvent *, bool * ) ),
    //qgis, SLOT( preNotify( QObject *, QEvent *))
    QgsCustomization::instance(), SLOT( preNotify( QObject *, QEvent *, bool * ) )
  );

  /////////////////////////////////////////////////////////////////////
  // If no --project was specified, parse the args to look for a     //
  // .qgs file and set myProjectFileName to it. This allows loading  //
  // of a project file by clicking on it in various desktop managers //
  // where an appropriate mime-type has been set up.                 //
  /////////////////////////////////////////////////////////////////////
  if ( myProjectFileName.isEmpty() )
  {
    // check for a .qgs
    for ( int i = 0; i < argc; i++ )
    {
      QString arg = QDir::convertSeparators( QFileInfo( QFile::decodeName( argv[i] ) ).absoluteFilePath() );
      if ( arg.contains( ".qgs" ) )
      {
        myProjectFileName = arg;
        break;
      }
    }
  }

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
#ifdef Q_WS_WIN
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

  int retval = myApp.exec();
  delete qgis;
  return retval;
}
