/***************************************************************************
  qgsmssqlprovidergui.cpp
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

#include "qgsmssqldataitemguiprovider.h"
#include "qgsmssqlprovider.h"
#include "qgsmssqlsourceselect.h"


//! Provider for msssql raster source select
class QgsMssqlSourceSelectProvider : public QgsSourceSelectProvider
{
  public:

    QString providerKey() const override { return QStringLiteral( "mssql" ); }
    QString text() const override { return QObject::tr( "MS SQL Server" ); }
    int ordering() const override { return QgsSourceSelectProvider::OrderDatabaseProvider + 30; }
    QIcon icon() const override { return QgsApplication::getThemeIcon( QStringLiteral( "/mActionAddMssqlLayer.svg" ) ); }
    QgsAbstractDataSourceWidget *createDataSourceWidget( QWidget *parent = nullptr, Qt::WindowFlags fl = Qt::Widget, QgsProviderRegistry::WidgetMode widgetMode = QgsProviderRegistry::WidgetMode::Embedded ) const override
    {
      return new QgsMssqlSourceSelect( parent, fl, widgetMode );
    }
};


class QgsMssqlProviderGuiMetadata: public QgsProviderGuiMetadata
{
  public:
    QgsMssqlProviderGuiMetadata()
      : QgsProviderGuiMetadata( QgsMssqlProvider::MSSQL_PROVIDER_KEY )
    {
    }

    QList<QgsDataItemGuiProvider *> dataItemGuiProviders() override
    {
      return QList<QgsDataItemGuiProvider *>()
             << new QgsMssqlDataItemGuiProvider;
    }

    QList<QgsSourceSelectProvider *> sourceSelectProviders() override
    {
      QList<QgsSourceSelectProvider *> providers;
      providers << new QgsMssqlSourceSelectProvider;
      return providers;
    }
};


QGISEXTERN QgsProviderGuiMetadata *providerGuiMetadataFactory()
{
  return new QgsMssqlProviderGuiMetadata();
}
