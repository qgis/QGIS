/***************************************************************************
                         qgsalgorithmdistancematrix.cpp
                         ---------------------
    begin                : May 2026
    copyright            : (C) 2026 by Alexander Bruy
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

#include "qgsalgorithmdistancematrix.h"

#include "qgsspatialindex.h"

#include <QString>

using namespace Qt::StringLiterals;

///@cond PRIVATE

QString QgsDistanceMatrixAlgorithm::name() const
{
  return u"distancematrix"_s;
}

QString QgsDistanceMatrixAlgorithm::displayName() const
{
  return QObject::tr( "Distance matrix" );
}

QStringList QgsDistanceMatrixAlgorithm::tags() const
{
  return QObject::tr( "point,distance,matrix,nearest,closest,summary" ).split( ',' );
}

QString QgsDistanceMatrixAlgorithm::group() const
{
  return QObject::tr( "Vector analysis" );
}

QString QgsDistanceMatrixAlgorithm::groupId() const
{
  return u"vectoranalysis"_s;
}

QString QgsDistanceMatrixAlgorithm::shortHelpString() const
{
  return QObject::tr(
    "This algorithm creates a table containing a distance matrix, "
    "with distances between all the points in a points layer.\n\n"
    "This algorithm uses purely Cartesian calculations for distance, "
    "and does not consider geodetic or ellipsoid properties when "
    "determining feature proximity. The measurement and output coordinate "
    "system is based on the coordinate system of the source layer."
  );
}

QString QgsDistanceMatrixAlgorithm::shortDescription() const
{
  return QObject::tr( "Creates a table containing a matrix of distances between all the points in a points layer." );
}

QgsDistanceMatrixAlgorithm *QgsDistanceMatrixAlgorithm::createInstance() const
{
  return new QgsDistanceMatrixAlgorithm();
}

void QgsDistanceMatrixAlgorithm::initAlgorithm( const QVariantMap & )
{
  auto inputParam = std::make_unique<QgsProcessingParameterFeatureSource>( u"INPUT"_s, QObject::tr( "Input point layer" ), QList<int>() << static_cast<int>( Qgis::ProcessingSourceType::VectorPoint ) );
  inputParam->setHelp( QObject::tr( "The layer containing the points from which distances will be calculated." ) );
  addParameter( inputParam.release() );
  auto inputFieldParam = std::make_unique<QgsProcessingParameterField>( u"INPUT_FIELD"_s, QObject::tr( "Input unique ID field" ), QVariant(), u"INPUT"_s );
  inputFieldParam->setHelp( QObject::tr( "A field of the input layer with unique values to identify each starting point in the output table." ) );
  addParameter( inputFieldParam.release() );
  auto targetParam = std::make_unique<QgsProcessingParameterFeatureSource>( u"TARGET"_s, QObject::tr( "Target point layer" ), QList<int>() << static_cast<int>( Qgis::ProcessingSourceType::VectorPoint ) );
  targetParam->setHelp( QObject::tr( "The layer containing the points to which distances will be measured. If the same as the input layer, distances between points in the same layer are calculated." ) );
  addParameter( targetParam.release() );
  auto targetFieldParam = std::make_unique<QgsProcessingParameterField>( u"TARGET_FIELD"_s, QObject::tr( "Target unique ID field" ), QVariant(), u"TARGET"_s );
  targetFieldParam->setHelp( QObject::tr( "A field of the target layer with unique values to identify each destination point in the output table." ) );
  addParameter( targetFieldParam.release() );
  auto matrixTypeParam = std::make_unique<QgsProcessingParameterEnum>(
    u"MATRIX_TYPE"_s,
    QObject::tr( "Output matrix type" ),
    QStringList() << QObject::tr( "Linear (N*k x 3) distance matrix" ) << QObject::tr( "Standard (N x T) distance matrix" ) << QObject::tr( "Summary distance matrix (mean, std. dev., min, max)" ),
    false,
    0
  );
  matrixTypeParam->setHelp(
    QObject::tr(
      "The format of the output table. Linear for a list of point pairs and their distance; "
      "standard for one row per input point with columns for each target and distance; "
      "summary for statistics (mean, min, max, etc.) for each input point."
    )
  );
  addParameter( matrixTypeParam.release() );
  auto pointsParam
    = std::make_unique<QgsProcessingParameterNumber>( u"NEAREST_POINTS"_s, QObject::tr( "Use only the nearest (k) target points" ), Qgis::ProcessingNumberParameterType::Integer, 0, false, 0 );
  pointsParam->setHelp( QObject::tr( "Limit the calculation to a specific number of the closest target points. If set to 0, distances to all target points will be calculated." ) );
  addParameter( pointsParam.release() );
  addParameter( new QgsProcessingParameterFeatureSink( u"OUTPUT"_s, QObject::tr( "Distance matrix" ), Qgis::ProcessingSourceType::VectorPoint, QVariant() ) );
}

bool QgsDistanceMatrixAlgorithm::prepareAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback * )
{
  mSource.reset( parameterAsSource( parameters, u"INPUT"_s, context ) );
  if ( !mSource )
  {
    throw QgsProcessingException( invalidSourceError( parameters, u"INPUT"_s ) );
  }

  if ( QgsWkbTypes::isMultiType( mSource->wkbType() ) )
  {
    throw QgsProcessingException( QObject::tr( "Input point layer is a MultiPoint layer - convert to single points before using this algorithm." ) );
  }

  mTarget.reset( parameterAsSource( parameters, u"TARGET"_s, context ) );
  if ( !mTarget )
  {
    throw QgsProcessingException( invalidSourceError( parameters, u"TARGET"_s ) );
  }

  if ( QgsWkbTypes::isMultiType( mTarget->wkbType() ) )
  {
    throw QgsProcessingException( QObject::tr( "Target point layer is a MultiPoint layer - convert to single points before using this algorithm." ) );
  }

  mSourceField = parameterAsString( parameters, u"INPUT_FIELD"_s, context );
  mTargetField = parameterAsString( parameters, u"TARGET_FIELD"_s, context );
  mMatrixType = static_cast<MatrixType>( parameterAsEnum( parameters, u"MATRIX_TYPE"_s, context ) );
  mSameLayer = ( parameters.value( u"INPUT"_s ) == parameters.value( u"TARGET"_s ) );

  mKPoints = parameterAsInt( parameters, u"NEAREST_POINTS"_s, context );
  if ( mKPoints < 1 )
  {
    mKPoints = static_cast<int>( mTarget->featureCount() );
  }

  return true;
}

QVariantMap QgsDistanceMatrixAlgorithm::processAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
{
  switch ( mMatrixType )
  {
    case Linear:
    case Summary:
      return linearMatrix( parameters, context, feedback );

    case Standard:
      return regularMatrix( parameters, context, feedback );
  }

  return {};
}

QVariantMap QgsDistanceMatrixAlgorithm::linearMatrix( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
{
  // when using the same layer we need to fetch an extra point from the index, since the closest match will always be the same as the input feature
  long long nPoints = mSameLayer ? mKPoints + 1 : mKPoints;

  const int sourceIndex = mSource->fields().lookupField( mSourceField );
  const int targetIndex = mTarget->fields().lookupField( mTargetField );

  QgsFields fields;
  QgsField inputIdField( mSource->fields().at( sourceIndex ) );
  inputIdField.setName( u"InputID"_s );
  fields.append( inputIdField );

  if ( mMatrixType == Linear )
  {
    QgsField targetIdField( mTarget->fields().at( targetIndex ) );
    targetIdField.setName( u"TargetID"_s );
    fields.append( targetIdField );
    fields.append( QgsField( u"Distance"_s, QMetaType::Type::Double ) );
  }
  else
  {
    fields.append( QgsField( u"MEAN"_s, QMetaType::Type::Double ) );
    fields.append( QgsField( u"STDDEV"_s, QMetaType::Type::Double ) );
    fields.append( QgsField( u"MIN"_s, QMetaType::Type::Double ) );
    fields.append( QgsField( u"MAX"_s, QMetaType::Type::Double ) );
  }

  const Qgis::WkbType outputWkbType = ( mMatrixType == Linear ) ? QgsWkbTypes::multiType( mSource->wkbType() ) : mSource->wkbType();

  QString dest;
  std::unique_ptr<QgsFeatureSink> sink( parameterAsSink( parameters, u"OUTPUT"_s, context, dest, fields, outputWkbType, mSource->sourceCrs() ) );
  if ( !sink )
  {
    throw QgsProcessingException( invalidSinkError( parameters, u"OUTPUT"_s ) );
  }

  const QgsFeatureIterator targetIterator = mTarget->getFeatures( QgsFeatureRequest().setSubsetOfAttributes( { targetIndex } ).setDestinationCrs( mSource->sourceCrs(), context.transformContext() ) );
  QHash<QgsFeatureId, QVariant> targetIdCache;
  double step = mTarget->featureCount() > 0 ? 50.0 / mTarget->featureCount() : 1;
  long long current = 0;
  const QgsSpatialIndex index(
    targetIterator,
    [&]( const QgsFeature &f ) -> bool {
      if ( feedback->isCanceled() )
      {
        return false;
      }

      targetIdCache.insert( f.id(), f.attribute( targetIndex ) );
      feedback->setProgress( static_cast<double>( current ) * step );
      current++;

      return true;
    },
    QgsSpatialIndex::FlagStoreFeatureGeometries
  );

  QgsDistanceArea da;
  da.setSourceCrs( mSource->sourceCrs(), context.transformContext() );

  double distance = 0;
  QVector<double> distancesList;
  distancesList.reserve( nPoints );

  current = 0;
  step = mSource->featureCount() > 0 ? 50.0 / static_cast<double>( mSource->featureCount() ) : 0;
  QgsFeatureIterator features = mSource->getFeatures( QgsFeatureRequest().setSubsetOfAttributes( { sourceIndex } ) );
  QgsFeature sourceFeature;

  while ( features.nextFeature( sourceFeature ) )
  {
    if ( feedback->isCanceled() )
    {
      break;
    }

    distancesList.clear();
    const QgsPointXY sourcePoint = sourceFeature.geometry().asPoint();
    const QString sourceId = sourceFeature.attribute( sourceIndex ).toString();
    const QList<QgsFeatureId> nearestIds = index.nearestNeighbor( sourcePoint, nPoints );

    if ( mMatrixType == Linear )
    {
      for ( const QgsFeatureId targetId : nearestIds )
      {
        if ( feedback->isCanceled() )
        {
          break;
        }

        if ( mSameLayer && sourceFeature.id() == targetId )
        {
          continue;
        }

        QgsPointXY targetPoint = index.geometry( targetId ).asPoint();
        distance = da.measureLine( sourcePoint, targetPoint );

        QgsFeature f;
        f.setGeometry( QgsGeometry::unaryUnion( { sourceFeature.geometry(), index.geometry( targetId ) } ) );
        f.setAttributes( QgsAttributes() << sourceId << targetIdCache.value( targetId ) << distance );
        if ( !sink->addFeature( f, QgsFeatureSink::FastInsert ) )
        {
          throw QgsProcessingException( writeFeatureError( sink.get(), parameters, u"OUTPUT"_s ) );
        }
      }
    }
    else // Summary
    {
      for ( const QgsFeatureId targetId : nearestIds )
      {
        if ( feedback->isCanceled() )
        {
          break;
        }

        if ( mSameLayer && sourceFeature.id() == targetId )
        {
          continue;
        }

        QgsPointXY targetPoint = index.geometry( targetId ).asPoint();
        distancesList << da.measureLine( sourcePoint, targetPoint );
      }

      QgsFeature f;
      f.setGeometry( sourceFeature.geometry() );
      if ( distancesList.isEmpty() )
      {
        f.setAttributes( QgsAttributes() << sourceId << QVariant() << QVariant() << QVariant() << QVariant() );
      }
      else
      {
        double sum = 0;
        double sumSquares = 0;
        double minDistance = std::numeric_limits<double>::max();
        double maxDistance = std::numeric_limits<double>::lowest();

        for ( const double d : std::as_const( distancesList ) )
        {
          sum += d;
          sumSquares += d * d;
          minDistance = std::min( minDistance, d );
          maxDistance = std::max( maxDistance, d );
        }

        const long long n = distancesList.size();
        const double mean = sum / static_cast<double>( n );
        const double variance = std::max( 0.0, ( sumSquares / static_cast<double>( n ) ) - ( mean * mean ) );
        const double stdDev = std::sqrt( variance );

        f.setAttributes( QgsAttributes() << sourceId << mean << stdDev << minDistance << maxDistance );
      }

      if ( !sink->addFeature( f, QgsFeatureSink::FastInsert ) )
      {
        throw QgsProcessingException( writeFeatureError( sink.get(), parameters, u"OUTPUT"_s ) );
      }
    }

    feedback->setProgress( 50.0 + static_cast<double>( current ) * step );
    current++;
  }

  sink->finalize();

  QVariantMap outputs;
  outputs.insert( u"OUTPUT"_s, dest );
  return outputs;
}

QVariantMap QgsDistanceMatrixAlgorithm::regularMatrix( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
{
  // when using the same layer we need to fetch an extra point from the index, since the closest match will always be the same as the input feature,
  // however if all features are requested we do not need to fetch more features
  long long nPoints = mSameLayer ? mKPoints == mTarget->featureCount() ? mKPoints : mKPoints + 1 : mKPoints;

  const int sourceIndex = mSource->fields().lookupField( mSourceField );
  const int targetIndex = mTarget->fields().lookupField( mTargetField );

  QgsFields fields;
  QgsField inputIdField( mSource->fields().at( sourceIndex ) );
  inputIdField.setName( u"InputID"_s );
  fields.append( inputIdField );

  QMetaType::Type targetIdType = mTarget->fields().at( targetIndex ).type();
  // creating nPoints - 1 fields, as we do not take into account the input feature
  for ( long long i = 0; i < nPoints - 1; i++ )
  {
    fields.append( QgsField( u"TargetID_%1"_s.arg( i + 1 ), targetIdType ) );
    fields.append( QgsField( u"Dist_%1"_s.arg( i + 1 ), QMetaType::Type::Double ) );
  }

  QString dest;
  std::unique_ptr<QgsFeatureSink> sink( parameterAsSink( parameters, u"OUTPUT"_s, context, dest, fields, mSource->wkbType(), mSource->sourceCrs() ) );
  if ( !sink )
  {
    throw QgsProcessingException( invalidSinkError( parameters, u"OUTPUT"_s ) );
  }

  const QgsFeatureIterator targetIterator = mTarget->getFeatures( QgsFeatureRequest().setSubsetOfAttributes( { targetIndex } ).setDestinationCrs( mSource->sourceCrs(), context.transformContext() ) );
  QHash<QgsFeatureId, QVariant> targetIdCache;
  double step = mTarget->featureCount() > 0 ? 50.0 / mTarget->featureCount() : 1;
  long long current = 0;
  const QgsSpatialIndex index(
    targetIterator,
    [&]( const QgsFeature &f ) -> bool {
      if ( feedback->isCanceled() )
      {
        return false;
      }
      targetIdCache.insert( f.id(), f.attribute( targetIndex ) );

      feedback->setProgress( static_cast<double>( current ) * step );
      current++;

      return true;
    },
    QgsSpatialIndex::FlagStoreFeatureGeometries
  );

  QgsDistanceArea da;
  da.setSourceCrs( mSource->sourceCrs(), context.transformContext() );
  double distance = 0;

  current = 0;
  step = mSource->featureCount() > 0 ? 50.0 / static_cast<double>( mSource->featureCount() ) : 0;
  QgsFeatureIterator features = mSource->getFeatures( QgsFeatureRequest().setSubsetOfAttributes( { sourceIndex } ) );
  QgsFeature sourceFeature;

  while ( features.nextFeature( sourceFeature ) )
  {
    if ( feedback->isCanceled() )
    {
      break;
    }

    const QgsPointXY sourcePoint = sourceFeature.geometry().asPoint();
    const QString sourceId = sourceFeature.attribute( sourceIndex ).toString();
    const QList<QgsFeatureId> nearestIds = index.nearestNeighbor( sourcePoint, nPoints );

    QgsAttributes attrs;
    attrs.reserve( 1 + nPoints * 2 );
    attrs << sourceId;
    for ( const QgsFeatureId targetId : nearestIds )
    {
      if ( feedback->isCanceled() )
      {
        break;
      }

      if ( mSameLayer && sourceFeature.id() == targetId )
      {
        continue;
      }

      QgsPointXY targetPoint = index.geometry( targetId ).asPoint();
      distance = da.measureLine( sourcePoint, targetPoint );
      attrs << targetIdCache.value( targetId ) << distance;
    }

    QgsFeature f;
    f.setGeometry( sourceFeature.geometry() );
    f.setAttributes( attrs );
    if ( !sink->addFeature( f, QgsFeatureSink::FastInsert ) )
    {
      throw QgsProcessingException( writeFeatureError( sink.get(), parameters, u"OUTPUT"_s ) );
    }

    feedback->setProgress( 50.0 + static_cast<double>( current ) * step );
    current++;
  }

  sink->finalize();

  QVariantMap outputs;
  outputs.insert( u"OUTPUT"_s, dest );
  return outputs;
}

///@endcond
