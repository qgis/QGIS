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

#include <QString>
#include <QVariant>

#include "odbc/Forwards.h"
#include "odbc/ResultSet.h"
#include "odbc/ResultSetMetaDataUnicode.h"

class QgsHanaResultSet;
typedef std::unique_ptr<QgsHanaResultSet> QgsHanaResultSetRef;

class QgsHanaResultSet
{
  private:
    friend class QgsHanaConnection;

  private:
    QgsHanaResultSet( odbc::ResultSetRef &&resultSet );

  public:
    static QgsHanaResultSetRef create( odbc::PreparedStatementRef &stmt );
    static QgsHanaResultSetRef create( odbc::StatementRef &stmt, const QString &sql );

    void close();
    bool next();

    double getDouble( unsigned short columnIndex );
    int getInt( unsigned short columnIndex );
    short getShort( unsigned short columnIndex );
    QString getString( unsigned short columnIndex );
    QVariant getValue( unsigned short columnIndex );
    QgsGeometry getGeometry( unsigned short columnIndex );

    odbc::ResultSetMetaDataUnicode &getMetadata() { return *mMetadata; }

  private:
    odbc::ResultSetRef mResultSet;
    odbc::ResultSetMetaDataUnicodeRef mMetadata;
};

#endif // QGSHANARESULTSET_H
