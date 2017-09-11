/***************************************************************************
      qgsafsdataitems.cpp
      -------------------
    begin                : Jun 03, 2015
    copyright            : (C) 2015 by Sandro Mani
    email                : smani@sourcepole.ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "qgslogger.h"
#include "qgsowsconnection.h"
#include "qgsafsdataitems.h"
#include "qgsafsprovider.h"
#include "qgsarcgisrestutils.h"

#ifdef HAVE_GUI
#include "qgsnewhttpconnection.h"
#include "qgsafssourceselect.h"
#endif

#include <QCoreApplication>
#include <QSettings>
#include <QUrl>


QgsAfsRootItem::QgsAfsRootItem( QgsDataItem *parent, const QString &name, const QString &path )
  : QgsDataCollectionItem( parent, name, path )
{
  mCapabilities |= Fast;
  mIconName = QStringLiteral( "mIconAfs.svg" );
  populate();
}

QVector<QgsDataItem *> QgsAfsRootItem::createChildren()
{
  QVector<QgsDataItem *> connections;

  Q_FOREACH ( const QString &connName, QgsOwsConnection::connectionList( "arcgisfeatureserver" ) )
  {
    QgsOwsConnection connection( QStringLiteral( "arcgisfeatureserver" ), connName );
    QString path = "afs:/" + connName;
    connections.append( new QgsAfsConnectionItem( this, connName, path, connection.uri().param( QStringLiteral( "url" ) ) ) );
  }
  return connections;
}

#ifdef HAVE_GUI
QList<QAction *> QgsAfsRootItem::actions()
{
  QAction *actionNew = new QAction( tr( "New Connection..." ), this );
  connect( actionNew, &QAction::triggered, this, &QgsAfsRootItem::newConnection );
  return QList<QAction *>() << actionNew;
}

QWidget *QgsAfsRootItem::paramWidget()
{
  QgsAfsSourceSelect *select = new QgsAfsSourceSelect( 0, 0, QgsProviderRegistry::WidgetMode::Manager );
  connect( select, &QgsArcGisServiceSourceSelect::connectionsChanged, this, &QgsAfsRootItem::connectionsChanged );
  return select;
}

void QgsAfsRootItem::connectionsChanged()
{
  refresh();
}

void QgsAfsRootItem::newConnection()
{
  QgsNewHttpConnection nc( 0, QgsNewHttpConnection::ConnectionOther, QStringLiteral( "qgis/connections-arcgisfeatureserver/" ) );
  nc.setWindowTitle( tr( "Create a New ArcGisFeatureServer Connection" ) );

  if ( nc.exec() )
  {
    refresh();
  }
}
#endif

///////////////////////////////////////////////////////////////////////////////

QgsAfsConnectionItem::QgsAfsConnectionItem( QgsDataItem *parent, const QString &name, const QString &path, const QString &url )
  : QgsDataCollectionItem( parent, name, path )
  , mUrl( url )
{
  mIconName = QStringLiteral( "mIconConnect.png" );
  mCapabilities |= Collapse;
}

QVector<QgsDataItem *> QgsAfsConnectionItem::createChildren()
{
  QVector<QgsDataItem *> layers;
  QString errorTitle, errorMessage;
  QVariantMap serviceData = QgsArcGisRestUtils::getServiceInfo( mUrl, errorTitle, errorMessage );
  if ( serviceData.isEmpty() )
  {
    return layers;
  }
  QString authid = QgsArcGisRestUtils::parseSpatialReference( serviceData[QStringLiteral( "spatialReference" )].toMap() ).authid();

  foreach ( const QVariant &layerInfo, serviceData["layers"].toList() )
  {
    QVariantMap layerInfoMap = layerInfo.toMap();
    QString id = layerInfoMap[QStringLiteral( "id" )].toString();
    QgsAfsLayerItem *layer = new QgsAfsLayerItem( this, mName, mUrl + "/" + id, layerInfoMap[QStringLiteral( "name" )].toString(), authid );
    layers.append( layer );
  }

  return layers;
}

bool QgsAfsConnectionItem::equal( const QgsDataItem *other )
{
  const QgsAfsConnectionItem *o = dynamic_cast<const QgsAfsConnectionItem *>( other );
  return ( type() == other->type() && o != 0 && mPath == o->mPath && mName == o->mName );
}

#ifdef HAVE_GUI
QList<QAction *> QgsAfsConnectionItem::actions()
{
  QList<QAction *> lst;

  QAction *actionEdit = new QAction( tr( "Edit..." ), this );
  connect( actionEdit, &QAction::triggered, this, &QgsAfsConnectionItem::editConnection );
  lst.append( actionEdit );

  QAction *actionDelete = new QAction( tr( "Delete" ), this );
  connect( actionDelete, &QAction::triggered, this, &QgsAfsConnectionItem::deleteConnection );
  lst.append( actionDelete );

  return lst;
}

void QgsAfsConnectionItem::editConnection()
{
  QgsNewHttpConnection nc( 0, QgsNewHttpConnection::ConnectionOther, QStringLiteral( "qgis/connections-arcgisfeatureserver/" ), mName );
  nc.setWindowTitle( tr( "Modify ArcGisFeatureServer Connection" ) );

  if ( nc.exec() )
  {
    mParent->refresh();
  }
}

void QgsAfsConnectionItem::deleteConnection()
{
  QgsOwsConnection::deleteConnection( QStringLiteral( "arcgisfeatureserver" ), mName );
  mParent->refresh();
}
#endif

///////////////////////////////////////////////////////////////////////////////

QgsAfsLayerItem::QgsAfsLayerItem( QgsDataItem *parent, const QString &name, const QString &url, const QString &title, const QString &authid )
  : QgsLayerItem( parent, title, parent->path() + "/" + name, QString(), QgsLayerItem::Vector, QStringLiteral( "arcgisfeatureserver" ) )
{
  mUri = QStringLiteral( "crs='%1' url='%2'" ).arg( authid, url );
  setState( Populated );
  mIconName = QStringLiteral( "mIconAfs.svg" );
}
