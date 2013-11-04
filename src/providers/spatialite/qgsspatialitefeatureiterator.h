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

class QgsSpatiaLiteProvider;

class QgsSpatiaLiteFeatureIterator : public QgsAbstractFeatureIterator
{
  public:
    QgsSpatiaLiteFeatureIterator( QgsSpatiaLiteProvider* p, const QgsFeatureRequest& request );

    ~QgsSpatiaLiteFeatureIterator();

    //! reset the iterator to the starting position
    virtual bool rewind();

    //! end of iterating: free the resources / lock
    virtual bool close();

  protected:

    //! fetch next feature, return true on success
    virtual bool fetchFeature( QgsFeature& feature );

    QgsSpatiaLiteProvider* P;

    QString whereClauseRect();
    QString whereClauseFid();
    QString mbr( const QgsRectangle& rect );
    bool prepareStatement( QString whereClause );
    QString quotedPrimaryKey();
    bool getFeature( sqlite3_stmt *stmt, QgsFeature &feature );
    QString fieldName( const QgsField& fld );
    QVariant getFeatureAttribute( sqlite3_stmt* stmt, int ic, const QVariant::Type& type );
    void getFeatureGeometry( sqlite3_stmt* stmt, int ic, QgsFeature& feature );

    /**
      * SQLite statement handle
     */
    sqlite3_stmt *sqliteStatement;

    /** geometry column index used when fetching geometry */
    int mGeomColIdx;

    //! Set to true, if geometry is in the requested columns
    bool mFetchGeometry;
};

#endif // QGSSPATIALITEFEATUREITERATOR_H
