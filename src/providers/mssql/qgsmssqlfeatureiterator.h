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

#include "qgsmssqlprovider.h"
#include "qgsfeatureiterator.h"
#include <QtSql/QSqlDatabase>
#include <QtSql/QSqlQuery>
#include <QtSql/QSqlError>

class QgsMssqlProvider;

class QgsMssqlFeatureIterator : public QgsAbstractFeatureIterator
{
  public:
    QgsMssqlFeatureIterator( QgsMssqlProvider* provider, const QgsFeatureRequest& request );

    ~QgsMssqlFeatureIterator();

    //! fetch next feature, return true on success
    virtual bool nextFeature( QgsFeature& feature );

    //! reset the iterator to the starting position
    virtual bool rewind();

    //! end of iterating: free the resources / lock
    virtual bool close();

  protected:
    QgsMssqlProvider* mProvider;

    void BuildStatement( const QgsFeatureRequest& request );

  private:
    // The current database
    QSqlDatabase mDatabase;

    // The current sql query
    QSqlQuery* mQuery;

    // Use query on provider (no new connection added)
    bool mUseProviderQuery;

    // The current sql statement
    QString mStatement;

    // Open connection flag
    bool mIsOpen;

    // Field index of FID column
    long mFidCol;

    // Field index of geometry column
    long mGeometryCol;

    // List of attribute indices to fetch with nextFeature calls
    QgsAttributeList mAttributesToFetch;
};

#endif // QGSMSSQLFEATUREITERATOR_H
