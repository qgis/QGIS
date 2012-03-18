#include "qgspostgresdataitems.h"

#include "qgspostgresconn.h"
#include "qgspgnewconnection.h"
#include "qgscolumntypethread.h"
#include "qgslogger.h"
#include "qgsdatasourceuri.h"

#include <QMessageBox>

// ---------------------------------------------------------------------------
QgsPGConnectionItem::QgsPGConnectionItem( QgsDataItem* parent, QString name, QString path )
    : QgsDataCollectionItem( parent, name, path )
{
  mIcon = QIcon( getThemePixmap( "mIconConnect.png" ) );
}

QgsPGConnectionItem::~QgsPGConnectionItem()
{
}

QVector<QgsDataItem*> QgsPGConnectionItem::createChildren()
{
  QgsDebugMsg( "Entered" );
  QVector<QgsDataItem*> children;
  QgsDataSourceURI uri = QgsPostgresConn::connUri( mName );

  mConn = QgsPostgresConn::connectDb( uri.connectionInfo(), true );
  if ( !mConn )
    return children;

  QVector<QgsPostgresLayerProperty> layerProperties;
  if ( !mConn->supportedLayers( layerProperties,
                                QgsPostgresConn::geometryColumnsOnly( mName ),
                                QgsPostgresConn::publicSchemaOnly( mName ),
                                QgsPostgresConn::allowGeometrylessTables( mName ) ) )
  {
    children.append( new QgsErrorItem( this, tr( "Failed to retrieve layers" ), mPath + "/error" ) );
    return children;
  }

  if ( layerProperties.isEmpty() )
  {
    children.append( new QgsErrorItem( this, tr( "No layers found." ), mPath + "/error" ) );
    return children;
  }

  QgsGeomColumnTypeThread *columnTypeThread = 0;

  foreach( QgsPostgresLayerProperty layerProperty, layerProperties )
  {
    QgsPGSchemaItem *schemaItem = mSchemaMap.value( layerProperty.schemaName, 0 );
    if ( !schemaItem )
    {
      schemaItem = new QgsPGSchemaItem( this, layerProperty.schemaName, mPath + "/" + layerProperty.schemaName );
      children.append( schemaItem );
      mSchemaMap[ layerProperty.schemaName ] = schemaItem;
    }

    if ( layerProperty.type == "GEOMETRY" )
    {
      if ( !columnTypeThread )
      {
        QgsPostgresConn *conn = QgsPostgresConn::connectDb( uri.connectionInfo(), true /* readonly */ );
        if ( conn )
        {
          columnTypeThread = new QgsGeomColumnTypeThread( conn, true /* use estimated metadata */ );

          connect( columnTypeThread, SIGNAL( setLayerType( QgsPostgresLayerProperty ) ),
                   this, SLOT( setLayerType( QgsPostgresLayerProperty ) ) );
          connect( this, SIGNAL( addGeometryColumn( QgsPostgresLayerProperty ) ),
                   columnTypeThread, SLOT( addGeometryColumn( QgsPostgresLayerProperty ) ) );
        }
      }

      emit addGeometryColumn( layerProperty );

      continue;
    }

    schemaItem->addLayer( layerProperty );
  }

  if ( columnTypeThread )
    columnTypeThread->start();

  return children;
}

void QgsPGConnectionItem::setLayerType( QgsPostgresLayerProperty layerProperty )
{
  QgsPGSchemaItem *schemaItem = mSchemaMap.value( layerProperty.schemaName, 0 );

  if ( !schemaItem )
  {
    QgsDebugMsg( QString( "schema item for %1 not found." ).arg( layerProperty.schemaName ) );
    return;
  }

  QStringList typeList = layerProperty.type.split( ",", QString::SkipEmptyParts );
  QStringList sridList = layerProperty.srid.split( ",", QString::SkipEmptyParts );
  Q_ASSERT( typeList.size() == sridList.size() );

  for ( int i = 0 ; i < typeList.size(); i++ )
  {
    QGis::GeometryType geomType = QgsPostgresConn::geomTypeFromPostgis( typeList[i] );
    if ( geomType == QGis::UnknownGeometry )
    {
      QgsDebugMsg( QString( "unsupported geometry type:%1" ).arg( typeList[i] ) );
      continue;
    }

    layerProperty.type = typeList[i];
    layerProperty.srid = sridList[i];
    schemaItem->addLayer( layerProperty );
  }
}

bool QgsPGConnectionItem::equal( const QgsDataItem *other )
{
  if ( type() != other->type() )
  {
    return false;
  }

  const QgsPGConnectionItem *o = qobject_cast<const QgsPGConnectionItem *>( other );
  return ( mPath == o->mPath && mName == o->mName && o->connection() == connection() );
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
  QgsPostgresConn::deleteConnection( mName );
  // the parent should be updated
  mParent->refresh();
}

