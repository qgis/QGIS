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
#include "qgsoverlayutils.h"
#include "qgsvectorlayer.h"

namespace Vectoranalysis
{

  SymDifferenceTool::SymDifferenceTool(
    QgsFeatureSource *layerA,
    QgsFeatureSource *layerB,
    QgsFeatureSink *output,
    QgsCoordinateTransformContext transformContext,
    double precision )
    : AbstractTool( output, transformContext, precision ), mLayerA( layerA ), mLayerB( layerB )
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
    QgsFeature f;
    if ( !mOutput || !mLayerA || !mLayerB )
    {
      return;
    }

    QgsGeometry geom = f.geometry();
    if ( job->taskFlag == ProcessLayerAFeature )
    {
      if ( !getFeatureAtId( f, job->featureid, mLayerA, mLayerA->fields().allAttributesList() ) )
      {
        return;
      }
      QgsFeatureList differenceA = QgsOverlayUtils::featureDifference( f, *mLayerA, *mLayerB, mSpatialIndexB, mTransformContext, mLayerA->fields().size(), mLayerB->fields().size(), QgsOverlayUtils::OutputAB );
      writeFeatures( differenceA );
    }
    else // if(job->taskFlag == ProcessLayerBFeature)
    {
      if ( !getFeatureAtId( f, job->featureid, mLayerB, mLayerB->fields().allAttributesList() ) )
      {
        return;
      }
      QgsFeatureList differenceB = QgsOverlayUtils::featureDifference( f, *mLayerB, *mLayerA, mSpatialIndexA, mTransformContext, mLayerB->fields().size(), mLayerA->fields().size(), QgsOverlayUtils::OutputBA );
      writeFeatures( differenceB );
    }
  }
} // Geoprocessing
