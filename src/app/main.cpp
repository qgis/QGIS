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
#include <QSplashScreen>
#include <QString>
#include <QStringList>
#include <QStyle>
#include <QStyleFactory>
#include <QDesktopWidget>
#include <QImageReader>
#include <QMessageBox>
#include <QStandardPaths>

#include <cstdio>
#include <cstdlib>
#include <cstdarg>
#include "qgsconfig.h"

#if !defined(Q_OS_WIN)
#include "sigwatch.h"
#endif

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

#ifdef Q_OS_UNIX
// For getrlimit() / setrlimit()
#include <sys/resource.h>
#include <sys/time.h>
#endif

#ifdef HAVE_CRASH_HANDLER
#if defined(__GLIBC__) || defined(__FreeBSD__)
#define QGIS_CRASH
#include <unistd.h>
#include <execinfo.h>
#include <csignal>
#include <sys/wait.h>
#include <cerrno>
#endif
#endif

#include "qgscustomization.h"
#include "qgssettings.h"
#include "qgsfontutils.h"
#include "qgspluginregistry.h"
#include "qgsmessagelog.h"
#include "qgspythonrunner.h"
#include "qgslocalec.h"
#include "qgisapp.h"
#include "qgsmapcanvas.h"
#include "qgsapplication.h"
#include "qgsconfig.h"
#include "qgsversion.h"
#include "qgsexception.h"
#include "qgsproject.h"
#include "qgsrectangle.h"
#include "qgslogger.h"
#include "qgsdxfexport.h"
#include "qgsmapthemes.h"
#include "qgsvectorlayer.h"
#include "qgis_app.h"
#ifdef HAVE_CRASH_HANDLER
#include "qgscrashhandler.h"
#endif
#include "qgsziputils.h"
#include "qgsversionmigration.h"
#include "qgsfirstrundialog.h"
#include "qgsproxystyle.h"
#include "qgsmessagebar.h"

#include "qgsuserprofilemanager.h"
#include "qgsuserprofile.h"

#ifdef HAVE_OPENCL
#include "qgsopenclutils.h"
#endif

/**
 * Print QGIS version
 */
void version( )
{
  const QString msg = QStringLiteral( "QGIS %1 '%2' (%3)\n" ).arg( VERSION ).arg( RELEASE_NAME ).arg( QGSVERSION );
  std::cout << msg.toStdString();
}

/**
 * Print usage text
 */
void usage( const QString &appName )
{
  QStringList msg;

  msg
      << QStringLiteral( "QGIS is a user friendly Open Source Geographic Information System.\n" )
      << QStringLiteral( "Usage: " ) << appName <<  QStringLiteral( " [OPTION] [FILE]\n" )
      << QStringLiteral( "  OPTION:\n" )
      << QStringLiteral( "\t[-v, --version]\tdisplay version information and exit\n" )
      << QStringLiteral( "\t[-s, --snapshot filename]\temit snapshot of loaded datasets to given file\n" )
      << QStringLiteral( "\t[-w, --width width]\twidth of snapshot to emit\n" )
      << QStringLiteral( "\t[-h, --height height]\theight of snapshot to emit\n" )
      << QStringLiteral( "\t[-l, --lang language]\tuse language for interface text (changes existing override)\n" )
      << QStringLiteral( "\t[-p, --project projectfile]\tload the given QGIS project\n" )
      << QStringLiteral( "\t[-e, --extent xmin,ymin,xmax,ymax]\tset initial map extent\n" )
      << QStringLiteral( "\t[-n, --nologo]\thide splash screen\n" )
      << QStringLiteral( "\t[-V, --noversioncheck]\tdon't check for new version of QGIS at startup\n" )
      << QStringLiteral( "\t[-P, --noplugins]\tdon't restore plugins on startup\n" )
      << QStringLiteral( "\t[-B, --skipbadlayers]\tdon't prompt for missing layers\n" )
      << QStringLiteral( "\t[-C, --nocustomization]\tdon't apply GUI customization\n" )
      << QStringLiteral( "\t[-z, --customizationfile path]\tuse the given ini file as GUI customization\n" )
      << QStringLiteral( "\t[-g, --globalsettingsfile path]\tuse the given ini file as Global Settings (defaults)\n" )
      << QStringLiteral( "\t[-a, --authdbdirectory path] use the given directory for authentication database\n" )
      << QStringLiteral( "\t[-f, --code path]\trun the given python file on load\n" )
      << QStringLiteral( "\t[-d, --defaultui]\tstart by resetting user ui settings to default\n" )
      << QStringLiteral( "\t[--hide-browser]\thide the browser widget\n" )
      << QStringLiteral( "\t[--dxf-export filename.dxf]\temit dxf output of loaded datasets to given file\n" )
      << QStringLiteral( "\t[--dxf-extent xmin,ymin,xmax,ymax]\tset extent to export to dxf\n" )
      << QStringLiteral( "\t[--dxf-symbology-mode none|symbollayer|feature]\tsymbology mode for dxf output\n" )
      << QStringLiteral( "\t[--dxf-scale-denom scale]\tscale for dxf output\n" )
      << QStringLiteral( "\t[--dxf-encoding encoding]\tencoding to use for dxf output\n" )
      << QStringLiteral( "\t[--dxf-map-theme maptheme]\tmap theme to use for dxf output\n" )
      << QStringLiteral( "\t[--take-screenshots output_path]\ttake screen shots for the user documentation\n" )
      << QStringLiteral( "\t[--screenshots-categories categories]\tspecify the categories of screenshot to be used (see QgsAppScreenShots::Categories).\n" )
      << QStringLiteral( "\t[--profile name]\tload a named profile from the users profiles folder.\n" )
      << QStringLiteral( "\t[-s, --profiles-path path]\tpath to store user profile folders. Will create profiles inside a {path}\\profiles folder \n" )
      << QStringLiteral( "\t[--version-migration]\tforce the settings migration from older version if found\n" )
#ifdef HAVE_OPENCL
      << QStringLiteral( "\t[--openclprogramfolder]\t\tpath to the folder containing the sources for OpenCL programs.\n" )
#endif
      << QStringLiteral( "\t[--help]\t\tthis text\n" )
      << QStringLiteral( "\t[--]\t\ttreat all following arguments as FILEs\n\n" )
      << QStringLiteral( "  FILE:\n" )
      << QStringLiteral( "    Files specified on the command line can include rasters, vectors,\n" )
      << QStringLiteral( "    QGIS layer definition files (.qlr) and QGIS project files (.qgs and .qgz): \n" )
      << QStringLiteral( "     1. Rasters - supported formats include GeoTiff, DEM \n" )
      << QStringLiteral( "        and others supported by GDAL\n" )
      << QStringLiteral( "     2. Vectors - supported formats include ESRI Shapefiles\n" )
      << QStringLiteral( "        and others supported by OGR and PostgreSQL layers using\n" )
      << QStringLiteral( "        the PostGIS extension\n" )  ; // OK

#ifdef Q_OS_WIN
  MessageBox( nullptr,
              msg.join( QString() ).toLocal8Bit().constData(),
              "QGIS command line options",
              MB_OK );
#else
  std::cout << msg.join( QString() ).toLocal8Bit().constData();
#endif

} // usage()


/////////////////////////////////////////////////////////////////
// Command line options 'behavior' flag setup
////////////////////////////////////////////////////////////////

// These two are global so that they can be set by the OpenDocuments
// AppleEvent handler as well as by the main routine argv processing

// This behavior will cause QGIS to autoload a project
static QString sProjectFileName;

// This is the 'leftover' arguments collection
static QStringList sFileList;


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

