/***************************************************************************
                         qgsalgorithmjoinbynearest.cpp
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

#include "qgsalgorithmjoinbynearest.h"

#include <algorithm>

#include "qgslinestring.h"
#include "qgsprocessingoutputs.h"
#include "qgsspatialindex.h"

///@cond PRIVATE

QString QgsJoinByNearestAlgorithm::name() const
{
  return u"joinbynearest"_s;
}

QString QgsJoinByNearestAlgorithm::displayName() const
{
  return QObject::tr( "Join attributes by nearest" );
}

QStringList QgsJoinByNearestAlgorithm::tags() const
{
  return QObject::tr( "join,connect,attributes,values,fields,tables,proximity,closest,neighbour,neighbor,n-nearest,distance" ).split( ',' );
}

QString QgsJoinByNearestAlgorithm::group() const
{
  return QObject::tr( "Vector general" );
}

QString QgsJoinByNearestAlgorithm::groupId() const
{
  return u"vectorgeneral"_s;
}

void QgsJoinByNearestAlgorithm::initAlgorithm( const QVariantMap & )
{
  addParameter( new QgsProcessingParameterFeatureSource( u"INPUT"_s, QObject::tr( "Input layer" ) ) );
  addParameter( new QgsProcessingParameterFeatureSource( u"INPUT_2"_s, QObject::tr( "Input layer 2" ) ) );

  addParameter( new QgsProcessingParameterField( u"FIELDS_TO_COPY"_s, QObject::tr( "Layer 2 fields to copy (leave empty to copy all fields)" ), QVariant(), u"INPUT_2"_s, Qgis::ProcessingFieldParameterDataType::Any, true, true ) );

  addParameter( new QgsProcessingParameterBoolean( u"DISCARD_NONMATCHING"_s, QObject::tr( "Discard records which could not be joined" ), false ) );

  addParameter( new QgsProcessingParameterString( u"PREFIX"_s, QObject::tr( "Joined field prefix" ), QVariant(), false, true ) );

  addParameter( new QgsProcessingParameterNumber( u"NEIGHBORS"_s, QObject::tr( "Maximum nearest neighbors" ), Qgis::ProcessingNumberParameterType::Integer, 1, false, 1 ) );

  addParameter( new QgsProcessingParameterDistance( u"MAX_DISTANCE"_s, QObject::tr( "Maximum distance" ), QVariant(), u"INPUT"_s, true, 0 ) );

  addParameter( new QgsProcessingParameterFeatureSink( u"OUTPUT"_s, QObject::tr( "Joined layer" ), Qgis::ProcessingSourceType::VectorAnyGeometry, QVariant(), true, true ) );

  auto nonMatchingSink = std::make_unique<QgsProcessingParameterFeatureSink>(
    u"NON_MATCHING"_s, QObject::tr( "Unjoinable features from first layer" ), Qgis::ProcessingSourceType::VectorAnyGeometry, QVariant(), true, false
  );
  // TODO GUI doesn't support advanced outputs yet
  //nonMatchingSink->setFlags(nonMatchingSink->flags() | Qgis::ProcessingParameterFlag::Advanced );
  addParameter( nonMatchingSink.release() );

  addOutput( new QgsProcessingOutputNumber( u"JOINED_COUNT"_s, QObject::tr( "Number of joined features from input table" ) ) );
  addOutput( new QgsProcessingOutputNumber( u"UNJOINABLE_COUNT"_s, QObject::tr( "Number of unjoinable features from input table" ) ) );
}

QString QgsJoinByNearestAlgorithm::shortHelpString() const
{
  return QObject::tr( "This algorithm takes an input vector layer and creates a new vector layer that is an extended version of the "
                      "input one, with additional attributes in its attribute table.\n\n"
                      "The additional attributes and their values are taken from a second vector layer, where features are joined "
                      "by finding the closest features from each layer. By default only the single nearest feature is joined,"
                      "but optionally the join can use the n-nearest neighboring features instead. If multiple features are found "
                      "with identical distances these will all be returned (even if the total number of features exceeds the specified "
                      "maximum feature count).\n\n"
                      "If a maximum distance is specified, then only features which are closer than this distance "
                      "will be matched.\n\n"
                      "The output features will contain the selected attributes from the nearest feature, "
                      "along with new attributes for the distance to the near feature, the index of the feature, "
                      "and the coordinates of the closest point on the input feature (feature_x, feature_y) "
                      "to the matched nearest feature, and the coordinates of the closet point on the matched feature "
                      "(nearest_x, nearest_y).\n\n"
                      "This algorithm uses purely Cartesian calculations for distance, and does not consider "
                      "geodetic or ellipsoid properties when determining feature proximity." );
}

QString QgsJoinByNearestAlgorithm::shortDescription() const
{
  return QObject::tr( "Joins a layer to another layer, using the closest features (nearest neighbors)." );
}

Qgis::ProcessingAlgorithmDocumentationFlags QgsJoinByNearestAlgorithm::documentationFlags() const
{
  return Qgis::ProcessingAlgorithmDocumentationFlag::RegeneratesPrimaryKey;
}

QgsJoinByNearestAlgorithm *QgsJoinByNearestAlgorithm::createInstance() const
{
  return new QgsJoinByNearestAlgorithm();
}

QVariantMap QgsJoinByNearestAlgorithm::processAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
{
  const int neighbors = parameterAsInt( parameters, u"NEIGHBORS"_s, context );
  const bool discardNonMatching = parameterAsBoolean( parameters, u"DISCARD_NONMATCHING"_s, context );
  const double maxDistance = parameters.value( u"MAX_DISTANCE"_s ).isValid() ? parameterAsDouble( parameters, u"MAX_DISTANCE"_s, context ) : std::numeric_limits<double>::quiet_NaN();
  std::unique_ptr<QgsProcessingFeatureSource> input( parameterAsSource( parameters, u"INPUT"_s, context ) );
  if ( !input )
    throw QgsProcessingException( invalidSourceError( parameters, u"INPUT"_s ) );

  std::unique_ptr<QgsProcessingFeatureSource> input2( parameterAsSource( parameters, u"INPUT_2"_s, context ) );
  if ( !input2 )
    throw QgsProcessingException( invalidSourceError( parameters, u"INPUT_2"_s ) );

  const bool sameSourceAndTarget = parameters.value( u"INPUT"_s ) == parameters.value( u"INPUT_2"_s );

  const QString prefix = parameterAsString( parameters, u"PREFIX"_s, context );
  const QStringList fieldsToCopy = parameterAsStrings( parameters, u"FIELDS_TO_COPY"_s, context );

  QgsFields outFields2;
  QgsAttributeList fields2Indices;
  if ( fieldsToCopy.empty() )
  {
    outFields2 = input2->fields();
    fields2Indices.reserve( outFields2.count() );
    for ( int i = 0; i < outFields2.count(); ++i )
    {
      fields2Indices << i;
    }
  }
  else
  {
    fields2Indices.reserve( fieldsToCopy.count() );
    for ( const QString &field : fieldsToCopy )
    {
      const int index = input2->fields().lookupField( field );
      if ( index >= 0 )
      {
        fields2Indices << index;
        outFields2.append( input2->fields().at( index ) );
      }
    }
  }

  if ( !prefix.isEmpty() )
  {
    for ( int i = 0; i < outFields2.count(); ++i )
    {
      outFields2.rename( i, prefix + outFields2[i].name() );
    }
  }

  const QgsAttributeList fields2Fetch = fields2Indices;

  QgsFields outFields = QgsProcessingUtils::combineFields( input->fields(), outFields2 );

  QgsFields resultFields;
  resultFields.append( QgsField( u"n"_s, QMetaType::Type::Int ) );
  resultFields.append( QgsField( u"distance"_s, QMetaType::Type::Double ) );
  resultFields.append( QgsField( u"feature_x"_s, QMetaType::Type::Double ) );
  resultFields.append( QgsField( u"feature_y"_s, QMetaType::Type::Double ) );
  resultFields.append( QgsField( u"nearest_x"_s, QMetaType::Type::Double ) );
  resultFields.append( QgsField( u"nearest_y"_s, QMetaType::Type::Double ) );
  outFields = QgsProcessingUtils::combineFields( outFields, resultFields );

  QString dest;
  std::unique_ptr<QgsFeatureSink> sink( parameterAsSink( parameters, u"OUTPUT"_s, context, dest, outFields, input->wkbType(), input->sourceCrs(), QgsFeatureSink::RegeneratePrimaryKey ) );
  if ( parameters.value( u"OUTPUT"_s ).isValid() && !sink )
    throw QgsProcessingException( invalidSinkError( parameters, u"OUTPUT"_s ) );

  QString destNonMatching1;
  std::unique_ptr<QgsFeatureSink> sinkNonMatching1( parameterAsSink( parameters, u"NON_MATCHING"_s, context, destNonMatching1, input->fields(), input->wkbType(), input->sourceCrs(), QgsFeatureSink::RegeneratePrimaryKey ) );
  if ( parameters.value( u"NON_MATCHING"_s ).isValid() && !sinkNonMatching1 )
    throw QgsProcessingException( invalidSinkError( parameters, u"NON_MATCHING"_s ) );

  // make spatial index
  const QgsFeatureIterator f2 = input2->getFeatures( QgsFeatureRequest().setDestinationCrs( input->sourceCrs(), context.transformContext() ).setSubsetOfAttributes( fields2Fetch ) );
  QHash<QgsFeatureId, QgsAttributes> input2AttributeCache;
  double step = input2->featureCount() > 0 ? 50.0 / input2->featureCount() : 1;
  int i = 0;
  const QgsSpatialIndex index( f2, [&]( const QgsFeature &f ) -> bool {
    i++;
    if ( feedback->isCanceled() )
      return false;

    feedback->setProgress( i * step );

    if ( !f.hasGeometry() )
      return true;

    // only keep selected attributes
    QgsAttributes attributes;
    for ( int field2Index : fields2Indices )
    {
      attributes << f.attribute( field2Index );
    }
    input2AttributeCache.insert( f.id(), attributes );

    return true; }, QgsSpatialIndex::FlagStoreFeatureGeometries );

  QgsFeature f;

  // create extra null attributes for non-matched records (the +2 is for the "n" and "distance", and start/end x/y fields)
  QgsAttributes nullMatch;
  nullMatch.reserve( fields2Indices.size() + 6 );
  for ( int i = 0; i < fields2Indices.count() + 6; ++i )
    nullMatch << QVariant();

  long long joinedCount = 0;
  long long unjoinedCount = 0;

  // Create output vector layer with additional attributes
  step = input->featureCount() > 0 ? 50.0 / input->featureCount() : 1;
  QgsFeatureIterator features = input->getFeatures();
  i = 0;
  while ( features.nextFeature( f ) )
  {
    i++;
    if ( feedback->isCanceled() )
    {
      break;
    }

    feedback->setProgress( 50 + i * step );

    if ( !f.hasGeometry() )
    {
      unjoinedCount++;
      if ( sinkNonMatching1 )
      {
        if ( !sinkNonMatching1->addFeature( f, QgsFeatureSink::FastInsert ) )
          throw QgsProcessingException( writeFeatureError( sinkNonMatching1.get(), parameters, u"NON_MATCHING"_s ) );
      }
      if ( sink && !discardNonMatching )
      {
        QgsAttributes attr = f.attributes();
        attr.append( nullMatch );
        f.setAttributes( attr );
        if ( !sink->addFeature( f, QgsFeatureSink::FastInsert ) )
          throw QgsProcessingException( writeFeatureError( sink.get(), parameters, u"OUTPUT"_s ) );
      }
    }
    else
    {
      // note - if using same source as target, we have to get one extra neighbor, since the first match will be the input feature

      // if the user didn't specify a distance (isnan), then use 0 for nearestNeighbor() parameter
      // if the user specified 0 exactly, then use the smallest positive double value instead
      const double searchDistance = std::isnan( maxDistance ) ? 0 : std::max( std::numeric_limits<double>::min(), maxDistance );
      const QList<QgsFeatureId> nearest = index.nearestNeighbor( f.geometry(), neighbors + ( sameSourceAndTarget ? 1 : 0 ), searchDistance );

      if ( nearest.count() > neighbors + ( sameSourceAndTarget ? 1 : 0 ) )
      {
        feedback->pushInfo( QObject::tr( "Multiple matching features found at same distance from search feature, found %n feature(s) instead of %1", nullptr, nearest.count() - ( sameSourceAndTarget ? 1 : 0 ) ).arg( neighbors ) );
      }
      QgsFeature out;
      out.setGeometry( f.geometry() );
      int j = 0;
      for ( const QgsFeatureId id : nearest )
      {
        if ( sameSourceAndTarget && id == f.id() )
          continue; // don't match to same feature if using a single input table
        j++;
        if ( sink )
        {
          QgsAttributes attr = f.attributes();
          attr.append( input2AttributeCache.value( id ) );
          attr.append( j );

          const QgsGeometry closestLine = f.geometry().shortestLine( index.geometry( id ) );
          if ( const QgsLineString *line = qgsgeometry_cast<const QgsLineString *>( closestLine.constGet() ) )
          {
            attr.append( line->length() );
            attr.append( line->startPoint().x() );
            attr.append( line->startPoint().y() );
            attr.append( line->endPoint().x() );
            attr.append( line->endPoint().y() );
          }
          else
          {
            attr.append( QVariant() ); //distance
            attr.append( QVariant() ); //start x
            attr.append( QVariant() ); //start y
            attr.append( QVariant() ); //end x
            attr.append( QVariant() ); //end y
          }
          out.setAttributes( attr );
          if ( !sink->addFeature( out, QgsFeatureSink::FastInsert ) )
            throw QgsProcessingException( writeFeatureError( sink.get(), parameters, u"OUTPUT"_s ) );
        }
      }
      if ( j > 0 )
        joinedCount++;
      else
      {
        if ( sinkNonMatching1 )
        {
          if ( !sinkNonMatching1->addFeature( f, QgsFeatureSink::FastInsert ) )
            throw QgsProcessingException( writeFeatureError( sinkNonMatching1.get(), parameters, u"NON_MATCHING"_s ) );
        }
        if ( !discardNonMatching && sink )
        {
          QgsAttributes attr = f.attributes();
          attr.append( nullMatch );
          f.setAttributes( attr );
          if ( !sink->addFeature( f, QgsFeatureSink::FastInsert ) )
            throw QgsProcessingException( writeFeatureError( sink.get(), parameters, u"OUTPUT"_s ) );
        }
        unjoinedCount++;
      }
    }
  }

  QVariantMap outputs;
  outputs.insert( u"JOINED_COUNT"_s, joinedCount );
  outputs.insert( u"UNJOINABLE_COUNT"_s, unjoinedCount );
  if ( sink )
  {
    sink->finalize();
    outputs.insert( u"OUTPUT"_s, dest );
  }
  if ( sinkNonMatching1 )
  {
    sinkNonMatching1->finalize();
    outputs.insert( u"NON_MATCHING"_s, destNonMatching1 );
  }
  return outputs;
}


///@endcond
