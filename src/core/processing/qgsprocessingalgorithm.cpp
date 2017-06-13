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

QgsProcessingAlgorithm::~QgsProcessingAlgorithm()
{
  qDeleteAll( mParameters );
  qDeleteAll( mOutputs );
}

QString QgsProcessingAlgorithm::id() const
{
  if ( mProvider )
    return QString( "%1:%2" ).arg( mProvider->id(), name() );
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
  return QgsApplication::iconPath( "processingAlgorithm.svg" );
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

bool QgsProcessingAlgorithm::addParameter( QgsProcessingParameterDefinition *definition )
{
  if ( !definition )
    return false;

  // check for duplicate named parameters
  if ( QgsProcessingAlgorithm::parameterDefinition( definition->name() ) )
    return false;

  mParameters << definition;
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

QVariantMap QgsProcessingAlgorithm::run( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback ) const
{
  return processAlgorithm( parameters, context, feedback );
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

QgsFeatureSink *QgsProcessingAlgorithm::parameterAsSink( const QVariantMap &parameters, const QString &name, QgsProcessingContext &context, const QgsFields &fields, QgsWkbTypes::Type geometryType, const QgsCoordinateReferenceSystem &crs, QString &destinationIdentifier ) const
{
  return QgsProcessingParameters::parameterAsSink( parameterDefinition( name ), parameters, fields, geometryType, crs, context, destinationIdentifier );
}

QgsFeatureSource *QgsProcessingAlgorithm::parameterAsSource( const QVariantMap &parameters, const QString &name, QgsProcessingContext &context ) const
{
  return QgsProcessingParameters::parameterAsSource( parameterDefinition( name ), parameters, context );
}

QgsMapLayer *QgsProcessingAlgorithm::parameterAsLayer( const QVariantMap &parameters, const QString &name, QgsProcessingContext &context ) const
{
  return QgsProcessingParameters::parameterAsLayer( parameterDefinition( name ), parameters, context );
}

QgsRasterLayer *QgsProcessingAlgorithm::parameterAsRasterLayer( const QVariantMap &parameters, const QString &name, QgsProcessingContext &context ) const
{
  return QgsProcessingParameters::parameterAsRasterLayer( parameterDefinition( name ), parameters, context );
}

QString QgsProcessingAlgorithm::parameterAsRasterOutputLayer( const QVariantMap &parameters, const QString &name, QgsProcessingContext &context ) const
{
  return QgsProcessingParameters::parameterAsRasterOutputLayer( parameterDefinition( name ), parameters, context );
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

QgsRectangle QgsProcessingAlgorithm::parameterAsExtent( const QVariantMap &parameters, const QString &name, QgsProcessingContext &context ) const
{
  return QgsProcessingParameters::parameterAsExtent( parameterDefinition( name ), parameters, context );
}

QgsPointXY QgsProcessingAlgorithm::parameterAsPoint( const QVariantMap &parameters, const QString &name, QgsProcessingContext &context ) const
{
  return QgsProcessingParameters::parameterAsPoint( parameterDefinition( name ), parameters, context );
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


