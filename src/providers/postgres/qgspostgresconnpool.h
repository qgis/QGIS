/***************************************************************************
    qgspostgresconnpool.h
    ---------------------
    begin                : January 2014
    copyright            : (C) 2014 by Martin Dobias
    email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSPOSTGRESCONNPOOL_H
#define QGSPOSTGRESCONNPOOL_H

#include "qgsconnectionpool.h"
#include "qgspostgresconn.h"
#include "qgssqlconnpool.h"


// QObject currently does not support templating (due to the macros Q_OBJECT, slots),
// hence this class explicitly specializes QgsSqlConnectionPoolGroup.
class QgsPostgresConnPoolGroup : public QObject, public QgsSqlConnectionPoolGroup<QgsPostgresConn>
{
    Q_OBJECT
  public:
    explicit QgsPostgresConnPoolGroup( const QString &name ) : QgsSqlConnectionPoolGroup<QgsPostgresConn>( name, this ) {}
  protected slots:
    void handleConnectionExpired() { onConnectionExpired(); }
    void startExpirationTimer() { mExpirationTimer->start(); }
    void stopExpirationTimer() { mExpirationTimer->stop(); }
  protected:
    Q_DISABLE_COPY( QgsPostgresConnPoolGroup )
};


//! PostgreSQL connection pool - singleton
using QgsPostgresConnPool = QgsSqlConnectionPool<QgsPostgresConn, QgsPostgresConnPoolGroup>;

#endif // QGSPOSTGRESCONNPOOL_H
