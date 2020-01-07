/***************************************************************************
 *  differencetool.h                                                   *
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

#ifndef VECTORANALYSIS_DIFFERENCE_TOOL_H
#define VECTORANALYSIS_DIFFERENCE_TOOL_H

#include "abstracttool.h"

namespace Vectoranalysis
{

  class ANALYSIS_EXPORT DifferenceTool : public AbstractTool
  {
    public:
      DifferenceTool( QgsFeatureSource *layerA,
                      QgsFeatureSource *layerB,
                      QgsFeatureSink *output,
                      QgsCoordinateTransformContext transformContext,
                      QgsFeatureRequest::InvalidGeometryCheck invalidGeometryCheck = QgsFeatureRequest::GeometryNoCheck );
      ~DifferenceTool() {}

    private:
      QgsSpatialIndex mSpatialIndex;
      QgsFeatureSource *mLayerA;
      QgsFeatureSource *mLayerB;

      void prepare();
      void processFeature( const Job *job );
  };

} // Geoprocessing

#endif // VECTORANALYSIS_DIFFERENCE_TOOL_H
