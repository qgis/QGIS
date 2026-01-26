/***************************************************************************
                         qgsalgorithmarraytranslatedfeatures.cpp
                         ---------------------
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

#include "qgsalgorithmarraytranslatedfeatures.h"

///@cond PRIVATE

QString QgsArrayTranslatedFeaturesAlgorithm::name() const
{
  return u"arraytranslatedfeatures"_s;
}

QString QgsArrayTranslatedFeaturesAlgorithm::displayName() const
{
  return QObject::tr( "Array of translated features" );
}

QStringList QgsArrayTranslatedFeaturesAlgorithm::tags() const
{
  return QObject::tr( "translate,parallel,offset,duplicate,grid,spaced,moved,copy,features,objects,step,repeat" ).split( ',' );
}

QString QgsArrayTranslatedFeaturesAlgorithm::group() const
{
  return QObject::tr( "Vector creation" );
}

QString QgsArrayTranslatedFeaturesAlgorithm::groupId() const
{
  return u"vectorcreation"_s;
}

QString QgsArrayTranslatedFeaturesAlgorithm::outputName() const
{
  return QObject::tr( "Translated" );
}

QString QgsArrayTranslatedFeaturesAlgorithm::shortHelpString() const
{
  return QObject::tr( "This algorithm creates copies of features in a layer, by creating multiple translated versions of each feature. "
                      "Each copy is incrementally displaced by a preset amount in the x/y/z/m axis." );
}

QString QgsArrayTranslatedFeaturesAlgorithm::shortDescription() const
{
  return QObject::tr( "Creates multiple translated copies of features in a layer." );
}

Qgis::ProcessingAlgorithmDocumentationFlags QgsArrayTranslatedFeaturesAlgorithm::documentationFlags() const
{
  return Qgis::ProcessingAlgorithmDocumentationFlag::RegeneratesPrimaryKey;
}

QgsArrayTranslatedFeaturesAlgorithm *QgsArrayTranslatedFeaturesAlgorithm::createInstance() const
{
  return new QgsArrayTranslatedFeaturesAlgorithm();
}

void QgsArrayTranslatedFeaturesAlgorithm::initParameters( const QVariantMap & )
{
  auto count = std::make_unique<QgsProcessingParameterNumber>( u"COUNT"_s, QObject::tr( "Number of features to create" ), Qgis::ProcessingNumberParameterType::Integer, 10, false, 1 );
  count->setIsDynamic( true );
  count->setDynamicPropertyDefinition( QgsPropertyDefinition( u"COUNT"_s, QObject::tr( "Number of features to create" ), QgsPropertyDefinition::IntegerPositiveGreaterZero ) );
  count->setDynamicLayerParameterName( u"INPUT"_s );
  addParameter( count.release() );

  auto xOffset = std::make_unique<QgsProcessingParameterDistance>( u"DELTA_X"_s, QObject::tr( "Step distance (x-axis)" ), 0.0, u"INPUT"_s );
  xOffset->setIsDynamic( true );
  xOffset->setDynamicPropertyDefinition( QgsPropertyDefinition( u"DELTA_X"_s, QObject::tr( "Offset distance (x-axis)" ), QgsPropertyDefinition::Double ) );
  xOffset->setDynamicLayerParameterName( u"INPUT"_s );
  addParameter( xOffset.release() );

  auto yOffset = std::make_unique<QgsProcessingParameterDistance>( u"DELTA_Y"_s, QObject::tr( "Step distance (y-axis)" ), 0.0, u"INPUT"_s );
  yOffset->setIsDynamic( true );
  yOffset->setDynamicPropertyDefinition( QgsPropertyDefinition( u"DELTA_Y"_s, QObject::tr( "Offset distance (y-axis)" ), QgsPropertyDefinition::Double ) );
  yOffset->setDynamicLayerParameterName( u"INPUT"_s );
  addParameter( yOffset.release() );

  auto zOffset = std::make_unique<QgsProcessingParameterNumber>( u"DELTA_Z"_s, QObject::tr( "Step distance (z-axis)" ), Qgis::ProcessingNumberParameterType::Double, 0.0 );
  zOffset->setIsDynamic( true );
  zOffset->setDynamicPropertyDefinition( QgsPropertyDefinition( u"DELTA_Z"_s, QObject::tr( "Offset distance (z-axis)" ), QgsPropertyDefinition::Double ) );
  zOffset->setDynamicLayerParameterName( u"INPUT"_s );
  addParameter( zOffset.release() );

  auto mOffset = std::make_unique<QgsProcessingParameterNumber>( u"DELTA_M"_s, QObject::tr( "Step distance (m values)" ), Qgis::ProcessingNumberParameterType::Double, 0.0 );
  mOffset->setIsDynamic( true );
  mOffset->setDynamicPropertyDefinition( QgsPropertyDefinition( u"DELTA_M"_s, QObject::tr( "Offset distance (m values)" ), QgsPropertyDefinition::Double ) );
  mOffset->setDynamicLayerParameterName( u"INPUT"_s );
  addParameter( mOffset.release() );
}

bool QgsArrayTranslatedFeaturesAlgorithm::prepareAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback * )
{
  mCount = parameterAsInt( parameters, u"COUNT"_s, context );
  mDynamicCount = QgsProcessingParameters::isDynamic( parameters, u"COUNT"_s );
  if ( mDynamicCount )
    mCountProperty = parameters.value( u"COUNT"_s ).value<QgsProperty>();

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

QgsFeatureList QgsArrayTranslatedFeaturesAlgorithm::processFeature( const QgsFeature &feature, QgsProcessingContext &context, QgsProcessingFeedback * )
{
  QgsFeatureList result = QgsFeatureList();

  if ( feature.hasGeometry() )
  {
    QgsGeometry geometry = feature.geometry();
    const QgsAttributes attr = feature.attributes();

    int count = mCount;
    if ( mDynamicCount )
      count = mCountProperty.valueAsInt( context.expressionContext(), count );
    result.reserve( count + 1 );

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

    // we add the original feature only after adding initial z/m values if needed
    if ( deltaZ != 0 && !geometry.constGet()->is3D() )
      geometry.get()->addZValue( 0 );
    if ( deltaM != 0 && !geometry.constGet()->isMeasure() )
      geometry.get()->addMValue( 0 );

    {
      QgsFeature original = feature;
      original.setGeometry( geometry );
      QgsAttributes outAttrs = attr;
      outAttrs << QVariant( 0 );
      original.setAttributes( outAttrs );
      result << original;
    }

    for ( int i = 0; i < count; ++i )
    {
      QgsFeature offsetFeature = feature;
      geometry.translate( deltaX, deltaY, deltaZ, deltaM );
      offsetFeature.setGeometry( geometry );
      QgsAttributes outAttrs = attr;
      outAttrs << QVariant( i + 1 );
      offsetFeature.setAttributes( outAttrs );
      result << offsetFeature;
    }
  }
  else
  {
    result << feature;
  }

  return result;
}

Qgis::WkbType QgsArrayTranslatedFeaturesAlgorithm::outputWkbType( Qgis::WkbType inputWkbType ) const
{
  Qgis::WkbType wkb = inputWkbType;
  if ( mDeltaZ != 0 )
    wkb = QgsWkbTypes::addZ( wkb );
  if ( mDeltaM != 0 )
    wkb = QgsWkbTypes::addM( wkb );
  return wkb;
}

QgsFields QgsArrayTranslatedFeaturesAlgorithm::outputFields( const QgsFields &inputFields ) const
{
  QgsFields output;
  output.append( QgsField( u"instance"_s, QMetaType::Type::Int ) );
  return QgsProcessingUtils::combineFields( inputFields, output );
}

QgsFeatureSink::SinkFlags QgsArrayTranslatedFeaturesAlgorithm::sinkFlags() const
{
  return QgsFeatureSink::RegeneratePrimaryKey;
}

///@endcond
