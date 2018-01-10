/***************************************************************************
                         qgsalgorithmmergevector.cpp
                         ------------------
    begin                : December 2017
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

#include "qgsalgorithmmergevector.h"

///@cond PRIVATE

QString QgsMergeVectorAlgorithm::name() const
{
  return QStringLiteral( "mergevectorlayers" );
}

QString QgsMergeVectorAlgorithm::displayName() const
{
  return QObject::tr( "Merge vector layers" );
}

QStringList QgsMergeVectorAlgorithm::tags() const
{
  return QObject::tr( "vector,layers,collect,merge,combine" ).split( ',' );
}

QString QgsMergeVectorAlgorithm::group() const
{
  return QObject::tr( "Vector general" );
}

QgsProcessingAlgorithm::Flags QgsMergeVectorAlgorithm::flags() const
{
  return QgsProcessingAlgorithm::flags() | QgsProcessingAlgorithm::FlagCanRunInBackground;
}

QString QgsMergeVectorAlgorithm::groupId() const
{
  return QStringLiteral( "vectorgeneral" );
}

void QgsMergeVectorAlgorithm::initAlgorithm( const QVariantMap & )
{
  addParameter( new QgsProcessingParameterMultipleLayers( QStringLiteral( "LAYERS" ), QObject::tr( "Input layers" ), QgsProcessing::TypeVector ) );
  addParameter( new QgsProcessingParameterCrs( QStringLiteral( "CRS" ), QObject::tr( "Destination CRS" ), QVariant(), true ) );
  addParameter( new QgsProcessingParameterFeatureSink( QStringLiteral( "OUTPUT" ), QObject::tr( "Merged" ) ) );
}

QString QgsMergeVectorAlgorithm::shortHelpString() const
{
  return QObject::tr( "This algorithm combines multiple vector layers of the same geometry type into a single one.\n\n"
                      "If attributes tables are different, the attribute table of the resulting layer will contain the attributes "
                      "from all input layers. New attributes will be added for the original layer name and source.\n\n"
                      "If any input layers contain Z or M values, then the output layer will also contain these values. Similarly, "
                      "if any of the input layers are multi-part, the output layer will also be a multi-part layer.\n\n"
                      "Optionally, the destination coordinate reference system (CRS) for the merged layer can be set. If it is not set, the CRS will be "
                      "taken from the first input layer. All layers will all be reprojected to match this CRS." );
}

QgsMergeVectorAlgorithm *QgsMergeVectorAlgorithm::createInstance() const
{
  return new QgsMergeVectorAlgorithm();
}

QVariantMap QgsMergeVectorAlgorithm::processAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
{
  const QList< QgsMapLayer * > layers = parameterAsLayerList( parameters, QStringLiteral( "LAYERS" ), context );

  QgsFields outputFields;
  long totalFeatureCount = 0;
  QgsWkbTypes::Type outputType = QgsWkbTypes::Unknown;
  QgsCoordinateReferenceSystem outputCrs = parameterAsCrs( parameters, QStringLiteral( "CRS" ), context );

  if ( outputCrs.isValid() )
    feedback->pushInfo( QObject::tr( "Using specified destination CRS %1" ).arg( outputCrs.authid() ) );

  bool errored = false;

  // loop through input layers and determine geometry type, crs, fields, total feature count,...
  long i = 0;
  for ( QgsMapLayer *layer : layers )
  {
    i++;

    if ( feedback->isCanceled() )
      break;

    if ( !layer )
    {
      feedback->pushDebugInfo( QObject::tr( "Error retrieving map layer." ) );
      errored = true;
      continue;
    }

    if ( layer->type() != QgsMapLayer::VectorLayer )
      throw QgsProcessingException( QObject::tr( "All layers must be vector layers!" ) );

    QgsVectorLayer *vl = qobject_cast< QgsVectorLayer * >( layer );

    if ( !outputCrs.isValid() && vl->crs().isValid() )
    {
      outputCrs = vl->crs();
      feedback->pushInfo( QObject::tr( "Taking destination CRS %1 from layer" ).arg( outputCrs.authid() ) );
    }

    // check wkb type
    if ( outputType != QgsWkbTypes::Unknown && outputType != QgsWkbTypes::NoGeometry )
    {
      if ( QgsWkbTypes::geometryType( outputType ) != QgsWkbTypes::geometryType( vl->wkbType() ) )
        throw QgsProcessingException( QObject::tr( "All layers must have same geometry type! Encountered a %1 layer when expecting a %2 layer." )
                                      .arg( QgsWkbTypes::geometryDisplayString( QgsWkbTypes::geometryType( vl->wkbType() ) ),
                                            QgsWkbTypes::geometryDisplayString( QgsWkbTypes::geometryType( outputType ) ) ) );

      if ( QgsWkbTypes::hasM( vl->wkbType() ) && !QgsWkbTypes::hasM( outputType ) )
      {
        outputType = QgsWkbTypes::addM( outputType );
        feedback->pushInfo( QObject::tr( "Found a layer with M values, upgrading output type to %1" ).arg( QgsWkbTypes::displayString( outputType ) ) );
      }
      if ( QgsWkbTypes::hasZ( vl->wkbType() ) && !QgsWkbTypes::hasZ( outputType ) )
      {
        outputType = QgsWkbTypes::addZ( outputType );
        feedback->pushInfo( QObject::tr( "Found a layer with Z values, upgrading output type to %1" ).arg( QgsWkbTypes::displayString( outputType ) ) );
      }
      if ( QgsWkbTypes::isMultiType( vl->wkbType() ) && !QgsWkbTypes::isMultiType( outputType ) )
      {
        outputType = QgsWkbTypes::multiType( outputType );
        feedback->pushInfo( QObject::tr( "Found a layer with multiparts, upgrading output type to %1" ).arg( QgsWkbTypes::displayString( outputType ) ) );
      }
    }
    else
    {
      outputType = vl->wkbType();
      feedback->pushInfo( QObject::tr( "Setting output type to %1" ).arg( QgsWkbTypes::displayString( outputType ) ) );
    }

    totalFeatureCount += vl->featureCount();

    // check field type
    for ( const QgsField &sourceField : vl->fields() )
    {
      bool found = false;
      for ( const QgsField &destField : outputFields )
      {
        if ( destField.name().compare( sourceField.name(), Qt::CaseInsensitive ) == 0 )
        {
          found = true;
          if ( destField.type() != sourceField.type() )
          {
            throw QgsProcessingException( QObject::tr( "%1 field in layer %2 has different data type than in other layers (%3 instead of %4)" )
                                          .arg( sourceField.name(), vl->name() ).arg( sourceField.typeName() ).arg( destField.typeName() ) );
          }
          break;
        }
      }

      if ( !found )
        outputFields.append( sourceField );
    }
  }

  bool addLayerField = false;
  if ( outputFields.lookupField( QStringLiteral( "layer" ) ) < 0 )
  {
    outputFields.append( QgsField( QStringLiteral( "layer" ), QVariant::String, QString(), 100 ) );
    addLayerField = true;
  }
  bool addPathField = false;
  if ( outputFields.lookupField( QStringLiteral( "path" ) ) < 0 )
  {
    outputFields.append( QgsField( QStringLiteral( "path" ), QVariant::String, QString(), 200 ) );
    addPathField = true;
  }

  QString dest;
  std::unique_ptr< QgsFeatureSink > sink( parameterAsSink( parameters, QStringLiteral( "OUTPUT" ), context, dest, outputFields, outputType, outputCrs ) );

  bool hasZ = QgsWkbTypes::hasZ( outputType );
  bool hasM = QgsWkbTypes::hasM( outputType );
  bool isMulti = QgsWkbTypes::isMultiType( outputType );
  double step = totalFeatureCount > 0 ? 100.0 / totalFeatureCount : 1;
  i = 0;
  for ( QgsMapLayer *layer : layers )
  {
    if ( !layer )
      continue;

    QgsVectorLayer *vl = qobject_cast< QgsVectorLayer * >( layer );
    if ( !vl )
      continue;

    feedback->pushInfo( QObject::tr( "Packaging layer %1/%2: %3" ).arg( i ).arg( layers.count() ).arg( layer->name() ) );

    QgsFeatureIterator it = vl->getFeatures( QgsFeatureRequest().setDestinationCrs( outputCrs, context.transformContext() ) );
    QgsFeature f;
    while ( it.nextFeature( f ) )
    {
      if ( feedback->isCanceled() )
        break;

      // ensure feature geometry is of correct type
      if ( f.hasGeometry() )
      {
        bool changed = false;
        QgsGeometry g = f.geometry();
        if ( hasZ && !g.constGet()->is3D() )
        {
          g.get()->addZValue( 0 );
          changed = true;
        }
        if ( hasM && !g.constGet()->isMeasure() )
        {
          g.get()->addMValue( 0 );
          changed = true;
        }
        if ( isMulti && !g.isMultipart() )
        {
          g.convertToMultiType();
          changed = true;
        }
        if ( changed )
          f.setGeometry( g );
      }

      // process feature attributes
      QgsAttributes destAttributes;
      for ( const QgsField &destField : outputFields )
      {
        if ( addLayerField && destField.name() == QLatin1String( "layer" ) )
        {
          destAttributes.append( layer->name() );
          continue;
        }
        else if ( addPathField && destField.name() == QLatin1String( "path" ) )
        {
          destAttributes.append( layer->publicSource() );
          continue;
        }

        QVariant destAttribute;
        int sourceIndex = vl->fields().lookupField( destField.name() );
        if ( sourceIndex >= 0 )
        {
          destAttribute = f.attributes().at( sourceIndex );
        }
        destAttributes.append( destAttribute );
      }
      f.setAttributes( destAttributes );

      sink->addFeature( f, QgsFeatureSink::FastInsert );
      i += 1;
      feedback->setProgress( i * step );
    }
  }

  if ( errored )
    throw QgsProcessingException( QObject::tr( "Error obtained while merging one or more layers." ) );

  QVariantMap outputs;
  outputs.insert( QStringLiteral( "OUTPUT" ), dest );
  return outputs;
}

///@endcond
