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
  return QStringLiteral( "affinetransform" );
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
  return QStringLiteral( "vectorgeometry" );
}

QString QgsAffineTransformationAlgorithm::outputName() const
{
  return QObject::tr( "Transformed" );
}

QString QgsAffineTransformationAlgorithm::shortHelpString() const
{
  return QObject::tr( "Applies an affine transformation to the geometries from a layer. Affine transformations can include "
                      "translation, scaling and rotation. The operations are performed in a scale, rotation, translation order." )
         + QStringLiteral( "\n\n" )
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
  std::unique_ptr< QgsProcessingParameterDistance > xOffset = std::make_unique< QgsProcessingParameterDistance >( QStringLiteral( "DELTA_X" ),
      QObject::tr( "Translation (x-axis)" ),
      0.0, QStringLiteral( "INPUT" ) );
  xOffset->setIsDynamic( true );
  xOffset->setDynamicPropertyDefinition( QgsPropertyDefinition( QStringLiteral( "DELTA_X" ), QObject::tr( "Offset distance (x-axis)" ), QgsPropertyDefinition::Double ) );
  xOffset->setDynamicLayerParameterName( QStringLiteral( "INPUT" ) );
  addParameter( xOffset.release() );

  std::unique_ptr< QgsProcessingParameterDistance > yOffset = std::make_unique< QgsProcessingParameterDistance >( QStringLiteral( "DELTA_Y" ),
      QObject::tr( "Translation (y-axis)" ),
      0.0, QStringLiteral( "INPUT" ) );
  yOffset->setIsDynamic( true );
  yOffset->setDynamicPropertyDefinition( QgsPropertyDefinition( QStringLiteral( "DELTA_Y" ), QObject::tr( "Offset distance (y-axis)" ), QgsPropertyDefinition::Double ) );
  yOffset->setDynamicLayerParameterName( QStringLiteral( "INPUT" ) );
  addParameter( yOffset.release() );

  std::unique_ptr< QgsProcessingParameterNumber > zOffset = std::make_unique< QgsProcessingParameterNumber >( QStringLiteral( "DELTA_Z" ),
      QObject::tr( "Translation (z-axis)" ), QgsProcessingParameterNumber::Double,
      0.0 );
  zOffset->setIsDynamic( true );
  zOffset->setDynamicPropertyDefinition( QgsPropertyDefinition( QStringLiteral( "DELTA_Z" ), QObject::tr( "Offset distance (z-axis)" ), QgsPropertyDefinition::Double ) );
  zOffset->setDynamicLayerParameterName( QStringLiteral( "INPUT" ) );
  addParameter( zOffset.release() );

  std::unique_ptr< QgsProcessingParameterNumber > mOffset = std::make_unique< QgsProcessingParameterNumber >( QStringLiteral( "DELTA_M" ),
      QObject::tr( "Translation (m values)" ), QgsProcessingParameterNumber::Double,
      0.0 );
  mOffset->setIsDynamic( true );
  mOffset->setDynamicPropertyDefinition( QgsPropertyDefinition( QStringLiteral( "DELTA_M" ), QObject::tr( "Offset distance (m values)" ), QgsPropertyDefinition::Double ) );
  mOffset->setDynamicLayerParameterName( QStringLiteral( "INPUT" ) );
  addParameter( mOffset.release() );

  std::unique_ptr< QgsProcessingParameterNumber > xScale = std::make_unique< QgsProcessingParameterNumber >( QStringLiteral( "SCALE_X" ),
      QObject::tr( "Scale factor (x-axis)" ), QgsProcessingParameterNumber::Double,
      1.0 );
  xScale->setIsDynamic( true );
  xScale->setDynamicPropertyDefinition( QgsPropertyDefinition( QStringLiteral( "SCALE_X" ), QObject::tr( "Scale factor (x-axis)" ), QgsPropertyDefinition::Double ) );
  xScale->setDynamicLayerParameterName( QStringLiteral( "INPUT" ) );
  addParameter( xScale.release() );

  std::unique_ptr< QgsProcessingParameterNumber > yScale = std::make_unique< QgsProcessingParameterNumber >( QStringLiteral( "SCALE_Y" ),
      QObject::tr( "Scale factor (y-axis)" ), QgsProcessingParameterNumber::Double,
      1.0 );
  yScale->setIsDynamic( true );
  yScale->setDynamicPropertyDefinition( QgsPropertyDefinition( QStringLiteral( "SCALE_Y" ), QObject::tr( "Scale factor (y-axis)" ), QgsPropertyDefinition::Double ) );
  yScale->setDynamicLayerParameterName( QStringLiteral( "INPUT" ) );
  addParameter( yScale.release() );

  std::unique_ptr< QgsProcessingParameterNumber > zScale = std::make_unique< QgsProcessingParameterNumber >( QStringLiteral( "SCALE_Z" ),
      QObject::tr( "Scale factor (z-axis)" ), QgsProcessingParameterNumber::Double,
      1.0 );
  zScale->setIsDynamic( true );
  zScale->setDynamicPropertyDefinition( QgsPropertyDefinition( QStringLiteral( "SCALE_Z" ), QObject::tr( "Scale factor (z-axis)" ), QgsPropertyDefinition::Double ) );
  zScale->setDynamicLayerParameterName( QStringLiteral( "INPUT" ) );
  addParameter( zScale.release() );

  std::unique_ptr< QgsProcessingParameterNumber > mScale = std::make_unique< QgsProcessingParameterNumber >( QStringLiteral( "SCALE_M" ),
      QObject::tr( "Scale factor (m values)" ), QgsProcessingParameterNumber::Double,
      1.0 );
  mScale->setIsDynamic( true );
  mScale->setDynamicPropertyDefinition( QgsPropertyDefinition( QStringLiteral( "SCALE_M" ), QObject::tr( "Scale factor (m values)" ), QgsPropertyDefinition::Double ) );
  mScale->setDynamicLayerParameterName( QStringLiteral( "INPUT" ) );
  addParameter( mScale.release() );

  std::unique_ptr< QgsProcessingParameterNumber > rotation = std::make_unique< QgsProcessingParameterNumber >( QStringLiteral( "ROTATION_Z" ),
      QObject::tr( "Rotation around z-axis (degrees counter-clockwise)" ), QgsProcessingParameterNumber::Double,
      0.0 );
  rotation->setIsDynamic( true );
  rotation->setDynamicPropertyDefinition( QgsPropertyDefinition( QStringLiteral( "ROTATION_Z" ), QObject::tr( "Rotation around z-axis (degrees counter-clockwise)" ), QgsPropertyDefinition::Double ) );
  rotation->setDynamicLayerParameterName( QStringLiteral( "INPUT" ) );
  addParameter( rotation.release() );
}

