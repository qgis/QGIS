/***************************************************************************
  qgspdalprovidergui.cpp
  --------------------------------------
  Date                 : November 2020
  Copyright            : (C) 2020 by Peter Petrik
  Email                : zilolv at gmail dot com
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

#include "qgspdalprovider.h"
#include "qgspdalsourceselect.h"


//! Provider for pdal source select
class QgsPdalSourceSelectProvider : public QgsSourceSelectProvider
{
  public:

    QString providerKey() const override { return QStringLiteral( "pdal" ); }
    QString text() const override { return QObject::tr( "PDAL Point Clouds" ); }
    int ordering() const override { return QgsSourceSelectProvider::OrderLocalProvider + 22; }
    QIcon icon() const override { return QgsApplication::getThemeIcon( QStringLiteral( "/mActionAddPdalLayer.svg" ) ); }
    QgsAbstractDataSourceWidget *createDataSourceWidget( QWidget *parent = nullptr, Qt::WindowFlags fl = Qt::Widget,
        QgsProviderRegistry::WidgetMode widgetMode = QgsProviderRegistry::WidgetMode::Embedded ) const override
    {
      return new QgsPdalSourceSelect( parent, fl, widgetMode );
    }
};


class QgsPdalProviderGuiMetadata: public QgsProviderGuiMetadata
{
  public:
    QgsPdalProviderGuiMetadata()
      : QgsProviderGuiMetadata( QStringLiteral( "pdal" ) )
    {
    }

    QList<QgsSourceSelectProvider *> sourceSelectProviders() override
    {
      QList<QgsSourceSelectProvider *> providers;
      providers << new QgsPdalSourceSelectProvider;
      return providers;
    }
};


QGISEXTERN QgsProviderGuiMetadata *providerGuiMetadataFactory()
{
  return new QgsPdalProviderGuiMetadata();
}
