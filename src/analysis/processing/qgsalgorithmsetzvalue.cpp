/***************************************************************************
                         qgsalgorithmsetzvalue.cpp
                         ---------------------
    begin                : November 2019
    copyright            : (C) 2019 by Alexander Bruy
    email                : alexander dot bruy at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsalgorithmsetzvalue.h"
#include "qgsvectorlayer.h"

///@cond PRIVATE

QString QgsSetZValueAlgorithm::name() const
{
  return QStringLiteral( "setzvalue" );
}

QString QgsSetZValueAlgorithm::displayName() const
{
  return QObject::tr( "Set Z value" );
}

QStringList QgsSetZValueAlgorithm::tags() const
{
  return QObject::tr( "set,add,z,25d,3d,values" ).split( ',' );
}

QString QgsSetZValueAlgorithm::group() const
{
  return QObject::tr( "Vector geometry" );
}

QString QgsSetZValueAlgorithm::groupId() const
{
  return QStringLiteral( "vectorgeometry" );
}

QString QgsSetZValueAlgorithm::shortHelpString() const
{
  return QObject::tr( "This algorithm sets the Z value for geometries in a layer.\n\n"
                      "If Z values already exist in the layer, they will be overwritten "
                      "with the new value. If no Z values exist, the geometry will be "
                      "upgraded to include Z values and the specified value used as "
                      "the initial Z value for all geometries." );
}

QString QgsSetZValueAlgorithm::outputName() const
{
  return QObject::tr( "Z Added" );
}

QgsSetZValueAlgorithm *QgsSetZValueAlgorithm::createInstance() const
{
  return new QgsSetZValueAlgorithm();
}

bool QgsSetZValueAlgorithm::supportInPlaceEdit( const QgsMapLayer *l ) const
{
  const QgsVectorLayer *layer = qobject_cast<const QgsVectorLayer *>( l );
  if ( !layer )
    return false;

  return QgsProcessingFeatureBasedAlgorithm::supportInPlaceEdit( l ) && QgsWkbTypes::hasZ( layer->wkbType() );
}

Qgis::ProcessingFeatureSourceFlags QgsSetZValueAlgorithm::sourceFlags() const
{
  return Qgis::ProcessingFeatureSourceFlag::SkipGeometryValidityChecks;
}

Qgis::WkbType QgsSetZValueAlgorithm::outputWkbType( Qgis::WkbType type ) const
{
  return QgsWkbTypes::addZ( type );
}

void QgsSetZValueAlgorithm::initParameters( const QVariantMap & )
{
  auto zValueParam = std::make_unique<QgsProcessingParameterNumber>( QStringLiteral( "Z_VALUE" ), QObject::tr( "Z Value" ), Qgis::ProcessingNumberParameterType::Double, 0.0 );
  zValueParam->setIsDynamic( true );
  zValueParam->setDynamicPropertyDefinition( QgsPropertyDefinition( QStringLiteral( "Z_VALUE" ), QObject::tr( "Z Value" ), QgsPropertyDefinition::Double ) );
  zValueParam->setDynamicLayerParameterName( QStringLiteral( "INPUT" ) );
  addParameter( zValueParam.release() );
}

bool QgsSetZValueAlgorithm::prepareAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback * )
{
  mZValue = parameterAsDouble( parameters, QStringLiteral( "Z_VALUE" ), context );
  mDynamicZValue = QgsProcessingParameters::isDynamic( parameters, QStringLiteral( "Z_VALUE" ) );
  if ( mDynamicZValue )
    mZValueProperty = parameters.value( QStringLiteral( "Z_VALUE" ) ).value<QgsProperty>();

  return true;
}

QgsFeatureList QgsSetZValueAlgorithm::processFeature( const QgsFeature &feature, QgsProcessingContext &context, QgsProcessingFeedback * )
{
  QgsFeature f = feature;

  if ( f.hasGeometry() )
  {
    std::unique_ptr<QgsAbstractGeometry> newGeometry( f.geometry().constGet()->clone() );
    // addZValue won't alter existing Z values, so drop them first
    if ( QgsWkbTypes::hasZ( newGeometry->wkbType() ) )
      newGeometry->dropZValue();

    double z = mZValue;
    if ( mDynamicZValue )
      z = mZValueProperty.valueAsDouble( context.expressionContext(), z );

    newGeometry->addZValue( z );

    f.setGeometry( QgsGeometry( std::move( newGeometry ) ) );
  }

  return QgsFeatureList() << f;
}

///@endcond
