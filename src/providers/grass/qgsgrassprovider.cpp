/***************************************************************************
    qgsgrassprovider.cpp -  Data provider for GRASS format
                             -------------------
    begin                : March, 2004
    copyright            : (C) 2004 by Gary E.Sherman, Radim Blazek
    email                : sherman@mrcc.com, blazek@itc.it
 ***************************************************************************/
/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <cstring>
#include <vector>
#include <cfloat>

#include <QString>
#include <QDateTime>

#include "qgis.h"
#include "qgsdataprovider.h"
#include "qgsfeature.h"
#include "qgsfield.h"
#include "qgsrectangle.h"
#include "qgsvectorlayer.h"
#include "qgsvectorlayereditbuffer.h"

#include "qgsgrass.h"
#include "qgsgrassprovider.h"
#include "qgsgrassfeatureiterator.h"
#include "qgsgrassvector.h"

#include "qgsapplication.h"
#include "qgscoordinatereferencesystem.h"
#include "qgslogger.h"

#include <QByteArray>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QMessageBox>
#include <QTextCodec>

#ifdef _MSC_VER
// enable grass prototypes
#define __STDC__ 1
#endif

extern "C"
{
#include <grass/version.h>
#include <grass/gprojects.h>
#include <grass/gis.h>
#include <grass/dbmi.h>
#if GRASS_VERSION_MAJOR < 7
#include <grass/Vect.h>
#else
#include <grass/vector.h>
#define BOUND_BOX bound_box
#endif
}

#ifdef _MSC_VER
#undef __STDC__
#endif

static QString GRASS_KEY = "grass";

QgsGrassProvider::QgsGrassProvider( QString uri )
    : QgsVectorDataProvider( uri )
    , mLayerField( -1 )
    , mLayerType( POINT )
    , mGrassType( 0 )
    , mQgisType( QGis::WKBUnknown )
    , mLayer( 0 )
    , mMapVersion( 0 )
    , mCidxFieldIndex( -1 )
    , mCidxFieldNumCats( 0 )
    , mNumberFeatures( 0 )
    , mEditBuffer( 0 )
{
  QgsDebugMsg( "uri = " + uri );

  mValid = false;
  if ( !QgsGrass::init() )
  {
    appendError( QgsGrass::errorMessage() );
    return;
  }

  QTime time;
  time.start();

  // Parse URI
  QDir dir( uri );   // it is not a directory in fact
  QString myURI = dir.path();  // no dupl '/'

  mLayerName = dir.dirName();
  myURI = myURI.left( dir.path().lastIndexOf( '/' ) );
  dir = QDir( myURI );
  QString mapName = dir.dirName();
  dir.cdUp();
  QString mapset = dir.dirName();
  dir.cdUp();
  QString location = dir.dirName();
  dir.cdUp();
  QString gisdbase = dir.path();

  mGrassObject = QgsGrassObject( gisdbase, location, mapset, mapName );
  QgsDebugMsg( "mGrassObject = " + mGrassObject.toString() + " mLayerName = " + mLayerName );

  /* Parse Layer, supported layers <field>_point, <field>_line, <field>_area
  *  Layer is opened even if it is empty (has no features)
  */
  if ( mLayerName.compare( "boundary" ) == 0 ) // currently not used
  {
    mLayerType = BOUNDARY;
    mGrassType = GV_BOUNDARY;
  }
  else if ( mLayerName.compare( "centroid" ) == 0 ) // currently not used
  {
    mLayerType = CENTROID;
    mGrassType = GV_CENTROID;
  }
  else if ( mLayerName == "topo_point" )
  {
    mLayerType = TOPO_POINT;
    mGrassType = GV_POINTS;
  }
  else if ( mLayerName == "topo_line" )
  {
    mLayerType = TOPO_LINE;
    mGrassType = GV_LINES;
  }
  else if ( mLayerName == "topo_node" )
  {
    mLayerType = TOPO_NODE;
    mGrassType = 0;
  }
  else
  {
    mLayerField = grassLayer( mLayerName );
    if ( mLayerField == -1 )
    {
      QgsDebugMsg( QString( "Invalid layer name, no underscore found: %1" ).arg( mLayerName ) );
      return;
    }

    mGrassType = grassLayerType( mLayerName );

    if ( mGrassType == GV_POINT )
    {
      mLayerType = POINT;
    }
    else if ( mGrassType == GV_LINES )
    {
      mLayerType = LINE;
    }
    else if ( mGrassType == GV_FACE )
    {
      mLayerType = FACE;
    }
    else if ( mGrassType == GV_AREA )
    {
      mLayerType = POLYGON;
    }
    else
    {
      QgsDebugMsg( QString( "Invalid layer name, wrong type: %1" ).arg( mLayerName ) );
      return;
    }

  }
  QgsDebugMsg( QString( "mLayerField: %1" ).arg( mLayerField ) );
  QgsDebugMsg( QString( "mLayerType: %1" ).arg( mLayerType ) );

  if ( mLayerType == BOUNDARY || mLayerType == CENTROID )
  {
    QgsDebugMsg( "Layer type not supported." );
    return;
  }

  // Set QGIS type
  switch ( mLayerType )
  {
    case POINT:
    case CENTROID:
    case TOPO_POINT:
    case TOPO_NODE:
      mQgisType = QGis::WKBPoint;
      break;
    case LINE:
    case BOUNDARY:
    case TOPO_LINE:
      mQgisType = QGis::WKBLineString;
      break;
    case POLYGON:
    case FACE:
      mQgisType = QGis::WKBPolygon;
      break;
  }

  // the map may be invalid (e.g. wrong uri or open failed)
  QgsGrassVectorMap *vectorMap = QgsGrassVectorMapStore::instance()->openMap( mGrassObject );
  if ( !vectorMap ) // should not happen
  {
    QgsDebugMsg( QString( "Cannot open map : %1" ).arg( myURI ) );
    return;
  }

  mLayer = vectorMap->openLayer( mLayerField );

  if ( !mLayer ) // should not happen
  {
    QgsDebugMsg( QString( "Cannot open GRASS layer : %1" ).arg( myURI ) );
    return;
  }

  loadMapInfo();
  setTopoFields();

  //mEditFields = QgsFields( mLayers[mLayerId].fields );
  //mEditFields.append( QgsField( "topo_symbol", QVariant::Int ) );

  mLayer->map()->version();

  mValid = true;

  QgsDebugMsg( QString( "New GRASS layer opened, time (ms): %1" ).arg( time.elapsed() ) );
}

