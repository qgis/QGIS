/***************************************************************************
 *  differencetool.cpp                                                 *
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
#include "differencetool.h"
#include "qgsgeometry.h"
#include "qgsgeos.h"
#include "qgsoverlayutils.h"
#include "qgsvectorlayer.h"

namespace Vectoranalysis
{

  DifferenceTool::DifferenceTool( QgsFeatureSource *layerA,
                                  QgsFeatureSource *layerB,
                                  QgsFeatureSink *output,
                                  QgsCoordinateTransformContext transformContext,
                                  double precision )
    : AbstractTool( output, transformContext, precision ), mLayerA( layerA ), mLayerB( layerB )
  {
  }

  void DifferenceTool::prepare()
  {
    appendToJobQueue( mLayerA );
    buildSpatialIndex( mSpatialIndex, mLayerB );
  }

  void DifferenceTool::processFeature( const Job *job )
  {
    QgsFeature f;
    if ( !mOutput || !mLayerA || !mLayerB || !getFeatureAtId( f, job->featureid, mLayerA, mLayerA->fields().allAttributesList() ) )
    {
      return;
    }

    QgsFeatureList difference = QgsOverlayUtils::featureDifference( f, *mLayerA, *mLayerB, mSpatialIndex, mTransformContext, mLayerA->fields().size(), mLayerB->fields().size(), QgsOverlayUtils::OutputA );
    writeFeatures( difference );
  }

} // Geoprocessing
