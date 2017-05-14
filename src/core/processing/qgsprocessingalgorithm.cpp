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
#include "qgsrectangle.h"

QgsProcessingAlgorithm::~QgsProcessingAlgorithm()
{
  qDeleteAll( mParameters );
}

QString QgsProcessingAlgorithm::id() const
{
  if ( mProvider )
    return QString( "%1:%2" ).arg( mProvider->id(), name() );
  else
    return name();
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

QgsProcessingProvider *QgsProcessingAlgorithm::provider() const
{
  return mProvider;
}

void QgsProcessingAlgorithm::setProvider( QgsProcessingProvider *provider )
{
  mProvider = provider;
}

QVariantMap QgsProcessingAlgorithm::run( const QVariantMap &, QgsProcessingContext &, QgsProcessingFeedback * ) const
{
  return QVariantMap();
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

QString QgsProcessingAlgorithm::parameterAsString( const QVariantMap &parameters, const QString &name, const QgsProcessingContext &context ) const
{
  return QgsProcessingParameters::parameterAsString( parameterDefinition( name ), parameters, name, context );
}

QString QgsProcessingAlgorithm::parameterAsExpression( const QVariantMap &parameters, const QString &name, const QgsProcessingContext &context ) const
{
  return QgsProcessingParameters::parameterAsExpression( parameterDefinition( name ), parameters, name, context );
}

double QgsProcessingAlgorithm::parameterAsDouble( const QVariantMap &parameters, const QString &name, const QgsProcessingContext &context ) const
{
  return QgsProcessingParameters::parameterAsDouble( parameterDefinition( name ), parameters, name, context );
}

int QgsProcessingAlgorithm::parameterAsInt( const QVariantMap &parameters, const QString &name, const QgsProcessingContext &context ) const
{
  return QgsProcessingParameters::parameterAsInt( parameterDefinition( name ), parameters, name, context );
}

int QgsProcessingAlgorithm::parameterAsEnum( const QVariantMap &parameters, const QString &name, const QgsProcessingContext &context ) const
{
  return QgsProcessingParameters::parameterAsEnum( parameterDefinition( name ), parameters, name, context );
}

QList<int> QgsProcessingAlgorithm::parameterAsEnums( const QVariantMap &parameters, const QString &name, const QgsProcessingContext &context ) const
{
  return QgsProcessingParameters::parameterAsEnums( parameterDefinition( name ), parameters, name, context );
}

bool QgsProcessingAlgorithm::parameterAsBool( const QVariantMap &parameters, const QString &name, const QgsProcessingContext &context ) const
{
  return QgsProcessingParameters::parameterAsBool( parameterDefinition( name ), parameters, name, context );
}

QgsMapLayer *QgsProcessingAlgorithm::parameterAsLayer( const QVariantMap &parameters, const QString &name, QgsProcessingContext &context ) const
{
  return QgsProcessingParameters::parameterAsLayer( parameterDefinition( name ), parameters, name, context );
}

QgsRasterLayer *QgsProcessingAlgorithm::parameterAsRasterLayer( const QVariantMap &parameters, const QString &name, QgsProcessingContext &context ) const
{
  return QgsProcessingParameters::parameterAsRasterLayer( parameterDefinition( name ), parameters, name, context );
}

QgsVectorLayer *QgsProcessingAlgorithm::parameterAsVectorLayer( const QVariantMap &parameters, const QString &name, QgsProcessingContext &context ) const
{
  return QgsProcessingParameters::parameterAsVectorLayer( parameterDefinition( name ), parameters, name, context );
}

QgsCoordinateReferenceSystem QgsProcessingAlgorithm::parameterAsCrs( const QVariantMap &parameters, const QString &name, QgsProcessingContext &context ) const
{
  return QgsProcessingParameters::parameterAsCrs( parameterDefinition( name ), parameters, name, context );
}

QgsRectangle QgsProcessingAlgorithm::parameterAsExtent( const QVariantMap &parameters, const QString &name, QgsProcessingContext &context ) const
{
  return QgsProcessingParameters::parameterAsExtent( parameterDefinition( name ), parameters, name, context );
}

QgsPoint QgsProcessingAlgorithm::parameterAsPoint( const QVariantMap &parameters, const QString &name, QgsProcessingContext &context ) const
{
  return QgsProcessingParameters::parameterAsPoint( parameterDefinition( name ), parameters, name, context );
}

QString QgsProcessingAlgorithm::parameterAsFile( const QVariantMap &parameters, const QString &name, QgsProcessingContext &context ) const
{
  return QgsProcessingParameters::parameterAsFile( parameterDefinition( name ), parameters, name, context );
}

QVariantList QgsProcessingAlgorithm::parameterAsMatrix( const QVariantMap &parameters, const QString &name, QgsProcessingContext &context ) const
{
  return QgsProcessingParameters::parameterAsMatrix( parameterDefinition( name ), parameters, name, context );
}

QList<QgsMapLayer *> QgsProcessingAlgorithm::parameterAsLayerList( const QVariantMap &parameters, const QString &name, QgsProcessingContext &context ) const
{
  return QgsProcessingParameters::parameterAsLayerList( parameterDefinition( name ), parameters, name, context );
}

QList<double> QgsProcessingAlgorithm::parameterAsRange( const QVariantMap &parameters, const QString &name, QgsProcessingContext &context ) const
{
  return QgsProcessingParameters::parameterAsRange( parameterDefinition( name ), parameters, name, context );
}

QStringList QgsProcessingAlgorithm::parameterAsFields( const QVariantMap &parameters, const QString &name, QgsProcessingContext &context ) const
{
  return QgsProcessingParameters::parameterAsFields( parameterDefinition( name ), parameters, name, context );
}