int QgsGrassProvider::capabilities() const
{
  // for now, only one map may be edited at time
  if ( mEditBuffer || ( mLayer && mLayer->map() && !mLayer->map()->isEdited() ) )
  {
    //return AddFeatures | DeleteFeatures | ChangeAttributeValues | AddAttributes | DeleteAttributes | ChangeGeometries;
    return ChangeGeometries;
  }
  return 0;
}

void QgsGrassProvider::loadMapInfo()
{
  // Getting the total number of features in the layer
  mNumberFeatures = 0;
  mCidxFieldIndex = -1;
  if ( mLayerType == TOPO_POINT )
  {
    mNumberFeatures = Vect_get_num_primitives( map(), GV_POINTS );
  }
  else if ( mLayerType == TOPO_LINE )
  {
    mNumberFeatures = Vect_get_num_primitives( map(), GV_LINES );
  }
  else if ( mLayerType == TOPO_NODE )
  {
    mNumberFeatures = Vect_get_num_nodes( map() );
  }
  else
  {
    if ( mLayerField >= 0 )
    {
      mCidxFieldIndex = Vect_cidx_get_field_index( map(), mLayerField );
      if ( mCidxFieldIndex >= 0 )
      {
        mNumberFeatures = Vect_cidx_get_type_count( map(), mLayerField, mGrassType );
        mCidxFieldNumCats = Vect_cidx_get_num_cats_by_index( map(), mCidxFieldIndex );
      }
    }
    else
    {
      // TODO nofield layers
      mNumberFeatures = 0;
      mCidxFieldNumCats = 0;
    }
  }
  QgsDebugMsg( QString( "mNumberFeatures = %1 mCidxFieldIndex = %2 mCidxFieldNumCats = %3" ).arg( mNumberFeatures ).arg( mCidxFieldIndex ).arg( mCidxFieldNumCats ) );

}

void QgsGrassProvider::update( void )
{
  QgsDebugMsg( "entered" );
  // TODO
#if 0
  mValid = false;

  if ( !map()s[mLayers[mLayerId].mapId].valid )
    return;

  // Getting the total number of features in the layer
  // It may happen that the field disappeares from the map (deleted features, new map without that field)
  loadMapInfo();

  map()Version = map()s[mLayers[mLayerId].mapId].version;

  mValid = true;
#endif
}

QgsGrassProvider::~QgsGrassProvider()
{
  QgsDebugMsg( "entered" );
  mLayer->close();
}


QgsAbstractFeatureSource* QgsGrassProvider::featureSource() const
{
  const_cast<QgsGrassProvider*>( this )->ensureUpdated();
  return new QgsGrassFeatureSource( this );
}


QString QgsGrassProvider::storageType() const
{
  return "GRASS (Geographic Resources Analysis and Support System) file";
}



QgsFeatureIterator QgsGrassProvider::getFeatures( const QgsFeatureRequest& request )
{
  //if ( isEdited() || isFrozen() || !mValid )
  if ( isFrozen() || !mValid )
  {
    return QgsFeatureIterator();
  }

  // check if outdated and update if necessary
  ensureUpdated();

  QgsGrassFeatureSource * source = new QgsGrassFeatureSource( this );
  QgsGrassFeatureIterator * iterator = new QgsGrassFeatureIterator( source, true, request );
  return QgsFeatureIterator( iterator );
}

QgsRectangle QgsGrassProvider::extent()
{
  if ( isValid() )
  {
    BOUND_BOX box;
    Vect_get_map_box( map(), &box );
    return QgsRectangle( box.W, box.S, box.E, box.N );
  }
  return QgsRectangle();
}

/**
* Return the feature type
*/
QGis::WkbType QgsGrassProvider::geometryType() const
{
  return mQgisType;
}
/**
* Return the feature type
*/
long QgsGrassProvider::featureCount() const
{
  return mNumberFeatures;
}

