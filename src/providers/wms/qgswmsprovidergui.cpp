/***************************************************************************
  qgswmsprovidergui.cpp
  --------------------------------------
  Date                 : June 2019
  Copyright            : (C) 2019 by Martin Dobias
  Email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgswmsprovidergui.h"

#include "qgsmaplayer.h"
#include "qgsproviderguimetadata.h"
#include "qgsprovidersourcewidgetprovider.h"
#include "qgssourceselectprovider.h"
#include "qgstilescalewidget.h"
#include "qgswmsdataitemguiproviders.h"
#include "qgswmsdataitems.h"
#include "qgswmsprovider.h"
#include "qgswmssourceselect.h"
#include "qgswmstsettingswidget.h"
#include "qgsxyzsourceselect.h"
#include "qgsxyzsourcewidget.h"

//! Provider for WMS layers source select
class QgsWmsSourceSelectProvider : public QgsSourceSelectProvider
{
  public:
    QString providerKey() const override { return u"wms"_s; }
    QString text() const override { return u"WMS/WMTS"_s; } // untranslatable string as acronym for this particular case. Use QObject::tr() otherwise
    int ordering() const override { return QgsSourceSelectProvider::OrderRemoteProvider + 10; }
    QIcon icon() const override { return QgsApplication::getThemeIcon( u"/mActionAddWmsLayer.svg"_s ); }
    QgsAbstractDataSourceWidget *createDataSourceWidget( QWidget *parent = nullptr, Qt::WindowFlags fl = Qt::Widget, QgsProviderRegistry::WidgetMode widgetMode = QgsProviderRegistry::WidgetMode::Embedded ) const override
    {
      return new QgsWMSSourceSelect( parent, fl, widgetMode );
    }
};

class QgsXyzSourceSelectProvider : public QgsSourceSelectProvider
{
  public:
    QString providerKey() const override { return u"xyz"_s; }
    QString text() const override { return u"XYZ"_s; } // untranslatable string as acronym for this particular case. Use QObject::tr() otherwise
    int ordering() const override { return QgsSourceSelectProvider::OrderRemoteProvider + 40; }
    QIcon icon() const override { return QgsApplication::getThemeIcon( u"/mActionAddXyzLayer.svg"_s ); }
    QgsAbstractDataSourceWidget *createDataSourceWidget( QWidget *parent = nullptr, Qt::WindowFlags fl = Qt::Widget, QgsProviderRegistry::WidgetMode widgetMode = QgsProviderRegistry::WidgetMode::Embedded ) const override
    {
      return new QgsXyzSourceSelect( parent, fl, widgetMode );
    }
};

class QgsXyzSourceWidgetProvider : public QgsProviderSourceWidgetProvider
{
  public:
    QgsXyzSourceWidgetProvider()
      : QgsProviderSourceWidgetProvider() {}
    QString providerKey() const override
    {
      return u"xyz"_s;
    }
    bool canHandleLayer( QgsMapLayer *layer ) const override
    {
      if ( layer->providerType() != "wms"_L1 )
        return false;

      const QVariantMap parts = QgsProviderRegistry::instance()->decodeUri( u"wms"_s, layer->source() );
      if ( parts.value( u"type"_s ).toString() != "xyz"_L1 )
        return false;

      return true;
    }
    QgsProviderSourceWidget *createWidget( QgsMapLayer *layer, QWidget *parent = nullptr ) override
    {
      if ( layer->providerType() != "wms"_L1 )
        return nullptr;

      const QVariantMap parts = QgsProviderRegistry::instance()->decodeUri( u"wms"_s, layer->source() );
      if ( parts.value( u"type"_s ).toString() != "xyz"_L1 )
        return nullptr;

      return new QgsXyzSourceWidget( parent );
    }
};


QgsWmsProviderGuiMetadata::QgsWmsProviderGuiMetadata()
  : QgsProviderGuiMetadata( QgsWmsProvider::WMS_KEY )
{
  mWmstConfigWidgetFactory = std::make_unique<QgsWmstSettingsConfigWidgetFactory>();
}

QList<QgsSourceSelectProvider *> QgsWmsProviderGuiMetadata::sourceSelectProviders()
{
  QList<QgsSourceSelectProvider *> providers;
  providers << new QgsWmsSourceSelectProvider << new QgsXyzSourceSelectProvider;
  return providers;
}

QList<QgsDataItemGuiProvider *> QgsWmsProviderGuiMetadata::dataItemGuiProviders()
{
  return QList<QgsDataItemGuiProvider *>()
         << new QgsWmsDataItemGuiProvider
         << new QgsXyzDataItemGuiProvider;
}

QList<QgsProviderSourceWidgetProvider *> QgsWmsProviderGuiMetadata::sourceWidgetProviders()
{
  QList<QgsProviderSourceWidgetProvider *> providers;
  providers << new QgsXyzSourceWidgetProvider();
  return providers;
}

void QgsWmsProviderGuiMetadata::registerGui( QMainWindow *widget )
{
  QgsTileScaleWidget::showTileScale( widget );
}

QList<const QgsMapLayerConfigWidgetFactory *> QgsWmsProviderGuiMetadata::mapLayerConfigWidgetFactories()
{
  return { mWmstConfigWidgetFactory.get() };
}

#ifndef HAVE_STATIC_PROVIDERS
QGISEXTERN QgsProviderGuiMetadata *providerGuiMetadataFactory()
{
  return new QgsWmsProviderGuiMetadata();
}
#endif
