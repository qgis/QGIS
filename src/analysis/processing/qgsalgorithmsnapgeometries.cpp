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
  return QStringLiteral( "snapgeometries" );
}

QString QgsSnapGeometriesAlgorithm::displayName() const
{
  return QObject::tr( "Snap geometries to layer" );
}

QString QgsSnapGeometriesAlgorithm::shortHelpString() const
{
  return QObject::tr( "Snaps the geometries in a layer. Snapping can be done either to the geometries "
                      "from another layer, or to geometries within the same layer." )
         + QStringLiteral( "\n\n" )
         + QObject::tr( "A tolerance is specified in layer units to control how close vertices need "
                        "to be to the reference layer geometries before they are snapped." )
         + QStringLiteral( "\n\n" )
         + QObject::tr( "Snapping occurs to both nodes and edges. Depending on the snapping behavior, "
                        "either nodes or edges will be preferred." )
         + QStringLiteral( "\n\n" )
         + QObject::tr( "Vertices will be inserted or removed as required to make the geometries match "
                        "the reference geometries." );
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
  return QStringLiteral( "vectorgeometry" );
}

QgsProcessingAlgorithm::Flags QgsSnapGeometriesAlgorithm::flags() const
{
  Flags f = QgsProcessingAlgorithm::flags();
  f |= QgsProcessingAlgorithm::FlagSupportsInPlaceEdits;
  return f;
}

bool QgsSnapGeometriesAlgorithm::supportInPlaceEdit( const QgsMapLayer *l ) const
{
  const QgsVectorLayer *layer = qobject_cast< const QgsVectorLayer * >( l );
  if ( !layer )
    return false;

  return layer->isSpatial();
}

void QgsSnapGeometriesAlgorithm::initAlgorithm( const QVariantMap & )
{
  addParameter( new QgsProcessingParameterFeatureSource( QStringLiteral( "INPUT" ), QObject::tr( "Input layer" ),
                QList< int >() << QgsProcessing::TypeVectorPoint << QgsProcessing::TypeVectorLine << QgsProcessing::TypeVectorPolygon ) );
  addParameter( new QgsProcessingParameterFeatureSource( QStringLiteral( "REFERENCE_LAYER" ), QObject::tr( "Reference layer" ),
                QList< int >() << QgsProcessing::TypeVectorPoint << QgsProcessing::TypeVectorLine << QgsProcessing::TypeVectorPolygon ) );
  addParameter( new QgsProcessingParameterDistance( QStringLiteral( "TOLERANCE" ), QObject::tr( "Tolerance" ),
                10.0, QStringLiteral( "INPUT" ), false, 0.00000001 ) );

  const QStringList options = QStringList()
                              << QObject::tr( "Prefer aligning nodes, insert extra vertices where required" )
                              << QObject::tr( "Prefer closest point, insert extra vertices where required" )
                              << QObject::tr( "Prefer aligning nodes, don't insert new vertices" )
                              << QObject::tr( "Prefer closest point, don't insert new vertices" )
                              << QObject::tr( "Move end points only, prefer aligning nodes" )
                              << QObject::tr( "Move end points only, prefer closest point" )
                              << QObject::tr( "Snap end points to end points only" )
                              << QObject::tr( "Snap to anchor nodes (single layer only)" );
  addParameter( new QgsProcessingParameterEnum( QStringLiteral( "BEHAVIOR" ), QObject::tr( "Behavior" ), options, false, QVariantList() << 0 ) );
  addParameter( new QgsProcessingParameterFeatureSink( QStringLiteral( "OUTPUT" ),
                QObject::tr( "Snapped geometry" ), QgsProcessing::TypeVectorAnyGeometry ) );
}

QgsSnapGeometriesAlgorithm *QgsSnapGeometriesAlgorithm::createInstance() const
{
  return new QgsSnapGeometriesAlgorithm();
}

QVariantMap QgsSnapGeometriesAlgorithm::processAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
{
  std::unique_ptr< QgsProcessingFeatureSource > source( parameterAsSource( parameters, QStringLiteral( "INPUT" ), context ) );
  if ( !source )
    throw QgsProcessingException( invalidSourceError( parameters, QStringLiteral( "INPUT" ) ) );

  const std::unique_ptr< QgsProcessingFeatureSource > referenceSource( parameterAsSource( parameters, QStringLiteral( "REFERENCE_LAYER" ), context ) );
  if ( !referenceSource )
    throw QgsProcessingException( invalidSourceError( parameters, QStringLiteral( "REFERENCE_LAYER" ) ) );

  const double tolerance = parameterAsDouble( parameters, QStringLiteral( "TOLERANCE" ), context );
  const QgsGeometrySnapper::SnapMode mode = static_cast<QgsGeometrySnapper::SnapMode>( parameterAsEnum( parameters, QStringLiteral( "BEHAVIOR" ), context ) );

  QString dest;
  std::unique_ptr< QgsFeatureSink > sink( parameterAsSink( parameters, QStringLiteral( "OUTPUT" ), context, dest, source->fields(), source->wkbType(), source->sourceCrs() ) );
  if ( !sink )
    throw QgsProcessingException( invalidSinkError( parameters, QStringLiteral( "OUTPUT" ) ) );

  QgsFeatureIterator features = source->getFeatures();

  if ( parameters.value( QStringLiteral( "INPUT" ) ) != parameters.value( QStringLiteral( "REFERENCE_LAYER" ) ) )
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
          throw QgsProcessingException( writeFeatureError( sink.get(), parameters, QStringLiteral( "OUTPUT" ) ) );
      }
      else
      {
        if ( !sink->addFeature( f ) )
          throw QgsProcessingException( writeFeatureError( sink.get(), parameters, QStringLiteral( "OUTPUT" ) ) );
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
          throw QgsProcessingException( writeFeatureError( sink.get(), parameters, QStringLiteral( "OUTPUT" ) ) );

        feedback->setProgress( processed * step );
      }
    }
  }

  QVariantMap outputs;
  outputs.insert( QStringLiteral( "OUTPUT" ), dest );
  return outputs;
}

///@endcond