bool QgsAffineTransformationAlgorithm::prepareAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback * )
{
  mDeltaX = parameterAsDouble( parameters, QStringLiteral( "DELTA_X" ), context );
  mDynamicDeltaX = QgsProcessingParameters::isDynamic( parameters, QStringLiteral( "DELTA_X" ) );
  if ( mDynamicDeltaX )
    mDeltaXProperty = parameters.value( QStringLiteral( "DELTA_X" ) ).value< QgsProperty >();

  mDeltaY = parameterAsDouble( parameters, QStringLiteral( "DELTA_Y" ), context );
  mDynamicDeltaY = QgsProcessingParameters::isDynamic( parameters, QStringLiteral( "DELTA_Y" ) );
  if ( mDynamicDeltaY )
    mDeltaYProperty = parameters.value( QStringLiteral( "DELTA_Y" ) ).value< QgsProperty >();

  mDeltaZ = parameterAsDouble( parameters, QStringLiteral( "DELTA_Z" ), context );
  mDynamicDeltaZ = QgsProcessingParameters::isDynamic( parameters, QStringLiteral( "DELTA_Z" ) );
  if ( mDynamicDeltaZ )
    mDeltaZProperty = parameters.value( QStringLiteral( "DELTA_Z" ) ).value< QgsProperty >();

  mDeltaM = parameterAsDouble( parameters, QStringLiteral( "DELTA_M" ), context );
  mDynamicDeltaM = QgsProcessingParameters::isDynamic( parameters, QStringLiteral( "DELTA_M" ) );
  if ( mDynamicDeltaM )
    mDeltaMProperty = parameters.value( QStringLiteral( "DELTA_M" ) ).value< QgsProperty >();

  mScaleX = parameterAsDouble( parameters, QStringLiteral( "SCALE_X" ), context );
  mDynamicScaleX = QgsProcessingParameters::isDynamic( parameters, QStringLiteral( "SCALE_X" ) );
  if ( mDynamicScaleX )
    mScaleXProperty = parameters.value( QStringLiteral( "SCALE_X" ) ).value< QgsProperty >();

  mScaleY = parameterAsDouble( parameters, QStringLiteral( "SCALE_Y" ), context );
  mDynamicScaleY = QgsProcessingParameters::isDynamic( parameters, QStringLiteral( "SCALE_Y" ) );
  if ( mDynamicScaleY )
    mScaleYProperty = parameters.value( QStringLiteral( "SCALE_Y" ) ).value< QgsProperty >();

  mScaleZ = parameterAsDouble( parameters, QStringLiteral( "SCALE_Z" ), context );
  mDynamicScaleZ = QgsProcessingParameters::isDynamic( parameters, QStringLiteral( "SCALE_Z" ) );
  if ( mDynamicScaleZ )
    mScaleZProperty = parameters.value( QStringLiteral( "SCALE_Z" ) ).value< QgsProperty >();

  mScaleM = parameterAsDouble( parameters, QStringLiteral( "SCALE_M" ), context );
  mDynamicScaleM = QgsProcessingParameters::isDynamic( parameters, QStringLiteral( "SCALE_M" ) );
  if ( mDynamicScaleM )
    mScaleMProperty = parameters.value( QStringLiteral( "SCALE_M" ) ).value< QgsProperty >();

  mRotationZ = parameterAsDouble( parameters, QStringLiteral( "ROTATION_Z" ), context );
  mDynamicRotationZ = QgsProcessingParameters::isDynamic( parameters, QStringLiteral( "ROTATION_Z" ) );
  if ( mDynamicRotationZ )
    mRotationZProperty = parameters.value( QStringLiteral( "ROTATION_Z" ) ).value< QgsProperty >();

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

QgsWkbTypes::Type QgsAffineTransformationAlgorithm::outputWkbType( QgsWkbTypes::Type inputWkbType ) const
{
  QgsWkbTypes::Type wkb = inputWkbType;
  if ( mDeltaZ != 0.0 )
    wkb = QgsWkbTypes::addZ( wkb );
  if ( mDeltaM != 0.0 )
    wkb = QgsWkbTypes::addM( wkb );
  return wkb;
}


bool QgsAffineTransformationAlgorithm::supportInPlaceEdit( const QgsMapLayer *l ) const
{
  const QgsVectorLayer *layer = qobject_cast< const QgsVectorLayer * >( l );
  if ( !layer )
    return false;

  if ( ! QgsProcessingFeatureBasedAlgorithm::supportInPlaceEdit( layer ) )
    return false;

  // If the type differs there is no sense in executing the algorithm and drop the result
  const QgsWkbTypes::Type inPlaceWkbType = layer->wkbType();
  return inPlaceWkbType == outputWkbType( inPlaceWkbType );
}
///@endcond