#ifdef QGIS_CRASH
  // Below there is a bunch of operations that are not safe in multi-threaded
  // environment (dup()+close() combo, wait(), juggling with file descriptors).
  // Maybe some problems could be resolved with dup2() and waitpid(), but it seems
  // that if the operations on descriptors are not serialized, things will get nasty.
  // That's why there's this lovely mutex here...
  static QMutex sMutex;
  QMutexLocker locker( &sMutex );

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
        QgsDebugMsg( QStringLiteral( "dup to stdin failed" ) );
      }

      close( fd[1] );        // close writing end
      execl( "/usr/bin/c++filt", "c++filt", static_cast< char * >( nullptr ) );
      perror( "could not start c++filt" );
      exit( 1 );
    }

    myPrint( "Stacktrace (piped through c++filt):\n" );
    stderr_fd = dup( STDERR_FILENO );
    close( fd[0] );          // close reading end
    close( STDERR_FILENO );  // close stderr

    // stderr to pipe
    int stderr_new = dup( fd[1] );
    if ( stderr_new != STDERR_FILENO )
    {
      if ( stderr_new >= 0 )
        close( stderr_new );
      QgsDebugMsg( QStringLiteral( "dup to stderr failed" ) );
    }

    close( fd[1] );  // close duped pipe
  }

  void **buffer = new void *[ depth ];
  int nptrs = backtrace( buffer, depth );
  backtrace_symbols_fd( buffer, nptrs, STDERR_FILENO );
  delete [] buffer;
  if ( stderr_fd >= 0 )
  {
    int status;
    close( STDERR_FILENO );
    int dup_stderr = dup( stderr_fd );
    if ( dup_stderr != STDERR_FILENO )
    {
      close( dup_stderr );
      QgsDebugMsg( QStringLiteral( "dup to stderr failed" ) );
    }
    close( stderr_fd );
    wait( &status );
  }
#elif defined(Q_OS_WIN)
  // TODO Replace with incoming QgsStackTrace
#else
  Q_UNUSED( depth )
#endif
}

