/***************************************************************************
                         qgsalgorithmfiltervertices.cpp
                         ------------------------------
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

#include "qgsalgorithmfiltervertices.h"

#include "qgsvectorlayer.h"

///@cond PRIVATE


QString QgsFilterVerticesAlgorithmBase::group() const
{
  return QObject::tr( "Vector geometry" );
}

QString QgsFilterVerticesAlgorithmBase::groupId() const
{
  return u"vectorgeometry"_s;
}

QString QgsFilterVerticesAlgorithmBase::outputName() const
{
  return QObject::tr( "Filtered" );
}

QString QgsFilterVerticesAlgorithmBase::shortHelpString() const
{
  return QObject::tr( "Filters away vertices based on their %1, returning geometries with only "
                      "vertex points that have a %1 ≥ the specified minimum value and ≤ "
                      "the maximum value.\n\n"
                      "If the minimum value is not specified then only the maximum value is tested, "
                      "and similarly if the maximum value is not specified then only the minimum value is tested.\n\n"
                      "Depending on the input geometry attributes and the filters used, "
                      "the resultant geometries created by this algorithm may no longer be valid." )
    .arg( componentString() );
}

QString QgsFilterVerticesAlgorithmBase::shortDescription() const
{
  return QObject::tr( "Filters away vertices based on their %1 value." )
    .arg( componentString() );
}

void QgsFilterVerticesAlgorithmBase::initParameters( const QVariantMap & )
{
  auto min = std::make_unique<QgsProcessingParameterNumber>( u"MIN"_s, QObject::tr( "Minimum" ), Qgis::ProcessingNumberParameterType::Double, QVariant(), true );
  min->setIsDynamic( true );
  min->setDynamicPropertyDefinition( QgsPropertyDefinition( u"Minimum"_s, QObject::tr( "Minimum value" ), QgsPropertyDefinition::Double ) );
  min->setDynamicLayerParameterName( u"INPUT"_s );
  addParameter( min.release() );

  auto max = std::make_unique<QgsProcessingParameterNumber>( u"MAX"_s, QObject::tr( "Maximum" ), Qgis::ProcessingNumberParameterType::Double, QVariant(), true );
  max->setIsDynamic( true );
  max->setDynamicPropertyDefinition( QgsPropertyDefinition( u"Maximum"_s, QObject::tr( "Maximum value" ), QgsPropertyDefinition::Double ) );
  max->setDynamicLayerParameterName( u"INPUT"_s );
  addParameter( max.release() );
}

bool QgsFilterVerticesAlgorithmBase::prepareAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback * )
{
  if ( parameters.contains( u"MIN"_s ) && parameters.value( u"MIN"_s ).isValid() )
    mMin = parameterAsDouble( parameters, u"MIN"_s, context );
  else
    mMin = std::numeric_limits<double>::quiet_NaN();

  mDynamicMin = QgsProcessingParameters::isDynamic( parameters, u"MIN"_s );
  if ( mDynamicMin )
    mMinProperty = parameters.value( u"MIN"_s ).value<QgsProperty>();

  if ( parameters.contains( u"MAX"_s ) && parameters.value( u"MAX"_s ).isValid() )
    mMax = parameterAsDouble( parameters, u"MAX"_s, context );
  else
    mMax = std::numeric_limits<double>::quiet_NaN();

  mDynamicMax = QgsProcessingParameters::isDynamic( parameters, u"MAX"_s );
  if ( mDynamicMax )
    mMaxProperty = parameters.value( u"MAX"_s ).value<QgsProperty>();

  return true;
}

QgsFeatureList QgsFilterVerticesAlgorithmBase::processFeature( const QgsFeature &feature, QgsProcessingContext &context, QgsProcessingFeedback * )
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

QString QgsFilterVerticesByM::name() const
{
  return u"filterverticesbym"_s;
}

QString QgsFilterVerticesByM::displayName() const
{
  return QObject::tr( "Filter vertices by M value" );
}

QStringList QgsFilterVerticesByM::tags() const
{
  return QObject::tr( "filter,points,vertex,m" ).split( ',' );
}

QgsFilterVerticesByM *QgsFilterVerticesByM::createInstance() const
{
  return new QgsFilterVerticesByM();
}

bool QgsFilterVerticesByM::supportInPlaceEdit( const QgsMapLayer *l ) const
{
  const QgsVectorLayer *layer = qobject_cast<const QgsVectorLayer *>( l );
  if ( !layer )
    return false;

  if ( !QgsFilterVerticesAlgorithmBase::supportInPlaceEdit( layer ) )
    return false;
  return QgsWkbTypes::hasM( layer->wkbType() );
}

QString QgsFilterVerticesByM::componentString() const
{
  return QObject::tr( "m-value" );
}

void QgsFilterVerticesByM::filter( QgsGeometry &geometry, double min, double max ) const
{
  geometry.filterVertices( [min, max]( const QgsPoint &point ) -> bool {
    return ( std::isnan( min ) || point.m() >= min )
           && ( std::isnan( max ) || point.m() <= max );
  } );
}


//
// QgsFilterPointsByZ
//

QString QgsFilterVerticesByZ::name() const
{
  return u"filterverticesbyz"_s;
}

QString QgsFilterVerticesByZ::displayName() const
{
  return QObject::tr( "Filter vertices by Z value" );
}

QStringList QgsFilterVerticesByZ::tags() const
{
  return QObject::tr( "filter,points,vertex,z" ).split( ',' );
}

QgsFilterVerticesByZ *QgsFilterVerticesByZ::createInstance() const
{
  return new QgsFilterVerticesByZ();
}

bool QgsFilterVerticesByZ::supportInPlaceEdit( const QgsMapLayer *l ) const
{
  const QgsVectorLayer *layer = qobject_cast<const QgsVectorLayer *>( l );
  if ( !layer )
    return false;

  if ( !QgsFilterVerticesAlgorithmBase::supportInPlaceEdit( layer ) )
    return false;
  return QgsWkbTypes::hasZ( layer->wkbType() );
}

QString QgsFilterVerticesByZ::componentString() const
{
  return QObject::tr( "z-value" );
}

void QgsFilterVerticesByZ::filter( QgsGeometry &geometry, double min, double max ) const
{
  geometry.filterVertices( [min, max]( const QgsPoint &point ) -> bool {
    return ( std::isnan( min ) || point.z() >= min )
           && ( std::isnan( max ) || point.z() <= max );
  } );
}

///@endcond
