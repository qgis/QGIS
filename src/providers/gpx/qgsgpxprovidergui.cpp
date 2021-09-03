/***************************************************************************
  qgsgpxprovidergui.cpp
  --------------------------------------
  Date                 : July 2021
  Copyright            : (C) 2021 by Nyall Dawson
  Email                : nyall dot dawson at gmail dot com
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

#include "qgsgpxprovider.h"
#include "qgsgpxsourceselect.h"


//! Provider for GPX source select
class QgsGpxSourceSelectProvider : public QgsSourceSelectProvider
{
  public:

    QString providerKey() const override { return QStringLiteral( "gpx" ); }
    QString text() const override { return QObject::tr( "GPS" ); }
    int ordering() const override { return QgsSourceSelectProvider::OrderLocalProvider + 65; }
    QIcon icon() const override { return QgsApplication::getThemeIcon( QStringLiteral( "/mActionAddGpsLayer.svg" ) ); }
    QgsAbstractDataSourceWidget *createDataSourceWidget( QWidget *parent = nullptr, Qt::WindowFlags fl = Qt::Widget, QgsProviderRegistry::WidgetMode widgetMode = QgsProviderRegistry::WidgetMode::Embedded ) const override
    {
      return new QgsGpxSourceSelect( parent, fl, widgetMode );
    }
};


class QgsGpxProviderGuiMetadata: public QgsProviderGuiMetadata
{
  public:
    QgsGpxProviderGuiMetadata()
      : QgsProviderGuiMetadata( QStringLiteral( "gpx" ) )
    {
    }

    QList<QgsSourceSelectProvider *> sourceSelectProviders() override
    {
      QList<QgsSourceSelectProvider *> providers;
      providers << new QgsGpxSourceSelectProvider;
      return providers;
    }
};


QGISEXTERN QgsProviderGuiMetadata *providerGuiMetadataFactory()
{
  return new QgsGpxProviderGuiMetadata();
}
