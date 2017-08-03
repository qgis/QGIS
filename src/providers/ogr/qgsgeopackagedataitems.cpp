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
#include "qgsvectorlayer.h"

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
  mIconName = QStringLiteral( "mGeoPackage.png" );
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
// TODO
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

  QgsVectorLayer layer( mPath, QStringLiteral( "ogr_tmp" ), QStringLiteral( "ogr" ) );
  if ( ! layer.isValid( ) )
  {
    children.append( new QgsErrorItem( this, tr( "Layer is not a valid GeoPackage layer" ), mPath + "/error" ) );
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
      QgsGeoPackageLayerItem::LayerType layerType;
      layerType = layerTypeFromDb( geometryType );
      if ( layerType != QgsGeoPackageLayerItem::LayerType::NoType )
      {
        // URI:    '/path/gdal_sample_v1.2_no_extensions.gpkg|layerid=7|geometrytype=Point'
        QString uri = QStringLiteral( "%1|layerid=%2|geometrytype=%3" ).arg( mPath, layerId, QgsGeoPackageLayerItem::layerTypeAsString( layerTypeFromDb( geometryType ) ) );
        QgsGeoPackageLayerItem *item = new QgsGeoPackageLayerItem( this, name, mPath, uri, layerType );
        QgsDebugMsg( QStringLiteral( "Adding GPKG item %1 %2 %3" ).arg( name, uri, geometryType ) );
        children.append( item );
      }
      else
      {
        children.append( new QgsErrorItem( this, tr( "Layer is not a supported GeoPackage layer geometry type: %1" ).arg( geometryType ), mPath + "/error" ) );
      }

    }
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

  QAction *actionRemoveConnection = new QAction( tr( "Remove connection" ), this );
  // TODO: implement layer deletion
  // connect( actionDeleteLayer, &QAction::triggered, this, &QgsGeoPackageLayerItem::deleteLayer );
  lst.append( actionRemoveConnection );
  return lst;
}
#endif

QgsLayerItem::LayerType QgsGeoPackageConnectionItem::layerTypeFromDb( const QString &geometryType )
{
  if ( QString::compare( geometryType, QStringLiteral( "Point" ), Qt::CaseInsensitive ) == 0 || QString::compare( geometryType, QStringLiteral( "MultiPoint" ), Qt::CaseInsensitive ) == 0 )
  {
    return QgsLayerItem::LayerType::Point;
  }
  else if ( QString::compare( geometryType, QStringLiteral( "Polygon" ), Qt::CaseInsensitive ) == 0 || QString::compare( geometryType, QStringLiteral( "MultiPolygon" ), Qt::CaseInsensitive ) == 0 )
  {
    return QgsLayerItem::LayerType::Polygon;
  }
  else if ( QString::compare( geometryType, QStringLiteral( "LineString" ), Qt::CaseInsensitive ) == 0 || QString::compare( geometryType, QStringLiteral( "MultiLineString" ), Qt::CaseInsensitive ) == 0 )
  {
    return QgsLayerItem::LayerType::Line;
  }
  // To be moved in a parent class that would also work for gdal and rasters
  else if ( QString::compare( geometryType, QStringLiteral( "Raster" ), Qt::CaseInsensitive ) == 0 )
  {
    return QgsLayerItem::LayerType::Raster;
  }
  return QgsLayerItem::LayerType::NoType;
}

#ifdef HAVE_GUI
QList<QAction *> QgsGeoPackageLayerItem::actions()
{
  QList<QAction *> lst;

  QAction *actionDeleteLayer = new QAction( tr( "Delete Layer" ), this );
  // TODO connect( actionDeleteLayer, &QAction::triggered, this, &QgsGeoPackageLayerItem::deleteLayer );
  lst.append( actionDeleteLayer );

  return lst;
}
#endif

QgsGeoPackageLayerItem::QgsGeoPackageLayerItem( QgsDataItem *parent, QString name, QString path, QString uri, QgsLayerItem::LayerType layerType )
  : QgsLayerItem( parent, name, path, uri, layerType, QStringLiteral( "ogr" ) )
{
  setState( Populated ); // no children are expected
}
