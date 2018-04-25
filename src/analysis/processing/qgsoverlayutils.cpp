/***************************************************************************
  qgsoverlayutils.cpp
  ---------------------
  Date                 : April 2018
  Copyright            : (C) 2018 by Martin Dobias
  Email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsoverlayutils.h"

#include "qgsgeometryengine.h"
#include "qgsprocessingalgorithm.h"

///@cond PRIVATE

void QgsOverlayUtils::difference( const QgsFeatureSource &sourceA, const QgsFeatureSource &sourceB, QgsFeatureSink &sink, QgsProcessingContext &context, QgsProcessingFeedback *feedback, int &count, int totalCount, QgsOverlayUtils::DifferenceOutput outputAttrs )
{
  QgsFeatureRequest requestB;
  requestB.setSubsetOfAttributes( QgsAttributeList() );
  if ( outputAttrs != OutputBA )
    requestB.setDestinationCrs( sourceA.sourceCrs(), context.transformContext() );
  QgsSpatialIndex indexB( sourceB.getFeatures( requestB ), feedback );

  int fieldsCountA = sourceA.fields().count();
  int fieldsCountB = sourceB.fields().count();
  QgsAttributes attrs;
  attrs.resize( outputAttrs == OutputA ? fieldsCountA : ( fieldsCountA + fieldsCountB ) );

  if ( totalCount == 0 )
    totalCount = 1;  // avoid division by zero

  QgsFeature featA;
  QgsFeatureRequest requestA;
  if ( outputAttrs == OutputBA )
    requestA.setDestinationCrs( sourceB.sourceCrs(), context.transformContext() );
  QgsFeatureIterator fitA = sourceA.getFeatures( requestA );
  while ( fitA.nextFeature( featA ) )
  {
    if ( feedback->isCanceled() )
      break;

    if ( featA.hasGeometry() )
    {
      QgsGeometry geom( featA.geometry() );
      QgsFeatureIds intersects = indexB.intersects( geom.boundingBox() ).toSet();

      QgsFeatureRequest request;
      request.setFilterFids( intersects );
      request.setSubsetOfAttributes( QgsAttributeList() );
      if ( outputAttrs != OutputBA )
        request.setDestinationCrs( sourceA.sourceCrs(), context.transformContext() );

      std::unique_ptr< QgsGeometryEngine > engine;
      if ( !intersects.isEmpty() )
      {
        // use prepared geometries for faster intersection tests
        engine.reset( QgsGeometry::createGeometryEngine( geom.constGet() ) );
        engine->prepareGeometry();
      }

      QVector<QgsGeometry> geometriesB;
      QgsFeature featB;
      QgsFeatureIterator fitB = sourceB.getFeatures( request );
      while ( fitB.nextFeature( featB ) )
      {
        if ( feedback->isCanceled() )
          break;

        if ( engine->intersects( featB.geometry().constGet() ) )
          geometriesB << featB.geometry();
      }

      if ( !geometriesB.isEmpty() )
      {
        QgsGeometry geomB = QgsGeometry::unaryUnion( geometriesB );
        geom = geom.difference( geomB );
      }

      // if geomB covers the whole source geometry, we get an empty geometry collection
      if ( geom.isEmpty() )
        continue;

      const QgsAttributes attrsA( featA.attributes() );
      switch ( outputAttrs )
      {
        case OutputA:
          attrs = attrsA;
          break;
        case OutputAB:
          for ( int i = 0; i < fieldsCountA; ++i )
            attrs[i] = attrsA[i];
          break;
        case OutputBA:
          for ( int i = 0; i < fieldsCountA; ++i )
            attrs[i + fieldsCountB] = attrsA[i];
          break;
      }

      QgsFeature outFeat;
      geom.convertToMultiType();
      outFeat.setGeometry( geom );
      outFeat.setAttributes( attrs );
      sink.addFeature( outFeat, QgsFeatureSink::FastInsert );
    }
    else
    {
      // TODO: should we write out features that do not have geometry?
      sink.addFeature( featA, QgsFeatureSink::FastInsert );
    }

    ++count;
    feedback->setProgress( count / ( double ) totalCount * 100. );
  }
}

///@endcond PRIVATE
