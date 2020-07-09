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

#if defined(Q_OS_UNIX) && !defined(Q_OS_ANDROID)
#include "sigwatch.h"
#endif

#include <iostream>
#include <QObject>

ConsoleFeedback::ConsoleFeedback()
{
  connect( this, &QgsFeedback::progressChanged, this, &ConsoleFeedback::showTerminalProgress );
  mTimer.start();
}

void ConsoleFeedback::setProgressText( const QString &text )
{
  std::cout << text.toLocal8Bit().constData() << '\n';
}

void ConsoleFeedback::reportError( const QString &error, bool )
{
  std::cerr << "ERROR:\t" << error.toLocal8Bit().constData() << '\n';
}

void ConsoleFeedback::pushInfo( const QString &info )
{
  std::cout << info.toLocal8Bit().constData() << '\n';
}

void ConsoleFeedback::pushCommandInfo( const QString &info )
{
  std::cout << info.toLocal8Bit().constData() << '\n';
}

void ConsoleFeedback::pushDebugInfo( const QString &info )
{
  std::cout << info.toLocal8Bit().constData() << '\n';
}

void ConsoleFeedback::pushConsoleInfo( const QString &info )
{
  std::cout << info.toLocal8Bit().constData() << '\n';
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
      std::cerr << QStringLiteral( "Couldn't load Python support library: %1" ).arg( pythonlib.errorString() ).toLocal8Bit().constData();
      return nullptr;
    }
  }

  typedef QgsPythonUtils*( *inst )();
  inst pythonlib_inst = reinterpret_cast< inst >( cast_to_fptr( pythonlib.resolve( "instance" ) ) );
  if ( !pythonlib_inst )
  {
    //using stderr on purpose because we want end users to see this [TS]
    std::cerr << "Couldn't resolve python support library's instance() symbol.";
    return nullptr;
  }

  std::unique_ptr< QgsPythonUtils > pythonUtils( pythonlib_inst() );
  if ( pythonUtils )
  {
    pythonUtils->initPython( nullptr, false );
  }

  return pythonUtils;
}

QgsProcessingExec::QgsProcessingExec()
{

}

