#include "qgsproviderregistry.h"
#include "qgsowsdataitems.h"
#include "qgsowsprovider.h"
#include "qgslogger.h"
#include "qgsdatasourceuri.h"
//#include "qgsowssourceselect.h"
#include "qgsowsconnection.h"
#include "qgsnewhttpconnection.h"

#include "qgsapplication.h"

#include <QFileInfo>

// ---------------------------------------------------------------------------
QgsOWSConnectionItem::QgsOWSConnectionItem( QgsDataItem* parent, QString name, QString path )
    : QgsDataCollectionItem( parent, name, path )
{
  mIcon = QgsApplication::getThemeIcon( "mIconConnect.png" );
}

QgsOWSConnectionItem::~QgsOWSConnectionItem()
{
}

QVector<QgsDataItem*> QgsOWSConnectionItem::createChildren()
{
  QgsDebugMsg( "Entered" );
  QVector<QgsDataItem*> children;

  QVector<QgsDataItem*> serviceItems;

  int layerCount;
  // Try to open with WMS,WFS,WCS
  foreach( QString key, QStringList() << "wms" << "WFS" << "gdal" )
  {
    QgsDebugMsg( "Add connection for provider " + key );
    QLibrary *library = QgsProviderRegistry::instance()->providerLibrary( key );
    if ( !library ) continue;

    dataItem_t * dItem = ( dataItem_t * ) cast_to_fptr( library->resolve( "dataItem" ) );
    if ( !dItem )
    {
      QgsDebugMsg( library->fileName() + " does not have dataItem" );
      continue;
    }

    QgsDataItem *item = dItem( mPath, this );  // empty path -> top level
    if ( !item ) continue;

    layerCount += item->rowCount();
    if ( item->rowCount() > 0 )
    {
      QgsDebugMsg( "Add new item : " + item->name() );
      serviceItems.append( item );
    }
    else
    {
      //delete item;
    }
  }

  foreach( QgsDataItem* item, serviceItems )
  {
    QgsDebugMsg( QString( "serviceItems.size = %1 layerCount = %2 rowCount = %3" ).arg( serviceItems.size() ).arg( layerCount ).arg( item->rowCount() ) );
    if ( serviceItems.size() == 1 || layerCount <= 30 || item->rowCount() <= 10 )
    {
      // Add layers directly to OWS connection
      foreach( QgsDataItem* subItem, item->children() )
      {
        item->removeChildItem( subItem );
        subItem->setParent( this );
        children.append( subItem );
      }
      delete item;
    }
    else // Add service
    {
      children.append( item );
    }
  }

  return children;
}

bool QgsOWSConnectionItem::equal( const QgsDataItem *other )
{
  if ( type() != other->type() )
  {
    return false;
  }
  const QgsOWSConnectionItem *o = dynamic_cast<const QgsOWSConnectionItem *>( other );
  return ( mPath == o->mPath && mName == o->mName );
}

QList<QAction*> QgsOWSConnectionItem::actions()
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

void QgsOWSConnectionItem::editConnection()
{
  /*
    QgsNewHttpConnection nc( 0, "/Qgis/connections-ows/", mName );

    if ( nc.exec() )
    {
      // the parent should be updated
      mParent->refresh();
    }
  */
}

void QgsOWSConnectionItem::deleteConnection()
{
  /*
    QgsOWSConnection::deleteConnection( "OWS", mName );
    // the parent should be updated
    mParent->refresh();
  */
}


// ---------------------------------------------------------------------------


QgsOWSRootItem::QgsOWSRootItem( QgsDataItem* parent, QString name, QString path )
    : QgsDataCollectionItem( parent, name, path )
{
  mIcon = QgsApplication::getThemeIcon( "mIconOws.png" );

  populate();
}

QgsOWSRootItem::~QgsOWSRootItem()
{
}

QVector<QgsDataItem*>QgsOWSRootItem::createChildren()
{
  QgsDebugMsg( "Entered" );
  QVector<QgsDataItem*> connections;
  // Combine all WMS,WFS,WCS connections
  QMap<QString, QStringList> uris;
  foreach( QString service, QStringList() << "WMS" << "WFS" << "WCS" )
  {
    foreach( QString connName, QgsOWSConnection::connectionList( service ) )
    {
      QgsOWSConnection connection( service, connName );

      QString encodedUri = connection.uri().encodedUri();
      QStringList labels = uris.value( encodedUri );
      if ( !labels.contains( connName ) )
      {
        labels << connName;
      }
      uris[encodedUri] = labels;
    }
  }
  foreach( QString encodedUri, uris.keys() )
  {
    QgsDataItem * conn = new QgsOWSConnectionItem( this, uris.value( encodedUri ).join( " / " ), encodedUri );
    connections.append( conn );
  }
  return connections;
}

QList<QAction*> QgsOWSRootItem::actions()
{
  QList<QAction*> lst;

  /*
  QAction* actionNew = new QAction( tr( "New Connection..." ), this );
  connect( actionNew, SIGNAL( triggered() ), this, SLOT( newConnection() ) );
  lst.append( actionNew );
  */
  return lst;
}


QWidget * QgsOWSRootItem::paramWidget()
{
  /*
  QgsOWSSourceSelect *select = new QgsOWSSourceSelect( 0, 0, true, true );
  connect( select, SIGNAL( connectionsChanged() ), this, SLOT( connectionsChanged() ) );
  return select;
  */
  return 0;
}
void QgsOWSRootItem::connectionsChanged()
{
  refresh();
}

void QgsOWSRootItem::newConnection()
{
  /*
  QgsNewHttpConnection nc( 0, "/Qgis/connections-ows/" );

  if ( nc.exec() )
  {
    refresh();
  }
  */
}


// ---------------------------------------------------------------------------

static QStringList extensions = QStringList();
static QStringList wildcards = QStringList();

QGISEXTERN int dataCapabilities()
{
  return QgsDataProvider::Net;
}

QGISEXTERN QgsDataItem * dataItem( QString thePath, QgsDataItem* parentItem )
{
  if ( thePath.isEmpty() )
  {
    return new QgsOWSRootItem( parentItem, "OWS", "ows:" );
  }
  return 0;
}

//QGISEXTERN QgsOWSSourceSelect * selectWidget( QWidget * parent, Qt::WFlags fl )
QGISEXTERN QDialog * selectWidget( QWidget * parent, Qt::WFlags fl )
{
  Q_UNUSED( parent );
  Q_UNUSED( fl );
  //return new QgsOWSSourceSelect( parent, fl );
  return 0;
}
