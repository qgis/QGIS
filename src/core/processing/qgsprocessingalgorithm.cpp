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
#include "qgsprocessingfeedback.h"

QgsProcessingAlgorithm::~QgsProcessingAlgorithm()
{
  qDeleteAll( mParameters );
  qDeleteAll( mOutputs );
}

QgsProcessingAlgorithm *QgsProcessingAlgorithm::create( const QVariantMap &configuration ) const
{
  std::unique_ptr< QgsProcessingAlgorithm > creation( createInstance() );
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
  Q_FOREACH ( const QgsProcessingParameterDefinition *def, mParameters )
  {
    if ( !def->checkValueIsAcceptable( parameters.value( def->name() ), &context ) )
    {
      if ( message )
        *message = QObject::tr( "Incorrect parameter value for %1" ).arg( def->name() );
      return false;
    }
  }
  return true;
}

QgsProcessingProvider *QgsProcessingAlgorithm::provider() const
{
  return mProvider;
}

void QgsProcessingAlgorithm::setProvider( QgsProcessingProvider *provider )
{
  mProvider = provider;

  // need to update all destination parameters to set whether the provider supports non file based outputs
  Q_FOREACH ( const QgsProcessingParameterDefinition *definition, mParameters )
  {
    if ( definition->isDestination() && mProvider )
    {
      const QgsProcessingDestinationParameter *destParam = static_cast< const QgsProcessingDestinationParameter *>( definition );
      const_cast< QgsProcessingDestinationParameter *>( destParam )->setSupportsNonFileBasedOutputs( mProvider->supportsNonFileBasedOutput() );
    }
  }
}

QWidget *QgsProcessingAlgorithm::createCustomParametersWidget( QWidget * ) const
{
  return nullptr;
}

