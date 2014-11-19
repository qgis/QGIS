/***************************************************************************
                         qgsmssqlfeatureiterator.h  -  description
                         -------------------
    begin                : 2011-10-08
    copyright            : (C) 2011 by Tamas Szekeres
    email                : szekerest at gmail.com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSMSSQLFEATUREITERATOR_H
#define QGSMSSQLFEATUREITERATOR_H

#include "qgsmssqlgeometryparser.h"
#include "qgsfeatureiterator.h"
#include <QtSql/QSqlDatabase>
#include <QtSql/QSqlQuery>
#include <QtSql/QSqlError>

class QgsMssqlProvider;

class QgsMssqlFeatureSource : public QgsAbstractFeatureSource
{
  public:
    QgsMssqlFeatureSource( const QgsMssqlProvider* p );
    ~QgsMssqlFeatureSource();

    virtual QgsFeatureIterator getFeatures( const QgsFeatureRequest& request );

  protected:
    QgsFields mFields;
    QString mFidColName;
    long mSRId;

    /* sql geo type */
    bool mIsGeography;

    QString mGeometryColName;
    QString mGeometryColType;

    // current layer name
    QString mSchemaName;
    QString mTableName;

    // login
    QString mUserName;
    QString mPassword;

    // server access
    QString mService;
    QString mDatabaseName;
    QString mHost;

    // SQL statement used to limit the features retrieved
    QString mSqlWhereClause;

    // Return True if this feature source has spatial attributes.
    bool isSpatial() { return !mGeometryColName.isEmpty() || !mGeometryColType.isEmpty(); }

    friend class QgsMssqlFeatureIterator;
};

class QgsMssqlFeatureIterator : public QgsAbstractFeatureIteratorFromSource<QgsMssqlFeatureSource>
{
  public:
    QgsMssqlFeatureIterator( QgsMssqlFeatureSource* source, bool ownSource, const QgsFeatureRequest& request );

    ~QgsMssqlFeatureIterator();

    //! reset the iterator to the starting position
    virtual bool rewind();

    //! end of iterating: free the resources / lock
    virtual bool close();

  protected:
    void BuildStatement( const QgsFeatureRequest& request );

  private:
    //! fetch next feature, return true on success
    virtual bool fetchFeature( QgsFeature& feature );

    // The current database
    QSqlDatabase mDatabase;

    // The current sql query
    QSqlQuery* mQuery;

    // The current sql statement
    QString mStatement;

    // Field index of FID column
    long mFidCol;

    // List of attribute indices to fetch with nextFeature calls
    QgsAttributeList mAttributesToFetch;

    // for parsing sql geometries
    QgsMssqlGeometryParser mParser;
};

#endif // QGSMSSQLFEATUREITERATOR_H
