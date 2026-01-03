/***************************************************************************
    qgsstacsourceselectprovider.cpp
    ---------------------
    begin                : October 2024
    copyright            : (C) 2024 by Stefanos Natsis
    email                : uclaros at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsstacsourceselectprovider.h"

#include "qgsapplication.h"
#include "qgsstacsourceselect.h"

QgsStacSourceSelectProvider::QgsStacSourceSelectProvider()
  : QgsSourceSelectProvider()
{
}

QString QgsStacSourceSelectProvider::providerKey() const
{
  return u"stac"_s;
}

QString QgsStacSourceSelectProvider::text() const
{
  return QObject::tr( "STAC" );
}

QString QgsStacSourceSelectProvider::toolTip() const
{
  return QObject::tr( "Spatio-Temporal Asset Catalog" );
}

QIcon QgsStacSourceSelectProvider::icon() const
{
  return QgsApplication::getThemeIcon( u"mIconStac.svg"_s );
}

QgsAbstractDataSourceWidget *QgsStacSourceSelectProvider::createDataSourceWidget( QWidget *parent, Qt::WindowFlags fl, QgsProviderRegistry::WidgetMode widgetMode ) const
{
  return new QgsStacSourceSelect( parent, fl, widgetMode );
}

int QgsStacSourceSelectProvider::ordering() const
{
  return OrderRemoteProvider + 201;
}
