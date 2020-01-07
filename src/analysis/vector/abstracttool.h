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

#ifndef VECTORANALYSIS_ABSTRACT_TOOL_H
#define VECTORANALYSIS_ABSTRACT_TOOL_H

#include "qgis_analysis.h"
#include <QObject>
#include <QFuture>
#include <QList>
#include <QMutexLocker>
#include "qgsspatialindex.h"
#include "qgsfeaturesink.h"
#include "qgsfeaturesource.h"

class QgsCoordinateReferenceSystem;
class QgsVectorLayer;

namespace Vectoranalysis
{
  class ANALYSIS_EXPORT AbstractTool
  {
    public:
      enum OutputCrs
      {
        CrsLayerA,
        CrsLayerB
      };

      AbstractTool( QgsFeatureSink *output, QgsCoordinateTransformContext transformContext, QgsFeatureRequest::InvalidGeometryCheck invalidGeometryCheck = QgsFeatureRequest::GeometryNoCheck );
      virtual ~AbstractTool();
      QFuture<void> init();
      virtual QFuture<void> execute( int task );
      virtual int getTaskCount() const { return 1; }
      void finalizeOutput() {}
      const QStringList &getExceptions() const { return mExceptions; }


    protected:
      static QString errFeatureDoesNotExist;
      static QString errFailedToFetchGeometry;

      struct Job
      {
        Job( const QgsFeatureId &_featureid, int _taskFlag )
          : featureid( _featureid ), taskFlag( _taskFlag ) {}
        virtual ~Job() {}
        QgsFeatureId featureid;
        int taskFlag;
      };

      struct ProcessFeatureWrapper
      {
        AbstractTool *instance;
        ProcessFeatureWrapper( AbstractTool *_instance ) : instance( _instance ) {}
        void operator()( const Job *job );
      };

      virtual void prepare() = 0;
      virtual void processFeature( const Job *job ) = 0;

      void buildSpatialIndex( QgsSpatialIndex &index, QgsFeatureSource *layer ) const;
      void appendToJobQueue( QgsFeatureSource *layer, int taskFlag = 0 );
      bool getFeatureAtId( QgsFeature &feature, QgsFeatureId id, QgsFeatureSource *layer, const QgsAttributeList &attIdx );
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

#endif // VECTORANALYSIS_ABSTRACT_TOOL_H
