/***************************************************************************
 *  intersectiontool.h                                                 *
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

#ifndef VECTORANALYSIS_INTERSECTION_TOOL_H
#define VECTORANALYSIS_INTERSECTION_TOOL_H

#include "abstracttool.h"

class QgsFeatureSource;

namespace Vectoranalysis
{

  class ANALYSIS_EXPORT IntersectionTool : public AbstractTool
  {
    public:
      IntersectionTool( QgsFeatureSource *layerA,
                        QgsFeatureSource *layerB,
                        const QgsAttributeList &fieldIndicesA,
                        const QgsAttributeList &fieldIndicesB,
                        QgsFeatureSink *output,
                        QgsCoordinateTransformContext transformContext,
                        double precision = 1E-7 );

    private:
      QgsSpatialIndex mSpatialIndex;
      QgsFeatureSource *mLayerA;
      QgsFeatureSource *mLayerB;
      QgsAttributeList mFieldIndicesA;
      QgsAttributeList mFieldIndicesB;

      void prepare();
      void processFeature( const Job *job );
  };

} // Geoprocessing

#endif // VECTORANALYSIS_INTERSECTION_TOOL_H
