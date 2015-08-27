/***************************************************************************
                 main.cpp  - Benchmark, derived form app/main.cpp
                             -------------------
    begin                : 2011-11-15
    copyright            : (C) 2011 Radim Blazek
    email                : radim dot blazek at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QSettings>
#include <QString>
#include <QStringList>
#include <QtTest/QTest>

#include <cstdio>
#include <stdio.h>
#include <stdlib.h>

#ifdef Q_OS_WIN
// Open files in binary mode
#include <fcntl.h> /*  _O_BINARY */
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

#include "qgsbench.h"
#include "qgsapplication.h"
#include <qgsconfig.h>
#include <qgsversion.h>
#include "qgsexception.h"
#include "qgsproject.h"
#include "qgsproviderregistry.h"
#include "qgsrectangle.h"
#include "qgslogger.h"


/** Print usage text
 */
void usage( std::string const & appName )
{
  std::cerr << "QGIS Benchmark - " << VERSION << " '" << RELEASE_NAME << "' ("
            << QGSVERSION << ")\n"
            << "QGIS (QGIS) Benchmark is console application for QGIS benchmarking\n"
            << "Usage: " << appName <<  " [options] [FILES]\n"
            << "  options:\n"
            << "\t[--iterations iterations]\tnumber of rendering cycles, default 1\n"
            << "\t[--snapshot filename]\temit snapshot of loaded datasets to given file\n"
            << "\t[--log filename]\twrite log (JSON) to given file\n"
            << "\t[--width width]\twidth of snapshot to emit\n"
            << "\t[--height height]\theight of snapshot to emit\n"
            << "\t[--project projectfile]\tload the given QGIS project\n"
            << "\t[--extent xmin,ymin,xmax,ymax]\tset initial map extent\n"
            << "\t[--optionspath path]\tuse the given QSettings path\n"
            << "\t[--configpath path]\tuse the given path for all user configuration\n"
            << "\t[--prefix path]\tpath to a different build of qgis, may be used to test old versions\n"
            << "\t[--quality]\trenderer hint(s), comma separated, possible values: Antialiasing,TextAntialiasing,SmoothPixmapTransform,NonCosmeticDefaultPen\n"
            << "\t[--parallel]\trender layers in parallel instead of sequentially\n"
            << "\t[--print type]\twhat kind of time to print, possible values: wall,total,user,sys. Default is total.\n"
            << "\t[--help]\t\tthis text\n\n"
            << "  FILES:\n"
            << "    Files specified on the command line can include rasters,\n"
            << "    vectors, and QGIS project files (.qgs): \n"
            << "     1. Rasters - Supported formats include GeoTiff, DEM \n"
            << "        and others supported by GDAL\n"
            << "     2. Vectors - Supported formats include ESRI Shapefiles\n"
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

int main( int argc, char *argv[] )
{
#ifdef Q_OS_WIN  // Windows
#ifdef _MSC_VER
  _set_fmode( _O_BINARY );
#else //MinGW
  _fmode = _O_BINARY;
#endif  // _MSC_VER
#endif  // Q_OS_WIN

  /////////////////////////////////////////////////////////////////
  // Command line options 'behaviour' flag setup
  ////////////////////////////////////////////////////////////////

  //
  // Parse the command line arguments, looking to see if the user has asked for any
  // special behaviours. Any remaining non command arguments will be kept aside to
  // be passed as a list of layers and / or a project that should be loaded.
  //

  int myIterations = 1;
  QString mySnapshotFileName = "";
  QString myLogFileName = "";
  QString myPrefixPath = "";
  int mySnapshotWidth = 800;
  int mySnapshotHeight = 600;
  QString myQuality = "";
  bool myParallel = false;
  QString myPrintTime = "total";

  // This behaviour will set initial extent of map canvas, but only if
  // there are no command line arguments. This gives a usable map
  // extent when qgis starts with no layers loaded. When layers are
  // loaded, we let the layers define the initial extent.
  QString myInitialExtent = "";
  if ( argc == 1 )
    myInitialExtent = "-1,-1,1,1";

  // The user can specify a path which will override the default path of custom
  // user settings (~/.qgis) and it will be used for QSettings INI file
  QString configpath;

#ifndef Q_OS_WIN
  ////////////////////////////////////////////////////////////////
  // USe the GNU Getopts utility to parse cli arguments
  // Invokes ctor `GetOpt (int argc, char **argv,  char *optstring);'
  ///////////////////////////////////////////////////////////////
  int optionChar;
  while ( 1 )
  {
    static struct option long_options[] =
    {
      /* These options set a flag. */
      {"help", no_argument, 0, '?'},
      /* These options don't set a flag.
       *  We distinguish them by their indices. */
      {"iterations",    required_argument, 0, 'i'},
      {"snapshot", required_argument, 0, 's'},
      {"log", required_argument, 0, 'l'},
      {"width",    required_argument, 0, 'w'},
      {"height",   required_argument, 0, 'h'},
      {"project",  required_argument, 0, 'p'},
      {"extent",   required_argument, 0, 'e'},
      {"optionspath", required_argument, 0, 'o'},
      {"configpath", required_argument, 0, 'c'},
      {"prefix", required_argument, 0, 'r'},
      {"quality", required_argument, 0, 'q'},
      {"parallel", no_argument, 0, 'P'},
      {"print", required_argument, 0, 'R'},
      {0, 0, 0, 0}
    };

    /* getopt_long stores the option index here. */
    int option_index = 0;

    optionChar = getopt_long( argc, argv, "islwhpeocrq",
                              long_options, &option_index );

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

      case 'i':
        myIterations = QString( optarg ).toInt();
        break;

      case 's':
        mySnapshotFileName = QDir::toNativeSeparators( QFileInfo( QFile::decodeName( optarg ) ).absoluteFilePath() );
        break;

      case 'l':
        myLogFileName = QDir::toNativeSeparators( QFileInfo( QFile::decodeName( optarg ) ).absoluteFilePath() );
        break;

      case 'w':
        mySnapshotWidth = QString( optarg ).toInt();
        break;

      case 'h':
        mySnapshotHeight = QString( optarg ).toInt();
        break;

      case 'p':
        myProjectFileName = QDir::toNativeSeparators( QFileInfo( QFile::decodeName( optarg ) ).absoluteFilePath() );
        break;

      case 'e':
        myInitialExtent = optarg;
        break;

      case 'o':
        QSettings::setPath( QSettings::IniFormat, QSettings::UserScope, optarg );
        break;

      case 'c':
        configpath = optarg;
        break;

      case 'r':
        myPrefixPath = optarg;
        break;

      case 'q':
        myQuality = optarg;
        break;

      case 'P':
        myParallel = true;
        break;

      case 'R':
        myPrintTime = optarg;
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
      myFileList.append( QDir::toNativeSeparators( QFileInfo( QFile::decodeName( argv[optind++] ) ).absoluteFilePath() ) );
    }
  }
#else
  for ( int i = 1; i < argc; i++ )
  {
    QString arg = argv[i];

    if ( arg == "--help" || arg == "-?" )
    {
      usage( argv[0] );
      return 2;
    }
    else if ( i + 1 < argc && ( arg == "--iterations" || arg == "-i" ) )
    {
      myIterations = QString( argv[++i] ).toInt();
    }
    else if ( i + 1 < argc && ( arg == "--snapshot" || arg == "-s" ) )
    {
      mySnapshotFileName = QDir::toNativeSeparators( QFileInfo( QFile::decodeName( argv[++i] ) ).absoluteFilePath() );
    }
    else if ( i + 1 < argc && ( arg == "--log" || arg == "-l" ) )
    {
      myLogFileName = QDir::toNativeSeparators( QFileInfo( QFile::decodeName( argv[++i] ) ).absoluteFilePath() );
    }
    else if ( i + 1 < argc && ( arg == "--width" || arg == "-w" ) )
    {
      mySnapshotWidth = QString( argv[++i] ).toInt();
    }
    else if ( i + 1 < argc && ( arg == "--height" || arg == "-h" ) )
    {
      mySnapshotHeight = QString( argv[++i] ).toInt();
    }
    else if ( i + 1 < argc && ( arg == "--project" || arg == "-p" ) )
    {
      myProjectFileName = QDir::toNativeSeparators( QFileInfo( QFile::decodeName( argv[++i] ) ).absoluteFilePath() );
    }
    else if ( i + 1 < argc && ( arg == "--extent" || arg == "-e" ) )
    {
      myInitialExtent = argv[++i];
    }
    else if ( i + 1 < argc && ( arg == "--optionspath" || arg == "-o" ) )
    {
      QSettings::setPath( QSettings::IniFormat, QSettings::UserScope, argv[++i] );
    }
    else if ( i + 1 < argc && ( arg == "--configpath" || arg == "-c" ) )
    {
      configpath = argv[++i];
      QSettings::setPath( QSettings::IniFormat, QSettings::UserScope, configpath );
    }
    else if ( i + 1 < argc && ( arg == "--prefix" ) )
    {
      myPrefixPath = argv[++i];
    }
    else if ( i + 1 < argc && ( arg == "--quality" || arg == "-q" ) )
    {
      myQuality = argv[++i];
    }
    else if ( arg == "--parallel" || arg == "-P" )
    {
      myParallel = true;
    }
    else if ( i + 1 < argc && ( arg == "--print" || arg == "-R" ) )
    {
      myPrintTime = argv[++i];
    }
    else
    {
      myFileList.append( QDir::toNativeSeparators( QFileInfo( QFile::decodeName( argv[i] ) ).absoluteFilePath() ) );
    }
  }
#endif // Q_OS_WIN

  /////////////////////////////////////////////////////////////////////
  // Now we have the handlers for the different behaviours...
  ////////////////////////////////////////////////////////////////////

  /////////////////////////////////////////////////////////////////////
  // Initialise the application and the translation stuff
  /////////////////////////////////////////////////////////////////////

  if ( !configpath.isEmpty() )
  {
    // tell QSettings to use INI format and save the file in custom config path
    QSettings::setPath( QSettings::IniFormat, QSettings::UserScope, configpath );
  }

  // TODO: Qt: there should be exactly one QCoreApplication object for console app.
  // but QgsApplication inherits from QApplication (GUI)
  // it is working, but maybe we should make QgsCoreApplication, which
  // could also be used by mapserver
  // Note (mkuhn,20.4.2013): Labeling does not work with QCoreApplication, because
  // fontconfig needs some X11 dependencies which are initialized in QApplication (GUI)
  // using it with QCoreApplication only crashes at the moment.
  // Only use QgsApplication( int, char **, bool GUIenabled, QString) for newer versions
  // so that this program may be run with old libraries
  //QgsApplication myApp( argc, argv, false, configpath );

  QCoreApplication *myApp;

#if VERSION_INT >= 10900
  myApp = new QgsApplication( argc, argv, false );
#else
  myApp = new QCoreApplication( argc, argv );
#endif

  if ( myPrefixPath.isEmpty() )
  {
    QDir dir( QCoreApplication::applicationDirPath() );
    dir.cdUp();
    myPrefixPath = dir.absolutePath();
  }
  QgsApplication::setPrefixPath( myPrefixPath, true );

  // Set up the QSettings environment must be done after qapp is created
  QgsApplication::setOrganizationName( "QGIS" );
  QgsApplication::setOrganizationDomain( "qgis.org" );
  QgsApplication::setApplicationName( "QGIS2" );

  QgsProviderRegistry::instance( QgsApplication::pluginPath() );

#ifdef Q_OS_MACX
  // If the GDAL plugins are bundled with the application and GDAL_DRIVER_PATH
  // is not already defined, use the GDAL plugins in the application bundle.
  QString gdalPlugins( QCoreApplication::applicationDirPath().append( "/lib/gdalplugins" ) );
  if ( QFile::exists( gdalPlugins ) && !getenv( "GDAL_DRIVER_PATH" ) )
  {
    setenv( "GDAL_DRIVER_PATH", gdalPlugins.toUtf8(), 1 );
  }

  // Point GDAL_DATA at any GDAL share directory embedded in the app bundle
  if ( !getenv( "GDAL_DATA" ) )
  {
    QStringList gdalShares;
    QString appResources( QDir::cleanPath( QgsApplication::pkgDataPath() ) );
    gdalShares << QCoreApplication::applicationDirPath().append( "/share/gdal" )
    << appResources.append( "/share/gdal" )
    << appResources.append( "/gdal" );
    Q_FOREACH ( const QString& gdalShare, gdalShares )
    {
      if ( QFile::exists( gdalShare ) )
      {
        setenv( "GDAL_DATA", gdalShare.toUtf8().constData(), 1 );
        break;
      }
    }
  }
#endif

  QSettings mySettings;

  // For non static builds on mac and win (static builds are not supported)
  // we need to be sure we can find the qt image
  // plugins. In mac be sure to look in the
  // application bundle...
#ifdef Q_OS_WIN
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
    myPath += "/Contents/plugins";
    QCoreApplication::addLibraryPath( myPath );
  }
