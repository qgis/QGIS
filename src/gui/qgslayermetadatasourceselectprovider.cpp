/***************************************************************************
  qgslayermetadatasourceselectprovider.cpp - QgsLayerMetadataSourceSelectProvider

 ---------------------
 begin                : 6.9.2022
 copyright            : (C) 2022 by Alessandro Pasotti
 email                : elpaso at itopen dot it
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "qgslayermetadatasourceselectprovider.h"
#include "qgslayermetadatasearchwidget.h"
#include "qgsapplication.h"

QgsLayerMetadataSourceSelectProvider::QgsLayerMetadataSourceSelectProvider()
  : QgsSourceSelectProvider()
{

}

QString QgsLayerMetadataSourceSelectProvider::providerKey() const
{
  return QStringLiteral( "layermetadata" );

}

QString QgsLayerMetadataSourceSelectProvider::text() const
{
  return QObject::tr( "Metadata Search" );
}

QIcon QgsLayerMetadataSourceSelectProvider::icon() const
{
  return QgsApplication::getThemeIcon( QStringLiteral( "search.svg" ) );
}

QgsAbstractDataSourceWidget *QgsLayerMetadataSourceSelectProvider::createDataSourceWidget( QWidget *parent, Qt::WindowFlags fl, QgsProviderRegistry::WidgetMode widgetMode ) const
{
  return new QgsLayerMetadataSearchWidget( parent, fl, widgetMode );
}

int QgsLayerMetadataSourceSelectProvider::ordering() const
{
  return OrderSearchProvider;
}
