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
       * @brief run analysis
       */
      void run( QgsFeedback *feedback );

      /**
       * Returns exceptions
       * @return List of exception messages occurred during tool execution
       */
      const QStringList &exceptions() const { return mExceptions; }

    private:

      /**
       * Executes the job queue
       */
      QFuture<void> execute();

    protected:

      /**
       * Job struct, contains feature id and task flag
       */
      struct Job
      {
        Job( QgsFeature _feature, int _taskFlag )
          : feature( _feature ), taskFlag( _taskFlag ) {}
        QgsFeature feature;
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
       * @brief Create job queue from next feature chunk
       * @return  true if there are more features. False if all fetures have been processed
       */
      virtual bool prepareNextChunk() = 0;

      /**
       * Process individual feature, implemented by subclasses
       */
      virtual void processFeature( const Job *job ) = 0;

      /**
       * Builds spatial index for source
       */
      void buildSpatialIndex( QgsSpatialIndex &index, QgsFeatureSource *layer ) const;

      /**
       * @brief appendNextChunkToJobQueue
       * @param layer
       * @param taskFlag
       */
      bool appendNextChunkToJobQueue( QgsFeatureSource *layer, int taskFlag = 0 );

      /**
       * Writes feature to output source
       */
      void writeFeatures( QgsFeatureList &outFeatures );

      void prepareLayer( QgsFeatureSource *source, const QgsAttributeList *sourceFieldIndices = 0 );

      QList<Job *> mJobQueue;
      QStringList mExceptions;

      QMutex mWriteMutex;
      QgsFeatureSink *mOutput;
      QgsCoordinateTransformContext mTransformContext;
      QgsFeatureRequest::InvalidGeometryCheck mInvalidGeometryCheck;

      const int mChunkSize = 100; //number of features fetched in one go
      QgsFeatureIterator mFeatureIterator;
      long mFeatureCount = 0; //number of features to be processed
  };

} // Geoprocessing

#endif // VECTORANALYSIS_QGSABSTRACT_TOOL_H