#endif

  QgsBench *qbench = new QgsBench( mySnapshotWidth, mySnapshotHeight, myIterations );

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
      QString arg = QDir::toNativeSeparators( QFileInfo( QFile::decodeName( argv[i] ) ).absoluteFilePath() );
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
    if ( ! qbench->openProject( myProjectFileName ) )
    {
      fprintf( stderr, "Cannot load project\n" );
      return 1;
    }
  }

  if ( ! myQuality.isEmpty() )
  {
    QPainter::RenderHints hints;
    QStringList list = myQuality.split( ',' );
    foreach ( QString q, list )
    {
      if ( q == "Antialiasing" ) hints |= QPainter::Antialiasing;
      else if ( q == "TextAntialiasing" ) hints |= QPainter::TextAntialiasing;
      else if ( q == "SmoothPixmapTransform" ) hints |= QPainter::SmoothPixmapTransform;
      else if ( q == "NonCosmeticDefaultPen" ) hints |= QPainter::NonCosmeticDefaultPen;
      else
      {
        fprintf( stderr, "Unknown quality option\n" );
        return 1;
      }
    }
    QgsDebugMsg( QString( "hints: %1" ).arg( hints ) );
    qbench->setRenderHints( hints );
  }

  qbench->setParallel( myParallel );

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
      fprintf( stderr, "Data files not yet supported\n" );
      return 1;
      //qbench->openLayer( myLayerName );
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
      qbench->setExtent( rect );
    }
  }

  qbench->render();

  if ( mySnapshotFileName != "" )
  {
    qbench->saveSnapsot( mySnapshotFileName );
  }

  if ( myLogFileName != "" )
  {
    qbench->saveLog( myLogFileName );
  }

  qbench->printLog( myPrintTime );

  delete qbench;
  delete myApp;
  QCoreApplication::exit( 0 );
}
