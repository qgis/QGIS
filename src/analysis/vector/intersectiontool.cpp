/***************************************************************************
 *  intersectiontool.cpp                                               *
 *  -------------------                                                    *
 *  begin                : Jun 10, 2014                                    *
 *  copyright            : (C) 2014 by Sandro Mani / Sourcepole AG         *
 *  email                : smani@sourcepole.ch                             *
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <QApplication>
#include "intersectiontool.h"
#include "qgsfeaturesource.h"
#include "qgsgeometry.h"
#include "qgsgeos.h"
#include "qgsvectorlayer.h"

namespace Vectoranalysis
{

  IntersectionTool::IntersectionTool(
    QgsFeatureSource *layerA,
    QgsFeatureSource *layerB,
    const QgsAttributeList &fieldIndicesA,
    const QgsAttributeList &fieldIndicesB,
    QgsFeatureSink *output,
    QgsWkbTypes::Type outWkbType,
    double precision )
    : AbstractTool( output, outWkbType, precision ), mLayerA( layerA ), mLayerB( layerB ), mFieldIndicesA( fieldIndicesA ), mFieldIndicesB( fieldIndicesB )
  {
  }

  void IntersectionTool::prepare()
  {
    appendToJobQueue( mLayerA );
    buildSpatialIndex( mSpatialIndex, mLayerB );
  }

  void IntersectionTool::processFeature( const Job *job )
  {
    // Get currently processed feature
    QgsFeature f;
    if ( !getFeatureAtId( f, job->featureid, mLayerA, mFieldIndicesA ) )
    {
      return;
    }
    QgsGeometry geom = f.geometry();

    // Get features which intersect current feature
    QVector<QgsFeature *> featureList = getIntersects( geom.boundingBox(), mSpatialIndex, mLayerB, mFieldIndicesB );

    // Perform intersections
    QgsGeos geos( geom.constGet() );
    geos.prepareGeometry();

    QVector<QgsFeature *> outputFeatures; // Use pointers to prevent copies
    QString errorMsg;

    for ( QgsFeature *testFeature : featureList )
    {
      QgsGeometry testGeom = testFeature->geometry();
      if ( geos.intersects( testGeom.constGet() ) )
      {
        QgsGeometry outGeometry = geom.intersection( testGeom );
        if ( outGeometry.isNull() )
        {
          reportGeometryError( QList<ErrorFeature>() << ErrorFeature( mLayerA, job->featureid ) << ErrorFeature( mLayerB, testFeature->id() ), outGeometry.lastError() );
        }
        else if ( outGeometry.isEmpty() )
        {
          reportGeometryError( QList<ErrorFeature>() << ErrorFeature( mLayerA, job->featureid ) << ErrorFeature( mLayerB, testFeature->id() ), QApplication::translate( "IntersectionTool", "GEOSIntersection returned empty geometry even though the geometries intersect" ) );
        }
        else
        {
          QgsFeature *outFeature = new QgsFeature();
          outFeature->setGeometry( outGeometry );
          QgsAttributes fAtt = f.attributes();
          QgsAttributes testAtt = testFeature->attributes();
          outFeature->setAttributes( fAtt + testAtt );
          outputFeatures.append( outFeature );
        }
      }
    }
    writeFeatures( outputFeatures );
    qDeleteAll( outputFeatures );
    qDeleteAll( featureList );
  }

} // Geoprocessing
