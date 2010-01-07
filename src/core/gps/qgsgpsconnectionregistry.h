/***************************************************************************
                          qgsgpsconnectionregistry.h  -  description
                          --------------------------
    begin                : December 27th, 2009
    copyright            : (C) 2009 by Marco Hugentobler
    email                : marco at hugis dot net
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSGPSCONNECTIONREGISTRY_H
#define QGSGPSCONNECTIONREGISTRY_H

#include <QList>
#include <QSet>

class QgsGPSConnection;

/**A singleton class to register / unregister existing GPS connections such that the information
  is available to all classes and plugins*/
class CORE_EXPORT QgsGPSConnectionRegistry
{
  public:
    static QgsGPSConnectionRegistry* instance();
    ~QgsGPSConnectionRegistry();

    /**Inserts a connection into the registry. The connection is owned by the registry class until it is unregistered again*/
    void registerConnection( QgsGPSConnection* c );
    /**Unregisters connection. The registry does no longer own the connection*/
    void unregisterConnection( QgsGPSConnection* c );

    QList< const QgsGPSConnection* > connectionList() const;


  protected:
    QgsGPSConnectionRegistry();

    static QgsGPSConnectionRegistry* mInstance;

    QSet<QgsGPSConnection*> mConnections;
};

#endif // QGSGPSCONNECTIONREGISTRY_H
