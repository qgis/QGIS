/***************************************************************************
                          qgsgpsconnectionregistry.cpp  -  description
                          ----------------------------
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

#include "qgsgpsconnectionregistry.h"
#include "qgsgpsconnection.h"

QgsGpsConnectionRegistry::~QgsGpsConnectionRegistry()
{
  qDeleteAll( mConnections );
}

void QgsGpsConnectionRegistry::registerConnection( QgsGpsConnection *c )
{
  mConnections.insert( c );
}

void QgsGpsConnectionRegistry::unregisterConnection( QgsGpsConnection *c )
{
  mConnections.remove( c );
}

QList< QgsGpsConnection * > QgsGpsConnectionRegistry::connectionList() const
{
  return QList< QgsGpsConnection * >( mConnections.begin(), mConnections.end() );
}
