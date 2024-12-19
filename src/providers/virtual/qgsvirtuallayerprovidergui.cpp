/***************************************************************************
  qgsvirtuallayerprovidergui.cpp
  --------------------------------------
  Date                 : Octpner 2021
  Copyright            : (C) 2021 by Mathieu Pellerin
  Email                : nirvn dot asia at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsvirtuallayerprovidergui.h"

#include "qgsapplication.h"
#include "qgssourceselectprovider.h"
#include "qgsvirtuallayerprovider.h"
#include "qgsvirtuallayersourceselect.h"

//! Provider for virtual layers source select
class QgsVirtualSourceSelectProvider : public QgsSourceSelectProvider
{
  public:

    QString providerKey() const override { return QStringLiteral( "virtual" ); }
    QString text() const override { return QObject::tr( "Virtual Layer" ); }
    int ordering() const override { return QgsSourceSelectProvider::OrderDatabaseProvider + 60; }
    QString toolTip() const override { return QObject::tr( "Add Virtual Layer" ); }
    QIcon icon() const override { return QgsApplication::getThemeIcon( QStringLiteral( "/mActionAddVirtualLayer.svg" ) ); }
    QgsAbstractDataSourceWidget *createDataSourceWidget( QWidget *parent = nullptr, Qt::WindowFlags fl = Qt::Widget, QgsProviderRegistry::WidgetMode widgetMode = QgsProviderRegistry::WidgetMode::Embedded ) const override
    {
      return new QgsVirtualLayerSourceSelect( parent, fl, widgetMode );
    }
};


QgsVirtualLayerProviderGuiMetadata::QgsVirtualLayerProviderGuiMetadata()
  : QgsProviderGuiMetadata( QgsVirtualLayerProvider::VIRTUAL_LAYER_KEY )
{
}

QList<QgsSourceSelectProvider *> QgsVirtualLayerProviderGuiMetadata::sourceSelectProviders()
{
  QList<QgsSourceSelectProvider *> providers;
  providers << new QgsVirtualSourceSelectProvider;
  return providers;
}


#ifndef HAVE_STATIC_PROVIDERS
QGISEXTERN QgsProviderGuiMetadata *providerGuiMetadataFactory()
{
  return new QgsVirtualLayerProviderGuiMetadata();
}
#endif
