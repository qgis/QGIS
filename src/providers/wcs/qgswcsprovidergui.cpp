/***************************************************************************
  qgswcsprovidergui.cpp
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

#include "qgswcsprovidergui.h"

#include "qgsmaplayer.h"
#include "qgsowssourcewidget.h"
#include "qgsproviderguimetadata.h"
#include "qgsprovidersourcewidgetprovider.h"
#include "qgssourceselectprovider.h"
#include "qgswcsdataitemguiprovider.h"
#include "qgswcsprovider.h"
#include "qgswcssourceselect.h"

//! Provider for WCS layers source select
class QgsWcsSourceSelectProvider : public QgsSourceSelectProvider
{
  public:
    QString providerKey() const override { return u"wcs"_s; }
    QString text() const override { return QObject::tr( "WCS" ); }
    int ordering() const override { return QgsSourceSelectProvider::OrderRemoteProvider + 30; }
    QIcon icon() const override { return QgsApplication::getThemeIcon( u"/mActionAddWcsLayer.svg"_s ); }
    QgsAbstractDataSourceWidget *createDataSourceWidget( QWidget *parent = nullptr, Qt::WindowFlags fl = Qt::Widget, QgsProviderRegistry::WidgetMode widgetMode = QgsProviderRegistry::WidgetMode::Embedded ) const override
    {
      return new QgsWCSSourceSelect( parent, fl, widgetMode );
    }
};


QgsWcsSourceWidgetProvider::QgsWcsSourceWidgetProvider() {}

QString QgsWcsSourceWidgetProvider::providerKey() const
{
  return u"wcs"_s;
}
bool QgsWcsSourceWidgetProvider::canHandleLayer( QgsMapLayer *layer ) const
{
  return layer->providerType() == "wcs"_L1;
}

QgsProviderSourceWidget *QgsWcsSourceWidgetProvider::createWidget( QgsMapLayer *layer, QWidget *parent )
{
  if ( layer->providerType() != "wcs"_L1 )
    return nullptr;

  QgsOWSSourceWidget *sourceWidget = new QgsOWSSourceWidget( "wcs"_L1, parent );

  return sourceWidget;
}


QgsWcsProviderGuiMetadata::QgsWcsProviderGuiMetadata()
  : QgsProviderGuiMetadata( QgsWcsProvider::WCS_KEY )
{
}

QList<QgsSourceSelectProvider *> QgsWcsProviderGuiMetadata::sourceSelectProviders()
{
  QList<QgsSourceSelectProvider *> providers;
  providers << new QgsWcsSourceSelectProvider;
  return providers;
}

QList<QgsDataItemGuiProvider *> QgsWcsProviderGuiMetadata::dataItemGuiProviders()
{
  return QList<QgsDataItemGuiProvider *>()
         << new QgsWcsDataItemGuiProvider;
}

QList<QgsProviderSourceWidgetProvider *> QgsWcsProviderGuiMetadata::sourceWidgetProviders()
{
  QList<QgsProviderSourceWidgetProvider *> providers;
  providers << new QgsWcsSourceWidgetProvider();
  return providers;
}


#ifndef HAVE_STATIC_PROVIDERS
QGISEXTERN QgsProviderGuiMetadata *providerGuiMetadataFactory()
{
  return new QgsWcsProviderGuiMetadata();
}
#endif
