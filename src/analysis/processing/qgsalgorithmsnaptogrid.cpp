/***************************************************************************
                         qgsalgorithmsnaptogrid.cpp
                         --------------------------
    begin                : October 2017
    copyright            : (C) 2017 by Nyall Dawson
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

#include "qgsalgorithmsnaptogrid.h"

///@cond PRIVATE

QString QgsSnapToGridAlgorithm::name() const
{
  return QStringLiteral( "snappointstogrid" );
}

QString QgsSnapToGridAlgorithm::displayName() const
{
  return QObject::tr( "Snap points to grid" );
}

QStringList QgsSnapToGridAlgorithm::tags() const
{
  return QObject::tr( "snapped,grid,simplify,round,precision" ).split( ',' );
}

QString QgsSnapToGridAlgorithm::group() const
{
  return QObject::tr( "Vector geometry" );
}

QString QgsSnapToGridAlgorithm::groupId() const
{
  return QStringLiteral( "vectorgeometry" );
}

QString QgsSnapToGridAlgorithm::outputName() const
{
  return QObject::tr( "Snapped" );
}

QString QgsSnapToGridAlgorithm::shortHelpString() const
{
  return QObject::tr( "This algorithm modifies the coordinates of geometries in a vector layer, so that all points "
                      "or vertices are snapped to the closest point of the grid.\n\n"
                      "If the snapped geometry cannot be calculated (or is totally collapsed) the feature's "
                      "geometry will be cleared.\n\n"
                      "Note that snapping to grid may generate an invalid geometry in some corner cases.\n\n"
                      "Snapping can be performed on the X, Y, Z or M axis. A grid spacing of 0 for any axis will "
                      "disable snapping for that axis." );
}

QgsSnapToGridAlgorithm *QgsSnapToGridAlgorithm::createInstance() const
{
  return new QgsSnapToGridAlgorithm();
}

void QgsSnapToGridAlgorithm::initParameters( const QVariantMap & )
{
  std::unique_ptr< QgsProcessingParameterDistance> hSpacing = std::make_unique< QgsProcessingParameterDistance >( QStringLiteral( "HSPACING" ),
      QObject::tr( "X Grid Spacing" ), 1, QStringLiteral( "INPUT" ), false, 0 );
  hSpacing->setIsDynamic( true );
  hSpacing->setDynamicPropertyDefinition( QgsPropertyDefinition( QStringLiteral( "HSPACING" ), QObject::tr( "X Grid Spacing" ), QgsPropertyDefinition::DoublePositive ) );
  hSpacing->setDynamicLayerParameterName( QStringLiteral( "INPUT" ) );
  addParameter( hSpacing.release() );

  std::unique_ptr< QgsProcessingParameterDistance> vSpacing = std::make_unique< QgsProcessingParameterDistance >( QStringLiteral( "VSPACING" ),
      QObject::tr( "Y Grid Spacing" ), 1, QStringLiteral( "INPUT" ), false, 0 );
  vSpacing->setIsDynamic( true );
  vSpacing->setDynamicPropertyDefinition( QgsPropertyDefinition( QStringLiteral( "VSPACING" ), QObject::tr( "Y Grid Spacing" ), QgsPropertyDefinition::DoublePositive ) );
  vSpacing->setDynamicLayerParameterName( QStringLiteral( "INPUT" ) );
  addParameter( vSpacing.release() );

  std::unique_ptr< QgsProcessingParameterNumber > zSpacing = std::make_unique< QgsProcessingParameterNumber >( QStringLiteral( "ZSPACING" ),
      QObject::tr( "Z Grid Spacing" ), QgsProcessingParameterNumber::Double,
      0, false, 0 );
  zSpacing->setIsDynamic( true );
  zSpacing->setDynamicPropertyDefinition( QgsPropertyDefinition( QStringLiteral( "ZSPACING" ), QObject::tr( "Z Grid Spacing" ), QgsPropertyDefinition::DoublePositive ) );
  zSpacing->setDynamicLayerParameterName( QStringLiteral( "INPUT" ) );
  addParameter( zSpacing.release() );

  std::unique_ptr< QgsProcessingParameterNumber > mSpacing = std::make_unique< QgsProcessingParameterNumber >( QStringLiteral( "MSPACING" ),
      QObject::tr( "M Grid Spacing" ), QgsProcessingParameterNumber::Double,
      0, false, 0 );
  mSpacing->setIsDynamic( true );
  mSpacing->setDynamicPropertyDefinition( QgsPropertyDefinition( QStringLiteral( "MSPACING" ), QObject::tr( "M Grid Spacing" ), QgsPropertyDefinition::DoublePositive ) );
  mSpacing->setDynamicLayerParameterName( QStringLiteral( "INPUT" ) );
  addParameter( mSpacing.release() );
}

bool QgsSnapToGridAlgorithm::prepareAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback * )
{
  mIntervalX = parameterAsDouble( parameters, QStringLiteral( "HSPACING" ), context );
  mDynamicIntervalX = QgsProcessingParameters::isDynamic( parameters, QStringLiteral( "HSPACING" ) );
  if ( mDynamicIntervalX )
    mIntervalXProperty = parameters.value( QStringLiteral( "HSPACING" ) ).value< QgsProperty >();

  mIntervalY = parameterAsDouble( parameters, QStringLiteral( "VSPACING" ), context );
  mDynamicIntervalY = QgsProcessingParameters::isDynamic( parameters, QStringLiteral( "VSPACING" ) );
  if ( mDynamicIntervalY )
    mIntervalYProperty = parameters.value( QStringLiteral( "VSPACING" ) ).value< QgsProperty >();

  mIntervalZ = parameterAsDouble( parameters, QStringLiteral( "ZSPACING" ), context );
  mDynamicIntervalZ = QgsProcessingParameters::isDynamic( parameters, QStringLiteral( "ZSPACING" ) );
  if ( mDynamicIntervalZ )
    mIntervalZProperty = parameters.value( QStringLiteral( "ZSPACING" ) ).value< QgsProperty >();

  mIntervalM = parameterAsDouble( parameters, QStringLiteral( "MSPACING" ), context );
  mDynamicIntervalM = QgsProcessingParameters::isDynamic( parameters, QStringLiteral( "MSPACING" ) );
  if ( mDynamicIntervalM )
    mIntervalMProperty = parameters.value( QStringLiteral( "MSPACING" ) ).value< QgsProperty >();

  return true;
}

QgsFeatureList QgsSnapToGridAlgorithm::processFeature( const QgsFeature &feature, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
{
  QgsFeature f = feature;
  if ( f.hasGeometry() )
  {
    double intervalX = mIntervalX;
    if ( mDynamicIntervalX )
      intervalX = mIntervalXProperty.valueAsDouble( context.expressionContext(), intervalX );

    double intervalY = mIntervalY;
    if ( mDynamicIntervalY )
      intervalY = mIntervalYProperty.valueAsDouble( context.expressionContext(), intervalY );

    double intervalZ = mIntervalZ;
    if ( mDynamicIntervalZ )
      intervalZ = mIntervalZProperty.valueAsDouble( context.expressionContext(), intervalZ );

    double intervalM = mIntervalM;
    if ( mDynamicIntervalM )
      intervalM = mIntervalMProperty.valueAsDouble( context.expressionContext(), intervalM );

    const QgsGeometry outputGeometry = f.geometry().snappedToGrid( intervalX, intervalY, intervalZ, intervalM );
    if ( outputGeometry.isNull() )
    {
      feedback->reportError( QObject::tr( "Error snapping geometry %1" ).arg( feature.id() ) );
    }
    f.setGeometry( outputGeometry );
  }
  return QgsFeatureList() << f;
}

QgsProcessingFeatureSource::Flag QgsSnapToGridAlgorithm::sourceFlags() const
{
  return QgsProcessingFeatureSource::FlagSkipGeometryValidityChecks;
}

///@endcond


