/***************************************************************************
   qgshanaresultset.h
   --------------------------------------
   Date      : 31-05-2019
   Copyright : (C) SAP SE
   Author    : Maxim Rylov
 ***************************************************************************/

/***************************************************************************
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 ***************************************************************************/
#ifndef QGSHANARESULTSET_H
#define QGSHANARESULTSET_H

#include "qgsgeometry.h"

#include <memory>
#include <vector>

#include <QString>
#include <QVariant>

#include "odbc/PreparedStatement.h"
#include "odbc/Statement.h"
#include "odbc/ResultSet.h"
#include "odbc/ResultSetMetaData.h"

using namespace odbc;

class QgsHanaResultSet;
typedef std::unique_ptr<QgsHanaResultSet> QgsHanaResultSetRef;

class QgsHanaResultSet
{
  private:
    QgsHanaResultSet( ResultSetRef &&resultSet );

  public:
    static QgsHanaResultSetRef create( PreparedStatementRef &stmt );
    static QgsHanaResultSetRef create( StatementRef &stmt, const QString &sql );

    void close();
    bool next();
    QVariant getValue( unsigned short columnIndex );
    QgsGeometry getGeometry( unsigned short columnIndex );
    ResultSetMetaDataRef getMetadata() { return mResultSet->getMetaData() ; }

  private:
    void ensureBufferCapacity( std::size_t capacity );

  private:
    ResultSetRef mResultSet;
    ResultSetMetaDataRef mMetadata;
    std::vector<unsigned char> mBuffer;
};

#endif // QGSHANARESULTSET_H
