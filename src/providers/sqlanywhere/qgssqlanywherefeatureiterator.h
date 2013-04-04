/***************************************************************************
    qgssqlanywherefeatureiterator.h - QGIS feature iterator for SQL Anywhere DBMS
    --------------------------
    begin                : Jan 2013
    copyright            : (C) 2013 by SAP AG or an SAP affiliate company.
    author               : David DeHaan, Mary Steele
    email                : ddehaan at sybase dot com

 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 3 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSSQLANYWHEREFEATUREITERATOR_H
#define QGSSQLANYWHEREFEATUREITERATOR_H

#include "qgsfeatureiterator.h"
#include "sqlanystatement.h"

class QgsSqlAnywhereProvider;
class QgsSqlAnywhereResult;

class QgsSqlAnywhereFeatureIterator : public QgsAbstractFeatureIterator
{
  public:
    QgsSqlAnywhereFeatureIterator( QgsSqlAnywhereProvider* p, const QgsFeatureRequest & request );

    ~QgsSqlAnywhereFeatureIterator();

    //! fetch next feature, return true on success
    virtual bool nextFeature( QgsFeature& feature );
    bool nextFeature( QgsFeature& feature, SqlAnyStatement *stmt );

    //! reset the iterator to the starting position
    virtual bool rewind();

    //! end of iterating: free the resources / lock
    virtual bool close();

  protected:
    QgsSqlAnywhereProvider* P;

  private:
    bool prepareStatement( QString whereClause );

    QString whereClauseRect() const;

    QString quotedPrimaryKey() const;
    QString whereClauseFid() const;

    /**
     * Statement handle for fetching of features by bounding rectangle
     */
    SqlAnyStatement *mStmt;

    QgsRectangle mStmtRect;

};

#endif // QGSSQLANYWHEREFEATUREITERATOR_H
