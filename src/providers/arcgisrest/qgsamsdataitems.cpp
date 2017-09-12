/***************************************************************************
      qgsamsdataitems.cpp
      -------------------
    begin                : Nov 26, 2015
    copyright            : (C) 2015 by Sandro Mani
    email                : smani@sourcepole.ch
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "qgsamsdataitems.h"
#include "qgsarcgisrestutils.h"
#include "qgsowsconnection.h"
#include "qgsproviderregistry.h"

#ifdef HAVE_GUI
#include "qgsamssourceselect.h"
#include "qgsnewhttpconnection.h"
#endif

#include <QImageReader>

QgsAmsRootItem::QgsAmsRootItem( QgsDataItem *parent, QString name, QString path )
  : QgsDataCollectionItem( parent, name, path )
{
  mCapabilities |= Fast | Collapse;
  mIconName = QStringLiteral( "mIconAms.svg" );
  populate();
}

QVector<QgsDataItem *> QgsAmsRootItem::createChildren()
{
  QVector<QgsDataItem *> connections;

  Q_FOREACH ( const QString &connName, QgsOwsConnection::connectionList( "arcgismapserver" ) )
  {
    QgsOwsConnection connection( QStringLiteral( "arcgismapserver" ), connName );
    QString path = "ams:/" + connName;
    connections.append( new QgsAmsConnectionItem( this, connName, path, connection.uri().param( QStringLiteral( "url" ) ) ) );
  }
  return connections;
}

#ifdef HAVE_GUI
QList<QAction *> QgsAmsRootItem::actions()
{
  QAction *actionNew = new QAction( tr( "New Connection..." ), this );
  connect( actionNew, &QAction::triggered, this, &QgsAmsRootItem::newConnection );
  return QList<QAction *>() << actionNew;
}


QWidget *QgsAmsRootItem::paramWidget()
{
  QgsAmsSourceSelect *select = new QgsAmsSourceSelect( 0, 0, QgsProviderRegistry::WidgetMode::Manager );
  connect( select, &QgsArcGisServiceSourceSelect::connectionsChanged, this, &QgsAmsRootItem::connectionsChanged );
  return select;
}

void QgsAmsRootItem::connectionsChanged()
{
  refresh();
}

void QgsAmsRootItem::newConnection()
{
  QgsNewHttpConnection nc( 0, QgsNewHttpConnection::ConnectionOther, QStringLiteral( "qgis/connections-arcgismapserver/" ) );
  nc.setWindowTitle( tr( "Create a New ArcGIS Map Server Connection" ) );

  if ( nc.exec() )
  {
    refreshConnections();
  }
}
#endif

///////////////////////////////////////////////////////////////////////////////

QgsAmsConnectionItem::QgsAmsConnectionItem( QgsDataItem *parent, QString name, QString path, QString url )
  : QgsDataCollectionItem( parent, name, path )
  , mUrl( url )
{
  mIconName = QStringLiteral( "mIconConnect.png" );
}

QVector<QgsDataItem *> QgsAmsConnectionItem::createChildren()
{
  QVector<QgsDataItem *> layers;
  QString errorTitle, errorMessage;
  QVariantMap serviceData = QgsArcGisRestUtils::getServiceInfo( mUrl, errorTitle, errorMessage );
  if ( serviceData.isEmpty() )
  {
    return layers;
  }

  QString authid = QgsArcGisRestUtils::parseSpatialReference( serviceData[QStringLiteral( "spatialReference" )].toMap() ).authid();
  QString format = QStringLiteral( "jpg" );
  bool found = false;
  QList<QByteArray> supportedFormats = QImageReader::supportedImageFormats();
  foreach ( const QString &encoding, serviceData["supportedImageFormatTypes"].toString().split( "," ) )
  {
    foreach ( const QByteArray &fmt, supportedFormats )
    {
      if ( encoding.startsWith( fmt, Qt::CaseInsensitive ) )
      {
        format = encoding;
        found = true;
        break;
      }
    }
    if ( found )
      break;
  }

  foreach ( const QVariant &layerInfo, serviceData["layers"].toList() )
  {
    QVariantMap layerInfoMap = layerInfo.toMap();
    QString id = layerInfoMap[QStringLiteral( "id" )].toString();
    QgsAmsLayerItem *layer = new QgsAmsLayerItem( this, mName, mUrl, id, layerInfoMap[QStringLiteral( "name" )].toString(), authid, format );
    layers.append( layer );
  }

  return layers;
}

bool QgsAmsConnectionItem::equal( const QgsDataItem *other )
{
  const QgsAmsConnectionItem *o = dynamic_cast<const QgsAmsConnectionItem *>( other );
  return ( type() == other->type() && o != 0 && mPath == o->mPath && mName == o->mName );
}

#ifdef HAVE_GUI
QList<QAction *> QgsAmsConnectionItem::actions()
{
  QList<QAction *> lst;

  QAction *actionEdit = new QAction( tr( "Edit..." ), this );
  connect( actionEdit, &QAction::triggered, this, &QgsAmsConnectionItem::editConnection );
  lst.append( actionEdit );

  QAction *actionDelete = new QAction( tr( "Delete" ), this );
  connect( actionDelete, &QAction::triggered, this, &QgsAmsConnectionItem::deleteConnection );
  lst.append( actionDelete );

  return lst;
}

void QgsAmsConnectionItem::editConnection()
{
  QgsNewHttpConnection nc( 0, QgsNewHttpConnection::ConnectionOther, QStringLiteral( "qgis/connections-arcgismapserver/" ), mName );
  nc.setWindowTitle( tr( "Modify ArcGIS Map Server Connection" ) );

  if ( nc.exec() )
  {
    mParent->refreshConnections();
  }
}

void QgsAmsConnectionItem::deleteConnection()
{
  QgsOwsConnection::deleteConnection( QStringLiteral( "arcgismapserver" ), mName );
  mParent->refreshConnections();
}
#endif

///////////////////////////////////////////////////////////////////////////////

QgsAmsLayerItem::QgsAmsLayerItem( QgsDataItem *parent, const QString &name, const QString &url, const QString &id, const QString &title, const QString &authid, const QString &format )
  : QgsLayerItem( parent, title, parent->path() + "/" + name, QString(), QgsLayerItem::Raster, QStringLiteral( "arcgismapserver" ) )
{
  mUri = QStringLiteral( "crs='%1' format='%2' layer='%3' url='%4'" ).arg( authid, format, id, url );
  setState( Populated );
  mIconName = QStringLiteral( "mIconAms.svg" );
}