#ifdef QGIS_CRASH
void qgisCrash( int signal )
{
  fprintf( stderr, "QGIS died on signal %d", signal );

  QgsCrashHandler::handle( 0 );

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
      myPrint( "Could not read link (%d: %s)\n", errno, strerror( errno ) );
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
        myPrint( "Cannot fork (%d: %s)\n", errno, strerror( errno ) );
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
void myMessageOutput( QtMsgType type, const QMessageLogContext &, const QString &msg )
{
  switch ( type )
  {
    case QtDebugMsg:
      myPrint( "%s\n", msg.toLocal8Bit().constData() );
      if ( msg.startsWith( QLatin1String( "Backtrace" ) ) )
      {
        const QString trace = msg.mid( 9 );
        dumpBacktrace( atoi( trace.toLocal8Bit().constData() ) );
      }
      break;
    case QtCriticalMsg:
      myPrint( "Critical: %s\n", msg.toLocal8Bit().constData() );

#ifdef QGISDEBUG
      dumpBacktrace( 20 );
#endif

      break;
    case QtWarningMsg:
    {
      /* Ignore:
       * - libpng iCPP known incorrect SRGB profile errors (which are thrown by 3rd party components
       *  we have no control over and have low value anyway);
       * - QtSVG warnings with regards to lack of implementation beyond Tiny SVG 1.2
       */
      if ( msg.contains( QLatin1String( "QXcbClipboard" ), Qt::CaseInsensitive ) ||
           msg.startsWith( QLatin1String( "libpng warning: iCCP: known incorrect sRGB profile" ), Qt::CaseInsensitive ) ||
           msg.contains( QLatin1String( "Could not add child element to parent element because the types are incorrect" ), Qt::CaseInsensitive ) ||
           msg.contains( QLatin1String( "OpenType support missing for" ), Qt::CaseInsensitive ) )
        break;

      myPrint( "Warning: %s\n", msg.toLocal8Bit().constData() );

#ifdef QGISDEBUG
      // Print all warnings except setNamedColor.
      // Only seems to happen on windows
      if ( !msg.startsWith( QLatin1String( "QColor::setNamedColor: Unknown color name 'param" ), Qt::CaseInsensitive )
           && !msg.startsWith( QLatin1String( "Trying to create a QVariant instance of QMetaType::Void type, an invalid QVariant will be constructed instead" ), Qt::CaseInsensitive )
           && !msg.startsWith( QLatin1String( "Logged warning" ), Qt::CaseInsensitive ) )
      {
        // TODO: Verify this code in action.
        dumpBacktrace( 20 );

        // also be super obnoxious -- we DON'T want to allow these errors to be ignored!!
        if ( QgisApp::instance() && QgisApp::instance()->messageBar() && QgisApp::instance()->thread() == QThread::currentThread() )
        {
          QgisApp::instance()->messageBar()->pushCritical( QStringLiteral( "Qt" ), msg );
        }
        else
        {
          QgsMessageLog::logMessage( msg, QStringLiteral( "Qt" ) );
        }
      }
#endif

      // TODO: Verify this code in action.
      if ( msg.startsWith( QLatin1String( "libpng error:" ), Qt::CaseInsensitive ) )
      {
        // Let the user know
        QgsMessageLog::logMessage( msg, QStringLiteral( "libpng" ) );
      }

      break;
    }

    case QtFatalMsg:
    {
      myPrint( "Fatal: %s\n", msg.toLocal8Bit().constData() );
#ifdef QGIS_CRASH
      qgisCrash( -1 );
#else
      dumpBacktrace( 256 );
      abort();                    // deliberately dump core
#endif
      break; // silence warnings
    }

    case QtInfoMsg:
      myPrint( "Info: %s\n", msg.toLocal8Bit().constData() );
      break;
  }
}

#ifdef _MSC_VER
#undef APP_EXPORT
#define APP_EXPORT __declspec(dllexport)
#endif

#if defined(ANDROID) || defined(Q_OS_WIN)
// On Android, there there is a libqgis.so instead of a qgis executable.
// The main method symbol of this library needs to be exported so it can be called by java
// On Windows this main is included in qgis_app and called from mainwin.cpp
APP_EXPORT
#endif
int main( int argc, char *argv[] )
{
  //log messages written before creating QgsApplication
  QStringList preApplicationLogMessages;

#ifdef Q_OS_UNIX
  // Increase file resource limits (i.e., number of allowed open files)
  // (from code provided by Larry Biehl, Purdue University, USA, from 'MultiSpec' project)
  // This is generally 256 for the soft limit on Mac
  // NOTE: setrlimit() must come *before* initialization of stdio strings,
  //       e.g. before any debug messages, or setrlimit() gets ignored
  // see: http://stackoverflow.com/a/17726104/2865523
  struct rlimit rescLimit;
  if ( getrlimit( RLIMIT_NOFILE, &rescLimit ) == 0 )
  {
    const rlim_t oldSoft( rescLimit.rlim_cur );
#ifdef OPEN_MAX
    rlim_t newSoft( OPEN_MAX );
#else
    rlim_t newSoft( 4096 );
#endif
    const char *qgisMaxFileCount = getenv( "QGIS_MAX_FILE_COUNT" );
    if ( qgisMaxFileCount )
      newSoft = static_cast<rlim_t>( atoi( qgisMaxFileCount ) );
    if ( rescLimit.rlim_cur < newSoft || qgisMaxFileCount )
    {
      rescLimit.rlim_cur = std::min( newSoft, rescLimit.rlim_max );

      if ( setrlimit( RLIMIT_NOFILE, &rescLimit ) == 0 )
      {
        QgsDebugMsg( QStringLiteral( "RLIMIT_NOFILE Soft NEW: %1 / %2" )
                     .arg( rescLimit.rlim_cur ).arg( rescLimit.rlim_max ) );
      }
    }
    Q_UNUSED( oldSoft ) //avoid warnings
    QgsDebugMsg( QStringLiteral( "RLIMIT_NOFILE Soft/Hard ORIG: %1 / %2" )
                 .arg( oldSoft ).arg( rescLimit.rlim_max ) );
  }
#endif

  QgsDebugMsg( QStringLiteral( "Starting qgis main" ) );
#ifdef WIN32  // Windows
#ifdef _MSC_VER
  _set_fmode( _O_BINARY );
#else //MinGW
  _fmode = _O_BINARY;
#endif  // _MSC_VER
#endif  // WIN32

  // Set up the custom qWarning/qDebug custom handler
#ifndef ANDROID
  qInstallMessageHandler( myMessageOutput );
#endif

#ifdef QGIS_CRASH
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

#ifdef _MSC_VER
  SetUnhandledExceptionFilter( QgsCrashHandler::handle );
#endif

  // initialize random number seed
  qsrand( time( nullptr ) );

  /////////////////////////////////////////////////////////////////
  // Command line options 'behavior' flag setup
  ////////////////////////////////////////////////////////////////

  //
  // Parse the command line arguments, looking to see if the user has asked for any
  // special behaviors. Any remaining non command arguments will be kept aside to
  // be passed as a list of layers and / or a project that should be loaded.
  //

  // This behavior is used to load the app, snapshot the map,
  // save the image to disk and then exit
  QString mySnapshotFileName;
  QString configLocalStorageLocation;
  QString profileName;
  int mySnapshotWidth = 800;
  int mySnapshotHeight = 600;

  bool myHideSplash = false;
  bool settingsMigrationForce = false;
  bool mySkipVersionCheck = false;
  bool hideBrowser = false;
#if defined(ANDROID)
  QgsDebugMsg( QStringLiteral( "Android: Splash hidden" ) );
  myHideSplash = true;
#endif

  bool myRestoreDefaultWindowState = false;
  bool myRestorePlugins = true;
  bool mySkipBadLayers = false;
  bool myCustomization = true;

  QString dxfOutputFile;
  QgsDxfExport::SymbologyExport dxfSymbologyMode = QgsDxfExport::SymbolLayerSymbology;
  double dxfScale = 50000.0;
  QString dxfEncoding = QStringLiteral( "CP1252" );
  QString dxfMapTheme;
  QgsRectangle dxfExtent;

  bool takeScreenShots = false;
  QString screenShotsPath;
  int screenShotsCategories = 0;

  // This behavior will set initial extent of map canvas, but only if
  // there are no command line arguments. This gives a usable map
  // extent when qgis starts with no layers loaded. When layers are
  // loaded, we let the layers define the initial extent.
  QString myInitialExtent;
  if ( argc == 1 )
    myInitialExtent = QStringLiteral( "-1,-1,1,1" );

  // This behavior will allow you to force the use of a translation file
  // which is useful for testing
  QString translationCode;

  // The user can specify a path which will override the default path of custom
  // user settings (~/.qgis) and it will be used for QgsSettings INI file
  QString authdbdirectory;

  QString pythonfile;

  QString customizationfile;
  QString globalsettingsfile;

#ifdef HAVE_OPENCL
  QString openClProgramFolder;
#endif

// TODO Fix android
#if defined(ANDROID)
  QgsDebugMsg( QStringLiteral( "Android: All params stripped" ) );// Param %1" ).arg( argv[0] ) );
  //put all QGIS settings in the same place
  QString configpath = QgsApplication::qgisSettingsDirPath();
  QgsDebugMsg( QStringLiteral( "Android: configpath set to %1" ).arg( configpath ) );
#endif

  QStringList args;

  {
    QCoreApplication coreApp( argc, argv );
    ( void ) QgsApplication::resolvePkgPath(); // trigger storing of application path in QgsApplication

    if ( !bundleclicked( argc, argv ) )
    {
      // Build a local QCoreApplication from arguments. This way, arguments are correctly parsed from their native locale
      // It will use QString::fromLocal8Bit( argv ) under Unix and GetCommandLine() under Windows.
      args = QCoreApplication::arguments();

      for ( int i = 1; i < args.size(); ++i )
      {
        const QString &arg = args[i];

        if ( arg == QLatin1String( "--help" ) || arg == QLatin1String( "-?" ) )
        {
          usage( args[0] );
          return EXIT_SUCCESS;
        }
        else if ( arg == QLatin1String( "--version" ) || arg == QLatin1String( "-v" ) )
        {
          version();
          return EXIT_SUCCESS;
        }
        else if ( arg == QLatin1String( "--nologo" ) || arg == QLatin1String( "-n" ) )
        {
          myHideSplash = true;
        }
        else if ( arg == QLatin1String( "--version-migration" ) )
        {
          settingsMigrationForce = true;
        }
        else if ( arg == QLatin1String( "--noversioncheck" ) || arg == QLatin1String( "-V" ) )
        {
          mySkipVersionCheck = true;
        }
        else if ( arg == QLatin1String( "--noplugins" ) || arg == QLatin1String( "-P" ) )
        {
          myRestorePlugins = false;
        }
        else if ( arg == QLatin1String( "--skipbadlayers" ) || arg == QLatin1String( "-B" ) )
        {
          QgsDebugMsg( QStringLiteral( "Skipping bad layers" ) );
          mySkipBadLayers = true;
        }
        else if ( arg == QLatin1String( "--nocustomization" ) || arg == QLatin1String( "-C" ) )
        {
          myCustomization = false;
        }
        else if ( i + 1 < argc && ( arg == QLatin1String( "--profile" ) ) )
        {
          profileName = args[++i];
        }
        else if ( i + 1 < argc && ( arg == QLatin1String( "--profiles-path" ) || arg == QLatin1String( "-S" ) ) )
        {
          configLocalStorageLocation = QDir::toNativeSeparators( QFileInfo( args[++i] ).absoluteFilePath() );
        }
        else if ( i + 1 < argc && ( arg == QLatin1String( "--snapshot" ) || arg == QLatin1String( "-s" ) ) )
        {
          mySnapshotFileName = QDir::toNativeSeparators( QFileInfo( args[++i] ).absoluteFilePath() );
        }
        else if ( i + 1 < argc && ( arg == QLatin1String( "--width" ) || arg == QLatin1String( "-w" ) ) )
        {
          mySnapshotWidth = QString( args[++i] ).toInt();
        }
        else if ( arg == QLatin1String( "--hide-browser" ) )
        {
          hideBrowser = true;
        }
        else if ( i + 1 < argc && ( arg == QLatin1String( "--height" ) || arg == QLatin1String( "-h" ) ) )
        {
          mySnapshotHeight = QString( args[++i] ).toInt();
        }
        else if ( i + 1 < argc && ( arg == QLatin1String( "--lang" ) || arg == QLatin1String( "-l" ) ) )
        {
          translationCode = args[++i];
        }
        else if ( i + 1 < argc && ( arg == QLatin1String( "--project" ) || arg == QLatin1String( "-p" ) ) )
        {
          const QString projectUri { args[++i] };
          const QFileInfo projectFileInfo { projectUri };
          if ( projectFileInfo.isFile() )
          {
            sProjectFileName = QDir::toNativeSeparators( projectFileInfo.absoluteFilePath() );
          }
          else
          {
            sProjectFileName = projectUri;
          }
        }
        else if ( i + 1 < argc && ( arg == QLatin1String( "--extent" ) || arg == QLatin1String( "-e" ) ) )
        {
          myInitialExtent = args[++i];
        }
        else if ( i + 1 < argc && ( arg == QLatin1String( "--authdbdirectory" ) || arg == QLatin1String( "-a" ) ) )
        {
          authdbdirectory = QDir::toNativeSeparators( QDir( args[++i] ).absolutePath() );
        }
        else if ( i + 1 < argc && ( arg == QLatin1String( "--code" ) || arg == QLatin1String( "-f" ) ) )
        {
          pythonfile = QDir::toNativeSeparators( QFileInfo( args[++i] ).absoluteFilePath() );
        }
        else if ( i + 1 < argc && ( arg == QLatin1String( "--customizationfile" ) || arg == QLatin1String( "-z" ) ) )
        {
          customizationfile = QDir::toNativeSeparators( QFileInfo( args[++i] ).absoluteFilePath() );
        }
        else if ( i + 1 < argc && ( arg == QLatin1String( "--globalsettingsfile" ) || arg == QLatin1String( "-g" ) ) )
        {
          globalsettingsfile = QDir::toNativeSeparators( QFileInfo( args[++i] ).absoluteFilePath() );
        }
        else if ( arg == QLatin1String( "--defaultui" ) || arg == QLatin1String( "-d" ) )
        {
          myRestoreDefaultWindowState = true;
        }
        else if ( arg == QLatin1String( "--dxf-export" ) )
        {
          dxfOutputFile = args[++i];
        }
        else if ( arg == QLatin1String( "--dxf-extent" ) )
        {
          QgsLocaleNumC l;
          QString ext( args[++i] );
          QStringList coords( ext.split( ',' ) );

          if ( coords.size() != 4 )
          {
            std::cerr << "invalid dxf extent " << ext.toStdString() << std::endl;
            return 2;
          }

          for ( int i = 0; i < 4; i++ )
          {
            bool ok;
            double d;

            d = coords[i].toDouble( &ok );
            if ( !ok )
            {
              std::cerr << "invalid dxf coordinate " << coords[i].toStdString() << " in extent " << ext.toStdString() << std::endl;
              return 2;
            }

            switch ( i )
            {
              case 0:
                dxfExtent.setXMinimum( d );
                break;
              case 1:
                dxfExtent.setYMinimum( d );
                break;
              case 2:
                dxfExtent.setXMaximum( d );
                break;
              case 3:
                dxfExtent.setYMaximum( d );
                break;
            }
          }
        }
        else if ( arg == QLatin1String( "--dxf-symbology-mode" ) )
        {
          QString mode( args[++i] );
          if ( mode == QLatin1String( "none" ) )
          {
            dxfSymbologyMode = QgsDxfExport::NoSymbology;
          }
          else if ( mode == QLatin1String( "symbollayer" ) )
          {
            dxfSymbologyMode = QgsDxfExport::SymbolLayerSymbology;
          }
          else if ( mode == QLatin1String( "feature" ) )
          {
            dxfSymbologyMode = QgsDxfExport::FeatureSymbology;
          }
          else
          {
            std::cerr << "invalid dxf symbology mode " << mode.toStdString() << std::endl;
            return 2;
          }
        }
        else if ( arg == QLatin1String( "--dxf-scale-denom" ) )
        {
          bool ok;
          QString scale( args[++i] );
          dxfScale = scale.toDouble( &ok );
          if ( !ok )
          {
            std::cerr << "invalid dxf scale " << scale.toStdString() << std::endl;
            return 2;
          }
        }
        else if ( arg == QLatin1String( "--dxf-encoding" ) )
        {
          dxfEncoding = args[++i];
        }
        else if ( arg == QLatin1String( "--dxf-map-theme" ) )
        {
          dxfMapTheme = args[++i];
        }
        else if ( arg == QLatin1String( "--take-screenshots" ) )
        {
          takeScreenShots = true;
          screenShotsPath = args[++i];
        }
        else if ( arg == QLatin1String( "--screenshots-categories" ) )
        {
          screenShotsCategories = args[++i].toInt();
        }
#ifdef HAVE_OPENCL
        else if ( arg == QLatin1String( "--openclprogramfolder" ) )
        {
          openClProgramFolder = args[++i];
        }
#endif
        else if ( arg == QLatin1String( "--" ) )
        {
          for ( i++; i < args.size(); ++i )
            sFileList.append( QDir::toNativeSeparators( QFileInfo( args[i] ).absoluteFilePath() ) );
        }
        else
        {
          sFileList.append( QDir::toNativeSeparators( QFileInfo( args[i] ).absoluteFilePath() ) );
        }
      }
    }
  }

  /////////////////////////////////////////////////////////////////////
  // If no --project was specified, parse the args to look for a     //
  // .qgs file and set myProjectFileName to it. This allows loading  //
  // of a project file by clicking on it in various desktop managers //
  // where an appropriate mime-type has been set up.                 //
  /////////////////////////////////////////////////////////////////////
  if ( sProjectFileName.isEmpty() )
  {
    // check for a .qgs/z
    for ( int i = 0; i < args.size(); i++ )
    {
      QString arg = QDir::toNativeSeparators( QFileInfo( args[i] ).absoluteFilePath() );
      if ( arg.endsWith( QLatin1String( ".qgs" ), Qt::CaseInsensitive ) ||
           arg.endsWith( QLatin1String( ".qgz" ), Qt::CaseInsensitive ) )
      {
        sProjectFileName = arg;
        break;
      }
    }
  }

  /////////////////////////////////////////////////////////////////////
  // Now we have the handlers for the different behaviors...
  /////////////////////////////////////////////////////////////////////

  /////////////////////////////////////////////////////////////////////
  // Initialize the application and the translation stuff
  /////////////////////////////////////////////////////////////////////

#if defined(Q_OS_UNIX) && !defined(Q_OS_MAC) && !defined(ANDROID)
  bool myUseGuiFlag = nullptr != getenv( "DISPLAY" );
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
    exit( 1 ); //exit for now until a version of qgis is capable of running non interactive
  }

  // GUI customization is enabled according to settings (loaded when instance is created)
  // we force disabled here if --nocustomization argument is used
  if ( !myCustomization )
  {
    QgsCustomization::instance()->setEnabled( false );
  }

  QCoreApplication::setOrganizationName( QgsApplication::QGIS_ORGANIZATION_NAME );
  QCoreApplication::setOrganizationDomain( QgsApplication::QGIS_ORGANIZATION_DOMAIN );
  QCoreApplication::setApplicationName( QgsApplication::QGIS_APPLICATION_NAME );
  QCoreApplication::setAttribute( Qt::AA_DontShowIconsInMenus, false );
  QCoreApplication::setAttribute( Qt::AA_DisableWindowContextHelpButton, true );

  // Set up an OpenGL Context to be shared between threads beforehand
  // for plugins that depend on Qt WebEngine module.
  // As suggested by Qt documentation at:
  //   - https://doc.qt.io/qt-5/qtwebengine.html
  //   - https://code.qt.io/cgit/qt/qtwebengine.git/plain/src/webenginewidgets/api/qtwebenginewidgetsglobal.cpp
#if defined(QT_OS_WIN) && !defined(QT_NO_OPENGL)
  QCoreApplication::setAttribute( Qt::AA_ShareOpenGLContexts, true );
#endif

  // Set up the QgsSettings Global Settings:
  // - use the path specified with --globalsettingsfile path,
  // - use the environment if not found
  // - use the AppDataLocation ($HOME/.local/share/QGIS/QGIS3 on Linux, roaming path on Windows)
  // - use a default location as a fallback
  if ( globalsettingsfile.isEmpty() )
  {
    globalsettingsfile = getenv( "QGIS_GLOBAL_SETTINGS_FILE" );
  }

  if ( globalsettingsfile.isEmpty() )
  {
    QStringList startupPaths = QStandardPaths::locateAll( QStandardPaths::AppDataLocation, QStringLiteral( "qgis_global_settings.ini" ) );
    if ( !startupPaths.isEmpty() )
    {
      globalsettingsfile = startupPaths.at( 0 );
    }
  }

  if ( globalsettingsfile.isEmpty() )
  {
    QString default_globalsettingsfile = QgsApplication::resolvePkgPath() + "/resources/qgis_global_settings.ini";
    if ( QFile::exists( default_globalsettingsfile ) )
    {
      globalsettingsfile = default_globalsettingsfile;
    }
  }

  if ( !globalsettingsfile.isEmpty() )
  {
    if ( !QgsSettings::setGlobalSettingsPath( globalsettingsfile ) )
    {
      preApplicationLogMessages << QObject::tr( "Invalid globalsettingsfile path: %1" ).arg( globalsettingsfile ), QStringLiteral( "QGIS" );
    }
    else
    {
      preApplicationLogMessages << QObject::tr( "Successfully loaded globalsettingsfile path: %1" ).arg( globalsettingsfile ), QStringLiteral( "QGIS" );
    }
  }

  if ( configLocalStorageLocation.isEmpty() )
  {
    QSettings globalSettings( globalsettingsfile, QSettings::IniFormat );
    if ( getenv( "QGIS_CUSTOM_CONFIG_PATH" ) )
    {
      configLocalStorageLocation = getenv( "QGIS_CUSTOM_CONFIG_PATH" );
    }
    else if ( globalSettings.contains( QStringLiteral( "core/profilesPath" ) ) )
    {
      configLocalStorageLocation = globalSettings.value( QStringLiteral( "core/profilesPath" ), "" ).toString();
      QgsDebugMsg( QStringLiteral( "Loading profiles path from global config at %1" ).arg( configLocalStorageLocation ) );
    }

    // If it is still empty at this point we get it from the standard location.
    if ( configLocalStorageLocation.isEmpty() )
    {
      configLocalStorageLocation = QStandardPaths::standardLocations( QStandardPaths::AppDataLocation ).value( 0 );
    }
  }

  QString rootProfileFolder = QgsUserProfileManager::resolveProfilesFolder( configLocalStorageLocation );
  QgsUserProfileManager manager( rootProfileFolder );
  QgsUserProfile *profile = manager.getProfile( profileName, true );
  QString profileFolder = profile->folder();
  profileName = profile->name();
  delete profile;

  {
    /* Translation file for QGIS.
    */
    QString myUserTranslation = QgsApplication::settingsLocaleUserLocale.value();
    QString myGlobalLocale = QgsApplication::settingsLocaleGlobalLocale.value();
    bool myShowGroupSeparatorFlag = false; // Default to false
    bool myLocaleOverrideFlag = QgsApplication::settingsLocaleOverrideFlag.value();

    // Override Show Group Separator if the global override flag is set
    if ( myLocaleOverrideFlag )
    {
      // Default to false again
      myShowGroupSeparatorFlag = QgsApplication::settingsLocaleShowGroupSeparator.value();
    }

    //
    // Priority of translation is:
    //  - command line
    //  - user specified in options dialog (with group checked on)
    //  - system locale
    //
    //  When specifying from the command line it will change the user
    //  specified user locale
    //
    if ( !translationCode.isNull() && !translationCode.isEmpty() )
    {
      QgsApplication::settingsLocaleUserLocale.setValue( translationCode );
    }
    else
    {
      if ( !myLocaleOverrideFlag || myUserTranslation.isEmpty() )
      {
        translationCode = QLocale().name();
        //setting the locale/userLocale when the --lang= option is not set will allow third party
        //plugins to always use the same locale as the QGIS, otherwise they can be out of sync
        QgsApplication::settingsLocaleUserLocale.setValue( translationCode );
      }
      else
      {
        translationCode = myUserTranslation;
      }
    }

    // Global locale settings
    if ( myLocaleOverrideFlag && ! myGlobalLocale.isEmpty( ) )
    {
      QLocale currentLocale( myGlobalLocale );
      QLocale::setDefault( currentLocale );
    }

    // Number settings
    QLocale currentLocale;
    if ( myShowGroupSeparatorFlag )
    {
      currentLocale.setNumberOptions( currentLocale.numberOptions() &= ~QLocale::NumberOption::OmitGroupSeparator );
    }
    else
    {
      currentLocale.setNumberOptions( currentLocale.numberOptions() |= QLocale::NumberOption::OmitGroupSeparator );
    }
    QLocale::setDefault( currentLocale );

    QgsApplication::setTranslation( translationCode );
  }

  QgsApplication myApp( argc, argv, myUseGuiFlag, QString(), QStringLiteral( "desktop" ) );

  // Set locale to emit QgsApplication's localeChanged signal
  QgsApplication::setLocale( QLocale() );

  //write the log messages written before creating QgsApplication
  for ( const QString &preApplicationLogMessage : std::as_const( preApplicationLogMessages ) )
    QgsMessageLog::logMessage( preApplicationLogMessage );

  // Settings migration is only supported on the default profile for now.
  if ( profileName == QLatin1String( "default" ) )
  {
    // Note: this flag is ka version number so that we can reset it once we change the version.
    // Note2: Is this a good idea can we do it better.
    // Note3: Updated to only show if we have a migration from QGIS 2 - see https://github.com/qgis/QGIS/pull/38616
    QString path = QSettings( "QGIS", "QGIS2" ).fileName() ;
    if ( QFile::exists( path ) )
    {
      QgsSettings migSettings;
      int firstRunVersion = migSettings.value( QStringLiteral( "migration/firstRunVersionFlag" ), 0 ).toInt();
      bool showWelcome = ( firstRunVersion == 0  || Qgis::versionInt() > firstRunVersion );
      std::unique_ptr< QgsVersionMigration > migration( QgsVersionMigration::canMigrate( 20000, Qgis::versionInt() ) );
      if ( migration && ( settingsMigrationForce || migration->requiresMigration() ) )
      {
        bool runMigration = true;
        if ( !settingsMigrationForce && showWelcome )
        {
          QgsFirstRunDialog dlg;
          dlg.exec();
          runMigration = dlg.migrateSettings();
          migSettings.setValue( QStringLiteral( "migration/firstRunVersionFlag" ), Qgis::versionInt() );
        }

        if ( runMigration )
        {
          QgsDebugMsg( QStringLiteral( "RUNNING MIGRATION" ) );
          migration->runMigration();
        }
      }
    }
  }

  // We can't use QgsSettings until this point because the format and
  // folder isn't set until profile is fetch.
  // Should be cleaned up in future to make this cleaner.
  QgsSettings settings;

  QgsDebugMsgLevel( QStringLiteral( "User profile details:" ), 2 );
  QgsDebugMsgLevel( QStringLiteral( "\t - %1" ).arg( profileName ), 2 );
  QgsDebugMsgLevel( QStringLiteral( "\t - %1" ).arg( profileFolder ), 2 );
  QgsDebugMsgLevel( QStringLiteral( "\t - %1" ).arg( rootProfileFolder ), 2 );

  QgsApplication::init( profileFolder );

  // Redefine QgsApplication::libraryPaths as necessary.
  // IMPORTANT: Do *after* QgsApplication myApp(...), but *before* Qt uses any plugins,
  //            e.g. loading splash screen, setting window icon, etc.
  //            Always honor QT_PLUGIN_PATH env var or qt.conf, which will
  //            be part of libraryPaths just after QgsApplication creation.
#ifdef Q_OS_WIN
  // For non static builds on win (static builds are not supported)
  // we need to be sure we can find the qt image plugins.
  QCoreApplication::addLibraryPath( QApplication::applicationDirPath()
                                    + QDir::separator() + "qtplugins" );
#endif
#if defined(Q_OS_UNIX)
  // Resulting libraryPaths has critical QGIS plugin paths first, then any Qt plugin paths, then
  // any dev-defined paths (in app's qt.conf) and/or user-defined paths (QT_PLUGIN_PATH env var).
  //
  // NOTE: Minimizes, though does not fully protect against, crashes due to dev/user-defined libs
  //       built against a different Qt/QGIS, while still allowing custom C++ plugins to load.
  QStringList libPaths( QCoreApplication::libraryPaths() );

  QgsDebugMsgLevel( QStringLiteral( "Initial macOS/UNIX QCoreApplication::libraryPaths: %1" )
                    .arg( libPaths.join( " " ) ), 4 );

  // Strip all critical paths that should always be prepended
  if ( libPaths.removeAll( QDir::cleanPath( QgsApplication::pluginPath() ) ) )
  {
    QgsDebugMsgLevel( QStringLiteral( "QgsApplication::pluginPath removed from initial libraryPaths" ), 4 );
  }
  if ( libPaths.removeAll( QCoreApplication::applicationDirPath() ) )
  {
    QgsDebugMsgLevel( QStringLiteral( "QCoreApplication::applicationDirPath removed from initial libraryPaths" ), 4 );
  }
  // Prepend path, so a standard Qt bundle directory is parsed
  QgsDebugMsgLevel( QStringLiteral( "Prepending QCoreApplication::applicationDirPath to libraryPaths" ), 4 );
  libPaths.prepend( QCoreApplication::applicationDirPath() );

  // Check if we are running in a 'release' app bundle, i.e. contains copied-in
  // standard Qt-specific plugin subdirectories (ones never created by QGIS, e.g. 'sqldrivers' is).
  // Note: bundleclicked(...) is inadequate to determine which *type* of bundle was opened, e.g. release or build dir.
  // An app bundled with QGIS_MACAPP_BUNDLE > 0 is considered a release bundle.
  QString  relLibPath( QDir::cleanPath( QCoreApplication::applicationDirPath().append( "/../PlugIns" ) ) );
  // Note: relLibPath becomes the defacto QT_PLUGINS_DIR of a release app bundle
  if ( QFile::exists( relLibPath + QStringLiteral( "/imageformats" ) ) )
  {
    // We are in a release app bundle.
    // Strip QT_PLUGINS_DIR because it will crash a launched release app bundle, since
    // the appropriate Qt frameworks and plugins have been copied into the bundle.
    if ( libPaths.removeAll( QT_PLUGINS_DIR ) )
    {
      QgsDebugMsgLevel( QStringLiteral( "QT_PLUGINS_DIR removed from initial libraryPaths" ), 4 );
    }
    // Prepend the Plugins path, so copied-in Qt plugin bundle directories are parsed.
    QgsDebugMsgLevel( QStringLiteral( "Prepending <bundle>/Plugins to libraryPaths" ), 4 );
    libPaths.prepend( relLibPath );

    // TODO: see if this or another method can be used to avoid QCA's install prefix plugins
    //       from being parsed and loaded (causes multi-Qt-loaded errors when bundled Qt should
    //       be the only one loaded). QCA core (> v2.1.3) needs an update first.
    //setenv( "QCA_PLUGIN_PATH", relLibPath.toUtf8().constData(), 1 );
  }
  else
  {
    // We are either running from build dir bundle, or launching Mach-O binary directly.  //#spellok
    // Add system Qt plugins, since they are not bundled, and not always referenced by default.
    // An app bundled with QGIS_MACAPP_BUNDLE = 0 will still have Plugins/qgis in it.
    // Note: Don't always prepend.
    //       User may have already defined it in QT_PLUGIN_PATH in a specific order.
    if ( !libPaths.contains( QT_PLUGINS_DIR ) )
    {
      QgsDebugMsgLevel( QStringLiteral( "Prepending QT_PLUGINS_DIR to libraryPaths" ), 4 );
      libPaths.prepend( QT_PLUGINS_DIR );
    }
  }

  QgsDebugMsgLevel( QStringLiteral( "Prepending QgsApplication::pluginPath to libraryPaths" ), 4 );
  libPaths.prepend( QDir::cleanPath( QgsApplication::pluginPath() ) );

  // Redefine library search paths.
  QCoreApplication::setLibraryPaths( libPaths );

  QgsDebugMsgLevel( QStringLiteral( "Rewritten macOS QCoreApplication::libraryPaths: %1" )
                    .arg( QCoreApplication::libraryPaths().join( " " ) ), 4 );
#endif

#ifdef Q_OS_MAC
  // Set hidpi icons; use SVG icons, as PNGs will be relatively too small
  QCoreApplication::setAttribute( Qt::AA_UseHighDpiPixmaps );

  // Set 1024x1024 icon for dock, app switcher, etc., rendering
  myApp.setWindowIcon( QIcon( QgsApplication::iconsPath() + QStringLiteral( "qgis-icon-macos.png" ) ) );
#else
  QgsApplication::setWindowIcon( QIcon( QgsApplication::appIconPath() ) );
#endif

  // TODO: use QgsSettings
  QSettings *customizationsettings = nullptr;

  if ( !customizationfile.isEmpty() )
  {
    // Using the customizationfile option always overrides the option and config path options.
    QgsCustomization::instance()->setEnabled( true );
  }
  else
  {
    // Use the default file location
    customizationfile = profileFolder + QDir::separator() + QStringLiteral( "QGIS" ) + QDir::separator() + QStringLiteral( "QGISCUSTOMIZATION3.ini" ) ;
  }

  customizationsettings = new QSettings( customizationfile, QSettings::IniFormat );

  // Load and set possible default customization, must be done after QgsApplication init and QgsSettings ( QCoreApplication ) init
  QgsCustomization::instance()->setSettings( customizationsettings );
  QgsCustomization::instance()->loadDefault();

#ifdef Q_OS_MACX
  if ( !getenv( "GDAL_DRIVER_PATH" ) )
  {
    // If the GDAL plugins are bundled with the application and GDAL_DRIVER_PATH
    // is not already defined, use the GDAL plugins in the application bundle.
    QString gdalPlugins( QCoreApplication::applicationDirPath().append( "/lib/gdalplugins" ) );
    if ( QFile::exists( gdalPlugins ) )
    {
      setenv( "GDAL_DRIVER_PATH", gdalPlugins.toUtf8(), 1 );
    }
  }

  // Point GDAL_DATA at any GDAL share directory embedded in the app bundle
  if ( !getenv( "GDAL_DATA" ) )
  {
    QStringList gdalShares;
    gdalShares << QCoreApplication::applicationDirPath().append( "/share/gdal" )
               << QDir::cleanPath( QgsApplication::pkgDataPath() ).append( "/share/gdal" )
               << QDir::cleanPath( QgsApplication::pkgDataPath() ).append( "/gdal" );
    const auto constGdalShares = gdalShares;
    for ( const QString &gdalShare : constGdalShares )
    {
      if ( QFile::exists( gdalShare ) )
      {
        setenv( "GDAL_DATA", gdalShare.toUtf8().constData(), 1 );
        break;
      }
    }
  }

  // Point PYTHONHOME to embedded interpreter if present in the bundle
  if ( !getenv( "PYTHONHOME" ) )
  {
    if ( QFile::exists( QCoreApplication::applicationDirPath().append( "/bin/python3" ) ) )
    {
      setenv( "PYTHONHOME", QCoreApplication::applicationDirPath().toUtf8().constData(), 1 );
    }
  }
#endif

  // custom environment variables
  QMap<QString, QString> systemEnvVars = QgsApplication::systemEnvVars();
  bool useCustomVars = settings.value( QStringLiteral( "qgis/customEnvVarsUse" ), QVariant( false ) ).toBool();
  if ( useCustomVars )
  {
    QStringList customVarsList = settings.value( QStringLiteral( "qgis/customEnvVars" ), "" ).toStringList();
    if ( !customVarsList.isEmpty() )
    {
      const auto constCustomVarsList = customVarsList;
      for ( const QString &varStr : constCustomVarsList )
      {
        int pos = varStr.indexOf( QLatin1Char( '|' ) );
        if ( pos == -1 )
          continue;
        QString envVarApply = varStr.left( pos );
        if ( envVarApply == QLatin1String( "skip" ) )
          continue;
        QString varStrNameValue = varStr.mid( pos + 1 );
        pos = varStrNameValue.indexOf( QLatin1Char( '=' ) );
        if ( pos == -1 )
          continue;
        QString envVarName = varStrNameValue.left( pos );
        QString envVarValue = varStrNameValue.mid( pos + 1 );

        if ( systemEnvVars.contains( envVarName ) )
        {
          if ( envVarApply == QLatin1String( "prepend" ) )
          {
            envVarValue += systemEnvVars.value( envVarName );
          }
          else if ( envVarApply == QLatin1String( "append" ) )
          {
            envVarValue = systemEnvVars.value( envVarName ) + envVarValue;
          }
        }

        if ( systemEnvVars.contains( envVarName ) && envVarApply == QLatin1String( "unset" ) )
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
          setenv( envVarName.toUtf8().constData(), envVarValue.toUtf8().constData(), envVarApply == QLatin1String( "undefined" ) ? 0 : 1 );
#endif
        }
      }
    }
  }

