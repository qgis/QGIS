/***************************************************************************
                         qgsalgorithmsetmvalue.cpp
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

#include "qgsalgorithmsetmvalue.h"
#include "qgsvectorlayer.h"

///@cond PRIVATE

QString QgsSetMValueAlgorithm::name() const
{
  return QStringLiteral( "setmvalue" );
}

QString QgsSetMValueAlgorithm::displayName() const
{
  return QObject::tr( "Set M value" );
}

QStringList QgsSetMValueAlgorithm::tags() const
{
  return QObject::tr( "set,add,m,measure,values" ).split( ',' );
}

QString QgsSetMValueAlgorithm::group() const
{
  return QObject::tr( "Vector geometry" );
}

QString QgsSetMValueAlgorithm::groupId() const
{
  return QStringLiteral( "vectorgeometry" );
}

QString QgsSetMValueAlgorithm::shortHelpString() const
{
  return QObject::tr( "This algorithm sets the M value for geometries in a layer.\n\n"
                      "If M values already exist in the layer, they will be overwritten "
                      "with the new value. If no M values exist, the geometry will be "
                      "upgraded to include M values and the specified value used as "
                      "the initial M value for all geometries." );
}

QString QgsSetMValueAlgorithm::outputName() const
{
  return QObject::tr( "M Added" );
}

QgsSetMValueAlgorithm *QgsSetMValueAlgorithm::createInstance() const
{
  return new QgsSetMValueAlgorithm();
}

bool QgsSetMValueAlgorithm::supportInPlaceEdit( const QgsMapLayer *l ) const
{
  const QgsVectorLayer *layer = qobject_cast<const QgsVectorLayer *>( l );
  if ( !layer )
    return false;

  return QgsProcessingFeatureBasedAlgorithm::supportInPlaceEdit( l ) && QgsWkbTypes::hasM( layer->wkbType() );
}

Qgis::ProcessingFeatureSourceFlags QgsSetMValueAlgorithm::sourceFlags() const
{
  return Qgis::ProcessingFeatureSourceFlag::SkipGeometryValidityChecks;
}

Qgis::WkbType QgsSetMValueAlgorithm::outputWkbType( Qgis::WkbType type ) const
{
  return QgsWkbTypes::addM( type );
}

void QgsSetMValueAlgorithm::initParameters( const QVariantMap & )
{
  auto mValueParam = std::make_unique<QgsProcessingParameterNumber>( QStringLiteral( "M_VALUE" ), QObject::tr( "M Value" ), Qgis::ProcessingNumberParameterType::Double, 0.0 );
  mValueParam->setIsDynamic( true );
  mValueParam->setDynamicPropertyDefinition( QgsPropertyDefinition( QStringLiteral( "M_VALUE" ), QObject::tr( "M Value" ), QgsPropertyDefinition::Double ) );
  mValueParam->setDynamicLayerParameterName( QStringLiteral( "INPUT" ) );
  addParameter( mValueParam.release() );
}

bool QgsSetMValueAlgorithm::prepareAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback * )
{
  mMValue = parameterAsDouble( parameters, QStringLiteral( "M_VALUE" ), context );
  mDynamicMValue = QgsProcessingParameters::isDynamic( parameters, QStringLiteral( "M_VALUE" ) );
  if ( mDynamicMValue )
    mMValueProperty = parameters.value( QStringLiteral( "M_VALUE" ) ).value<QgsProperty>();

  return true;
}

QgsFeatureList QgsSetMValueAlgorithm::processFeature( const QgsFeature &feature, QgsProcessingContext &context, QgsProcessingFeedback * )
{
  QgsFeature f = feature;

  if ( f.hasGeometry() )
  {
    std::unique_ptr<QgsAbstractGeometry> newGeometry( f.geometry().constGet()->clone() );
    // addMValue won't alter existing M values, so drop them first
    if ( QgsWkbTypes::hasM( newGeometry->wkbType() ) )
      newGeometry->dropMValue();

    double m = mMValue;
    if ( mDynamicMValue )
      m = mMValueProperty.valueAsDouble( context.expressionContext(), m );

    newGeometry->addMValue( m );

    f.setGeometry( QgsGeometry( std::move( newGeometry ) ) );
  }

  return QgsFeatureList() << f;
}

///@endcond
