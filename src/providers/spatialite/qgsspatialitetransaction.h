/***************************************************************************
  qgsspatialitetransaction.h - QgsSpatialiteTransaction

 ---------------------
 begin                : 30.3.2020
 copyright            : (C) 2020 by Alessandro Pasotti
 email                : elpaso at itopen dot it
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSSPATIALITETRANSACTION_H
#define QGSSPATIALITETRANSACTION_H

#include "qgstransaction.h"
#include "qgsspatialiteconnection.h"
#include "qgis_sip.h"

///@cond PRIVATE
#define SIP_NO_FILE

class QgsSpatiaLiteTransaction : public QgsTransaction
{
    Q_OBJECT

  public:
    explicit QgsSpatiaLiteTransaction( const QString &connString, QgsSqliteHandle *sharedHandle );

    /**
     * Executes the SQL query in database.
     *
     * \param sql The SQL query to execute
     * \param error The error or an empty string if none
     * \param isDirty True to add an undo/redo command in the edition buffer, false otherwise
     * \param name Name of the operation ( only used if `isDirty` is true)
     */
    bool executeSql( const QString &sql, QString &error, bool isDirty = false, const QString &name = QString() ) override;

    /**
     * Returns the (possibly NULL) sqlite handle
     */
    sqlite3 *sqliteHandle() const;

  private:
    QgsSqliteHandle *mSharedHandle = nullptr;
    int mSavepointId;

    //! SQLite handle
    sqlite3 *mSqliteHandle = nullptr;

    bool beginTransaction( QString &error, int statementTimeout ) override;
    bool commitTransaction( QString &error ) override;
    bool rollbackTransaction( QString &error ) override;

    static QAtomicInt sSavepointId;
};

///@endcond
#endif // QGSSPATIALITETRANSACTION_H