/**
* Return fields
*/
const QgsFields & QgsGrassProvider::fields() const
{
  // TODO
  return mLayer->fields();

  if ( mEditBuffer )
  {
    return mEditFields;
  }
  else if ( !isTopoType() )
  {
    return mLayer->fields();
  }
  return mTopoFields;
}

int QgsGrassProvider::keyField()
{
  return mLayer->keyColumn();
}

QVariant QgsGrassProvider::minimumValue( int index )
{
  if ( isValid() )
  {
    mLayer->minMax().value( index ).first;
  }
  return QVariant();
}


QVariant QgsGrassProvider::maxValue( int index )
{
  if ( isValid() )
  {
    mLayer->minMax().value( index ).second;
  }
  return QVariant();
}

bool QgsGrassProvider::isValid()
{
  QgsDebugMsg( QString( "returned: %1" ).arg( mValid ) );

  return mValid && mLayer && mLayer->map() && mLayer->map()->map();
}

QgsCoordinateReferenceSystem QgsGrassProvider::crs()
{
  return QgsGrass::crs( mGrassObject.gisdbase(), mGrassObject.location() );
}

int QgsGrassProvider::grassLayer()
{
  return mLayerField;
}

int QgsGrassProvider::grassLayer( QString name )
{
  // Get field number
  int pos = name.indexOf( '_' );

  if ( pos == -1 )
  {
    return -1;
  }

  return name.left( pos ).toInt();
}

int QgsGrassProvider::grassLayerType( QString name )
{
  int pos = name.indexOf( '_' );

  if ( pos == -1 )
  {
    return -1;
  }

  QString ts = name.right( name.length() - pos - 1 );
  if ( ts.compare( "point" ) == 0 )
  {
    return GV_POINT; // ?! centroids may be points
  }
  else if ( ts.compare( "line" ) == 0 )
  {
    return GV_LINES;
  }
  else if ( ts.compare( "face" ) == 0 )
  {
    return GV_FACE;
  }
  else if ( ts.compare( "polygon" ) == 0 )
  {
    return GV_AREA;
  }

  return -1;
}

//-----------------------------------------  Edit -------------------------------------------------------

bool QgsGrassProvider::isGrassEditable( void )
{
  QgsDebugMsg( "entered" );

  if ( !isValid() )
    return false;

  /* Check if current user is owner of mapset */
  if ( G__mapset_permissions2( mGrassObject.gisdbase().toUtf8().data(), mGrassObject.location().toUtf8().data(), mGrassObject.mapset().toUtf8().data() ) != 1 )
    return false;

  // TODO: check format? (cannot edit OGR layers)

  return true;
}

bool QgsGrassProvider::isEdited( void )
{
  QgsDebugMsgLevel( "entered", 3 );
  return ( mEditBuffer );
}

bool QgsGrassProvider::isFrozen( void )
{
  QgsDebugMsgLevel( "entered", 3 );

  // TODO ?
  //return ( map()->mFrozen );
  return false;
}

void QgsGrassProvider::freeze()
{
  QgsDebugMsg( "entered" );

  // TODO ?
#if 0
  if ( !isValid() )
    return;

  QgsGrassVectorMap *map = &( map()s[mLayers[mLayerId].mapId] );

  if ( map->mFrozen )
    return;

  map->mFrozen = true;
  Vect_close( map->map() );
#endif
}

void QgsGrassProvider::thaw()
{
  QgsDebugMsg( "entered" );

  // TODO ?
#if 0
  if ( !isValid() )
    return;
  QgsGrassVectorMap *map = &( map()s[mLayers[mLayerId].mapId] );

  if ( !map->mFrozen )
    return;

  if ( reopenMap() )
  {
    map->mFrozen = false;
  }
#endif
}

bool QgsGrassProvider::closeEdit( bool newMap )
{
  QgsDebugMsg( "entered" );

  if ( !isValid() )
  {
    QgsDebugMsg( "not valid" );
    return false;
  }

  mEditBuffer = 0;
  return mLayer->map()->closeEdit( newMap );
}

void QgsGrassProvider::ensureUpdated()
{
  // TODO
#if 0
  int mapId = mLayers[mLayerId].mapId;
  if ( mapOutdated( mapId ) )
  {
    updateMap( mapId );
  }
  if ( map()Version < map()s[mapId].version )
  {
    update();
  }
  if ( attributesOutdated( mapId ) )
  {
    loadAttributes( mLayers[mLayerId] );
  }
#endif
}

int QgsGrassProvider::numLines( void )
{
  QgsDebugMsg( "entered" );

  //return ( Vect_get_num_lines( map() ) );
  return mLayer->map()->numLines();
}

int QgsGrassProvider::numNodes( void )
{
  QgsDebugMsg( "entered" );

  return ( Vect_get_num_nodes( map() ) );
}

int QgsGrassProvider::readLine( struct line_pnts *Points, struct line_cats *Cats, int line )
{
  QgsDebugMsgLevel( "entered", 3 );

  if ( Points )
    Vect_reset_line( Points );

  if ( Cats )
    Vect_reset_cats( Cats );

  if ( !Vect_line_alive( map(), line ) )
    return -1;

  return ( Vect_read_line( map(), Points, Cats, line ) );
}

