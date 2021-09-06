/***************************************************************************
  qgsmssqltransaction.h - QgsMssqlTransaction
 ---------------------
 begin                : 11.3.2021
 copyright            : (C) 2021 by Vincent Cloarec
 email                : vcloarec at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSMSSQLTRANSACTION_H
#define QGSMSSQLTRANSACTION_H

#include "qgstransaction.h"
#include <memory>

class QgsMssqlDatabase;


class QgsMssqlTransaction : public QgsTransaction
{
    Q_OBJECT
  public:
    QgsMssqlTransaction( const QString &connString );
    ~QgsMssqlTransaction();

    virtual bool executeSql( const QString &sql, QString &error, bool isDirty = false, const QString &name = QString() ) override;

    QString createSavepoint( const QString &savePointId, QString &error ) override;
    bool rollbackToSavepoint( const QString &name, QString &error ) override;

    std::shared_ptr<QgsMssqlDatabase> conn() { return mConn; }

  private:
    virtual bool beginTransaction( QString &error, int statementTimeout ) override;
    virtual bool commitTransaction( QString &error ) override;
    virtual bool rollbackTransaction( QString &error ) override;

  private:
    //! connection is primarily owned by this class, but it may be also shared with QgsMssqlFeatureSource
    std::shared_ptr<QgsMssqlDatabase> mConn;
};


#endif // QGSMSSQLTRANSACTION_H
