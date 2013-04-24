/***************************************************************************
    qgsowsprovider.cpp  -  OWS meta provider for WMS,WFS,WCS in browser
                         -------------------
    begin                : 4/2012
    copyright            : (C) 2010 by Radim Blazek
    email                : radim dot blazek at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgslogger.h"
#include "qgsowsprovider.h"
#include "qgsconfig.h"

#include <QString>

static QString PROVIDER_KEY = "ows";
static QString PROVIDER_DESCRIPTION = "OWS meta provider";

QgsOwsProvider::QgsOwsProvider( QString const & uri )
    : QgsDataProvider( uri )
{
}

QgsOwsProvider::~QgsOwsProvider()
{
}

QGISEXTERN QgsOwsProvider * classFactory( const QString *uri )
{
  return new QgsOwsProvider( *uri );
}

QString QgsOwsProvider::name() const
{
  return PROVIDER_KEY;
}

QString QgsOwsProvider::description() const
{
  return PROVIDER_DESCRIPTION;
}

QGISEXTERN QString providerKey()
{
  return PROVIDER_KEY;
}

QGISEXTERN QString description()
{
  return PROVIDER_DESCRIPTION;
}

QGISEXTERN bool isProvider()
{
  return true;
}

