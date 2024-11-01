/***************************************************************************
  qgsmdalprovidergui.cpp
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

#include "qgsmdalprovider.h"
#include "qgsmdalsourceselect.h"


//! Provider for mdal mesh source select
class QgsMdalMeshSourceSelectProvider : public QgsSourceSelectProvider
{
  public:
    QString providerKey() const override { return QStringLiteral( "mdal" ); }
    QString text() const override { return QObject::tr( "Mesh" ); }
    int ordering() const override { return QgsSourceSelectProvider::OrderLocalProvider + 22; }
    QIcon icon() const override { return QgsApplication::getThemeIcon( QStringLiteral( "/mActionAddMeshLayer.svg" ) ); }
    QgsAbstractDataSourceWidget *createDataSourceWidget( QWidget *parent = nullptr, Qt::WindowFlags fl = Qt::Widget, QgsProviderRegistry::WidgetMode widgetMode = QgsProviderRegistry::WidgetMode::Embedded ) const override
    {
      return new QgsMdalSourceSelect( parent, fl, widgetMode );
    }
};


class QgsMdalProviderGuiMetadata : public QgsProviderGuiMetadata
{
  public:
    QgsMdalProviderGuiMetadata()
      : QgsProviderGuiMetadata( QgsMdalProvider::MDAL_PROVIDER_KEY )
    {
    }

    QList<QgsSourceSelectProvider *> sourceSelectProviders() override
    {
      QList<QgsSourceSelectProvider *> providers;
      providers << new QgsMdalMeshSourceSelectProvider;
      return providers;
    }
};


QGISEXTERN QgsProviderGuiMetadata *providerGuiMetadataFactory()
{
  return new QgsMdalProviderGuiMetadata();
}
