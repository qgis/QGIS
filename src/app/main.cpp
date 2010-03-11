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
/* $Id$ */

//qt includes
#include <QBitmap>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QFont>
#include <QMessageBox>
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

#include "qgspluginregistry.h"

#include <cstdio>
#include <stdio.h>
#include <stdlib.h>

#ifdef WIN32
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

#include "qgisapp.h"
#include "qgsmapcanvas.h"
#include "qgsapplication.h"
#include <qgsconfig.h>
#include <qgssvnversion.h>
#include "qgsexception.h"
#include "qgsproject.h"
#include "qgsrectangle.h"
#include "qgslogger.h"

static const char * const ident_ = "$Id$";

/** print usage text
 */
void usage( std::string const & appName )
{
  std::cerr << "Quantum GIS - " << VERSION << " '" << RELEASE_NAME << "' ("
            << QGSSVNVERSION << ")\n"
            << "Quantum GIS (QGIS) is a viewer for spatial data sets, including\n"
            << "raster and vector data.\n"
            << "Usage: " << appName <<  " [options] [FILES]\n"
            << "  options:\n"
            << "\t[--snapshot filename]\temit snapshot of loaded datasets to given file\n"
            << "\t[--width width]\twidth of snapshot to emit\n"
            << "\t[--height height]\theight of snapshot to emit\n"
            << "\t[--lang language]\tuse language for interface text\n"
            << "\t[--project projectfile]\tload the given QGIS project\n"
            << "\t[--extent xmin,ymin,xmax,ymax]\tset initial map extent\n"
            << "\t[--nologo]\thide splash screen\n"
            << "\t[--optionspath path]\tuse the given QSettings path\n"
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


#ifdef Q_OS_MACX
/* Mac OS OpenDocuments AppleEvent handler called when files are double-clicked.
 * May be called at startup before application is initialized as well as
 * at any time while the application is running.
 */
OSErr openDocumentsAEHandler( const AppleEvent *event, AppleEvent *reply, SRefCon refCon )
{
  AEDescList docs;
  if ( AEGetParamDesc( event, keyDirectObject, typeAEList, &docs ) == noErr )
  {
    // Get count of files to open
    long count = 0;
    AECountItems( &docs, &count );

    // Examine files and load first project file followed by all other non-project files
    myProjectFileName.truncate( 0 );
    myFileList.clear();
    for ( int i = 0; i < count; i++ )
    {
      FSRef ref;
      UInt8 strBuffer[256];
      if ( AEGetNthPtr( &docs, i + 1, typeFSRef, 0, 0, &ref, sizeof( ref ), 0 ) == noErr &&
           FSRefMakePath( &ref, strBuffer, 256 ) == noErr )
      {
        QString fileName( QString::fromUtf8( reinterpret_cast<char *>( strBuffer ) ) );
        if ( fileName.endsWith( ".qgs" ) )
        {
          // Load first project file and ignore all other project files
          if ( myProjectFileName.isEmpty() )
          {
            myProjectFileName = fileName;
          }
        }
        else
        {
          // Load all non-project files
          myFileList.append( fileName );
        }
      }
    }

    // Open files now if application has been fully initialized (has objectName).
    // Otherwise (if this routine is called by processEvents inside the QgisApp constructor
    // at startup) wait for the command line file loader to notice these files.
    QgisApp *qgis = QgisApp::instance();
    if ( qgis && qgis->objectName() == "QgisApp" )
    {
      if ( !myProjectFileName.isEmpty() )
      {
        qgis->openProject( myProjectFileName );
      }
      for ( QStringList::Iterator myIterator = myFileList.begin();
            myIterator != myFileList.end(); ++myIterator )
      {
        QString fileName = *myIterator;
        qgis->openLayer( fileName );
      }
    }
  }
  return noErr;
}
#endif


/* Test to determine if this program was started on Mac OS X by double-clicking
 * the application bundle rather then from a command line. If clicked, argv[1]
 * contains a process serial number in the form -psn_0_1234567. Don't process
 * the command line arguments in this case because argv[1] confuses the processing.
 */
bool bundleclicked( int argc, char *argv[] )
{
  return ( argc > 1 && memcmp( argv[1], "-psn_", 5 ) == 0 );
}


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

      // TODO: Verify this code in action.
      if ( 0 == strncmp( msg, "libpng error:", 13 ) )
      {
        // Let the user know
        QMessageBox::warning( 0, "libpng Error", msg );
      }

      break;
    case QtFatalMsg:
      fprintf( stderr, "Fatal: %s\n", msg );
      abort();                    // deliberately core dump
  }
}


