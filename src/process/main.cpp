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
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <memory>

#include "qgsapplication.h"

#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QSettings>
#include <QStandardPaths>
#include <QString>
#include <QStringList>
#include <QTimer>

using namespace Qt::StringLiterals;

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
#include "qgsuserprofilemanager.h"
#include "qgsuserprofile.h"
#include "qgserror.h"

#ifdef HAVE_OPENCL
#include "qgsopenclutils.h"
#endif

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

// The value-taking global options, shared by the fast path and the main argument
// parser so both agree on which options exist and what a valid value looks like.
static bool isProfileOption( const QString &arg )
{
  return arg == "--profile"_L1 || arg == "--profiles-path"_L1 || arg == "-S"_L1;
}

// A value that is missing, empty or itself an option would silently swallow the
// following token if it were accepted as a profile name or path.
static bool isValidProfileValue( const QString &value )
{
  return !value.isEmpty() && !value.startsWith( "--"_L1 ) && value != "-S"_L1 && value != "-h"_L1 && value != "-v"_L1;
}

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
    if ( arg == "--json"_L1 || arg == "--verbose"_L1 || arg == "--no-python"_L1 )
    {
      // ignore these arguments
      continue;
    }
    if ( isProfileOption( arg ) )
    {
      // A malformed profile option is always an error, so stop looking for the
      // help/version shortcut here and let the parser below report it.
      if ( i + 1 >= argc || !isValidProfileValue( QString( argv[i + 1] ) ) )
        break;
      // skip the value as well and keep looking for --help/--version
      ++i;
      continue;
    }
    if ( arg == "--help"_L1 || arg == "-h"_L1 )
    {
      hasHelpArgument = true;
      break;
    }
    else if ( arg == "--version"_L1 || arg == "-v"_L1 )
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

  QgsApplication app( argc, argv, false, QString(), u"qgis_process"_s );

  // Build a local QCoreApplication from arguments. This way, arguments are correctly parsed from their native locale
  // It will use QString::fromLocal8Bit( argv ) under Unix and GetCommandLine() under Windows.
  QStringList args = QCoreApplication::arguments();

  // Profile selection is a global option and must be resolved before
  // QgsApplication::init() and plugin loading, because the chosen profile
  // decides where settings and plugins are read from. Every argument before the
  // "--" separator is scanned, so a profile option can sit next to the other
  // global options without consuming algorithm parameters that follow the
  // separator. This runs before the flags below are removed, so that a
  // forgotten value cannot swallow one of them.
  QString profileName;
  QString configLocalStorageLocation;
  for ( int i = 1; i < args.size(); )
  {
    const QString arg = args.at( i );
    if ( arg == "--"_L1 )
      break;

    if ( arg == "--profile"_L1 )
    {
      if ( i + 1 >= args.size() || !isValidProfileValue( args.at( i + 1 ) ) )
      {
        std::cerr << "The --profile option requires a profile name\n";
        return 1;
      }
      profileName = args.at( i + 1 );
      args.removeAt( i );
      args.removeAt( i );
    }
    else if ( arg == "--profiles-path"_L1 || arg == "-S"_L1 )
    {
      if ( i + 1 >= args.size() || !isValidProfileValue( args.at( i + 1 ) ) )
      {
        std::cerr << "The --profiles-path option requires a directory path\n";
        return 1;
      }
      configLocalStorageLocation = QDir::toNativeSeparators( QFileInfo( args.at( i + 1 ) ).absoluteFilePath() );
      args.removeAt( i );
      args.removeAt( i );
    }
    else
    {
      ++i;
    }
  }

  QgsProcessingExec::Flags flags;

  const int jsonIndex = args.indexOf( "--json"_L1 );
  if ( jsonIndex >= 0 )
  {
    flags |= QgsProcessingExec::Flag::UseJson;
    args.removeAt( jsonIndex );
  }

  const int verboseIndex = args.indexOf( "--verbose"_L1 );
  Qgis::ProcessingLogLevel logLevel = Qgis::ProcessingLogLevel::DefaultLevel;
  if ( verboseIndex >= 0 )
  {
    logLevel = Qgis::ProcessingLogLevel::Verbose;
    args.removeAt( verboseIndex );
  }

  const int noPythonIndex = args.indexOf( "--no-python"_L1 );
  if ( noPythonIndex >= 0 )
  {
    flags |= QgsProcessingExec::Flag::SkipPython;
    args.removeAt( noPythonIndex );
  }

  const int skipLoadingPluginsIndex = args.indexOf( "--skip-loading-plugins"_L1 );
  if ( skipLoadingPluginsIndex >= 0 )
  {
    flags |= QgsProcessingExec::Flag::SkipLoadingPlugins;
    args.removeAt( skipLoadingPluginsIndex );
  }

  const QString command = args.value( 1 );
  if ( args.size() == 1 || command == "--help"_L1 || command == "-h"_L1 )
  {
    // a shortcut -- if we are showing usage information, we don't need to initialize
    // QgsApplication at all!
    QgsProcessingExec::showUsage( args.at( 0 ) );
    return 0;
  }

  // Set up the QSettings environment must be done after qapp is created
  QgsApplication::setOrganizationName( u"QGIS"_s );
  QgsApplication::setOrganizationDomain( u"qgis.org"_s );
  QgsApplication::setApplicationName( u"QGIS4"_s );

  // Resolve the requested profile with the same infrastructure QGIS Desktop uses.
  // If no profile option was given both variables are empty and we keep the
  // original init() call, so the default-profile behavior is unchanged.
  if ( !profileName.isEmpty() || !configLocalStorageLocation.isEmpty() )
  {
    if ( configLocalStorageLocation.isEmpty() )
    {
      if ( getenv( "QGIS_CUSTOM_CONFIG_PATH" ) )
        configLocalStorageLocation = getenv( "QGIS_CUSTOM_CONFIG_PATH" );
      else
        configLocalStorageLocation = QStandardPaths::standardLocations( QStandardPaths::AppDataLocation ).value( 0 );
    }

    const QString rootProfileFolder = QgsUserProfileManager::resolveProfilesFolder( configLocalStorageLocation );
    QgsUserProfileManager manager( rootProfileFolder );

    // An empty profile name means the default profile.
    const QString requestedProfile = profileName.isEmpty() ? manager.defaultProfileName() : profileName;

    // Create the profile explicitly so that a creation failure is reported with
    // its reason; getProfile() drops the error. Only a folder-creation failure
    // (for example an unwritable profiles path) is fatal here, so we check that
    // the folder really appeared. A missing master database is tolerated, the
    // same way QGIS itself does.
    if ( !manager.profileExists( requestedProfile ) )
    {
      const QgsError error = manager.createUserProfile( requestedProfile );
      if ( !error.isEmpty() && !manager.profileExists( requestedProfile ) )
      {
        std::cerr << "Could not create profile " << requestedProfile.toLocal8Bit().constData() << ": " << error.summary().toLocal8Bit().constData() << "\n";
        return 1;
      }
    }

    // Creation is handled above, so do not let getProfile() create it again.
    std::unique_ptr<QgsUserProfile> profile = manager.getProfile( requestedProfile, false );
    QgsApplication::init( profile->folder() );
  }
  else
  {
    QgsApplication::init();
  }
  QgsApplication::initQgis();

  QgsProviderRegistry::instance( QgsApplication::pluginPath() );

  ( void ) QgsApplication::resolvePkgPath(); // trigger storing of application path in QgsApplication

#ifdef HAVE_OPENCL

  // Set the OpenCL source path to the folder containing the programs
  // - use the environment variable QGIS_OPENCL_PROGRAM_FOLDER,
  // - use a default location as a fallback

  QString openClProgramFolder = getenv( "QGIS_OPENCL_PROGRAM_FOLDER" );

  if ( openClProgramFolder.isEmpty() )
  {
    openClProgramFolder = QDir( QgsApplication::pkgDataPath() ).absoluteFilePath( u"resources/opencl_programs"_s );
  }

  QgsOpenClUtils::setSourcePath( openClProgramFolder );

#endif

  QgsProcessingExec exec;
  int res = 0;
  QTimer::singleShot( 0, &app, [&exec, args, logLevel, flags, &res] {
    res = exec.run( args, logLevel, flags );
    QgsApplication::exitQgis();
    QCoreApplication::exit( res );
  } );
  return QgsApplication::exec();
}
