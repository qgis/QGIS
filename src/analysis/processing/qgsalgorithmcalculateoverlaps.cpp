/***************************************************************************
                         qgsalgorithmcalculateoverlaps.cpp
                         ------------------
    begin                : May 2019
    copyright            : (C) 2019 by Nyall Dawson
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

#include "qgsalgorithmcalculateoverlaps.h"
#include "qgsvectorlayer.h"
#include "qgsgeometryengine.h"
#include "qgsdistancearea.h"

///@cond PRIVATE

QString QgsCalculateVectorOverlapsAlgorithm::name() const
{
  return QStringLiteral( "calculatevectoroverlaps" );
}

QString QgsCalculateVectorOverlapsAlgorithm::displayName() const
{
  return QObject::tr( "Overlap analysis" );
}

QStringList QgsCalculateVectorOverlapsAlgorithm::tags() const
{
  return QObject::tr( "vector,overlay,area,percentage,intersection" ).split( ',' );
}

QString QgsCalculateVectorOverlapsAlgorithm::group() const
{
  return QObject::tr( "Vector analysis" );
}

QString QgsCalculateVectorOverlapsAlgorithm::groupId() const
{
  return QStringLiteral( "vectoranalysis" );
}

void QgsCalculateVectorOverlapsAlgorithm::initAlgorithm( const QVariantMap & )
{
  addParameter( new QgsProcessingParameterFeatureSource( QStringLiteral( "INPUT" ), QObject::tr( "Input layer" ), QList< int >() << QgsProcessing::TypeVectorPolygon ) );
  addParameter( new QgsProcessingParameterMultipleLayers( QStringLiteral( "LAYERS" ), QObject::tr( "Overlay layers" ), QgsProcessing::TypeVectorPolygon ) );
  addParameter( new QgsProcessingParameterFeatureSink( QStringLiteral( "OUTPUT" ), QObject::tr( "Overlap" ) ) );
}

QIcon QgsCalculateVectorOverlapsAlgorithm::icon() const
{
  return QgsApplication::getThemeIcon( QStringLiteral( "/algorithms/mAlgorithmClip.svg" ) );
}

QString QgsCalculateVectorOverlapsAlgorithm::svgIconPath() const
{
  return QgsApplication::iconPath( QStringLiteral( "/algorithms/mAlgorithmClip.svg" ) );
}

QString QgsCalculateVectorOverlapsAlgorithm::shortHelpString() const
{
  return QObject::tr( "This algorithm calculates the area and percentage cover by which features from an input layer "
                      "are overlapped by features from a selection of overlay layers.\n\n"
                      "New attributes are added to the output layer reporting the total area of overlap and percentage of the input feature overlapped "
                      "by each of the selected overlay layers." );
}

QgsCalculateVectorOverlapsAlgorithm *QgsCalculateVectorOverlapsAlgorithm::createInstance() const
{
  return new QgsCalculateVectorOverlapsAlgorithm();
}

bool QgsCalculateVectorOverlapsAlgorithm::prepareAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback * )
{
  mSource.reset( parameterAsSource( parameters, QStringLiteral( "INPUT" ), context ) );
  if ( !mSource )
    throw QgsProcessingException( invalidSourceError( parameters, QStringLiteral( "INPUT" ) ) );

  mOutputFields = mSource->fields();

  const QList< QgsMapLayer * > layers = parameterAsLayerList( parameters, QStringLiteral( "LAYERS" ), context );
  mOverlayerSources.reserve( layers.size() );
  mLayerNames.reserve( layers.size() );
  for ( QgsMapLayer *layer : layers )
  {
    if ( QgsVectorLayer *vl = qobject_cast< QgsVectorLayer * >( layer ) )
    {
      mLayerNames << layer->name();
      mOverlayerSources.emplace_back( std::make_unique< QgsVectorLayerFeatureSource >( vl ) );
      mOutputFields.append( QgsField( QStringLiteral( "%1_area" ).arg( vl->name() ), QVariant::Double ) );
      mOutputFields.append( QgsField( QStringLiteral( "%1_pc" ).arg( vl->name() ), QVariant::Double ) );
    }
  }

  mOutputType = mSource->wkbType();
  mCrs = mSource->sourceCrs();
  mInputCount = mSource->featureCount();
  mInputFeatures = mSource->getFeatures();
  return true;
}

QVariantMap QgsCalculateVectorOverlapsAlgorithm::processAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
{
  QString destId;
  std::unique_ptr< QgsFeatureSink > sink( parameterAsSink( parameters, QStringLiteral( "OUTPUT" ), context, destId, mOutputFields,
                                          mOutputType, mCrs ) );
  if ( !sink )
    throw QgsProcessingException( invalidSinkError( parameters, QStringLiteral( "OUTPUT" ) ) );

  // build a spatial index for each constraint layer for speed. We also store input constraint geometries here,
  // to avoid refetching and projecting them later
  QList< QgsSpatialIndex > spatialIndices;
  spatialIndices.reserve( mLayerNames.size() );
  auto nameIt = mLayerNames.constBegin();
  for ( auto sourceIt = mOverlayerSources.begin(); sourceIt != mOverlayerSources.end(); ++sourceIt, ++nameIt )
  {
    feedback->pushInfo( QObject::tr( "Preparing %1" ).arg( *nameIt ) );
    const QgsFeatureIterator featureIt = ( *sourceIt )->getFeatures( QgsFeatureRequest().setSubsetOfAttributes( QgsAttributeList() ).setDestinationCrs( mCrs, context.transformContext() ).setInvalidGeometryCheck( context.invalidGeometryCheck() ).setInvalidGeometryCallback( context.invalidGeometryCallback() ) );
    spatialIndices << QgsSpatialIndex( featureIt, feedback, QgsSpatialIndex::FlagStoreFeatureGeometries );
  }

  QgsDistanceArea da;
  da.setSourceCrs( mCrs, context.transformContext() );
  da.setEllipsoid( context.ellipsoid() );

  // loop through input
  const double step = mInputCount > 0 ? 100.0 / mInputCount : 0;
  long i = 0;
  QgsFeature feature;
  while ( mInputFeatures.nextFeature( feature ) )
  {
    if ( feedback->isCanceled() )
      break;

    QgsAttributes outAttributes = feature.attributes();
    if ( feature.hasGeometry() && !qgsDoubleNear( feature.geometry().area(), 0.0 ) )
    {
      const QgsGeometry inputGeom = feature.geometry();
      const double inputArea = da.measureArea( inputGeom );

      // prepare for lots of intersection tests (for speed)
      std::unique_ptr< QgsGeometryEngine > bufferGeomEngine( QgsGeometry::createGeometryEngine( inputGeom.constGet() ) );
      bufferGeomEngine->prepareGeometry();

      // calculate overlap attributes
      auto spatialIteratorIt = spatialIndices.begin();
      for ( auto it = mOverlayerSources.begin(); it != mOverlayerSources.end(); ++ it, ++spatialIteratorIt )
      {
        if ( feedback->isCanceled() )
          break;

        const QgsSpatialIndex &index = *spatialIteratorIt;
        const QList<QgsFeatureId> matches = index.intersects( inputGeom.boundingBox() );
        QVector< QgsGeometry > intersectingGeoms;
        intersectingGeoms.reserve( matches.count() );
        for ( const QgsFeatureId match : matches )
        {
          if ( feedback->isCanceled() )
            break;

          const QgsGeometry overlayGeometry = index.geometry( match );
          if ( bufferGeomEngine->intersects( overlayGeometry.constGet() ) )
          {
            intersectingGeoms.append( overlayGeometry );
          }
        }

        if ( feedback->isCanceled() )
          break;

        // dissolve intersecting features, calculate total area of them within our buffer
        const QgsGeometry overlayDissolved = QgsGeometry::unaryUnion( intersectingGeoms );

        if ( feedback->isCanceled() )
          break;

        const QgsGeometry overlayIntersection = inputGeom.intersection( overlayDissolved );

        const double overlayArea = da.measureArea( overlayIntersection );
        outAttributes.append( overlayArea );
        outAttributes.append( 100 * overlayArea / inputArea );
      }
    }
    else
    {
      // input feature has no geometry
      for ( auto it = mOverlayerSources.begin(); it != mOverlayerSources.end(); ++ it )
      {
        outAttributes.append( QVariant() );
        outAttributes.append( QVariant() );
      }
    }

    feature.setAttributes( outAttributes );
    if ( !sink->addFeature( feature, QgsFeatureSink::FastInsert ) )
      throw QgsProcessingException( writeFeatureError( sink.get(), parameters, QStringLiteral( "OUTPUT" ) ) );

    i++;
    feedback->setProgress( i * step );
  }

  QVariantMap outputs;
  outputs.insert( QStringLiteral( "OUTPUT" ), destId );
  return outputs;
}

///@endcond
