/***************************************************************************
  qgsdb2geometrycolumns.h - Access DB2 geometry columns table
  --------------------------------------
  Date      : 2016-01-27
  Copyright : (C) 2016 by David Adler
                          Shirley Xiao, David Nguyen
  Email     : dadler at adtechgeospatial.com
              xshirley2012 at yahoo.com, davidng0123 at gmail.com
****************************************************************************
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 ***************************************************************************/

#ifndef QGSDB2GEOMETRYCOLUMNS_H
#define QGSDB2GEOMETRYCOLUMNS_H

#include "qgsdb2tablemodel.h" // needed for QgsDB2LayerProperty
#include <QtSql>

/**
 * @class QgsDb2GeometryColumns
 * @brief Data provider for DB2 server.
 */

static const int ENV_LUW = 1, ENV_ZOS = 2;

class QgsDb2GeometryColumns
{
  public:
    explicit QgsDb2GeometryColumns( const QSqlDatabase db );

    ~QgsDb2GeometryColumns();

    bool isActive();
    void close();
    int  open();
    int  open( const QString &schemaName, const QString &tableName );
    bool populateLayerProperty( QgsDb2LayerProperty &layer );
    int  db2Environment();

  private:

    QSqlDatabase mDatabase;
    QSqlQuery mQuery;
    int mEnvironment;

};

#endif
