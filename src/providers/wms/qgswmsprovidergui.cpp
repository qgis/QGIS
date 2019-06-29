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

#include "qgswmsprovider.h"
#include "qgswmssourceselect.h"
#include "qgssourceselectprovider.h"
#include "qgstilescalewidget.h"
#include "qgsproviderguimetadata.h"
#include "qgswmsdataitemguiproviders.h"


//! Provider for WMS layers source select
class QgsWmsSourceSelectProvider : public QgsSourceSelectProvider
{
  public:

    QString providerKey() const override { return QStringLiteral( "wms" ); }
    QString text() const override { return QStringLiteral( "WMS/WMTS" ); } // untranslatable string as acronym for this particular case. Use QObject::tr() otherwise
    int ordering() const override { return QgsSourceSelectProvider::OrderRemoteProvider + 10; }
    QIcon icon() const override { return QgsApplication::getThemeIcon( QStringLiteral( "/mActionAddWmsLayer.svg" ) ); }
    QgsAbstractDataSourceWidget *createDataSourceWidget( QWidget *parent = nullptr, Qt::WindowFlags fl = Qt::Widget, QgsProviderRegistry::WidgetMode widgetMode = QgsProviderRegistry::WidgetMode::Embedded ) const override
    {
      return new QgsWMSSourceSelect( parent, fl, widgetMode );
    }
};


class QgsWmsProviderGuiMetadata: public QgsProviderGuiMetadata
{
  public:
    QgsWmsProviderGuiMetadata(): QgsProviderGuiMetadata( QgsWmsProvider::WMS_KEY ) {}
    QList<QgsSourceSelectProvider *> sourceSelectProviders() override
    {
      QList<QgsSourceSelectProvider *> providers;
      providers << new QgsWmsSourceSelectProvider;
      return providers;
    }

    QList<QgsDataItemGuiProvider *> dataItemGuiProviders() override
    {
      return QList<QgsDataItemGuiProvider *>()
             << new QgsWmsDataItemGuiProvider
             << new QgsXyzDataItemGuiProvider;
    }

    void registerGui( QMainWindow *widget ) override
    {
      QgsTileScaleWidget::showTileScale( widget );
    }
};


QGISEXTERN QgsProviderGuiMetadata *providerGuiMetadataFactory()
{
  return new QgsWmsProviderGuiMetadata();
}
