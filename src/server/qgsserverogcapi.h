/***************************************************************************
  qgsserverogcapi.h - QgsServerOgcApi

 ---------------------
 begin                : 10.7.2019
 copyright            : (C) 2019 by ale
 email                : [your-email-here]
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSSERVEROGCAPI_H
#define QGSSERVEROGCAPI_H

#include "qgsserverapi.h"
#include "qgis_server.h"

/**
 * \ingroup server
 * Server OGC API endpoint abstract base class.
 *
 * \since QGIS 3.10
 */
class SERVER_EXPORT QgsServerOgcApi : public QgsServerApi
{
  public:
    QgsServerOgcApi( QgsServerInterface *serverIface );
};

#endif // QGSSERVEROGCAPI_H