bool QgsGrassProvider::nodeCoor( int node, double *x, double *y )
{
  QgsDebugMsgLevel( "entered", 3 );

  if ( !Vect_node_alive( map(), node ) )
  {
    *x = 0.0;
    *y = 0.0;
    return false;
  }

  Vect_get_node_coor( map(), node, x, y, NULL );
  return true;
}

bool QgsGrassProvider::lineNodes( int line, int *node1, int *node2 )
{
  QgsDebugMsgLevel( "entered", 3 );

  if ( !Vect_line_alive( map(), line ) )
  {
    *node1 = 0;
    *node2 = 0;
    return false;
  }

#if GRASS_VERSION_MAJOR < 7
  Vect_get_line_nodes( map(), line, node1, node2 );
#else
  /* points don't have topology in GRASS >= 7 */
  *node1 = 0;
  *node2 = 0;
#endif
  return true;
}

int QgsGrassProvider::writeLine( int type, struct line_pnts *Points, struct line_cats *Cats )
{
  QgsDebugMsg( QString( "n_points = %1 n_cats = %2" ).arg( Points->n_points ).arg( Cats->n_cats ) );

  if ( !isEdited() )
    return -1;

  return (( int ) Vect_write_line( map(), type, Points, Cats ) );
}

int QgsGrassProvider::rewriteLine( int line, int type, struct line_pnts *Points, struct line_cats *Cats )
{
  QgsDebugMsg( QString( "n_points = %1 n_cats = %2" ).arg( Points->n_points ).arg( Cats->n_cats ) );

  if ( !isEdited() )
    return -1;

  return ( Vect_rewrite_line( map(), line, type, Points, Cats ) );
}


int QgsGrassProvider::deleteLine( int line )
{
  QgsDebugMsg( "entered" );

  if ( !isEdited() )
    return -1;

  return ( Vect_delete_line( map(), line ) );
}

int QgsGrassProvider::findLine( double x, double y, int type, double threshold )
{
  QgsDebugMsgLevel( "entered", 3 );

  return ( Vect_find_line( map(), x, y, 0, type, threshold, 0, 0 ) );
}

int QgsGrassProvider::findNode( double x, double y, double threshold )
{
  return ( Vect_find_node( map(), x, y, 0, threshold, 0 ) );
}

bool QgsGrassProvider::lineAreas( int line, int *left, int *right )
{
  QgsDebugMsgLevel( "entered", 3 );

  if ( !Vect_line_alive( map(), line ) )
  {
    *left = 0;
    *right = 0;
    return false;
  }

  Vect_get_line_areas( map(), line, left, right );
  return true;
}

int QgsGrassProvider::isleArea( int isle )
{
  QgsDebugMsgLevel( "entered", 3 );

  if ( !Vect_isle_alive( map(), isle ) )
  {
    return 0;
  }

  return ( Vect_get_isle_area( map(), isle ) );
}

int QgsGrassProvider::centroidArea( int centroid )
{
  QgsDebugMsgLevel( "entered", 3 );

  if ( !Vect_line_alive( map(), centroid ) )
  {
    return 0;
  }

  return ( Vect_get_centroid_area( map(), centroid ) );
}

int QgsGrassProvider::nodeNLines( int node )
{
  QgsDebugMsgLevel( "entered", 3 );

  if ( !Vect_node_alive( map(), node ) )
  {
    return 0;
  }

  return ( Vect_get_node_n_lines( map(), node ) );
}

int QgsGrassProvider::nodeLine( int node, int idx )
{
  QgsDebugMsgLevel( "entered", 3 );

  if ( !Vect_node_alive( map(), node ) )
  {
    return 0;
  }

  return ( Vect_get_node_line( map(), node, idx ) );
}

int QgsGrassProvider::lineAlive( int line )
{
  QgsDebugMsgLevel( "entered", 3 );

  return ( Vect_line_alive( map(), line ) );
}

int QgsGrassProvider::nodeAlive( int node )
{
  QgsDebugMsgLevel( "QgsGrassProvider::nodeAlive", 3 );

  return ( Vect_node_alive( map(), node ) );
}

int QgsGrassProvider::numUpdatedLines( void )
{
  QgsDebugMsg( QString( "numUpdatedLines = %1" ).arg( Vect_get_num_updated_lines( map() ) ) );

  return ( Vect_get_num_updated_lines( map() ) );
}

int QgsGrassProvider::numUpdatedNodes( void )
{
  QgsDebugMsg( QString( "numUpdatedNodes = %1" ).arg( Vect_get_num_updated_nodes( map() ) ) );

  return ( Vect_get_num_updated_nodes( map() ) );
}

int QgsGrassProvider::updatedLine( int idx )
{
  QgsDebugMsg( QString( "idx = %1" ).arg( idx ) );
  QgsDebugMsg( QString( "  updatedLine = %1" ).arg( Vect_get_updated_line( map(), idx ) ) );

  return ( Vect_get_updated_line( map(), idx ) );
}

