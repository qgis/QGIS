/***************************************************************************
    qgsdamengfeatureiterator.h
    ---------------------
    begin                : Juli 2012
    copyright            : ( C ) 2012 by Martin Dobias
    email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   ( at your option ) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSDAMENGFEATUREITERATOR_H
#define QGSDAMENGFEATUREITERATOR_H

#include "qgsfeatureiterator.h"
#include "qgsdbquerylog_p.h"

#include <QQueue>

#include "qgsdamengprovider.h"
#include "qgscoordinatetransform.h"

class QgsDamengProvider;
class QgsDamengResult;
class QgsDamengTransaction;


class QgsDamengFeatureSource final: public QgsAbstractFeatureSource
{
  public:
    explicit QgsDamengFeatureSource( const QgsDamengProvider *p );
    ~QgsDamengFeatureSource() override;

    QgsFeatureIterator getFeatures( const QgsFeatureRequest &request ) override;

  private:

    QString mConnInfo;

    QString mGeometryColumn;
    QString mBoundingBoxColumn;
    QString mSqlWhereClause;
    QgsFields mFields;
    QgsDamengGeometryColumnType mSpatialColType;
    QString mRequestedSrid;
    QString mDetectedSrid;
    Qgis::WkbType mRequestedGeomType; //!< Geometry type requested in the uri
    Qgis::WkbType mDetectedGeomType;  //!< Geometry type detected in the database
    QgsDamengPrimaryKeyType mPrimaryKeyType;
    QList<int> mPrimaryKeyAttrs;
    QString mQuery;
    // TODO: loadFields()
    QgsCoordinateReferenceSystem mCrs;

    std::shared_ptr<QgsDamengSharedData> mShared;

    /* The transaction connection ( if any ) gets refed/unrefed when creating/
     * destroying the QgsDamengFeatureSource, to ensure that the transaction
     * connection remains valid during the life time of the feature source
     * even if the QgsDamengTransaction object which initially created the
     * connection has since been destroyed. */
    QgsDamengConn *mTransactionConnection = nullptr;

    friend class QgsDamengFeatureIterator;
    friend class QgsDamengExpressionCompiler;
};


class QgsDamengConn;

class QgsDamengFeatureIterator final: public QgsAbstractFeatureIteratorFromSource<QgsDamengFeatureSource>
{
  public:
    QgsDamengFeatureIterator( QgsDamengFeatureSource *source, bool ownSource, const QgsFeatureRequest &request );

    ~QgsDamengFeatureIterator() override;

    bool rewind() override;
    bool close() override;

  protected:
    bool fetchFeature( QgsFeature &feature ) override;
    bool nextFeatureFilterExpression( QgsFeature &f ) override;
    bool prepareSimplification( const QgsSimplifyMethod &simplifyMethod ) override;

  private:

    QgsDamengConn *mConn = nullptr;


    QString whereClauseRect();
    bool getFeature( QgsDamengResult &queryResult, int row, QgsFeature &feature );
    void getFeatureAttribute( int idx, QgsDamengResult &queryResult, int row, int &col, QgsFeature &feature );
    bool openQuery( const QString &whereClause, long limit = -1, bool closeOnFail = true, const QString &orderBy = QString() );

    /**
     * Feature queue that GetNextFeature will retrieve from
     * before the next fetch from Dameng
     */
    QQueue<QgsFeature> mFeatureQueue;

    //! Maximal size of the feature queue
    int mFeatureQueueSize = 2000;

    //! Number of retrieved features
    int mFetched = 0;

    //! Sets to true, if geometry is in the requested columns
    bool mFetchGeometry = false;

    bool mIsTransactionConnection = false;

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
    std::unique_ptr<QgsGeometryEngine> mDistanceWithinEngine;
};

#endif // QGSDAMENGFEATUREITERATOR_H
