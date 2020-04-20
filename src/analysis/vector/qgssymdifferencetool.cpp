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
#include "qgssymdifferencetool.h"
#include "qgsgeometry.h"
#include "qgsgeos.h"
#include "qgsoverlayutils.h"
#include "qgsvectorlayer.h"

namespace Vectoranalysis
{

  QgsSymDifferenceTool::QgsSymDifferenceTool(
    QgsFeatureSource *layerA,
    QgsFeatureSource *layerB,
    QgsFeatureSink *output,
    QgsCoordinateTransformContext transformContext,
    QgsFeatureRequest::InvalidGeometryCheck invalidGeometryCheck )
    : QgsAbstractTool( output, transformContext, invalidGeometryCheck ), mLayerA( layerA ), mLayerB( layerB )
  {
  }

  void QgsSymDifferenceTool::prepare()
  {
    buildSpatialIndex( mSpatialIndexA, mLayerA, mLayerA->sourceCrs() );
    buildSpatialIndex( mSpatialIndexB, mLayerB, mLayerA->sourceCrs() );
    mLayerAFinished = false;
    prepareLayer( mLayerA, mLayerA->sourceCrs() );
    mFeatureCount = mLayerA->featureCount() + mLayerB->featureCount();
  }

  void QgsSymDifferenceTool::processFeature( const Job *job )
  {
    if ( !mOutput || !mLayerA || !mLayerB )
    {
      return;
    }

    if ( job->taskFlag == ProcessLayerAFeature )
    {
      QgsFeatureList differenceA = QgsOverlayUtils::featureDifference( job->feature, *mLayerA, *mLayerB, mSpatialIndexB, mTransformContext, mLayerA->fields().size(), mLayerB->fields().size(), QgsOverlayUtils::OutputAB );
      writeFeatures( differenceA );
    }
    else // if(job->taskFlag == ProcessLayerBFeature)
    {
      QgsFeatureList differenceB = QgsOverlayUtils::featureDifference( job->feature, *mLayerB, *mLayerA, mSpatialIndexA, mTransformContext, mLayerB->fields().size(), mLayerA->fields().size(), QgsOverlayUtils::OutputBA );
      writeFeatures( differenceB );
    }
  }

  bool QgsSymDifferenceTool::prepareNextChunk()
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
      prepareLayer( mLayerB, mLayerA->sourceCrs() );
    }

    return appendNextChunkToJobQueue( mLayerB, ProcessLayerBFeature );
  }

} // Geoprocessing
