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

#include "qgscommandlineutils.h"
#include "qgsprocess.h"
#include "qgsprocessingregistry.h"
#include "qgsprocessingalgorithm.h"
#include "qgsnativealgorithms.h"
#ifdef HAVE_3D
#include "qgs3dalgorithms.h"
#endif
#include "qgssettings.h"
#include "qgsapplication.h"
#include "qgsprocessingparametertype.h"
#include "processing/models/qgsprocessingmodelalgorithm.h"
#include "qgsproject.h"
#include "qgsgeos.h"
#include "qgsjsonutils.h"

#if defined(Q_OS_UNIX) && !defined(Q_OS_ANDROID)
#include "sigwatch.h"
#endif

#include <iostream>
#include <string>
#include <QObject>
#include <QLibrary>

#include <ogr_api.h>
#include <gdal_version.h>
#include <proj.h>
#include <json.hpp>

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
    if ( !mJsonLog.contains( QStringLiteral( "errors" ) ) )
      mJsonLog.insert( QStringLiteral( "errors" ), QStringList() );
    mJsonLog[ QStringLiteral( "errors" )] = mJsonLog.value( QStringLiteral( "errors" ) ).toStringList() << error;
  }
  QgsProcessingFeedback::reportError( error, fatalError );
}

void ConsoleFeedback::pushWarning( const QString &warning )
{
  if ( !mUseJson )
    std::cout << "WARNING:\t" << warning.toLocal8Bit().constData() << '\n';
  else
  {
    if ( !mJsonLog.contains( QStringLiteral( "warning" ) ) )
      mJsonLog.insert( QStringLiteral( "warning" ), QStringList() );
    mJsonLog[ QStringLiteral( "warning" )] = mJsonLog.value( QStringLiteral( "warning" ) ).toStringList() << warning;
  }
  QgsProcessingFeedback::pushWarning( warning );
}

void ConsoleFeedback::pushInfo( const QString &info )
{
  if ( !mUseJson )
    std::cout << info.toLocal8Bit().constData() << '\n';
  else
  {
    if ( !mJsonLog.contains( QStringLiteral( "info" ) ) )
      mJsonLog.insert( QStringLiteral( "info" ), QStringList() );
    mJsonLog[ QStringLiteral( "info" )] = mJsonLog.value( QStringLiteral( "info" ) ).toStringList() << info;
  }
  QgsProcessingFeedback::pushInfo( info );
}

void ConsoleFeedback::pushCommandInfo( const QString &info )
{
  if ( !mUseJson )
    std::cout << info.toLocal8Bit().constData() << '\n';
  else
  {
    if ( !mJsonLog.contains( QStringLiteral( "info" ) ) )
      mJsonLog.insert( QStringLiteral( "info" ), QStringList() );
    mJsonLog[ QStringLiteral( "info" )] = mJsonLog.value( QStringLiteral( "info" ) ).toStringList() << info;
  }
  QgsProcessingFeedback::pushCommandInfo( info );
}

void ConsoleFeedback::pushDebugInfo( const QString &info )
{
  if ( !mUseJson )
    std::cout << info.toLocal8Bit().constData() << '\n';
  else
  {
    if ( !mJsonLog.contains( QStringLiteral( "info" ) ) )
      mJsonLog.insert( QStringLiteral( "info" ), QStringList() );
    mJsonLog[ QStringLiteral( "info" )] = mJsonLog.value( QStringLiteral( "info" ) ).toStringList() << info;
  }
  QgsProcessingFeedback::pushDebugInfo( info );
}

