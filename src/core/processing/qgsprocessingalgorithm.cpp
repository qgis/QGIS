/***************************************************************************
                         qgsprocessingalgorithm.cpp
                         --------------------------
    begin                : December 2016
    copyright            : (C) 2016 by Nyall Dawson
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

#include "qgsprocessingalgorithm.h"
#include "qgsapplication.h"
#include "qgsprocessingprovider.h"
#include "qgsprocessingparameters.h"
#include "qgsprocessingoutputs.h"
#include "qgsrectangle.h"
#include "qgsprocessingcontext.h"
#include "qgsprocessingutils.h"
#include "qgsexception.h"
#include "qgsmessagelog.h"
#include "qgsvectorlayer.h"
#include "qgsprocessingfeedback.h"
#include "qgsmeshlayer.h"
#include "qgspointcloudlayer.h"
#include "qgsexpressioncontextutils.h"
#include <QRegularExpression>
#include <QRegularExpressionMatch>

QgsProcessingAlgorithm::~QgsProcessingAlgorithm()
{
  qDeleteAll( mParameters );
  qDeleteAll( mOutputs );
}

QgsProcessingAlgorithm *QgsProcessingAlgorithm::create( const QVariantMap &configuration ) const
{
  std::unique_ptr< QgsProcessingAlgorithm > creation( createInstance() );
  if ( ! creation )
    throw QgsProcessingException( QObject::tr( "Error creating algorithm from createInstance()" ) );
  creation->setProvider( provider() );
  creation->initAlgorithm( configuration );
  return creation.release();
}

QString QgsProcessingAlgorithm::id() const
{
  if ( mProvider )
    return QStringLiteral( "%1:%2" ).arg( mProvider->id(), name() );
  else
    return name();
}

QString QgsProcessingAlgorithm::shortDescription() const
{
  return QString();
}

QString QgsProcessingAlgorithm::shortHelpString() const
{
  return QString();
}

QString QgsProcessingAlgorithm::helpString() const
{
  return QString();
}

QString QgsProcessingAlgorithm::helpUrl() const
{
  return QString();
}

QIcon QgsProcessingAlgorithm::icon() const
{
  return QgsApplication::getThemeIcon( "/processingAlgorithm.svg" );
}

QString QgsProcessingAlgorithm::svgIconPath() const
{
  return QgsApplication::iconPath( QStringLiteral( "processingAlgorithm.svg" ) );
}

QgsProcessingAlgorithm::Flags QgsProcessingAlgorithm::flags() const
{
  return FlagSupportsBatch | FlagCanCancel;
}

bool QgsProcessingAlgorithm::canExecute( QString * ) const
{
  return true;
}

bool QgsProcessingAlgorithm::checkParameterValues( const QVariantMap &parameters, QgsProcessingContext &context, QString *message ) const
{
  for ( const QgsProcessingParameterDefinition *def : mParameters )
  {
    if ( !def->checkValueIsAcceptable( parameters.value( def->name() ), &context ) )
    {
      if ( message )
      {
        // TODO QGIS 4 - move the message handling to the parameter subclasses (but this
        // requires a change in signature for the virtual checkValueIsAcceptable method)
        if ( def->type() == QgsProcessingParameterFeatureSource::typeName() )
          *message = invalidSourceError( parameters, def->name() );
        else if ( def->type() == QgsProcessingParameterFeatureSink::typeName() )
          *message = invalidSinkError( parameters, def->name() );
        else if ( def->type() == QgsProcessingParameterRasterLayer::typeName() )
          *message = invalidRasterError( parameters, def->name() );
        else
          *message = QObject::tr( "Incorrect parameter value for %1" ).arg( def->name() );
      }
      return false;
    }
  }
  return true;
}

QVariantMap QgsProcessingAlgorithm::preprocessParameters( const QVariantMap &parameters )
{
  return parameters;
}

QgsProcessingProvider *QgsProcessingAlgorithm::provider() const
{
  return mProvider;
}

void QgsProcessingAlgorithm::setProvider( QgsProcessingProvider *provider )
{
  mProvider = provider;

  if ( mProvider && !mProvider->supportsNonFileBasedOutput() )
  {
    // need to update all destination parameters to turn off non file based outputs
    for ( const QgsProcessingParameterDefinition *definition : std::as_const( mParameters ) )
    {
      if ( definition->isDestination() )
      {
        const QgsProcessingDestinationParameter *destParam = static_cast< const QgsProcessingDestinationParameter *>( definition );
        const_cast< QgsProcessingDestinationParameter *>( destParam )->setSupportsNonFileBasedOutput( false );
      }
    }
  }
}

QWidget *QgsProcessingAlgorithm::createCustomParametersWidget( QWidget * ) const
{
  return nullptr;
}

QgsExpressionContext QgsProcessingAlgorithm::createExpressionContext( const QVariantMap &parameters,
    QgsProcessingContext &context, QgsProcessingFeatureSource *source ) const
{
  // start with context's expression context
  QgsExpressionContext c = context.expressionContext();

  // If there's a source capable of generating a context scope, use it
  if ( source )
  {
    QgsExpressionContextScope *scope = source->createExpressionContextScope();
    if ( scope )
      c << scope;
  }
  else if ( c.scopeCount() == 0 )
  {
    //empty scope, populate with initial scopes
    c << QgsExpressionContextUtils::globalScope()
      << QgsExpressionContextUtils::projectScope( context.project() );
  }

  c << QgsExpressionContextUtils::processingAlgorithmScope( this, parameters, context );
  return c;
}

bool QgsProcessingAlgorithm::validateInputCrs( const QVariantMap &parameters, QgsProcessingContext &context ) const
{
  if ( !( flags() & FlagRequiresMatchingCrs ) )
  {
    // I'm a well behaved algorithm - I take work AWAY from users!
    return true;
  }

  bool foundCrs = false;
  QgsCoordinateReferenceSystem crs;
  for ( const QgsProcessingParameterDefinition *def : mParameters )
  {
    if ( def->type() == QgsProcessingParameterMapLayer::typeName() || def->type() == QgsProcessingParameterRasterLayer::typeName() )
    {
      QgsMapLayer *layer = QgsProcessingParameters::parameterAsLayer( def, parameters, context );
      if ( layer )
      {
        if ( foundCrs && layer->crs().isValid() && crs != layer->crs() )
        {
          return false;
        }
        else if ( !foundCrs && layer->crs().isValid() )
        {
          foundCrs = true;
          crs = layer->crs();
        }
      }
    }
    else if ( def->type() == QgsProcessingParameterFeatureSource::typeName() )
    {
      std::unique_ptr< QgsFeatureSource  > source( QgsProcessingParameters::parameterAsSource( def, parameters, context ) );
      if ( source )
      {
        if ( foundCrs && source->sourceCrs().isValid() && crs != source->sourceCrs() )
        {
          return false;
        }
        else if ( !foundCrs && source->sourceCrs().isValid() )
        {
          foundCrs = true;
          crs = source->sourceCrs();
        }
      }
    }
    else if ( def->type() == QgsProcessingParameterMultipleLayers::typeName() )
    {
      QList< QgsMapLayer *> layers = QgsProcessingParameters::parameterAsLayerList( def, parameters, context );
      const auto constLayers = layers;
      for ( QgsMapLayer *layer : constLayers )
      {
        if ( !layer )
          continue;

        if ( foundCrs && layer->crs().isValid() && crs != layer->crs() )
        {
          return false;
        }
        else if ( !foundCrs && layer->crs().isValid() )
        {
          foundCrs = true;
          crs = layer->crs();
        }
      }
    }
    else if ( def->type() == QgsProcessingParameterExtent::typeName() )
    {
      QgsCoordinateReferenceSystem extentCrs = QgsProcessingParameters::parameterAsExtentCrs( def, parameters, context );
      if ( foundCrs && extentCrs.isValid() && crs != extentCrs )
      {
        return false;
      }
      else if ( !foundCrs && extentCrs.isValid() )
      {
        foundCrs = true;
        crs = extentCrs;
      }
    }
    else if ( def->type() == QgsProcessingParameterPoint::typeName() )
    {
      QgsCoordinateReferenceSystem pointCrs = QgsProcessingParameters::parameterAsPointCrs( def, parameters, context );
      if ( foundCrs && pointCrs.isValid() && crs != pointCrs )
      {
        return false;
      }
      else if ( !foundCrs && pointCrs.isValid() )
      {
        foundCrs = true;
        crs = pointCrs;
      }
    }
    else if ( def->type() == QgsProcessingParameterGeometry::typeName() )
    {
      QgsCoordinateReferenceSystem geomCrs = QgsProcessingParameters::parameterAsGeometryCrs( def, parameters, context );
      if ( foundCrs && geomCrs.isValid() && crs != geomCrs )
      {
        return false;
      }
      else if ( !foundCrs && geomCrs.isValid() )
      {
        foundCrs = true;
        crs = geomCrs;
      }
    }

  }
  return true;
}

QString QgsProcessingAlgorithm::asPythonCommand( const QVariantMap &parameters, QgsProcessingContext &context ) const
{
  QString s = QStringLiteral( "processing.run(\"%1\"," ).arg( id() );

  QStringList parts;
  for ( const QgsProcessingParameterDefinition *def : mParameters )
  {
    if ( def->flags() & QgsProcessingParameterDefinition::FlagHidden )
      continue;

    if ( !parameters.contains( def->name() ) )
      continue;

    parts << QStringLiteral( "'%1':%2" ).arg( def->name(), def->valueAsPythonString( parameters.value( def->name() ), context ) );
  }

  s += QStringLiteral( " {%1})" ).arg( parts.join( ',' ) );
  return s;
}

QString QgsProcessingAlgorithm::asQgisProcessCommand( const QVariantMap &parameters, QgsProcessingContext &context, bool &ok ) const
{
  ok = true;
  QStringList parts;
  parts.append( QStringLiteral( "qgis_process" ) );
  parts.append( QStringLiteral( "run" ) );
  parts.append( id() );

  QgsProcessingContext::ProcessArgumentFlags argumentFlags;
  // we only include the project path argument if a project is actually required by the algorithm
  if ( flags() & FlagRequiresProject )
    argumentFlags |= QgsProcessingContext::ProcessArgumentFlag::IncludeProjectPath;

  parts.append( context.asQgisProcessArguments( argumentFlags ) );

  auto escapeIfNeeded = []( const QString & input ) -> QString
  {
    // play it safe and escape everything UNLESS it's purely alphanumeric characters (and a very select scattering of other common characters!)
    const thread_local QRegularExpression nonAlphaNumericRx( QStringLiteral( "[^a-zA-Z0-9.\\-/_]" ) );
    if ( nonAlphaNumericRx.match( input ).hasMatch() )
    {
      QString escaped = input;
      escaped.replace( '\'', QLatin1String( "'\\''" ) );
      return QStringLiteral( "'%1'" ).arg( escaped );
    }
    else
    {
      return input;
    }
  };

  for ( const QgsProcessingParameterDefinition *def : mParameters )
  {
    if ( def->flags() & QgsProcessingParameterDefinition::FlagHidden )
      continue;

    if ( !parameters.contains( def->name() ) )
      continue;

    const QStringList partValues = def->valueAsStringList( parameters.value( def->name() ), context, ok );
    if ( !ok )
      return QString();

    for ( const QString &partValue : partValues )
    {
      parts << QStringLiteral( "--%1=%2" ).arg( def->name(), escapeIfNeeded( partValue ) );
    }
  }

  return parts.join( ' ' );
}

QVariantMap QgsProcessingAlgorithm::asMap( const QVariantMap &parameters, QgsProcessingContext &context ) const
{
  QVariantMap properties = context.exportToMap();

  // we only include the project path argument if a project is actually required by the algorithm
  if ( !( flags() & FlagRequiresProject ) )
    properties.remove( QStringLiteral( "project_path" ) );

  QVariantMap paramValues;
  for ( const QgsProcessingParameterDefinition *def : mParameters )
  {
    if ( def->flags() & QgsProcessingParameterDefinition::FlagHidden )
      continue;

    if ( !parameters.contains( def->name() ) )
      continue;

    paramValues.insert( def->name(), def->valueAsJsonObject( parameters.value( def->name() ), context ) );
  }

  properties.insert( QStringLiteral( "inputs" ), paramValues );
  return properties;
}

bool QgsProcessingAlgorithm::addParameter( QgsProcessingParameterDefinition *definition, bool createOutput )
{
  if ( !definition )
    return false;

  // check for duplicate named parameters
  const QgsProcessingParameterDefinition *existingDef = QgsProcessingAlgorithm::parameterDefinition( definition->name() );
  if ( existingDef && existingDef->name() == definition->name() ) // parameterDefinition is case-insensitive, but we DO allow case-different duplicate names
  {
    QgsMessageLog::logMessage( QObject::tr( "Duplicate parameter %1 registered for alg %2" ).arg( definition->name(), id() ), QObject::tr( "Processing" ) );
    delete definition;
    return false;
  }

  if ( definition->isDestination() && mProvider )
  {
    QgsProcessingDestinationParameter *destParam = static_cast< QgsProcessingDestinationParameter *>( definition );
    if ( !mProvider->supportsNonFileBasedOutput() )
      destParam->setSupportsNonFileBasedOutput( false );
  }

  mParameters << definition;
  definition->mAlgorithm = this;

  if ( createOutput )
    return createAutoOutputForParameter( definition );
  else
    return true;
}

void QgsProcessingAlgorithm::removeParameter( const QString &name )
{
  const QgsProcessingParameterDefinition *def = parameterDefinition( name );
  if ( def )
  {
    delete def;
    mParameters.removeAll( def );

    // remove output automatically created when adding parameter
    const QgsProcessingOutputDefinition *outputDef = QgsProcessingAlgorithm::outputDefinition( name );
    if ( outputDef && outputDef->autoCreated() )
    {
      delete outputDef;
      mOutputs.removeAll( outputDef );
    }
  }
}

bool QgsProcessingAlgorithm::addOutput( QgsProcessingOutputDefinition *definition )
{
  if ( !definition )
    return false;

  // check for duplicate named outputs
  if ( QgsProcessingAlgorithm::outputDefinition( definition->name() ) )
  {
    QgsMessageLog::logMessage( QObject::tr( "Duplicate output %1 registered for alg %2" ).arg( definition->name(), id() ), QObject::tr( "Processing" ) );
    delete definition;
    return false;
  }

  mOutputs << definition;
  return true;
}

bool QgsProcessingAlgorithm::prepareAlgorithm( const QVariantMap &, QgsProcessingContext &, QgsProcessingFeedback * )
{
  return true;
}

QVariantMap QgsProcessingAlgorithm::postProcessAlgorithm( QgsProcessingContext &, QgsProcessingFeedback * )
{
  return QVariantMap();
}

const QgsProcessingParameterDefinition *QgsProcessingAlgorithm::parameterDefinition( const QString &name ) const
{
  // first pass - case sensitive match
  for ( const QgsProcessingParameterDefinition *def : mParameters )
  {
    if ( def->name() == name )
      return def;
  }

  // second pass - case insensitive
  for ( const QgsProcessingParameterDefinition *def : mParameters )
  {
    if ( def->name().compare( name, Qt::CaseInsensitive ) == 0 )
      return def;
  }
  return nullptr;
}

int QgsProcessingAlgorithm::countVisibleParameters() const
{
  int count = 0;
  for ( const QgsProcessingParameterDefinition *def : mParameters )
  {
    if ( !( def->flags() & QgsProcessingParameterDefinition::FlagHidden ) )
      count++;
  }
  return count;
}

QgsProcessingParameterDefinitions QgsProcessingAlgorithm::destinationParameterDefinitions() const
{
  QgsProcessingParameterDefinitions result;
  for ( const QgsProcessingParameterDefinition *def : mParameters )
  {
    if ( def->isDestination() )
      result << def;
  }
  return result;
}

const QgsProcessingOutputDefinition *QgsProcessingAlgorithm::outputDefinition( const QString &name ) const
{
  for ( const QgsProcessingOutputDefinition *def : mOutputs )
  {
    if ( def->name().compare( name, Qt::CaseInsensitive ) == 0 )
      return def;
  }
  return nullptr;
}

bool QgsProcessingAlgorithm::hasHtmlOutputs() const
{
  for ( const QgsProcessingOutputDefinition *def : mOutputs )
  {
    if ( def->type() == QLatin1String( "outputHtml" ) )
      return true;
  }
  return false;
}

QgsProcessingAlgorithm::VectorProperties QgsProcessingAlgorithm::sinkProperties( const QString &, const QVariantMap &, QgsProcessingContext &, const QMap<QString, QgsProcessingAlgorithm::VectorProperties> & ) const
{
  return VectorProperties();
}

QVariantMap QgsProcessingAlgorithm::run( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback, bool *ok, const QVariantMap &configuration, bool catchExceptions ) const
{
  std::unique_ptr< QgsProcessingAlgorithm > alg( create( configuration ) );
  if ( ok )
    *ok = false;

  bool res = alg->prepare( parameters, context, feedback );
  if ( !res )
    return QVariantMap();

  QVariantMap runRes;
  try
  {
    runRes = alg->runPrepared( parameters, context, feedback );
  }
  catch ( QgsProcessingException &e )
  {
    if ( !catchExceptions )
      throw e;

    QgsMessageLog::logMessage( e.what(), QObject::tr( "Processing" ), Qgis::MessageLevel::Critical );
    feedback->reportError( e.what() );
    return QVariantMap();
  }

  if ( ok )
    *ok = true;

  QVariantMap ppRes = alg->postProcess( context, feedback );
  if ( !ppRes.isEmpty() )
    return ppRes;
  else
    return runRes;
}

bool QgsProcessingAlgorithm::prepare( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
{
  Q_ASSERT_X( QThread::currentThread() == context.temporaryLayerStore()->thread(), "QgsProcessingAlgorithm::prepare", "prepare() must be called from the same thread as context was created in" );
  Q_ASSERT_X( !mHasPrepared, "QgsProcessingAlgorithm::prepare", "prepare() has already been called for the algorithm instance" );
  try
  {
    mHasPrepared = prepareAlgorithm( parameters, context, feedback );
    return mHasPrepared;
  }
  catch ( QgsProcessingException &e )
  {
    QgsMessageLog::logMessage( e.what(), QObject::tr( "Processing" ), Qgis::MessageLevel::Critical );
    feedback->reportError( e.what() );
    return false;
  }
}

QVariantMap QgsProcessingAlgorithm::runPrepared( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
{
  Q_ASSERT_X( mHasPrepared, "QgsProcessingAlgorithm::runPrepared", QStringLiteral( "prepare() was not called for the algorithm instance %1" ).arg( name() ).toLatin1() );
  Q_ASSERT_X( !mHasExecuted, "QgsProcessingAlgorithm::runPrepared", "runPrepared() was already called for this algorithm instance" );

  // Hey kids, let's all be thread safe! It's the fun thing to do!
  //
  // First, let's see if we're going to run into issues.
  QgsProcessingContext *runContext = nullptr;
  if ( context.thread() == QThread::currentThread() )
  {
    // OH. No issues. Seems you're running everything in the same thread, so go about your business. Sorry about
    // the intrusion, we're just making sure everything's nice and safe here. We like to keep a clean and tidy neighbourhood,
    // you know, for the kids and dogs and all.
    runContext = &context;
  }
  else
  {
    // HA! I knew things looked a bit suspicious - seems you're running this algorithm in a different thread
    // from that which the passed context has an affinity for. That's fine and all, but we need to make sure
    // we proceed safely...

    // So first we create a temporary local context with affinity for the current thread
    mLocalContext.reset( new QgsProcessingContext() );
    // copy across everything we can safely do from the passed context
    mLocalContext->copyThreadSafeSettings( context );
    // and we'll run the actual algorithm processing using the local thread safe context
    runContext = mLocalContext.get();
  }

  try
  {
    QVariantMap runResults = processAlgorithm( parameters, *runContext, feedback );

    mHasExecuted = true;
    if ( mLocalContext )
    {
      // ok, time to clean things up. We need to push the temporary context back into
      // the thread that the passed context is associated with (we can only push from the
      // current thread, so we HAVE to do this here)
      mLocalContext->pushToThread( context.thread() );
    }
    return runResults;
  }
  catch ( QgsProcessingException & )
  {
    if ( mLocalContext )
    {
      // see above!
      mLocalContext->pushToThread( context.thread() );
    }
    //rethrow
    throw;
  }
}

QVariantMap QgsProcessingAlgorithm::postProcess( QgsProcessingContext &context, QgsProcessingFeedback *feedback )
{
  Q_ASSERT_X( QThread::currentThread() == context.temporaryLayerStore()->thread(), "QgsProcessingAlgorithm::postProcess", "postProcess() must be called from the same thread the context was created in" );
  Q_ASSERT_X( mHasExecuted, "QgsProcessingAlgorithm::postProcess", QStringLiteral( "algorithm instance %1 was not executed" ).arg( name() ).toLatin1() );
  Q_ASSERT_X( !mHasPostProcessed, "QgsProcessingAlgorithm::postProcess", "postProcess() was already called for this algorithm instance" );

  if ( mLocalContext )
  {
    // algorithm was processed using a temporary thread safe context. So now we need
    // to take the results from that temporary context, and smash them into the passed
    // context
    context.takeResultsFrom( *mLocalContext );
    // now get lost, we don't need you anymore
    mLocalContext.reset();
  }

  mHasPostProcessed = true;
  try
  {
    return postProcessAlgorithm( context, feedback );
  }
  catch ( QgsProcessingException &e )
  {
    QgsMessageLog::logMessage( e.what(), QObject::tr( "Processing" ), Qgis::MessageLevel::Critical );
    feedback->reportError( e.what() );
    return QVariantMap();
  }
}

QString QgsProcessingAlgorithm::parameterAsString( const QVariantMap &parameters, const QString &name, const QgsProcessingContext &context ) const
{
  return QgsProcessingParameters::parameterAsString( parameterDefinition( name ), parameters, context );
}

QString QgsProcessingAlgorithm::parameterAsExpression( const QVariantMap &parameters, const QString &name, const QgsProcessingContext &context ) const
{
  return QgsProcessingParameters::parameterAsExpression( parameterDefinition( name ), parameters, context );
}

double QgsProcessingAlgorithm::parameterAsDouble( const QVariantMap &parameters, const QString &name, const QgsProcessingContext &context ) const
{
  return QgsProcessingParameters::parameterAsDouble( parameterDefinition( name ), parameters, context );
}

int QgsProcessingAlgorithm::parameterAsInt( const QVariantMap &parameters, const QString &name, const QgsProcessingContext &context ) const
{
  return QgsProcessingParameters::parameterAsInt( parameterDefinition( name ), parameters, context );
}

QList<int> QgsProcessingAlgorithm::parameterAsInts( const QVariantMap &parameters, const QString &name, const QgsProcessingContext &context ) const
{
  return QgsProcessingParameters::parameterAsInts( parameterDefinition( name ), parameters, context );
}

int QgsProcessingAlgorithm::parameterAsEnum( const QVariantMap &parameters, const QString &name, const QgsProcessingContext &context ) const
{
  return QgsProcessingParameters::parameterAsEnum( parameterDefinition( name ), parameters, context );
}

QList<int> QgsProcessingAlgorithm::parameterAsEnums( const QVariantMap &parameters, const QString &name, const QgsProcessingContext &context ) const
{
  return QgsProcessingParameters::parameterAsEnums( parameterDefinition( name ), parameters, context );
}

QString QgsProcessingAlgorithm::parameterAsEnumString( const QVariantMap &parameters, const QString &name, const QgsProcessingContext &context ) const
{
  return QgsProcessingParameters::parameterAsEnumString( parameterDefinition( name ), parameters, context );
}

QStringList QgsProcessingAlgorithm::parameterAsEnumStrings( const QVariantMap &parameters, const QString &name, const QgsProcessingContext &context ) const
{
  return QgsProcessingParameters::parameterAsEnumStrings( parameterDefinition( name ), parameters, context );
}

bool QgsProcessingAlgorithm::parameterAsBool( const QVariantMap &parameters, const QString &name, const QgsProcessingContext &context ) const
{
  return QgsProcessingParameters::parameterAsBool( parameterDefinition( name ), parameters, context );
}

bool QgsProcessingAlgorithm::parameterAsBoolean( const QVariantMap &parameters, const QString &name, const QgsProcessingContext &context ) const
{
  return QgsProcessingParameters::parameterAsBool( parameterDefinition( name ), parameters, context );
}

QgsFeatureSink *QgsProcessingAlgorithm::parameterAsSink( const QVariantMap &parameters, const QString &name, QgsProcessingContext &context, QString &destinationIdentifier, const QgsFields &fields, QgsWkbTypes::Type geometryType, const QgsCoordinateReferenceSystem &crs, QgsFeatureSink::SinkFlags sinkFlags, const QVariantMap &createOptions, const QStringList &datasourceOptions, const QStringList &layerOptions ) const
{
  if ( !parameterDefinition( name ) )
    throw QgsProcessingException( QObject::tr( "No parameter definition for the sink '%1'" ).arg( name ) );

  return QgsProcessingParameters::parameterAsSink( parameterDefinition( name ), parameters, fields, geometryType, crs, context, destinationIdentifier, sinkFlags, createOptions, datasourceOptions, layerOptions );
}

QgsProcessingFeatureSource *QgsProcessingAlgorithm::parameterAsSource( const QVariantMap &parameters, const QString &name, QgsProcessingContext &context ) const
{
  return QgsProcessingParameters::parameterAsSource( parameterDefinition( name ), parameters, context );
}

QString QgsProcessingAlgorithm::parameterAsCompatibleSourceLayerPath( const QVariantMap &parameters, const QString &name, QgsProcessingContext &context, const QStringList &compatibleFormats, const QString &preferredFormat, QgsProcessingFeedback *feedback ) const
{
  return QgsProcessingParameters::parameterAsCompatibleSourceLayerPath( parameterDefinition( name ), parameters, context, compatibleFormats, preferredFormat, feedback );
}

QString QgsProcessingAlgorithm::parameterAsCompatibleSourceLayerPathAndLayerName( const QVariantMap &parameters, const QString &name, QgsProcessingContext &context, const QStringList &compatibleFormats, const QString &preferredFormat, QgsProcessingFeedback *feedback, QString *layerName ) const
{
  return QgsProcessingParameters::parameterAsCompatibleSourceLayerPathAndLayerName( parameterDefinition( name ), parameters, context, compatibleFormats, preferredFormat, feedback, layerName );
}

QgsMapLayer *QgsProcessingAlgorithm::parameterAsLayer( const QVariantMap &parameters, const QString &name, QgsProcessingContext &context ) const
{
  return QgsProcessingParameters::parameterAsLayer( parameterDefinition( name ), parameters, context );
}

QgsRasterLayer *QgsProcessingAlgorithm::parameterAsRasterLayer( const QVariantMap &parameters, const QString &name, QgsProcessingContext &context ) const
{
  return QgsProcessingParameters::parameterAsRasterLayer( parameterDefinition( name ), parameters, context );
}

QgsMeshLayer *QgsProcessingAlgorithm::parameterAsMeshLayer( const QVariantMap &parameters, const QString &name, QgsProcessingContext &context ) const
{
  return QgsProcessingParameters::parameterAsMeshLayer( parameterDefinition( name ), parameters, context );
}

QString QgsProcessingAlgorithm::parameterAsOutputLayer( const QVariantMap &parameters, const QString &name, QgsProcessingContext &context ) const
{
  return QgsProcessingParameters::parameterAsOutputLayer( parameterDefinition( name ), parameters, context );
}

QString QgsProcessingAlgorithm::parameterAsFileOutput( const QVariantMap &parameters, const QString &name, QgsProcessingContext &context ) const
{
  return QgsProcessingParameters::parameterAsFileOutput( parameterDefinition( name ), parameters, context );
}

QgsVectorLayer *QgsProcessingAlgorithm::parameterAsVectorLayer( const QVariantMap &parameters, const QString &name, QgsProcessingContext &context ) const
{
  return QgsProcessingParameters::parameterAsVectorLayer( parameterDefinition( name ), parameters, context );
}

QgsCoordinateReferenceSystem QgsProcessingAlgorithm::parameterAsCrs( const QVariantMap &parameters, const QString &name, QgsProcessingContext &context ) const
{
  return QgsProcessingParameters::parameterAsCrs( parameterDefinition( name ), parameters, context );
}

QgsCoordinateReferenceSystem QgsProcessingAlgorithm::parameterAsExtentCrs( const QVariantMap &parameters, const QString &name, QgsProcessingContext &context ) const
{
  return QgsProcessingParameters::parameterAsExtentCrs( parameterDefinition( name ), parameters, context );
}

QgsRectangle QgsProcessingAlgorithm::parameterAsExtent( const QVariantMap &parameters, const QString &name, QgsProcessingContext &context, const QgsCoordinateReferenceSystem &crs ) const
{
  return QgsProcessingParameters::parameterAsExtent( parameterDefinition( name ), parameters, context, crs );
}

QgsGeometry QgsProcessingAlgorithm::parameterAsExtentGeometry( const QVariantMap &parameters, const QString &name, QgsProcessingContext &context, const QgsCoordinateReferenceSystem &crs ) const
{
  return QgsProcessingParameters::parameterAsExtentGeometry( parameterDefinition( name ), parameters, context, crs );
}

QgsPointXY QgsProcessingAlgorithm::parameterAsPoint( const QVariantMap &parameters, const QString &name, QgsProcessingContext &context, const QgsCoordinateReferenceSystem &crs ) const
{
  return QgsProcessingParameters::parameterAsPoint( parameterDefinition( name ), parameters, context, crs );
}

QgsCoordinateReferenceSystem QgsProcessingAlgorithm::parameterAsPointCrs( const QVariantMap &parameters, const QString &name, QgsProcessingContext &context ) const
{
  return QgsProcessingParameters::parameterAsPointCrs( parameterDefinition( name ), parameters, context );
}

QgsGeometry QgsProcessingAlgorithm::parameterAsGeometry( const QVariantMap &parameters, const QString &name, QgsProcessingContext &context, const QgsCoordinateReferenceSystem &crs ) const
{
  return QgsProcessingParameters::parameterAsGeometry( parameterDefinition( name ), parameters, context, crs );
}

QgsCoordinateReferenceSystem QgsProcessingAlgorithm::parameterAsGeometryCrs( const QVariantMap &parameters, const QString &name, QgsProcessingContext &context ) const
{
  return QgsProcessingParameters::parameterAsGeometryCrs( parameterDefinition( name ), parameters, context );
}

QString QgsProcessingAlgorithm::parameterAsFile( const QVariantMap &parameters, const QString &name, QgsProcessingContext &context ) const
{
  return QgsProcessingParameters::parameterAsFile( parameterDefinition( name ), parameters, context );
}

QVariantList QgsProcessingAlgorithm::parameterAsMatrix( const QVariantMap &parameters, const QString &name, QgsProcessingContext &context ) const
{
  return QgsProcessingParameters::parameterAsMatrix( parameterDefinition( name ), parameters, context );
}

QList<QgsMapLayer *> QgsProcessingAlgorithm::parameterAsLayerList( const QVariantMap &parameters, const QString &name, QgsProcessingContext &context ) const
{
  return QgsProcessingParameters::parameterAsLayerList( parameterDefinition( name ), parameters, context );
}

QStringList QgsProcessingAlgorithm::parameterAsFileList( const QVariantMap &parameters, const QString &name, QgsProcessingContext &context ) const
{
  return QgsProcessingParameters::parameterAsFileList( parameterDefinition( name ), parameters, context );
}

QList<double> QgsProcessingAlgorithm::parameterAsRange( const QVariantMap &parameters, const QString &name, QgsProcessingContext &context ) const
{
  return QgsProcessingParameters::parameterAsRange( parameterDefinition( name ), parameters, context );
}

QStringList QgsProcessingAlgorithm::parameterAsFields( const QVariantMap &parameters, const QString &name, QgsProcessingContext &context ) const
{
  return QgsProcessingParameters::parameterAsFields( parameterDefinition( name ), parameters, context );
}

QgsPrintLayout *QgsProcessingAlgorithm::parameterAsLayout( const QVariantMap &parameters, const QString &name, QgsProcessingContext &context )
{
  return QgsProcessingParameters::parameterAsLayout( parameterDefinition( name ), parameters, context );
}

QgsLayoutItem *QgsProcessingAlgorithm::parameterAsLayoutItem( const QVariantMap &parameters, const QString &name, QgsProcessingContext &context, QgsPrintLayout *layout )
{
  return QgsProcessingParameters::parameterAsLayoutItem( parameterDefinition( name ), parameters, context, layout );
}

QColor QgsProcessingAlgorithm::parameterAsColor( const QVariantMap &parameters, const QString &name, QgsProcessingContext &context ) const
{
  return QgsProcessingParameters::parameterAsColor( parameterDefinition( name ), parameters, context );
}

QString QgsProcessingAlgorithm::parameterAsConnectionName( const QVariantMap &parameters, const QString &name, QgsProcessingContext &context ) const
{
  return QgsProcessingParameters::parameterAsConnectionName( parameterDefinition( name ), parameters, context );
}

QDateTime QgsProcessingAlgorithm::parameterAsDateTime( const QVariantMap &parameters, const QString &name, QgsProcessingContext &context ) const
{
  return QgsProcessingParameters::parameterAsDateTime( parameterDefinition( name ), parameters, context );
}

QString QgsProcessingAlgorithm::parameterAsSchema( const QVariantMap &parameters, const QString &name, QgsProcessingContext &context ) const
{
  return QgsProcessingParameters::parameterAsSchema( parameterDefinition( name ), parameters, context );
}

QString QgsProcessingAlgorithm::parameterAsDatabaseTableName( const QVariantMap &parameters, const QString &name, QgsProcessingContext &context ) const
{
  return QgsProcessingParameters::parameterAsDatabaseTableName( parameterDefinition( name ), parameters, context );
}

QgsPointCloudLayer *QgsProcessingAlgorithm::parameterAsPointCloudLayer( const QVariantMap &parameters, const QString &name, QgsProcessingContext &context ) const
{
  return QgsProcessingParameters::parameterAsPointCloudLayer( parameterDefinition( name ), parameters, context );
}

QgsAnnotationLayer *QgsProcessingAlgorithm::parameterAsAnnotationLayer( const QVariantMap &parameters, const QString &name, QgsProcessingContext &context ) const
{
  return QgsProcessingParameters::parameterAsAnnotationLayer( parameterDefinition( name ), parameters, context );
}

QString QgsProcessingAlgorithm::invalidSourceError( const QVariantMap &parameters, const QString &name )
{
  if ( !parameters.contains( name ) )
    return QObject::tr( "Could not load source layer for %1: no value specified for parameter" ).arg( name );
  else
  {
    QVariant var = parameters.value( name );
    if ( var.userType() == QMetaType::type( "QgsProcessingFeatureSourceDefinition" ) )
    {
      QgsProcessingFeatureSourceDefinition fromVar = qvariant_cast<QgsProcessingFeatureSourceDefinition>( var );
      var = fromVar.source;
    }
    else if ( var.userType() == QMetaType::type( "QgsProcessingOutputLayerDefinition" ) )
    {
      QgsProcessingOutputLayerDefinition fromVar = qvariant_cast<QgsProcessingOutputLayerDefinition>( var );
      var = fromVar.sink;
    }
    if ( var.userType() == QMetaType::type( "QgsProperty" ) )
    {
      QgsProperty p = var.value< QgsProperty >();
      if ( p.propertyType() == QgsProperty::StaticProperty )
      {
        var = p.staticValue();
      }
    }
    if ( !var.toString().isEmpty() )
      return QObject::tr( "Could not load source layer for %1: %2 not found" ).arg( name, var.toString() );
    else
      return QObject::tr( "Could not load source layer for %1: invalid value" ).arg( name );
  }
}

QString QgsProcessingAlgorithm::invalidRasterError( const QVariantMap &parameters, const QString &name )
{
  if ( !parameters.contains( name ) )
    return QObject::tr( "Could not load source layer for %1: no value specified for parameter" ).arg( name );
  else
  {
    QVariant var = parameters.value( name );
    if ( var.userType() == QMetaType::type( "QgsProperty" ) )
    {
      QgsProperty p = var.value< QgsProperty >();
      if ( p.propertyType() == QgsProperty::StaticProperty )
      {
        var = p.staticValue();
      }
    }
    if ( !var.toString().isEmpty() )
      return QObject::tr( "Could not load source layer for %1: %2 not found" ).arg( name, var.toString() );
    else
      return QObject::tr( "Could not load source layer for %1: invalid value" ).arg( name );
  }
}

QString QgsProcessingAlgorithm::invalidSinkError( const QVariantMap &parameters, const QString &name )
{
  if ( !parameters.contains( name ) )
    return QObject::tr( "Could not create destination layer for %1: no value specified for parameter" ).arg( name );
  else
  {
    QVariant var = parameters.value( name );
    if ( var.userType() == QMetaType::type( "QgsProcessingOutputLayerDefinition" ) )
    {
      QgsProcessingOutputLayerDefinition fromVar = qvariant_cast<QgsProcessingOutputLayerDefinition>( var );
      var = fromVar.sink;
    }
    if ( var.userType() == QMetaType::type( "QgsProperty" ) )
    {
      QgsProperty p = var.value< QgsProperty >();
      if ( p.propertyType() == QgsProperty::StaticProperty )
      {
        var = p.staticValue();
      }
    }
    if ( !var.toString().isEmpty() )
      return QObject::tr( "Could not create destination layer for %1: %2" ).arg( name, var.toString() );
    else
      return QObject::tr( "Could not create destination layer for %1: invalid value" ).arg( name );
  }
}

QString QgsProcessingAlgorithm::writeFeatureError( QgsFeatureSink *sink, const QVariantMap &parameters, const QString &name )
{
  Q_UNUSED( sink );
  Q_UNUSED( parameters );
  if ( !name.isEmpty() )
    return QObject::tr( "Could not write feature into %1" ).arg( name );
  else
    return QObject::tr( "Could not write feature" );
}

bool QgsProcessingAlgorithm::supportInPlaceEdit( const QgsMapLayer *layer ) const
{
  Q_UNUSED( layer )
  return false;
}


bool QgsProcessingAlgorithm::createAutoOutputForParameter( QgsProcessingParameterDefinition *parameter )
{
  if ( !parameter->isDestination() )
    return true; // nothing created, but nothing went wrong - so return true

  QgsProcessingDestinationParameter *dest = static_cast< QgsProcessingDestinationParameter * >( parameter );
  QgsProcessingOutputDefinition *output( dest->toOutputDefinition() );
  if ( !output )
    return true; // nothing created - but nothing went wrong - so return true
  output->setAutoCreated( true );

  if ( !addOutput( output ) )
  {
    // couldn't add output - probably a duplicate name
    return false;
  }
  else
  {
    return true;
  }
}


//
// QgsProcessingFeatureBasedAlgorithm
//

QgsProcessingAlgorithm::Flags QgsProcessingFeatureBasedAlgorithm::flags() const
{
  Flags f = QgsProcessingAlgorithm::flags();
  f |= QgsProcessingAlgorithm::FlagSupportsInPlaceEdits;
  return f;
}

void QgsProcessingFeatureBasedAlgorithm::initAlgorithm( const QVariantMap &config )
{
  addParameter( new QgsProcessingParameterFeatureSource( inputParameterName(), inputParameterDescription(), inputLayerTypes() ) );
  initParameters( config );
  addParameter( new QgsProcessingParameterFeatureSink( QStringLiteral( "OUTPUT" ), outputName(), outputLayerType(), QVariant(), false, true, true ) );
}

QString QgsProcessingFeatureBasedAlgorithm::inputParameterName() const
{
  return QStringLiteral( "INPUT" );
}

QString QgsProcessingFeatureBasedAlgorithm::inputParameterDescription() const
{
  return QObject::tr( "Input layer" );
}

QList<int> QgsProcessingFeatureBasedAlgorithm::inputLayerTypes() const
{
  return QList<int>();
}

QgsProcessing::SourceType QgsProcessingFeatureBasedAlgorithm::outputLayerType() const
{
  return QgsProcessing::TypeVectorAnyGeometry;
}

QgsProcessingFeatureSource::Flag QgsProcessingFeatureBasedAlgorithm::sourceFlags() const
{
  return static_cast<QgsProcessingFeatureSource::Flag>( 0 );
}

QgsFeatureSink::SinkFlags QgsProcessingFeatureBasedAlgorithm::sinkFlags() const
{
  return QgsFeatureSink::SinkFlags();
}

QgsWkbTypes::Type QgsProcessingFeatureBasedAlgorithm::outputWkbType( QgsWkbTypes::Type inputWkbType ) const
{
  return inputWkbType;
}

QgsFields QgsProcessingFeatureBasedAlgorithm::outputFields( const QgsFields &inputFields ) const
{
  return inputFields;
}

QgsCoordinateReferenceSystem QgsProcessingFeatureBasedAlgorithm::outputCrs( const QgsCoordinateReferenceSystem &inputCrs ) const
{
  return inputCrs;
}

void QgsProcessingFeatureBasedAlgorithm::initParameters( const QVariantMap & )
{
}

QgsCoordinateReferenceSystem QgsProcessingFeatureBasedAlgorithm::sourceCrs() const
{
  if ( mSource )
    return mSource->sourceCrs();
  else
    return QgsCoordinateReferenceSystem();
}

QVariantMap QgsProcessingFeatureBasedAlgorithm::processAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
{
  prepareSource( parameters, context );
  QString dest;
  std::unique_ptr< QgsFeatureSink > sink( parameterAsSink( parameters, QStringLiteral( "OUTPUT" ), context, dest,
                                          outputFields( mSource->fields() ),
                                          outputWkbType( mSource->wkbType() ),
                                          outputCrs( mSource->sourceCrs() ),
                                          sinkFlags() ) );
  if ( !sink )
    throw QgsProcessingException( invalidSinkError( parameters, QStringLiteral( "OUTPUT" ) ) );

  // prepare expression context for feature iteration
  QgsExpressionContext prevContext = context.expressionContext();
  QgsExpressionContext algContext = prevContext;

  algContext.appendScopes( createExpressionContext( parameters, context, mSource.get() ).takeScopes() );
  context.setExpressionContext( algContext );

  long count = mSource->featureCount();

  QgsFeature f;
  QgsFeatureIterator it = mSource->getFeatures( request(), sourceFlags() );

  double step = count > 0 ? 100.0 / count : 1;
  int current = 0;
  while ( it.nextFeature( f ) )
  {
    if ( feedback->isCanceled() )
    {
      break;
    }

    context.expressionContext().setFeature( f );
    const QgsFeatureList transformed = processFeature( f, context, feedback );
    for ( QgsFeature transformedFeature : transformed )
      sink->addFeature( transformedFeature, QgsFeatureSink::FastInsert );

    feedback->setProgress( current * step );
    current++;
  }

  mSource.reset();

  // probably not necessary - context's aren't usually recycled, but can't hurt
  context.setExpressionContext( prevContext );

  QVariantMap outputs;
  outputs.insert( QStringLiteral( "OUTPUT" ), dest );
  return outputs;
}

QgsFeatureRequest QgsProcessingFeatureBasedAlgorithm::request() const
{
  return QgsFeatureRequest();
}

bool QgsProcessingFeatureBasedAlgorithm::supportInPlaceEdit( const QgsMapLayer *l ) const
{
  const QgsVectorLayer *layer = qobject_cast< const QgsVectorLayer * >( l );
  if ( !layer )
    return false;

  QgsWkbTypes::GeometryType inPlaceGeometryType = layer->geometryType();
  if ( !inputLayerTypes().empty() &&
       !inputLayerTypes().contains( QgsProcessing::TypeVector ) &&
       !inputLayerTypes().contains( QgsProcessing::TypeVectorAnyGeometry ) &&
       ( ( inPlaceGeometryType == QgsWkbTypes::PolygonGeometry && !inputLayerTypes().contains( QgsProcessing::TypeVectorPolygon ) ) ||
         ( inPlaceGeometryType == QgsWkbTypes::LineGeometry && !inputLayerTypes().contains( QgsProcessing::TypeVectorLine ) ) ||
         ( inPlaceGeometryType == QgsWkbTypes::PointGeometry && !inputLayerTypes().contains( QgsProcessing::TypeVectorPoint ) ) ) )
    return false;

  QgsWkbTypes::Type type = QgsWkbTypes::Unknown;
  if ( inPlaceGeometryType == QgsWkbTypes::PointGeometry )
    type = QgsWkbTypes::Point;
  else if ( inPlaceGeometryType == QgsWkbTypes::LineGeometry )
    type = QgsWkbTypes::LineString;
  else if ( inPlaceGeometryType == QgsWkbTypes::PolygonGeometry )
    type = QgsWkbTypes::Polygon;

  if ( QgsWkbTypes::geometryType( outputWkbType( type ) ) != inPlaceGeometryType )
    return false;

  return true;
}

void QgsProcessingFeatureBasedAlgorithm::prepareSource( const QVariantMap &parameters, QgsProcessingContext &context )
{
  if ( ! mSource )
  {
    mSource.reset( parameterAsSource( parameters, inputParameterName(), context ) );
    if ( !mSource )
      throw QgsProcessingException( invalidSourceError( parameters, inputParameterName() ) );
  }
}


QgsProcessingAlgorithm::VectorProperties QgsProcessingFeatureBasedAlgorithm::sinkProperties( const QString &sink, const QVariantMap &parameters, QgsProcessingContext &context, const QMap<QString, QgsProcessingAlgorithm::VectorProperties> &sourceProperties ) const
{
  QgsProcessingAlgorithm::VectorProperties result;
  if ( sink == QLatin1String( "OUTPUT" ) )
  {
    if ( sourceProperties.value( QStringLiteral( "INPUT" ) ).availability == QgsProcessingAlgorithm::Available )
    {
      const VectorProperties inputProps = sourceProperties.value( QStringLiteral( "INPUT" ) );
      result.fields = outputFields( inputProps.fields );
      result.crs = outputCrs( inputProps.crs );
      result.wkbType = outputWkbType( inputProps.wkbType );
      result.availability = Available;
      return result;
    }
    else
    {
      std::unique_ptr< QgsProcessingFeatureSource > source( parameterAsSource( parameters, QStringLiteral( "INPUT" ), context ) );
      if ( source )
      {
        result.fields = outputFields( source->fields() );
        result.crs = outputCrs( source->sourceCrs() );
        result.wkbType = outputWkbType( source->wkbType() );
        result.availability = Available;
        return result;
      }
    }
  }
  return result;
}

