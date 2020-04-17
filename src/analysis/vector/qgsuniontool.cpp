/***************************************************************************
 *  unionaytool.cpp                                                    *
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
#include "qgsuniontool.h"
#include "qgsgeos.h"
#include "qgsgeometry.h"
#include "qgsoverlayutils.h"
#include "qgsvectorlayer.h"

namespace Vectoranalysis
{

  QgsUnionTool::QgsUnionTool( QgsFeatureSource *layerA,
                              QgsFeatureSource *layerB,
                              const QgsAttributeList &fieldIndicesA,
                              const QgsAttributeList &fieldIndicesB,
                              QgsFeatureSink *output,
                              QgsCoordinateTransformContext transformContext,
                              QgsFeatureRequest::InvalidGeometryCheck invalidGeometryCheck )
    : QgsAbstractTool( output, transformContext, invalidGeometryCheck ), mLayerA( layerA ), mLayerB( layerB ), mFieldIndicesA( fieldIndicesA ), mFieldIndicesB( fieldIndicesB )
  {
  }

  void QgsUnionTool::prepare()
  {
    bool mLayerAFinished = false;
    buildSpatialIndex( mSpatialIndexA, mLayerA );
    buildSpatialIndex( mSpatialIndexB, mLayerB );
    prepareLayer( mLayerA, &mFieldIndicesA );
    mFeatureCount = mLayerA->featureCount() + mLayerB->featureCount();
  }

  void QgsUnionTool::processFeature( const Job *job )
  {
    if ( !mOutput || !mLayerA || !mLayerB )
    {
      return;
    }

    if ( job->taskFlag == ProcessLayerAFeature )
    {
      QgsFeatureList intersection = QgsOverlayUtils::featureIntersection( job->feature, *mLayerA, *mLayerB, mSpatialIndexB, mTransformContext, mFieldIndicesA, mFieldIndicesB );
      writeFeatures( intersection );
      QgsFeatureList differenceA = QgsOverlayUtils::featureDifference( job->feature, *mLayerA, *mLayerB, mSpatialIndexB, mTransformContext, mLayerA->fields().size(), mLayerB->fields().size(), QgsOverlayUtils::OutputAB );
      writeFeatures( differenceA );
    }
    else // if(job->taskFlag == ProcessLayerBFeature)
    {
      QgsFeatureList differenceB = QgsOverlayUtils::featureDifference( job->feature, *mLayerB, *mLayerA, mSpatialIndexA, mTransformContext, mLayerB->fields().size(), mLayerA->fields().size(), QgsOverlayUtils::OutputBA );
      writeFeatures( differenceB );
    }
  }

  bool QgsUnionTool::prepareNextChunk()
  {
    if ( !mLayerA || !mLayerB )
    {
      return false;
    }

    if ( !mLayerAFinished )
    {
      bool a = appendNextChunkToJobQueue( mLayerA, ProcessLayerAFeature );
      if ( a )
      {
        return true;
      }

      mLayerAFinished = true;
      prepareLayer( mLayerB, &mFieldIndicesB );
    }

    return appendNextChunkToJobQueue( mLayerB, ProcessLayerBFeature );
  }
} // Geoprocessing
