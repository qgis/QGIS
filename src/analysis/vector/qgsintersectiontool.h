/***************************************************************************
 *  qgsintersectiontool.h                                                 *
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

#ifndef VECTORANALYSIS_QGSINTERSECTION_TOOL_H
#define VECTORANALYSIS_QGSINTERSECTION_TOOL_H

#define SIP_NO_FILE

#include "qgsabstracttool.h"

class QgsFeatureSource;

namespace Vectoranalysis
{

  /**
   * \ingroup analysis
   * Multithreaded vector intersection tool
   * \since QGIS 3.14
  */
  class ANALYSIS_EXPORT QgsIntersectionTool : public QgsAbstractTool
  {
    public:

      /**
       * QgsIntersectionTool constructor
       */
      QgsIntersectionTool( QgsFeatureSource *layerA,
                           QgsFeatureSource *layerB,
                           const QgsAttributeList &fieldIndicesA,
                           const QgsAttributeList &fieldIndicesB,
                           QgsFeatureSink *output,
                           QgsCoordinateTransformContext transformContext,
                           QgsFeatureRequest::InvalidGeometryCheck invalidGeometryCheck = QgsFeatureRequest::GeometryNoCheck );

    protected:

      /**
       * Prepare jobs
       */
      void prepare() override;

      /**
       * Process feature
       */
      void processFeature( const Job *job ) override;

      /**
       * @brief Create job queue from next feature chunk
       * @return  true if there are more features. False if all fetures have been processed
       */
      bool prepareNextChunk() override;

    private:
      QgsSpatialIndex mSpatialIndex;
      QgsFeatureSource *mLayerA;
      QgsFeatureSource *mLayerB;
      QgsAttributeList mFieldIndicesA;
      QgsAttributeList mFieldIndicesB;


  };

} // Geoprocessing

#endif // VECTORANALYSIS_QGSINTERSECTION_TOOL_H
