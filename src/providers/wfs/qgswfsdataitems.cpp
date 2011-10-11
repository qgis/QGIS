#include "qgswfsdataitems.h"

#include "qgswfsprovider.h"
#include "qgswfsconnection.h"
#include "qgswfssourceselect.h"

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
    // TODO: return an "error" item
  }

  mConn->deleteLater();
  mConn = NULL;

  return layers;
}

void QgsWFSConnectionItem::gotCapabilities()
{
  mGotCapabilities = true;
}

//////


QgsWFSRootItem::QgsWFSRootItem( QgsDataItem* parent, QString name, QString path )
    : QgsDataCollectionItem( parent, name, path )
{
  mIcon = QIcon( getThemePixmap( "mIconWms.png" ) );

  populate();
}

QgsWFSRootItem::~QgsWFSRootItem()
{
}

QVector<QgsDataItem*> QgsWFSRootItem::createChildren()
{
  QVector<QgsDataItem*> connections;
  QSettings settings;

  settings.beginGroup( "/Qgis/connections-wfs" );
  foreach( QString connName,  settings.childGroups() )
  {
    QgsDataItem * conn = new QgsWFSConnectionItem( this, connName, mPath + "/" + connName );
    connections.append( conn );
  }
  return connections;
}

QWidget * QgsWFSRootItem::paramWidget()
{
  QgsWFSSourceSelect *select = new QgsWFSSourceSelect( 0, 0 );
  connect( select, SIGNAL( connectionsChanged() ), this, SLOT( connectionsChanged() ) );
  return select;
}

void QgsWFSRootItem::connectionsChanged()
{
  refresh();
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

