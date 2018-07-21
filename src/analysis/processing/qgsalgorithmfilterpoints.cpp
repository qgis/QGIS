/***************************************************************************
                         qgsalgorithmfilterpoints.cpp
                         ----------------------------
    begin                : July 2018
    copyright            : (C) 2018 by Nyall Dawson
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

#include "qgsalgorithmfilterpoints.h"

///@cond PRIVATE


QString QgsFilterPointsAlgorithmBase::group() const
{
  return QObject::tr( "Vector geometry" );
}

QString QgsFilterPointsAlgorithmBase::groupId() const
{
  return QStringLiteral( "vectorgeometry" );
}

QString QgsFilterPointsAlgorithmBase::outputName() const
{
  return QObject::tr( "Filtered" );
}

QString QgsFilterPointsAlgorithmBase::shortHelpString() const
{
  return QObject::tr( "Filters away vertices based on their %1, returning geometries with only "
                      "vertex points that have a %1 ≥ the specified minimum value and ≤ "
                      "the maximum value.\n\n"
                      "If the minimum value is not specified than only the maximum value is tested, "
                      "and similarly if the maximum value is not specified than only the minimum value is tested.\n\n"
                      "The resultant geometries created by this algorithm may not be valid "
                      "and may need to be run through the \"Repair geometries\" algorithm to ensure "
                      "their validity." ).arg( componentString() );
}

QList<int> QgsFilterPointsAlgorithmBase::inputLayerTypes() const
{
  return QList<int>() << QgsProcessing::TypeVectorLine << QgsProcessing::TypeVectorPolygon;
}

void QgsFilterPointsAlgorithmBase::initParameters( const QVariantMap & )
{
  auto min = qgis::make_unique< QgsProcessingParameterNumber >( QStringLiteral( "MIN" ),
             QObject::tr( "Minimum" ), QgsProcessingParameterNumber::Double, QVariant(), true );
  min->setIsDynamic( true );
  min->setDynamicPropertyDefinition( QgsPropertyDefinition( QStringLiteral( "Minimum" ), QObject::tr( "Minimum value" ), QgsPropertyDefinition::Double ) );
  min->setDynamicLayerParameterName( QStringLiteral( "INPUT" ) );
  addParameter( min.release() );

  auto max = qgis::make_unique< QgsProcessingParameterNumber >( QStringLiteral( "MAX" ),
             QObject::tr( "Maximum" ), QgsProcessingParameterNumber::Double, QVariant(), true );
  max->setIsDynamic( true );
  max->setDynamicPropertyDefinition( QgsPropertyDefinition( QStringLiteral( "Maximum" ), QObject::tr( "Maximum value" ), QgsPropertyDefinition::Double ) );
  max->setDynamicLayerParameterName( QStringLiteral( "INPUT" ) );
  addParameter( max.release() );
}

bool QgsFilterPointsAlgorithmBase::prepareAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback * )
{
  if ( parameters.contains( QStringLiteral( "MIN" ) ) && parameters.value( QStringLiteral( "MIN" ) ).isValid() )
    mMin = parameterAsDouble( parameters, QStringLiteral( "MIN" ), context );
  else
    mMin = std::numeric_limits<double>::quiet_NaN();

  mDynamicMin = QgsProcessingParameters::isDynamic( parameters, QStringLiteral( "MIN" ) );
  if ( mDynamicMin )
    mMinProperty = parameters.value( QStringLiteral( "MIN" ) ).value< QgsProperty >();

  if ( parameters.contains( QStringLiteral( "MAX" ) ) && parameters.value( QStringLiteral( "MAX" ) ).isValid() )
    mMax = parameterAsDouble( parameters, QStringLiteral( "MAX" ), context );
  else
    mMax = std::numeric_limits<double>::quiet_NaN();

  mDynamicMax = QgsProcessingParameters::isDynamic( parameters, QStringLiteral( "MAX" ) );
  if ( mDynamicMax )
    mMaxProperty = parameters.value( QStringLiteral( "MAX" ) ).value< QgsProperty >();

  return true;
}

QgsFeatureList QgsFilterPointsAlgorithmBase::processFeature( const QgsFeature &feature, QgsProcessingContext &context, QgsProcessingFeedback * )
{
  QgsFeature f = feature;
  if ( f.hasGeometry() )
  {
    QgsGeometry geometry = f.geometry();
    double min = mMin;
    if ( mDynamicMin )
      min = mMinProperty.valueAsDouble( context.expressionContext(), std::numeric_limits<double>::quiet_NaN() );

    double max = mMax;
    if ( mDynamicMax )
      max = mMaxProperty.valueAsDouble( context.expressionContext(), std::numeric_limits<double>::quiet_NaN() );

    filter( geometry, min, max );
    f.setGeometry( geometry );
  }
  return QgsFeatureList() << f;
}

//
// QgsFilterPointsByM
//

QString QgsFilterPointsByM::name() const
{
  return QStringLiteral( "filterpointsbym" );
}

QString QgsFilterPointsByM::displayName() const
{
  return QObject::tr( "Filter vertices by m value" );
}

QStringList QgsFilterPointsByM::tags() const
{
  return QObject::tr( "filter,points,vertex,m" ).split( ',' );
}

QgsFilterPointsByM *QgsFilterPointsByM::createInstance() const
{
  return new QgsFilterPointsByM();
}

QString QgsFilterPointsByM::componentString() const
{
  return QObject::tr( "m-value" );
}

void QgsFilterPointsByM::filter( QgsGeometry &geometry, double min, double max ) const
{
  geometry.filterVertices( [min, max]( const QgsPoint & point )->bool
  {
    return ( std::isnan( min ) || point.m() >= min )
    && ( std::isnan( max ) || point.m() <= max );
  } );
}


//
// QgsFilterPointsByZ
//

QString QgsFilterPointsByZ::name() const
{
  return QStringLiteral( "filterpointsbyz" );
}

QString QgsFilterPointsByZ::displayName() const
{
  return QObject::tr( "Filter vertices by z value" );
}

QStringList QgsFilterPointsByZ::tags() const
{
  return QObject::tr( "filter,points,vertex,z" ).split( ',' );
}

QgsFilterPointsByZ *QgsFilterPointsByZ::createInstance() const
{
  return new QgsFilterPointsByZ();
}

QString QgsFilterPointsByZ::componentString() const
{
  return QObject::tr( "z-value" );
}

void QgsFilterPointsByZ::filter( QgsGeometry &geometry, double min, double max ) const
{
  geometry.filterVertices( [min, max]( const QgsPoint & point )->bool
  {
    return ( std::isnan( min ) || point.z() >= min )
    && ( std::isnan( max ) || point.z() <= max );
  } );
}

///@endcond


