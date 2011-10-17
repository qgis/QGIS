#include "qgswfsdataitems.h"

#include "qgswfsprovider.h"
#include "qgswfsconnection.h"
#include "qgswfssourceselect.h"

#include "qgsnewhttpconnection.h"

#include <QSettings>
#include <QCoreApplication>


QgsWFSLayerItem::QgsWFSLayerItem( QgsDataItem* parent, QString connName, QString name, QString title )
    : QgsLayerItem( parent, title, parent->path() + "/" + name, QString(), QgsLayerItem::Vector, "WFS" )
{
  mUri = QgsWFSConnection( connName ).uriGetFeature( name );
  mPopulated = true;
}

QgsWFSLayerItem::~QgsWFSLayerItem()
{
}

////

QgsWFSConnectionItem::QgsWFSConnectionItem( QgsDataItem* parent, QString name, QString path )
    : QgsDataCollectionItem( parent, name, path ), mName( name ), mConn( NULL )
{
  mIcon = QIcon( getThemePixmap( "mIconConnect.png" ) );
}

QgsWFSConnectionItem::~QgsWFSConnectionItem()
{
}

QVector<QgsDataItem*> QgsWFSConnectionItem::createChildren()
{
  mGotCapabilities = false;
  mConn = new QgsWFSConnection( mName, this );
  connect( mConn, SIGNAL( gotCapabilities() ), this, SLOT( gotCapabilities() ) );

  mConn->requestCapabilities();

  while ( !mGotCapabilities )
  {
    QCoreApplication::processEvents( QEventLoop::ExcludeUserInputEvents );
  }

  QVector<QgsDataItem*> layers;
  if ( mConn->errorCode() == QgsWFSConnection::NoError )
  {
    QgsWFSConnection::GetCapabilities caps = mConn->capabilities();
    foreach( const QgsWFSConnection::FeatureType& featureType, caps.featureTypes )
    {
      QgsWFSLayerItem* layer = new QgsWFSLayerItem( this, mName, featureType.name, featureType.title );
      layers.append( layer );
    }
  }
  else
  {
    layers.append( new QgsErrorItem( this, tr( "Failed to retrieve layers" ), mPath + "/error" ) );
  }

  mConn->deleteLater();
  mConn = NULL;

  return layers;
}

void QgsWFSConnectionItem::gotCapabilities()
{
  mGotCapabilities = true;
}

QList<QAction*> QgsWFSConnectionItem::actions()
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

void QgsWFSConnectionItem::editConnection()
{
  QgsNewHttpConnection nc( 0, "/Qgis/connections-wfs/", mName );
  nc.setWindowTitle( tr( "Modify WFS connection" ) );

  if ( nc.exec() )
  {
    // the parent should be updated
    mParent->refresh();
  }
}

void QgsWFSConnectionItem::deleteConnection()
{
  QgsWFSConnection::deleteConnection( mName );
  // the parent should be updated
  mParent->refresh();
}



//////


QgsWFSRootItem::QgsWFSRootItem( QgsDataItem* parent, QString name, QString path )
    : QgsDataCollectionItem( parent, name, path )
{
  mIcon = QIcon( getThemePixmap( "mIconWfs.png" ) );

  populate();
}

QgsWFSRootItem::~QgsWFSRootItem()
{
}

QVector<QgsDataItem*> QgsWFSRootItem::createChildren()
{
  QVector<QgsDataItem*> connections;

  foreach( QString connName, QgsWFSConnection::connectionList() )
  {
    QgsDataItem * conn = new QgsWFSConnectionItem( this, connName, mPath + "/" + connName );
    connections.append( conn );
  }
  return connections;
}

QList<QAction*> QgsWFSRootItem::actions()
{
  QList<QAction*> lst;

  QAction* actionNew = new QAction( tr( "New..." ), this );
  connect( actionNew, SIGNAL( triggered() ), this, SLOT( newConnection() ) );
  lst.append( actionNew );

  return lst;
}

QWidget * QgsWFSRootItem::paramWidget()
{
  QgsWFSSourceSelect *select = new QgsWFSSourceSelect( 0, 0, true );
  connect( select, SIGNAL( connectionsChanged() ), this, SLOT( connectionsChanged() ) );
  return select;
}

void QgsWFSRootItem::connectionsChanged()
{
  refresh();
}

void QgsWFSRootItem::newConnection()
{
  QgsNewHttpConnection nc( 0, "/Qgis/connections-wfs/" );
  nc.setWindowTitle( tr( "Create a new WFS connection" ) );

  if ( nc.exec() )
  {
    refresh();
  }
}


QGISEXTERN int dataCapabilities()
{
  return  QgsDataProvider::Net;
}

QGISEXTERN QgsDataItem * dataItem( QString thePath, QgsDataItem* parentItem )
{
  Q_UNUSED( thePath );

  return new QgsWFSRootItem( parentItem, "WFS", "wfs:" );
}