int QgsGrassProvider::updatedNode( int idx )
{
  QgsDebugMsg( QString( "idx = %1" ).arg( idx ) );
  QgsDebugMsg( QString( "  updatedNode = %1" ).arg( Vect_get_updated_node( map(), idx ) ) );

  return ( Vect_get_updated_node( map(), idx ) );
}

// ------------------ Attributes -------------------------------------------------

QString QgsGrassProvider::key( int field )
{
  QgsDebugMsg( QString( "field = %1" ).arg( field ) );

  struct field_info *fi = Vect_get_field( map(), field ); // should work also with field = 0
  if ( !fi )
  {
    QgsDebugMsg( "No field info -> no attributes" );
    return QString();
  }

  return QString::fromUtf8( fi->key );
}

QgsAttributeMap *QgsGrassProvider::attributes( int field, int cat )
{
  QgsDebugMsg( QString( "field = %1 cat = %2" ).arg( field ).arg( cat ) );

  QgsAttributeMap *att = new QgsAttributeMap;

  struct  field_info *fi = Vect_get_field( map(), field ); // should work also with field = 0

  // Read attributes
  if ( !fi )
  {
    QgsDebugMsg( "No field info -> no attributes" );
    return att;
  }

  QgsDebugMsg( "Field info found -> open database" );
  setMapset();
  dbDriver *driver = db_start_driver_open_database( fi->driver, fi->database );

  if ( !driver )
  {
    QgsDebugMsg( QString( "Cannot open database %1 by driver %2" ).arg( fi->database ).arg( fi->driver ) );
    return att;
  }

  QgsDebugMsg( "Database opened -> read attributes" );

  dbString dbstr;
  db_init_string( &dbstr );
  QString query;
  query.sprintf( "select * from %s where %s = %d", fi->table, fi->key, cat );
  db_set_string( &dbstr, query.toUtf8().data() );

  QgsDebugMsg( QString( "SQL: %1" ).arg( db_get_string( &dbstr ) ) );

  dbCursor databaseCursor;
  if ( db_open_select_cursor( driver, &dbstr, &databaseCursor, DB_SCROLL ) != DB_OK )
  {
    db_close_database_shutdown_driver( driver );
    QgsDebugMsg( "Cannot select attributes from table" );
    return att;
  }

  int nRecords = db_get_num_rows( &databaseCursor );
  QgsDebugMsg( QString( "Number of records: %1" ).arg( nRecords ) );

  if ( nRecords < 1 )
  {
    db_close_database_shutdown_driver( driver );
    QgsDebugMsg( "No DB record" );
    return att;
  }

  dbTable  *databaseTable = db_get_cursor_table( &databaseCursor );
  int nColumns = db_get_table_number_of_columns( databaseTable );

  int more;
  if ( db_fetch( &databaseCursor, DB_NEXT, &more ) != DB_OK )
  {
    db_close_database_shutdown_driver( driver );
    QgsDebugMsg( "Cannot fetch DB record" );
    return att;
  }

  // Read columns' description
  for ( int i = 0; i < nColumns; i++ )
  {
    dbColumn *column = db_get_table_column( databaseTable, i );
    db_convert_column_value_to_string( column, &dbstr );

    QString v = mEncoding->toUnicode( db_get_string( &dbstr ) );
    QgsDebugMsg( QString( "Value: %1" ).arg( v ) );
    att->insert( i, QVariant( v ) );
  }

  db_close_cursor( &databaseCursor );
  db_close_database_shutdown_driver( driver );
  db_free_string( &dbstr );

  return att;
}

QString QgsGrassProvider::updateAttributes( int field, int cat, const QString &values )
{
  QgsDebugMsg( QString( "field = %1 cat = %2" ).arg( field ).arg( cat ) );

  // Read attributes
  struct field_info *fi = Vect_get_field( map(), field ); // should work also with field = 0
  if ( !fi )
  {
    QgsDebugMsg( "No field info -> no attributes" );
    return "Cannot get field info";
  }

  QgsDebugMsg( "Field info found -> open database" );
  setMapset();

  dbDriver *driver = db_start_driver_open_database( fi->driver, fi->database );
  if ( !driver )
  {
    QgsDebugMsg( QString( "Cannot open database %1 by driver %2" ).arg( fi->database ).arg( fi->driver ) );
    return "Cannot open database";
  }

  QgsDebugMsg( "Database opened -> read attributes" );

  dbString dbstr;
  db_init_string( &dbstr );
  QString query;

  query = "update " + QString( fi->table ) + " set " + values + " where " + QString( fi->key )
          + " = " + QString::number( cat );

  QgsDebugMsg( QString( "query: %1" ).arg( query ) );

  // For some strange reason, mEncoding->fromUnicode(query) does not work,
  // but probably it is not correct, because Qt widgets will use current locales for input
  //  -> it is possible to edit only in current locales at present
  // QCString qcs = mEncoding->fromUnicode(query);

  QByteArray qcs = query.toUtf8();
  QgsDebugMsg( QString( "qcs: %1" ).arg( qcs.data() ) );

  char *cs = new char[qcs.length() + 1];
  strcpy( cs, ( const char * )qcs );
  db_set_string( &dbstr, cs );
  delete[] cs;

  QgsDebugMsg( QString( "SQL: %1" ).arg( db_get_string( &dbstr ) ) );

  QString error;
  int ret = db_execute_immediate( driver, &dbstr );
  if ( ret != DB_OK )
  {
    QgsDebugMsg( QString( "Error: %1" ).arg( db_get_error_msg() ) );
    error = QString::fromLatin1( db_get_error_msg() );
  }

  db_close_database_shutdown_driver( driver );
  db_free_string( &dbstr );

  return error;
}

