/***************************************************************************
  qgsoraclefeatureiterator.h -  QGIS data provider for Oracle layers
                             -------------------
    begin                : December 2012
    copyright            : (C) 2012 by Juergen E. Fischer
    email                : jef at norbit dot de
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSORACLEFEATUREITERATOR_H
#define QGSORACLEFEATUREITERATOR_H

#include "qgsfeatureiterator.h"

#include <QSqlQuery>

#include "qgsoracleprovider.h"
#include "qgscoordinatetransform.h"

class QgsOracleConn;
class QgsOracleProvider;


class QgsOracleFeatureSource final: public QgsAbstractFeatureSource
{
  public:
    explicit QgsOracleFeatureSource( const QgsOracleProvider *p );
    ~QgsOracleFeatureSource() override;
    QgsFeatureIterator getFeatures( const QgsFeatureRequest &request ) override;

  protected:
    QgsDataSourceUri mUri;
    QgsFields mFields;

    QString mGeometryColumn;          //!< Name of the geometry column
    int mSrid;                        //!< Srid of column
    bool mHasSpatialIndex;            //!< Has spatial index of geometry column
    QgsWkbTypes::Type mDetectedGeomType;  //!< Geometry type detected in the database
    QgsWkbTypes::Type mRequestedGeomType; //!< Geometry type requested in the uri
    QString mSqlWhereClause;
    QgsOraclePrimaryKeyType mPrimaryKeyType;
    QList<int> mPrimaryKeyAttrs;
    QString mQuery;
    QgsCoordinateReferenceSystem mCrs;

    std::shared_ptr<QgsOracleSharedData> mShared;

    /* The transaction connection (if any) gets refed/unrefed when creating/
     * destroying the QgsOracleFeatureSource, to ensure that the transaction
     * connection remains valid during the life time of the feature source
     * even if the QgsOracleFeatureSource object which initially created the
     * connection has since been destroyed.
    */
    QgsOracleConn *mTransactionConnection = nullptr;


    friend class QgsOracleFeatureIterator;
    friend class QgsOracleExpressionCompiler;
};


class QgsOracleFeatureIterator final: public QgsAbstractFeatureIteratorFromSource<QgsOracleFeatureSource>
{
  public:
    QgsOracleFeatureIterator( QgsOracleFeatureSource *source, bool ownSource, const QgsFeatureRequest &request );

    ~QgsOracleFeatureIterator() override;

    bool rewind() override;
    bool close() override;

  protected:
    bool fetchFeature( QgsFeature &feature ) override;
    bool nextFeatureFilterExpression( QgsFeature &f ) override;

    bool openQuery( const QString &whereClause, const QVariantList &args, bool showLog = true );

    bool execQuery( const QString &query, const QVariantList &args, int retryCount = 0 );

    inline void lock();
    inline void unlock();

    QgsOracleConn *mConnection = nullptr;
    QSqlQuery mQry;
    bool mRewind = false;
    bool mExpressionCompiled = false;
    bool mFetchGeometry = false;
    QgsAttributeList mAttributeList;
    QString mSql;
    QVariantList mArgs;

    QgsCoordinateTransform mTransform;
    QgsRectangle mFilterRect;

    QgsGeometry mDistanceWithinGeom;
    std::unique_ptr< QgsGeometryEngine > mDistanceWithinEngine;

    bool mIsTransactionConnection = false;
};

#endif // QGSORACLEFEATUREITERATOR_H
