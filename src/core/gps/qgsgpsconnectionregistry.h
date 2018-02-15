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

#include "qgis_core.h"

class QgsGpsConnection;

/**
 * \ingroup core
 * A class to register / unregister existing GPS connections such that the information
 * is available to all classes and plugins.
 *
 * QgsGpsConnectionRegistry is not usually directly created, but rather accessed through
 * QgsApplication::gpsConnectionRegistry().
*/
class CORE_EXPORT QgsGpsConnectionRegistry
{
  public:

    /**
     * Constructor for QgsGpsConnectionRegistry.
     */
    QgsGpsConnectionRegistry() = default;
    ~QgsGpsConnectionRegistry();

    //! QgsGpsConnectionRegistry cannot be copied.
    QgsGpsConnectionRegistry( const QgsGpsConnectionRegistry &rh ) = delete;
    //! QgsGpsConnectionRegistry cannot be copied.
    QgsGpsConnectionRegistry &operator=( const QgsGpsConnectionRegistry &rh ) = delete;

    //! Inserts a connection into the registry. The connection is owned by the registry class until it is unregistered again
    void registerConnection( QgsGpsConnection *c );
    //! Unregisters connection. The registry does no longer own the connection
    void unregisterConnection( QgsGpsConnection *c );

    QList< QgsGpsConnection *> connectionList() const;

  private:
#ifdef SIP_RUN
    QgsGpsConnectionRegistry( const QgsGpsConnectionRegistry &rh );
#endif

    QSet<QgsGpsConnection *> mConnections;
};

#endif // QGSGPSCONNECTIONREGISTRY_H