int QgsGrassProvider::numDbLinks( void )
{
  QgsDebugMsg( "entered" );

  return ( Vect_get_num_dblinks( map() ) );
}

int QgsGrassProvider::dbLinkField( int link )
{
  QgsDebugMsg( "entered" );

  struct  field_info *fi = Vect_get_dblink( map(), link );

  if ( !fi )
    return 0;

  return ( fi->number );
}

QString QgsGrassProvider::executeSql( int field, const QString &sql )
{
  QgsDebugMsg( QString( "field = %1" ).arg( field ) );

  // Read attributes
  struct  field_info *fi = Vect_get_field( map(), field ); // should work also with field = 0
  if ( !fi )
  {
    QgsDebugMsg( "No field info -> no attributes" );
    return "Cannot get field info";
  }

  QgsDebugMsg( "Field info found -> open database" );

  setMapset();
  dbDriver *driver = db_start_driver_open_database( fi->driver, fi->database );

  if ( !driver )
  {
    QgsDebugMsg( QString( "Cannot open database %1 by driver %2" ).arg( fi->database ).arg( fi->driver ) );
    return "Cannot open database";
  }

  QgsDebugMsg( "Database opened" );

  dbString dbstr;
  db_init_string( &dbstr );
  db_set_string( &dbstr, sql.toLatin1().data() );

  QgsDebugMsg( QString( "SQL: %1" ).arg( db_get_string( &dbstr ) ) );

  QString error;

  int ret = db_execute_immediate( driver, &dbstr );
  if ( ret != DB_OK )
  {
    QgsDebugMsg( QString( "Error: %1" ).arg( db_get_error_msg() ) );
    error = QString::fromLatin1( db_get_error_msg() );
  }

  db_close_database_shutdown_driver( driver );
  db_free_string( &dbstr );

  return error;

}

QString QgsGrassProvider::createTable( int field, const QString &key, const QString &columns )
{
  QgsDebugMsg( QString( "field = %1" ).arg( field ) );

  // TODO: use QgsGrass::createTable

  // Read attributes
  struct field_info *fi = Vect_get_field( map(), field ); // should work also with field = 0
  if ( fi != NULL )
  {
    QgsDebugMsg( "The table for this field already exists" );
    return "The table for this field already exists";
  }

  QgsDebugMsg( "Field info not found -> create new table" );

  // We must set mapset before Vect_default_field_info
  setMapset();

  int nLinks = Vect_get_num_dblinks( map() );
  if ( nLinks == 0 )
  {
    fi = Vect_default_field_info( map(), field, NULL, GV_1TABLE );
  }
  else
  {
    fi = Vect_default_field_info( map(), field, NULL, GV_MTABLE );
  }

  dbDriver *driver = db_start_driver_open_database( fi->driver, fi->database );
  if ( !driver )
  {
    QgsDebugMsg( QString( "Cannot open database %1 by driver %2" ).arg( fi->database ).arg( fi->driver ) );
    return "Cannot open database";
  }

  QgsDebugMsg( "Database opened -> create table" );

  dbString dbstr;
  db_init_string( &dbstr );
  QString query;

  query.sprintf( "create table %s ( %s )", fi->table, columns.toLatin1().constData() );
  db_set_string( &dbstr, query.toLatin1().data() );

  QgsDebugMsg( QString( "SQL: %1" ).arg( db_get_string( &dbstr ) ) );

  QString error;
  int ret = db_execute_immediate( driver, &dbstr );
  if ( ret != DB_OK )
  {
    QgsDebugMsg( QString( "Error: %1" ).arg( db_get_error_msg() ) );
    error = QString::fromLatin1( db_get_error_msg() );
  }

  db_close_database_shutdown_driver( driver );
  db_free_string( &dbstr );

  if ( !error.isEmpty() )
    return error;

  ret = Vect_map_add_dblink( map(), field, NULL, fi->table, key.toLatin1().data(),
                             fi->database, fi->driver );

  if ( ret == -1 )
  {
    QgsDebugMsg( "Error: Cannot add dblink" );
    error = QString::fromLatin1( "Cannot create link to the table. The table was created!" );
  }

  return error;
}

QString QgsGrassProvider::addColumn( int field, const QString &column )
{
  QgsDebugMsg( QString( "field = %1" ).arg( field ) );

  // Read attributes
  struct field_info *fi = Vect_get_field( map(), field ); // should work also with field = 0
  if ( !fi )
  {
    QgsDebugMsg( "No field info" );
    return "Cannot get field info";
  }

  QString query;
  query.sprintf( "alter table %s add column %s", fi->table, column.toLatin1().constData() );
  return executeSql( field, query );
}

