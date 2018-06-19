/***************************************************************************
    qgspostgrestransaction.h  -  Transaction support for PostgreSQL/PostGIS layers
                             -------------------
    begin                : Jan 8, 2015
    copyright            : (C) 2015 by Sandro Mani
    email                : manisandro at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSPOSTGRESTRANSACTION_H
#define QGSPOSTGRESTRANSACTION_H

#include "qgstransaction.h"

class QgsPostgresConn;


class QgsPostgresTransaction : public QgsTransaction
{
    Q_OBJECT

  public:
    explicit QgsPostgresTransaction( const QString &connString );

    /**
     * Executes the SQL query in database.
     *
     * \param sql The SQL query to execute
     * \param error The error or an empty string if none
     * \param isDirty True to add an undo/redo command in the edition buffer, false otherwise
     * \param name Name of the operation ( only used if `isDirty` is true)
     */
    bool executeSql( const QString &sql, QString &error, bool isDirty = false, const QString &name = QString() ) override;

    QgsPostgresConn *connection() const { return mConn; }


  private:
    QgsPostgresConn *mConn = nullptr;

    bool beginTransaction( QString &error, int statementTimeout ) override;
    bool commitTransaction( QString &error ) override;
    bool rollbackTransaction( QString &error ) override;

};

#endif // QGSPOSTGRESTRANSACTION_H
