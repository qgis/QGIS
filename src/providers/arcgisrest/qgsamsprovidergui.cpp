/***************************************************************************
  qgsamsprovidergui.cpp
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

#include "qgsapplication.h"
#include "qgsproviderguimetadata.h"
#include "qgssourceselectprovider.h"

#include "qgsamsdataitemguiprovider.h"
#include "qgsamsprovider.h"
#include "qgsamssourceselect.h"


//! Provider for AMS layers source select
class QgsAmsSourceSelectProvider : public QgsSourceSelectProvider
{
  public:

    QString providerKey() const override { return QStringLiteral( "arcgismapserver" ); }
    QString text() const override { return QObject::tr( "ArcGIS Map Server" ); }
    int ordering() const override { return QgsSourceSelectProvider::OrderRemoteProvider + 140; }
    QIcon icon() const override { return QgsApplication::getThemeIcon( QStringLiteral( "/mActionAddAmsLayer.svg" ) ); }
    QgsAbstractDataSourceWidget *createDataSourceWidget( QWidget *parent = nullptr, Qt::WindowFlags fl = Qt::Widget, QgsProviderRegistry::WidgetMode widgetMode = QgsProviderRegistry::WidgetMode::Embedded ) const override
    {
      return new QgsAmsSourceSelect( parent, fl, widgetMode );
    }
};


class QgsAmsProviderGuiMetadata: public QgsProviderGuiMetadata
{
  public:
    QgsAmsProviderGuiMetadata()
      : QgsProviderGuiMetadata( QgsAmsProvider::AMS_PROVIDER_KEY )
    {
    }

    QList<QgsDataItemGuiProvider *> dataItemGuiProviders() override
    {
      QList<QgsDataItemGuiProvider *> providers;
      providers << new QgsAmsDataItemGuiProvider();
      return providers;
    }

    QList<QgsSourceSelectProvider *> sourceSelectProviders() override
    {
      QList<QgsSourceSelectProvider *> providers;
      providers << new QgsAmsSourceSelectProvider;
      return providers;
    }
};


QGISEXTERN QgsProviderGuiMetadata *providerGuiMetadataFactory()
{
  return new QgsAmsProviderGuiMetadata();
}
