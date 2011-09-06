/***************************************************************************
    qgspostgresconnection.h  -  PostgresSQL/PostGIS connection
                             -------------------
    begin                : 3 June 2011
    copyright            : (C) 2011 by Giuseppe Sucameli
    email                : brush.tyler at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSPOSTGRESCONNECTION_H
#define QGSPOSTGRESCONNECTION_H

#include <QStringList>

class QgsPostgresProvider;

/*!
 * \brief   Connections management
 */
class QgsPostgresConnection : public QObject
{
    Q_OBJECT

  public:
    //! Constructor
    QgsPostgresConnection( QString theConnName );
    //! Destructor
    ~QgsPostgresConnection();

    static QStringList connectionList();

    static QString selectedConnection();
    static void setSelectedConnection( QString name );

  public:
    QgsPostgresProvider *provider();
    QString connectionInfo();
    QString mConnName;
    QString mConnectionInfo;
};


#endif // QGSPOSTGRESCONNECTION_H