#ifdef QGISDEBUG
  QgsFontUtils::loadStandardTestFonts( QStringList() << QStringLiteral( "Roman" ) << QStringLiteral( "Bold" ) );
#endif

  // Set the application style.  If it's not set QT will use the platform style except on Windows
  // as it looks really ugly so we use QPlastiqueStyle.
  QString desiredStyle = settings.value( QStringLiteral( "qgis/style" ) ).toString();
  const QString theme = settings.value( QStringLiteral( "UI/UITheme" ) ).toString();
  if ( theme != QLatin1String( "default" ) )
  {
    if ( QStyleFactory::keys().contains( QStringLiteral( "fusion" ), Qt::CaseInsensitive ) )
    {
      desiredStyle = QStringLiteral( "fusion" );
    }
  }
  const QString activeStyleName = QApplication::style()->metaObject()->className();
  if ( desiredStyle.contains( QLatin1String( "adwaita" ), Qt::CaseInsensitive )
       || ( desiredStyle.isEmpty() && activeStyleName.contains( QLatin1String( "adwaita" ), Qt::CaseInsensitive ) ) )
  {
    //never allow Adwaita themes - the Qt variants of these are VERY broken
    //for apps like QGIS. E.g. oversized controls like spinbox widgets prevent actually showing
    //any content in these widgets, leaving a very bad impression of QGIS

    //note... we only do this if there's a known good style available (fusion), as SOME
    //style choices can cause Qt apps to crash...
    if ( QStyleFactory::keys().contains( QStringLiteral( "fusion" ), Qt::CaseInsensitive ) )
    {
      desiredStyle = QStringLiteral( "fusion" );
    }
  }
  if ( !desiredStyle.isEmpty() )
  {
    QApplication::setStyle( new QgsAppStyle( desiredStyle ) );

    if ( activeStyleName != desiredStyle )
      settings.setValue( QStringLiteral( "qgis/style" ), desiredStyle );
  }
  else
  {
    // even if user has not set a style, we need to override the application style with the QgsAppStyle proxy
    // based on the default style (or we miss custom style tweaks)
    QApplication::setStyle( new QgsAppStyle( activeStyleName ) );
  }

  // set authentication database directory
  if ( !authdbdirectory.isEmpty() )
  {
    QgsApplication::setAuthDatabaseDirPath( authdbdirectory );
  }

  //set up splash screen
  QString mySplashPath( QgsCustomization::instance()->splashPath() );
  QPixmap myPixmap( mySplashPath + QStringLiteral( "splash.png" ) );

  int w = 600 * qApp->desktop()->logicalDpiX() / 96;
  int h = 300 * qApp->desktop()->logicalDpiY() / 96;

  QSplashScreen *mypSplash = new QSplashScreen( myPixmap.scaled( w, h, Qt::KeepAspectRatio, Qt::SmoothTransformation ) );
  if ( !takeScreenShots && !myHideSplash && !settings.value( QStringLiteral( "qgis/hideSplash" ) ).toBool() )
  {
    //for win and linux we can just automask and png transparency areas will be used
    mypSplash->setMask( myPixmap.mask() );
    mypSplash->show();
  }

  // optionally restore default window state
  // use restoreDefaultWindowState setting only if NOT using command line (then it is set already)
  if ( myRestoreDefaultWindowState || settings.value( QStringLiteral( "qgis/restoreDefaultWindowState" ), false ).toBool() )
  {
    QgsDebugMsg( QStringLiteral( "Resetting /UI/state settings!" ) );
    settings.remove( QStringLiteral( "/UI/state" ) );
    settings.remove( QStringLiteral( "/qgis/restoreDefaultWindowState" ) );
  }

  if ( hideBrowser )
  {
    if ( settings.value( QStringLiteral( "/Windows/Data Source Manager/tab" ) ).toInt() == 0 )
      settings.setValue( QStringLiteral( "/Windows/Data Source Manager/tab" ), 1 );
    settings.setValue( QStringLiteral( "/UI/hidebrowser" ), true );
  }

  // set max. thread count
  // this should be done in QgsApplication::init() but it doesn't know the settings dir.
  QgsApplication::setMaxThreads( settings.value( QStringLiteral( "qgis/max_threads" ), -1 ).toInt() );

  QgisApp *qgis = new QgisApp( mypSplash, myRestorePlugins, mySkipBadLayers, mySkipVersionCheck, rootProfileFolder, profileName ); // "QgisApp" used to find canonical instance
  qgis->setObjectName( QStringLiteral( "QgisApp" ) );

  QgsApplication::connect(
    &myApp, SIGNAL( preNotify( QObject *, QEvent *, bool * ) ),
    //qgis, SLOT( preNotify( QObject *, QEvent *))
    QgsCustomization::instance(), SLOT( preNotify( QObject *, QEvent *, bool * ) )
  );

  /////////////////////////////////////////////////////////////////////
  // Load a project file if one was specified
  /////////////////////////////////////////////////////////////////////
  if ( ! sProjectFileName.isEmpty() )
  {
    // in case the project contains broken layers, interactive
    // "Handle Bad Layers" is displayed that could be blocked by splash screen
    mypSplash->hide();
    qgis->openProject( sProjectFileName );
  }

  /////////////////////////////////////////////////////////////////////
  // autoload any file names that were passed in on the command line
  /////////////////////////////////////////////////////////////////////
  for ( const QString &layerName : std::as_const( sFileList ) )
  {
    QgsDebugMsg( QStringLiteral( "Trying to load file : %1" ).arg( layerName ) );
    // don't load anything with a .qgs extension - these are project files
    if ( layerName.endsWith( QLatin1String( ".qgs" ), Qt::CaseInsensitive ) ||
         layerName.endsWith( QLatin1String( ".qgz" ), Qt::CaseInsensitive ) ||
         QgsZipUtils::isZipFile( layerName ) )
    {
      continue;
    }
    else if ( layerName.endsWith( QLatin1String( ".qlr" ), Qt::CaseInsensitive ) )
    {
      qgis->openLayerDefinition( layerName );
    }
    else
    {
      qgis->openLayer( layerName );
    }
  }


  /////////////////////////////////////////////////////////////////////
  // Set initial extent if requested
  /////////////////////////////////////////////////////////////////////
  if ( ! myInitialExtent.isEmpty() )
  {
    QgsLocaleNumC l;
    double coords[4];
    int pos, posOld = 0;
    bool ok = true;

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

#if QT_VERSION < QT_VERSION_CHECK(5, 15, 2)
      coords[i] = myInitialExtent.midRef( posOld, pos - posOld ).toDouble( &ok );
#else
      coords[i] = QStringView {myInitialExtent}.mid( posOld, pos - posOld ).toDouble( &ok );
#endif
      if ( !ok )
        break;

      posOld = pos + 1;
    }

    // parse last coordinate
    if ( ok )
    {
#if QT_VERSION < QT_VERSION_CHECK(5, 15, 2)
      coords[3] = myInitialExtent.midRef( posOld ).toDouble( &ok );
#else
      coords[3] = QStringView {myInitialExtent}.mid( posOld ).toDouble( &ok );
#endif
    }

    if ( !ok )
    {
      QgsDebugMsg( QStringLiteral( "Error while parsing initial extent!" ) );
    }
    else
    {
      // set extent from parsed values
      QgsRectangle rect( coords[0], coords[1], coords[2], coords[3] );
      qgis->setExtent( rect );
    }
  }

  if ( !pythonfile.isEmpty() )
  {
#ifdef Q_OS_WIN
    //replace backslashes with forward slashes
    pythonfile.replace( '\\', '/' );
#endif
    QgsPythonRunner::run( QStringLiteral( "with open('%1','r') as f: exec(f.read())" ).arg( pythonfile ) );
  }

  /////////////////////////////////`////////////////////////////////////
  // Take a snapshot of the map view then exit if snapshot mode requested
  /////////////////////////////////////////////////////////////////////
  if ( !mySnapshotFileName.isEmpty() )
  {
    /*You must have at least one paintEvent() delivered for the window to be
      rendered properly.

      It looks like you don't run the event loop in non-interactive mode, so the
      event is never occurring.

      To achieve this without running the event loop: show the window, then call
      qApp->processEvents(), grab the pixmap, save it, hide the window and exit.
      */
    //qgis->show();
    QgsApplication::processEvents();
    QPixmap *myQPixmap = new QPixmap( mySnapshotWidth, mySnapshotHeight );
    myQPixmap->fill();
    qgis->saveMapAsImage( mySnapshotFileName, myQPixmap );
    QgsApplication::processEvents();
    qgis->hide();

    return 1;
  }

  if ( !dxfOutputFile.isEmpty() )
  {
    qgis->hide();

    QgsDxfExport dxfExport;
    dxfExport.setSymbologyScale( dxfScale );
    dxfExport.setSymbologyExport( dxfSymbologyMode );
    dxfExport.setExtent( dxfExtent );

    QStringList layerIds;
    QList< QgsDxfExport::DxfLayer > layers;
    if ( !dxfMapTheme.isEmpty() )
    {
      const auto constMapThemeVisibleLayers = QgsProject::instance()->mapThemeCollection()->mapThemeVisibleLayers( dxfMapTheme );
      for ( QgsMapLayer *layer : constMapThemeVisibleLayers )
      {
        QgsVectorLayer *vl = qobject_cast<QgsVectorLayer *>( layer );
        if ( !vl )
          continue;
        layers << QgsDxfExport::DxfLayer( vl );
        layerIds << vl->id();
      }
    }
    else
    {
      const auto constMapLayers = QgsProject::instance()->mapLayers();
      for ( QgsMapLayer *ml : constMapLayers )
      {
        QgsVectorLayer *vl = qobject_cast<QgsVectorLayer *>( ml );
        if ( !vl )
          continue;
        layers << QgsDxfExport::DxfLayer( vl );
        layerIds << vl->id();
      }
    }

    if ( !layers.isEmpty() )
    {
      dxfExport.addLayers( layers );
    }

    QFile dxfFile;
    if ( dxfOutputFile == QLatin1String( "-" ) )
    {
      if ( !dxfFile.open( stdout, QIODevice::WriteOnly | QIODevice::Truncate ) )
      {
        std::cerr << "could not open stdout" << std::endl;
        return 2;
      }
    }
    else
    {
      if ( !dxfOutputFile.endsWith( QLatin1String( ".dxf" ), Qt::CaseInsensitive ) )
        dxfOutputFile += QLatin1String( ".dxf" );
      dxfFile.setFileName( dxfOutputFile );
    }

    QgsDxfExport::ExportResult res = dxfExport.writeToFile( &dxfFile, dxfEncoding );
    switch ( res )
    {
      case QgsDxfExport::ExportResult::Success:
        break;

      case QgsDxfExport::ExportResult::DeviceNotWritableError:
        std::cerr << "dxf output failed, the device is not wriable" << std::endl;
        break;

      case QgsDxfExport::ExportResult::InvalidDeviceError:
        std::cerr << "dxf output failed, the device is invalid" << std::endl;
        break;

      case QgsDxfExport::ExportResult::EmptyExtentError:
        std::cerr << "dxf output failed, the extent could not be determined" << std::endl;
        break;
    }

    delete qgis;

    return static_cast<int>( res );
  }

  // make sure we don't have a dirty blank project after launch
  QgsProject::instance()->setDirty( false );

