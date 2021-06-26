/***************************************************************************
                          qgsgpsconnectionregistry.cpp  -  description
                          ----------------------------
    begin                : December 27th, 2009
    copyright            : (C) 2009 by Marco Hugentobler
    email                : marco at hugis dot net
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   *
 *  
 *        *
 *                                     *
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
  return qgis::setToList( mConnections );
}
