/***************************************************************************
                             main.cpp
                             --------
    begin                : 2019-02-25
    copyright            : (C) 2019 Nyall Dawson
    email                : nyall dot dawson at gmail dot com
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
#include <QTimer>

#include <cstdio>
#include <cstdlib>

#include "qgsapplication.h"

#ifdef Q_OS_WIN
#include <fcntl.h> /*  _O_BINARY */
#else
#include <getopt.h>
#endif

#ifdef Q_OS_MACOS
#include <ApplicationServices/ApplicationServices.h>
#if MAC_OS_X_VERSION_MAX_ALLOWED < 1050
typedef SInt32 SRefCon;
#endif
#endif

#include "qgsprocess.h"
#include "qgsapplication.h"
#include "qgsproviderregistry.h"

#undef QgsDebugCall
#undef QgsDebugError
#undef QgsDebugMsgLevel
#define QgsDebugCall
#define QgsDebugError( str )
#define QgsDebugMsgLevel( str, level )


/////////////////////////////////////////////////////////////////
// Command line options 'behavior' flag setup
////////////////////////////////////////////////////////////////

// These two are global so that they can be set by the OpenDocuments
// AppleEvent handler as well as by the main routine argv processing

// This behavior will cause QGIS to autoload a project
static QString myProjectFileName;

// This is the 'leftover' arguments collection
static QStringList sFileList;

int main( int argc, char *argv[] )
{
#ifdef Q_OS_WIN // Windows
#ifdef _MSC_VER
  _set_fmode( _O_BINARY );
#else  //MinGW
  _fmode = _O_BINARY;
#endif // _MSC_VER
#endif // Q_OS_WIN

  // a shortcut -- let's see if we are being called without any arguments, or just the usage/version argument.
  // If so, let's skip the startup cost of a QCoreApplication/QgsApplication
  bool hasHelpArgument = false;
  bool hasVersionArgument = false;
  for ( int i = 1; i < argc; ++i )
  {
    const QString arg( argv[i] );
    if ( arg == QLatin1String( "--json" )
         || arg == QLatin1String( "--verbose" )
         || arg == QLatin1String( "--no-python" ) )
    {
      // ignore these arguments
      continue;
    }
    if ( arg == QLatin1String( "--help" ) || arg == QLatin1String( "-h" ) )
    {
      hasHelpArgument = true;
      break;
    }
    else if ( arg == QLatin1String( "--version" ) || arg == QLatin1String( "-v" ) )
    {
      hasVersionArgument = true;
      break;
    }
    break;
  }

  if ( argc == 1 || hasHelpArgument )
  {
    QgsProcessingExec::showUsage( QString( argv[0] ) );
    return 0;
  }
  else if ( hasVersionArgument )
  {
    QgsProcessingExec::showVersionInformation();
    return 0;
  }

  QgsApplication app( argc, argv, false, QString(), QStringLiteral( "qgis_process" ) );

  // Build a local QCoreApplication from arguments. This way, arguments are correctly parsed from their native locale
  // It will use QString::fromLocal8Bit( argv ) under Unix and GetCommandLine() under Windows.
  QStringList args = QCoreApplication::arguments();

  QgsProcessingExec::Flags flags;

  const int jsonIndex = args.indexOf( QLatin1String( "--json" ) );
  if ( jsonIndex >= 0 )
  {
    flags |= QgsProcessingExec::Flag::UseJson;
    args.removeAt( jsonIndex );
  }

  const int verboseIndex = args.indexOf( QLatin1String( "--verbose" ) );
  Qgis::ProcessingLogLevel logLevel = Qgis::ProcessingLogLevel::DefaultLevel;
  if ( verboseIndex >= 0 )
  {
    logLevel = Qgis::ProcessingLogLevel::Verbose;
    args.removeAt( verboseIndex );
  }

  const int noPythonIndex = args.indexOf( QLatin1String( "--no-python" ) );
  if ( noPythonIndex >= 0 )
  {
    flags |= QgsProcessingExec::Flag::SkipPython;
    args.removeAt( noPythonIndex );
  }

  const int skipLoadingPluginsIndex = args.indexOf( QLatin1String( "--skip-loading-plugins" ) );
  if ( skipLoadingPluginsIndex >= 0 )
  {
    flags |= QgsProcessingExec::Flag::SkipLoadingPlugins;
    args.removeAt( skipLoadingPluginsIndex );
  }

  const QString command = args.value( 1 );
  if ( args.size() == 1 || command == QLatin1String( "--help" ) || command == QLatin1String( "-h" ) )
  {
    // a shortcut -- if we are showing usage information, we don't need to initialize
    // QgsApplication at all!
    QgsProcessingExec::showUsage( args.at( 0 ) );
    return 0;
  }

  QString myPrefixPath;
  if ( myPrefixPath.isEmpty() )
  {
    QDir dir( QCoreApplication::applicationDirPath() );
    dir.cdUp();
    myPrefixPath = dir.absolutePath();
  }
  QgsApplication::setPrefixPath( myPrefixPath, true );

  // Set up the QSettings environment must be done after qapp is created
  QgsApplication::setOrganizationName( QStringLiteral( "QGIS" ) );
  QgsApplication::setOrganizationDomain( QStringLiteral( "qgis.org" ) );
  QgsApplication::setApplicationName( QStringLiteral( "QGIS3" ) );

  QgsApplication::init();
  QgsApplication::initQgis();

  QgsProviderRegistry::instance( QgsApplication::pluginPath() );

  ( void ) QgsApplication::resolvePkgPath(); // trigger storing of application path in QgsApplication

  QgsProcessingExec exec;
  int res = 0;
  QTimer::singleShot( 0, &app, [&exec, args, logLevel, flags, &res] {
    res = exec.run( args, logLevel, flags );
    QgsApplication::exitQgis();
    QCoreApplication::exit( res );
  } );
  return QgsApplication::exec();
}
