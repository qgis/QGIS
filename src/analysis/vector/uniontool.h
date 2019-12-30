/***************************************************************************
 *  unionaytool.h                                                      *
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

#ifndef VECTORANALYSIS_UNION_TOOL_H
#define VECTORANALYSIS_UNION_TOOL_H

#include "abstracttool.h"

namespace Vectoranalysis
{

  class ANALYSIS_EXPORT UnionTool : public AbstractTool
  {
    public:
      UnionTool( QgsFeatureSource *layerA,
                 QgsFeatureSource *layerB,
                 const QgsAttributeList &fieldIndicesA,
                 const QgsAttributeList &fieldIndicesB,
                 QgsFeatureSink *output,
                 QgsWkbTypes::Type outWkbType,
                 QgsCoordinateTransformContext transformContext,
                 double precision = 1E-7 );

    private:
      enum Task
      {
        ProcessLayerAFeature,
        ProcessLayerBFeature
      };

      QgsSpatialIndex mSpatialIndexA;
      QgsSpatialIndex mSpatialIndexB;
      QgsFeatureSource *mLayerA;
      QgsFeatureSource *mLayerB;
      QgsAttributeList mFieldIndicesA;
      QgsAttributeList mFieldIndicesB;

      void prepare();
      void processFeature( const Job *job );
  };

} // Geoprocessing

#endif // VECTORANALYSIS_UNION_TOOL_H
