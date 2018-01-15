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

QgsProcessingAlgorithm::Flags QgsSplitWithLinesAlgorithm::flags() const
{
  return QgsProcessingAlgorithm::flags() | QgsProcessingAlgorithm::FlagCanRunInBackground;
}

void QgsSplitWithLinesAlgorithm::initAlgorithm( const QVariantMap & )
{
  addParameter( new QgsProcessingParameterFeatureSource( QStringLiteral( "INPUT" ),
                QObject::tr( "Input layer" ), QList< int >() << QgsProcessing::TypeVectorLine << QgsProcessing::TypeVectorPolygon ) );
  addParameter( new QgsProcessingParameterFeatureSource( QStringLiteral( "LINES" ),
                QObject::tr( "Split layer" ), QList< int >() << QgsProcessing::TypeVectorLine ) );
  addParameter( new QgsProcessingParameterFeatureSink( QStringLiteral( "OUTPUT" ), QObject::tr( "Split" ) ) );
}

QString QgsSplitWithLinesAlgorithm::shortHelpString() const
{
  return QObject::tr( "This algorithm splits the lines or polygons in one layer using the lines in another layer to define the breaking points. "
                      "Intersection between geometries in both layers are considered as split points." );
}

QgsSplitWithLinesAlgorithm *QgsSplitWithLinesAlgorithm::createInstance() const
{
  return new QgsSplitWithLinesAlgorithm();
}

QVariantMap QgsSplitWithLinesAlgorithm::processAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
{
  std::unique_ptr< QgsFeatureSource > source( parameterAsSource( parameters, QStringLiteral( "INPUT" ), context ) );
  if ( !source )
    return QVariantMap();

  std::unique_ptr< QgsFeatureSource > linesSource( parameterAsSource( parameters, QStringLiteral( "LINES" ), context ) );
  if ( !linesSource )
    return QVariantMap();

  bool sameLayer = parameters.value( QStringLiteral( "INPUT" ) ) == parameters.value( QStringLiteral( "LINES" ) );

  QString dest;
  std::unique_ptr< QgsFeatureSink > sink( parameterAsSink( parameters, QStringLiteral( "OUTPUT" ), context, dest, source->fields(),
                                          QgsWkbTypes::multiType( source->wkbType() ),  source->sourceCrs() ) );
  if ( !sink )
    return QVariantMap();

  QgsSpatialIndex spatialIndex;
  QMap< QgsFeatureId, QgsGeometry > splitGeoms;
  QgsFeatureRequest request;
  request.setSubsetOfAttributes( QgsAttributeList() );
  request.setDestinationCrs( source->sourceCrs(), context.transformContext() );

  QgsFeatureIterator splitLines = linesSource->getFeatures( request );
  QgsFeature aSplitFeature;
  while ( splitLines.nextFeature( aSplitFeature ) )
  {
    if ( feedback->isCanceled() )
    {
      break;
    }

    splitGeoms.insert( aSplitFeature.id(), aSplitFeature.geometry() );
    spatialIndex.insertFeature( aSplitFeature );
  }

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
      sink->addFeature( inFeatureA, QgsFeatureSink::FastInsert );
      continue;
    }

    QgsGeometry inGeom = inFeatureA.geometry();
    outFeat.setAttributes( inFeatureA.attributes() );

    QVector< QgsGeometry > inGeoms = inGeom.asGeometryCollection();

    const QgsFeatureIds lines = spatialIndex.intersects( inGeom.boundingBox() ).toSet();
    if ( !lines.empty() ) // has intersection of bounding boxes
    {
      QVector< QgsGeometry > splittingLines;

      // use prepared geometries for faster intersection tests
      std::unique_ptr< QgsGeometryEngine > engine;

      for ( QgsFeatureId line : lines )
      {
        // check if trying to self-intersect
        if ( sameLayer && inFeatureA.id() == line )
          continue;

        QgsGeometry splitGeom = splitGeoms.value( line );
        if ( !engine )
        {
          engine.reset( QgsGeometry::createGeometryEngine( inGeom.constGet() ) );
          engine->prepareGeometry();
        }

        if ( engine->intersects( splitGeom.constGet() ) )
        {
          QVector< QgsGeometry > splitGeomParts = splitGeom.asGeometryCollection();
          splittingLines.append( splitGeomParts );
        }
      }

      if ( !splittingLines.empty() )
      {
        for ( const QgsGeometry &splitGeom : qgis::as_const( splittingLines ) )
        {
          QVector<QgsPointXY> splitterPList;
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
            if ( !inGeom )
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
                      splitterPList << QgsPointXY( pt );
                    }
                  }
                }
              }

              QVector< QgsGeometry > newGeometries;
              QVector<QgsPointXY> topologyTestPoints;
              QgsGeometry::OperationResult result = inGeom.splitGeometry( splitterPList, newGeometries, false, topologyTestPoints );

              // splitGeometry: If there are several intersections
              // between geometry and splitLine, only the first one is considered.
              if ( result == QgsGeometry::Success ) // split occurred
              {
                if ( inGeom.isGeosEqual( before ) )
                {
                  // bug in splitGeometry: sometimes it returns 0 but
                  // the geometry is unchanged
                  outGeoms.append( inGeom );
                }
                else
                {
                  inGeoms.append( inGeom );
                  inGeoms.append( newGeometries );
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
    for ( const QgsGeometry &aGeom : qgis::as_const( inGeoms ) )
    {
      if ( feedback->isCanceled() )
      {
        break;
      }

      bool passed = true;
      if ( QgsWkbTypes::geometryType( aGeom.wkbType() ) == QgsWkbTypes::LineGeometry )
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
      sink->addFeature( outFeat, QgsFeatureSink::FastInsert );
    }

    feedback->setProgress( i * step );
  }

  QVariantMap outputs;
  outputs.insert( QStringLiteral( "OUTPUT" ), dest );
  return outputs;
}



///@endcond


