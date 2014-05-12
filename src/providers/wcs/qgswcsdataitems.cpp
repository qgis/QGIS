/***************************************************************************
    qgswcsdataitems.cpp
    ---------------------
    begin                : 2 July, 2012
    copyright            : (C) 2012 by Radim Blazek
    email                : radim dot blazek at gmail.com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "qgsapplication.h"
#include "qgswcsdataitems.h"
#include "qgswcsprovider.h"
#include "qgslogger.h"
#include "qgsdatasourceuri.h"
#include "qgswcssourceselect.h"
#include "qgsowsconnection.h"
#include "qgsnewhttpconnection.h"

#include <QFileInfo>
#include <QSettings>

QgsWCSConnectionItem::QgsWCSConnectionItem( QgsDataItem* parent, QString name, QString path )
    : QgsDataCollectionItem( parent, name, path )
{
  mIcon = QgsApplication::getThemeIcon( "mIconWcs.svg" );
}

QgsWCSConnectionItem::~QgsWCSConnectionItem()
{
  QgsDebugMsg( "Entered" );
}

QVector<QgsDataItem*> QgsWCSConnectionItem::createChildren()
{
  QgsDebugMsg( "Entered" );
  QVector<QgsDataItem*> children;

  QString encodedUri = mPath;
  QgsDataSourceURI uri;
  uri.setEncodedUri( encodedUri );
  QgsDebugMsg( "encodedUri = " + encodedUri );

  mCapabilities.setUri( uri );

  // Attention: supportedLayers() gives tree leafes, not top level
  if ( !mCapabilities.lastError().isEmpty() )
  {
    //children.append( new QgsErrorItem( this, tr( "Failed to retrieve layers" ), mPath + "/error" ) );
    // TODO: show the error without adding child
    return children;
  }

  foreach ( QgsWcsCoverageSummary coverageSummary, mCapabilities.capabilities().contents.coverageSummary )
  {
    // Attention, the name may be empty
    QgsDebugMsg( QString::number( coverageSummary.orderId ) + " " + coverageSummary.identifier + " " + coverageSummary.title );
    QString pathName = coverageSummary.identifier.isEmpty() ? QString::number( coverageSummary.orderId ) : coverageSummary.identifier;

    QgsWCSLayerItem * layer = new QgsWCSLayerItem( this, coverageSummary.title, mPath + "/" + pathName, mCapabilities.capabilities(), uri, coverageSummary );

    children.append( layer );
  }
  return children;
}

bool QgsWCSConnectionItem::equal( const QgsDataItem *other )
{
  if ( type() != other->type() )
  {
    return false;
  }
  const QgsWCSConnectionItem *o = dynamic_cast<const QgsWCSConnectionItem *>( other );
  if ( !o )
  {
    return false;
  }

  return ( mPath == o->mPath && mName == o->mName );
}

QList<QAction*> QgsWCSConnectionItem::actions()
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

void QgsWCSConnectionItem::editConnection()
{
  QgsNewHttpConnection nc( 0, "/Qgis/connections-wcs/", mName );

  if ( nc.exec() )
  {
    // the parent should be updated
    mParent->refresh();
  }
}

void QgsWCSConnectionItem::deleteConnection()
{
  QgsOWSConnection::deleteConnection( "WCS", mName );
  // the parent should be updated
  mParent->refresh();
}


// ---------------------------------------------------------------------------

QgsWCSLayerItem::QgsWCSLayerItem( QgsDataItem* parent, QString name, QString path, QgsWcsCapabilitiesProperty capabilitiesProperty, QgsDataSourceURI dataSourceUri, QgsWcsCoverageSummary coverageSummary )
    : QgsLayerItem( parent, name, path, QString(), QgsLayerItem::Raster, "wcs" ),
    mCapabilities( capabilitiesProperty ),
    mDataSourceUri( dataSourceUri ),
    mCoverageSummary( coverageSummary )
{
  QgsDebugMsg( "uri = " + mDataSourceUri.encodedUri() );
  mUri = createUri();
  // Populate everything, it costs nothing, all info about layers is collected
  foreach ( QgsWcsCoverageSummary coverageSummary, mCoverageSummary.coverageSummary )
  {
    // Attention, the name may be empty
    QgsDebugMsg( QString::number( coverageSummary.orderId ) + " " + coverageSummary.identifier + " " + coverageSummary.title );
    QString pathName = coverageSummary.identifier.isEmpty() ? QString::number( coverageSummary.orderId ) : coverageSummary.identifier;
    QgsWCSLayerItem * layer = new QgsWCSLayerItem( this, coverageSummary.title, mPath + "/" + pathName, mCapabilities, mDataSourceUri, coverageSummary );
    mChildren.append( layer );
  }

  if ( mChildren.size() == 0 )
  {
    //mIcon = iconRaster();
    mIcon = QgsApplication::getThemeIcon( "mIconWcs.svg" );
  }
  mPopulated = true;
}

QgsWCSLayerItem::~QgsWCSLayerItem()
{
}

QString QgsWCSLayerItem::createUri()
{
  if ( mCoverageSummary.identifier.isEmpty() )
    return ""; // layer collection

  // Number of styles must match number of layers
  mDataSourceUri.setParam( "identifier", mCoverageSummary.identifier );

  // TODO(?): with WCS 1.0 GetCapabilities does not contain CRS and formats,
  // to get them we would need to call QgsWcsCapabilities::describeCoverage
  // but it is problematic to get QgsWcsCapabilities here (copy not allowed
  // by QObject, pointer is dangerous (OWS provider is changing parent))
  // We leave CRS and format default for now.

  QString format;
  // get first supported by GDAL and server
  // TODO
  //QStringList mimes = QgsGdalProvider::supportedMimes().keys();
  QStringList mimes;
  // prefer tiff
  if ( mimes.contains( "image/tiff" ) && mCoverageSummary.supportedFormat.contains( "image/tiff" ) )
  {
    format = "image/tiff";
  }
  else
  {
    foreach ( QString f, mimes )
    {
      if ( mCoverageSummary.supportedFormat.indexOf( f ) >= 0 )
      {
        format = f;
        break;
      }
    }
  }
  if ( !format.isEmpty() )
  {
    mDataSourceUri.setParam( "format", format );
  }

  QString crs;

  // TODO: prefer project CRS
  // get first known if possible
  QgsCoordinateReferenceSystem testCrs;
  foreach ( QString c, mCoverageSummary.supportedCrs )
  {
    testCrs.createFromOgcWmsCrs( c );
    if ( testCrs.isValid() )
    {
      crs = c;
      break;
    }
  }
  if ( crs.isEmpty() && mCoverageSummary.supportedCrs.size() > 0 )
  {
    crs = mCoverageSummary.supportedCrs.value( 0 );
  }
  if ( !crs.isEmpty() )
  {
    mDataSourceUri.setParam( "crs", crs );
  }

  return mDataSourceUri.encodedUri();
}

// ---------------------------------------------------------------------------

QgsWCSRootItem::QgsWCSRootItem( QgsDataItem* parent, QString name, QString path )
    : QgsDataCollectionItem( parent, name, path )
{
  mIcon = QgsApplication::getThemeIcon( "mIconWcs.svg" );

  populate();
}

QgsWCSRootItem::~QgsWCSRootItem()
{
}

QVector<QgsDataItem*>QgsWCSRootItem::createChildren()
{
  QVector<QgsDataItem*> connections;
  foreach ( QString connName, QgsOWSConnection::connectionList( "WCS" ) )
  {
    //QgsDataItem * conn = new QgsWCSConnectionItem( this, connName, mPath + "/" + connName );
    QgsOWSConnection connection( "WCS", connName );
    QgsDataItem * conn = new QgsWCSConnectionItem( this, connName, connection.uri().encodedUri() );

    conn->setIcon( QgsApplication::getThemeIcon( "mIconConnect.png" ) );
    connections.append( conn );
  }
  return connections;
}

QList<QAction*> QgsWCSRootItem::actions()
{
  QList<QAction*> lst;

  QAction* actionNew = new QAction( tr( "New Connection..." ), this );
  connect( actionNew, SIGNAL( triggered() ), this, SLOT( newConnection() ) );
  lst.append( actionNew );

  return lst;
}


QWidget * QgsWCSRootItem::paramWidget()
{
  QgsWCSSourceSelect *select = new QgsWCSSourceSelect( 0, 0, true, true );
  connect( select, SIGNAL( connectionsChanged() ), this, SLOT( connectionsChanged() ) );
  return select;
}

void QgsWCSRootItem::connectionsChanged()
{
  refresh();
}

void QgsWCSRootItem::newConnection()
{
  QgsNewHttpConnection nc( 0, "/Qgis/connections-wcs/" );

  if ( nc.exec() )
  {
    refresh();
  }
}

// ---------------------------------------------------------------------------

static QString filterString;
static QStringList extensions = QStringList();
static QStringList wildcards = QStringList();

QGISEXTERN int dataCapabilities()
{
  return  QgsDataProvider::Net;
}

QGISEXTERN QgsDataItem * dataItem( QString thePath, QgsDataItem* parentItem )
{
  QgsDebugMsg( "thePath = " + thePath );
  if ( thePath.isEmpty() )
  {
    // Top level WCS
    return new QgsWCSRootItem( parentItem, "WCS", "wcs:" );
  }

  // OWS server
  QgsDebugMsg( "connection found in uri" );
  return new QgsWCSConnectionItem( parentItem, "WCS", thePath );
}

QGISEXTERN QgsWCSSourceSelect * selectWidget( QWidget * parent, Qt::WindowFlags fl )
{
  return new QgsWCSSourceSelect( parent, fl );
}
