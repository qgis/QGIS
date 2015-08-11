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
    QgsSpatiaLiteFeatureSource( const QgsSpatiaLiteProvider* p );
    ~QgsSpatiaLiteFeatureSource();

    virtual QgsFeatureIterator getFeatures( const QgsFeatureRequest& request ) override;

  protected:
    QString mGeometryColumn;
    QString mSubsetString;
    QgsFields mFields;
    QString mQuery;
    bool isQuery;
    bool mVShapeBased;
    QString mIndexTable;
    QString mIndexGeometry;
    QString mPrimaryKey;
    bool spatialIndexRTree;
    bool spatialIndexMbrCache;
    QString mSqlitePath;

    friend class QgsSpatiaLiteFeatureIterator;
};

class QgsSpatiaLiteFeatureIterator : public QgsAbstractFeatureIteratorFromSource<QgsSpatiaLiteFeatureSource>
{
  public:
    QgsSpatiaLiteFeatureIterator( QgsSpatiaLiteFeatureSource* source, bool ownSource, const QgsFeatureRequest& request );

    ~QgsSpatiaLiteFeatureIterator();

    //! reset the iterator to the starting position
    virtual bool rewind() override;

    //! end of iterating: free the resources / lock
    virtual bool close() override;

  protected:

    //! fetch next feature, return true on success
    virtual bool fetchFeature( QgsFeature& feature ) override;

    QString whereClauseRect();
    QString whereClauseFid();
    QString whereClauseFids();
    QString mbr( const QgsRectangle& rect );
    bool prepareStatement( QString whereClause );
    QString quotedPrimaryKey();
    bool getFeature( sqlite3_stmt *stmt, QgsFeature &feature );
    QString fieldName( const QgsField& fld );
    QVariant getFeatureAttribute( sqlite3_stmt* stmt, int ic, const QVariant::Type& type );
    void getFeatureGeometry( sqlite3_stmt* stmt, int ic, QgsFeature& feature );

    //! wrapper of the SQLite database connection
    QgsSqliteHandle* mHandle;

    /**
      * SQLite statement handle
     */
    sqlite3_stmt *sqliteStatement;

    /** Geometry column index used when fetching geometry */
    int mGeomColIdx;

    //! Set to true, if geometry is in the requested columns
    bool mFetchGeometry;

    bool mHasPrimaryKey;
    QgsFeatureId mRowNumber;
};

#endif // QGSSPATIALITEFEATUREITERATOR_H
