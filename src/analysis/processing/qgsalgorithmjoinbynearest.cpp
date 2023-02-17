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
#include "qgsprocessingoutputs.h"
#include "qgslinestring.h"

#include <algorithm>

///@cond PRIVATE

QString QgsJoinByNearestAlgorithm::name() const
{
  return QStringLiteral( "joinbynearest" );
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
  return QStringLiteral( "vectorgeneral" );
}

void QgsJoinByNearestAlgorithm::initAlgorithm( const QVariantMap & )
{
  addParameter( new QgsProcessingParameterFeatureSource( QStringLiteral( "INPUT" ),
                QObject::tr( "Input layer" ) ) );
  addParameter( new QgsProcessingParameterFeatureSource( QStringLiteral( "INPUT_2" ),
                QObject::tr( "Input layer 2" ) ) );

  addParameter( new QgsProcessingParameterField( QStringLiteral( "FIELDS_TO_COPY" ),
                QObject::tr( "Layer 2 fields to copy (leave empty to copy all fields)" ),
                QVariant(), QStringLiteral( "INPUT_2" ), QgsProcessingParameterField::Any,
                true, true ) );

  addParameter( new QgsProcessingParameterBoolean( QStringLiteral( "DISCARD_NONMATCHING" ),
                QObject::tr( "Discard records which could not be joined" ),
                false ) );

  addParameter( new QgsProcessingParameterString( QStringLiteral( "PREFIX" ),
                QObject::tr( "Joined field prefix" ), QVariant(), false, true ) );

  addParameter( new QgsProcessingParameterNumber( QStringLiteral( "NEIGHBORS" ),
                QObject::tr( "Maximum nearest neighbors" ), QgsProcessingParameterNumber::Integer, 1, false, 1 ) );

  addParameter( new QgsProcessingParameterDistance( QStringLiteral( "MAX_DISTANCE" ),
                QObject::tr( "Maximum distance" ), QVariant(), QStringLiteral( "INPUT" ), true, 0 ) );

  addParameter( new QgsProcessingParameterFeatureSink( QStringLiteral( "OUTPUT" ), QObject::tr( "Joined layer" ), QgsProcessing::TypeVectorAnyGeometry, QVariant(), true, true ) );

  std::unique_ptr< QgsProcessingParameterFeatureSink > nonMatchingSink = std::make_unique< QgsProcessingParameterFeatureSink >(
        QStringLiteral( "NON_MATCHING" ), QObject::tr( "Unjoinable features from first layer" ), QgsProcessing::TypeVectorAnyGeometry, QVariant(), true, false );
  // TODO GUI doesn't support advanced outputs yet
  //nonMatchingSink->setFlags(nonMatchingSink->flags() | QgsProcessingParameterDefinition::FlagAdvanced );
  addParameter( nonMatchingSink.release() );

  addOutput( new QgsProcessingOutputNumber( QStringLiteral( "JOINED_COUNT" ), QObject::tr( "Number of joined features from input table" ) ) );
  addOutput( new QgsProcessingOutputNumber( QStringLiteral( "UNJOINABLE_COUNT" ), QObject::tr( "Number of unjoinable features from input table" ) ) );
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

QgsJoinByNearestAlgorithm *QgsJoinByNearestAlgorithm::createInstance() const
{
  return new QgsJoinByNearestAlgorithm();
}

QVariantMap QgsJoinByNearestAlgorithm::processAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
{
  const int neighbors = parameterAsInt( parameters, QStringLiteral( "NEIGHBORS" ), context );
  const bool discardNonMatching = parameterAsBoolean( parameters, QStringLiteral( "DISCARD_NONMATCHING" ), context );
  const double maxDistance = parameters.value( QStringLiteral( "MAX_DISTANCE" ) ).isValid() ? parameterAsDouble( parameters, QStringLiteral( "MAX_DISTANCE" ), context ) : std::numeric_limits< double >::quiet_NaN();
  std::unique_ptr< QgsProcessingFeatureSource > input( parameterAsSource( parameters, QStringLiteral( "INPUT" ), context ) );
  if ( !input )
    throw QgsProcessingException( invalidSourceError( parameters, QStringLiteral( "INPUT" ) ) );

  std::unique_ptr< QgsProcessingFeatureSource > input2( parameterAsSource( parameters, QStringLiteral( "INPUT_2" ), context ) );
  if ( !input2 )
    throw QgsProcessingException( invalidSourceError( parameters, QStringLiteral( "INPUT_2" ) ) );

  const bool sameSourceAndTarget = parameters.value( QStringLiteral( "INPUT" ) ) == parameters.value( QStringLiteral( "INPUT_2" ) );

  const QString prefix = parameterAsString( parameters, QStringLiteral( "PREFIX" ), context );
  const QStringList fieldsToCopy = parameterAsFields( parameters, QStringLiteral( "FIELDS_TO_COPY" ), context );

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
      outFields2.rename( i, prefix + outFields2[ i ].name() );
    }
  }

  const QgsAttributeList fields2Fetch = fields2Indices;

  QgsFields outFields = QgsProcessingUtils::combineFields( input->fields(), outFields2 );

  QgsFields resultFields;
  resultFields.append( QgsField( QStringLiteral( "n" ), QVariant::Int ) );
  resultFields.append( QgsField( QStringLiteral( "distance" ), QVariant::Double ) );
  resultFields.append( QgsField( QStringLiteral( "feature_x" ), QVariant::Double ) );
  resultFields.append( QgsField( QStringLiteral( "feature_y" ), QVariant::Double ) );
  resultFields.append( QgsField( QStringLiteral( "nearest_x" ), QVariant::Double ) );
  resultFields.append( QgsField( QStringLiteral( "nearest_y" ), QVariant::Double ) );
  outFields = QgsProcessingUtils::combineFields( outFields, resultFields );

  QString dest;
  std::unique_ptr< QgsFeatureSink > sink( parameterAsSink( parameters, QStringLiteral( "OUTPUT" ), context, dest, outFields,
                                          input->wkbType(), input->sourceCrs(), QgsFeatureSink::RegeneratePrimaryKey ) );
  if ( parameters.value( QStringLiteral( "OUTPUT" ) ).isValid() && !sink )
    throw QgsProcessingException( invalidSinkError( parameters, QStringLiteral( "OUTPUT" ) ) );

  QString destNonMatching1;
  std::unique_ptr< QgsFeatureSink > sinkNonMatching1( parameterAsSink( parameters, QStringLiteral( "NON_MATCHING" ), context, destNonMatching1, input->fields(),
      input->wkbType(), input->sourceCrs(), QgsFeatureSink::RegeneratePrimaryKey ) );
  if ( parameters.value( QStringLiteral( "NON_MATCHING" ) ).isValid() && !sinkNonMatching1 )
    throw QgsProcessingException( invalidSinkError( parameters, QStringLiteral( "NON_MATCHING" ) ) );

  // make spatial index
  const QgsFeatureIterator f2 = input2->getFeatures( QgsFeatureRequest().setDestinationCrs( input->sourceCrs(), context.transformContext() ).setSubsetOfAttributes( fields2Fetch ) );
  QHash< QgsFeatureId, QgsAttributes > input2AttributeCache;
  double step = input2->featureCount() > 0 ? 50.0 / input2->featureCount() : 1;
  int i = 0;
  const QgsSpatialIndex index( f2, [&]( const QgsFeature & f )->bool
  {
    i++;
    if ( feedback->isCanceled() )
      return false;

    feedback->setProgress( i * step );

    if ( !f.hasGeometry() )
      return true;

    // only keep selected attributes
    QgsAttributes attributes;
    for ( int j = 0; j < f.attributes().count(); ++j )
    {
      if ( ! fields2Indices.contains( j ) )
        continue;
      attributes << f.attribute( j );
    }
    input2AttributeCache.insert( f.id(), attributes );

    return true;
  }, QgsSpatialIndex::FlagStoreFeatureGeometries );

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
          throw QgsProcessingException( writeFeatureError( sinkNonMatching1.get(), parameters, QStringLiteral( "NON_MATCHING" ) ) );
      }
      if ( sink && !discardNonMatching )
      {
        QgsAttributes attr = f.attributes();
        attr.append( nullMatch );
        f.setAttributes( attr );
        if ( !sink->addFeature( f, QgsFeatureSink::FastInsert ) )
          throw QgsProcessingException( writeFeatureError( sink.get(), parameters, QStringLiteral( "OUTPUT" ) ) );
      }
    }
    else
    {
      // note - if using same source as target, we have to get one extra neighbor, since the first match will be the input feature

      // if the user didn't specify a distance (isnan), then use 0 for nearestNeighbor() parameter
      // if the user specified 0 exactly, then use the smallest positive double value instead
      const double searchDistance = std::isnan( maxDistance ) ? 0 : std::max( std::numeric_limits<double>::min(), maxDistance );
      const QList< QgsFeatureId > nearest = index.nearestNeighbor( f.geometry(), neighbors + ( sameSourceAndTarget ? 1 : 0 ), searchDistance );

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
          if ( const QgsLineString *line = qgsgeometry_cast< const QgsLineString *>( closestLine.constGet() ) )
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
            throw QgsProcessingException( writeFeatureError( sink.get(), parameters, QStringLiteral( "OUTPUT" ) ) );
        }
      }
      if ( j > 0 )
        joinedCount++;
      else
      {
        if ( sinkNonMatching1 )
        {
          if ( !sinkNonMatching1->addFeature( f, QgsFeatureSink::FastInsert ) )
            throw QgsProcessingException( writeFeatureError( sinkNonMatching1.get(), parameters, QStringLiteral( "NON_MATCHING" ) ) );
        }
        if ( !discardNonMatching && sink )
        {
          QgsAttributes attr = f.attributes();
          attr.append( nullMatch );
          f.setAttributes( attr );
          if ( !sink->addFeature( f, QgsFeatureSink::FastInsert ) )
            throw QgsProcessingException( writeFeatureError( sink.get(), parameters, QStringLiteral( "OUTPUT" ) ) );
        }
        unjoinedCount++;
      }
    }
  }

  QVariantMap outputs;
  outputs.insert( QStringLiteral( "JOINED_COUNT" ), joinedCount );
  outputs.insert( QStringLiteral( "UNJOINABLE_COUNT" ), unjoinedCount );
  if ( sink )
    outputs.insert( QStringLiteral( "OUTPUT" ), dest );
  if ( sinkNonMatching1 )
    outputs.insert( QStringLiteral( "NON_MATCHING" ), destNonMatching1 );
  return outputs;
}


///@endcond