void ConsoleFeedback::pushConsoleInfo( const QString &info )
{
  if ( !mUseJson )
    std::cout << info.toLocal8Bit().constData() << '\n';
  else
  {
    if ( !mJsonLog.contains( QStringLiteral( "info" ) ) )
      mJsonLog.insert( QStringLiteral( "info" ), QStringList() );
    mJsonLog[ QStringLiteral( "info" )] = mJsonLog.value( QStringLiteral( "info" ) ).toStringList() << info;
  }
  QgsProcessingFeedback::pushConsoleInfo( info );
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
std::unique_ptr< QgsPythonUtils > QgsProcessingExec::loadPythonSupport()
{
  QString pythonlibName( QStringLiteral( "qgispython" ) );
#if defined(Q_OS_UNIX) && !defined(Q_OS_ANDROID)
  pythonlibName.prepend( QgsApplication::libraryPath() );
#endif
#ifdef __MINGW32__
  pythonlibName.prepend( "lib" );
#endif
  QString version = QStringLiteral( "%1.%2.%3" ).arg( Qgis::versionInt() / 10000 ).arg( Qgis::versionInt() / 100 % 100 ).arg( Qgis::versionInt() % 100 );
  QgsDebugMsg( QStringLiteral( "load library %1 (%2)" ).arg( pythonlibName, version ) );
  QLibrary pythonlib( pythonlibName, version );
  // It's necessary to set these two load hints, otherwise Python library won't work correctly
  // see http://lists.kde.org/?l=pykde&m=117190116820758&w=2
  pythonlib.setLoadHints( QLibrary::ResolveAllSymbolsHint | QLibrary::ExportExternalSymbolsHint );
  if ( !pythonlib.load() )
  {
    pythonlib.setFileName( pythonlibName );
    if ( !pythonlib.load() )
    {
      std::cerr << QStringLiteral( "Couldn't load Python support library: %1\n" ).arg( pythonlib.errorString() ).toLocal8Bit().constData();
      return nullptr;
    }
  }

  typedef QgsPythonUtils*( *inst )();
  inst pythonlib_inst = reinterpret_cast< inst >( cast_to_fptr( pythonlib.resolve( "instance" ) ) );
  if ( !pythonlib_inst )
  {
    //using stderr on purpose because we want end users to see this [TS]
    std::cerr << "Couldn't resolve Python support library's instance() symbol.\n";
    return nullptr;
  }

  std::unique_ptr< QgsPythonUtils > pythonUtils( pythonlib_inst() );
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

int QgsProcessingExec::run( const QStringList &constArgs )
{
  QStringList args = constArgs;
  QObject::connect( QgsApplication::messageLog(), static_cast < void ( QgsMessageLog::* )( const QString &message, const QString &tag, Qgis::MessageLevel level ) >( &QgsMessageLog::messageReceived ), QgsApplication::instance(),
                    [ = ]( const QString & message, const QString &, Qgis::MessageLevel level )
  {
    if ( level == Qgis::MessageLevel::Critical )
    {
      if ( !message.contains( QLatin1String( "DeprecationWarning:" ) ) )
        std::cerr << message.toLocal8Bit().constData() << '\n';
    }
  } );

  const int jsonIndex = args.indexOf( QLatin1String( "--json" ) );
  bool useJson = false;
  if ( jsonIndex >= 0 )
  {
    useJson = true;
    args.removeAt( jsonIndex );
  }

  const int verboseIndex = args.indexOf( QLatin1String( "--verbose" ) );
  QgsProcessingContext::LogLevel logLevel = QgsProcessingContext::DefaultLevel;
  if ( verboseIndex >= 0 )
  {
    logLevel = QgsProcessingContext::Verbose;
    args.removeAt( verboseIndex );
  }

  const int noPythonIndex = args.indexOf( QLatin1String( "--no-python" ) );
  if ( noPythonIndex >= 0 )
  {
    mSkipPython = true;
    args.removeAt( noPythonIndex );
  }

  if ( args.size() == 1 )
  {
    showUsage( args.at( 0 ) );
    return 0;
  }

  // core providers
  QgsApplication::processingRegistry()->addProvider( new QgsNativeAlgorithms( QgsApplication::processingRegistry() ) );
#ifdef HAVE_3D
  QgsApplication::processingRegistry()->addProvider( new Qgs3DAlgorithms( QgsApplication::processingRegistry() ) );
#endif

#ifdef WITH_BINDINGS
  if ( !mSkipPython )
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

  const QString command = args.at( 1 );
  if ( command == QLatin1String( "plugins" ) )
  {
    if ( args.size() == 2 || ( args.size() == 3 && args.at( 2 ) == QLatin1String( "list" ) ) )
    {
      loadPlugins();
      listPlugins( useJson, true );
      return 0;
    }
    else if ( args.size() == 4 && args.at( 2 ) == QLatin1String( "enable" ) )
    {
      return enablePlugin( args.at( 3 ), true );
    }
    else if ( args.size() == 4 && args.at( 2 ) == QLatin1String( "disable" ) )
    {
      return enablePlugin( args.at( 3 ), false );
    }
    std::cerr << QStringLiteral( "Command %1 not known!\n" ).arg( args.value( 2 ) ).toLocal8Bit().constData();
    return 1;
  }
  else if ( command == QLatin1String( "list" ) )
  {
    loadPlugins();
    listAlgorithms( useJson );
    return 0;
  }
  else if ( command == QLatin1String( "--version" ) || command == QLatin1String( "-v" ) )
  {
    std::cout << QgsCommandLineUtils::allVersions().toStdString();
    return 0;
  }
  else if ( command == QLatin1String( "--help" ) || command == QLatin1String( "-h" ) )
  {
    showUsage( args.at( 0 ) );
    return 0;
  }
  else if ( command == QLatin1String( "help" ) )
  {
    if ( args.size() < 3 )
    {
      std::cerr << QStringLiteral( "Algorithm ID or model file not specified\n" ).toLocal8Bit().constData();
      return 1;
    }

    loadPlugins();
    const QString algId = args.at( 2 );
    return showAlgorithmHelp( algId, useJson );
  }
  else if ( command == QLatin1String( "run" ) )
  {
    if ( args.size() < 3 )
    {
      std::cerr << QStringLiteral( "Algorithm ID or model file not specified\n" ).toLocal8Bit().constData();
      return 1;
    }

    loadPlugins();

    const QString algId = args.at( 2 );

    // build parameter map
    QString ellipsoid;
    QgsUnitTypes::DistanceUnit distanceUnit = QgsUnitTypes::DistanceUnknownUnit;
    QgsUnitTypes::AreaUnit areaUnit = QgsUnitTypes::AreaUnknownUnit;
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
        std::cerr << QStringLiteral( "Could not parse JSON parameters: %1" ).arg( error ).toLocal8Bit().constData() << std::endl;
        return 1;
      }
      if ( !json.contains( QStringLiteral( "inputs" ) ) )
      {
        std::cerr << QStringLiteral( "JSON parameters object must contain an \"inputs\" key." ).toLocal8Bit().constData() << std::endl;
        return 1;
      }

      params = json.value( QStringLiteral( "inputs" ) ).toMap();

      // JSON format for input parameters implies JSON output format
      useJson = true;

      ellipsoid = json.value( QStringLiteral( "ellipsoid" ) ).toString();
      projectPath = json.value( QStringLiteral( "project_path" ) ).toString();
      if ( json.contains( "distance_units" ) )
      {
        bool ok = false;
        const QString distanceUnitsString = json.value( QStringLiteral( "distance_units" ) ).toString();
        distanceUnit = QgsUnitTypes::decodeDistanceUnit( distanceUnitsString, &ok );
        if ( !ok )
        {
          std::cerr << QStringLiteral( "%1 is not a valid distance unit value." ).arg( distanceUnitsString ).toLocal8Bit().constData() << std::endl;
          return 1;
        }
      }

      if ( json.contains( "area_units" ) )
      {
        bool ok = false;
        const QString areaUnitsString = json.value( QStringLiteral( "area_units" ) ).toString();
        areaUnit = QgsUnitTypes::decodeAreaUnit( areaUnitsString, &ok );
        if ( !ok )
        {
          std::cerr << QStringLiteral( "%1 is not a valid area unit value." ).arg( areaUnitsString ).toLocal8Bit().constData() << std::endl;
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

        if ( arg == QLatin1String( "--" ) )
        {
          break;
        }

        if ( arg.startsWith( QLatin1String( "--" ) ) )
          arg = arg.mid( 2 );

        const QStringList parts = arg.split( '=' );
        if ( parts.count() >= 2 )
        {
          const QString name = parts.at( 0 );

          if ( name.compare( QLatin1String( "ellipsoid" ), Qt::CaseInsensitive ) == 0 )
          {
            ellipsoid = parts.mid( 1 ).join( '=' );
          }
          else if ( name.compare( QLatin1String( "distance_units" ), Qt::CaseInsensitive ) == 0 )
          {
            distanceUnit = QgsUnitTypes::decodeDistanceUnit( parts.mid( 1 ).join( '=' ) );
          }
          else if ( name.compare( QLatin1String( "area_units" ), Qt::CaseInsensitive ) == 0 )
          {
            areaUnit = QgsUnitTypes::decodeAreaUnit( parts.mid( 1 ).join( '=' ) );
          }
          else if ( name.compare( QLatin1String( "project_path" ), Qt::CaseInsensitive ) == 0 )
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
          std::cerr << QStringLiteral( "Invalid parameter value %1. Parameter values must be entered after \"--\" e.g.\n  Example:\n    qgis_process run algorithm_name -- PARAM1=VALUE PARAM2=42\"\n" ).arg( arg ).toLocal8Bit().constData();
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

    return execute( algId, params, ellipsoid, distanceUnit, areaUnit, logLevel, useJson, projectPath );
  }
  else
  {
    std::cerr << QStringLiteral( "Command %1 not known!\n" ).arg( command ).toLocal8Bit().constData();
  }
  return 1;
}

void QgsProcessingExec::showUsage( const QString &appName )
{
  QStringList msg;

  msg << "QGIS Processing Executor - " << VERSION << " '" << RELEASE_NAME << "' ("
      << Qgis::version() << ")\n"
      << "Usage: " << appName <<  " [--help] [--version] [--json] [--verbose] [--no-python] [command] [algorithm id, path to model file, or path to Python script] [parameters]\n"
      << "\nOptions:\n"
      << "\t--help or -h\t\tOutput the help\n"
      << "\t--version or -v\t\tOutput all versions related to QGIS Process\n"
      << "\t--json\t\tOutput results as JSON objects\n"
      << "\t--verbose\tOutput verbose logs\n"
      << "\t--no-python\tDisable Python support (results in faster startup)"
      << "\nAvailable commands:\n"
      << "\tplugins\t\tlist available and active plugins\n"
      << "\tplugins enable\tenables an installed plugin. The plugin name must be specified, e.g. \"plugins enable cartography_tools\"\n"
      << "\tplugins disable\tdisables an installed plugin. The plugin name must be specified, e.g. \"plugins disable cartography_tools\"\n"
      << "\tlist\t\tlist all available processing algorithms\n"
      << "\thelp\t\tshow help for an algorithm. The algorithm id or a path to a model file must be specified.\n"
      << "\trun\t\truns an algorithm. The algorithm id or a path to a model file and parameter values must be specified. Parameter values are specified after -- with PARAMETER=VALUE syntax. Ordered list values for a parameter can be created by specifying the parameter multiple times, e.g. --LAYERS=layer1.shp --LAYERS=layer2.shp\n"
      << "\t\t\tAlternatively, a '-' character in place of the parameters argument indicates that the parameters should be read from STDIN as a JSON object. The JSON should be structured as a map containing at least the \"inputs\" key specifying a map of input parameter values. This implies the --json option for output as a JSON object.\n"
      << "\t\t\tIf required, the ellipsoid to use for distance and area calculations can be specified via the \"--ELLIPSOID=name\" argument.\n"
      << "\t\t\tIf required, an existing QGIS project to use during the algorithm execution can be specified via the \"--PROJECT_PATH=path\" argument.\n";

  std::cout << msg.join( QString() ).toLocal8Bit().constData();
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
    if ( plugin == QLatin1String( "processing" ) || ( mPythonUtils->isPluginEnabled( plugin ) && mPythonUtils->pluginHasProcessingProvider( plugin ) ) )
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
#endif
}

void QgsProcessingExec::listAlgorithms( bool useJson )
{
  QVariantMap json;
  if ( !useJson )
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

    if ( !useJson )
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
      if ( algorithm->flags() & QgsProcessingAlgorithm::FlagNotAvailableInStandaloneTool )
        continue;

      if ( !useJson )
      {
        if ( algorithm->flags() & QgsProcessingAlgorithm::FlagDeprecated )
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

    if ( !useJson )
    {
      std::cout << "\n";
    }
    else
    {
      providerJson.insert( QStringLiteral( "algorithms" ), algorithmsJson );
      jsonProviders.insert( provider->id(), providerJson );
    }
  }

  if ( useJson )
  {
    json.insert( QStringLiteral( "providers" ), jsonProviders );
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
        jsonPlugin.insert( QStringLiteral( "loaded" ), showLoaded ? mPythonUtils->isPluginLoaded( plugin ) : mPythonUtils->isPluginEnabled( plugin ) );
        jsonPlugins.insert( plugin, jsonPlugin );
      }
    }
  }

  if ( useJson )
  {
    json.insert( QStringLiteral( "plugins" ), jsonPlugins );
    std::cout << QgsJsonUtils::jsonFromVariant( json ).dump( 2 );
  }
#endif
}

int QgsProcessingExec::enablePlugin( const QString &name, bool enabled )
{
  if ( enabled )
    std::cout << QStringLiteral( "Enabling plugin: \"%1\"\n" ).arg( name ).toLocal8Bit().constData();
  else
    std::cout << QStringLiteral( "Disabling plugin: \"%1\"\n" ).arg( name ).toLocal8Bit().constData();

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
    mPythonUtils->loadPlugin( name );
    mPythonUtils->startProcessingPlugin( name );
    const QString pluginName = mPythonUtils->getPluginMetadata( name, QStringLiteral( "name" ) );
    settings.setValue( "/PythonPlugins/" + name, true );

    std::cout << QStringLiteral( "Enabled %1 (%2)\n\n" ).arg( name, pluginName ).toLocal8Bit().constData();

    settings.remove( "/PythonPlugins/watchDog/" + name );
  }
  else
  {
    if ( mPythonUtils->isPluginLoaded( name ) )
      mPythonUtils->unloadPlugin( name );

    settings.setValue( "/PythonPlugins/" + name, false );

    std::cout << QStringLiteral( "Disabled %1\n\n" ).arg( name ).toLocal8Bit().constData();
  }
  listPlugins( false, false );

  return 0;
#else
  std::cerr << "No Python support\n";
  return 1;
#endif
}

int QgsProcessingExec::showAlgorithmHelp( const QString &inputId, bool useJson )
{
  QString id = inputId;

  std::unique_ptr< QgsProcessingModelAlgorithm > model;
  const QgsProcessingAlgorithm *alg = nullptr;
  if ( QFile::exists( id ) && QFileInfo( id ).suffix() == QLatin1String( "model3" ) )
  {
    model = std::make_unique< QgsProcessingModelAlgorithm >();
    if ( !model->fromFile( id ) )
    {
      std::cerr << QStringLiteral( "File %1 is not a valid Processing model!\n" ).arg( id ).toLocal8Bit().constData();
      return 1;
    }

    alg = model.get();
  }
#ifdef WITH_BINDINGS
  else if ( mPythonUtils && QFile::exists( id ) && QFileInfo( id ).suffix() == QLatin1String( "py" ) )
  {
    QString res;
    if ( !mPythonUtils->evalString( QStringLiteral( "qgis.utils.import_script_algorithm(\"%1\")" ).arg( id ), res ) || res.isEmpty() )
    {
      std::cerr << QStringLiteral( "File %1 is not a valid Processing script!\n" ).arg( id ).toLocal8Bit().constData();
      return 1;
    }

    id = res;
  }
#endif

  if ( !alg )
  {
    alg = QgsApplication::processingRegistry()->algorithmById( id );
    if ( ! alg )
    {
      std::cerr << QStringLiteral( "Algorithm %1 not found!\n" ).arg( id ).toLocal8Bit().constData();
      return 1;
    }
  }

  QVariantMap json;
  if ( !useJson )
  {
    std::cout << QStringLiteral( "%1 (%2)\n" ).arg( alg->displayName(), alg->id() ).toLocal8Bit().constData();

    std::cout << "\n----------------\n";
    std::cout << "Description\n";
    std::cout << "----------------\n";

    if ( const QgsProcessingModelAlgorithm *model = dynamic_cast< const QgsProcessingModelAlgorithm * >( alg ) )
    {
      // show finer help content for models
      const QVariantMap help = model->helpContent();
      std::cout << help.value( QStringLiteral( "ALG_DESC" ) ).toString().toLocal8Bit().constData() << '\n';

      if ( !help.value( QStringLiteral( "ALG_CREATOR" ) ).toString().isEmpty() ||
           !help.value( QStringLiteral( "ALG_VERSION" ) ).toString().isEmpty() )
        std::cout << '\n';

      if ( !help.value( QStringLiteral( "ALG_CREATOR" ) ).toString().isEmpty() )
        std::cout << "Algorithm author:\t" << help.value( QStringLiteral( "ALG_CREATOR" ) ).toString().toLocal8Bit().constData() << '\n';
      if ( !help.value( QStringLiteral( "ALG_VERSION" ) ).toString().isEmpty() )
        std::cout << "Algorithm version:\t" << help.value( QStringLiteral( "ALG_VERSION" ) ).toString().toLocal8Bit().constData() << '\n';

      if ( !help.value( QStringLiteral( "EXAMPLES" ) ).toString().isEmpty() )
      {
        std::cout << "\n----------------\n";
        std::cout << "Examples\n";
        std::cout << "----------------\n";
        std::cout << help.value( QStringLiteral( "EXAMPLES" ) ).toString().toLocal8Bit().constData() << '\n';
      }
    }
    else
    {
      if ( !alg->shortDescription().isEmpty() )
        std::cout << alg->shortDescription().toLocal8Bit().constData() << '\n';
      if ( !alg->shortHelpString().isEmpty() && alg->shortHelpString() != alg->shortDescription() )
        std::cout << alg->shortHelpString().toLocal8Bit().constData() << '\n';
    }

    std::cout << "\n----------------\n";
    std::cout << "Arguments\n";
    std::cout << "----------------\n\n";
  }
  else
  {
    addVersionInformation( json );

    QVariantMap algorithmDetails;
    algorithmDetails.insert( QStringLiteral( "id" ), alg->id() );
    addAlgorithmInformation( algorithmDetails, alg );
    json.insert( QStringLiteral( "algorithm_details" ), algorithmDetails );
    QVariantMap providerJson;
    addProviderInformation( providerJson, alg->provider() );
    json.insert( QStringLiteral( "provider_details" ), providerJson );
  }


  QVariantMap parametersJson;
  const QgsProcessingParameterDefinitions defs = alg->parameterDefinitions();
  for ( const QgsProcessingParameterDefinition *p : defs )
  {
    if ( p->flags() & QgsProcessingParameterDefinition::FlagHidden )
      continue;

    QVariantMap parameterJson;

    if ( !useJson )
      std::cout << QStringLiteral( "%1: %2\n" ).arg( p->name(), p->description() ).toLocal8Bit().constData();
    else
    {
      parameterJson.insert( QStringLiteral( "name" ), p->name() );
      parameterJson.insert( QStringLiteral( "description" ), p->description() );


      if ( const QgsProcessingParameterType *type = QgsApplication::processingRegistry()->parameterType( p->type() ) )
      {
        QVariantMap typeDetails;
        typeDetails.insert( QStringLiteral( "id" ), type->id() );
        typeDetails.insert( QStringLiteral( "name" ), type->name() );
        typeDetails.insert( QStringLiteral( "description" ), type->description() );
        typeDetails.insert( QStringLiteral( "metadata" ), type->metadata() );
        typeDetails.insert( QStringLiteral( "acceptable_values" ), type->acceptedStringValues() );

        parameterJson.insert( QStringLiteral( "type" ), typeDetails );
      }
      else
      {
        parameterJson.insert( QStringLiteral( "type" ), p->type() );
      }

      parameterJson.insert( QStringLiteral( "is_destination" ), p->isDestination() );
      parameterJson.insert( QStringLiteral( "default_value" ), p->defaultValue() );
      parameterJson.insert( QStringLiteral( "optional" ), bool( p->flags() & QgsProcessingParameterDefinition::FlagOptional ) );
      parameterJson.insert( QStringLiteral( "is_advanced" ), bool( p->flags() & QgsProcessingParameterDefinition::FlagAdvanced ) );

      parameterJson.insert( QStringLiteral( "raw_definition" ), p->toVariantMap() );
    }

    if ( ! p->help().isEmpty() )
    {
      if ( !useJson )
        std::cout << QStringLiteral( "\t%1\n" ).arg( p->help() ).toLocal8Bit().constData();
      else
        parameterJson.insert( QStringLiteral( "help" ), p->help() );
    }
    if ( !useJson )
      std::cout << QStringLiteral( "\tArgument type:\t%1\n" ).arg( p->type() ).toLocal8Bit().constData();

    if ( p->type() == QgsProcessingParameterEnum::typeName() )
    {
      const QgsProcessingParameterEnum *enumParam = static_cast< const QgsProcessingParameterEnum * >( p );
      QStringList options;
      QVariantMap jsonOptions;
      for ( int i = 0; i < enumParam->options().count(); ++i )
      {
        options << QStringLiteral( "\t\t- %1: %2" ).arg( i ).arg( enumParam->options().at( i ) );
        jsonOptions.insert( QString::number( i ), enumParam->options().at( i ) );
      }

      if ( !useJson )
        std::cout << QStringLiteral( "\tAvailable values:\n%1\n" ).arg( options.join( '\n' ) ).toLocal8Bit().constData();
      else
        parameterJson.insert( QStringLiteral( "available_options" ), jsonOptions );
    }

    // acceptable command line values
    if ( !useJson )
    {
      if ( const QgsProcessingParameterType *type = QgsApplication::processingRegistry()->parameterType( p->type() ) )
      {
        const QStringList values = type->acceptedStringValues();
        if ( !values.isEmpty() )
        {
          std::cout << "\tAcceptable values:\n";
          for ( const QString &val : values )
          {
            std::cout << QStringLiteral( "\t\t- %1" ).arg( val ).toLocal8Bit().constData() << "\n";
          }
        }
      }
    }

    parametersJson.insert( p->name(), parameterJson );
  }

  QVariantMap outputsJson;
  if ( !useJson )
  {
    std::cout << "\n----------------\n";
    std::cout << "Outputs\n";
    std::cout << "----------------\n\n";
  }
  const QgsProcessingOutputDefinitions outputs = alg->outputDefinitions();
  for ( const QgsProcessingOutputDefinition *o : outputs )
  {
    QVariantMap outputJson;
    if ( !useJson )
    {
      std::cout << QStringLiteral( "%1: <%2>\n" ).arg( o->name(), o->type() ).toLocal8Bit().constData();
      if ( !o->description().isEmpty() )
        std::cout << "\t" << o->description().toLocal8Bit().constData() << '\n';
    }
    else
    {
      outputJson.insert( QStringLiteral( "description" ), o->description() );
      outputJson.insert( QStringLiteral( "type" ), o->type() );
      outputsJson.insert( o->name(), outputJson );
    }
  }

  if ( !useJson )
  {
    std::cout << "\n\n";
  }
  else
  {
    json.insert( QStringLiteral( "parameters" ), parametersJson );
    json.insert( QStringLiteral( "outputs" ), outputsJson );
    std::cout << QgsJsonUtils::jsonFromVariant( json ).dump( 2 );
  }

  return 0;
}

int QgsProcessingExec::execute( const QString &inputId, const QVariantMap &params, const QString &ellipsoid, QgsUnitTypes::DistanceUnit distanceUnit, QgsUnitTypes::AreaUnit areaUnit, QgsProcessingContext::LogLevel logLevel, bool useJson, const QString &projectPath )
{
  QVariantMap json;
  if ( useJson )
  {
    addVersionInformation( json );
  }

  QString id = inputId;

  std::unique_ptr< QgsProcessingModelAlgorithm > model;
  const QgsProcessingAlgorithm *alg = nullptr;
  if ( QFile::exists( id ) && QFileInfo( id ).suffix() == QLatin1String( "model3" ) )
  {
    model = std::make_unique< QgsProcessingModelAlgorithm >();
    if ( !model->fromFile( id ) )
    {
      std::cerr << QStringLiteral( "File %1 is not a valid Processing model!\n" ).arg( id ).toLocal8Bit().constData();
      return 1;
    }

    alg = model.get();
  }
#ifdef WITH_BINDINGS
  else if ( mPythonUtils && QFile::exists( id ) && QFileInfo( id ).suffix() == QLatin1String( "py" ) )
  {
    QString res;
    if ( !mPythonUtils->evalString( QStringLiteral( "qgis.utils.import_script_algorithm(\"%1\")" ).arg( id ), res ) || res.isEmpty() )
    {
      std::cerr << QStringLiteral( "File %1 is not a valid Processing script!\n" ).arg( id ).toLocal8Bit().constData();
      return 1;
    }

    id = res;
  }
#endif

  if ( !alg )
  {
    alg = QgsApplication::processingRegistry()->algorithmById( id );
    if ( ! alg )
    {
      std::cerr << QStringLiteral( "Algorithm %1 not found!\n" ).arg( id ).toLocal8Bit().constData();
      return 1;
    }

    if ( alg->flags() & QgsProcessingAlgorithm::FlagNotAvailableInStandaloneTool )
    {
      std::cerr << QStringLiteral( "The \"%1\" algorithm is not available for use outside of the QGIS desktop application\n" ).arg( id ).toLocal8Bit().constData();
      return 1;
    }

    if ( !useJson && alg->flags() & QgsProcessingAlgorithm::FlagKnownIssues )
    {
      std::cout << "\n****************\n";
      std::cout << "Warning: this algorithm contains known issues and the results may be unreliable!\n";
      std::cout << "****************\n\n";
    }

    if ( !useJson && alg->flags() & QgsProcessingAlgorithm::FlagDeprecated )
    {
      std::cout << "\n****************\n";
      std::cout << "Warning: this algorithm is deprecated and may be removed in a future QGIS version!\n";
      std::cout << "****************\n\n";
    }

    if ( alg->flags() & QgsProcessingAlgorithm::FlagRequiresProject && projectPath.isEmpty() )
    {
      std::cerr << QStringLiteral( "The \"%1\" algorithm requires a QGIS project to execute. Specify a path to an existing project with the \"--PROJECT_PATH=xxx\" argument.\n" ).arg( id ).toLocal8Bit().constData();
      return 1;
    }
  }

  if ( useJson )
  {
    QVariantMap algorithmDetails;
    algorithmDetails.insert( QStringLiteral( "id" ), alg->id() );
    addAlgorithmInformation( algorithmDetails, alg );
    json.insert( QStringLiteral( "algorithm_details" ), algorithmDetails );

    if ( alg->provider() )
    {
      QVariantMap providerJson;
      addProviderInformation( providerJson, alg->provider() );
      json.insert( QStringLiteral( "provider_details" ), providerJson );
    }
  }

  std::unique_ptr< QgsProject > project;
  if ( !projectPath.isEmpty() )
  {
    project = std::make_unique< QgsProject >();
    if ( !project->read( projectPath ) )
    {
      std::cerr << QStringLiteral( "Could not load the QGIS project \"%1\"\n" ).arg( projectPath ).toLocal8Bit().constData();
      return 1;
    }
    QgsProject::setInstance( project.get() );
    json.insert( QStringLiteral( "project_path" ), projectPath );
  }

  if ( !useJson )
  {
    std::cout << "\n----------------\n";
    std::cout << "Inputs\n";
    std::cout << "----------------\n\n";
  }
  QVariantMap inputsJson;
  for ( auto it = params.constBegin(); it != params.constEnd(); ++it )
  {
    if ( !useJson )
      std::cout << it.key().toLocal8Bit().constData() << ":\t" << it.value().toString().toLocal8Bit().constData() << '\n';
    else
      inputsJson.insert( it.key(), it.value() );
  }
  if ( !useJson )
    std::cout << "\n";
  else
    json.insert( QStringLiteral( "inputs" ), inputsJson );

  if ( !ellipsoid.isEmpty() )
  {
    if ( !useJson )
      std::cout << "Using ellipsoid:\t" << ellipsoid.toLocal8Bit().constData() << '\n';
    else
      json.insert( QStringLiteral( "ellipsoid" ), ellipsoid );
  }
  if ( distanceUnit != QgsUnitTypes::DistanceUnknownUnit )
  {
    if ( !useJson )
      std::cout << "Using distance unit:\t" << QgsUnitTypes::toString( distanceUnit ).toLocal8Bit().constData() << '\n';
    else
      json.insert( QStringLiteral( "distance_unit" ), QgsUnitTypes::toString( distanceUnit ) );
  }
  if ( areaUnit != QgsUnitTypes::AreaUnknownUnit )
  {
    if ( !useJson )
      std::cout << "Using area unit:\t" << QgsUnitTypes::toString( areaUnit ).toLocal8Bit().constData() << '\n';
    else
      json.insert( QStringLiteral( "area_unit" ), QgsUnitTypes::toString( areaUnit ) );
  }


  QgsProcessingContext context;
  context.setEllipsoid( ellipsoid );
  context.setDistanceUnit( distanceUnit );
  context.setAreaUnit( areaUnit );
  context.setProject( project.get() );
  context.setLogLevel( logLevel );

  const QgsProcessingParameterDefinitions defs = alg->parameterDefinitions();
  QList< const QgsProcessingParameterDefinition * > missingParams;
  for ( const QgsProcessingParameterDefinition *p : defs )
  {
    if ( !p->checkValueIsAcceptable( params.value( p->name() ), &context ) )
    {
      if ( !( p->flags() & QgsProcessingParameterDefinition::FlagOptional ) && !params.contains( p->name() ) )
      {
        missingParams << p;
      }
    }
  }

  if ( !missingParams.isEmpty() )
  {
    std::cerr << QStringLiteral( "ERROR: The following mandatory parameters were not specified\n\n" ).toLocal8Bit().constData();
    for ( const QgsProcessingParameterDefinition *p : std::as_const( missingParams ) )
    {
      std::cerr << QStringLiteral( "\t%1:\t%2\n" ).arg( p->name(), p->description() ).toLocal8Bit().constData();
    }

    return 1;
  }

  QString message;
  if ( !alg->checkParameterValues( params, context, &message ) )
  {
    std::cerr << QStringLiteral( "ERROR:\tAn error was encountered while checking parameter values\n" ).toLocal8Bit().constData();
    std::cerr << QStringLiteral( "\t%1\n" ).arg( message ).toLocal8Bit().constData();
    return 1;
  }

  ConsoleFeedback feedback( useJson );

#if defined(Q_OS_UNIX) && !defined(Q_OS_ANDROID)
  UnixSignalWatcher sigwatch;
  sigwatch.watchForSignal( SIGINT );

  QObject::connect( &sigwatch, &UnixSignalWatcher::unixSignal, &feedback, [&feedback ]( int signal )
  {
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

  bool ok = false;
  if ( !useJson )
    std::cout << "\n";

  QVariantMap res = alg->run( params, context, &feedback, &ok );

  if ( ok )
  {
    QVariantMap resultsJson;
    if ( !useJson )
    {
      std::cout << "\n----------------\n";
      std::cout << "Results\n";
      std::cout << "----------------\n\n";
    }
    else
    {
      json.insert( QStringLiteral( "log" ), feedback.jsonLog() );
    }

    for ( auto it = res.constBegin(); it != res.constEnd(); ++it )
    {
      if ( it.key() == QLatin1String( "CHILD_INPUTS" ) || it.key() == QLatin1String( "CHILD_RESULTS" ) )
        continue;

      QVariant result = it.value();
      if ( !useJson )
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

    if ( useJson )
    {
      json.insert( QStringLiteral( "results" ), resultsJson );
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
  json.insert( QStringLiteral( "qgis_version" ), Qgis::version() );
  if ( QString( Qgis::devVersion() ) != QLatin1String( "exported" ) )
  {
    json.insert( QStringLiteral( "qgis_code_revision" ), Qgis::devVersion() );
  }
  json.insert( QStringLiteral( "qt_version" ), qVersion() );
  json.insert( QStringLiteral( "python_version" ), PYTHON_VERSION );
  json.insert( QStringLiteral( "gdal_version" ), GDALVersionInfo( "RELEASE_NAME" ) );
  json.insert( QStringLiteral( "geos_version" ), GEOSversion() );

  PJ_INFO info = proj_info();
  json.insert( QStringLiteral( "proj_version" ), info.release );
}

void QgsProcessingExec::addAlgorithmInformation( QVariantMap &algorithmJson, const QgsProcessingAlgorithm *algorithm )
{
  algorithmJson.insert( QStringLiteral( "name" ), algorithm->displayName() );
  algorithmJson.insert( QStringLiteral( "short_description" ), algorithm->shortDescription() );
  algorithmJson.insert( QStringLiteral( "tags" ), algorithm->tags() );
  algorithmJson.insert( QStringLiteral( "help_url" ), algorithm->helpUrl() );
  algorithmJson.insert( QStringLiteral( "group" ), algorithm->group() );
  algorithmJson.insert( QStringLiteral( "can_cancel" ), bool( algorithm->flags() & QgsProcessingAlgorithm::FlagCanCancel ) );
  algorithmJson.insert( QStringLiteral( "requires_matching_crs" ), bool( algorithm->flags() & QgsProcessingAlgorithm::FlagRequiresMatchingCrs ) );
  algorithmJson.insert( QStringLiteral( "has_known_issues" ), bool( algorithm->flags() & QgsProcessingAlgorithm::FlagKnownIssues ) );
  algorithmJson.insert( QStringLiteral( "deprecated" ), bool( algorithm->flags() & QgsProcessingAlgorithm::FlagDeprecated ) );
}

void QgsProcessingExec::addProviderInformation( QVariantMap &providerJson, QgsProcessingProvider *provider )
{
  providerJson.insert( QStringLiteral( "name" ), provider->name() );
  providerJson.insert( QStringLiteral( "long_name" ), provider->longName() );
  providerJson.insert( QStringLiteral( "version" ), provider->versionInfo() );
  providerJson.insert( QStringLiteral( "can_be_activated" ), provider->canBeActivated() );
  if ( !provider->warningMessage().isEmpty() )
  {
    providerJson.insert( QStringLiteral( "warning" ), provider->warningMessage() );
  }
  providerJson.insert( QStringLiteral( "is_active" ), provider->isActive() );
  providerJson.insert( QStringLiteral( "supported_output_raster_extensions" ), provider->supportedOutputRasterLayerExtensions() );
  providerJson.insert( QStringLiteral( "supported_output_vector_extensions" ), provider->supportedOutputVectorLayerExtensions() );
  providerJson.insert( QStringLiteral( "supported_output_table_extensions" ), provider->supportedOutputTableExtensions() );
  providerJson.insert( QStringLiteral( "default_vector_file_extension" ), provider->defaultVectorFileExtension() );
  providerJson.insert( QStringLiteral( "default_raster_file_extension" ), provider->defaultRasterFileExtension() );
  providerJson.insert( QStringLiteral( "supports_non_file_based_output" ), provider->supportsNonFileBasedOutput() );
}