QString QgsGrassProvider::insertAttributes( int field, int cat )
{
  QgsDebugMsg( QString( "field = %1 cat = %2" ).arg( field ).arg( cat ) );

  // Read attributes
  struct field_info *fi = Vect_get_field( map(), field ); // should work also with field = 0
  if ( !fi )
  {
    QgsDebugMsg( "No field info -> no attributes" );
    return "Cannot get field info";
  }

  QString query;
  query.sprintf( "insert into %s ( %s ) values ( %d )", fi->table, fi->key, cat );
  return executeSql( field, query );
}

QString QgsGrassProvider::deleteAttribute( int field, int cat )
{
  QgsDebugMsg( QString( "field = %1 cat = %2" ).arg( field ).arg( cat ) );

  // Read attributes
  struct field_info *fi = Vect_get_field( map(), field ); // should work also with field = 0
  if ( !fi )
  {
    QgsDebugMsg( "No field info -> no attributes" );
    return "Cannot get field info";
  }

  QString query;
  query.sprintf( "delete from %s where %s = %d", fi->table, fi->key, cat );
  return executeSql( field, query );
}

QString QgsGrassProvider::isOrphan( int field, int cat, int &orphan )
{
  QgsDebugMsg( QString( "field = %1 cat = %2" ).arg( field ).arg( cat ) );

  // Check first if another line with such cat exists
  int fieldIndex = Vect_cidx_get_field_index( map(), field );
  if ( fieldIndex >= 0 )
  {
    int t, id;
    int ret = Vect_cidx_find_next( map(), fieldIndex, cat,
                                   GV_POINTS | GV_LINES | GV_FACE, 0, &t, &id );

    if ( ret >= 0 )
    {
      // Category exists
      orphan = false;
      return QString();
    }
  }

  // Check if attribute exists
  // Read attributes
  struct field_info *fi = Vect_get_field( map(), field ); // should work also with field = 0
  if ( !fi )
  {
    QgsDebugMsg( "No field info -> no attributes" );
    orphan = false;
    return QString();
  }

  QgsDebugMsg( "Field info found -> open database" );
  setMapset();
  dbDriver *driver = db_start_driver_open_database( fi->driver, fi->database );
  if ( !driver )
  {
    QgsDebugMsg( QString( "Cannot open database %1 by driver %2" ).arg( fi->database ).arg( fi->driver ) );
    return "Cannot open database";
  }

  QgsDebugMsg( "Database opened -> select record" );

  dbString dbstr;
  db_init_string( &dbstr );

  QString query;
  query.sprintf( "select %s from %s where %s = %d", fi->key, fi->table, fi->key, cat );
  db_set_string( &dbstr, query.toLatin1().data() );

  QgsDebugMsg( QString( "SQL: %1" ).arg( db_get_string( &dbstr ) ) );

  dbCursor cursor;
  if ( db_open_select_cursor( driver, &dbstr, &cursor, DB_SCROLL ) != DB_OK )
  {
    db_close_database_shutdown_driver( driver );
    return QString( "Cannot query database: %1" ).arg( query );
  }
  int nRecords = db_get_num_rows( &cursor );
  QgsDebugMsg( QString( "Number of records: %1" ).arg( nRecords ) );

  if ( nRecords > 0 )
    orphan = true;

  db_close_database_shutdown_driver( driver );
  db_free_string( &dbstr );

  return QString();
}

bool QgsGrassProvider::isTopoType() const
{
  return isTopoType( mLayerType );
}

bool QgsGrassProvider::isTopoType( int layerType )
{
  return layerType == TOPO_POINT || layerType == TOPO_LINE || layerType == TOPO_NODE;
}

void QgsGrassProvider::setTopoFields()
{
  mTopoFields.append( QgsField( "id", QVariant::Int ) );

  if ( mLayerType == TOPO_POINT )
  {
    mTopoFields.append( QgsField( "type", QVariant::String ) );
    mTopoFields.append( QgsField( "node", QVariant::Int ) );
  }
  else if ( mLayerType == TOPO_LINE )
  {
    mTopoFields.append( QgsField( "type", QVariant::String ) );
    mTopoFields.append( QgsField( "node1", QVariant::Int ) );
    mTopoFields.append( QgsField( "node2", QVariant::Int ) );
    mTopoFields.append( QgsField( "left", QVariant::Int ) );
    mTopoFields.append( QgsField( "right", QVariant::Int ) );
  }
  else if ( mLayerType == TOPO_NODE )
  {
    mTopoFields.append( QgsField( "lines", QVariant::String ) );
  }
}

