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
  return u"snappointstogrid"_s;
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
  return u"vectorgeometry"_s;
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

QString QgsSnapToGridAlgorithm::shortDescription() const
{
  return QObject::tr( "Modifies the coordinates of geometries in a vector layer, so that all points "
                      "or vertices are snapped to the closest point of a grid." );
}

QgsSnapToGridAlgorithm *QgsSnapToGridAlgorithm::createInstance() const
{
  return new QgsSnapToGridAlgorithm();
}

void QgsSnapToGridAlgorithm::initParameters( const QVariantMap & )
{
  auto hSpacing = std::make_unique<QgsProcessingParameterDistance>( u"HSPACING"_s, QObject::tr( "X Grid Spacing" ), 1, u"INPUT"_s, false, 0 );
  hSpacing->setIsDynamic( true );
  hSpacing->setDynamicPropertyDefinition( QgsPropertyDefinition( u"HSPACING"_s, QObject::tr( "X Grid Spacing" ), QgsPropertyDefinition::DoublePositive ) );
  hSpacing->setDynamicLayerParameterName( u"INPUT"_s );
  addParameter( hSpacing.release() );

  auto vSpacing = std::make_unique<QgsProcessingParameterDistance>( u"VSPACING"_s, QObject::tr( "Y Grid Spacing" ), 1, u"INPUT"_s, false, 0 );
  vSpacing->setIsDynamic( true );
  vSpacing->setDynamicPropertyDefinition( QgsPropertyDefinition( u"VSPACING"_s, QObject::tr( "Y Grid Spacing" ), QgsPropertyDefinition::DoublePositive ) );
  vSpacing->setDynamicLayerParameterName( u"INPUT"_s );
  addParameter( vSpacing.release() );

  auto zSpacing = std::make_unique<QgsProcessingParameterNumber>( u"ZSPACING"_s, QObject::tr( "Z Grid Spacing" ), Qgis::ProcessingNumberParameterType::Double, 0, false, 0 );
  zSpacing->setIsDynamic( true );
  zSpacing->setDynamicPropertyDefinition( QgsPropertyDefinition( u"ZSPACING"_s, QObject::tr( "Z Grid Spacing" ), QgsPropertyDefinition::DoublePositive ) );
  zSpacing->setDynamicLayerParameterName( u"INPUT"_s );
  addParameter( zSpacing.release() );

  auto mSpacing = std::make_unique<QgsProcessingParameterNumber>( u"MSPACING"_s, QObject::tr( "M Grid Spacing" ), Qgis::ProcessingNumberParameterType::Double, 0, false, 0 );
  mSpacing->setIsDynamic( true );
  mSpacing->setDynamicPropertyDefinition( QgsPropertyDefinition( u"MSPACING"_s, QObject::tr( "M Grid Spacing" ), QgsPropertyDefinition::DoublePositive ) );
  mSpacing->setDynamicLayerParameterName( u"INPUT"_s );
  addParameter( mSpacing.release() );
}

bool QgsSnapToGridAlgorithm::prepareAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback * )
{
  mIntervalX = parameterAsDouble( parameters, u"HSPACING"_s, context );
  mDynamicIntervalX = QgsProcessingParameters::isDynamic( parameters, u"HSPACING"_s );
  if ( mDynamicIntervalX )
    mIntervalXProperty = parameters.value( u"HSPACING"_s ).value<QgsProperty>();

  mIntervalY = parameterAsDouble( parameters, u"VSPACING"_s, context );
  mDynamicIntervalY = QgsProcessingParameters::isDynamic( parameters, u"VSPACING"_s );
  if ( mDynamicIntervalY )
    mIntervalYProperty = parameters.value( u"VSPACING"_s ).value<QgsProperty>();

  mIntervalZ = parameterAsDouble( parameters, u"ZSPACING"_s, context );
  mDynamicIntervalZ = QgsProcessingParameters::isDynamic( parameters, u"ZSPACING"_s );
  if ( mDynamicIntervalZ )
    mIntervalZProperty = parameters.value( u"ZSPACING"_s ).value<QgsProperty>();

  mIntervalM = parameterAsDouble( parameters, u"MSPACING"_s, context );
  mDynamicIntervalM = QgsProcessingParameters::isDynamic( parameters, u"MSPACING"_s );
  if ( mDynamicIntervalM )
    mIntervalMProperty = parameters.value( u"MSPACING"_s ).value<QgsProperty>();

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

Qgis::ProcessingFeatureSourceFlags QgsSnapToGridAlgorithm::sourceFlags() const
{
  return Qgis::ProcessingFeatureSourceFlag::SkipGeometryValidityChecks;
}

///@endcond