#ifdef HAVE_OPENCL

  // Overrides the OpenCL path to the folder containing the programs
  // - use the path specified with --openclprogramfolder,
  // - use the environment QGIS_OPENCL_PROGRAM_FOLDER if not found
  // - use a default location as a fallback (this is set in QgsApplication initialization)
  if ( openClProgramFolder.isEmpty() )
  {
    openClProgramFolder = getenv( "QGIS_OPENCL_PROGRAM_FOLDER" );
  }

  if ( ! openClProgramFolder.isEmpty() )
  {
    QgsOpenClUtils::setSourcePath( openClProgramFolder );
  }

#endif

  if ( takeScreenShots )
  {
    qgis->takeAppScreenShots( screenShotsPath, screenShotsCategories );
  }

  /////////////////////////////////////////////////////////////////////
  // Continue on to interactive gui...
  /////////////////////////////////////////////////////////////////////
  qgis->show();
  QgsApplication::connect( &myApp, SIGNAL( lastWindowClosed() ), &myApp, SLOT( quit() ) );

  mypSplash->finish( qgis );
  delete mypSplash;

  qgis->completeInitialization();

#if defined(ANDROID)
  // fix for Qt Ministro hiding app's menubar in favor of native Android menus
  qgis->menuBar()->setNativeMenuBar( false );
  qgis->menuBar()->setVisible( true );
#endif

#if !defined(Q_OS_WIN)
  UnixSignalWatcher sigwatch;
  sigwatch.watchForSignal( SIGINT );

  QObject::connect( &sigwatch, &UnixSignalWatcher::unixSignal, &myApp, [ ]( int signal )
  {
    switch ( signal )
    {
      case SIGINT:
        QgsApplication::exit( 1 );
        break;

      default:
        break;
    }
  } );
#endif

  int retval = QgsApplication::exec();
  delete qgis;
  return retval;
}
