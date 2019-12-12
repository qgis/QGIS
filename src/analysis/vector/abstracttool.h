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

      typedef QPair<QgsFeatureSource *, QgsFeatureId> ErrorFeature;
      struct Error
      {
        Error( const QList<ErrorFeature> &_features, const QString &_errorMsg )
          : features( _features ), errorMsg( _errorMsg ) {}
        QList<ErrorFeature> features;
        QString errorMsg;
      };

      AbstractTool( QgsFeatureSink *output, QgsWkbTypes::Type outWkbType, double precision = 1E-7 );
      virtual ~AbstractTool();
      QFuture<void> init();
      virtual QFuture<void> execute( int task );
      virtual int getTaskCount() const { return 1; }
      void finalizeOutput() {}

      bool errorsOccurred() const { return !mGeometryErrorList.isEmpty() || !mFeatureErrorList.isEmpty() || !mWriteErrors.isEmpty(); }
      const QList<Error> &getFeatureErrorList() const { return mFeatureErrorList; }
      const QList<Error> &getGeometryErrorList() const { return mGeometryErrorList; }
      const QList<QString> &getWriteErrors() const { return mWriteErrors; }
      const QList<QString> &getExceptions() const { return mExceptions; }


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
      QVector<QgsFeature *> getIntersects( const QgsRectangle &rect, QgsSpatialIndex &index, QgsFeatureSource *layer, const QgsAttributeList &attIdx );
      void writeFeatures( const QVector<QgsFeature *> &outFeatures );
      void writeFeature( const QgsGeometry &geom, const QgsAttributes &att );

      void reportInvalidFeatureError( QgsFeatureSource *layer, const QgsFeatureId &id, const QString &errorMessage )
      {
        QMutexLocker locker( &mErrorMutex );
        mFeatureErrorList.append( Error( QList<ErrorFeature>() << ErrorFeature( layer, id ), errorMessage ) );
      }

      void reportGeometryError( const QList<ErrorFeature> &features, const QString &errorMessage )
      {
        QMutexLocker locker( &mErrorMutex );
        mGeometryErrorList.append( Error( features, errorMessage ) );
      }

      QList<Job *> mJobQueue;
      QList<Error> mGeometryErrorList;
      QList<Error> mFeatureErrorList;
      QList<QString> mExceptions;

      QList<QString> mWriteErrors;
      QMutex mErrorMutex;
      QMutex mIntersectMutex;
      QMutex mWriteMutex;
      QgsFeatureSink *mOutput;
      int mNumOutFields;
      QgsWkbTypes::Type mOutWkbType;
      QString mOutputDriverName;
      double mPrecision;
  };

} // Geoprocessing

#endif // VECTORANALYSIS_ABSTRACT_TOOL_H
