/***************************************************************************
                          qgsprocess.h
                          -------------------
    begin                : February 2019
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

#include "qgsprocess.h"

#include "qgscommandlineutils.h"
#include "qgsnativealgorithms.h"
#include "qgsprocessingalgorithm.h"
#include "qgsprocessingregistry.h"

#include "moc_qgsprocess.cpp"

#ifdef HAVE_3D
#include "qgs3dalgorithms.h"
#endif
#include "qgspdalalgorithms.h"
#ifdef WITH_SFCGAL
#include <SFCGAL/capi/sfcgal_c.h>
#endif
#ifdef WITH_GEOGRAPHICLIB
#include <GeographicLib/Constants.hpp>
#endif
#include "qgssettings.h"
#include "qgsapplication.h"
#include "qgsprocessingparametertype.h"
#include "processing/models/qgsprocessingmodelalgorithm.h"
#include "qgsproject.h"
#include "qgsgeos.h"
#include "qgsunittypes.h"
#include "qgsjsonutils.h"
#include "qgsmessagelog.h"

#if defined( Q_OS_UNIX ) && !defined( Q_OS_ANDROID )
#include "sigwatch.h"
#endif

#include <iostream>
#include <string>
#include <QObject>
#include <QLibrary>

#include <ogr_api.h>
#include <gdal_version.h>
#include <proj.h>
#include <nlohmann/json.hpp>

ConsoleFeedback::ConsoleFeedback( bool useJson )
  : mUseJson( useJson )
{
  if ( !mUseJson )
  {
    connect( this, &QgsFeedback::progressChanged, this, &ConsoleFeedback::showTerminalProgress );
    mTimer.start();
  }
}

void ConsoleFeedback::setProgressText( const QString &text )
{
  if ( !mUseJson )
    std::cout << text.toLocal8Bit().constData() << '\n';
  QgsProcessingFeedback::setProgressText( text );
}

void ConsoleFeedback::reportError( const QString &error, bool fatalError )
{
  if ( !mUseJson )
    std::cerr << "ERROR:\t" << error.toLocal8Bit().constData() << '\n';
  else
  {
    if ( !mJsonLog.contains( u"errors"_s ) )
      mJsonLog.insert( u"errors"_s, QStringList() );
    mJsonLog[u"errors"_s] = mJsonLog.value( u"errors"_s ).toStringList() << error;
  }
  QgsProcessingFeedback::reportError( error, fatalError );
}

void ConsoleFeedback::pushWarning( const QString &warning )
{
  if ( !mUseJson )
    std::cout << "WARNING:\t" << warning.toLocal8Bit().constData() << '\n';
  else
  {
    if ( !mJsonLog.contains( u"warning"_s ) )
      mJsonLog.insert( u"warning"_s, QStringList() );
    mJsonLog[u"warning"_s] = mJsonLog.value( u"warning"_s ).toStringList() << warning;
  }
  QgsProcessingFeedback::pushWarning( warning );
}

void ConsoleFeedback::pushInfo( const QString &info )
{
  if ( !mUseJson )
    std::cout << info.toLocal8Bit().constData() << '\n';
  else
  {
    if ( !mJsonLog.contains( u"info"_s ) )
      mJsonLog.insert( u"info"_s, QStringList() );
    mJsonLog[u"info"_s] = mJsonLog.value( u"info"_s ).toStringList() << info;
  }
  QgsProcessingFeedback::pushInfo( info );
}

void ConsoleFeedback::pushCommandInfo( const QString &info )
{
  if ( !mUseJson )
    std::cout << info.toLocal8Bit().constData() << '\n';
  else
  {
    if ( !mJsonLog.contains( u"info"_s ) )
      mJsonLog.insert( u"info"_s, QStringList() );
    mJsonLog[u"info"_s] = mJsonLog.value( u"info"_s ).toStringList() << info;
  }
  QgsProcessingFeedback::pushCommandInfo( info );
}

void ConsoleFeedback::pushDebugInfo( const QString &info )
{
  if ( !mUseJson )
    std::cout << info.toLocal8Bit().constData() << '\n';
  else
  {
    if ( !mJsonLog.contains( u"info"_s ) )
      mJsonLog.insert( u"info"_s, QStringList() );
    mJsonLog[u"info"_s] = mJsonLog.value( u"info"_s ).toStringList() << info;
  }
  QgsProcessingFeedback::pushDebugInfo( info );
}

void ConsoleFeedback::pushConsoleInfo( const QString &info )
{
  if ( !mUseJson )
    std::cout << info.toLocal8Bit().constData() << '\n';
  else
  {
    if ( !mJsonLog.contains( u"info"_s ) )
      mJsonLog.insert( u"info"_s, QStringList() );
    mJsonLog[u"info"_s] = mJsonLog.value( u"info"_s ).toStringList() << info;
  }
  QgsProcessingFeedback::pushConsoleInfo( info );
}

void ConsoleFeedback::pushFormattedMessage( const QString &html, const QString &text )
{
  if ( !mUseJson )
    std::cout << text.toLocal8Bit().constData() << '\n';
  else
  {
    if ( !mJsonLog.contains( u"info"_s ) )
      mJsonLog.insert( u"info"_s, QStringList() );
    mJsonLog[u"info"_s] = mJsonLog.value( u"info"_s ).toStringList() << text;
  }
  QgsProcessingFeedback::pushFormattedMessage( html, text );
}

QVariantMap ConsoleFeedback::jsonLog() const
{
  return mJsonLog;
}

void ConsoleFeedback::showTerminalProgress( double progress )
{
  const int thisTick = std::min( 40, std::max( 0, static_cast<int>( progress * 0.4 ) ) );

  if ( mTimer.elapsed() > 2000 )
  {
    // check for signals every 2s to allow for responsive cancellation
    QCoreApplication::processEvents();
    mTimer.restart();
  }

  // Have we started a new progress run?
  if ( thisTick <= mLastTick )
    return;

  while ( thisTick > mLastTick )
  {
    ++mLastTick;
    if ( mLastTick % 4 == 0 )
      fprintf( stdout, "%d", ( mLastTick / 4 ) * 10 );
    else
      fprintf( stdout, "." );
  }

  if ( thisTick == 40 )
    fprintf( stdout, " - done.\n" );
  else
    fflush( stdout );
}

#ifdef WITH_BINDINGS
//! load Python support if possible
std::unique_ptr<QgsPythonUtils> QgsProcessingExec::loadPythonSupport()
{
  QString pythonlibName( u"qgispython"_s );
#if defined( Q_OS_UNIX ) && !defined( Q_OS_ANDROID )
  pythonlibName.prepend( QgsApplication::libraryPath() );
#endif
#ifdef __MINGW32__
  pythonlibName.prepend( "lib" );
#endif
  QString version = u"%1.%2.%3"_s.arg( Qgis::versionInt() / 10000 ).arg( Qgis::versionInt() / 100 % 100 ).arg( Qgis::versionInt() % 100 );
  QgsDebugMsgLevel( u"load library %1 (%2)"_s.arg( pythonlibName, version ), 1 );
  QLibrary pythonlib( pythonlibName, version );
  // It's necessary to set these two load hints, otherwise Python library won't work correctly
  // see http://lists.kde.org/?l=pykde&m=117190116820758&w=2
  pythonlib.setLoadHints( QLibrary::ResolveAllSymbolsHint | QLibrary::ExportExternalSymbolsHint );
  if ( !pythonlib.load() )
  {
    pythonlib.setFileName( pythonlibName );
    if ( !pythonlib.load() )
    {
      std::cerr << u"Couldn't load Python support library: %1\n"_s.arg( pythonlib.errorString() ).toLocal8Bit().constData();
      return nullptr;
    }
  }

  typedef QgsPythonUtils *( *inst )();
  inst pythonlib_inst = reinterpret_cast<inst>( cast_to_fptr( pythonlib.resolve( "instance" ) ) );
  if ( !pythonlib_inst )
  {
    //using stderr on purpose because we want end users to see this [TS]
    std::cerr << "Couldn't resolve Python support library's instance() symbol.\n";
    return nullptr;
  }

  std::unique_ptr<QgsPythonUtils> pythonUtils( pythonlib_inst() );
  if ( pythonUtils )
  {
    pythonUtils->initPython( nullptr, false );
  }

  return pythonUtils;
}
#endif

QgsProcessingExec::QgsProcessingExec()
{
}

int QgsProcessingExec::run( const QStringList &args, Qgis::ProcessingLogLevel logLevel, Flags flags )
{
  mFlags = flags;

  QObject::connect( QgsApplication::messageLog(), static_cast<void ( QgsMessageLog::* )( const QString &message, const QString &tag, Qgis::MessageLevel level )>( &QgsMessageLog::messageReceived ), QgsApplication::instance(), []( const QString &message, const QString &, Qgis::MessageLevel level ) {
    if ( level == Qgis::MessageLevel::Critical )
    {
      if ( !message.contains( "DeprecationWarning:"_L1 ) )
        std::cerr << message.toLocal8Bit().constData() << '\n';
    }
  } );

  // core providers
  QgsApplication::processingRegistry()->addProvider( new QgsNativeAlgorithms( QgsApplication::processingRegistry() ) );
#ifdef HAVE_3D
  QgsApplication::processingRegistry()->addProvider( new Qgs3DAlgorithms( QgsApplication::processingRegistry() ) );
#endif
  QgsApplication::processingRegistry()->addProvider( new QgsPdalAlgorithms( QgsApplication::processingRegistry() ) );

#ifdef WITH_BINDINGS
  if ( !( mFlags & Flag::SkipPython ) )
  {
    // give Python plugins a chance to load providers
    mPythonUtils = loadPythonSupport();
    if ( !mPythonUtils )
    {
      QCoreApplication::exit( 1 );
      return 1;
    }
  }
#endif

  const QString command = args.value( 1 );
  if ( command == "plugins"_L1 )
  {
    if ( args.size() == 2 || ( args.size() == 3 && args.at( 2 ) == "list"_L1 ) )
    {
      if ( !( mFlags & Flag::SkipLoadingPlugins ) )
      {
        loadPlugins();
      }
      listPlugins( mFlags & Flag::UseJson, !( mFlags & Flag::SkipLoadingPlugins ) );
      return 0;
    }
    else if ( args.size() == 4 && args.at( 2 ) == "enable"_L1 )
    {
      return enablePlugin( args.at( 3 ), true );
    }
    else if ( args.size() == 4 && args.at( 2 ) == "disable"_L1 )
    {
      return enablePlugin( args.at( 3 ), false );
    }
    std::cerr << u"Command %1 not known!\n"_s.arg( args.value( 2 ) ).toLocal8Bit().constData();
    return 1;
  }
  else if ( command == "list"_L1 )
  {
    if ( !( mFlags & Flag::SkipLoadingPlugins ) )
    {
      loadPlugins();
    }
    listAlgorithms();
    return 0;
  }
  else if ( command == "help"_L1 )
  {
    if ( args.size() < 3 )
    {
      std::cerr << u"Algorithm ID or model file not specified\n"_s.toLocal8Bit().constData();
      return 1;
    }

    if ( !( mFlags & Flag::SkipLoadingPlugins ) )
    {
      loadPlugins();
    }
    const QString algId = args.at( 2 );
    return showAlgorithmHelp( algId );
  }
  else if ( command == "run"_L1 )
  {
    if ( args.size() < 3 )
    {
      std::cerr << u"Algorithm ID or model file not specified\n"_s.toLocal8Bit().constData();
      return 1;
    }

    if ( !( mFlags & Flag::SkipLoadingPlugins ) )
    {
      loadPlugins();
    }

    const QString algId = args.at( 2 );

    // build parameter map
    QString ellipsoid;
    Qgis::DistanceUnit distanceUnit = Qgis::DistanceUnit::Unknown;
    Qgis::AreaUnit areaUnit = Qgis::AreaUnit::Unknown;
    QString projectPath;
    QVariantMap params;

    if ( args.size() == 4 && args.at( 3 ) == '-' )
    {
      // read arguments as JSON value from stdin
      std::string stdinJson;
      for ( std::string line; std::getline( std::cin, line ); )
      {
        stdinJson.append( line + '\n' );
      }

      QString error;
      const QVariantMap json = QgsJsonUtils::parseJson( stdinJson, error ).toMap();
      if ( !error.isEmpty() )
      {
        std::cerr << u"Could not parse JSON parameters: %1"_s.arg( error ).toLocal8Bit().constData() << std::endl;
        return 1;
      }
      if ( !json.contains( u"inputs"_s ) )
      {
        std::cerr << u"JSON parameters object must contain an \"inputs\" key."_s.toLocal8Bit().constData() << std::endl;
        return 1;
      }

      params = json.value( u"inputs"_s ).toMap();

      // JSON format for input parameters implies JSON output format
      mFlags |= Flag::UseJson;

      ellipsoid = json.value( u"ellipsoid"_s ).toString();
      projectPath = json.value( u"project_path"_s ).toString();
      if ( json.contains( "distance_units" ) )
      {
        bool ok = false;
        const QString distanceUnitsString = json.value( u"distance_units"_s ).toString();
        distanceUnit = QgsUnitTypes::decodeDistanceUnit( distanceUnitsString, &ok );
        if ( !ok )
        {
          std::cerr << u"%1 is not a valid distance unit value."_s.arg( distanceUnitsString ).toLocal8Bit().constData() << std::endl;
          return 1;
        }
      }

      if ( json.contains( "area_units" ) )
      {
        bool ok = false;
        const QString areaUnitsString = json.value( u"area_units"_s ).toString();
        areaUnit = QgsUnitTypes::decodeAreaUnit( areaUnitsString, &ok );
        if ( !ok )
        {
          std::cerr << u"%1 is not a valid area unit value."_s.arg( areaUnitsString ).toLocal8Bit().constData() << std::endl;
          return 1;
        }
      }
    }
    else
    {
      int i = 3;
      for ( ; i < args.count(); i++ )
      {
        QString arg = args.at( i );

        if ( arg == "--"_L1 )
        {
          break;
        }

        if ( arg.startsWith( "--"_L1 ) )
          arg = arg.mid( 2 );

        const QStringList parts = arg.split( '=' );
        if ( parts.count() >= 2 )
        {
          const QString name = parts.at( 0 );

          if ( name.compare( "ellipsoid"_L1, Qt::CaseInsensitive ) == 0 )
          {
            ellipsoid = parts.mid( 1 ).join( '=' );
          }
          else if ( name.compare( "distance_units"_L1, Qt::CaseInsensitive ) == 0 )
          {
            distanceUnit = QgsUnitTypes::decodeDistanceUnit( parts.mid( 1 ).join( '=' ) );
          }
          else if ( name.compare( "area_units"_L1, Qt::CaseInsensitive ) == 0 )
          {
            areaUnit = QgsUnitTypes::decodeAreaUnit( parts.mid( 1 ).join( '=' ) );
          }
          else if ( name.compare( "project_path"_L1, Qt::CaseInsensitive ) == 0 )
          {
            projectPath = parts.mid( 1 ).join( '=' );
          }
          else
          {
            const QString value = parts.mid( 1 ).join( '=' );
            if ( params.contains( name ) )
            {
              // parameter specified multiple times, store all of them in a list...
              if ( params.value( name ).type() == QVariant::StringList )
              {
                // append to existing list
                QStringList listValue = params.value( name ).toStringList();
                listValue << value;
                params.insert( name, listValue );
              }
              else
              {
                // upgrade previous value to list
                QStringList listValue = QStringList() << params.value( name ).toString()
                                                      << value;
                params.insert( name, listValue );
              }
            }
            else
            {
              params.insert( name, value );
            }
          }
        }
        else
        {
          std::cerr << u"Invalid parameter value %1. Parameter values must be entered after \"--\" e.g.\n  Example:\n    qgis_process run algorithm_name -- PARAM1=VALUE PARAM2=42\"\n"_s.arg( arg ).toLocal8Bit().constData();
          return 1;
        }
      }

      // After '--' we only have params
      for ( ; i < args.count(); i++ )
      {
        const QString arg = args.at( i );
        const QStringList parts = arg.split( '=' );
        if ( parts.count() >= 2 )
        {
          const QString name = parts.first();
          const QString value = parts.mid( 1 ).join( '=' );
          if ( params.contains( name ) )
          {
            // parameter specified multiple times, store all of them in a list...
            if ( params.value( name ).type() == QVariant::StringList )
            {
              // append to existing list
              QStringList listValue = params.value( name ).toStringList();
              listValue << value;
              params.insert( name, listValue );
            }
            else
            {
              // upgrade previous value to list
              QStringList listValue = QStringList() << params.value( name ).toString()
                                                    << value;
              params.insert( name, listValue );
            }
          }
          else
          {
            params.insert( name, value );
          }
        }
      }
    }

    return execute( algId, params, ellipsoid, distanceUnit, areaUnit, logLevel, projectPath );
  }
  else
  {
    std::cerr << u"Command %1 not known!\n"_s.arg( command ).toLocal8Bit().constData();
  }
  return 1;
}

void QgsProcessingExec::showUsage( const QString &appName )
{
  QStringList msg;

  msg << "QGIS Processing Executor - " << VERSION << " '" << RELEASE_NAME << "' ("
      << Qgis::version() << ")\n"
      << "Usage: " << appName << " [--help] [--version] [--json] [--verbose] [--no-python] [--skip-loading-plugins] [command] [algorithm id, path to model file, or path to Python script] [parameters]\n"
      << "\nOptions:\n"
      << "\t--help or -h\t\tOutput the help\n"
      << "\t--version or -v\t\tOutput all versions related to QGIS Process\n"
      << "\t--json\t\t\tOutput results as JSON objects\n"
      << "\t--verbose\t\tOutput verbose logs\n"
      << "\t--no-python\t\tDisable Python support (results in faster startup)\n"
      << "\t--skip-loading-plugins\tAvoid loading enabled plugins (results in faster startup)\n"
      << "Available commands:\n"
      << "\tplugins\t\tlist available and active plugins\n"
      << "\tplugins enable\tenables an installed plugin. The plugin name must be specified, e.g. \"plugins enable cartography_tools\"\n"
      << "\tplugins disable\tdisables an installed plugin. The plugin name must be specified, e.g. \"plugins disable cartography_tools\"\n"
      << "\tlist\t\tlist all available processing algorithms\n"
      << "\thelp\t\tshow help for an algorithm. The algorithm id or a path to a model file must be specified.\n"
      << "\trun\t\truns an algorithm. The algorithm id or a path to a model file and parameter values must be specified. Parameter values are specified after -- with PARAMETER=VALUE syntax. Ordered list values for a parameter can be created by specifying the parameter multiple times, e.g. --LAYERS=layer1.shp --LAYERS=layer2.shp\n"
      << "\t\t\tAlternatively, a '-' character in place of the parameters argument indicates that the parameters should be read from STDIN as a JSON object. The JSON should be structured as a map containing at least the \"inputs\" key specifying a map of input parameter values. This implies the --json option for output as a JSON object.\n"
      << "\t\t\tIf required, the ellipsoid to use for distance and area calculations can be specified via the \"--ELLIPSOID=name\" argument.\n"
      << "\t\t\tIf required, an existing QGIS project to use during the algorithm execution can be specified via the \"--PROJECT_PATH=path\" argument.\n"
      << "\t\t\tWhen passing parameters as a JSON object from STDIN, these extra arguments can be provided as an \"ellipsoid\" and a \"project_path\" key respectively.\n";

  std::cout << msg.join( QString() ).toLocal8Bit().constData();
}

void QgsProcessingExec::showVersionInformation()
{
  std::cout << QgsCommandLineUtils::allVersions().toStdString();
}

void QgsProcessingExec::loadPlugins()
{
#ifdef WITH_BINDINGS
  if ( !mPythonUtils )
    return;

  QgsSettings settings;
  // load plugins
  const QStringList plugins = mPythonUtils->pluginList();
  for ( const QString &plugin : plugins )
  {
    if ( plugin == "processing"_L1 || ( mPythonUtils->isPluginEnabled( plugin ) && mPythonUtils->pluginHasProcessingProvider( plugin ) ) )
    {
      if ( !mPythonUtils->loadPlugin( plugin ) )
      {
        std::cerr << "error loading plugin: " << plugin.toLocal8Bit().constData() << "\n\n";
      }
      else if ( !mPythonUtils->startProcessingPlugin( plugin ) )
      {
        std::cerr << "error starting plugin: " << plugin.toLocal8Bit().constData() << "\n\n";
      }
    }
  }

  if ( !mPythonUtils->finalizeProcessingStartup() )
  {
    std::cerr << "error finalizing Processing plugin startup\n\n";
  }

#endif
}

void QgsProcessingExec::listAlgorithms()
{
  QVariantMap json;
  if ( !( mFlags & Flag::UseJson ) )
  {
    std::cout << "Available algorithms\n\n";
  }
  else
  {
    addVersionInformation( json );
  }

  const QList<QgsProcessingProvider *> providers = QgsApplication::processingRegistry()->providers();
  QVariantMap jsonProviders;
  for ( QgsProcessingProvider *provider : providers )
  {
    QVariantMap providerJson;

    if ( !( mFlags & Flag::UseJson ) )
    {
      std::cout << provider->name().toLocal8Bit().constData() << "\n";
    }
    else
    {
      addProviderInformation( providerJson, provider );
    }
    QVariantMap algorithmsJson;
    const QList<const QgsProcessingAlgorithm *> algorithms = provider->algorithms();
    for ( const QgsProcessingAlgorithm *algorithm : algorithms )
    {
      if ( algorithm->flags() & Qgis::ProcessingAlgorithmFlag::NotAvailableInStandaloneTool )
        continue;

      if ( !( mFlags & Flag::UseJson ) )
      {
        if ( algorithm->flags() & Qgis::ProcessingAlgorithmFlag::Deprecated )
          continue;
        std::cout << "\t" << algorithm->id().toLocal8Bit().constData() << "\t" << algorithm->displayName().toLocal8Bit().constData() << "\n";
      }
      else
      {
        QVariantMap algorithmJson;
        addAlgorithmInformation( algorithmJson, algorithm );
        algorithmsJson.insert( algorithm->id(), algorithmJson );
      }
    }

    if ( !( mFlags & Flag::UseJson ) )
    {
      std::cout << "\n";
    }
    else
    {
      providerJson.insert( u"algorithms"_s, algorithmsJson );
      jsonProviders.insert( provider->id(), providerJson );
    }
  }

  if ( mFlags & Flag::UseJson )
  {
    json.insert( u"providers"_s, jsonProviders );
    std::cout << QgsJsonUtils::jsonFromVariant( json ).dump( 2 );
  }
}

void QgsProcessingExec::listPlugins( bool useJson, bool showLoaded )
{
  QVariantMap json;

  if ( !useJson )
  {
    std::cout << "Available plugins\n";
    if ( showLoaded )
      std::cout << "(* indicates loaded plugins which implement Processing providers)\n\n";
    else
      std::cout << "(* indicates enabled plugins which implement Processing providers)\n\n";
  }
  else
  {
    addVersionInformation( json );
  }

#ifdef WITH_BINDINGS
  QVariantMap jsonPlugins;
  if ( mPythonUtils )
  {
    const QStringList plugins = mPythonUtils->pluginList();
    for ( const QString &plugin : plugins )
    {
      if ( !mPythonUtils->pluginHasProcessingProvider( plugin ) )
        continue;

      if ( !useJson )
      {
        if ( showLoaded ? mPythonUtils->isPluginLoaded( plugin ) : mPythonUtils->isPluginEnabled( plugin ) )
          std::cout << "* ";
        else
          std::cout << "  ";
        std::cout << plugin.toLocal8Bit().constData() << "\n";
      }
      else
      {
        QVariantMap jsonPlugin;
        jsonPlugin.insert( u"loaded"_s, showLoaded ? mPythonUtils->isPluginLoaded( plugin ) : mPythonUtils->isPluginEnabled( plugin ) );
        jsonPlugins.insert( plugin, jsonPlugin );
      }
    }
  }

  if ( useJson )
  {
    json.insert( u"plugins"_s, jsonPlugins );
    std::cout << QgsJsonUtils::jsonFromVariant( json ).dump( 2 );
  }
#endif
}

int QgsProcessingExec::enablePlugin( const QString &name, bool enabled )
{
  if ( enabled )
    std::cout << u"Enabling plugin: \"%1\"\n"_s.arg( name ).toLocal8Bit().constData();
  else
    std::cout << u"Disabling plugin: \"%1\"\n"_s.arg( name ).toLocal8Bit().constData();

#ifdef WITH_BINDINGS
  if ( !mPythonUtils )
  {
    std::cerr << "\nPython not available!";
    return 1;
  }

  const QStringList plugins = mPythonUtils->pluginList();
  if ( !plugins.contains( name ) )
  {
    std::cerr << "\nNo matching plugins found!\n\n";
    listPlugins( false, false );
    return 1;
  }

  if ( enabled && !mPythonUtils->pluginHasProcessingProvider( name ) )
    std::cout << "WARNING: Plugin does not report having a Processing provider, but enabling anyway.\n\n"
                 "Either the plugin does not support Processing, or the plugin's metadata is incorrect.\n"
                 "See https://docs.qgis.org/latest/en/docs/pyqgis_developer_cookbook/processing.html#updating-a-plugin for\n"
                 "instructions on how to fix the plugin metadata to remove this warning.\n\n";

  if ( enabled && mPythonUtils->isPluginEnabled( name ) )
  {
    std::cerr << "Plugin is already enabled!\n";
    return 1;
  }
  else if ( !enabled && !mPythonUtils->isPluginEnabled( name ) )
  {
    std::cerr << "Plugin is already disabled!\n";
    return 1;
  }

  QgsSettings settings;
  if ( enabled )
  {
    if ( !mPythonUtils->loadPlugin( name ) )
    {
      std::cerr << "error loading plugin: " << name.toLocal8Bit().constData() << "\n\n";
      return 1;
    }
    else if ( !mPythonUtils->startProcessingPlugin( name ) )
    {
      std::cerr << "error starting plugin: " << name.toLocal8Bit().constData() << "\n\n";
      return 1;
    }

    const QString pluginName = mPythonUtils->getPluginMetadata( name, u"name"_s );
    settings.setValue( "/PythonPlugins/" + name, true );

    std::cout << u"Enabled %1 (%2)\n\n"_s.arg( name, pluginName ).toLocal8Bit().constData();

    settings.remove( "/PythonPlugins/watchDog/" + name );
  }
  else
  {
    if ( mPythonUtils->isPluginLoaded( name ) )
      mPythonUtils->unloadPlugin( name );

    settings.setValue( "/PythonPlugins/" + name, false );

    std::cout << u"Disabled %1\n\n"_s.arg( name ).toLocal8Bit().constData();
  }
  listPlugins( false, false );

  return 0;
#else
  std::cerr << "No Python support\n";
  return 1;
#endif
}

int QgsProcessingExec::showAlgorithmHelp( const QString &inputId )
{
  QString id = inputId;

  std::unique_ptr<QgsProcessingModelAlgorithm> model;
  const QgsProcessingAlgorithm *alg = nullptr;
  if ( QFile::exists( id ) && QFileInfo( id ).suffix() == "model3"_L1 )
  {
    model = std::make_unique<QgsProcessingModelAlgorithm>();
    if ( !model->fromFile( id ) )
    {
      std::cerr << u"File %1 is not a valid Processing model!\n"_s.arg( id ).toLocal8Bit().constData();
      return 1;
    }

    alg = model.get();
  }
#ifdef WITH_BINDINGS
  else if ( mPythonUtils && QFile::exists( id ) && QFileInfo( id ).suffix() == "py"_L1 )
  {
    QString res;
    if ( !mPythonUtils->evalString( u"qgis.utils.import_script_algorithm(\"%1\")"_s.arg( id ), res ) || res.isEmpty() )
    {
      std::cerr << u"File %1 is not a valid Processing script!\n"_s.arg( id ).toLocal8Bit().constData();
      return 1;
    }

    id = res;
  }
#endif

  if ( !alg )
  {
    alg = QgsApplication::processingRegistry()->algorithmById( id );
    if ( !alg )
    {
      std::cerr << u"Algorithm %1 not found!\n"_s.arg( id ).toLocal8Bit().constData();
      return 1;
    }
  }

  if ( alg->flags() & Qgis::ProcessingAlgorithmFlag::NotAvailableInStandaloneTool )
  {
    std::cerr << u"The \"%1\" algorithm is not available for use outside of the QGIS desktop application\n"_s.arg( id ).toLocal8Bit().constData();
    return 1;
  }

  QVariantMap json;
  if ( !( mFlags & Flag::UseJson ) )
  {
    std::cout << u"%1 (%2)\n"_s.arg( alg->displayName(), alg->id() ).toLocal8Bit().constData();

    std::cout << "\n----------------\n";
    std::cout << "Description\n";
    std::cout << "----------------\n";

    if ( const QgsProcessingModelAlgorithm *model = dynamic_cast<const QgsProcessingModelAlgorithm *>( alg ) )
    {
      // show finer help content for models
      const QVariantMap help = model->helpContent();
      std::cout << help.value( u"ALG_DESC"_s ).toString().toLocal8Bit().constData() << '\n';

      if ( !help.value( u"ALG_CREATOR"_s ).toString().isEmpty() || !help.value( u"ALG_VERSION"_s ).toString().isEmpty() )
        std::cout << '\n';

      if ( !help.value( u"ALG_CREATOR"_s ).toString().isEmpty() )
        std::cout << "Algorithm author:\t" << help.value( u"ALG_CREATOR"_s ).toString().toLocal8Bit().constData() << '\n';
      if ( !help.value( u"ALG_VERSION"_s ).toString().isEmpty() )
        std::cout << "Algorithm version:\t" << help.value( u"ALG_VERSION"_s ).toString().toLocal8Bit().constData() << '\n';

      if ( !help.value( u"EXAMPLES"_s ).toString().isEmpty() )
      {
        std::cout << "\n----------------\n";
        std::cout << "Examples\n";
        std::cout << "----------------\n";
        std::cout << help.value( u"EXAMPLES"_s ).toString().toLocal8Bit().constData() << '\n';
      }
    }
    else
    {
      if ( !alg->shortDescription().isEmpty() )
        std::cout << alg->shortDescription().toLocal8Bit().constData() << '\n';
      if ( !alg->shortHelpString().isEmpty() && alg->shortHelpString() != alg->shortDescription() )
        std::cout << alg->shortHelpString().toLocal8Bit().constData() << '\n';
    }

    if ( alg->documentationFlags() != Qgis::ProcessingAlgorithmDocumentationFlags() )
    {
      std::cout << "\n----------------\n";
      std::cout << "Notes\n";
      std::cout << "----------------\n\n";

      for ( Qgis::ProcessingAlgorithmDocumentationFlag flag : qgsEnumList<Qgis::ProcessingAlgorithmDocumentationFlag>() )
      {
        if ( alg->documentationFlags() & flag )
        {
          std::cout << " - " << QgsProcessing::documentationFlagToString( flag ).toUtf8().constData();
        }
      }

      std::cout << "\n";
    }

    std::cout << "\n----------------\n";
    std::cout << "Arguments\n";
    std::cout << "----------------\n\n";
  }
  else
  {
    addVersionInformation( json );

    QVariantMap algorithmDetails;
    algorithmDetails.insert( u"id"_s, alg->id() );
    addAlgorithmInformation( algorithmDetails, alg );
    json.insert( u"algorithm_details"_s, algorithmDetails );
    QVariantMap providerJson;
    if ( alg->provider() )
      addProviderInformation( providerJson, alg->provider() );
    json.insert( u"provider_details"_s, providerJson );
  }

  QgsProcessingContext context;
  QVariantMap parametersJson;
  const QgsProcessingParameterDefinitions defs = alg->parameterDefinitions();
  for ( const QgsProcessingParameterDefinition *p : defs )
  {
    if ( p->flags() & Qgis::ProcessingParameterFlag::Hidden )
      continue;

    QVariantMap parameterJson;

    if ( !( mFlags & Flag::UseJson ) )
    {
      QString line = u"%1: %2"_s.arg( p->name(), p->description() );
      if ( p->flags() & Qgis::ProcessingParameterFlag::Optional )
        line += " (optional)"_L1;
      std::cout << u"%1\n"_s.arg( line ).toLocal8Bit().constData();

      if ( p->defaultValue().isValid() )
      {
        bool ok = false;
        std::cout << u"\tDefault value:\t%1\n"_s.arg( p->valueAsString( p->defaultValue(), context, ok ) ).toLocal8Bit().constData();
      }
    }
    else
    {
      parameterJson.insert( u"name"_s, p->name() );
      parameterJson.insert( u"description"_s, p->description() );


      if ( const QgsProcessingParameterType *type = QgsApplication::processingRegistry()->parameterType( p->type() ) )
      {
        QVariantMap typeDetails;
        typeDetails.insert( u"id"_s, type->id() );
        typeDetails.insert( u"name"_s, type->name() );
        typeDetails.insert( u"description"_s, type->description() );
        typeDetails.insert( u"metadata"_s, type->metadata() );
        typeDetails.insert( u"acceptable_values"_s, type->acceptedStringValues() );

        parameterJson.insert( u"type"_s, typeDetails );
      }
      else
      {
        parameterJson.insert( u"type"_s, p->type() );
      }

      parameterJson.insert( u"is_destination"_s, p->isDestination() );
      parameterJson.insert( u"default_value"_s, p->defaultValue() );
      parameterJson.insert( u"optional"_s, bool( p->flags() & Qgis::ProcessingParameterFlag::Optional ) );
      parameterJson.insert( u"is_advanced"_s, bool( p->flags() & Qgis::ProcessingParameterFlag::Advanced ) );

      parameterJson.insert( u"raw_definition"_s, p->toVariantMap() );
    }

    if ( !p->help().isEmpty() )
    {
      if ( !( mFlags & Flag::UseJson ) )
        std::cout << u"\t%1\n"_s.arg( p->help() ).toLocal8Bit().constData();
      else
        parameterJson.insert( u"help"_s, p->help() );
    }
    if ( !( mFlags & Flag::UseJson ) )
      std::cout << u"\tArgument type:\t%1\n"_s.arg( p->type() ).toLocal8Bit().constData();

    if ( p->type() == QgsProcessingParameterEnum::typeName() )
    {
      const QgsProcessingParameterEnum *enumParam = static_cast<const QgsProcessingParameterEnum *>( p );
      QStringList options;
      QVariantMap jsonOptions;
      for ( int i = 0; i < enumParam->options().count(); ++i )
      {
        options << u"\t\t- %1: %2"_s.arg( i ).arg( enumParam->options().at( i ) );
        jsonOptions.insert( QString::number( i ), enumParam->options().at( i ) );
      }

      if ( !( mFlags & Flag::UseJson ) )
        std::cout << u"\tAvailable values:\n%1\n"_s.arg( options.join( '\n' ) ).toLocal8Bit().constData();
      else
        parameterJson.insert( u"available_options"_s, jsonOptions );
    }

    // acceptable command line values
    if ( !( mFlags & Flag::UseJson ) )
    {
      if ( const QgsProcessingParameterType *type = QgsApplication::processingRegistry()->parameterType( p->type() ) )
      {
        const QStringList values = type->acceptedStringValues();
        if ( !values.isEmpty() )
        {
          std::cout << "\tAcceptable values:\n";
          for ( const QString &val : values )
          {
            std::cout << u"\t\t- %1"_s.arg( val ).toLocal8Bit().constData() << "\n";
          }
        }
      }
    }

    parametersJson.insert( p->name(), parameterJson );
  }

  QVariantMap outputsJson;
  if ( !( mFlags & Flag::UseJson ) )
  {
    std::cout << "\n----------------\n";
    std::cout << "Outputs\n";
    std::cout << "----------------\n\n";
  }
  const QgsProcessingOutputDefinitions outputs = alg->outputDefinitions();
  for ( const QgsProcessingOutputDefinition *o : outputs )
  {
    QVariantMap outputJson;
    if ( !( mFlags & Flag::UseJson ) )
    {
      std::cout << u"%1: <%2>\n"_s.arg( o->name(), o->type() ).toLocal8Bit().constData();
      if ( !o->description().isEmpty() )
        std::cout << "\t" << o->description().toLocal8Bit().constData() << '\n';
    }
    else
    {
      outputJson.insert( u"description"_s, o->description() );
      outputJson.insert( u"type"_s, o->type() );
      outputsJson.insert( o->name(), outputJson );
    }
  }

  if ( !( mFlags & Flag::UseJson ) )
  {
    std::cout << "\n\n";
  }
  else
  {
    json.insert( u"parameters"_s, parametersJson );
    json.insert( u"outputs"_s, outputsJson );
    std::cout << QgsJsonUtils::jsonFromVariant( json ).dump( 2 );
  }

  return 0;
}

int QgsProcessingExec::execute( const QString &inputId, const QVariantMap &inputs, const QString &ellipsoid, Qgis::DistanceUnit distanceUnit, Qgis::AreaUnit areaUnit, Qgis::ProcessingLogLevel logLevel, const QString &projectPath )
{
  QVariantMap json;
  if ( mFlags & Flag::UseJson )
  {
    addVersionInformation( json );
  }

  bool ok = false;
  QString error;
  const QVariantMap params = QgsProcessingUtils::preprocessQgisProcessParameters( inputs, ok, error );
  if ( !ok )
  {
    std::cerr << error.toLocal8Bit().constData();
    return 1;
  }

  QString id = inputId;

  std::unique_ptr<QgsProcessingModelAlgorithm> model;
  const QgsProcessingAlgorithm *alg = nullptr;
  if ( QFile::exists( id ) && QFileInfo( id ).suffix() == "model3"_L1 )
  {
    model = std::make_unique<QgsProcessingModelAlgorithm>();
    if ( !model->fromFile( id ) )
    {
      std::cerr << u"File %1 is not a valid Processing model!\n"_s.arg( id ).toLocal8Bit().constData();
      return 1;
    }

    alg = model.get();
  }
#ifdef WITH_BINDINGS
  else if ( mPythonUtils && QFile::exists( id ) && QFileInfo( id ).suffix() == "py"_L1 )
  {
    QString res;
    if ( !mPythonUtils->evalString( u"qgis.utils.import_script_algorithm(\"%1\")"_s.arg( id ), res ) || res.isEmpty() )
    {
      std::cerr << u"File %1 is not a valid Processing script!\n"_s.arg( id ).toLocal8Bit().constData();
      return 1;
    }

    id = res;
  }
#endif

  if ( !alg )
  {
    alg = QgsApplication::processingRegistry()->algorithmById( id );
    if ( !alg )
    {
      std::cerr << u"Algorithm %1 not found!\n"_s.arg( id ).toLocal8Bit().constData();
      return 1;
    }

    if ( alg->flags() & Qgis::ProcessingAlgorithmFlag::NotAvailableInStandaloneTool )
    {
      std::cerr << u"The \"%1\" algorithm is not available for use outside of the QGIS desktop application\n"_s.arg( id ).toLocal8Bit().constData();
      return 1;
    }

    if ( !( mFlags & Flag::UseJson ) && alg->flags() & Qgis::ProcessingAlgorithmFlag::KnownIssues )
    {
      std::cout << "\n****************\n";
      std::cout << "Warning: this algorithm contains known issues and the results may be unreliable!\n";
      std::cout << "****************\n\n";
    }

    if ( !( mFlags & Flag::UseJson ) && alg->flags() & Qgis::ProcessingAlgorithmFlag::Deprecated )
    {
      std::cout << "\n****************\n";
      std::cout << "Warning: this algorithm is deprecated and may be removed in a future QGIS version!\n";
      std::cout << "****************\n\n";
    }

    if ( alg->flags() & Qgis::ProcessingAlgorithmFlag::RequiresProject && projectPath.isEmpty() )
    {
      std::cerr << u"The \"%1\" algorithm requires a QGIS project to execute. Specify a path to an existing project with the \"--PROJECT_PATH=xxx\" argument.\n"_s.arg( id ).toLocal8Bit().constData();
      return 1;
    }
  }

  if ( mFlags & Flag::UseJson )
  {
    QVariantMap algorithmDetails;
    algorithmDetails.insert( u"id"_s, alg->id() );
    addAlgorithmInformation( algorithmDetails, alg );
    json.insert( u"algorithm_details"_s, algorithmDetails );

    if ( alg->provider() )
    {
      QVariantMap providerJson;
      addProviderInformation( providerJson, alg->provider() );
      json.insert( u"provider_details"_s, providerJson );
    }
  }

  QgsProject *project = nullptr;
  if ( !projectPath.isEmpty() )
  {
    project = QgsProject::instance();
    if ( !project->read( projectPath ) )
    {
      std::cerr << u"Could not load the QGIS project \"%1\"\n"_s.arg( projectPath ).toLocal8Bit().constData();
      return 1;
    }
    json.insert( u"project_path"_s, projectPath );
  }

  if ( !( mFlags & Flag::UseJson ) )
  {
    std::cout << "\n----------------\n";
    std::cout << "Inputs\n";
    std::cout << "----------------\n\n";
  }
  QVariantMap inputsJson;
  for ( auto it = inputs.constBegin(); it != inputs.constEnd(); ++it )
  {
    if ( !( mFlags & Flag::UseJson ) )
      std::cout << it.key().toLocal8Bit().constData() << ":\t" << it.value().toString().toLocal8Bit().constData() << '\n';
    else
      inputsJson.insert( it.key(), it.value() );
  }
  if ( !( mFlags & Flag::UseJson ) )
    std::cout << "\n";
  else
    json.insert( u"inputs"_s, inputsJson );

  if ( !ellipsoid.isEmpty() )
  {
    if ( !( mFlags & Flag::UseJson ) )
      std::cout << "Using ellipsoid:\t" << ellipsoid.toLocal8Bit().constData() << '\n';
    else
      json.insert( u"ellipsoid"_s, ellipsoid );
  }
  if ( distanceUnit != Qgis::DistanceUnit::Unknown )
  {
    if ( !( mFlags & Flag::UseJson ) )
      std::cout << "Using distance unit:\t" << QgsUnitTypes::toString( distanceUnit ).toLocal8Bit().constData() << '\n';
    else
      json.insert( u"distance_unit"_s, QgsUnitTypes::toString( distanceUnit ) );
  }
  if ( areaUnit != Qgis::AreaUnit::Unknown )
  {
    if ( !( mFlags & Flag::UseJson ) )
      std::cout << "Using area unit:\t" << QgsUnitTypes::toString( areaUnit ).toLocal8Bit().constData() << '\n';
    else
      json.insert( u"area_unit"_s, QgsUnitTypes::toString( areaUnit ) );
  }


  QgsProcessingContext context;
  context.setEllipsoid( ellipsoid );
  context.setDistanceUnit( distanceUnit );
  context.setAreaUnit( areaUnit );
  if ( project )
    context.setProject( project );
  context.setLogLevel( logLevel );

  const QgsProcessingParameterDefinitions defs = alg->parameterDefinitions();
  QList<const QgsProcessingParameterDefinition *> missingParams;
  for ( const QgsProcessingParameterDefinition *p : defs )
  {
    if ( !p->checkValueIsAcceptable( params.value( p->name() ), &context ) )
    {
      if ( !( p->flags() & Qgis::ProcessingParameterFlag::Optional ) && !params.contains( p->name() ) )
      {
        missingParams << p;
      }
    }
  }

  if ( !missingParams.isEmpty() )
  {
    std::cerr << u"ERROR: The following mandatory parameters were not specified\n\n"_s.toLocal8Bit().constData();
    for ( const QgsProcessingParameterDefinition *p : std::as_const( missingParams ) )
    {
      std::cerr << u"\t%1:\t%2\n"_s.arg( p->name(), p->description() ).toLocal8Bit().constData();
    }

    return 1;
  }

  QString message;
  if ( !alg->checkParameterValues( params, context, &message ) )
  {
    std::cerr << u"ERROR:\tAn error was encountered while checking parameter values\n"_s.toLocal8Bit().constData();
    std::cerr << u"\t%1\n"_s.arg( message ).toLocal8Bit().constData();
    return 1;
  }

  ConsoleFeedback feedback( mFlags & Flag::UseJson );

#if defined( Q_OS_UNIX ) && !defined( Q_OS_ANDROID )
  UnixSignalWatcher sigwatch;
  sigwatch.watchForSignal( SIGINT );

  QObject::connect( &sigwatch, &UnixSignalWatcher::unixSignal, &feedback, [&feedback]( int signal ) {
    switch ( signal )
    {
      case SIGINT:
        feedback.cancel();
        break;

      default:
        break;
    }
  } );
#endif

  ok = false;
  if ( !( mFlags & Flag::UseJson ) )
    std::cout << "\n";

  QVariantMap res = alg->run( params, context, &feedback, &ok );

  if ( ok )
  {
    QVariantMap resultsJson;
    if ( !( mFlags & Flag::UseJson ) )
    {
      std::cout << "\n----------------\n";
      std::cout << "Results\n";
      std::cout << "----------------\n\n";
    }
    else
    {
      json.insert( u"log"_s, feedback.jsonLog() );
    }

    for ( auto it = res.constBegin(); it != res.constEnd(); ++it )
    {
      if ( it.key() == "CHILD_INPUTS"_L1 || it.key() == "CHILD_RESULTS"_L1 )
        continue;

      QVariant result = it.value();
      if ( !( mFlags & Flag::UseJson ) )
      {
        if ( result.type() == QVariant::List || result.type() == QVariant::StringList )
        {
          QStringList list;
          for ( const QVariant &v : result.toList() )
            list << v.toString();
          result = list.join( ", " );
        }
        std::cout << it.key().toLocal8Bit().constData() << ":\t" << result.toString().toLocal8Bit().constData() << '\n';
      }
      else
      {
        resultsJson.insert( it.key(), result );
      }
    }

    if ( mFlags & Flag::UseJson )
    {
      json.insert( u"results"_s, resultsJson );
      std::cout << QgsJsonUtils::jsonFromVariant( json ).dump( 2 );
    }
    return 0;
  }
  else
  {
    return 1;
  }
}

void QgsProcessingExec::addVersionInformation( QVariantMap &json )
{
  json.insert( u"qgis_version"_s, Qgis::version() );
  if ( QString( Qgis::devVersion() ) != "exported"_L1 )
  {
    json.insert( u"qgis_code_revision"_s, Qgis::devVersion() );
  }
  json.insert( u"qt_version"_s, qVersion() );
  json.insert( u"python_version"_s, PYTHON_VERSION );
  json.insert( u"gdal_version"_s, GDALVersionInfo( "RELEASE_NAME" ) );
  json.insert( u"geos_version"_s, GEOSversion() );

  PJ_INFO info = proj_info();
  json.insert( u"proj_version"_s, info.release );

#ifdef WITH_SFCGAL
  json.insert( u"sfcgal_version"_s, sfcgal_version() );
#else
  json.insert( u"sfcgal_version"_s, "no support" );
#endif

#ifdef WITH_GEOGRAPHICLIB
  json.insert( u"geographiclib_version"_s, u"%1.%2.%3"_s.arg( GEOGRAPHICLIB_VERSION_MAJOR ).arg( GEOGRAPHICLIB_VERSION_MINOR ).arg( GEOGRAPHICLIB_VERSION_PATCH ) );
#else
  json.insert( u"geographiclib_version"_s, "no support" );
#endif
}

void QgsProcessingExec::addAlgorithmInformation( QVariantMap &algorithmJson, const QgsProcessingAlgorithm *algorithm )
{
  algorithmJson.insert( u"name"_s, algorithm->displayName() );
  algorithmJson.insert( u"short_description"_s, algorithm->shortDescription() );
  algorithmJson.insert( u"tags"_s, algorithm->tags() );
  algorithmJson.insert( u"help_url"_s, algorithm->helpUrl() );
  algorithmJson.insert( u"group"_s, algorithm->group() );
  algorithmJson.insert( u"can_cancel"_s, bool( algorithm->flags() & Qgis::ProcessingAlgorithmFlag::CanCancel ) );
  algorithmJson.insert( u"requires_matching_crs"_s, bool( algorithm->flags() & Qgis::ProcessingAlgorithmFlag::RequiresMatchingCrs ) );
  algorithmJson.insert( u"has_known_issues"_s, bool( algorithm->flags() & Qgis::ProcessingAlgorithmFlag::KnownIssues ) );
  algorithmJson.insert( u"deprecated"_s, bool( algorithm->flags() & Qgis::ProcessingAlgorithmFlag::Deprecated ) );

  if ( algorithm->documentationFlags() != Qgis::ProcessingAlgorithmDocumentationFlags() )
  {
    QStringList documentationFlags;
    for ( Qgis::ProcessingAlgorithmDocumentationFlag flag : qgsEnumList<Qgis::ProcessingAlgorithmDocumentationFlag>() )
    {
      if ( algorithm->documentationFlags() & flag )
      {
        switch ( flag )
        {
          case Qgis::ProcessingAlgorithmDocumentationFlag::RegeneratesPrimaryKey:
            documentationFlags << u"regenerates_primary_key"_s;
            break;
          case Qgis::ProcessingAlgorithmDocumentationFlag::RegeneratesPrimaryKeyInSomeScenarios:
            documentationFlags << u"regenerates_primary_key_in_some_scenarios"_s;
            break;
          case Qgis::ProcessingAlgorithmDocumentationFlag::RespectsEllipsoid:
            documentationFlags << u"respects_ellipsoid"_s;
            break;
        }
        algorithmJson.insert( u"documentation_flags"_s, documentationFlags );
      }
    }
  }
}

void QgsProcessingExec::addProviderInformation( QVariantMap &providerJson, QgsProcessingProvider *provider )
{
  providerJson.insert( u"name"_s, provider->name() );
  providerJson.insert( u"long_name"_s, provider->longName() );
  providerJson.insert( u"version"_s, provider->versionInfo() );
  providerJson.insert( u"can_be_activated"_s, provider->canBeActivated() );
  if ( !provider->warningMessage().isEmpty() )
  {
    providerJson.insert( u"warning"_s, provider->warningMessage() );
  }
  providerJson.insert( u"is_active"_s, provider->isActive() );
  providerJson.insert( u"supported_output_raster_extensions"_s, provider->supportedOutputRasterLayerExtensions() );
  providerJson.insert( u"supported_output_vector_extensions"_s, provider->supportedOutputVectorLayerExtensions() );
  providerJson.insert( u"supported_output_table_extensions"_s, provider->supportedOutputTableExtensions() );
  providerJson.insert( u"default_vector_file_extension"_s, provider->defaultVectorFileExtension() );
  providerJson.insert( u"default_raster_file_format"_s, provider->defaultRasterFileFormat() );
  providerJson.insert( u"supports_non_file_based_output"_s, provider->supportsNonFileBasedOutput() );
}
