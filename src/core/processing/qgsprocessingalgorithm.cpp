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
  return FlagSupportsBatch;
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

QgsMapLayer *QgsProcessingAlgorithm::parameterAsLayer( const QVariantMap &parameters, const QString &name, QgsProcessingContext &context ) const
{
  return QgsProcessingParameters::parameterAsLayer( parameterDefinition( name ), parameters, context );
}

QgsRasterLayer *QgsProcessingAlgorithm::parameterAsRasterLayer( const QVariantMap &parameters, const QString &name, QgsProcessingContext &context ) const
{
  return QgsProcessingParameters::parameterAsRasterLayer( parameterDefinition( name ), parameters, context );
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


