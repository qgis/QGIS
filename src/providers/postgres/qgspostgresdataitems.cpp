#include "qgspostgresdataitems.h"

#include "qgslogger.h"

#include "qgspostgresconnection.h"
#include "qgspgsourceselect.h"
#include "qgspgnewconnection.h"

// ---------------------------------------------------------------------------
QgsPGConnectionItem::QgsPGConnectionItem( QgsDataItem* parent, QString name, QString path )
    : QgsDataCollectionItem( parent, name, path )
{
}

QgsPGConnectionItem::~QgsPGConnectionItem()
{
}

QVector<QgsDataItem*> QgsPGConnectionItem::createChildren()
{
  QgsDebugMsg( "Entered" );
  QVector<QgsDataItem*> children;
  QgsPostgresConnection connection( mName );
  QgsPostgresProvider *pgProvider = connection.provider( );
  if ( !pgProvider )
    return children;

  QString mConnInfo = connection.connectionInfo();
  QgsDebugMsg( "mConnInfo = " + mConnInfo );

  if ( !pgProvider->supportedLayers( mLayerProperties, true, false, false ) )
  {
    children.append( new QgsErrorItem( this, tr( "Failed to retrieve layers" ), mPath + "/error" ) );
    return children;
  }

  QMap<QString, QVector<QgsPostgresLayerProperty> > schemasMap;
  foreach( QgsPostgresLayerProperty layerProperty, mLayerProperties )
  {
    schemasMap[ layerProperty.schemaName ].push_back( layerProperty );
  }

  QMap<QString, QVector<QgsPostgresLayerProperty> >::const_iterator it = schemasMap.constBegin();
  for ( ; it != schemasMap.constEnd(); it++ )
  {
    QgsDebugMsg( "schema: " + it.key() );
    QgsPGSchemaItem * schema = new QgsPGSchemaItem( this, it.key(), mPath + "/" + it.key(), mConnInfo, it.value() );

    children.append( schema );
  }
  return children;
}

bool QgsPGConnectionItem::equal( const QgsDataItem *other )
{
  if ( type() != other->type() )
  {
    return false;
  }
  const QgsPGConnectionItem *o = dynamic_cast<const QgsPGConnectionItem *>( other );
  return ( mPath == o->mPath && mName == o->mName && mConnInfo == o->mConnInfo );
}

QList<QAction*> QgsPGConnectionItem::actions()
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

void QgsPGConnectionItem::editConnection()
{
  QgsPgNewConnection nc( NULL, mName );
  if ( nc.exec() )
  {
    // the parent should be updated
    mParent->refresh();
  }
}

void QgsPGConnectionItem::deleteConnection()
{
  QgsPgSourceSelect::deleteConnection( mName );
  // the parent should be updated
  mParent->refresh();
}


// ---------------------------------------------------------------------------
QgsPGLayerItem::QgsPGLayerItem( QgsDataItem* parent, QString name, QString path, QString connInfo, QgsLayerItem::LayerType layerType, QgsPostgresLayerProperty layerProperty )
    : QgsLayerItem( parent, name, path, QString(), layerType, "postgres" ),
    mConnInfo( connInfo ),
    mLayerProperty( layerProperty )
{
  mUri = createUri();
  mPopulated = true;
}

QgsPGLayerItem::~QgsPGLayerItem()
{
}

QString QgsPGLayerItem::createUri()
{
  QString pkColName = mLayerProperty.pkCols.size() > 0 ? mLayerProperty.pkCols.at( 0 ) : QString::null;
  QgsDataSourceURI uri( mConnInfo );
  uri.setDataSource( mLayerProperty.schemaName, mLayerProperty.tableName, mLayerProperty.geometryColName, mLayerProperty.sql, pkColName );
  return uri.uri();
}

// ---------------------------------------------------------------------------
QgsPGSchemaItem::QgsPGSchemaItem( QgsDataItem* parent, QString name, QString path, QString connInfo, QVector<QgsPostgresLayerProperty> layerProperties )
    : QgsDataCollectionItem( parent, name, path )
{
  mIcon = QIcon( getThemePixmap( "mIconNamespace.png" ) );

  // Populate everything, it costs nothing, all info about layers is collected
  foreach( QgsPostgresLayerProperty layerProperty, layerProperties )
  {
    QgsDebugMsg( "table: " + layerProperty.schemaName + "." + layerProperty.tableName );

    QgsLayerItem::LayerType layerType = QgsLayerItem::NoType;
    if ( layerProperty.type.contains( "POINT" ) )
    {
      layerType = QgsLayerItem::Point;
    }
    else if ( layerProperty.type.contains( "LINE" ) )
    {
      layerType = QgsLayerItem::Line;
    }
    else if ( layerProperty.type.contains( "POLYGON" ) )
    {
      layerType = QgsLayerItem::Polygon;
    }
    else if ( layerProperty.type == QString::null )
    {
      if ( layerProperty.geometryColName == QString::null )
      {
        layerType = QgsLayerItem::TableLayer;
      }
    }

    QgsPGLayerItem * layer = new QgsPGLayerItem( this, layerProperty.tableName, mPath + "/" + layerProperty.tableName, connInfo, layerType, layerProperty );
    mChildren.append( layer );
  }

  mPopulated = true;
}

QgsPGSchemaItem::~QgsPGSchemaItem()
{
}

// ---------------------------------------------------------------------------
QgsPGRootItem::QgsPGRootItem( QgsDataItem* parent, QString name, QString path )
    : QgsDataCollectionItem( parent, name, path )
{
  //mIcon = QIcon( getThemePixmap( "mIconPg.png" ) );
  populate();
}

QgsPGRootItem::~QgsPGRootItem()
{
}

QVector<QgsDataItem*>QgsPGRootItem::createChildren()
{
  QVector<QgsDataItem*> connections;
  foreach( QString connName,  QgsPostgresConnection::connectionList() )
  {
    QgsDataItem * conn = new QgsPGConnectionItem( this, connName, mPath + "/" + connName );
    connections.push_back( conn );
  }
  return connections;
}

QList<QAction*> QgsPGRootItem::actions()
{
  QList<QAction*> lst;

  QAction* actionNew = new QAction( tr( "New..." ), this );
  connect( actionNew, SIGNAL( triggered() ), this, SLOT( newConnection() ) );
  lst.append( actionNew );

  return lst;
}

QWidget * QgsPGRootItem::paramWidget()
{
  QgsPgSourceSelect *select = new QgsPgSourceSelect( 0, 0, true, true );
  connect( select, SIGNAL( connectionsChanged() ), this, SLOT( connectionsChanged() ) );
  return select;
}
void QgsPGRootItem::connectionsChanged()
{
  refresh();
}

void QgsPGRootItem::newConnection()
{
  QgsPgNewConnection nc( NULL );
  if ( nc.exec() )
  {
    refresh();
  }
}

// ---------------------------------------------------------------------------

QGISEXTERN QgsDataItem * dataItem( QString thePath, QgsDataItem* parentItem )
{
  Q_UNUSED( thePath );
  return new QgsPGRootItem( parentItem, "PostGIS", "pg:" );
}
