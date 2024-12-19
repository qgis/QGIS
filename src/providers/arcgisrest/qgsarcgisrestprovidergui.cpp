/***************************************************************************
  qgsafsprovidergui.cpp
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

#include "qgsarcgisrestprovidergui.h"

#include "qgsapplication.h"
#include "qgsproviderguimetadata.h"
#include "qgssourceselectprovider.h"

#include "qgsarcgisrestdataitemguiprovider.h"
#include "qgsafsprovider.h"
#include "qgsarcgisrestsourceselect.h"
#include "qgsarcgisrestsourcewidget.h"
#include "qgsprovidersourcewidgetprovider.h"
#include "qgsmaplayer.h"

//! Provider for AFS layers source select
class QgsArcGisRestSourceSelectProvider : public QgsSourceSelectProvider
{
  public:

    QString providerKey() const override { return QgsAfsProvider::AFS_PROVIDER_KEY; }
    QString text() const override { return QObject::tr( "ArcGIS REST Server" ); }
    int ordering() const override { return QgsSourceSelectProvider::OrderRemoteProvider + 150; }
    QIcon icon() const override { return QgsApplication::getThemeIcon( QStringLiteral( "/mActionAddAfsLayer.svg" ) ); }
    QgsAbstractDataSourceWidget *createDataSourceWidget( QWidget *parent = nullptr, Qt::WindowFlags fl = Qt::Widget, QgsProviderRegistry::WidgetMode widgetMode = QgsProviderRegistry::WidgetMode::Embedded ) const override
    {
      return new QgsArcGisRestSourceSelect( parent, fl, widgetMode );
    }
};

class QgsArcGisRestSourceWidgetProvider : public QgsProviderSourceWidgetProvider
{
  public:
    QgsArcGisRestSourceWidgetProvider() : QgsProviderSourceWidgetProvider() {}
    QString providerKey() const override
    {
      return QgsAfsProvider::AFS_PROVIDER_KEY;
    }
    bool canHandleLayer( QgsMapLayer *layer ) const override
    {
      if ( layer->providerType() != QgsAfsProvider::AFS_PROVIDER_KEY && layer->providerType() != QLatin1String( "arcgismapserver" ) )
        return false;

      return true;
    }
    QgsProviderSourceWidget *createWidget( QgsMapLayer *layer, QWidget *parent = nullptr ) override
    {
      if ( layer->providerType() != QgsAfsProvider::AFS_PROVIDER_KEY && layer->providerType() != QLatin1String( "arcgismapserver" ) )
        return nullptr;

      return new QgsArcGisRestSourceWidget( layer->providerType(), parent );
    }
};


QgsArcGisRestProviderGuiMetadata::QgsArcGisRestProviderGuiMetadata()
  : QgsProviderGuiMetadata( QgsAfsProvider::AFS_PROVIDER_KEY )
{
}

QList<QgsDataItemGuiProvider *> QgsArcGisRestProviderGuiMetadata::dataItemGuiProviders()
{
  QList<QgsDataItemGuiProvider *> providers;
  providers << new QgsArcGisRestDataItemGuiProvider();
  return providers;
}

QList<QgsSourceSelectProvider *> QgsArcGisRestProviderGuiMetadata::sourceSelectProviders()
{
  QList<QgsSourceSelectProvider *> providers;
  providers << new QgsArcGisRestSourceSelectProvider;
  return providers;
}

QList<QgsProviderSourceWidgetProvider *> QgsArcGisRestProviderGuiMetadata::sourceWidgetProviders()
{
  QList<QgsProviderSourceWidgetProvider *> providers;
  providers << new QgsArcGisRestSourceWidgetProvider();
  return providers;
}


#ifndef HAVE_STATIC_PROVIDERS
QGISEXTERN QgsProviderGuiMetadata *providerGuiMetadataFactory()
{
  return new QgsArcGisRestProviderGuiMetadata();
}
#endif
