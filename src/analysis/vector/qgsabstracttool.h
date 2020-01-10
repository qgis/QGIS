/***************************************************************************
 *  abstracttool.h                                                      *
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

#ifndef VECTORANALYSIS_QGSABSTRACT_TOOL_H
#define VECTORANALYSIS_QGSABSTRACT_TOOL_H

#define SIP_NO_FILE

#include "qgis_analysis.h"
#include <QObject>
#include <QFuture>
#include <QList>
#include <QMutexLocker>
#include "qgsspatialindex.h"
#include "qgsfeaturesink.h"
#include "qgsfeaturesource.h"

class QgsVectorLayer;

namespace Vectoranalysis
{

  /**
   * \ingroup analysis
   * Base class for multithreaded vector analysis tools
   * \since QGIS 3.14
  */
  class ANALYSIS_EXPORT QgsAbstractTool
  {
    public:

      /**
       * QgsAbstractTool constructor
       */
      QgsAbstractTool( QgsFeatureSink *output, QgsCoordinateTransformContext transformContext, QgsFeatureRequest::InvalidGeometryCheck invalidGeometryCheck = QgsFeatureRequest::GeometryNoCheck );
      virtual ~QgsAbstractTool();

      /**
       * Prepares the analysis tool
       * @return Future object
       */
      QFuture<void> init();

      /**
       * Executes the analysis tool
       */
      virtual QFuture<void> execute();

      /**
       * Returns exceptions
       * @return List of exception messages occurred during tool execution
       */
      const QStringList &exceptions() const { return mExceptions; }


    protected:

      /**
       * Job struct, contains feature id and task flag
       */
      struct Job
      {
        Job( const QgsFeatureId &_featureid, int _taskFlag )
          : featureid( _featureid ), taskFlag( _taskFlag ) {}
        virtual ~Job() {}
        QgsFeatureId featureid;
        int taskFlag;
      };

      /**
       * Wrapper class to execute job
       */
      struct ProcessFeatureWrapper
      {
        QgsAbstractTool *instance;
        ProcessFeatureWrapper( QgsAbstractTool *_instance ) : instance( _instance ) {}
        void operator()( const Job *job );
      };

      /**
       * Prepares analysis tool, needs to be implemented by subclasses
       */
      virtual void prepare() = 0;

      /**
       * Process individual feature, implemented by subclasses
       */
      virtual void processFeature( const Job *job ) = 0;

      /**
       * Builds spatial index for source
       */
      void buildSpatialIndex( QgsSpatialIndex &index, QgsFeatureSource *layer ) const;

      /**
       * Creates jobs for each feature and adds them to the job queue
       */
      void appendToJobQueue( QgsFeatureSource *layer, int taskFlag = 0 );

      /**
       * Fetch feature at id
       */
      bool getFeatureAtId( QgsFeature &feature, QgsFeatureId id, QgsFeatureSource *layer, const QgsAttributeList &attIdx );

      /**
       * Writes feature to output source
       */
      void writeFeatures( QgsFeatureList &outFeatures );

      QList<Job *> mJobQueue;
      QStringList mExceptions;

      QMutex mIntersectMutex;
      QMutex mWriteMutex;
      QgsFeatureSink *mOutput;
      QgsCoordinateTransformContext mTransformContext;
      QgsFeatureRequest::InvalidGeometryCheck mInvalidGeometryCheck;
  };

} // Geoprocessing

#endif // VECTORANALYSIS_QGSABSTRACT_TOOL_H
