/***************************************************************************
    qgspostgresfeatureiterator.h
    ---------------------
    begin                : Juli 2012
    copyright            : (C) 2012 by Martin Dobias
    email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSPOSTGRESFEATUREITERATOR_H
#define QGSPOSTGRESFEATUREITERATOR_H

#include "qgsfeatureiterator.h"

#include <QQueue>

#include "qgspostgresprovider.h"
#include "qgscoordinatetransform.h"

class QgsPostgresProvider;
class QgsPostgresResult;
class QgsPostgresTransaction;


class QgsPostgresFeatureSource final: public QgsAbstractFeatureSource
{
  public:
    explicit QgsPostgresFeatureSource( const QgsPostgresProvider *p );
    ~QgsPostgresFeatureSource() override;

    QgsFeatureIterator getFeatures( const QgsFeatureRequest &request ) override;

  private:

    QString mConnInfo;

    QString mGeometryColumn;
    QString mBoundingBoxColumn;
    QString mSqlWhereClause;
    QgsFields mFields;
    QgsPostgresGeometryColumnType mSpatialColType;
    QString mRequestedSrid;
    QString mDetectedSrid;
    QgsWkbTypes::Type mRequestedGeomType; //!< Geometry type requested in the uri
    QgsWkbTypes::Type mDetectedGeomType;  //!< Geometry type detected in the database
    QgsPostgresPrimaryKeyType mPrimaryKeyType;
    QList<int> mPrimaryKeyAttrs;
    QString mQuery;
    // TODO: loadFields()
    QgsCoordinateReferenceSystem mCrs;

    std::shared_ptr<QgsPostgresSharedData> mShared;

    /* The transaction connection (if any) gets refed/unrefed when creating/
     * destroying the QgsPostgresFeatureSource, to ensure that the transaction
     * connection remains valid during the life time of the feature source
     * even if the QgsPostgresTransaction object which initially created the
     * connection has since been destroyed. */
    QgsPostgresConn *mTransactionConnection = nullptr;

    friend class QgsPostgresFeatureIterator;
    friend class QgsPostgresExpressionCompiler;
};


class QgsPostgresConn;

class QgsPostgresFeatureIterator final: public QgsAbstractFeatureIteratorFromSource<QgsPostgresFeatureSource>
{
  public:
    QgsPostgresFeatureIterator( QgsPostgresFeatureSource *source, bool ownSource, const QgsFeatureRequest &request );

    ~QgsPostgresFeatureIterator() override;

    bool rewind() override;
    bool close() override;

  protected:
    bool fetchFeature( QgsFeature &feature ) override;
    bool nextFeatureFilterExpression( QgsFeature &f ) override;
    bool prepareSimplification( const QgsSimplifyMethod &simplifyMethod ) override;

  private:

    QgsPostgresConn *mConn = nullptr;


    QString whereClauseRect();
    bool getFeature( QgsPostgresResult &queryResult, int row, QgsFeature &feature );
    void getFeatureAttribute( int idx, QgsPostgresResult &queryResult, int row, int &col, QgsFeature &feature );
    bool declareCursor( const QString &whereClause, long limit = -1, bool closeOnFail = true, const QString &orderBy = QString() );

    QString mCursorName;

    /**
     * Feature queue that GetNextFeature will retrieve from
     * before the next fetch from PostgreSQL
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
    std::unique_ptr< QgsGeometryEngine > mDistanceWithinEngine;

};

#endif // QGSPOSTGRESFEATUREITERATOR_H
