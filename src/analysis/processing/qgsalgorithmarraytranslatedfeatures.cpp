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
  return QStringLiteral( "arraytranslatedfeatures" );
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
  return QStringLiteral( "vectorcreation" );
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

QgsArrayTranslatedFeaturesAlgorithm *QgsArrayTranslatedFeaturesAlgorithm::createInstance() const
{
  return new QgsArrayTranslatedFeaturesAlgorithm();
}

void QgsArrayTranslatedFeaturesAlgorithm::initParameters( const QVariantMap & )
{
  std::unique_ptr< QgsProcessingParameterNumber > count = std::make_unique< QgsProcessingParameterNumber >( QStringLiteral( "COUNT" ),
      QObject::tr( "Number of features to create" ), QgsProcessingParameterNumber::Integer,
      10, false, 1 );
  count->setIsDynamic( true );
  count->setDynamicPropertyDefinition( QgsPropertyDefinition( QStringLiteral( "COUNT" ), QObject::tr( "Number of features to create" ), QgsPropertyDefinition::IntegerPositiveGreaterZero ) );
  count->setDynamicLayerParameterName( QStringLiteral( "INPUT" ) );
  addParameter( count.release() );

  std::unique_ptr< QgsProcessingParameterDistance > xOffset = std::make_unique< QgsProcessingParameterDistance >( QStringLiteral( "DELTA_X" ),
      QObject::tr( "Step distance (x-axis)" ),
      0.0, QStringLiteral( "INPUT" ) );
  xOffset->setIsDynamic( true );
  xOffset->setDynamicPropertyDefinition( QgsPropertyDefinition( QStringLiteral( "DELTA_X" ), QObject::tr( "Offset distance (x-axis)" ), QgsPropertyDefinition::Double ) );
  xOffset->setDynamicLayerParameterName( QStringLiteral( "INPUT" ) );
  addParameter( xOffset.release() );

  std::unique_ptr< QgsProcessingParameterDistance > yOffset = std::make_unique< QgsProcessingParameterDistance >( QStringLiteral( "DELTA_Y" ),
      QObject::tr( "Step distance (y-axis)" ),
      0.0, QStringLiteral( "INPUT" ) );
  yOffset->setIsDynamic( true );
  yOffset->setDynamicPropertyDefinition( QgsPropertyDefinition( QStringLiteral( "DELTA_Y" ), QObject::tr( "Offset distance (y-axis)" ), QgsPropertyDefinition::Double ) );
  yOffset->setDynamicLayerParameterName( QStringLiteral( "INPUT" ) );
  addParameter( yOffset.release() );

  std::unique_ptr< QgsProcessingParameterNumber > zOffset = std::make_unique< QgsProcessingParameterNumber >( QStringLiteral( "DELTA_Z" ),
      QObject::tr( "Step distance (z-axis)" ), QgsProcessingParameterNumber::Double,
      0.0 );
  zOffset->setIsDynamic( true );
  zOffset->setDynamicPropertyDefinition( QgsPropertyDefinition( QStringLiteral( "DELTA_Z" ), QObject::tr( "Offset distance (z-axis)" ), QgsPropertyDefinition::Double ) );
  zOffset->setDynamicLayerParameterName( QStringLiteral( "INPUT" ) );
  addParameter( zOffset.release() );

  std::unique_ptr< QgsProcessingParameterNumber > mOffset = std::make_unique< QgsProcessingParameterNumber >( QStringLiteral( "DELTA_M" ),
      QObject::tr( "Step distance (m values)" ), QgsProcessingParameterNumber::Double,
      0.0 );
  mOffset->setIsDynamic( true );
  mOffset->setDynamicPropertyDefinition( QgsPropertyDefinition( QStringLiteral( "DELTA_M" ), QObject::tr( "Offset distance (m values)" ), QgsPropertyDefinition::Double ) );
  mOffset->setDynamicLayerParameterName( QStringLiteral( "INPUT" ) );
  addParameter( mOffset.release() );
}

bool QgsArrayTranslatedFeaturesAlgorithm::prepareAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback * )
{
  mCount = parameterAsInt( parameters, QStringLiteral( "COUNT" ), context );
  mDynamicCount = QgsProcessingParameters::isDynamic( parameters, QStringLiteral( "COUNT" ) );
  if ( mDynamicCount )
    mCountProperty = parameters.value( QStringLiteral( "COUNT" ) ).value< QgsProperty >();

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

QgsWkbTypes::Type QgsArrayTranslatedFeaturesAlgorithm::outputWkbType( QgsWkbTypes::Type inputWkbType ) const
{
  QgsWkbTypes::Type wkb = inputWkbType;
  if ( mDeltaZ != 0 )
    wkb = QgsWkbTypes::addZ( wkb );
  if ( mDeltaM != 0 )
    wkb = QgsWkbTypes::addM( wkb );
  return wkb;
}

QgsFields QgsArrayTranslatedFeaturesAlgorithm::outputFields( const QgsFields &inputFields ) const
{
  QgsFields output;
  output.append( QgsField( QStringLiteral( "instance" ), QVariant::Int ) );
  return QgsProcessingUtils::combineFields( inputFields, output );
}

QgsFeatureSink::SinkFlags QgsArrayTranslatedFeaturesAlgorithm::sinkFlags() const
{
  return QgsFeatureSink::RegeneratePrimaryKey;
}

///@endcond


