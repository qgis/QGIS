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
#include "qgsspatialiteutils.h"

extern "C"
{
//#include <sys/types.h>
#include <sqlite3.h>
}

class QgsSqliteHandle;
class QgsSpatiaLiteProvider;

class QgsSpatiaLiteFeatureSource : public QgsAbstractFeatureSource
{
  public:
    explicit QgsSpatiaLiteFeatureSource( const QgsSpatiaLiteProvider *p );

    virtual QgsFeatureIterator getFeatures( const QgsFeatureRequest &request ) override;

  private:
    //! wrapper of the SQLite database connection
    QgsSqliteHandle *mHandle = nullptr;
    SpatialiteDbInfo *mSpatialiteDbInfo = nullptr;
    SpatialiteDbLayer *mDbLayer = nullptr;
    //! The active Layer
    SpatialiteDbLayer *getDbLayer() const { return mDbLayer; }
    //! The sqlite handler
    sqlite3 *dbSqliteHandle() const { return getDbLayer()->dbSqliteHandle(); }
    QgsSqliteHandle *getQSqliteHandle() const { return mHandle; }
    //! Name of the geometry column in the table
    QString getGeometryColumn() const { return getDbLayer()->getGeometryColumn(); }
    //! List of layer fields in the table
    QgsFields getAttributeFields() const { return getDbLayer()->getAttributeFields(); }
    //! Name of the primary key column in the table
    QString getPrimaryKey() const { return getDbLayer()->getPrimaryKey(); }
    //! Name of the Layer format: 'table_name(geometry_name)'
    QString getLayerName() const { return getDbLayer()->getLayerName(); }
    //! The Spatialite Geometry-Type being read (as String)
    QString getGeometryTypeString() const { return getDbLayer()->getGeometryTypeString(); }
    //! The SpatialiIndex used for the Geometry
    int getSpatialIndexType() const { return getDbLayer()->getSpatialIndexType(); }
    QString mSubsetString;
    QString mQuery;
    bool mIsQuery;
    bool mViewBased;
    bool mVShapeBased;
    QString mIndexTable;
    QString mIndexGeometry;
    bool mSpatialIndexRTree;
    bool mSpatialIndexMbrCache;
    QString mSqlitePath;
    QgsCoordinateReferenceSystem mCrs;

    friend class QgsSpatiaLiteFeatureIterator;
    friend class QgsSpatiaLiteExpressionCompiler;
};

class QgsSpatiaLiteFeatureIterator : public QgsAbstractFeatureIteratorFromSource<QgsSpatiaLiteFeatureSource>
{
  public:
    QgsSpatiaLiteFeatureIterator( QgsSpatiaLiteFeatureSource *source, bool ownSource, const QgsFeatureRequest &request );

    ~QgsSpatiaLiteFeatureIterator();
    virtual bool rewind() override;
    virtual bool close() override;

  protected:

    virtual bool fetchFeature( QgsFeature &feature ) override;
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
    QVariant getFeatureAttribute( sqlite3_stmt *stmt, int ic, QVariant::Type type, QVariant::Type subType );
    void getFeatureGeometry( sqlite3_stmt *stmt, int ic, QgsFeature &feature );

    //! wrapper of the SQLite database connection
    QgsSqliteHandle *mHandle = nullptr;

    /**
      * SQLite statement handle
     */
    sqlite3_stmt *sqliteStatement = nullptr;

    //! Geometry column index used when fetching geometry
    int mGeomColIdx;

    //! Set to true, if geometry is in the requested columns
    bool mFetchGeometry;

    bool mHasPrimaryKey;
    QgsFeatureId mRowNumber;

    bool prepareOrderBy( const QList<QgsFeatureRequest::OrderByClause> &orderBys ) override;

    bool mOrderByCompiled;
    bool mExpressionCompiled;

    QgsRectangle mFilterRect;
    QgsCoordinateTransform mTransform;
};

#endif // QGSSPATIALITEFEATUREITERATOR_H
