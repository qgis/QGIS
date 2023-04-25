/***************************************************************************
   qgsredshiftfeatureiterator.h
   --------------------------------------
   Date      : 16.02.2021
   Copyright : (C) 2021 Amazon Inc. or its affiliates
   Author    : Marcel Bezdrighin
 ***************************************************************************/

/***************************************************************************
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 ***************************************************************************/
#ifndef QGSREDSHIFTFEATUREITERATOR_H
#define QGSREDSHIFTFEATUREITERATOR_H

#include <QQueue>

#include "qgscoordinatetransform.h"
#include "qgsfeatureiterator.h"
#include "qgsgeometry.h"
#include "qgsgeometryengine.h"
#include "qgsredshiftprovider.h"

class QgsRedshiftProvider;
class QgsRedshiftResult;

class QgsRedshiftFeatureSource final : public QgsAbstractFeatureSource
{
  public:
    explicit QgsRedshiftFeatureSource( const QgsRedshiftProvider *p );

    QgsFeatureIterator getFeatures( const QgsFeatureRequest &request ) override;

  private:
    QString mConnInfo;

    QString mGeometryColumn;
    QString mBoundingBoxColumn;
    QString mSqlWhereClause;
    QgsFields mFields;
    QgsRedshiftGeometryColumnType mSpatialColType;
    QString mRequestedSrid;
    QString mDetectedSrid;
    Qgis::WkbType mRequestedGeomType; //!< Geometry type requested in the uri
    Qgis::WkbType mDetectedGeomType;  //!< Geometry type detected in the database
    QList<int> mPrimaryKeyAttrs;
    QString mQuery;
    QgsCoordinateReferenceSystem mCrs;
    bool mIsExternalDatabase = false;

    std::shared_ptr<QgsRedshiftSharedData> mShared;

    friend class QgsRedshiftFeatureIterator;
    friend class QgsRedshiftExpressionCompiler;
};

class QgsRedshiftConn;

class QgsRedshiftFeatureIterator final : public QgsAbstractFeatureIteratorFromSource<QgsRedshiftFeatureSource>
{
  public:
    QgsRedshiftFeatureIterator( QgsRedshiftFeatureSource *source, bool ownSource, const QgsFeatureRequest &request );

    ~QgsRedshiftFeatureIterator() override;

    bool rewind() override;
    bool close() override;

  protected:
    bool fetchFeature( QgsFeature &feature ) override;
    bool nextFeatureFilterExpression( QgsFeature &f ) override;
    bool prepareSimplification( const QgsSimplifyMethod &simplifyMethod ) override;

  private:
    QgsRedshiftConn *mConn = nullptr;

    QString whereClauseRect();
    bool getFeature( QgsRedshiftResult &queryResult, int row, QgsFeature &feature );
    void getFeatureAttribute( int idx, QgsRedshiftResult &queryResult, int row, int &col, QgsFeature &feature );
    bool declareCursor( const QString &whereClause, long limit = -1, bool closeOnFail = true,
                        const QString &orderBy = QString() );
    bool initCursor();

    QString mCursorName;

    /**
     * Feature queue that GetNextFeature will retrieve from
     * before the next fetch from Redshift
     */
    QQueue<QgsFeature> mFeatureQueue;

    //! Number of retrieved features
    int mFetched = 0;

    //! Sets to true, if geometry is in the requested columns
    bool mFetchGeometry = false;

    bool providerCanSimplify( QgsSimplifyMethod::MethodType methodType ) const override;

    bool prepareOrderBy( const QList<QgsFeatureRequest::OrderByClause> &orderBys ) override;

    inline void lock();
    inline void unlock();

    bool mExpressionCompiled = false;
    bool mOrderByCompiled = false;
    bool mLastFetch = false;
    bool mFilterRequiresGeometry = false;

    QgsCoordinateTransform mTransform;
    QgsRectangle mFilterRect;
    QgsGeometry mDistanceWithinGeom;
    std::unique_ptr< QgsGeometryEngine > mDistanceWithinEngine;
};

#endif // QGSREDSHIFTFEATUREITERATOR_H
