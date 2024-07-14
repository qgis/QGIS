/***************************************************************************
                         qgsalgorithmsplitwithlines.cpp
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

#include "qgsalgorithmsplitwithlines.h"
#include "qgsgeometryengine.h"
#include "qgsvectorlayer.h"
#include "qgsspatialindex.h"

///@cond PRIVATE

QString QgsSplitWithLinesAlgorithm::name() const
{
  return QStringLiteral( "splitwithlines" );
}

QString QgsSplitWithLinesAlgorithm::displayName() const
{
  return QObject::tr( "Split with lines" );
}

QStringList QgsSplitWithLinesAlgorithm::tags() const
{
  return QObject::tr( "split,cut,lines" ).split( ',' );
}

QString QgsSplitWithLinesAlgorithm::group() const
{
  return QObject::tr( "Vector overlay" );
}

QString QgsSplitWithLinesAlgorithm::groupId() const
{
  return QStringLiteral( "vectoroverlay" );
}

void QgsSplitWithLinesAlgorithm::initAlgorithm( const QVariantMap & )
{
  addParameter( new QgsProcessingParameterFeatureSource( QStringLiteral( "INPUT" ),
                QObject::tr( "Input layer" ), QList< int >() << static_cast< int >( Qgis::ProcessingSourceType::VectorLine ) << static_cast< int >( Qgis::ProcessingSourceType::VectorPolygon ) ) );
  addParameter( new QgsProcessingParameterFeatureSource( QStringLiteral( "LINES" ),
                QObject::tr( "Split layer" ), QList< int >() << static_cast< int >( Qgis::ProcessingSourceType::VectorLine ) << static_cast< int >( Qgis::ProcessingSourceType::VectorPolygon ) ) );
  addParameter( new QgsProcessingParameterFeatureSink( QStringLiteral( "OUTPUT" ), QObject::tr( "Split" ) ) );
}

QString QgsSplitWithLinesAlgorithm::shortHelpString() const
{
  return QObject::tr( "This algorithm splits the lines or polygons in one layer using the lines or polygon rings in another layer to define the breaking points. "
                      "Intersection between geometries in both layers are considered as split points." );
}

Qgis::ProcessingAlgorithmDocumentationFlags QgsSplitWithLinesAlgorithm::documentationFlags() const
{
  return Qgis::ProcessingAlgorithmDocumentationFlag::RegeneratesPrimaryKey;
}

QgsSplitWithLinesAlgorithm *QgsSplitWithLinesAlgorithm::createInstance() const
{
  return new QgsSplitWithLinesAlgorithm();
}

Qgis::ProcessingAlgorithmFlags QgsSplitWithLinesAlgorithm::flags() const
{
  Qgis::ProcessingAlgorithmFlags f = QgsProcessingAlgorithm::flags();
  f |= Qgis::ProcessingAlgorithmFlag::SupportsInPlaceEdits;
  return f;
}

bool QgsSplitWithLinesAlgorithm::supportInPlaceEdit( const QgsMapLayer *l ) const
{
  const QgsVectorLayer *layer = qobject_cast< const QgsVectorLayer * >( l );
  if ( !layer )
    return false;

  if ( layer->geometryType() != Qgis::GeometryType::Line && layer->geometryType() != Qgis::GeometryType::Polygon )
    return false;

  return true;
}

QVariantMap QgsSplitWithLinesAlgorithm::processAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
{
  std::unique_ptr< QgsFeatureSource > source( parameterAsSource( parameters, QStringLiteral( "INPUT" ), context ) );
  if ( !source )
    throw QgsProcessingException( invalidSourceError( parameters, QStringLiteral( "INPUT" ) ) );

  std::unique_ptr< QgsFeatureSource > linesSource( parameterAsSource( parameters, QStringLiteral( "LINES" ), context ) );
  if ( !linesSource )
    throw QgsProcessingException( invalidSourceError( parameters, QStringLiteral( "LINES" ) ) );

  bool sameLayer = parameters.value( QStringLiteral( "INPUT" ) ) == parameters.value( QStringLiteral( "LINES" ) );

  QString dest;
  std::unique_ptr< QgsFeatureSink > sink( parameterAsSink( parameters, QStringLiteral( "OUTPUT" ), context, dest, source->fields(),
                                          QgsWkbTypes::multiType( source->wkbType() ),  source->sourceCrs(), QgsFeatureSink::RegeneratePrimaryKey ) );
  if ( !sink )
    throw QgsProcessingException( invalidSinkError( parameters, QStringLiteral( "OUTPUT" ) ) );

  QgsFeatureRequest request;
  request.setNoAttributes();
  request.setDestinationCrs( source->sourceCrs(), context.transformContext() );

  QgsFeatureIterator splitFeatures = linesSource->getFeatures( request );
  QgsFeature aSplitFeature;

  const QgsSpatialIndex splitFeaturesIndex( splitFeatures, feedback, QgsSpatialIndex::FlagStoreFeatureGeometries );

  QgsFeature outFeat;
  QgsFeatureIterator features = source->getFeatures();

  double step = source->featureCount() > 0 ? 100.0 / source->featureCount() : 1;
  int i = 0;
  QgsFeature inFeatureA;
  while ( features.nextFeature( inFeatureA ) )
  {
    i++;
    if ( feedback->isCanceled() )
    {
      break;
    }

    if ( !inFeatureA.hasGeometry() )
    {
      if ( !sink->addFeature( inFeatureA, QgsFeatureSink::FastInsert ) )
        throw QgsProcessingException( writeFeatureError( sink.get(), parameters, QStringLiteral( "OUTPUT" ) ) );
      continue;
    }

    const QgsGeometry originalGeometry = inFeatureA.geometry();
    outFeat.setAttributes( inFeatureA.attributes() );

    QVector< QgsGeometry > inGeoms = originalGeometry.asGeometryCollection();

    const QgsFeatureIds splitFeatureCandidates = qgis::listToSet( splitFeaturesIndex.intersects( originalGeometry.boundingBox() ) );
    if ( !splitFeatureCandidates.empty() ) // has intersection of bounding boxes
    {
      QVector< QgsGeometry > splittingLines;

      // use prepared geometries for faster intersection tests
      std::unique_ptr< QgsGeometryEngine > originalGeometryEngine;

      for ( QgsFeatureId splitFeatureCandidateId : splitFeatureCandidates )
      {
        // check if trying to self-intersect
        if ( sameLayer && inFeatureA.id() == splitFeatureCandidateId )
          continue;

        const QgsGeometry splitFeatureCandidate = splitFeaturesIndex.geometry( splitFeatureCandidateId );
        if ( !originalGeometryEngine )
        {
          originalGeometryEngine.reset( QgsGeometry::createGeometryEngine( originalGeometry.constGet() ) );
          originalGeometryEngine->prepareGeometry();
        }

        if ( originalGeometryEngine->intersects( splitFeatureCandidate.constGet() ) )
        {

          QVector< QgsGeometry > splitGeomParts = splitFeatureCandidate.convertToType( Qgis::GeometryType::Line, true ).asGeometryCollection();
          splittingLines.append( splitGeomParts );
        }
      }

      if ( !splittingLines.empty() )
      {
        for ( const QgsGeometry &splitGeom : std::as_const( splittingLines ) )
        {
          QgsPointSequence splitterPList;
          QVector< QgsGeometry > outGeoms;

          // use prepared geometries for faster intersection tests
          std::unique_ptr< QgsGeometryEngine > splitGeomEngine( QgsGeometry::createGeometryEngine( splitGeom.constGet() ) );
          splitGeomEngine->prepareGeometry();
          while ( !inGeoms.empty() )
          {
            if ( feedback->isCanceled() )
            {
              break;
            }

            QgsGeometry inGeom = inGeoms.takeFirst();
            if ( inGeom.isNull() )
              continue;

            if ( splitGeomEngine->intersects( inGeom.constGet() ) )
            {
              QgsGeometry before = inGeom;
              if ( splitterPList.empty() )
              {
                const QgsCoordinateSequence sequence = splitGeom.constGet()->coordinateSequence();
                for ( const QgsRingSequence &part : sequence )
                {
                  for ( const QgsPointSequence &ring : part )
                  {
                    for ( const QgsPoint &pt : ring )
                    {
                      splitterPList << pt;
                    }
                  }
                }
              }

              QVector< QgsGeometry > newGeometries;
              QgsPointSequence topologyTestPoints;
              Qgis::GeometryOperationResult result = inGeom.splitGeometry( splitterPList, newGeometries, false, topologyTestPoints, true );

              // splitGeometry: If there are several intersections
              // between geometry and splitLine, only the first one is considered.
              if ( result == Qgis::GeometryOperationResult::Success )
              {
                // sometimes the resultant geometry has changed from the input, but only because of numerical precision issues.
                // and is effectively indistinguishable from the input. By testing the Hausdorff distance is less than this threshold
                // we are checking that the maximum "change" between the result and the input is actually significant enough to be meaningful...
                if ( inGeom.hausdorffDistance( before ) < 1e-12 )
                {
                  // effectively no change!!
                  outGeoms.append( inGeom );
                }
                else
                {
                  outGeoms.append( inGeom );
                  outGeoms.append( newGeometries );
                }
              }
              else
              {
                outGeoms.append( inGeom );
              }
            }
            else
            {
              outGeoms.append( inGeom );
            }

          }
          inGeoms = outGeoms;
        }
      }
    }

    QVector< QgsGeometry > parts;
    for ( const QgsGeometry &aGeom : std::as_const( inGeoms ) )
    {
      if ( feedback->isCanceled() )
      {
        break;
      }

      bool passed = true;
      if ( QgsWkbTypes::geometryType( aGeom.wkbType() ) == Qgis::GeometryType::Line )
      {
        int numPoints = aGeom.constGet()->nCoordinates();

        if ( numPoints <= 2 )
        {
          if ( numPoints == 2 )
            passed = !static_cast< const QgsCurve * >( aGeom.constGet() )->isClosed(); // tests if vertex 0 = vertex 1
          else
            passed = false; // sometimes splitting results in lines of zero length
        }
      }

      if ( passed )
        parts.append( aGeom );
    }

    for ( const QgsGeometry &g : parts )
    {
      outFeat.setGeometry( g );
      if ( !sink->addFeature( outFeat, QgsFeatureSink::FastInsert ) )
        throw QgsProcessingException( writeFeatureError( sink.get(), parameters, QStringLiteral( "OUTPUT" ) ) );
    }

    feedback->setProgress( i * step );
  }

  QVariantMap outputs;
  outputs.insert( QStringLiteral( "OUTPUT" ), dest );
  return outputs;
}



///@endcond


