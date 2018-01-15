/***************************************************************************
                         qgsalgorithmmeancoordinates.cpp
                         ---------------------
    begin                : April 2017
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

#include "qgsalgorithmmeancoordinates.h"

///@cond PRIVATE

QString QgsMeanCoordinatesAlgorithm::name() const
{
  return QStringLiteral( "meancoordinates" );
}

QString QgsMeanCoordinatesAlgorithm::displayName() const
{
  return QObject::tr( "Mean coordinate(s)" );
}

QStringList QgsMeanCoordinatesAlgorithm::tags() const
{
  return QObject::tr( "mean,average,coordinate" ).split( ',' );
}

QString QgsMeanCoordinatesAlgorithm::group() const
{
  return QObject::tr( "Vector analysis" );
}

QString QgsMeanCoordinatesAlgorithm::groupId() const
{
  return QStringLiteral( "vectoranalysis" );
}

QgsProcessingAlgorithm::Flags QgsMeanCoordinatesAlgorithm::flags() const
{
  return QgsProcessingAlgorithm::flags() | QgsProcessingAlgorithm::FlagCanRunInBackground;
}

void QgsMeanCoordinatesAlgorithm::initAlgorithm( const QVariantMap & )
{
  addParameter( new QgsProcessingParameterFeatureSource( QStringLiteral( "INPUT" ),
                QObject::tr( "Input layer" ), QList< int >() << QgsProcessing::TypeVectorAnyGeometry ) );
  addParameter( new QgsProcessingParameterField( QStringLiteral( "WEIGHT" ), QObject::tr( "Weight field" ),
                QVariant(), QStringLiteral( "INPUT" ),
                QgsProcessingParameterField::Numeric, false, true ) );
  addParameter( new QgsProcessingParameterField( QStringLiteral( "UID" ),
                QObject::tr( "Unique ID field" ), QVariant(),
                QStringLiteral( "INPUT" ), QgsProcessingParameterField::Any, false, true ) );
  addParameter( new QgsProcessingParameterFeatureSink( QStringLiteral( "OUTPUT" ), QObject::tr( "Mean coordinates" ), QgsProcessing::TypeVectorPoint ) );
}

QString QgsMeanCoordinatesAlgorithm::shortHelpString() const
{
  return QObject::tr( "This algorithm computes a point layer with the center of mass of geometries in an input layer.\n\n"
                      "An attribute can be specified as containing weights to be applied to each feature when computing the center of mass.\n\n"
                      "If an attribute is selected in the <Unique ID field> parameter, features will be grouped according "
                      "to values in this field. Instead of a single point with the center of mass of the whole layer, "
                      "the output layer will contain a center of mass for the features in each category." );
}

QgsMeanCoordinatesAlgorithm *QgsMeanCoordinatesAlgorithm::createInstance() const
{
  return new QgsMeanCoordinatesAlgorithm();
}

QVariantMap QgsMeanCoordinatesAlgorithm::processAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
{
  std::unique_ptr< QgsFeatureSource > source( parameterAsSource( parameters, QStringLiteral( "INPUT" ), context ) );
  if ( !source )
    return QVariantMap();

  QString weightFieldName = parameterAsString( parameters, QStringLiteral( "WEIGHT" ), context );
  QString uniqueFieldName = parameterAsString( parameters, QStringLiteral( "UID" ), context );

  QgsAttributeList attributes;
  int weightIndex = -1;
  if ( !weightFieldName.isEmpty() )
  {
    weightIndex = source->fields().lookupField( weightFieldName );
    if ( weightIndex >= 0 )
      attributes.append( weightIndex );
  }

  int uniqueFieldIndex = -1;
  if ( !uniqueFieldName.isEmpty() )
  {
    uniqueFieldIndex = source->fields().lookupField( uniqueFieldName );
    if ( uniqueFieldIndex >= 0 )
      attributes.append( uniqueFieldIndex );
  }

  QgsFields fields;
  fields.append( QgsField( QStringLiteral( "MEAN_X" ), QVariant::Double, QString(), 24, 15 ) );
  fields.append( QgsField( QStringLiteral( "MEAN_Y" ), QVariant::Double, QString(), 24, 15 ) );
  if ( uniqueFieldIndex >= 0 )
  {
    QgsField uniqueField = source->fields().at( uniqueFieldIndex );
    fields.append( uniqueField );
  }

  QString dest;
  std::unique_ptr< QgsFeatureSink > sink( parameterAsSink( parameters, QStringLiteral( "OUTPUT" ), context, dest, fields,
                                          QgsWkbTypes::Point, source->sourceCrs() ) );
  if ( !sink )
    return QVariantMap();

  QgsFeatureIterator features = source->getFeatures( QgsFeatureRequest().setSubsetOfAttributes( attributes ) );

  double step = source->featureCount() > 0 ? 50.0 / source->featureCount() : 1;
  int i = 0;
  QgsFeature feat;

  QHash< QVariant, QList< double > > means;
  while ( features.nextFeature( feat ) )
  {
    i++;
    if ( feedback->isCanceled() )
    {
      break;
    }

    feedback->setProgress( i * step );
    if ( !feat.hasGeometry() )
      continue;


    QVariant featureClass;
    if ( uniqueFieldIndex >= 0 )
    {
      featureClass = feat.attribute( uniqueFieldIndex );
    }
    else
    {
      featureClass = QStringLiteral( "#####singleclass#####" );
    }

    double weight = 1;
    if ( weightIndex >= 0 )
    {
      bool ok = false;
      weight = feat.attribute( weightIndex ).toDouble( &ok );
      if ( !ok )
        weight = 1.0;
    }

    if ( weight < 0 )
    {
      throw QgsProcessingException( QObject::tr( "Negative weight value found. Please fix your data and try again." ) );
    }

    QList< double > values = means.value( featureClass );
    double cx = 0;
    double cy = 0;
    double totalWeight = 0;
    if ( !values.empty() )
    {
      cx = values.at( 0 );
      cy = values.at( 1 );
      totalWeight = values.at( 2 );
    }

    QgsVertexId vid;
    QgsPoint pt;
    const QgsAbstractGeometry *g = feat.geometry().constGet();
    // NOTE - should this be including the duplicate nodes for closed rings? currently it is,
    // but I suspect that the expected behavior would be to NOT include these
    while ( g->nextVertex( vid, pt ) )
    {
      cx += pt.x() * weight;
      cy += pt.y() * weight;
      totalWeight += weight;
    }

    means[featureClass] = QList< double >() << cx << cy << totalWeight;
  }

  i = 0;
  step = !means.empty() ? 50.0 / means.count() : 1;
  for ( auto it = means.constBegin(); it != means.constEnd(); ++it )
  {
    i++;
    if ( feedback->isCanceled() )
    {
      break;
    }

    feedback->setProgress( 50 + i * step );
    if ( qgsDoubleNear( it.value().at( 2 ), 0 ) )
      continue;

    QgsFeature outFeat;
    double cx = it.value().at( 0 ) / it.value().at( 2 );
    double cy = it.value().at( 1 ) / it.value().at( 2 );

    QgsPointXY meanPoint( cx, cy );
    outFeat.setGeometry( QgsGeometry::fromPointXY( meanPoint ) );

    QgsAttributes attributes;
    attributes << cx << cy;
    if ( uniqueFieldIndex >= 0 )
      attributes.append( it.key() );

    outFeat.setAttributes( attributes );
    sink->addFeature( outFeat, QgsFeatureSink::FastInsert );
  }

  QVariantMap outputs;
  outputs.insert( QStringLiteral( "OUTPUT" ), dest );
  return outputs;
}


///@endcond


