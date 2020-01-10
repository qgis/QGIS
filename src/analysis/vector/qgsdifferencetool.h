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

#ifndef VECTORANALYSIS_QGSDIFFERENCE_TOOL_H
#define VECTORANALYSIS_QGSDIFFERENCE_TOOL_H

#define SIP_NO_FILE

#include "qgsabstracttool.h"

namespace Vectoranalysis
{

  /**
   * \ingroup analysis
   * Multithreaded vector difference tool
   * \since QGIS 3.14
  */
  class ANALYSIS_EXPORT QgsDifferenceTool : public QgsAbstractTool
  {
    public:

      /**
       * QgsDifferenceTool constructor
       */
      QgsDifferenceTool( QgsFeatureSource *layerA,
                         QgsFeatureSource *layerB,
                         QgsFeatureSink *output,
                         QgsCoordinateTransformContext transformContext,
                         QgsFeatureRequest::InvalidGeometryCheck invalidGeometryCheck = QgsFeatureRequest::GeometryNoCheck );
      ~QgsDifferenceTool() {}

    private:
      QgsSpatialIndex mSpatialIndex;
      QgsFeatureSource *mLayerA;
      QgsFeatureSource *mLayerB;

      /**
       * Prepare jobs
       */
      void prepare();

      /**
       * Process feature
       */
      void processFeature( const Job *job );
  };

} // Geoprocessing

#endif // VECTORANALYSIS_QGSDIFFERENCE_TOOL_H
