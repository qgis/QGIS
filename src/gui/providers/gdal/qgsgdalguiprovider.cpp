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
#include "qgssourceselectprovider.h"
#include "qgsgdalsourceselect.h"
#include "qgsapplication.h"
#include "qgsprovidermetadata.h"
#include "qgsdataitem.h"
#include "qgsdataitemguiprovider.h"
#include "qgsgdaldataitems.h"

static QString PROVIDER_KEY = QStringLiteral( "gdal" );

QgsGdalItemGuiProvider::QgsGdalItemGuiProvider() = default;

QgsGdalItemGuiProvider::~QgsGdalItemGuiProvider() = default;

QString QgsGdalItemGuiProvider::name()
{
  return QStringLiteral( "gdal_items" );
}

void QgsGdalItemGuiProvider::onDeleteLayer()
{
  QAction *s = qobject_cast<QAction *>( sender() );
  QVariantMap data = s->data().toMap();
  const QString uri = data[QStringLiteral( "uri" )].toString();
  const QString path = data[QStringLiteral( "path" )].toString();
  QPointer< QgsDataItem > parent = data[QStringLiteral( "parent" )].value<QPointer< QgsDataItem >>();

  const QString title = QObject::tr( "Delete File" );
  // Check if the layer is in the project
  const QgsMapLayer *projectLayer = nullptr;
  const auto mapLayers = QgsProject::instance()->mapLayers();
  for ( auto it = mapLayers.constBegin(); it != mapLayers.constEnd(); ++it )
  {
    if ( it.value()->publicSource() == uri )
    {
      projectLayer = it.value();
    }
  }
  if ( ! projectLayer )
  {
    const QString confirmMessage = QObject::tr( "Are you sure you want to delete file '%1'?" ).arg( path );

    if ( QMessageBox::question( nullptr, title,
                                confirmMessage,
                                QMessageBox::Yes | QMessageBox::No, QMessageBox::No ) != QMessageBox::Yes )
      return;


    if ( !QFile::remove( path ) )
    {
      QMessageBox::warning( nullptr, title, tr( "Could not delete file." ) );
    }
    else
    {
      QMessageBox::information( nullptr, title, tr( "File deleted successfully." ) );
      if ( parent )
        parent->refresh();
    }
  }
  else
  {
    QMessageBox::warning( nullptr, title, QObject::tr( "The layer '%1' cannot be deleted because it is in the current project as '%2',"
                          " remove it from the project and retry." ).arg( path, projectLayer->name() ) );
  }
}

void QgsGdalItemGuiProvider::populateContextMenu( QgsDataItem *item, QMenu *menu, const QList<QgsDataItem *> &selectedItems, QgsDataItemGuiContext context )
{
  Q_UNUSED( selectedItems );
  Q_UNUSED( context );

  if ( QgsGdalLayerItem *layerItem = qobject_cast< QgsGdalLayerItem * >( item ) )
  {
    // Messages are different for files and tables
    const QString message = QObject::tr( "Delete File “%1”…" ).arg( layerItem->name() );
    QAction *actionDeleteLayer = new QAction( message, menu );
    QVariantMap data;
    data.insert( QStringLiteral( "uri" ), layerItem->uri() );
    data.insert( QStringLiteral( "path" ), layerItem->path() );
    data.insert( QStringLiteral( "parentItem" ), QVariant::fromValue( QPointer< QgsDataItem >( layerItem->parent() ) ) );
    actionDeleteLayer->setData( data );
    connect( actionDeleteLayer, &QAction::triggered, this, &QgsGdalItemGuiProvider::onDeleteLayer );
    menu->addAction( actionDeleteLayer );
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

///@endcond
