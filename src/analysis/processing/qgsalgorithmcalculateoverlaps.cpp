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

#include "qgsdistancearea.h"
#include "qgsgeometryengine.h"
#include "qgsspatialindex.h"
#include "qgsvectorlayer.h"

///@cond PRIVATE

QString QgsCalculateVectorOverlapsAlgorithm::name() const
{
  return u"calculatevectoroverlaps"_s;
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
  return u"vectoranalysis"_s;
}

void QgsCalculateVectorOverlapsAlgorithm::initAlgorithm( const QVariantMap & )
{
  addParameter( new QgsProcessingParameterFeatureSource( u"INPUT"_s, QObject::tr( "Input layer" ), QList<int>() << static_cast<int>( Qgis::ProcessingSourceType::VectorPolygon ) ) );
  addParameter( new QgsProcessingParameterMultipleLayers( u"LAYERS"_s, QObject::tr( "Overlay layers" ), Qgis::ProcessingSourceType::VectorPolygon ) );
  addParameter( new QgsProcessingParameterFeatureSink( u"OUTPUT"_s, QObject::tr( "Overlap" ) ) );

  auto gridSize = std::make_unique<QgsProcessingParameterNumber>( u"GRID_SIZE"_s, QObject::tr( "Grid size" ), Qgis::ProcessingNumberParameterType::Double, QVariant(), true, 0 );
  gridSize->setFlags( gridSize->flags() | Qgis::ProcessingParameterFlag::Advanced );
  addParameter( gridSize.release() );
}

QIcon QgsCalculateVectorOverlapsAlgorithm::icon() const
{
  return QgsApplication::getThemeIcon( u"/algorithms/mAlgorithmClip.svg"_s );
}

QString QgsCalculateVectorOverlapsAlgorithm::svgIconPath() const
{
  return QgsApplication::iconPath( u"/algorithms/mAlgorithmClip.svg"_s );
}

QString QgsCalculateVectorOverlapsAlgorithm::shortHelpString() const
{
  return QObject::tr( "This algorithm calculates the area and percentage cover by which features from an input layer "
                      "are overlapped by features from a selection of overlay layers.\n\n"
                      "New attributes are added to the output layer reporting the total area of overlap and percentage of the input feature overlapped "
                      "by each of the selected overlay layers." );
}

QString QgsCalculateVectorOverlapsAlgorithm::shortDescription() const
{
  return QObject::tr( "Calculates the area and percentage cover by which features from an input layer "
                      "are overlapped by features from a selection of overlay layers." );
}

Qgis::ProcessingAlgorithmDocumentationFlags QgsCalculateVectorOverlapsAlgorithm::documentationFlags() const
{
  return Qgis::ProcessingAlgorithmDocumentationFlag::RespectsEllipsoid;
}

QgsCalculateVectorOverlapsAlgorithm *QgsCalculateVectorOverlapsAlgorithm::createInstance() const
{
  return new QgsCalculateVectorOverlapsAlgorithm();
}

bool QgsCalculateVectorOverlapsAlgorithm::prepareAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback * )
{
  mSource.reset( parameterAsSource( parameters, u"INPUT"_s, context ) );
  if ( !mSource )
    throw QgsProcessingException( invalidSourceError( parameters, u"INPUT"_s ) );

  mOutputFields = mSource->fields();

  const QList<QgsMapLayer *> layers = parameterAsLayerList( parameters, u"LAYERS"_s, context );
  mOverlayerSources.reserve( layers.size() );
  mLayerNames.reserve( layers.size() );
  for ( QgsMapLayer *layer : layers )
  {
    if ( QgsVectorLayer *vl = qobject_cast<QgsVectorLayer *>( layer ) )
    {
      mLayerNames << layer->name();
      mOverlayerSources.emplace_back( std::make_unique<QgsVectorLayerFeatureSource>( vl ) );
      QgsFields newFields;
      newFields.append( QgsField( u"%1_area"_s.arg( vl->name() ), QMetaType::Type::Double ) );
      newFields.append( QgsField( u"%1_pc"_s.arg( vl->name() ), QMetaType::Type::Double ) );
      mOutputFields = QgsProcessingUtils::combineFields( mOutputFields, newFields );
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
  std::unique_ptr<QgsFeatureSink> sink( parameterAsSink( parameters, u"OUTPUT"_s, context, destId, mOutputFields, mOutputType, mCrs ) );
  if ( !sink )
    throw QgsProcessingException( invalidSinkError( parameters, u"OUTPUT"_s ) );

  // build a spatial index for each constraint layer for speed. We also store input constraint geometries here,
  // to avoid refetching and projecting them later
  QList<QgsSpatialIndex> spatialIndices;
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

  QgsGeometryParameters geometryParameters;
  if ( parameters.value( u"GRID_SIZE"_s ).isValid() )
  {
    geometryParameters.setGridSize( parameterAsDouble( parameters, u"GRID_SIZE"_s, context ) );
  }

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

      double inputArea = 0;
      try
      {
        inputArea = da.measureArea( inputGeom );
      }
      catch ( QgsCsException & )
      {
        throw QgsProcessingException( QObject::tr( "An error occurred while calculating feature area" ) );
      }

      // prepare for lots of intersection tests (for speed)
      std::unique_ptr<QgsGeometryEngine> bufferGeomEngine( QgsGeometry::createGeometryEngine( inputGeom.constGet() ) );
      bufferGeomEngine->prepareGeometry();

      // calculate overlap attributes
      auto spatialIteratorIt = spatialIndices.begin();
      for ( auto it = mOverlayerSources.begin(); it != mOverlayerSources.end(); ++it, ++spatialIteratorIt )
      {
        if ( feedback->isCanceled() )
          break;

        const QgsSpatialIndex &index = *spatialIteratorIt;
        const QList<QgsFeatureId> matches = index.intersects( inputGeom.boundingBox() );
        QVector<QgsGeometry> intersectingGeoms;
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
        const QgsGeometry overlayDissolved = QgsGeometry::unaryUnion( intersectingGeoms, geometryParameters );

        if ( feedback->isCanceled() )
          break;

        const QgsGeometry overlayIntersection = inputGeom.intersection( overlayDissolved, geometryParameters );

        double overlayArea = 0;
        try
        {
          overlayArea = da.measureArea( overlayIntersection );
        }
        catch ( QgsCsException & )
        {
          throw QgsProcessingException( QObject::tr( "An error occurred while calculating feature area" ) );
        }

        outAttributes.append( overlayArea );
        outAttributes.append( 100 * overlayArea / inputArea );
      }
    }
    else
    {
      // input feature has no geometry
      for ( auto it = mOverlayerSources.begin(); it != mOverlayerSources.end(); ++it )
      {
        outAttributes.append( QVariant() );
        outAttributes.append( QVariant() );
      }
    }

    feature.setAttributes( outAttributes );
    if ( !sink->addFeature( feature, QgsFeatureSink::FastInsert ) )
      throw QgsProcessingException( writeFeatureError( sink.get(), parameters, u"OUTPUT"_s ) );

    i++;
    feedback->setProgress( i * step );
  }

  sink->finalize();

  QVariantMap outputs;
  outputs.insert( u"OUTPUT"_s, destId );
  return outputs;
}

///@endcond
