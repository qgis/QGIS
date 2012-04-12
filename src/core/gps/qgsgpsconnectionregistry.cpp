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

QgsGPSConnectionRegistry* QgsGPSConnectionRegistry::mInstance = 0;

QgsGPSConnectionRegistry::QgsGPSConnectionRegistry()
{

}

QgsGPSConnectionRegistry::~QgsGPSConnectionRegistry()
{
  QSet<QgsGPSConnection*>::iterator it = mConnections.begin();
  for ( ; it != mConnections.end(); ++it )
  {
    delete *it;
  }
}

QgsGPSConnectionRegistry* QgsGPSConnectionRegistry::instance()
{
  if ( !mInstance )
  {
    mInstance = new QgsGPSConnectionRegistry();
  }
  return mInstance;
}

void QgsGPSConnectionRegistry::registerConnection( QgsGPSConnection* c )
{
  mConnections.insert( c );
}

void QgsGPSConnectionRegistry::unregisterConnection( QgsGPSConnection* c )
{
  mConnections.remove( c );
}

QList< QgsGPSConnection* > QgsGPSConnectionRegistry::connectionList() const
{
  return mConnections.toList();
}
