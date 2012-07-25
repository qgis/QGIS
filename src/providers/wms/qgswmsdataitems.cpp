/***************************************************************************
    qgswmsdataitems.cpp
    ---------------------
    begin                : October 2011
    copyright            : (C) 2011 by Martin Dobias
    email                : wonder.sk at gmail.com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "qgswmsdataitems.h"

#include "qgslogger.h"

#include "qgsdatasourceuri.h"
#include "qgswmsconnection.h"
#include "qgswmssourceselect.h"

#include "qgsnewhttpconnection.h"

#include "qgstilescalewidget.h"

#include "qgsapplication.h"

// ---------------------------------------------------------------------------
QgsWMSConnectionItem::QgsWMSConnectionItem( QgsDataItem* parent, QString name, QString path )
    : QgsDataCollectionItem( parent, name, path )
{
  mIcon = QgsApplication::getThemeIcon( "mIconWms.png" );
}

QgsWMSConnectionItem::~QgsWMSConnectionItem()
{
}

QVector<QgsDataItem*> QgsWMSConnectionItem::createChildren()
{
  QgsDebugMsg( "Entered" );
  QVector<QgsDataItem*> children;

  QString encodedUri = mPath;
  QgsDataSourceURI uri;
  uri.setEncodedUri( encodedUri );
#if 0
  if ( mPath.contains( "url=" ) )
  {
    encodedUri = mPath;
    uri.setEncodedUri( encodedUri );
  }
  else
  {
    QgsWMSConnection connection( mName );
    uri = connection.uri();
    encodedUri = uri.encodedUri();
  }
#endif
  QgsDebugMsg( "encodedUri = " + encodedUri );

  QgsWmsProvider *wmsProvider = new QgsWmsProvider( encodedUri );
  if ( !wmsProvider ) return children;

  // Attention: supportedLayers() gives tree leafes, not top level
  if ( !wmsProvider->supportedLayers( mLayerProperties ) )
  {
    //children.append( new QgsErrorItem( this, tr( "Failed to retrieve layers" ), mPath + "/error" ) );
    // TODO: show the error without adding child
    return children;
  }

  QgsWmsCapabilitiesProperty mCapabilitiesProperty = wmsProvider->capabilitiesProperty();
  QgsWmsCapabilityProperty capabilityProperty = mCapabilitiesProperty.capability;

  // Top level layer is present max once
  // <element name="Capability">
  //    <element ref="wms:Layer" minOccurs="0"/>  - default maxOccurs=1
  QgsWmsLayerProperty topLayerProperty = capabilityProperty.layer;
  foreach( QgsWmsLayerProperty layerProperty, topLayerProperty.layer )
  {
    // Attention, the name may be empty
    QgsDebugMsg( QString::number( layerProperty.orderId ) + " " + layerProperty.name + " " + layerProperty.title );
    QString pathName = layerProperty.name.isEmpty() ? QString::number( layerProperty.orderId ) : layerProperty.name;

    QgsWMSLayerItem * layer = new QgsWMSLayerItem( this, layerProperty.title, mPath + "/" + pathName, mCapabilitiesProperty, uri, layerProperty );

    children.append( layer );
  }
  return children;
}

bool QgsWMSConnectionItem::equal( const QgsDataItem *other )
{
  if ( type() != other->type() )
  {
    return false;
  }
  const QgsWMSConnectionItem *o = dynamic_cast<const QgsWMSConnectionItem *>( other );
  if ( !o )
  {
    return false;
  }

  return ( mPath == o->mPath && mName == o->mName );
}

QList<QAction*> QgsWMSConnectionItem::actions()
{
  QList<QAction*> lst;

  QAction* actionEdit = new QAction( tr( "Edit..." ), this );
  connect( actionEdit, SIGNAL( triggered() ), this, SLOT( editConnection() ) );
  lst.append( actionEdit );

  QAction* actionDelete = new QAction( tr( "Delete" ), this );
  connect( actionDelete, SIGNAL( triggered() ), this, SLOT( deleteConnection() ) );
  lst.append( actionDelete );

  return lst;
}

void QgsWMSConnectionItem::editConnection()
{
  QgsNewHttpConnection nc( 0, "/Qgis/connections-wms/", mName );

  if ( nc.exec() )
  {
    // the parent should be updated
    mParent->refresh();
  }
}

void QgsWMSConnectionItem::deleteConnection()
{
  QgsWMSConnection::deleteConnection( mName );
  // the parent should be updated
  mParent->refresh();
}


// ---------------------------------------------------------------------------

QgsWMSLayerItem::QgsWMSLayerItem( QgsDataItem* parent, QString name, QString path, QgsWmsCapabilitiesProperty capabilitiesProperty, QgsDataSourceURI dataSourceUri, QgsWmsLayerProperty layerProperty )
    : QgsLayerItem( parent, name, path, QString(), QgsLayerItem::Raster, "wms" ),
    mCapabilitiesProperty( capabilitiesProperty ),
    mDataSourceUri( dataSourceUri ),
    mLayerProperty( layerProperty )
    //mProviderKey ("wms"),
    //mLayerType ( QgsLayerItem::Raster )
{
  QgsDebugMsg( "uri = " + mDataSourceUri.encodedUri() );
  mUri = createUri();
  // Populate everything, it costs nothing, all info about layers is collected
  foreach( QgsWmsLayerProperty layerProperty, mLayerProperty.layer )
  {
    // Attention, the name may be empty
    QgsDebugMsg( QString::number( layerProperty.orderId ) + " " + layerProperty.name + " " + layerProperty.title );
    QString pathName = layerProperty.name.isEmpty() ? QString::number( layerProperty.orderId ) : layerProperty.name;
    QgsWMSLayerItem * layer = new QgsWMSLayerItem( this, layerProperty.title, mPath + "/" + pathName, mCapabilitiesProperty, mDataSourceUri, layerProperty );
    mChildren.append( layer );
  }

  if ( mChildren.size() == 0 )
  {
    //mIcon = iconRaster();
    mIcon = QgsApplication::getThemeIcon( "mIconWms.png" );
  }
  mPopulated = true;
}

QgsWMSLayerItem::~QgsWMSLayerItem()
{
}

QString QgsWMSLayerItem::createUri()
{
  if ( mLayerProperty.name.isEmpty() )
    return ""; // layer collection

  // Number of styles must match number of layers
  mDataSourceUri.setParam( "layers", mLayerProperty.name );
  QString style = mLayerProperty.style.size() > 0 ? mLayerProperty.style[0].name : "";
  mDataSourceUri.setParam( "styles", style );

  QString format;
  // get first supported by qt and server
  QVector<QgsWmsSupportedFormat> formats = QgsWmsProvider::supportedFormats();
  foreach( QgsWmsSupportedFormat f, formats )
  {
    if ( mCapabilitiesProperty.capability.request.getMap.format.indexOf( f.format ) >= 0 )
    {
      format = f.format;
      break;
    }
  }
  mDataSourceUri.setParam( "format", format );

  QString crs;
  // get first known if possible
  QgsCoordinateReferenceSystem testCrs;
  foreach( QString c, mLayerProperty.crs )
  {
    testCrs.createFromOgcWmsCrs( c );
    if ( testCrs.isValid() )
    {
      crs = c;
      break;
    }
  }
  if ( crs.isEmpty() && mLayerProperty.crs.size() > 0 )
  {
    crs = mLayerProperty.crs[0];
  }
  mDataSourceUri.setParam( "crs", crs );
  //uri = rasterLayerPath + "|layers=" + layers.join( "," ) + "|styles=" + styles.join( "," ) + "|format=" + format + "|crs=" + crs;

  return mDataSourceUri.encodedUri();
}

// ---------------------------------------------------------------------------
QgsWMSRootItem::QgsWMSRootItem( QgsDataItem* parent, QString name, QString path )
    : QgsDataCollectionItem( parent, name, path )
{
  mIcon = QgsApplication::getThemeIcon( "mIconWms.png" );

  populate();
}

QgsWMSRootItem::~QgsWMSRootItem()
{
}

QVector<QgsDataItem*>QgsWMSRootItem::createChildren()
{
  QVector<QgsDataItem*> connections;

  foreach( QString connName, QgsWMSConnection::connectionList() )
  {
    //QgsDataItem * conn = new QgsWMSConnectionItem( this, connName, mPath + "/" + connName );
    QgsWMSConnection connection( connName );
    QgsDataItem * conn = new QgsWMSConnectionItem( this, connName, connection.uri().encodedUri() );

    conn->setIcon( QgsApplication::getThemeIcon( "mIconConnect.png" ) );
    connections.append( conn );
  }
  return connections;
}

QList<QAction*> QgsWMSRootItem::actions()
{
  QList<QAction*> lst;

  QAction* actionNew = new QAction( tr( "New Connection..." ), this );
  connect( actionNew, SIGNAL( triggered() ), this, SLOT( newConnection() ) );
  lst.append( actionNew );

  return lst;
}


QWidget * QgsWMSRootItem::paramWidget()
{
  QgsWMSSourceSelect *select = new QgsWMSSourceSelect( 0, 0, true, true );
  connect( select, SIGNAL( connectionsChanged() ), this, SLOT( connectionsChanged() ) );
  return select;
}
void QgsWMSRootItem::connectionsChanged()
{
  refresh();
}

void QgsWMSRootItem::newConnection()
{
  QgsNewHttpConnection nc( 0 );

  if ( nc.exec() )
  {
    refresh();
  }
}


// ---------------------------------------------------------------------------

QGISEXTERN void registerGui( QMainWindow *mainWindow )
{
  QgsTileScaleWidget::showTileScale( mainWindow );
}

QGISEXTERN QgsWMSSourceSelect * selectWidget( QWidget * parent, Qt::WFlags fl )
{
  return new QgsWMSSourceSelect( parent, fl );
}

QGISEXTERN int dataCapabilities()
{
  return  QgsDataProvider::Net;
}

QGISEXTERN QgsDataItem * dataItem( QString thePath, QgsDataItem* parentItem )
{
  if ( thePath.isEmpty() )
  {
    return new QgsWMSRootItem( parentItem, "WMS", "wms:" );
  }

  // The path should contain encoded connection URI
  return new QgsWMSConnectionItem( parentItem, "WMS", thePath );
}