bool QgsPGConnectionItem::handleDrop( const QMimeData * data, Qt::DropAction )
{
  if ( !QgsMimeDataUtils::isUriList( data ) )
    return false;

  // TODO: probably should show a GUI with settings etc
  QgsDataSourceURI uri = QgsPostgresConn::connUri( mName );

  qApp->setOverrideCursor( Qt::WaitCursor );

  QStringList importResults;
  bool hasError = false;
  QgsMimeDataUtils::UriList lst = QgsMimeDataUtils::decodeUriList( data );
  foreach( const QgsMimeDataUtils::Uri& u, lst )
  {
    if ( u.layerType != "vector" )
    {
      importResults.append( tr( "%1: Not a vector layer!" ).arg( u.name ) );
      hasError = true; // only vectors can be imported
      continue;
    }

    // open the source layer
    QgsVectorLayer* srcLayer = new QgsVectorLayer( u.uri, u.name, u.providerKey );

    if ( srcLayer->isValid() )
    {
      uri.setDataSource( QString(), u.name, "qgs_geometry" );
      QgsDebugMsg( "URI " + uri.uri() );
      QgsVectorLayerImport::ImportError err;
      QString importError;
      err = QgsVectorLayerImport::importLayer( srcLayer, uri.uri(), "postgres", &srcLayer->crs(), false, &importError );
      if ( err == QgsVectorLayerImport::NoError )
        importResults.append( tr( "%1: OK!" ).arg( u.name ) );
      else
      {
        importResults.append( QString( "%1: %2" ).arg( u.name ).arg( importError ) );
        hasError = true;
      }
    }
    else
    {
      importResults.append( tr( "%1: OK!" ).arg( u.name ) );
      hasError = true;
    }

    delete srcLayer;
  }

  qApp->restoreOverrideCursor();

  if ( hasError )
  {
    QMessageBox::warning( 0, tr( "Import to PostGIS database" ), tr( "Failed to import some layers!\n\n" ) + importResults.join( "\n" ) );
  }
  else
  {
    QMessageBox::information( 0, tr( "Import to PostGIS database" ), tr( "Import was successful." ) );
  }

  refresh();

  return true;
}

// ---------------------------------------------------------------------------
QgsPGLayerItem::QgsPGLayerItem( QgsDataItem* parent, QString name, QString path, QgsLayerItem::LayerType layerType, QgsPostgresLayerProperty layerProperty )
    : QgsLayerItem( parent, name, path, QString(), layerType, "postgres" )
    , mLayerProperty( layerProperty )
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
  QgsPGConnectionItem *connItem = qobject_cast<QgsPGConnectionItem *>( parent() ? parent()->parent() : 0 );

  if ( !connItem )
  {
    QgsDebugMsg( "connection item not found." );
    return QString::null;
  }

  QgsDebugMsg( QString( "connInfo: %1" ).arg( connItem->connection()->connInfo() ) );

  QgsDataSourceURI uri( connItem->connection()->connInfo() );
  uri.setDataSource( mLayerProperty.schemaName, mLayerProperty.tableName, mLayerProperty.geometryColName, mLayerProperty.sql, pkColName );
  uri.setSrid( mLayerProperty.srid );
  uri.setGeometryType( QgsPostgresConn::geomTypeFromPostgis( mLayerProperty.type ) );
  QgsDebugMsg( QString( "layer uri: %1" ).arg( uri.uri() ) );
  return uri.uri();
}

// ---------------------------------------------------------------------------
QgsPGSchemaItem::QgsPGSchemaItem( QgsDataItem* parent, QString name, QString path )
    : QgsDataCollectionItem( parent, name, path )
{
  mIcon = QIcon( getThemePixmap( "mIconDbSchema.png" ) );
}

QVector<QgsDataItem*> QgsPGSchemaItem::createChildren()
{
  QgsDebugMsg( "Entering." );
  return QVector<QgsDataItem*>();
}

QgsPGSchemaItem::~QgsPGSchemaItem()
{
}

void QgsPGSchemaItem::addLayer( QgsPostgresLayerProperty layerProperty )
{
  QGis::GeometryType geomType = QgsPostgresConn::geomTypeFromPostgis( layerProperty.type );
  QString tip = tr( "%1 as %2 in %3" ).arg( layerProperty.geometryColName ).arg( QgsPostgresConn::displayStringForGeomType( geomType ) ).arg( layerProperty.srid );

  QgsLayerItem::LayerType layerType;
  switch ( geomType )
  {
    case QGis::Point:
      layerType = QgsLayerItem::Point;
      break;
    case QGis::Line:
      layerType = QgsLayerItem::Line;
      break;
    case QGis::Polygon:
      layerType = QgsLayerItem::Polygon;
      break;
    default:
      if ( layerProperty.type.isEmpty() && layerProperty.geometryColName.isEmpty() )
      {
        layerType = QgsLayerItem::TableLayer;
        tip = tr( "as geometryless table" );
      }
      else
      {
        return;
      }
  }

  QgsPGLayerItem *layerItem = new QgsPGLayerItem( this, layerProperty.tableName, mPath + "/" + layerProperty.tableName, layerType, layerProperty );
  layerItem->setToolTip( tip );
  addChild( layerItem );
}

// ---------------------------------------------------------------------------
QgsPGRootItem::QgsPGRootItem( QgsDataItem* parent, QString name, QString path )
    : QgsDataCollectionItem( parent, name, path )
{
  mIcon = QIcon( getThemePixmap( "mIconPostgis.png" ) );
  populate();
}

QgsPGRootItem::~QgsPGRootItem()
{
}

QVector<QgsDataItem*> QgsPGRootItem::createChildren()
{
  QVector<QgsDataItem*> connections;
  foreach( QString connName, QgsPostgresConn::connectionList() )
  {
    connections << new QgsPGConnectionItem( this, connName, mPath + "/" + connName );
  }
  return connections;
}

QList<QAction*> QgsPGRootItem::actions()
{
  QList<QAction*> lst;

  QAction* actionNew = new QAction( tr( "New Connection..." ), this );
  connect( actionNew, SIGNAL( triggered() ), this, SLOT( newConnection() ) );
  lst.append( actionNew );

  return lst;
}

QWidget *QgsPGRootItem::paramWidget()
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
