#include "qgsspatialitedataitems.h"

#include "qgsspatialiteprovider.h"
#include "qgsspatialiteconnection.h"

#include "qgslogger.h"

#include <QAction>


QgsSLConnectionItem::QgsSLConnectionItem( QgsDataItem* parent, QString name, QString path )
    : QgsDataCollectionItem( parent, name, path )
{
}

QgsSLConnectionItem::~QgsSLConnectionItem()
{
}

static QgsLayerItem::LayerType _layerTypeFromDb( QString dbType )
{
  if ( dbType == "POINT" || dbType == "MULTIPOINT" )
  {
    return QgsLayerItem::Point;
  }
  else if ( dbType == "LINESTRING" || dbType == "MULTILINESTRING" )
  {
    return QgsLayerItem::Line;
  }
  else if ( dbType == "POLYGON" || dbType == "MULTIPOLYGON" )
  {
    return QgsLayerItem::Polygon;
  }
  else if ( dbType == "qgis_table" )
  {
    return QgsLayerItem::Table;
  }
  else
  {
    return QgsLayerItem::NoType;
  }
}

QVector<QgsDataItem*> QgsSLConnectionItem::createChildren()
{
  QgsDebugMsg( "Entered" );
  QVector<QgsDataItem*> children;
  QgsSpatiaLiteConnection connection( mName );

  QgsSpatiaLiteConnection::Error err = connection.fetchTables( false ); // TODO: allow geometryless tables
  if ( err != QgsSpatiaLiteConnection::NoError )
  {
    QString msg;
    switch ( err )
    {
      case QgsSpatiaLiteConnection::NotExists: msg = tr( "Database does not exist" ); break;
      case QgsSpatiaLiteConnection::FailedToOpen: msg = tr( "Failed to open database" ); break;
      case QgsSpatiaLiteConnection::FailedToCheckMetadata: msg = tr( "Failed to check metadata" ); break;
      case QgsSpatiaLiteConnection::FailedToGetTables: msg = tr( "Failed to get list of tables" ); break;
      default: msg = tr( "Unknown error" ); break;
    }
    QString msgDetails = connection.errorMessage();
    if ( !msgDetails.isEmpty() )
      msg = QString( "%1 (%2)" ).arg( msg ).arg( msgDetails );
    children.append( new QgsErrorItem( this, msg, mPath + "/error" ) );
    return children;
  }

  QString connectionInfo = QString( "dbname='%1'" ).arg( QString( connection.path() ).replace( "'", "\\'" ) );
  QgsDataSourceURI uri( connectionInfo );

  foreach( const QgsSpatiaLiteConnection::TableEntry& entry, connection.tables() )
  {
    uri.setDataSource( QString(), entry.tableName, entry.column, QString(), QString() );
    QgsSLLayerItem * layer = new QgsSLLayerItem( this, entry.tableName, mPath + "/" + entry.tableName, uri.uri(), _layerTypeFromDb( entry.type ) );
    children.append( layer );
  }
  return children;
}

bool QgsSLConnectionItem::equal( const QgsDataItem *other )
{
  if ( type() != other->type() )
  {
    return false;
  }
  const QgsSLConnectionItem *o = dynamic_cast<const QgsSLConnectionItem *>( other );
  return ( mPath == o->mPath && mName == o->mName );
}

QList<QAction*> QgsSLConnectionItem::actions()
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

void QgsSLConnectionItem::editConnection()
{
  /*
  QgsPgNewConnection nc( NULL, mName );
  if ( nc.exec() )
  {
    // the parent should be updated
    mParent->refresh();
  }
  */
}

void QgsSLConnectionItem::deleteConnection()
{
  /*
  QgsPgSourceSelect::deleteConnection( mName );
  // the parent should be updated
  mParent->refresh();
  */
}


// ---------------------------------------------------------------------------

QgsSLRootItem::QgsSLRootItem( QgsDataItem* parent, QString name, QString path )
    : QgsDataCollectionItem( parent, name, path )
{
  mIcon = QIcon( getThemePixmap( "mIconSpatialite.png" ) );
  populate();
}

QgsSLRootItem::~QgsSLRootItem()
{
}

QVector<QgsDataItem*> QgsSLRootItem::createChildren()
{
  QVector<QgsDataItem*> connections;
  foreach( QString connName, QgsSpatiaLiteConnection::connectionList() )
  {
    QgsDataItem * conn = new QgsSLConnectionItem( this, connName, mPath + "/" + connName );
    connections.push_back( conn );
  }
  return connections;
}

QList<QAction*> QgsSLRootItem::actions()
{
  QList<QAction*> lst;

  QAction* actionNew = new QAction( tr( "New..." ), this );
  connect( actionNew, SIGNAL( triggered() ), this, SLOT( newConnection() ) );
  lst.append( actionNew );

  return lst;
}

QWidget * QgsSLRootItem::paramWidget()
{
  /*
  QgsSLSourceSelect *select = new QgsSLSourceSelect( 0, 0, true, true );
  connect( select, SIGNAL( connectionsChanged() ), this, SLOT( connectionsChanged() ) );
  return select;
  */
  return NULL;
}

void QgsSLRootItem::connectionsChanged()
{
  refresh();
}

void QgsSLRootItem::newConnection()
{
  /*
  QgsSLNewConnection nc( NULL );
  if ( nc.exec() )
  {
    refresh();
  }
  */
}

// ---------------------------------------------------------------------------

QGISEXTERN int dataCapabilities()
{
  return  QgsDataProvider::Database;
}

QGISEXTERN QgsDataItem * dataItem( QString thePath, QgsDataItem* parentItem )
{
  Q_UNUSED( thePath );
  return new QgsSLRootItem( parentItem, "SpatiaLite", "spatialite:" );
}
