/***************************************************************************
  qgsvectortileproviderguimetadata.cpp
  --------------------------------------
  Date                 : March 2020
  Copyright            : (C) 2020 by Martin Dobias
  Email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsapplication.h"
#include "qgssourceselectprovider.h"
#include "qgsvectortilesourceselect.h"
#include "qgsvectortileproviderguimetadata.h"
#include "qgsvectortiledataitemguiprovider.h"

///@cond PRIVATE

class QgsVectorTileSourceSelectProvider : public QgsSourceSelectProvider
{
  public:
    QString providerKey() const override { return QStringLiteral( "vectortile" ); }
    QString text() const override { return QObject::tr( "Vector Tile" ); }
    int ordering() const override { return QgsSourceSelectProvider::OrderRemoteProvider + 50; }
    QIcon icon() const override { return QgsApplication::getThemeIcon( QStringLiteral( "/mActionAddVectorTileLayer.svg" ) ); }
    QgsAbstractDataSourceWidget *createDataSourceWidget( QWidget *parent = nullptr, Qt::WindowFlags fl = Qt::Widget, QgsProviderRegistry::WidgetMode widgetMode = QgsProviderRegistry::WidgetMode::Embedded ) const override
    {
      return new QgsVectorTileSourceSelect( parent, fl, widgetMode );
    }
};

QgsVectorTileProviderGuiMetadata::QgsVectorTileProviderGuiMetadata()
  : QgsProviderGuiMetadata( QStringLiteral( "vectortile" ) )
{
}

QList<QgsDataItemGuiProvider *> QgsVectorTileProviderGuiMetadata::dataItemGuiProviders()
{
  return QList<QgsDataItemGuiProvider *>()
         << new QgsVectorTileDataItemGuiProvider;
}

QList<QgsSourceSelectProvider *> QgsVectorTileProviderGuiMetadata::sourceSelectProviders()
{
  QList<QgsSourceSelectProvider *> providers;
  providers << new QgsVectorTileSourceSelectProvider;
  return providers;
}

///@endcond
