/***************************************************************************
                         qgsalgorithmtranslate.cpp
                         ---------------------
    begin                : November 2017
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

#include "qgsalgorithmtranslate.h"

#include "qgsvectorlayer.h"

///@cond PRIVATE

QString QgsTranslateAlgorithm::name() const
{
  return u"translategeometry"_s;
}

QString QgsTranslateAlgorithm::displayName() const
{
  return QObject::tr( "Translate" );
}

QStringList QgsTranslateAlgorithm::tags() const
{
  return QObject::tr( "move,shift,transform,z,m,values,add" ).split( ',' );
}

QString QgsTranslateAlgorithm::group() const
{
  return QObject::tr( "Vector geometry" );
}

QString QgsTranslateAlgorithm::groupId() const
{
  return u"vectorgeometry"_s;
}

QString QgsTranslateAlgorithm::outputName() const
{
  return QObject::tr( "Translated" );
}

QString QgsTranslateAlgorithm::shortHelpString() const
{
  return QObject::tr( "This algorithm moves the geometries within a layer, by offsetting them with a specified x and y displacement." )
         + u"\n\n"_s
         + QObject::tr( "Z and M values present in the geometry can also be translated." );
}

QString QgsTranslateAlgorithm::shortDescription() const
{
  return QObject::tr( "Moves the geometries within a layer, by offsetting them with a specified x, y, z or m displacement." );
}

QgsTranslateAlgorithm *QgsTranslateAlgorithm::createInstance() const
{
  return new QgsTranslateAlgorithm();
}

void QgsTranslateAlgorithm::initParameters( const QVariantMap & )
{
  auto xOffset = std::make_unique<QgsProcessingParameterDistance>( u"DELTA_X"_s, QObject::tr( "Offset distance (x-axis)" ), 0.0, u"INPUT"_s );
  xOffset->setIsDynamic( true );
  xOffset->setDynamicPropertyDefinition( QgsPropertyDefinition( u"DELTA_X"_s, QObject::tr( "Offset distance (x-axis)" ), QgsPropertyDefinition::Double ) );
  xOffset->setDynamicLayerParameterName( u"INPUT"_s );
  addParameter( xOffset.release() );

  auto yOffset = std::make_unique<QgsProcessingParameterDistance>( u"DELTA_Y"_s, QObject::tr( "Offset distance (y-axis)" ), 0.0, u"INPUT"_s );
  yOffset->setIsDynamic( true );
  yOffset->setDynamicPropertyDefinition( QgsPropertyDefinition( u"DELTA_Y"_s, QObject::tr( "Offset distance (y-axis)" ), QgsPropertyDefinition::Double ) );
  yOffset->setDynamicLayerParameterName( u"INPUT"_s );
  addParameter( yOffset.release() );

  auto zOffset = std::make_unique<QgsProcessingParameterNumber>( u"DELTA_Z"_s, QObject::tr( "Offset distance (z-axis)" ), Qgis::ProcessingNumberParameterType::Double, 0.0 );
  zOffset->setIsDynamic( true );
  zOffset->setDynamicPropertyDefinition( QgsPropertyDefinition( u"DELTA_Z"_s, QObject::tr( "Offset distance (z-axis)" ), QgsPropertyDefinition::Double ) );
  zOffset->setDynamicLayerParameterName( u"INPUT"_s );
  addParameter( zOffset.release() );

  auto mOffset = std::make_unique<QgsProcessingParameterNumber>( u"DELTA_M"_s, QObject::tr( "Offset distance (m values)" ), Qgis::ProcessingNumberParameterType::Double, 0.0 );
  mOffset->setIsDynamic( true );
  mOffset->setDynamicPropertyDefinition( QgsPropertyDefinition( u"DELTA_M"_s, QObject::tr( "Offset distance (m values)" ), QgsPropertyDefinition::Double ) );
  mOffset->setDynamicLayerParameterName( u"INPUT"_s );
  addParameter( mOffset.release() );
}

bool QgsTranslateAlgorithm::prepareAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback * )
{
  mDeltaX = parameterAsDouble( parameters, u"DELTA_X"_s, context );
  mDynamicDeltaX = QgsProcessingParameters::isDynamic( parameters, u"DELTA_X"_s );
  if ( mDynamicDeltaX )
    mDeltaXProperty = parameters.value( u"DELTA_X"_s ).value<QgsProperty>();

  mDeltaY = parameterAsDouble( parameters, u"DELTA_Y"_s, context );
  mDynamicDeltaY = QgsProcessingParameters::isDynamic( parameters, u"DELTA_Y"_s );
  if ( mDynamicDeltaY )
    mDeltaYProperty = parameters.value( u"DELTA_Y"_s ).value<QgsProperty>();

  mDeltaZ = parameterAsDouble( parameters, u"DELTA_Z"_s, context );
  mDynamicDeltaZ = QgsProcessingParameters::isDynamic( parameters, u"DELTA_Z"_s );
  if ( mDynamicDeltaZ )
    mDeltaZProperty = parameters.value( u"DELTA_Z"_s ).value<QgsProperty>();

  mDeltaM = parameterAsDouble( parameters, u"DELTA_M"_s, context );
  mDynamicDeltaM = QgsProcessingParameters::isDynamic( parameters, u"DELTA_M"_s );
  if ( mDynamicDeltaM )
    mDeltaMProperty = parameters.value( u"DELTA_M"_s ).value<QgsProperty>();

  return true;
}

QgsFeatureList QgsTranslateAlgorithm::processFeature( const QgsFeature &feature, QgsProcessingContext &context, QgsProcessingFeedback * )
{
  QgsFeature f = feature;
  if ( f.hasGeometry() )
  {
    QgsGeometry geometry = f.geometry();

    double deltaX = mDeltaX;
    if ( mDynamicDeltaX )
      deltaX = mDeltaXProperty.valueAsDouble( context.expressionContext(), deltaX );
    double deltaY = mDeltaY;
    if ( mDynamicDeltaY )
      deltaY = mDeltaYProperty.valueAsDouble( context.expressionContext(), deltaY );
    double deltaZ = mDeltaZ;
    if ( mDynamicDeltaZ )
      deltaZ = mDeltaZProperty.valueAsDouble( context.expressionContext(), deltaZ );
    double deltaM = mDeltaM;
    if ( mDynamicDeltaM )
      deltaM = mDeltaMProperty.valueAsDouble( context.expressionContext(), deltaM );

    if ( QgsWkbTypes::hasZ( mOutputWkbType ) && !geometry.constGet()->is3D() )
      geometry.get()->addZValue( 0 );
    if ( QgsWkbTypes::hasM( mOutputWkbType ) && !geometry.constGet()->isMeasure() )
      geometry.get()->addMValue( 0 );

    geometry.translate( deltaX, deltaY, deltaZ, deltaM );
    f.setGeometry( geometry );
  }
  return QgsFeatureList() << f;
}

Qgis::WkbType QgsTranslateAlgorithm::outputWkbType( Qgis::WkbType inputWkbType ) const
{
  mOutputWkbType = inputWkbType;
  if ( mDynamicDeltaZ || mDeltaZ != 0.0 )
    mOutputWkbType = QgsWkbTypes::addZ( mOutputWkbType );
  if ( mDynamicDeltaM || mDeltaM != 0.0 )
    mOutputWkbType = QgsWkbTypes::addM( mOutputWkbType );
  return mOutputWkbType;
}

bool QgsTranslateAlgorithm::supportInPlaceEdit( const QgsMapLayer *l ) const
{
  const QgsVectorLayer *layer = qobject_cast<const QgsVectorLayer *>( l );
  if ( !layer )
    return false;

  if ( !QgsProcessingFeatureBasedAlgorithm::supportInPlaceEdit( layer ) )
    return false;

  // Check if we can drop Z/M and still have some work done
  if ( mDeltaX != 0.0 || mDeltaY != 0.0 )
    return true;

  // If the type differs there is no sense in executing the algorithm and drop the result
  const Qgis::WkbType inPlaceWkbType = layer->wkbType();
  return inPlaceWkbType == outputWkbType( inPlaceWkbType );
}

///@endcond
