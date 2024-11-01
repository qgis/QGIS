/***************************************************************************
                          qgsserverapi.cpp

  Class defining the service interface for QGIS server APIs.
  -------------------
  begin                : 2019-04-16
  copyright            : (C) 2019 by Alessandro Pasotti
  email                : elpaso at itopen dot it
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/


#include "qgsserverapi.h"

QgsServerApi::QgsServerApi( QgsServerInterface *serverIface )
  : mServerIface( serverIface )
{
}

bool QgsServerApi::accept( const QUrl &url ) const
{
  return url.path().contains( rootPath() );
}

QgsServerInterface *QgsServerApi::serverIface() const
{
  return mServerIface;
}