int QgsProcessingExec::run( const QStringList &args )
{
  QObject::connect( QgsApplication::messageLog(), static_cast < void ( QgsMessageLog::* )( const QString &message, const QString &tag, Qgis::MessageLevel level ) >( &QgsMessageLog::messageReceived ), QgsApplication::instance(),
                    [ = ]( const QString & message, const QString &, Qgis::MessageLevel level )
  {
    if ( level == Qgis::Critical )
    {
      if ( !message.contains( QLatin1String( "DeprecationWarning:" ) ) )
        std::cerr << message.toLocal8Bit().constData();
    }
  } );

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

  // give Python plugins a chance to load providers
  mPythonUtils = loadPythonSupport();
  if ( !mPythonUtils )
  {
    QCoreApplication::exit( 1 );
    return 1;
  }
  loadPlugins();

  const QString command = args.at( 1 );
  if ( command == QLatin1String( "plugins" ) )
  {
    listPlugins();
    return 0;
  }
  else if ( command == QLatin1String( "list" ) )
  {
    listAlgorithms();
    return 0;
  }
  else if ( command == QLatin1String( "help" ) )
  {
    if ( args.size() < 3 )
    {
      std::cerr << QStringLiteral( "Algorithm ID or model file not specified\n" ).toLocal8Bit().constData();
      return 1;
    }

    const QString algId = args.at( 2 );
    return showAlgorithmHelp( algId );
  }
  else if ( command == QLatin1String( "run" ) )
  {
    if ( args.size() < 3 )
    {
      std::cerr << QStringLiteral( "Algorithm ID or model file not specified\n" ).toLocal8Bit().constData();
      return 1;
    }

    const QString algId = args.at( 2 );

    // build parameter map
    QVariantMap params;
    for ( int i = 3; i < args.count(); i++ )
    {
      QString arg = args.at( i );
      if ( arg.startsWith( QLatin1String( "--" ) ) )
        arg = arg.mid( 2 );

      const QStringList parts = arg.split( '=' );
      if ( parts.count() >= 2 )
      {
        const QString name = parts.at( 0 );
        const QString value = parts.mid( 1 ).join( '=' );
        params.insert( name, value );
      }
      else
      {
        std::cerr << QStringLiteral( "Invalid parameter value %1. Parameter values must be of the form \"--NAME=VALUE\"\n" ).arg( arg ).toLocal8Bit().constData();
        return 1;
      }
    }

    return execute( algId, params );
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
      << "Usage: " << appName <<  " [command] [algorithm id or path to model file] [parameters]\n"
      << "\nAvailable commands:\n"
      << "\tplugins\tlist available and active plugins\n"
      << "\tlist\tlist all available processing algorithms\n"
      << "\thelp\tshow help for an algorithm. The algorithm id or a path to a model file must be specified.\n"
      << "\trun\truns an algorithm. The algorithm id or a path to a model file and parameter values must be specified. Parameter values are specified via the --PARAMETER=VALUE syntax\n";

  std::cout << msg.join( QString() ).toLocal8Bit().constData();
}

void QgsProcessingExec::loadPlugins()
{
  QgsSettings settings;
  // load plugins
  const QStringList plugins = mPythonUtils->pluginList();
  for ( const QString &plugin : plugins )
  {
    if ( mPythonUtils->isPluginEnabled( plugin ) && mPythonUtils->pluginHasProcessingProvider( plugin ) )
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
}

void QgsProcessingExec::listAlgorithms()
{
  std::cout << "Available algorithms\n\n";

  const QList<QgsProcessingProvider *> providers = QgsApplication::processingRegistry()->providers();
  for ( QgsProcessingProvider *provider : providers )
  {
    std::cout << provider->name().toLocal8Bit().constData() << "\n";
    const QList<const QgsProcessingAlgorithm *> algorithms = provider->algorithms();
    for ( const QgsProcessingAlgorithm *algorithm : algorithms )
    {
      std::cout << "\t" << algorithm->id().toLocal8Bit().constData() << "\t" << algorithm->displayName().toLocal8Bit().constData() << "\n";
    }

    std::cout << "\n";
  }
}

void QgsProcessingExec::listPlugins()
{
  std::cout << "Available plugins\n";
  std::cout << "(* indicates loaded plugins which implement Processing providers)\n\n";

  const QStringList plugins = mPythonUtils->pluginList();
  for ( const QString &plugin : plugins )
  {
    if ( !mPythonUtils->pluginHasProcessingProvider( plugin ) )
      continue;

    if ( mPythonUtils->isPluginLoaded( plugin ) )
      std::cout << "* ";
    else
      std::cout << "  ";
    std::cout << plugin.toLocal8Bit().constData() << "\n";
  }
}

int QgsProcessingExec::showAlgorithmHelp( const QString &id )
{
  std::unique_ptr< QgsProcessingModelAlgorithm > model;
  const QgsProcessingAlgorithm *alg = nullptr;
  if ( QFile::exists( id ) && QFileInfo( id ).suffix() == QLatin1String( "model3" ) )
  {
    model = qgis::make_unique< QgsProcessingModelAlgorithm >();
    if ( !model->fromFile( id ) )
    {
      std::cerr << QStringLiteral( "File %1 is not a valid Processing model!\n" ).arg( id ).toLocal8Bit().constData();
      return 1;
    }

    alg = model.get();
  }
  else
  {
    alg = QgsApplication::processingRegistry()->algorithmById( id );
    if ( ! alg )
    {
      std::cerr << QStringLiteral( "Algorithm %1 not found!\n" ).arg( id ).toLocal8Bit().constData();
      return 1;
    }
  }

  std::cout << QStringLiteral( "%1 (%2)\n" ).arg( alg->displayName(), alg->id() ).toLocal8Bit().constData();
  std::cout << "\n----------------\n";
  std::cout << "Description\n";
  std::cout << "----------------\n";
  if ( !alg->shortDescription().isEmpty() )
    std::cout << alg->shortDescription().toLocal8Bit().constData() << '\n';
  if ( !alg->shortHelpString().isEmpty() && alg->shortHelpString() != alg->shortDescription() )
    std::cout << alg->shortHelpString().toLocal8Bit().constData() << '\n';

  std::cout << "\n----------------\n";
  std::cout << "Arguments\n";
  std::cout << "----------------\n\n";

  const QgsProcessingParameterDefinitions defs = alg->parameterDefinitions();
  for ( const QgsProcessingParameterDefinition *p : defs )
  {
    if ( p->flags() & QgsProcessingParameterDefinition::FlagHidden )
      continue;

    std::cout << QStringLiteral( "%1: %2\n" ).arg( p->name(), p->description() ).toLocal8Bit().constData();
    std::cout << QStringLiteral( "\tArgument type:\t%1\n" ).arg( p->type() ).toLocal8Bit().constData();

    if ( p->type() == QgsProcessingParameterEnum::typeName() )
    {
      const QgsProcessingParameterEnum *enumParam = static_cast< const QgsProcessingParameterEnum * >( p );
      QStringList options;
      for ( int i = 0; i < enumParam->options().count(); ++i )
        options << QStringLiteral( "\t\t- %1: %2" ).arg( i ).arg( enumParam->options().at( i ) );

      std::cout << QStringLiteral( "\tAvailable values:\n%1\n" ).arg( options.join( '\n' ) ).toLocal8Bit().constData();
    }

    // acceptable command line values
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

  std::cout << "\n----------------\n";
  std::cout << "Outputs\n";
  std::cout << "----------------\n\n";
  const QgsProcessingOutputDefinitions outputs = alg->outputDefinitions();
  for ( const QgsProcessingOutputDefinition *o : outputs )
  {
    std::cout << QStringLiteral( "%1: <%2>\n" ).arg( o->name(), o->type() ).toLocal8Bit().constData();
    if ( !o->description().isEmpty() )
      std::cout << "\t" << o->description().toLocal8Bit().constData() << '\n';
  }
  std::cout << "\n\n";

  return 0;
}

int QgsProcessingExec::execute( const QString &id, const QVariantMap &params )
{
  std::unique_ptr< QgsProcessingModelAlgorithm > model;
  const QgsProcessingAlgorithm *alg = nullptr;
  if ( QFile::exists( id ) && QFileInfo( id ).suffix() == QLatin1String( "model3" ) )
  {
    model = qgis::make_unique< QgsProcessingModelAlgorithm >();
    if ( !model->fromFile( id ) )
    {
      std::cerr << QStringLiteral( "File %1 is not a valid Processing model!\n" ).arg( id ).toLocal8Bit().constData();
      return 1;
    }

    alg = model.get();
  }
  else
  {
    alg = QgsApplication::processingRegistry()->algorithmById( id );
    if ( ! alg )
    {
      std::cerr << QStringLiteral( "Algorithm %1 not found!\n" ).arg( id ).toLocal8Bit().constData();
      return 1;
    }
  }

  std::cout << "\n----------------\n";
  std::cout << "Inputs\n";
  std::cout << "----------------\n\n";
  for ( auto it = params.constBegin(); it != params.constEnd(); ++it )
  {
    std::cout << it.key().toLocal8Bit().constData() << ":\t" << it.value().toString().toLocal8Bit().constData() << '\n';
  }

  QgsProcessingContext context;
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
    for ( const QgsProcessingParameterDefinition *p : qgis::as_const( missingParams ) )
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

  ConsoleFeedback feedback;

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
  std::cout << "\n";
  QVariantMap res = alg->run( params, context, &feedback, &ok );

  if ( ok )
  {
    std::cout << "\n----------------\n";
    std::cout << "Results\n";
    std::cout << "----------------\n\n";

    for ( auto it = res.constBegin(); it != res.constEnd(); ++it )
    {
      if ( it.key() == QLatin1String( "CHILD_INPUTS" ) || it.key() == QLatin1String( "CHILD_RESULTS" ) )
        continue;

      QVariant result = it.value();
      if ( result.type() == QVariant::List || result.type() == QVariant::StringList )
      {
        QStringList list;
        for ( const QVariant &v : result.toList() )
          list << v.toString();
        result = list.join( ", " );
      }
      std::cout << it.key().toLocal8Bit().constData() << ":\t" << result.toString().toLocal8Bit().constData() << '\n';
    }
    return 0;
  }
  else
  {
    return 1;
  }
}
