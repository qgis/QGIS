/***************************************************************************
  qgsdb2providergui.cpp
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

#include "qgsdb2dataitemguiprovider.h"
#include "qgsdb2provider.h"
#include "qgsdb2sourceselect.h"


//! Provider for DB2 source select
class QgsDb2SourceSelectProvider : public QgsSourceSelectProvider
{
  public:

    QString providerKey() const override { return QgsDb2Provider::DB2_PROVIDER_KEY; }
    QString text() const override { return QObject::tr( "DB2" ); }
    int ordering() const override { return QgsSourceSelectProvider::OrderDatabaseProvider + 50; }
    QIcon icon() const override { return QgsApplication::getThemeIcon( QStringLiteral( "/mActionAddDb2Layer.svg" ) ); }
    QgsAbstractDataSourceWidget *createDataSourceWidget( QWidget *parent = nullptr, Qt::WindowFlags fl = Qt::Widget, QgsProviderRegistry::WidgetMode widgetMode = QgsProviderRegistry::WidgetMode::Embedded ) const override
    {
      return new QgsDb2SourceSelect( parent, fl, widgetMode );
    }
};


class QgsDb2ProviderGuiMetadata: public QgsProviderGuiMetadata
{
  public:
    QgsDb2ProviderGuiMetadata()
      : QgsProviderGuiMetadata( QgsDb2Provider::DB2_PROVIDER_KEY )
    {
    }

    QList<QgsDataItemGuiProvider *> dataItemGuiProviders() override
    {
      QList<QgsDataItemGuiProvider *> providers;
      providers << new QgsDb2DataItemGuiProvider;
      return providers;
    }

    QList<QgsSourceSelectProvider *> sourceSelectProviders() override
    {
      QList<QgsSourceSelectProvider *> providers;
      providers << new QgsDb2SourceSelectProvider;
      return providers;
    }
};


QGISEXTERN QgsProviderGuiMetadata *providerGuiMetadataFactory()
{
  return new QgsDb2ProviderGuiMetadata();
}
