/***************************************************************************
  qgspostgresprovidergui.cpp
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

#include "qgspostgresprovidergui.h"
#include "qgsapplication.h"
#include "qgsproviderguimetadata.h"
#include "qgspgsourceselect.h"
#include "qgssourceselectprovider.h"
#include "qgspostgresprovider.h"
#include "qgspostgresdataitems.h"
#include "qgsprojectstorageguiprovider.h"
#include "qgspostgresprojectstoragedialog.h"
#include "qgspostgresdataitemguiprovider.h"
#include "raster/qgspostgresrastertemporalsettingswidget.h"


//! Provider for postgres source select
class QgsPostgresSourceSelectProvider : public QgsSourceSelectProvider  //#spellok
{
  public:

    QString providerKey() const override { return QStringLiteral( "postgres" ); }
    QString text() const override { return QObject::tr( "PostgreSQL" ); }
    int ordering() const override { return QgsSourceSelectProvider::OrderDatabaseProvider + 20; }
    QIcon icon() const override { return QgsApplication::getThemeIcon( QStringLiteral( "/mActionAddPostgisLayer.svg" ) ); }
    QgsAbstractDataSourceWidget *createDataSourceWidget( QWidget *parent = nullptr, Qt::WindowFlags fl = Qt::Widget, QgsProviderRegistry::WidgetMode widgetMode = QgsProviderRegistry::WidgetMode::Embedded ) const override
    {
      return new QgsPgSourceSelect( parent, fl, widgetMode );
    }
};


class QgsPostgresProjectStorageGuiProvider : public QgsProjectStorageGuiProvider
{
  public:
    QString type() override { return QStringLiteral( "postgresql" ); }
    QString visibleName() override
    {
      return QObject::tr( "PostgreSQL" );
    }

    QString showLoadGui() override
    {
      QgsPostgresProjectStorageDialog dlg( false );
      if ( !dlg.exec() )
        return QString();

      return dlg.currentProjectUri();
    }

    QString showSaveGui() override
    {
      QgsPostgresProjectStorageDialog dlg( true );
      if ( !dlg.exec() )
        return QString();

      return dlg.currentProjectUri();
    }

};


QgsPostgresProviderGuiMetadata::QgsPostgresProviderGuiMetadata():
  QgsProviderGuiMetadata( QgsPostgresProvider::POSTGRES_KEY )
{
  mRasterTemporalWidgetFactory = std::make_unique< QgsPostgresRasterTemporalSettingsConfigWidgetFactory>();
}

QList<QgsSourceSelectProvider *> QgsPostgresProviderGuiMetadata::sourceSelectProviders()
{
  QList<QgsSourceSelectProvider *> providers;
  providers << new QgsPostgresSourceSelectProvider;  //#spellok
  return providers;
}

QList<QgsDataItemGuiProvider *> QgsPostgresProviderGuiMetadata::dataItemGuiProviders()
{
  return QList<QgsDataItemGuiProvider *>()
         << new QgsPostgresDataItemGuiProvider;
}

QList<QgsProjectStorageGuiProvider *> QgsPostgresProviderGuiMetadata::projectStorageGuiProviders()
{
  QList<QgsProjectStorageGuiProvider *> providers;
  providers << new QgsPostgresProjectStorageGuiProvider;
  return providers;
}

QList<const QgsMapLayerConfigWidgetFactory *> QgsPostgresProviderGuiMetadata::mapLayerConfigWidgetFactories()
{
  return { mRasterTemporalWidgetFactory.get() };
}

#ifndef HAVE_STATIC_PROVIDERS
QGISEXTERN QgsProviderGuiMetadata *providerGuiMetadataFactory()
{
  return new QgsPostgresProviderGuiMetadata();
}
#endif
