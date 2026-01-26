/***************************************************************************
  qgswfsprovidergui.cpp
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

#include "qgswfsprovidergui.h"

#include "qgsproviderguimetadata.h"
#include "qgsquerybuilder.h"
#include "qgssourceselectprovider.h"
#include "qgssubsetstringeditorprovider.h"
#include "qgswfsdataitemguiprovider.h"
#include "qgswfsprovider.h"
#include "qgswfssourceselect.h"
#include "qgswfssubsetstringeditor.h"

//! Provider for WFS layers source select
class QgsWfsSourceSelectProvider : public QgsSourceSelectProvider
{
  public:
    QString providerKey() const override { return QgsWFSProvider::WFS_PROVIDER_KEY; }
    QString text() const override { return QObject::tr( "WFS / OGC API - Features" ); }
    int ordering() const override { return QgsSourceSelectProvider::OrderRemoteProvider + 20; }
    QIcon icon() const override { return QgsApplication::getThemeIcon( u"/mActionAddWfsLayer.svg"_s ); }
    QgsAbstractDataSourceWidget *createDataSourceWidget( QWidget *parent = nullptr, Qt::WindowFlags fl = Qt::Widget, QgsProviderRegistry::WidgetMode widgetMode = QgsProviderRegistry::WidgetMode::Embedded ) const override
    {
      return new QgsWFSSourceSelect( parent, fl, widgetMode );
    }
};

//! Provider for dedicated subset string editor for WFS layers
class QgsWfsSubsetStringEditorProvider : public QgsSubsetStringEditorProvider
{
  public:
    QString providerKey() const override { return QgsWFSProvider::WFS_PROVIDER_KEY; }

    bool canHandleLayer( QgsVectorLayer *layer ) const override
    {
      QgsDataProvider *provider = layer->dataProvider();
      return static_cast<bool>( dynamic_cast<QgsWFSProvider *>( provider ) );
    }

    QgsSubsetStringEditorInterface *createDialog( QgsVectorLayer *layer, QWidget *parent, Qt::WindowFlags fl ) override
    {
      QgsDataProvider *provider = layer->dataProvider();
      QgsWFSProvider *wfsProvider = dynamic_cast<QgsWFSProvider *>( provider );
      if ( !wfsProvider )
        return nullptr;

      // If we have an existing full SQL request, open the SQL editor
      // Otherwise use the standard expression builder.
      const QString subsetString = wfsProvider->subsetString();
      if ( subsetString.startsWith( "SELECT "_L1, Qt::CaseInsensitive ) || subsetString.startsWith( "SELECT\t"_L1, Qt::CaseInsensitive ) || subsetString.startsWith( "SELECT\r"_L1, Qt::CaseInsensitive ) || subsetString.startsWith( "SELECT\n"_L1, Qt::CaseInsensitive ) )
      {
        return QgsWfsSubsetStringEditor::create( layer, wfsProvider, parent, fl );
      }

      return new QgsQueryBuilder( layer, parent, fl );
    }
};


QgsWfsProviderGuiMetadata::QgsWfsProviderGuiMetadata()
  : QgsProviderGuiMetadata( QgsWFSProvider::WFS_PROVIDER_KEY )
{
}

QList<QgsSourceSelectProvider *> QgsWfsProviderGuiMetadata::sourceSelectProviders()
{
  QList<QgsSourceSelectProvider *> providers;
  providers << new QgsWfsSourceSelectProvider;
  return providers;
}

QList<QgsDataItemGuiProvider *> QgsWfsProviderGuiMetadata::dataItemGuiProviders()
{
  return QList<QgsDataItemGuiProvider *>()
         << new QgsWfsDataItemGuiProvider;
}

QList<QgsSubsetStringEditorProvider *> QgsWfsProviderGuiMetadata::subsetStringEditorProviders()
{
  return QList<QgsSubsetStringEditorProvider *>()
         << new QgsWfsSubsetStringEditorProvider;
}


#ifndef HAVE_STATIC_PROVIDERS
QGISEXTERN QgsProviderGuiMetadata *providerGuiMetadataFactory()
{
  return new QgsWfsProviderGuiMetadata();
}
#endif
