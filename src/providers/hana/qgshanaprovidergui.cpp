/***************************************************************************
   qgshanaprovidergui.cpp
   --------------------------------------
   Date      : 31-05-2019
   Copyright : (C) SAP SE
   Author    : Maxim Rylov
 ***************************************************************************/

/***************************************************************************
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 ***************************************************************************/

#include "qgsapplication.h"
#include "qgshanadataitems.h"
#include "qgshanadataitemguiprovider.h"
#include "qgshanasourceselect.h"
#include "qgshanaprovider.h"
#include "qgsproviderguimetadata.h"
#include "qgssourceselectprovider.h"

class QgsHanaSourceSelectProvider : public QgsSourceSelectProvider
{
  public:
    QString providerKey() const override { return QgsHanaProvider::HANA_KEY; }

    QString text() const override { return QObject::tr( "SAP HANA" ); }

    int ordering() const override { return QgsSourceSelectProvider::OrderDatabaseProvider + 70; }

    QIcon icon() const override { return QgsApplication::getThemeIcon( QStringLiteral( "/mActionAddHanaLayer.svg" ) ); }

    QgsAbstractDataSourceWidget *createDataSourceWidget( QWidget *parent = nullptr, Qt::WindowFlags fl = Qt::Widget, QgsProviderRegistry::WidgetMode widgetMode = QgsProviderRegistry::WidgetMode::Embedded ) const override
    {
      return new QgsHanaSourceSelect( parent, fl, widgetMode );
    }
};

class QgsHanaProviderGuiMetadata : public QgsProviderGuiMetadata
{
  public:
    QgsHanaProviderGuiMetadata()
      : QgsProviderGuiMetadata( QgsHanaProvider::HANA_KEY )
    {
    }

    QList<QgsSourceSelectProvider *> sourceSelectProviders() override
    {
      QList<QgsSourceSelectProvider *> providers;
      providers << new QgsHanaSourceSelectProvider;
      return providers;
    }

    QList<QgsDataItemGuiProvider *> dataItemGuiProviders() override
    {
      return QList<QgsDataItemGuiProvider *>()
             << new QgsHanaDataItemGuiProvider;
    }
};

QGISEXTERN QgsProviderGuiMetadata *providerGuiMetadataFactory()
{
  return new QgsHanaProviderGuiMetadata();
}
