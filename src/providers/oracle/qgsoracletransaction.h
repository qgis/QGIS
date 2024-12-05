/***************************************************************************
    qgsoracletransaction.h
    -------------------
    begin                : August 2019
    copyright            : (C) 2019 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSORACLETRANSACTION_H
#define QGSORACLETRANSACTION_H

#include "qgstransaction.h"
#include "qgis_sip.h"

///@cond PRIVATE
#define SIP_NO_FILE

class QgsOracleConn;

class QgsOracleTransaction : public QgsTransaction
{
    Q_OBJECT

  public:
    explicit QgsOracleTransaction( const QString &connString );

    ~QgsOracleTransaction() override;

    bool executeSql( const QString &sql, QString &error, bool isDirty = false, const QString &name = QString() ) override;

    QgsOracleConn *connection() const { return mConn; }

  private:
    QgsOracleConn *mConn = nullptr;

    bool beginTransaction( QString &error, int statementTimeout ) override;
    bool commitTransaction( QString &error ) override;
    bool rollbackTransaction( QString &error ) override;
};

///@endcond
#endif // QGSORACLETRANSACTION_H