int main( int argc, char *argv[] )
{
#ifdef WIN32  // Windows
#ifdef _MSC_VER
  _set_fmode( _O_BINARY );
#else //MinGW
  _fmode = _O_BINARY;
#endif  // _MSC_VER
#endif  // WIN32

#ifndef _MSC_VER
  // Set up the custom qWarning/qDebug custom handler
  qInstallMsgHandler( myMessageOutput );
#endif

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

#ifndef WIN32
  if ( !bundleclicked( argc, argv ) )
  {

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
        {"nologo", no_argument, 0, 'n'},
        /* These options don't set a flag.
         *  We distinguish them by their indices. */
        {"snapshot", required_argument, 0, 's'},
        {"width",    required_argument, 0, 'w'},
        {"height",   required_argument, 0, 'h'},
        {"lang",     required_argument, 0, 'l'},
        {"project",  required_argument, 0, 'p'},
        {"extent",   required_argument, 0, 'e'},
        {"optionspath", required_argument, 0, 'o'},
        {0, 0, 0, 0}
      };

      /* getopt_long stores the option index here. */
      int option_index = 0;

      optionChar = getopt_long( argc, argv, "swhlpeo",
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

        case 'e':
          myInitialExtent = optarg;
          break;

        case 'o':
          QSettings::setPath( QSettings::NativeFormat, QSettings::UserScope, optarg );
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
#else
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
      QSettings::setPath( QSettings::NativeFormat, QSettings::UserScope, argv[++i] );
    }
    else
    {
      myFileList.append( QDir::convertSeparators( QFileInfo( QFile::decodeName( argv[i] ) ).absoluteFilePath() ) );
    }
  }
#endif //WIN32

  /////////////////////////////////////////////////////////////////////
  // Now we have the handlers for the different behaviours...
  ////////////////////////////////////////////////////////////////////

  /////////////////////////////////////////////////////////////////////
  // Initialise the application and the translation stuff
  /////////////////////////////////////////////////////////////////////

#ifdef Q_WS_X11
  bool myUseGuiFlag = getenv( "DISPLAY" ) != 0;
#else
  bool myUseGuiFlag = TRUE;
#endif
  if ( !myUseGuiFlag )
  {
    QgsDebugMsg( "QGIS starting in non-interactive mode not supported.\n You "
                 "are seeing this message most likely because you have no DISPLAY "
                 "environment variable set." );
    exit( 1 ); //exit for now until a version of qgis is capabable of running non interactive
  }
  QgsApplication myApp( argc, argv, myUseGuiFlag );
  //
  // Set up the QSettings environment must be done after qapp is created
  QCoreApplication::setOrganizationName( "QuantumGIS" );
  QCoreApplication::setOrganizationDomain( "qgis.org" );
  QCoreApplication::setApplicationName( "QGIS" );
#ifdef Q_OS_MACX
  // Install OpenDocuments AppleEvent handler after application object is initialized
  // but before any other event handling (including dialogs or splash screens) occurs.
  // If an OpenDocuments event has been created before the application was launched,
  // it must be handled before some other event handler runs and dismisses it as unknown.
  // If run at startup, the handler will set either or both of myProjectFileName and myFileList.
  AEInstallEventHandler( kCoreEventClass, kAEOpenDocuments, openDocumentsAEHandler, 0, false );

  // If the GDAL plugins are bundled with the application and GDAL_DRIVER_PATH
  // is not already defined, use the GDAL plugins in the application bundle.
  QString gdalPlugins( QCoreApplication::applicationDirPath().append( "/lib/gdalplugins" ) );
  if ( QFile::exists( gdalPlugins ) && !getenv( "GDAL_DRIVER_PATH" ) )
  {
    setenv( "GDAL_DRIVER_PATH", gdalPlugins.toUtf8(), 1 );
  }
#endif

#ifdef Q_WS_WIN
  //for windows lets use plastique style!
  QApplication::setStyle( new QPlastiqueStyle );
#endif

  // Check to see if qgis was started from the source directory.
  // This is done by checking whether qgis binary is in 'src/app'
  // directory. If running from there, exit gracefully.
  // (QGIS might work incorrectly when run from the sources)
  QString appDir = qApp->applicationDirPath();

  if ( appDir.endsWith( "/src/app" ) )
  {
    QMessageBox::critical( 0, "QGIS Not Installed",
                           "You appear to be running QGIS from the source directory.\n"
                           "You must install QGIS using make install and run it from the "
                           "installed directory." );
    exit( 1 );
  }

  QString i18nPath = QgsApplication::i18nPath();

  /* Translation file for QGIS.
   */
  QSettings mySettings;
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
  /* Translation file for Qt.
   * The strings from the QMenuBar context section are used by Qt/Mac to shift
   * the About, Preferences and Quit items to the Mac Application menu.
   * These items must be translated identically in both qt_ and qgis_ files.
   */
  QTranslator qttor( 0 );
  if ( qttor.load( QString( "qt_" ) + myTranslationCode, i18nPath ) )
  {
    myApp.installTranslator( &qttor );
  }

  //set up splash screen
  QString mySplashPath( QgsApplication::splashPath() );
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
    myPath += "/Contents/plugins";
    QCoreApplication::addLibraryPath( myPath );
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

  QgisApp *qgis = new QgisApp( mypSplash ); // "QgisApp" used to find canonical instance
  qgis->setObjectName( "QgisApp" );

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

  /////////////////////////////////////////////////////////////////////
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
  return myApp.exec();
}
