/***************************************************************************
 *  symdifferencetool.cpp                                              *
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
#include "symdifferencetool.h"
#include "qgsgeometry.h"
#include "qgsgeos.h"
#include "qgsvectorlayer.h"

namespace Vectoranalysis
{

  SymDifferenceTool::SymDifferenceTool(
    QgsFeatureSource *layerA,
    QgsFeatureSource *layerB,
    QgsFeatureSink *output,
    QgsWkbTypes::Type outWkbType,
    double precision )
    : AbstractTool( output, outWkbType, precision ), mLayerA( layerA ), mLayerB( layerB )
  {
  }

  void SymDifferenceTool::prepare()
  {
    appendToJobQueue( mLayerA, ProcessLayerAFeature );
    appendToJobQueue( mLayerB, ProcessLayerBFeature );
    buildSpatialIndex( mSpatialIndexA, mLayerA );
    buildSpatialIndex( mSpatialIndexB, mLayerB );
  }

  void SymDifferenceTool::processFeature( const Job *job )
  {
    QgsFeatureSource *layerCurr = 0, * layerInter = 0;
    QgsSpatialIndex *spatialIndex = 0;

    if ( job->taskFlag == ProcessLayerAFeature )
    {
      layerCurr = mLayerA;
      layerInter = mLayerB;
      spatialIndex = &mSpatialIndexB;
    }
    else // if(job->taskFlag == ProcessLayerBFeature)
    {
      layerCurr = mLayerB;
      layerInter = mLayerA;;
      spatialIndex = &mSpatialIndexA;
    }

    // Get currently processed feature
    QgsFeature f;
    if ( !getFeatureAtId( f, job->featureid, layerCurr, layerCurr->fields().allAttributesList() ) )
    {
      return;
    }
    QgsGeometry geom = f.geometry();

    int nAttA = mLayerA->fields().size();
    int nAttB = mLayerB->fields().size();

    // Get features which intersect current feature
    QVector<QgsFeature *> featureList = getIntersects( geom.boundingBox(), *spatialIndex, layerInter, layerInter->fields().allAttributesList() );

    // Cut off all parts of current feature which intersect with features from intersecting layer
    QgsGeos geos( geom.constGet() );
    geos.prepareGeometry();

    QgsGeometry newGeom( geom );
    for ( QgsFeature *testFeature : featureList )
    {
      QgsGeometry testGeom = testFeature->geometry();
      if ( geos.intersects( testGeom.constGet() ) )
      {
        QgsGeometry newGeomTmp = newGeom.difference( testGeom );
        if ( newGeomTmp.isNull() )
        {
          reportGeometryError( QList<ErrorFeature>() << ErrorFeature( layerCurr, f.id() ) << ErrorFeature( layerInter, testFeature->id() ), newGeomTmp.lastError() );
          continue;
        }
        else
        {
          newGeom = newGeomTmp;
        }
      }
    }

    if ( !newGeom.isEmpty() )
    {
      QgsFeature outFeature;
      outFeature.setGeometry( newGeom );
      QgsAttributes fAtt;
      if ( job->taskFlag == ProcessLayerAFeature )
      {
        fAtt.append( f.attributes() );
        for ( int i = 0; i < nAttB; ++i )
        {
          fAtt.append( QVariant() );
        }
      }
      else
      {
        for ( int i = 0; i < nAttA; ++i )
        {
          fAtt.append( QVariant() );
        }
        fAtt.append( f.attributes() );
      }
      outFeature.setAttributes( fAtt );
      writeFeatures( QVector<QgsFeature *>() << &outFeature );
    }

    qDeleteAll( featureList );
  }

} // Geoprocessing
