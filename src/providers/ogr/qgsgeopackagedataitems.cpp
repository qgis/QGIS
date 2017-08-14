/***************************************************************************
    qgsgeopackagedataitems.h
    ---------------------
    begin                : August 2017
    copyright            : (C) 2017 by Alessandro Pasotti
    email                : apasotti at boundlessgeo dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsgeopackagedataitems.h"
#include "qgsgeopackageconnection.h"
#include "qgslogger.h"
#include "qgssettings.h"
#include "qgsproject.h"
#include "qgsvectorlayer.h"
#include "qgsrasterlayer.h"
#include "qgsnewgeopackagelayerdialog.h"

#include <QAction>
#include <QMessageBox>
#include <QFileDialog>
#include <QInputDialog>

QgsDataItem *QgsGeoPackageDataItemProvider::createDataItem( const QString &path, QgsDataItem *parentItem )
{
  QgsDebugMsg( "path = " + path );
  if ( path.isEmpty() )
  {
    return new QgsGeoPackageRootItem( parentItem, QStringLiteral( "GeoPackage" ), QStringLiteral( "gpkg:" ) );
  }
  return nullptr;
}

QgsGeoPackageRootItem::QgsGeoPackageRootItem( QgsDataItem *parent, QString name, QString path )
  : QgsDataCollectionItem( parent, name, path )
{
  mCapabilities |= Fast;
  mIconName = QStringLiteral( "mGeoPackage.svg" );
  populate();
}

QgsGeoPackageRootItem::~QgsGeoPackageRootItem()
{

}

QVector<QgsDataItem *> QgsGeoPackageRootItem::createChildren()
{
  QVector<QgsDataItem *> connections;

  Q_FOREACH ( const QString &connName, QgsGeoPackageConnection::connectionList() )
  {
    QgsGeoPackageConnection connection( connName );
    QgsDataItem *conn = new QgsGeoPackageConnectionItem( this, connection.name(), connection.uri().encodedUri() );

    connections.append( conn );
  }
  return connections;
}

QList<QAction *> QgsGeoPackageRootItem::actions()
{
  QList<QAction *> lst;

  QAction *actionNew = new QAction( tr( "New Connection..." ), this );
  connect( actionNew, &QAction::triggered, this, &QgsGeoPackageRootItem::newConnection );
  lst.append( actionNew );

  QAction *actionCreateDatabase = new QAction( tr( "Create Database..." ), this );
  connect( actionCreateDatabase, &QAction::triggered, this, &QgsGeoPackageRootItem::createDatabase );
  lst.append( actionCreateDatabase );

  return lst;
}

QWidget *QgsGeoPackageRootItem::paramWidget()
{
  return nullptr;
}


void QgsGeoPackageRootItem::connectionsChanged()
{
  refresh();
}

void QgsGeoPackageRootItem::newConnection()
{
  // TODO use QgsFileWidget
  QString path = QFileDialog::getOpenFileName( nullptr, tr( "Open GeoPackage" ), "", tr( "GeoPackage Database (*.gpkg)" ) );
  QFileInfo fileInfo( path );
  QString folder = fileInfo.path();
  QString connName = fileInfo.fileName();
  if ( ! path.isEmpty() )
  {
    bool ok = true;
    while ( ok && ! QgsGeoPackageConnection( connName ).path( ).isEmpty( ) )
    {

      connName = QInputDialog::getText( nullptr, tr( "Cannot add connection '%1'" ).arg( connName ),
                                        tr( "A connection with the same name already exists,\nplease provide a new name:" ), QLineEdit::Normal,
                                        QLatin1String( "" ), &ok );
    }
    if ( ok && ! connName.isEmpty() )
    {
      QgsGeoPackageConnection connection( connName );
      connection.setPath( path );
      connection.save();
      refreshConnections();
    }
  }
}


void QgsGeoPackageRootItem::createDatabase()
{
  QgsNewGeoPackageLayerDialog dialog( nullptr );
  dialog.setCrs( QgsProject::instance()->defaultCrsForNewLayers() );
  dialog.exec();
}


QgsGeoPackageConnectionItem::QgsGeoPackageConnectionItem( QgsDataItem *parent, QString name, QString path )
  : QgsDataCollectionItem( parent, name, path )
  , mPath( path )
{
  mCapabilities |= Collapse;
}

QVector<QgsDataItem *> QgsGeoPackageConnectionItem::createChildren()
{
  QVector<QgsDataItem *> children;

  // Vector layers
  QgsVectorLayer layer( mPath, QStringLiteral( "ogr_tmp" ), QStringLiteral( "ogr" ) );
  if ( ! layer.isValid( ) )
  {
    QgsDebugMsgLevel( tr( "Layer is not a valid GeoPackage Vector layer %1" ).arg( mPath ), 3 );
  }
  else
  {
    Q_FOREACH ( const QString &descriptor, layer.dataProvider()->subLayers( ) )
    {
      QStringList pieces = descriptor.split( ':' );
      QString layerId = pieces[0];
      QString name = pieces[1];
      QString featuresCount = pieces[2];
      QString geometryType = pieces[3];
      QgsLayerItem::LayerType layerType;
      layerType = layerTypeFromDb( geometryType );
      if ( layerType != QgsLayerItem::LayerType::NoType )
      {
        if ( geometryType.contains( QStringLiteral( "Collection" ), Qt::CaseInsensitive ) )
        {
          QgsDebugMsgLevel( QStringLiteral( "Layer %1 is a geometry collection: skipping %2" ).arg( name, mPath ), 3 );
        }
        else
        {
          // example URI:    '/path/gdal_sample_v1.2_no_extensions.gpkg|layerid=7|geometrytype=Point'
          QString uri = QStringLiteral( "%1|layerid=%2|geometrytype=%3" ).arg( mPath, layerId, geometryType );
          // TODO?: not sure, but if it's a collection, an expandable node would be better?
          QgsGeoPackageVectorLayerItem *item = new QgsGeoPackageVectorLayerItem( this, name, mPath, uri, layerType );
          QgsDebugMsgLevel( QStringLiteral( "Adding GPKG Vector item %1 %2 %3" ).arg( name, uri, geometryType ), 3 );
          children.append( item );
        }
      }
      else
      {
        QgsDebugMsgLevel( QStringLiteral( "Layer type is not a supported GeoPackage Vector layer %1" ).arg( mPath ), 3 );
      }

    }
  }
  // Raster layers
  QgsRasterLayer rlayer( mPath, QStringLiteral( "gdal_tmp" ), QStringLiteral( "gdal" ), false );
  Q_FOREACH ( const QString &uri, rlayer.dataProvider()->subLayers( ) )
  {
    QStringList pieces = uri.split( ':' );
    QString name = pieces.value( pieces.length() - 1 );
    QgsDebugMsg( QStringLiteral( "Adding GPKG Raster item %1 %2 %3" ).arg( name, uri ) );
    QgsGeoPackageRasterLayerItem *item = new QgsGeoPackageRasterLayerItem( this, name, mPath, uri );
    children.append( item );

  }
  return children;

}

bool QgsGeoPackageConnectionItem::equal( const QgsDataItem *other )
{
  if ( type() != other->type() )
  {
    return false;
  }
  const QgsGeoPackageConnectionItem *o = dynamic_cast<const QgsGeoPackageConnectionItem *>( other );
  return o && mPath == o->mPath && mName == o->mName;

}

#ifdef HAVE_GUI

QList<QAction *> QgsGeoPackageConnectionItem::actions()
{
  QList<QAction *> lst;


  QAction *actiondeleteConnection = new QAction( tr( "Remove connection" ), this );
  connect( actiondeleteConnection, &QAction::triggered, this, &QgsGeoPackageConnectionItem::deleteConnection );
  lst.append( actiondeleteConnection );
  return lst;
}
#endif

QgsLayerItem::LayerType QgsGeoPackageConnectionItem::layerTypeFromDb( const QString &geometryType )
{
  if ( geometryType.contains( QStringLiteral( "Point" ), Qt::CaseInsensitive ) )
  {
    return QgsLayerItem::LayerType::Point;
  }
  else if ( geometryType.contains( QStringLiteral( "Polygon" ), Qt::CaseInsensitive ) )
  {
    return QgsLayerItem::LayerType::Polygon;
  }
  else if ( geometryType.contains( QStringLiteral( "LineString" ), Qt::CaseInsensitive ) )
  {
    return QgsLayerItem::LayerType::Line;
  }
  else if ( geometryType.contains( QStringLiteral( "Collection" ), Qt::CaseInsensitive ) )
  {
    return QgsLayerItem::LayerType::Vector;
  }
  else if ( geometryType.contains( QStringLiteral( "Table" ), Qt::CaseInsensitive ) )
  {
    return QgsLayerItem::LayerType::Table;
  }
  // To be moved in a parent class that would also work for gdal and rasters
  else if ( geometryType.contains( QStringLiteral( "Raster" ), Qt::CaseInsensitive ) )
  {
    return QgsLayerItem::LayerType::Raster;
  }
  return QgsLayerItem::LayerType::NoType;
}

void QgsGeoPackageConnectionItem::deleteConnection()
{
  QgsGeoPackageConnection::deleteConnection( name() );
  mParent->refreshConnections();
}

#ifdef HAVE_GUI
QList<QAction *> QgsGeoPackageAbstractLayerItem::actions()
{
  QList<QAction *> lst;

  // TODO: delete layer when the provider supports it (not currently implemented)
  return lst;
}
#endif

QgsGeoPackageAbstractLayerItem::QgsGeoPackageAbstractLayerItem( QgsDataItem *parent, QString name, QString path, QString uri, QgsLayerItem::LayerType layerType, QString providerKey )
  : QgsLayerItem( parent, name, path, uri, layerType, providerKey )
{
  setState( Populated ); // no children are expected
}


QgsGeoPackageVectorLayerItem::QgsGeoPackageVectorLayerItem( QgsDataItem *parent, QString name, QString path, QString uri, LayerType layerType )
  : QgsGeoPackageAbstractLayerItem( parent, name, path, uri, layerType, QStringLiteral( "ogr" ) )
{

}


QgsGeoPackageRasterLayerItem::QgsGeoPackageRasterLayerItem( QgsDataItem *parent, QString name, QString path, QString uri )
  : QgsGeoPackageAbstractLayerItem( parent, name, path, uri, QgsLayerItem::LayerType::Raster, QStringLiteral( "gdal" ) )
{

}
