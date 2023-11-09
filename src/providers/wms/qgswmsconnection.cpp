/***************************************************************************
    qgswmsconnection.cpp  -  selector for WMS servers, etc.
                             -------------------
    begin                : 3 April 2005
    copyright            :
    original             : (C) 2005 by Brendan Morley email  : morb at ozemail dot com dot au
    wms search           : (C) 2009 Mathias Walker <mwa at sourcepole.ch>, Sourcepole AG
    wms-c support        : (C) 2010 Juergen E. Fischer < jef at norbit dot de >, norBIT GmbH

 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "../providers/wms/qgswmsprovider.h"
#include "qgis.h" // GEO_EPSG_CRS_ID
#include "qgsdatasourceuri.h"
#include "qgsproject.h"
#include "qgsproviderregistry.h"
#include "qgswmsconnection.h"
#include "qgsnetworkaccessmanager.h"
#include "qgssettings.h"
#include "qgsowsconnection.h"

static const QString SERVICE_WMS = QStringLiteral( "WMS" );


QgsWMSConnection::QgsWMSConnection( const QString &connName )
  : QgsOwsConnection( SERVICE_WMS, connName )
{
}

QStringList QgsWMSConnection::connectionList()
{
  return QgsOwsConnection::connectionList( SERVICE_WMS );
}

QString QgsWMSConnection::selectedConnection()
{
  return QgsOwsConnection::selectedConnection( SERVICE_WMS );
}

void QgsWMSConnection::setSelectedConnection( const QString &name )
{
  QgsOwsConnection::setSelectedConnection( SERVICE_WMS, name );
}

void QgsWMSConnection::deleteConnection( const QString &name )
{
  QgsOwsConnection::deleteConnection( SERVICE_WMS, name );
}