void QgsGrassProvider::startEditing( QgsVectorLayer *vectorLayer )
{
  QgsDebugMsg( "uri = " +  dataSourceUri() );
  if ( !vectorLayer || !vectorLayer->editBuffer() )
  {
    QgsDebugMsg( "vector or buffer is null" );
    return;
  }
  if ( !isValid() || !isGrassEditable() )
  {
    QgsDebugMsg( "not valid or not editable" );
    return;
  }
  if ( mEditBuffer )
  {
    QgsDebugMsg( "already edited" );
    return;
  }

  mLayer->map()->startEdit();

  mEditBuffer = vectorLayer->editBuffer();
  connect( mEditBuffer, SIGNAL( geometryChanged( QgsFeatureId, QgsGeometry & ) ), SLOT( bufferGeometryChanged( QgsFeatureId, QgsGeometry & ) ) );
  connect( vectorLayer, SIGNAL( beforeCommitChanges() ), SLOT( onBeforeCommitChanges() ) );
  connect( vectorLayer, SIGNAL( editingStopped() ), SLOT( onEditingStopped() ) );

  connect( vectorLayer->undoStack(), SIGNAL( indexChanged( int ) ), this, SLOT( onUndoIndexChanged( int ) ) );

  QgsDebugMsg( "edit started" );
}

void QgsGrassProvider::bufferGeometryChanged( QgsFeatureId fid, QgsGeometry &geom )
{
  int oldLid = QgsGrassFeatureIterator::lidFormFid( fid );
  int realLine = oldLid;
  if ( mLayer->map()->newLids().contains( oldLid ) ) // if it was changed already
  {
    realLine = mLayer->map()->newLids().value( oldLid );
  }
  QgsDebugMsg( QString( "fid = %1 oldLid = %2 realLine = %3" ).arg( fid ).arg( oldLid ).arg( realLine ) );

  struct line_pnts *points = Vect_new_line_struct();
  struct line_cats *cats = Vect_new_cats_struct();

  int type = Vect_read_line( map(), points, cats, realLine );
  QgsDebugMsg( QString( "type = %1 n_points = %2" ).arg( type ).arg( points->n_points ) );

  // store only the first original geometry, changed geometries are stored in the buffer
  if ( !mLayer->map()->oldGeometries().contains( oldLid ) )
  {
    QgsAbstractGeometryV2 *geometry = mLayer->map()->lineGeometry( oldLid );
    mLayer->map()->oldGeometries().insert( oldLid, geometry );
  }

  if ( type == GV_POINT || type == GV_CENTROID )
  {
    QgsPoint point = geom.asPoint();
    points->x[0] = point.x();
    points->y[0] = point.y();
    QgsDebugMsg( QString( "x = %1 y = %2" ).arg( point.x() ).arg( point.y() ) );
  }
  else if ( type == GV_LINE || type == GV_BOUNDARY )
  {
    QgsPolyline polyline = geom.asPolyline();
    for ( int i = 0; i < points->n_points; i++ )
    {
      points->x[i] = polyline.value( i ).x();
      points->y[i] = polyline.value( i ).y();
    }
  }
  else
  {
    QgsDebugMsg( "unknown type" );
    return;
  }

  mLayer->map()->lockReadWrite();
  // Vect_rewrite_line may delete/write the line with a new id
  int newLid = Vect_rewrite_line( map(), realLine, type, points, cats );
  mLayer->map()->unlockReadWrite();

  QgsDebugMsg( QString( "oldLid = %1 newLine = %2" ).arg( oldLid ).arg( newLid ) );
  // oldLids are maping to the very first, original version (used by undo)
  int oldestLid = oldLid;
  if ( mLayer->map()->oldLids().contains( oldLid ) )
  {
    oldestLid = mLayer->map()->oldLids().value( oldLid );
  }
  mLayer->map()->oldLids()[newLid] = oldestLid;
  mLayer->map()->newLids()[oldLid] = newLid;
}

void QgsGrassProvider::onUndoIndexChanged( int index )
{
  QgsDebugMsg( QString( "index = %1" ).arg( index ) );
}

void QgsGrassProvider::onBeforeCommitChanges()
{
  QgsDebugMsg( "entered" );
}

void QgsGrassProvider::onEditingStopped()
{
  QgsDebugMsg( "entered" );
  closeEdit( false );
}

// -------------------------------------------------------------------------------

int QgsGrassProvider::cidxGetNumFields()
{
  return ( Vect_cidx_get_num_fields( map() ) );
}

int QgsGrassProvider::cidxGetFieldNumber( int idx )
{
  return ( Vect_cidx_get_field_number( map(), idx ) );
}

int QgsGrassProvider::cidxGetMaxCat( int idx )
{
  int ncats = Vect_cidx_get_num_cats_by_index( map(), idx );

  int cat, type, id;
  Vect_cidx_get_cat_by_index( map(), idx, ncats - 1, &cat, &type, &id );

  return ( cat );
}

QgsGrassVectorMapLayer * QgsGrassProvider::openLayer() const
{
  return mLayer->map()->openLayer( mLayerField );
}

struct Map_info * QgsGrassProvider::map()
{
  Q_ASSERT( mLayer );
  Q_ASSERT( mLayer->map() );
  Q_ASSERT( mLayer->map()->map() );
  return mLayer->map()->map();
}

void QgsGrassProvider::setMapset()
{
  QgsGrass::setMapset( mGrassObject.gisdbase(), mGrassObject.location(), mGrassObject.mapset() );
}

QString QgsGrassProvider::name() const
{
  return GRASS_KEY;
}

QString QgsGrassProvider::description() const
{
  return tr( "GRASS %1 vector provider" ).arg( GRASS_VERSION_MAJOR );
}

