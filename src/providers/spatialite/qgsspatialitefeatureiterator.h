/***************************************************************************
    qgsspatialitefeatureiterator.h
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
#ifndef QGSSPATIALITEFEATUREITERATOR_H
#define QGSSPATIALITEFEATUREITERATOR_H

#include "qgsfeatureiterator.h"
#include "qgsfields.h"
#include "qgscoordinatetransform.h"
#include "qgsdbquerylog.h"

extern "C"
{
//#include <sys/types.h>
#include <sqlite3.h>
}

class QgsSqliteHandle;
class QgsSpatiaLiteProvider;

class QgsSpatiaLiteFeatureSource final: public QgsAbstractFeatureSource
{
  public:
    explicit QgsSpatiaLiteFeatureSource( const QgsSpatiaLiteProvider *p );

    QgsFeatureIterator getFeatures( const QgsFeatureRequest &request ) override;

    sqlite3 *transactionHandle();

  private:
    QString mGeometryColumn;
    QString mSubsetString;
    QgsFields mFields;
    QString mQuery;
    bool mIsQuery;
    bool mViewBased;
    bool mVShapeBased;
    QString mIndexTable;
    QString mIndexGeometry;
    QString mPrimaryKey;
    bool mSpatialIndexRTree;
    bool mSpatialIndexMbrCache;
    QString mSqlitePath;
    QgsCoordinateReferenceSystem mCrs;
    sqlite3 *mTransactionHandle = nullptr;

    friend class QgsSpatiaLiteFeatureIterator;
    friend class QgsSpatiaLiteExpressionCompiler;
};

class QgsSpatiaLiteFeatureIterator final: public QgsAbstractFeatureIteratorFromSource<QgsSpatiaLiteFeatureSource>
{
  public:
    QgsSpatiaLiteFeatureIterator( QgsSpatiaLiteFeatureSource *source, bool ownSource, const QgsFeatureRequest &request );

    ~QgsSpatiaLiteFeatureIterator() override;
    bool rewind() override;
    bool close() override;

  protected:

    bool fetchFeature( QgsFeature &feature ) override;
    bool nextFeatureFilterExpression( QgsFeature &f ) override;

  private:

    QString whereClauseRect();
    QString whereClauseFid();
    QString whereClauseFids();
    QString mbr( const QgsRectangle &rect );
    bool prepareStatement( const QString &whereClause, long limit = -1, const QString &orderBy = QString() );
    QString quotedPrimaryKey();
    bool getFeature( sqlite3_stmt *stmt, QgsFeature &feature );
    QString fieldName( const QgsField &fld );
    QVariant getFeatureAttribute( sqlite3_stmt *stmt, int ic, QMetaType::Type type, QMetaType::Type subType );
    void getFeatureGeometry( sqlite3_stmt *stmt, int ic, QgsFeature &feature );

    //! QGIS wrapper of the SQLite database connection
    QgsSqliteHandle *mHandle = nullptr;
    //! The low level connection
    sqlite3 *mSqliteHandle = nullptr;

    /**
      * SQLite statement handle
     */
    sqlite3_stmt *sqliteStatement = nullptr;

    //! Geometry column index used when fetching geometry
    int mGeomColIdx = 1;

    //! Sets to true, if geometry is in the requested columns
    bool mFetchGeometry = true;

    bool mHasPrimaryKey;
    QgsFeatureId mRowNumber;

    bool prepareOrderBy( const QList<QgsFeatureRequest::OrderByClause> &orderBys ) override;

    bool mOrderByCompiled = false;
    bool mExpressionCompiled = false;

    QgsRectangle mFilterRect;
    QgsCoordinateTransform mTransform;
    QgsGeometry mDistanceWithinGeom;
    std::unique_ptr< QgsGeometryEngine > mDistanceWithinEngine;

    // Last prepared sql statement for logging purposes
    QString mLastSql;

    std::unique_ptr<QgsDatabaseQueryLogWrapper> mQueryLogWrapper;
};

#endif // QGSSPATIALITEFEATUREITERATOR_H
