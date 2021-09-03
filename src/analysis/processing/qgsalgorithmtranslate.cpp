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
  return QStringLiteral( "translategeometry" );
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
  return QStringLiteral( "vectorgeometry" );
}

QString QgsTranslateAlgorithm::outputName() const
{
  return QObject::tr( "Translated" );
}

QString QgsTranslateAlgorithm::shortHelpString() const
{
  return QObject::tr( "This algorithm moves the geometries within a layer, by offsetting them with a specified x and y displacement." )
         + QStringLiteral( "\n\n" )
         + QObject::tr( "Z and M values present in the geometry can also be translated." );
}

QgsTranslateAlgorithm *QgsTranslateAlgorithm::createInstance() const
{
  return new QgsTranslateAlgorithm();
}

void QgsTranslateAlgorithm::initParameters( const QVariantMap & )
{
  std::unique_ptr< QgsProcessingParameterDistance > xOffset = std::make_unique< QgsProcessingParameterDistance >( QStringLiteral( "DELTA_X" ),
      QObject::tr( "Offset distance (x-axis)" ),
      0.0, QStringLiteral( "INPUT" ) );
  xOffset->setIsDynamic( true );
  xOffset->setDynamicPropertyDefinition( QgsPropertyDefinition( QStringLiteral( "DELTA_X" ), QObject::tr( "Offset distance (x-axis)" ), QgsPropertyDefinition::Double ) );
  xOffset->setDynamicLayerParameterName( QStringLiteral( "INPUT" ) );
  addParameter( xOffset.release() );

  std::unique_ptr< QgsProcessingParameterDistance > yOffset = std::make_unique< QgsProcessingParameterDistance >( QStringLiteral( "DELTA_Y" ),
      QObject::tr( "Offset distance (y-axis)" ),
      0.0, QStringLiteral( "INPUT" ) );
  yOffset->setIsDynamic( true );
  yOffset->setDynamicPropertyDefinition( QgsPropertyDefinition( QStringLiteral( "DELTA_Y" ), QObject::tr( "Offset distance (y-axis)" ), QgsPropertyDefinition::Double ) );
  yOffset->setDynamicLayerParameterName( QStringLiteral( "INPUT" ) );
  addParameter( yOffset.release() );

  std::unique_ptr< QgsProcessingParameterNumber > zOffset = std::make_unique< QgsProcessingParameterNumber >( QStringLiteral( "DELTA_Z" ),
      QObject::tr( "Offset distance (z-axis)" ), QgsProcessingParameterNumber::Double,
      0.0 );
  zOffset->setIsDynamic( true );
  zOffset->setDynamicPropertyDefinition( QgsPropertyDefinition( QStringLiteral( "DELTA_Z" ), QObject::tr( "Offset distance (z-axis)" ), QgsPropertyDefinition::Double ) );
  zOffset->setDynamicLayerParameterName( QStringLiteral( "INPUT" ) );
  addParameter( zOffset.release() );

  std::unique_ptr< QgsProcessingParameterNumber > mOffset = std::make_unique< QgsProcessingParameterNumber >( QStringLiteral( "DELTA_M" ),
      QObject::tr( "Offset distance (m values)" ), QgsProcessingParameterNumber::Double,
      0.0 );
  mOffset->setIsDynamic( true );
  mOffset->setDynamicPropertyDefinition( QgsPropertyDefinition( QStringLiteral( "DELTA_M" ), QObject::tr( "Offset distance (m values)" ), QgsPropertyDefinition::Double ) );
  mOffset->setDynamicLayerParameterName( QStringLiteral( "INPUT" ) );
  addParameter( mOffset.release() );
}

bool QgsTranslateAlgorithm::prepareAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback * )
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

    if ( deltaZ != 0.0 && !geometry.constGet()->is3D() )
      geometry.get()->addZValue( 0 );
    if ( deltaM != 0.0 && !geometry.constGet()->isMeasure() )
      geometry.get()->addMValue( 0 );

    geometry.translate( deltaX, deltaY, deltaZ, deltaM );
    f.setGeometry( geometry );
  }
  return QgsFeatureList() << f;
}

QgsWkbTypes::Type QgsTranslateAlgorithm::outputWkbType( QgsWkbTypes::Type inputWkbType ) const
{
  QgsWkbTypes::Type wkb = inputWkbType;
  if ( mDeltaZ != 0.0 )
    wkb = QgsWkbTypes::addZ( wkb );
  if ( mDeltaM != 0.0 )
    wkb = QgsWkbTypes::addM( wkb );
  return wkb;
}


bool QgsTranslateAlgorithm::supportInPlaceEdit( const QgsMapLayer *l ) const
{
  const QgsVectorLayer *layer = qobject_cast< const QgsVectorLayer * >( l );
  if ( !layer )
    return false;

  if ( ! QgsProcessingFeatureBasedAlgorithm::supportInPlaceEdit( layer ) )
    return false;

  // Check if we can drop Z/M and still have some work done
  if ( mDeltaX != 0.0 || mDeltaY != 0.0 )
    return true;

  // If the type differs there is no sense in executing the algorithm and drop the result
  const QgsWkbTypes::Type inPlaceWkbType = layer->wkbType();
  return inPlaceWkbType == outputWkbType( inPlaceWkbType );
}
///@endcond


