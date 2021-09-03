/***************************************************************************
      qgsgdalguiprovider.cpp  - GUI for QGIS Data provider for GDAL rasters
                             -------------------
    begin                : June, 2019
    copyright            : (C) 2019 by Peter Petrik
    email                : zilolv at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsgdalguiprovider.h"
///@cond PRIVATE

#include <QList>
#include <QAction>
#include <QDesktopServices>
#include <QMenu>
#include <QMessageBox>

#include "qgsproject.h"
#include "qgsrasterlayer.h"
#include "qgssourceselectprovider.h"
#include "qgsgdalsourceselect.h"
#include "qgsapplication.h"
#include "qgsprovidermetadata.h"
#include "qgsdataitemguiprovider.h"
#include "qgsmaplayer.h"
#include "qgsgdalfilesourcewidget.h"
#include "qgsabstractdatabaseproviderconnection.h"
#include "qgslayeritem.h"

static QString PROVIDER_KEY = QStringLiteral( "gdal" );

QgsGdalItemGuiProvider::QgsGdalItemGuiProvider() = default;

QgsGdalItemGuiProvider::~QgsGdalItemGuiProvider() = default;

QString QgsGdalItemGuiProvider::name()
{
  return QStringLiteral( "gdal_items" );
}

void QgsGdalItemGuiProvider::onDeletePostgresRasterLayer( QgsDataItemGuiContext context )
{
  QAction *s = qobject_cast<QAction *>( sender() );
  QVariantMap data = s->data().toMap();
  const QString uri = data[QStringLiteral( "uri" )].toString();
  const QString path = data[QStringLiteral( "path" )].toString();
  const QPointer< QgsDataItem > parent = data[QStringLiteral( "parentItem" )].value<QPointer< QgsDataItem >>();

  const QString title = tr( "Delete Table" );

  // Check if the layer is in the project
  const QgsMapLayer *projectLayer = nullptr;
  const QMap<QString, QgsMapLayer *> mapLayers = QgsProject::instance()->mapLayers();
  for ( auto it = mapLayers.constBegin(); it != mapLayers.constEnd(); ++it )
  {
    if ( it.value()->publicSource() == uri )
    {
      projectLayer = it.value();
    }
  }

  if ( ! projectLayer )
  {
    const QString confirmMessage = tr( "Are you sure you want to delete table “%1”?" ).arg( path );

    if ( QMessageBox::question( nullptr, title,
                                confirmMessage,
                                QMessageBox::Yes | QMessageBox::No, QMessageBox::No ) != QMessageBox::Yes )
      return;

    QString errorMessage;
    bool deleted { false };
    QgsProviderMetadata *postgresMetadata { QgsProviderRegistry::instance()->providerMetadata( QLatin1String( "postgres" ) ) };
    if ( postgresMetadata )
    {
      std::unique_ptr<QgsAbstractDatabaseProviderConnection> connection { static_cast<QgsAbstractDatabaseProviderConnection *>( postgresMetadata->createConnection( uri, {} ) ) };
      const QgsDataSourceUri dsUri { QgsDataSourceUri( uri ) };
      if ( connection )
      {
        try
        {
          // Try hard to get the schema
          QString schema = dsUri.schema();
          if ( schema.isEmpty() )
          {
            schema = dsUri.param( QStringLiteral( "schema" ) );
          }
          if ( schema.isEmpty() )
          {
            schema = QStringLiteral( "public" );
          }

          connection->dropRasterTable( schema, dsUri.table() );
          deleted = true;

        }
        catch ( QgsProviderConnectionException &ex )
        {
          errorMessage = ex.what();
        }
      }
      else
      {
        errorMessage = tr( "could not create a connection to the database" );
      }
    }
    else
    {
      errorMessage = tr( "could not retrieve provider metadata" );
    }

    if ( deleted )
    {
      notify( title, tr( "Table deleted successfully." ), context, Qgis::MessageLevel::Success );
      if ( parent )
        parent->refresh();
    }
    else
    {
      notify( title, errorMessage.isEmpty() ?
              tr( "Could not delete table." ) :
              tr( "Could not delete table, reason: %1." ).arg( errorMessage ), context, Qgis::MessageLevel::Warning );
    }
  }
  else
  {
    notify( title, tr( "The layer “%1” cannot be deleted because it is in the current project as “%2”,"
                       " remove it from the project and retry." ).arg( path, projectLayer->name() ), context, Qgis::MessageLevel::Warning );
  }
}

void QgsGdalItemGuiProvider::populateContextMenu( QgsDataItem *item, QMenu *menu, const QList<QgsDataItem *> &selectedItems, QgsDataItemGuiContext context )
{
  Q_UNUSED( selectedItems )

  if ( QgsLayerItem *layerItem = qobject_cast< QgsLayerItem * >( item ) )
  {
    if ( layerItem->providerKey() == QLatin1String( "gdal" ) )
    {
      // We only show a delete layer action for postgres rasters -- GDAL itself only supports
      // deletion of raster layers from geopackage files, and we have special handling elsewhere for those
      const bool isPostgresRaster { layerItem->uri().startsWith( QLatin1String( "PG:" ) ) };
      if ( isPostgresRaster )
      {
        const QString message = QObject::tr( "Delete Table “%1”…" ).arg( layerItem->name() );
        QAction *actionDeleteLayer = new QAction( message, menu );
        QVariantMap data;
        data.insert( QStringLiteral( "uri" ), layerItem->uri() );
        data.insert( QStringLiteral( "path" ), layerItem->name() );
        data.insert( QStringLiteral( "parentItem" ), QVariant::fromValue( QPointer< QgsDataItem >( layerItem->parent() ) ) );
        actionDeleteLayer->setData( data );
        connect( actionDeleteLayer, &QAction::triggered, this, [ = ] { onDeletePostgresRasterLayer( context ); } );
        menu->addAction( actionDeleteLayer );
      }
    }
  }
}

//! Provider for gdal raster source select
class QgsGdalRasterSourceSelectProvider : public QgsSourceSelectProvider
{
  public:

    QString providerKey() const override { return QStringLiteral( "gdal" ); }
    QString text() const override { return QObject::tr( "Raster" ); }
    int ordering() const override { return QgsSourceSelectProvider::OrderLocalProvider + 20; }
    QIcon icon() const override { return QgsApplication::getThemeIcon( QStringLiteral( "/mActionAddRasterLayer.svg" ) ); }
    QgsAbstractDataSourceWidget *createDataSourceWidget( QWidget *parent = nullptr, Qt::WindowFlags fl = Qt::Widget, QgsProviderRegistry::WidgetMode widgetMode = QgsProviderRegistry::WidgetMode::Embedded ) const override
    {
      return new QgsGdalSourceSelect( parent, fl, widgetMode );
    }
};

//
// QgsGdalSourceWidgetProvider
//

QgsGdalSourceWidgetProvider::QgsGdalSourceWidgetProvider()
{

}

QString QgsGdalSourceWidgetProvider::providerKey() const
{
  return QStringLiteral( "gdal" );
}

bool QgsGdalSourceWidgetProvider::canHandleLayer( QgsMapLayer *layer ) const
{
  if ( layer->providerType() != QLatin1String( "gdal" ) )
    return false;

  const QVariantMap parts = QgsProviderRegistry::instance()->decodeUri( QStringLiteral( "gdal" ), layer->source() );
  if ( parts.value( QStringLiteral( "path" ) ).toString().isEmpty() )
    return false;

  return true;
}

QgsProviderSourceWidget *QgsGdalSourceWidgetProvider::createWidget( QgsMapLayer *layer, QWidget *parent )
{
  if ( layer->providerType() != QLatin1String( "gdal" ) )
    return nullptr;

  const QVariantMap parts = QgsProviderRegistry::instance()->decodeUri( QStringLiteral( "gdal" ), layer->source() );
  if ( parts.value( QStringLiteral( "path" ) ).toString().isEmpty() )
    return nullptr;

  return new QgsGdalFileSourceWidget( parent );
}


//
// QgsGdalGuiProviderMetadata
//
QgsGdalGuiProviderMetadata::QgsGdalGuiProviderMetadata():
  QgsProviderGuiMetadata( PROVIDER_KEY )
{
}

QList<QgsSourceSelectProvider *> QgsGdalGuiProviderMetadata::sourceSelectProviders()
{
  QList<QgsSourceSelectProvider *> providers;
  providers << new QgsGdalRasterSourceSelectProvider;
  return providers;
}

QList<QgsDataItemGuiProvider *> QgsGdalGuiProviderMetadata::dataItemGuiProviders()
{
  QList<QgsDataItemGuiProvider *> providers;
  providers << new QgsGdalItemGuiProvider();
  return providers;
}

QList<QgsProviderSourceWidgetProvider *> QgsGdalGuiProviderMetadata::sourceWidgetProviders()
{
  QList<QgsProviderSourceWidgetProvider *> providers;
  providers << new QgsGdalSourceWidgetProvider();
  return providers;
}

///@endcond