QgsExpressionContext QgsProcessingAlgorithm::createExpressionContext( const QVariantMap &parameters,
    QgsProcessingContext &context ) const
{
  // start with context's expression context
  QgsExpressionContext c = context.expressionContext();
  if ( c.scopeCount() == 0 )
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
  Q_FOREACH ( const QgsProcessingParameterDefinition *def, mParameters )
  {
    if ( def->type() == QStringLiteral( "layer" ) || def->type() == QStringLiteral( "raster" ) )
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
    else if ( def->type() == QStringLiteral( "source" ) )
    {
      QgsFeatureSource *source = QgsProcessingParameters::parameterAsSource( def, parameters, context );
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
    else if ( def->type() == QStringLiteral( "multilayer" ) )
    {
      QList< QgsMapLayer *> layers = QgsProcessingParameters::parameterAsLayerList( def, parameters, context );
      Q_FOREACH ( QgsMapLayer *layer, layers )
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
  }
  return true;
}

QString QgsProcessingAlgorithm::asPythonCommand( const QVariantMap &parameters, QgsProcessingContext &context ) const
{
  QString s = QStringLiteral( "processing.run(\"%1\"," ).arg( id() );

  QStringList parts;
  Q_FOREACH ( const QgsProcessingParameterDefinition *def, mParameters )
  {
    if ( def->flags() & QgsProcessingParameterDefinition::FlagHidden )
      continue;

    if ( !parameters.contains( def->name() ) || !parameters.value( def->name() ).isValid() )
      continue;

    parts << QStringLiteral( "'%1':%2" ).arg( def->name(), def->valueAsPythonString( parameters.value( def->name() ), context ) );
  }

  s += QStringLiteral( " {%1})" ).arg( parts.join( ',' ) );
  return s;
}

bool QgsProcessingAlgorithm::addParameter( QgsProcessingParameterDefinition *definition, bool createOutput )
{
  if ( !definition )
    return false;

  // check for duplicate named parameters
  if ( QgsProcessingAlgorithm::parameterDefinition( definition->name() ) )
    return false;

  if ( definition->isDestination() && mProvider )
  {
    QgsProcessingDestinationParameter *destParam = static_cast< QgsProcessingDestinationParameter *>( definition );
    destParam->setSupportsNonFileBasedOutputs( mProvider->supportsNonFileBasedOutput() );
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
  }
}

bool QgsProcessingAlgorithm::addOutput( QgsProcessingOutputDefinition *definition )
{
  if ( !definition )
    return false;

  // check for duplicate named outputs
  if ( QgsProcessingAlgorithm::outputDefinition( definition->name() ) )
    return false;

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
  Q_FOREACH ( const QgsProcessingParameterDefinition *def, mParameters )
  {
    if ( def->name().compare( name, Qt::CaseInsensitive ) == 0 )
      return def;
  }
  return nullptr;
}

int QgsProcessingAlgorithm::countVisibleParameters() const
{
  int count = 0;
  Q_FOREACH ( const QgsProcessingParameterDefinition *def, mParameters )
  {
    if ( !( def->flags() & QgsProcessingParameterDefinition::FlagHidden ) )
      count++;
  }
  return count;
}

QgsProcessingParameterDefinitions QgsProcessingAlgorithm::destinationParameterDefinitions() const
{
  QgsProcessingParameterDefinitions result;
  Q_FOREACH ( const QgsProcessingParameterDefinition *def, mParameters )
  {
    if ( !def->isDestination() )
      continue;

    result << def;
  }
  return result;
}

const QgsProcessingOutputDefinition *QgsProcessingAlgorithm::outputDefinition( const QString &name ) const
{
  Q_FOREACH ( const QgsProcessingOutputDefinition *def, mOutputs )
  {
    if ( def->name().compare( name, Qt::CaseInsensitive ) == 0 )
      return def;
  }
  return nullptr;
}

bool QgsProcessingAlgorithm::hasHtmlOutputs() const
{
  Q_FOREACH ( const QgsProcessingOutputDefinition *def, mOutputs )
  {
    if ( def->type() == QStringLiteral( "outputHtml" ) )
      return true;
  }
  return false;
}

QVariantMap QgsProcessingAlgorithm::run( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback, bool *ok ) const
{
  std::unique_ptr< QgsProcessingAlgorithm > alg( create() );
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
    QgsMessageLog::logMessage( e.what(), QObject::tr( "Processing" ), QgsMessageLog::CRITICAL );
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
    QgsMessageLog::logMessage( e.what(), QObject::tr( "Processing" ), QgsMessageLog::CRITICAL );
    feedback->reportError( e.what() );
    return false;
  }
}

QVariantMap QgsProcessingAlgorithm::runPrepared( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
{
  Q_ASSERT_X( mHasPrepared, "QgsProcessingAlgorithm::runPrepared", QString( "prepare() was not called for the algorithm instance %1" ).arg( name() ).toLatin1() );
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
  Q_ASSERT_X( mHasExecuted, "QgsProcessingAlgorithm::postProcess", QString( "algorithm instance %1 was not executed" ).arg( name() ).toLatin1() );
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
    QgsMessageLog::logMessage( e.what(), QObject::tr( "Processing" ), QgsMessageLog::CRITICAL );
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

int QgsProcessingAlgorithm::parameterAsEnum( const QVariantMap &parameters, const QString &name, const QgsProcessingContext &context ) const
{
  return QgsProcessingParameters::parameterAsEnum( parameterDefinition( name ), parameters, context );
}

QList<int> QgsProcessingAlgorithm::parameterAsEnums( const QVariantMap &parameters, const QString &name, const QgsProcessingContext &context ) const
{
  return QgsProcessingParameters::parameterAsEnums( parameterDefinition( name ), parameters, context );
}

bool QgsProcessingAlgorithm::parameterAsBool( const QVariantMap &parameters, const QString &name, const QgsProcessingContext &context ) const
{
  return QgsProcessingParameters::parameterAsBool( parameterDefinition( name ), parameters, context );
}

QgsFeatureSink *QgsProcessingAlgorithm::parameterAsSink( const QVariantMap &parameters, const QString &name, QgsProcessingContext &context, QString &destinationIdentifier, const QgsFields &fields, QgsWkbTypes::Type geometryType, const QgsCoordinateReferenceSystem &crs ) const
{
  return QgsProcessingParameters::parameterAsSink( parameterDefinition( name ), parameters, fields, geometryType, crs, context, destinationIdentifier );
}

QgsProcessingFeatureSource *QgsProcessingAlgorithm::parameterAsSource( const QVariantMap &parameters, const QString &name, QgsProcessingContext &context ) const
{
  return QgsProcessingParameters::parameterAsSource( parameterDefinition( name ), parameters, context );
}

QString QgsProcessingAlgorithm::parameterAsCompatibleSourceLayerPath( const QVariantMap &parameters, const QString &name, QgsProcessingContext &context, const QStringList &compatibleFormats, const QString &preferredFormat, QgsProcessingFeedback *feedback )
{
  return QgsProcessingParameters::parameterAsCompatibleSourceLayerPath( parameterDefinition( name ), parameters, context, compatibleFormats, preferredFormat, feedback );
}

QgsMapLayer *QgsProcessingAlgorithm::parameterAsLayer( const QVariantMap &parameters, const QString &name, QgsProcessingContext &context ) const
{
  return QgsProcessingParameters::parameterAsLayer( parameterDefinition( name ), parameters, context );
}

QgsRasterLayer *QgsProcessingAlgorithm::parameterAsRasterLayer( const QVariantMap &parameters, const QString &name, QgsProcessingContext &context ) const
{
  return QgsProcessingParameters::parameterAsRasterLayer( parameterDefinition( name ), parameters, context );
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

QgsCoordinateReferenceSystem QgsProcessingAlgorithm::parameterAsExtentCrs( const QVariantMap &parameters, const QString &name, QgsProcessingContext &context )
{
  return QgsProcessingParameters::parameterAsCrs( parameterDefinition( name ), parameters, context );
}

QgsRectangle QgsProcessingAlgorithm::parameterAsExtent( const QVariantMap &parameters, const QString &name, QgsProcessingContext &context, const QgsCoordinateReferenceSystem &crs ) const
{
  return QgsProcessingParameters::parameterAsExtent( parameterDefinition( name ), parameters, context, crs );
}

QgsGeometry QgsProcessingAlgorithm::parameterAsExtentGeometry( const QVariantMap &parameters, const QString &name, QgsProcessingContext &context, const QgsCoordinateReferenceSystem &crs )
{
  return QgsProcessingParameters::parameterAsExtentGeometry( parameterDefinition( name ), parameters, context, crs );
}

QgsPointXY QgsProcessingAlgorithm::parameterAsPoint( const QVariantMap &parameters, const QString &name, QgsProcessingContext &context, const QgsCoordinateReferenceSystem &crs ) const
{
  return QgsProcessingParameters::parameterAsPoint( parameterDefinition( name ), parameters, context, crs );
}

QgsCoordinateReferenceSystem QgsProcessingAlgorithm::parameterAsPointCrs( const QVariantMap &parameters, const QString &name, QgsProcessingContext &context )
{
  return QgsProcessingParameters::parameterAsPointCrs( parameterDefinition( name ), parameters, context );
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

QList<double> QgsProcessingAlgorithm::parameterAsRange( const QVariantMap &parameters, const QString &name, QgsProcessingContext &context ) const
{
  return QgsProcessingParameters::parameterAsRange( parameterDefinition( name ), parameters, context );
}

QStringList QgsProcessingAlgorithm::parameterAsFields( const QVariantMap &parameters, const QString &name, QgsProcessingContext &context ) const
{
  return QgsProcessingParameters::parameterAsFields( parameterDefinition( name ), parameters, context );
}

bool QgsProcessingAlgorithm::createAutoOutputForParameter( QgsProcessingParameterDefinition *parameter )
{
  if ( !parameter->isDestination() )
    return true; // nothing created, but nothing went wrong - so return true

  QgsProcessingDestinationParameter *dest = static_cast< QgsProcessingDestinationParameter * >( parameter );
  QgsProcessingOutputDefinition *output( dest->toOutputDefinition() );
  if ( !output )
    return true; // nothing created - but nothing went wrong - so return true

  if ( !addOutput( output ) )
  {
    // couldn't add output - probably a duplicate name
    delete output;
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

void QgsProcessingFeatureBasedAlgorithm::initAlgorithm( const QVariantMap &config )
{
  addParameter( new QgsProcessingParameterFeatureSource( QStringLiteral( "INPUT" ), QObject::tr( "Input layer" ), inputLayerTypes() ) );
  initParameters( config );
  addParameter( new QgsProcessingParameterFeatureSink( QStringLiteral( "OUTPUT" ), outputName(), outputLayerType() ) );
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
  mSource.reset( parameterAsSource( parameters, QStringLiteral( "INPUT" ), context ) );
  if ( !mSource )
    return QVariantMap();

  QString dest;
  std::unique_ptr< QgsFeatureSink > sink( parameterAsSink( parameters, QStringLiteral( "OUTPUT" ), context, dest,
                                          outputFields( mSource->fields() ),
                                          outputWkbType( mSource->wkbType() ),
                                          outputCrs( mSource->sourceCrs() ) ) );
  if ( !sink )
    return QVariantMap();

  long count = mSource->featureCount();

  QgsFeature f;
  QgsFeatureIterator it = mSource->getFeatures( QgsFeatureRequest(), sourceFlags() );

  double step = count > 0 ? 100.0 / count : 1;
  int current = 0;
  while ( it.nextFeature( f ) )
  {
    if ( feedback->isCanceled() )
    {
      break;
    }

    QgsFeature transformed = processFeature( f, feedback );
    if ( transformed.isValid() )
      sink->addFeature( transformed, QgsFeatureSink::FastInsert );

    feedback->setProgress( current * step );
    current++;
  }

  mSource.reset();

  QVariantMap outputs;
  outputs.insert( QStringLiteral( "OUTPUT" ), dest );
  return outputs;
}

QgsFeatureRequest QgsProcessingFeatureBasedAlgorithm::request() const
{
  return QgsFeatureRequest();
}
