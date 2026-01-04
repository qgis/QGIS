/***************************************************************************
                         qgsalgorithmsnapgeometries.cpp
                         ---------------------
    begin                : May 2020
    copyright            : (C) 2020 by Alexander Bruy
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

#include "qgsalgorithmsnapgeometries.h"

#include "qgsvectorlayer.h"
#include "vector/qgsgeometrysnapper.h"
#include "vector/qgsgeometrysnappersinglesource.h"

///@cond PRIVATE

QString QgsSnapGeometriesAlgorithm::name() const
{
  return u"snapgeometries"_s;
}

QString QgsSnapGeometriesAlgorithm::displayName() const
{
  return QObject::tr( "Snap geometries to layer" );
}

QString QgsSnapGeometriesAlgorithm::shortHelpString() const
{
  return QObject::tr( "This algorithm snaps the geometries in a layer. Snapping can be done either to the geometries "
                      "from another layer, or to geometries within the same layer." )
         + u"\n\n"_s
         + QObject::tr( "A tolerance is specified in layer units to control how close vertices need "
                        "to be to the reference layer geometries before they are snapped." )
         + u"\n\n"_s
         + QObject::tr( "Snapping occurs to both nodes and edges. Depending on the snapping behavior, "
                        "either nodes or edges will be preferred." )
         + u"\n\n"_s
         + QObject::tr( "Vertices will be inserted or removed as required to make the geometries match "
                        "the reference geometries." );
}

QString QgsSnapGeometriesAlgorithm::shortDescription() const
{
  return QObject::tr( "Snaps the geometries to the geometries within the same layer or from another layer." );
}

QStringList QgsSnapGeometriesAlgorithm::tags() const
{
  return QObject::tr( "geometry,snap,tolerance" ).split( ',' );
}

QString QgsSnapGeometriesAlgorithm::group() const
{
  return QObject::tr( "Vector geometry" );
}

QString QgsSnapGeometriesAlgorithm::groupId() const
{
  return u"vectorgeometry"_s;
}

Qgis::ProcessingAlgorithmFlags QgsSnapGeometriesAlgorithm::flags() const
{
  Qgis::ProcessingAlgorithmFlags f = QgsProcessingAlgorithm::flags();
  f |= Qgis::ProcessingAlgorithmFlag::SupportsInPlaceEdits;
  return f;
}

bool QgsSnapGeometriesAlgorithm::supportInPlaceEdit( const QgsMapLayer *l ) const
{
  const QgsVectorLayer *layer = qobject_cast<const QgsVectorLayer *>( l );
  if ( !layer )
    return false;

  return layer->isSpatial();
}

void QgsSnapGeometriesAlgorithm::initAlgorithm( const QVariantMap & )
{
  addParameter( new QgsProcessingParameterFeatureSource( u"INPUT"_s, QObject::tr( "Input layer" ), QList<int>() << static_cast<int>( Qgis::ProcessingSourceType::VectorPoint ) << static_cast<int>( Qgis::ProcessingSourceType::VectorLine ) << static_cast<int>( Qgis::ProcessingSourceType::VectorPolygon ) ) );
  addParameter( new QgsProcessingParameterFeatureSource( u"REFERENCE_LAYER"_s, QObject::tr( "Reference layer" ), QList<int>() << static_cast<int>( Qgis::ProcessingSourceType::VectorPoint ) << static_cast<int>( Qgis::ProcessingSourceType::VectorLine ) << static_cast<int>( Qgis::ProcessingSourceType::VectorPolygon ) ) );

  auto tolParam = std::make_unique<QgsProcessingParameterDistance>( u"TOLERANCE"_s, QObject::tr( "Tolerance" ), 10.0, u"INPUT"_s, false, 0.00000001 );
  tolParam->setMetadata(
    { QVariantMap( { { u"widget_wrapper"_s, QVariantMap( { { u"decimals"_s, 8 } } ) } } )
    }
  );
  addParameter( tolParam.release() );

  const QStringList options = QStringList()
                              << QObject::tr( "Prefer aligning nodes, insert extra vertices where required" )
                              << QObject::tr( "Prefer closest point, insert extra vertices where required" )
                              << QObject::tr( "Prefer aligning nodes, don't insert new vertices" )
                              << QObject::tr( "Prefer closest point, don't insert new vertices" )
                              << QObject::tr( "Move end points only, prefer aligning nodes" )
                              << QObject::tr( "Move end points only, prefer closest point" )
                              << QObject::tr( "Snap end points to end points only" )
                              << QObject::tr( "Snap to anchor nodes (single layer only)" );
  addParameter( new QgsProcessingParameterEnum( u"BEHAVIOR"_s, QObject::tr( "Behavior" ), options, false, QVariantList() << 0 ) );
  addParameter( new QgsProcessingParameterFeatureSink( u"OUTPUT"_s, QObject::tr( "Snapped geometry" ), Qgis::ProcessingSourceType::VectorAnyGeometry ) );
}

QgsSnapGeometriesAlgorithm *QgsSnapGeometriesAlgorithm::createInstance() const
{
  return new QgsSnapGeometriesAlgorithm();
}

QVariantMap QgsSnapGeometriesAlgorithm::processAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
{
  std::unique_ptr<QgsProcessingFeatureSource> source( parameterAsSource( parameters, u"INPUT"_s, context ) );
  if ( !source )
    throw QgsProcessingException( invalidSourceError( parameters, u"INPUT"_s ) );

  const std::unique_ptr<QgsProcessingFeatureSource> referenceSource( parameterAsSource( parameters, u"REFERENCE_LAYER"_s, context ) );
  if ( !referenceSource )
    throw QgsProcessingException( invalidSourceError( parameters, u"REFERENCE_LAYER"_s ) );

  const double tolerance = parameterAsDouble( parameters, u"TOLERANCE"_s, context );
  const QgsGeometrySnapper::SnapMode mode = static_cast<QgsGeometrySnapper::SnapMode>( parameterAsEnum( parameters, u"BEHAVIOR"_s, context ) );

  QString dest;
  std::unique_ptr<QgsFeatureSink> sink( parameterAsSink( parameters, u"OUTPUT"_s, context, dest, source->fields(), source->wkbType(), source->sourceCrs() ) );
  if ( !sink )
    throw QgsProcessingException( invalidSinkError( parameters, u"OUTPUT"_s ) );

  QgsFeatureIterator features = source->getFeatures();

  if ( parameters.value( u"INPUT"_s ) != parameters.value( u"REFERENCE_LAYER"_s ) )
  {
    const double step = source->featureCount() > 0 ? 100.0 / source->featureCount() : 1;
    if ( mode == 7 )
      throw QgsProcessingException( QObject::tr( "This mode applies when the input and reference layer are the same." ) );

    const QgsGeometrySnapper snapper( referenceSource.get() );
    long long processed = 0;
    QgsFeature f;
    while ( features.nextFeature( f ) )
    {
      if ( feedback->isCanceled() )
        break;

      if ( f.hasGeometry() )
      {
        QgsFeature outFeature( f );
        outFeature.setGeometry( snapper.snapGeometry( f.geometry(), tolerance, mode ) );
        if ( !sink->addFeature( outFeature, QgsFeatureSink::FastInsert ) )
          throw QgsProcessingException( writeFeatureError( sink.get(), parameters, u"OUTPUT"_s ) );
      }
      else
      {
        if ( !sink->addFeature( f ) )
          throw QgsProcessingException( writeFeatureError( sink.get(), parameters, u"OUTPUT"_s ) );
      }
      processed += 1;
      feedback->setProgress( processed * step );
    }
  }
  else if ( mode == 7 )
  {
    // input layer == reference layer
    const int modified = QgsGeometrySnapperSingleSource::run( *source, *sink, tolerance, feedback );
    feedback->pushInfo( QObject::tr( "Snapped %n geometries.", nullptr, modified ) );
  }
  else
  {
    // snapping internally
    const double step = source->featureCount() > 0 ? 100.0 / ( source->featureCount() * 2 ) : 1;
    long long processed = 0;

    QgsInternalGeometrySnapper snapper( tolerance, mode );
    QgsFeature f;
    QList<QgsFeatureId> editedFeatureIds;
    QMap<QgsFeatureId, QgsFeature> editedFeatures;
    while ( features.nextFeature( f ) )
    {
      if ( feedback->isCanceled() )
        break;

      QgsFeature editedFeature( f );
      if ( f.hasGeometry() )
      {
        editedFeature.setGeometry( snapper.snapFeature( f ) );
      }
      editedFeatureIds << editedFeature.id();
      editedFeatures.insert( editedFeature.id(), editedFeature );
      processed += 1;
    }

    // reversed order snapping round is required to insure geometries are snapped against all features
    snapper = QgsInternalGeometrySnapper( tolerance, mode );
    std::reverse( editedFeatureIds.begin(), editedFeatureIds.end() );
    for ( const QgsFeatureId &fid : std::as_const( editedFeatureIds ) )
    {
      if ( feedback->isCanceled() )
        break;

      QgsFeature editedFeature( editedFeatures.value( fid ) );
      if ( editedFeature.hasGeometry() )
      {
        editedFeature.setGeometry( snapper.snapFeature( editedFeature ) );
        editedFeatures.insert( editedFeature.id(), editedFeature );
      }
    }
    std::reverse( editedFeatureIds.begin(), editedFeatureIds.end() );
    processed += 1;

    if ( !feedback->isCanceled() )
    {
      for ( const QgsFeatureId &fid : std::as_const( editedFeatureIds ) )
      {
        QgsFeature outFeature( editedFeatures.value( fid ) );
        if ( !sink->addFeature( outFeature ) )
          throw QgsProcessingException( writeFeatureError( sink.get(), parameters, u"OUTPUT"_s ) );

        feedback->setProgress( processed * step );
      }
    }
  }

  sink->finalize();

  QVariantMap outputs;
  outputs.insert( u"OUTPUT"_s, dest );
  return outputs;
}

///@endcond
