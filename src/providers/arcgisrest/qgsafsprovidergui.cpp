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

#include "qgsapplication.h"
#include "qgsproviderguimetadata.h"
#include "qgssourceselectprovider.h"

#include "qgsafsdataitemguiprovider.h"
#include "qgsafsprovider.h"
#include "qgsafssourceselect.h"


//! Provider for AFS layers source select
class QgsAfsSourceSelectProvider : public QgsSourceSelectProvider
{
  public:

    QString providerKey() const override { return QgsAfsProvider::AFS_PROVIDER_KEY; }
    QString text() const override { return QObject::tr( "ArcGIS Feature Server" ); }
    int ordering() const override { return QgsSourceSelectProvider::OrderRemoteProvider + 150; }
    QIcon icon() const override { return QgsApplication::getThemeIcon( QStringLiteral( "/mActionAddAfsLayer.svg" ) ); }
    QgsAbstractDataSourceWidget *createDataSourceWidget( QWidget *parent = nullptr, Qt::WindowFlags fl = Qt::Widget, QgsProviderRegistry::WidgetMode widgetMode = QgsProviderRegistry::WidgetMode::Embedded ) const override
    {
      return new QgsAfsSourceSelect( parent, fl, widgetMode );
    }
};


class QgsAfsProviderGuiMetadata: public QgsProviderGuiMetadata
{
  public:
    QgsAfsProviderGuiMetadata()
      : QgsProviderGuiMetadata( QgsAfsProvider::AFS_PROVIDER_KEY )
    {
    }

    QList<QgsDataItemGuiProvider *> dataItemGuiProviders() override
    {
      QList<QgsDataItemGuiProvider *> providers;
      providers << new QgsAfsDataItemGuiProvider();
      return providers;
    }

    QList<QgsSourceSelectProvider *> sourceSelectProviders() override
    {
      QList<QgsSourceSelectProvider *> providers;
      providers << new QgsAfsSourceSelectProvider;
      return providers;
    }
};


QGISEXTERN QgsProviderGuiMetadata *providerGuiMetadataFactory()
{
  return new QgsAfsProviderGuiMetadata();
}
