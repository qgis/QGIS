/***************************************************************************
    qgsdamengtransaction.h  -  Transaction support for DamengSQL/DAMENG layers
                             -------------------
    begin                : 2025/01/14
    copyright            : ( C ) 2025 by Haiyang Zhao
    email                : zhaohaiyang@dameng.com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   ( at your option ) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSDAMENGTRANSACTION_H
#define QGSDAMENGTRANSACTION_H

#include "qgstransaction.h"

class QgsDamengConn;


class QgsDamengTransaction : public QgsTransaction
{
    Q_OBJECT

  public:
    explicit QgsDamengTransaction( const QString &connString );
    ~QgsDamengTransaction();

    bool executeSql( const QString &sql, QString &error, bool isDirty = false, const QString &name = QString() ) override;

    QString createSavepoint( const QString &savePointId, QString &error ) override;
    bool rollbackToSavepoint( const QString &name, QString &error ) override;

    QgsDamengConn *connection() const { return mConn; }

  private:
    QgsDamengConn *mConn = nullptr;

    bool beginTransaction( QString &error, int statementTimeout ) override;
    bool commitTransaction( QString &error ) override;
    bool rollbackTransaction( QString &error ) override;

};

#endif // QGSDAMENGTRANSACTION_H
