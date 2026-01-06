/***************************************************************************
                         qgsalgorithmaffinetransform.cpp
                         ---------------------
    begin                : December 2019
    copyright            : (C) 2019 by Nyall Dawson
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

#include "qgsalgorithmaffinetransform.h"

#include "qgsvectorlayer.h"

///@cond PRIVATE

QString QgsAffineTransformationAlgorithm::name() const
{
  return u"affinetransform"_s;
}

QString QgsAffineTransformationAlgorithm::displayName() const
{
  return QObject::tr( "Affine transform" );
}

QStringList QgsAffineTransformationAlgorithm::tags() const
{
  return QObject::tr( "move,shift,transform,affine,scale,rotate,resize,matrix" ).split( ',' );
}

QString QgsAffineTransformationAlgorithm::group() const
{
  return QObject::tr( "Vector geometry" );
}

QString QgsAffineTransformationAlgorithm::groupId() const
{
  return u"vectorgeometry"_s;
}

QString QgsAffineTransformationAlgorithm::outputName() const
{
  return QObject::tr( "Transformed" );
}

QString QgsAffineTransformationAlgorithm::shortHelpString() const
{
  return QObject::tr( "This algorithm applies an affine transformation to the geometries from a layer. Affine transformations can include "
                      "translation, scaling and rotation. The operations are performed in a scale, rotation, translation order." )
         + u"\n\n"_s
         + QObject::tr( "Z and M values present in the geometry can also be translated and scaled independently." );
}

QString QgsAffineTransformationAlgorithm::shortDescription() const
{
  return QObject::tr( "Applies an affine transformation to geometries." );
}

QgsAffineTransformationAlgorithm *QgsAffineTransformationAlgorithm::createInstance() const
{
  return new QgsAffineTransformationAlgorithm();
}

void QgsAffineTransformationAlgorithm::initParameters( const QVariantMap & )
{
  auto xOffset = std::make_unique<QgsProcessingParameterDistance>( u"DELTA_X"_s, QObject::tr( "Translation (x-axis)" ), 0.0, u"INPUT"_s );
  xOffset->setIsDynamic( true );
  xOffset->setDynamicPropertyDefinition( QgsPropertyDefinition( u"DELTA_X"_s, QObject::tr( "Offset distance (x-axis)" ), QgsPropertyDefinition::Double ) );
  xOffset->setDynamicLayerParameterName( u"INPUT"_s );
  addParameter( xOffset.release() );

  auto yOffset = std::make_unique<QgsProcessingParameterDistance>( u"DELTA_Y"_s, QObject::tr( "Translation (y-axis)" ), 0.0, u"INPUT"_s );
  yOffset->setIsDynamic( true );
  yOffset->setDynamicPropertyDefinition( QgsPropertyDefinition( u"DELTA_Y"_s, QObject::tr( "Offset distance (y-axis)" ), QgsPropertyDefinition::Double ) );
  yOffset->setDynamicLayerParameterName( u"INPUT"_s );
  addParameter( yOffset.release() );

  auto zOffset = std::make_unique<QgsProcessingParameterNumber>( u"DELTA_Z"_s, QObject::tr( "Translation (z-axis)" ), Qgis::ProcessingNumberParameterType::Double, 0.0 );
  zOffset->setIsDynamic( true );
  zOffset->setDynamicPropertyDefinition( QgsPropertyDefinition( u"DELTA_Z"_s, QObject::tr( "Offset distance (z-axis)" ), QgsPropertyDefinition::Double ) );
  zOffset->setDynamicLayerParameterName( u"INPUT"_s );
  addParameter( zOffset.release() );

  auto mOffset = std::make_unique<QgsProcessingParameterNumber>( u"DELTA_M"_s, QObject::tr( "Translation (m values)" ), Qgis::ProcessingNumberParameterType::Double, 0.0 );
  mOffset->setIsDynamic( true );
  mOffset->setDynamicPropertyDefinition( QgsPropertyDefinition( u"DELTA_M"_s, QObject::tr( "Offset distance (m values)" ), QgsPropertyDefinition::Double ) );
  mOffset->setDynamicLayerParameterName( u"INPUT"_s );
  addParameter( mOffset.release() );

  auto xScale = std::make_unique<QgsProcessingParameterNumber>( u"SCALE_X"_s, QObject::tr( "Scale factor (x-axis)" ), Qgis::ProcessingNumberParameterType::Double, 1.0 );
  xScale->setIsDynamic( true );
  xScale->setDynamicPropertyDefinition( QgsPropertyDefinition( u"SCALE_X"_s, QObject::tr( "Scale factor (x-axis)" ), QgsPropertyDefinition::Double ) );
  xScale->setDynamicLayerParameterName( u"INPUT"_s );
  addParameter( xScale.release() );

  auto yScale = std::make_unique<QgsProcessingParameterNumber>( u"SCALE_Y"_s, QObject::tr( "Scale factor (y-axis)" ), Qgis::ProcessingNumberParameterType::Double, 1.0 );
  yScale->setIsDynamic( true );
  yScale->setDynamicPropertyDefinition( QgsPropertyDefinition( u"SCALE_Y"_s, QObject::tr( "Scale factor (y-axis)" ), QgsPropertyDefinition::Double ) );
  yScale->setDynamicLayerParameterName( u"INPUT"_s );
  addParameter( yScale.release() );

  auto zScale = std::make_unique<QgsProcessingParameterNumber>( u"SCALE_Z"_s, QObject::tr( "Scale factor (z-axis)" ), Qgis::ProcessingNumberParameterType::Double, 1.0 );
  zScale->setIsDynamic( true );
  zScale->setDynamicPropertyDefinition( QgsPropertyDefinition( u"SCALE_Z"_s, QObject::tr( "Scale factor (z-axis)" ), QgsPropertyDefinition::Double ) );
  zScale->setDynamicLayerParameterName( u"INPUT"_s );
  addParameter( zScale.release() );

  auto mScale = std::make_unique<QgsProcessingParameterNumber>( u"SCALE_M"_s, QObject::tr( "Scale factor (m values)" ), Qgis::ProcessingNumberParameterType::Double, 1.0 );
  mScale->setIsDynamic( true );
  mScale->setDynamicPropertyDefinition( QgsPropertyDefinition( u"SCALE_M"_s, QObject::tr( "Scale factor (m values)" ), QgsPropertyDefinition::Double ) );
  mScale->setDynamicLayerParameterName( u"INPUT"_s );
  addParameter( mScale.release() );

  auto rotation = std::make_unique<QgsProcessingParameterNumber>( u"ROTATION_Z"_s, QObject::tr( "Rotation around z-axis (degrees counter-clockwise)" ), Qgis::ProcessingNumberParameterType::Double, 0.0 );
  rotation->setIsDynamic( true );
  rotation->setDynamicPropertyDefinition( QgsPropertyDefinition( u"ROTATION_Z"_s, QObject::tr( "Rotation around z-axis (degrees counter-clockwise)" ), QgsPropertyDefinition::Double ) );
  rotation->setDynamicLayerParameterName( u"INPUT"_s );
  addParameter( rotation.release() );
}

bool QgsAffineTransformationAlgorithm::prepareAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback * )
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

  mScaleX = parameterAsDouble( parameters, u"SCALE_X"_s, context );
  mDynamicScaleX = QgsProcessingParameters::isDynamic( parameters, u"SCALE_X"_s );
  if ( mDynamicScaleX )
    mScaleXProperty = parameters.value( u"SCALE_X"_s ).value<QgsProperty>();

  mScaleY = parameterAsDouble( parameters, u"SCALE_Y"_s, context );
  mDynamicScaleY = QgsProcessingParameters::isDynamic( parameters, u"SCALE_Y"_s );
  if ( mDynamicScaleY )
    mScaleYProperty = parameters.value( u"SCALE_Y"_s ).value<QgsProperty>();

  mScaleZ = parameterAsDouble( parameters, u"SCALE_Z"_s, context );
  mDynamicScaleZ = QgsProcessingParameters::isDynamic( parameters, u"SCALE_Z"_s );
  if ( mDynamicScaleZ )
    mScaleZProperty = parameters.value( u"SCALE_Z"_s ).value<QgsProperty>();

  mScaleM = parameterAsDouble( parameters, u"SCALE_M"_s, context );
  mDynamicScaleM = QgsProcessingParameters::isDynamic( parameters, u"SCALE_M"_s );
  if ( mDynamicScaleM )
    mScaleMProperty = parameters.value( u"SCALE_M"_s ).value<QgsProperty>();

  mRotationZ = parameterAsDouble( parameters, u"ROTATION_Z"_s, context );
  mDynamicRotationZ = QgsProcessingParameters::isDynamic( parameters, u"ROTATION_Z"_s );
  if ( mDynamicRotationZ )
    mRotationZProperty = parameters.value( u"ROTATION_Z"_s ).value<QgsProperty>();

  return true;
}

QgsFeatureList QgsAffineTransformationAlgorithm::processFeature( const QgsFeature &feature, QgsProcessingContext &context, QgsProcessingFeedback * )
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

    if ( deltaZ != 0.0 && !geometry.constGet()->is3D() )
      geometry.get()->addZValue( 0 );
    if ( deltaM != 0.0 && !geometry.constGet()->isMeasure() )
      geometry.get()->addMValue( 0 );

    double scaleX = mScaleX;
    if ( mDynamicScaleX )
      scaleX = mScaleXProperty.valueAsDouble( context.expressionContext(), scaleX );
    double scaleY = mScaleY;
    if ( mDynamicScaleY )
      scaleY = mScaleYProperty.valueAsDouble( context.expressionContext(), scaleY );
    double scaleZ = mScaleZ;
    if ( mDynamicScaleZ )
      scaleZ = mScaleZProperty.valueAsDouble( context.expressionContext(), scaleZ );
    double scaleM = mScaleM;
    if ( mDynamicScaleM )
      scaleM = mScaleMProperty.valueAsDouble( context.expressionContext(), scaleM );

    double rotationZ = mRotationZ;
    if ( mDynamicRotationZ )
      rotationZ = mRotationZProperty.valueAsDouble( context.expressionContext(), rotationZ );

    QTransform transform;
    transform.translate( deltaX, deltaY );
    transform.rotate( rotationZ );
    transform.scale( scaleX, scaleY );

    geometry.transform( transform, deltaZ, scaleZ, deltaM, scaleM );
    f.setGeometry( geometry );
  }
  return QgsFeatureList() << f;
}

Qgis::WkbType QgsAffineTransformationAlgorithm::outputWkbType( Qgis::WkbType inputWkbType ) const
{
  Qgis::WkbType wkb = inputWkbType;
  if ( mDeltaZ != 0.0 )
    wkb = QgsWkbTypes::addZ( wkb );
  if ( mDeltaM != 0.0 )
    wkb = QgsWkbTypes::addM( wkb );
  return wkb;
}


bool QgsAffineTransformationAlgorithm::supportInPlaceEdit( const QgsMapLayer *l ) const
{
  const QgsVectorLayer *layer = qobject_cast<const QgsVectorLayer *>( l );
  if ( !layer )
    return false;

  if ( !QgsProcessingFeatureBasedAlgorithm::supportInPlaceEdit( layer ) )
    return false;

  // If the type differs there is no sense in executing the algorithm and drop the result
  const Qgis::WkbType inPlaceWkbType = layer->wkbType();
  return inPlaceWkbType == outputWkbType( inPlaceWkbType );
}
///@endcond
