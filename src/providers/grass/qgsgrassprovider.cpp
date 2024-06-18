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
#include <QElapsedTimer>

#include "qgis.h"
#include "qgsdataprovider.h"
#include "qgsfeature.h"
#include "qgsfields.h"
#include "qgslinestring.h"
#include "qgspoint.h"
#include "qgspolygon.h"
#include "qgsrectangle.h"
#include "qgsvectorlayer.h"
#include "qgsvectorlayereditbuffer.h"
#include "qgsgrass.h"
#include "qgsgrassprovider.h"
#include "qgsgrassfeatureiterator.h"
#include "qgsgrassundocommand.h"
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
#if defined(_MSC_VER) && defined(M_PI_4)
#undef M_PI_4 //avoid redefinition warning
#endif
#include <grass/gprojects.h>
#include <grass/gis.h>
#include <grass/dbmi.h>
#include <grass/vector.h>
}

#ifdef _MSC_VER
#undef __STDC__
#endif

// Vect_rewrite_line and Vect_delete_line use int in GRASS 6 and off_t in GRASS 7 for line id argument.
// off_t size is not specified by C, POSIX specifies it as signed integer and its size depends on compiler.
// In OSGeo4W 32bit the off_t size is 8 bytes in GRASS (32bit, compiled with MinGW, 'g.version -g' prints build_off_t_size=8 )
// and 4 bytes QGIS (32bit, compiled with MSVC), which is causing crashes.
// The problem with off_t size was also reported for custom build of QGIS on Xubuntu 14.04 LTS using GRASS 7.0.1-2~ubuntu14.04.1.
// See: https://lists.osgeo.org/pipermail/grass-dev/2015-June/075539.html
//      https://lists.osgeo.org/pipermail/grass-dev/2015-September/076338.html
// TODO: get real off_t size from GRASS, probably contribute a patch which will save 'g.version -g' to a header file during compilation
#ifdef Q_OS_WIN
typedef qint64 grass_off_t;
#else
#if defined(GRASS_OFF_T_SIZE) && GRASS_OFF_T_SIZE == 4
typedef qint32 grass_off_t;
#elif defined(GRASS_OFF_T_SIZE) && GRASS_OFF_T_SIZE == 8
typedef qint64 grass_off_t;
#else
typedef off_t grass_off_t; // GRASS_OFF_T_SIZE undefined, default to off_t
#endif
#endif
typedef grass_off_t Vect_rewrite_line_function_type( struct Map_info *, grass_off_t, int, const struct line_pnts *, const struct line_cats * );
typedef int Vect_delete_line_function_type( struct Map_info *, grass_off_t );
Vect_rewrite_line_function_type *Vect_rewrite_line_function_pointer = ( Vect_rewrite_line_function_type * )Vect_rewrite_line;
Vect_delete_line_function_type *Vect_delete_line_function_pointer = ( Vect_delete_line_function_type * )Vect_delete_line;

static QString GRASS_KEY = QStringLiteral( "grass" );

int QgsGrassProvider::sLastType = -9999;
int QgsGrassProvider::sEditedCount = 0;

QgsGrassProvider::QgsGrassProvider( const QString &uri )
  : QgsVectorDataProvider( uri )
{
  QgsDebugMsgLevel( "uri = " + uri, 2 );

  mValid = false;
  if ( !QgsGrass::init() )
  {
    appendError( QgsGrass::initError() );
    return;
  }

  QElapsedTimer time;
  time.start();

  mPoints = Vect_new_line_struct();
  mCats = Vect_new_cats_struct();

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
  QgsDebugMsgLevel( "mGrassObject = " + mGrassObject.toString() + " mLayerName = " + mLayerName, 2 );

  /* Parse Layer, supported layers <field>_point, <field>_line, <field>_area
  *  Layer is opened even if it is empty (has no features)
  */
  if ( mLayerName.compare( QLatin1String( "boundary" ) ) == 0 ) // currently not used
  {
    mLayerType = Boundary;
    mGrassType = GV_BOUNDARY;
  }
  else if ( mLayerName.compare( QLatin1String( "centroid" ) ) == 0 ) // currently not used
  {
    mLayerType = Centroid;
    mGrassType = GV_CENTROID;
  }
  else if ( mLayerName == QLatin1String( "topo_point" ) )
  {
    mLayerType = TopoPoint;
    mGrassType = GV_POINTS;
  }
  else if ( mLayerName == QLatin1String( "topo_line" ) )
  {
    mLayerType = TopoLine;
    mGrassType = GV_LINES;
  }
  else if ( mLayerName == QLatin1String( "topo_node" ) )
  {
    mLayerType = TopoNode;
    mGrassType = 0;
  }
  else
  {
    mLayerField = grassLayer( mLayerName );
    if ( mLayerField == -1 )
    {
      QgsDebugError( QString( "Invalid layer name, no underscore found: %1" ).arg( mLayerName ) );
      return;
    }

    mGrassType = grassLayerType( mLayerName );

    if ( mGrassType == GV_POINT )
    {
      mLayerType = Point;
    }
    else if ( mGrassType == GV_LINES )
    {
      mLayerType = Line;
    }
    else if ( mGrassType == GV_FACE )
    {
      mLayerType = Face;
    }
    else if ( mGrassType == GV_AREA )
    {
      mLayerType = Polygon;
    }
    else
    {
      QgsDebugError( QString( "Invalid layer name, wrong type: %1" ).arg( mLayerName ) );
      return;
    }

  }
  QgsDebugMsgLevel( QString( "mLayerField: %1" ).arg( mLayerField ), 2 );
  QgsDebugMsgLevel( QString( "mLayerType: %1" ).arg( mLayerType ), 2 );

  if ( mLayerType == Boundary || mLayerType == Centroid )
  {
    QgsDebugError( "Layer type not supported." );
    return;
  }

  // Set QGIS type
  switch ( mLayerType )
  {
    case Point:
    case Centroid:
    case TopoPoint:
    case TopoNode:
      mQgisType = Qgis::WkbType::Point;
      break;
    case Line:
    case Boundary:
    case TopoLine:
      mQgisType = Qgis::WkbType::LineString;
      break;
    case Polygon:
    case Face:
      mQgisType = Qgis::WkbType::Polygon;
      break;
  }

  if ( !openLayer() )
  {
    QgsDebugError( "Cannot open layer" );
    return;
  }

  loadMapInfo();
  setTopoFields();

  connect( mLayer->map(),
           &QgsGrassVectorMap::dataChanged, this, &QgsGrassProvider::onDataChanged );

  // TODO: types according to database
  setNativeTypes( QList<NativeType>()
                  << QgsVectorDataProvider::NativeType( tr( "Whole number (integer)" ), QStringLiteral( "integer" ), QMetaType::Type::Int, -1, -1, -1, -1 )
                  << QgsVectorDataProvider::NativeType( tr( "Decimal number (real)" ), QStringLiteral( "double precision" ), QMetaType::Type::Double, -1, -1, -1, -1 )
                  << QgsVectorDataProvider::NativeType( tr( "Text" ), QStringLiteral( "text" ), QMetaType::Type::QString )
                  // TODO:
                  // << QgsVectorDataProvider::NativeType( tr( "Date" ), "date", QVariant::Date, 8, 8 );
                );

  // Assign default encoding
  if ( !textEncoding() )
    QgsVectorDataProvider::setEncoding( QStringLiteral( "UTF-8" ) );

  mValid = true;

  QgsDebugMsgLevel( QString( "New GRASS layer opened, time (ms): %1" ).arg( time.elapsed() ), 2 );
}

QgsGrassProvider::~QgsGrassProvider()
{
  if ( mLayer )
  {
    mLayer->close();
  }
  if ( mPoints )
  {
    Vect_destroy_line_struct( mPoints );
  }
  if ( mCats )
  {
    Vect_destroy_cats_struct( mCats );
  }
}

QgsVectorDataProvider::Capabilities QgsGrassProvider::capabilities() const
{
  // Because of bug in GRASS https://trac.osgeo.org/grass/ticket/2775 it is not possible
  // close db drivers in random order on Unix and probably Mac -> disable editing if another layer is edited
#ifndef Q_OS_WIN
  if ( sEditedCount > 0 && !mEditBuffer )
  {
    return QgsVectorDataProvider::Capabilities();
  }
#endif
  // for now, only one map may be edited at time
  if ( mEditBuffer || ( mLayer && mLayer->map() && !mLayer->map()->isEdited() ) )
  {
    return AddFeatures | DeleteFeatures | ChangeGeometries | AddAttributes | DeleteAttributes | ChangeAttributeValues;
  }
  return QgsVectorDataProvider::Capabilities();
}

bool QgsGrassProvider::openLayer()
{
  // the map may be invalid (e.g. wrong uri or open failed)
  QgsGrassVectorMap *vectorMap = QgsGrassVectorMapStore::instance()->openMap( mGrassObject );
  if ( !vectorMap ) // should not happen
  {
    QgsDebugError( "Cannot open map" );
    return false;
  }
  if ( !vectorMap->isValid() ) // may happen
  {
    QgsDebugError( "vectorMap is not valid" );
    return false;
  }

  mLayer = vectorMap->openLayer( mLayerField );

  if ( !mLayer ) // should not happen
  {
    QgsDebugError( "Cannot open layer" );
    return false;
  }
  if ( !mLayer->map() || !mLayer->map()->map() ) // should not happen
  {
    QgsDebugError( "map is null" );
    return false;
  }
  mMapVersion = mLayer->map()->version();
  return true;
}

void QgsGrassProvider::loadMapInfo()
{
  // Getting the total number of features in the layer
  int cidxFieldIndex = -1;
  mNumberFeatures = 0;
  if ( mLayerType == TopoPoint )
  {
    mNumberFeatures = Vect_get_num_primitives( map(), GV_POINTS );
  }
  else if ( mLayerType == TopoLine )
  {
    mNumberFeatures = Vect_get_num_primitives( map(), GV_LINES );
  }
  else if ( mLayerType == TopoNode )
  {
    mNumberFeatures = Vect_get_num_nodes( map() );
  }
  else
  {
    if ( mLayerField >= 0 )
    {
      cidxFieldIndex = Vect_cidx_get_field_index( map(), mLayerField );
      if ( cidxFieldIndex >= 0 )
      {
        mNumberFeatures = Vect_cidx_get_type_count( map(), mLayerField, mGrassType );
      }
    }
    else
    {
      // TODO nofield layers
      mNumberFeatures = 0;
    }
  }
  QgsDebugMsgLevel( QString( "mNumberFeatures = %1 cidxFieldIndex = %2" ).arg( mNumberFeatures ).arg( cidxFieldIndex ), 2 );
}

void QgsGrassProvider::update()
{

  mValid = false;

  if ( mLayer )
  {
    mLayer->close();
    mLayer = nullptr;
  }

  if ( !openLayer() )
  {
    QgsDebugError( "Cannot open layer" );
    return;
  }

  loadMapInfo();

  mValid = true;
}

QgsAbstractFeatureSource *QgsGrassProvider::featureSource() const
{
  const_cast<QgsGrassProvider *>( this )->ensureUpdated();
  return new QgsGrassFeatureSource( this );
}


QString QgsGrassProvider::storageType() const
{
  return QStringLiteral( "GRASS (Geographic Resources Analysis and Support System) file" );
}



QgsFeatureIterator QgsGrassProvider::getFeatures( const QgsFeatureRequest &request ) const
{
  if ( !mValid )
  {
    return QgsFeatureIterator();
  }

  // check if outdated and update if necessary
  ensureUpdated();

  QgsGrassFeatureSource *source = new QgsGrassFeatureSource( this );
  QgsGrassFeatureIterator *iterator = new QgsGrassFeatureIterator( source, true, request );
  return QgsFeatureIterator( iterator );
}

QgsRectangle QgsGrassProvider::extent() const
{
  if ( isValid() )
  {
    bound_box box;
    Vect_get_map_box( map(), &box );
    return QgsRectangle( box.W, box.S, box.E, box.N );
  }
  return QgsRectangle();
}

/**
* Return the feature type
*/
Qgis::WkbType QgsGrassProvider::wkbType() const
{
  return mQgisType;
}

long long QgsGrassProvider::featureCount() const
{
  return mNumberFeatures;
}

QgsFields QgsGrassProvider::fields() const
{
  if ( isTopoType() )
  {
    return mTopoFields;
  }
  else if ( mLayer )
  {
    // Original fields must be returned during editing because edit buffer updates fields by indices
    if ( mEditBuffer )
    {
      return mLayer->fields();
    }
    else
    {
      return mLayer->tableFields();
    }
  }
  return QgsFields();
}

int QgsGrassProvider::keyField()
{
  return mLayer ? mLayer->keyColumn() : -1;
}

QVariant QgsGrassProvider::minimumValue( int index ) const
{
  if ( isValid() )
  {
    return mLayer->minMax().value( index ).first;
  }
  return QVariant();
}


QVariant QgsGrassProvider::maxValue( int index )
{
  if ( isValid() )
  {
    return mLayer->minMax().value( index ).second;
  }
  return QVariant();
}

bool QgsGrassProvider::isValid() const
{
  bool valid = mValid && mLayer && mLayer->map() && mLayer->map()->map();
  QgsDebugMsgLevel( QString( "valid = %1" ).arg( valid ), 2 );
  return valid;
}

QgsCoordinateReferenceSystem QgsGrassProvider::crs() const
{
  QString error;
  return QgsGrass::crs( mGrassObject.gisdbase(), mGrassObject.location(), error );
}

int QgsGrassProvider::grassLayer()
{
  return mLayerField;
}

int QgsGrassProvider::grassLayer( const QString &name )
{
  // Get field number
  int pos = name.indexOf( '_' );

  if ( pos == -1 )
  {
    return -1;
  }

  return name.left( pos ).toInt();
}

int QgsGrassProvider::grassLayerType( const QString &name )
{
  int pos = name.indexOf( '_' );

  if ( pos == -1 )
  {
    return -1;
  }

  QString ts = name.right( name.length() - pos - 1 );
  if ( ts.compare( QLatin1String( "point" ) ) == 0 )
  {
    return GV_POINT; // ?! centroids may be points
  }
  else if ( ts.compare( QLatin1String( "line" ) ) == 0 )
  {
    return GV_LINES;
  }
  else if ( ts.compare( QLatin1String( "face" ) ) == 0 )
  {
    return GV_FACE;
  }
  else if ( ts.compare( QLatin1String( "polygon" ) ) == 0 )
  {
    return GV_AREA;
  }

  return -1;
}

void QgsGrassProvider::onDataChanged()
{
  update();
  emit dataChanged();
}

//-----------------------------------------  Edit -------------------------------------------------------

bool QgsGrassProvider::isGrassEditable( void )
{

  if ( !isValid() )
    return false;

  /* Check if current user is owner of mapset */
  if ( G_mapset_permissions2( mGrassObject.gisdbase().toUtf8().constData(), mGrassObject.location().toUtf8().constData(), mGrassObject.mapset().toUtf8().constData() ) != 1 )
    return false;

  // TODO: check format? (cannot edit OGR layers)

  return true;
}

bool QgsGrassProvider::isEdited( void )
{
  QgsDebugMsgLevel( "entered", 3 );
  return ( mEditBuffer );
}

void QgsGrassProvider::freeze()
{

  if ( !isValid() )
  {
    return;
  }

  mValid = false;

  if ( mLayer )
  {
    mLayer->close();
    mLayer->map()->close(); // closes all iterators, blocking
    mLayer = nullptr;
  }
}

void QgsGrassProvider::thaw()
{

  if ( !openLayer() )
  {
    QgsDebugError( "Cannot open layer" );
    return;
  }

  loadMapInfo();

  mValid = true;
}

bool QgsGrassProvider::closeEdit( bool newMap, QgsVectorLayer *vectorLayer )
{

  if ( !isValid() )
  {
    QgsDebugError( "not valid" );
    return false;
  }

  mEditBuffer = nullptr;
  mEditLayer = nullptr;
  // drivers must be closed in reversed order in which were opened
  // TODO: close driver order for simultaneous editing of multiple vector maps
  for ( int i = mOtherEditLayers.size() - 1; i >= 0; i-- )
  {
    QgsGrassVectorMapLayer *layer = mOtherEditLayers[i];
    layer->closeEdit();
    mLayer->map()->closeLayer( layer );
  }
  mOtherEditLayers.clear();
  mLayer->closeEdit();
  if ( mLayer->map()->closeEdit( newMap ) )
  {
    loadMapInfo();
    if ( vectorLayer )
    {
      vectorLayer->updateFields();
    }
    connect( mLayer->map(), &QgsGrassVectorMap::dataChanged, this, &QgsGrassProvider::onDataChanged );
    emit fullExtentCalculated();
    sEditedCount--;
    return true;
  }
  return false;
}

void QgsGrassProvider::ensureUpdated() const
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
  if ( !isValid() )
    return -1;

  //return ( Vect_get_num_lines( map() ) );
  return mLayer->map()->numLines();
}

int QgsGrassProvider::numNodes( void )
{
  if ( !isValid() )
    return -1;

  return ( Vect_get_num_nodes( map() ) );
}

int QgsGrassProvider::readLine( struct line_pnts *Points, struct line_cats *Cats, int line )
{
  QgsDebugMsgLevel( "entered", 3 );

  if ( Points )
  {
    Vect_reset_line( Points );
  }

  if ( Cats )
  {
    Vect_reset_cats( Cats );
  }

  if ( !map() )
  {
    return -1;
  }

  if ( !Vect_line_alive( map(), line ) )
  {
    return -1;
  }

  int type = -1;
  G_TRY
  {
    type = Vect_read_line( map(), mPoints, mCats, line );
  }
  G_CATCH( QgsGrass::Exception & e )
  {
    QgsDebugError( QString( "Cannot read line : %1" ).arg( e.what() ) );
  }
  return type;
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

  Vect_get_node_coor( map(), node, x, y, nullptr );
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

  /* points don't have topology in GRASS >= 7 */
  *node1 = 0;
  *node2 = 0;
  return true;
}

int QgsGrassProvider::writeLine( int type, struct line_pnts *Points, struct line_cats *Cats )
{
  QgsDebugMsgLevel( QString( "n_points = %1 n_cats = %2" ).arg( Points->n_points ).arg( Cats->n_cats ), 2 );

  if ( !isEdited() )
    return -1;

  return ( ( int ) Vect_write_line( map(), type, Points, Cats ) );
}

int QgsGrassProvider::rewriteLine( int oldLid, int type, struct line_pnts *Points, struct line_cats *Cats )
{
  QgsDebugMsgLevel( QString( "n_points = %1 n_cats = %2" ).arg( Points->n_points ).arg( Cats->n_cats ), 2 );

  if ( !isValid() || !map() || !isEdited() )
  {
    return -1;
  }

  int newLid = -1;
  G_TRY
  {
    newLid = Vect_rewrite_line_function_pointer( map(), oldLid, type, Points, Cats );

    // oldLids are mapping to the very first, original version (used by undo)
    int oldestLid = oldLid;
    if ( mLayer->map()->oldLids().contains( oldLid ) ) // if it was changed already
    {
      oldestLid = mLayer->map()->oldLids().value( oldLid );
    }

    QgsDebugMsgLevel( QString( "oldLid = %1 oldestLid = %2 newLine = %3 numLines = %4" )
                      .arg( oldLid ).arg( oldestLid ).arg( newLid ).arg( mLayer->map()->numLines() ), 2 );
    QgsDebugMsgLevel( QString( "oldLids : %1 -> %2" ).arg( newLid ).arg( oldestLid ), 2 );
    mLayer->map()->oldLids()[newLid] = oldestLid;
    QgsDebugMsgLevel( QString( "newLids : %1 -> %2" ).arg( oldestLid ).arg( newLid ), 2 );
    mLayer->map()->newLids()[oldestLid] = newLid;
  }
  G_CATCH( QgsGrass::Exception & e )
  {
    QgsDebugError( QString( "Cannot write line : %1" ).arg( e.what() ) );
  }
  return newLid;
}


int QgsGrassProvider::deleteLine( int line )
{

  if ( !isEdited() )
    return -1;

  return ( Vect_delete_line_function_pointer( map(), line ) );
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
  QgsDebugMsgLevel( QString( "numUpdatedLines = %1" ).arg( Vect_get_num_updated_lines( map() ) ), 2 );

  return ( Vect_get_num_updated_lines( map() ) );
}

int QgsGrassProvider::numUpdatedNodes( void )
{
  QgsDebugMsgLevel( QString( "numUpdatedNodes = %1" ).arg( Vect_get_num_updated_nodes( map() ) ), 2 );

  return ( Vect_get_num_updated_nodes( map() ) );
}

int QgsGrassProvider::updatedLine( int idx )
{
  QgsDebugMsgLevel( QString( "idx = %1" ).arg( idx ), 2 );
  QgsDebugMsgLevel( QString( "  updatedLine = %1" ).arg( Vect_get_updated_line( map(), idx ) ), 2 );

  return ( Vect_get_updated_line( map(), idx ) );
}

int QgsGrassProvider::updatedNode( int idx )
{
  QgsDebugMsgLevel( QString( "idx = %1" ).arg( idx ), 2 );
  QgsDebugMsgLevel( QString( "  updatedNode = %1" ).arg( Vect_get_updated_node( map(), idx ) ), 2 );

  return ( Vect_get_updated_node( map(), idx ) );
}

// ------------------ Attributes -------------------------------------------------

QString QgsGrassProvider::key( int field )
{
  QgsDebugMsgLevel( QString( "field = %1" ).arg( field ), 2 );

  struct field_info *fi = Vect_get_field( map(), field ); // should work also with field = 0
  if ( !fi )
  {
    QgsDebugError( "No field info -> no attributes" );
    return QString();
  }

  return QString::fromUtf8( fi->key );
}

QgsAttributeMap *QgsGrassProvider::attributes( int field, int cat )
{
  QgsDebugMsgLevel( QString( "field = %1 cat = %2" ).arg( field ).arg( cat ), 2 );

  QgsAttributeMap *att = new QgsAttributeMap;

  struct  field_info *fi = Vect_get_field( map(), field ); // should work also with field = 0

  // Read attributes
  if ( !fi )
  {
    QgsDebugError( "No field info -> no attributes" );
    return att;
  }

  QgsDebugMsgLevel( "Field info found -> open database", 2 );
  setMapset();
  dbDriver *driver = db_start_driver_open_database( fi->driver, fi->database );

  if ( !driver )
  {
    QgsDebugError( QString( "Cannot open database %1 by driver %2" ).arg( fi->database, fi->driver ) );
    return att;
  }

  QgsDebugMsgLevel( "Database opened -> read attributes", 2 );

  dbString dbstr;
  db_init_string( &dbstr );
  QString query = QStringLiteral( "select * from %1 where %2=%3" ).arg( fi->table, fi->key ).arg( cat );
  db_set_string( &dbstr, query.toUtf8().constData() );

  QgsDebugMsgLevel( QString( "SQL: %1" ).arg( db_get_string( &dbstr ) ), 2 );

  dbCursor databaseCursor;
  if ( db_open_select_cursor( driver, &dbstr, &databaseCursor, DB_SCROLL ) != DB_OK )
  {
    db_close_database_shutdown_driver( driver );
    QgsDebugError( "Cannot select attributes from table" );
    return att;
  }

  int nRecords = db_get_num_rows( &databaseCursor );
  QgsDebugMsgLevel( QString( "Number of records: %1" ).arg( nRecords ), 2 );

  if ( nRecords < 1 )
  {
    db_close_database_shutdown_driver( driver );
    QgsDebugError( "No DB record" );
    return att;
  }

  dbTable  *databaseTable = db_get_cursor_table( &databaseCursor );
  int nColumns = db_get_table_number_of_columns( databaseTable );

  int more;
  if ( db_fetch( &databaseCursor, DB_NEXT, &more ) != DB_OK )
  {
    db_close_database_shutdown_driver( driver );
    QgsDebugError( "Cannot fetch DB record" );
    return att;
  }

  // Read columns' description
  for ( int i = 0; i < nColumns; i++ )
  {
    dbColumn *column = db_get_table_column( databaseTable, i );
    db_convert_column_value_to_string( column, &dbstr );

    QString v = textEncoding()->toUnicode( db_get_string( &dbstr ) );
    QgsDebugMsgLevel( QString( "Value: %1" ).arg( v ), 2 );
    att->insert( i, QVariant( v ) );
  }

  db_close_cursor( &databaseCursor );
  db_close_database_shutdown_driver( driver );
  db_free_string( &dbstr );

  return att;
}



int QgsGrassProvider::numDbLinks( void )
{

  return ( Vect_get_num_dblinks( map() ) );
}

int QgsGrassProvider::dbLinkField( int link )
{

  struct  field_info *fi = Vect_get_dblink( map(), link );

  if ( !fi )
    return 0;

  return ( fi->number );
}

bool QgsGrassProvider::isTopoType() const
{
  return isTopoType( mLayerType );
}

bool QgsGrassProvider::isTopoType( int layerType )
{
  return layerType == TopoPoint || layerType == TopoLine || layerType == TopoNode;
}

void QgsGrassProvider::setTopoFields()
{
  mTopoFields.append( QgsField( QStringLiteral( "id" ), QMetaType::Type::Int ) );

  if ( mLayerType == TopoPoint )
  {
    mTopoFields.append( QgsField( QStringLiteral( "type" ), QMetaType::Type::QString ) );
    mTopoFields.append( QgsField( QStringLiteral( "node" ), QMetaType::Type::Int ) );
  }
  else if ( mLayerType == TopoLine )
  {
    mTopoFields.append( QgsField( QStringLiteral( "type" ), QMetaType::Type::QString ) );
    mTopoFields.append( QgsField( QStringLiteral( "node1" ), QMetaType::Type::Int ) );
    mTopoFields.append( QgsField( QStringLiteral( "node2" ), QMetaType::Type::Int ) );
    mTopoFields.append( QgsField( QStringLiteral( "left" ), QMetaType::Type::Int ) );
    mTopoFields.append( QgsField( QStringLiteral( "right" ), QMetaType::Type::Int ) );
  }
  else if ( mLayerType == TopoNode )
  {
    mTopoFields.append( QgsField( QStringLiteral( "lines" ), QMetaType::Type::QString ) );
  }
}

void QgsGrassProvider::startEditing( QgsVectorLayer *vectorLayer )
{
  QgsDebugMsgLevel( "uri = " +  dataSourceUri(), 2 );
  if ( !vectorLayer || !vectorLayer->editBuffer() )
  {
    QgsDebugError( "vector or buffer is null" );
    return;
  }
  mEditLayer = vectorLayer;
  if ( !isValid() || !isGrassEditable() )
  {
    QgsDebugError( "not valid or not editable" );
    return;
  }
  if ( mEditBuffer )
  {
    QgsDebugError( "already edited" );
    return;
  }

  // disconnect dataChanged() because the changes are done here and we know about them
  disconnect( mLayer->map(), &QgsGrassVectorMap::dataChanged, this, &QgsGrassProvider::onDataChanged );
  mLayer->map()->startEdit();
  mLayer->startEdit();

  mEditBuffer = vectorLayer->editBuffer();
  connect( mEditBuffer, &QgsVectorLayerEditBuffer::featureAdded, this, &QgsGrassProvider::onFeatureAdded );
  connect( mEditBuffer, &QgsVectorLayerEditBuffer::featureDeleted, this, &QgsGrassProvider::onFeatureDeleted );
  connect( mEditBuffer, &QgsVectorLayerEditBuffer::geometryChanged, this, &QgsGrassProvider::onGeometryChanged );
  connect( mEditBuffer, &QgsVectorLayerEditBuffer::attributeValueChanged, this, &QgsGrassProvider::onAttributeValueChanged );
  connect( mEditBuffer, &QgsVectorLayerEditBuffer::attributeAdded, this, &QgsGrassProvider::onAttributeAdded );
  connect( mEditBuffer, &QgsVectorLayerEditBuffer::attributeDeleted, this, &QgsGrassProvider::onAttributeDeleted );
  connect( vectorLayer, &QgsVectorLayer::beforeCommitChanges, this, &QgsGrassProvider::onBeforeCommitChanges );
  connect( vectorLayer, &QgsVectorLayer::beforeRollBack, this, &QgsGrassProvider::onBeforeRollBack );
  connect( vectorLayer, &QgsVectorLayer::editingStopped, this, &QgsGrassProvider::onEditingStopped );

  connect( vectorLayer->undoStack(), &QUndoStack::indexChanged, this, &QgsGrassProvider::onUndoIndexChanged );

  // let qgis know (attribute table etc.) that we added topo symbol field
  vectorLayer->updateFields();
  mEditLayerFields = vectorLayer->fields();

  // TODO: enable cats editing once all consequences are implemented
  // disable cat and topo symbol editing
  QgsEditFormConfig formConfig = vectorLayer->editFormConfig();
  formConfig.setReadOnly( mLayer->keyColumn(), true );
  formConfig.setReadOnly( mLayer->fields().size() - 1, true );
  vectorLayer->setEditFormConfig( formConfig );

  sEditedCount++;

  QgsDebugMsgLevel( "edit started", 2 );
}

void QgsGrassProvider::setPoints( struct line_pnts *points, const QgsAbstractGeometry *geometry )
{
  if ( !points )
  {
    return;
  }
  Vect_reset_line( points );
  if ( !geometry )
  {
    return;
  }
  if ( geometry->wkbType() == Qgis::WkbType::Point || geometry->wkbType() == Qgis::WkbType::PointZ )
  {
    const QgsPoint *point = dynamic_cast<const QgsPoint *>( geometry );
    if ( point )
    {
      Vect_append_point( points, point->x(), point->y(), point->z() );
      QgsDebugMsgLevel( QString( "x = %1 y = %2" ).arg( point->x() ).arg( point->y() ), 2 );
    }
  }
  else if ( geometry->wkbType() == Qgis::WkbType::LineString || geometry->wkbType() == Qgis::WkbType::LineStringZ )
  {
    const QgsLineString *lineString = dynamic_cast<const QgsLineString *>( geometry );
    if ( lineString )
    {
      for ( int i = 0; i < lineString->numPoints(); i++ )
      {
        QgsPoint point = lineString->pointN( i );
        Vect_append_point( points, point.x(), point.y(), point.z() );
      }
    }
  }
  else if ( geometry->wkbType() == Qgis::WkbType::Polygon || geometry->wkbType() == Qgis::WkbType::PolygonZ )
  {
    const QgsPolygon *polygon = dynamic_cast<const QgsPolygon *>( geometry );
    if ( polygon && polygon->exteriorRing() )
    {
      QgsLineString *lineString = polygon->exteriorRing()->curveToLine();
      if ( lineString )
      {
        for ( int i = 0; i < lineString->numPoints(); i++ )
        {
          QgsPoint point = lineString->pointN( i );
          Vect_append_point( points, point.x(), point.y(), point.z() );
        }
      }
    }
  }
  else
  {
    QgsDebugError( "unknown type : " + geometry->geometryType() );
  }
}

void QgsGrassProvider::onFeatureAdded( QgsFeatureId fid )
{
  if ( !mLayer )
    return;

  // fid is negative for new features

  int lid = QgsGrassFeatureIterator::lidFromFid( fid );
  int cat = QgsGrassFeatureIterator::catFromFid( fid );

  QgsDebugMsgLevel( QString( "fid = %1 lid = %2 cat = %3" ).arg( fid ).arg( lid ).arg( cat ), 2 );

  Vect_reset_cats( mCats );
  int realLine = 0; // number of line to be rewritten if exists
  int newCat = 0;

  // TODO: get also old type if it is feature previously deleted
  int type = 0;

  if ( FID_IS_NEW( fid ) )
  {
    if ( mNewFeatureType == QgsGrassProvider::sLastType )
    {
      type = mLastType;
      QgsDebugMsgLevel( QString( "use mLastType = %1" ).arg( mLastType ), 2 );
    }
    else
    {
      type = mNewFeatureType == GV_AREA ? GV_BOUNDARY : mNewFeatureType;
    }
    // geometry
    const QgsAbstractGeometry *geometry = nullptr;
    if ( !mEditBuffer->isFeatureAdded( fid ) )
    {
#ifdef QGISDEBUG
      QgsDebugMsgLevel( "the feature is missing in buffer addedFeatures :", 2 );

      const auto constKeys = mEditBuffer->addedFeatures().keys();
      for ( QgsFeatureId id : constKeys )
      {
        QgsDebugMsgLevel( QString( "addedFeatures : id = %1" ).arg( id ), 2 );
      }
#endif
      return;
    }
    QgsFeature feature = mEditBuffer->addedFeatures().value( fid );
    QgsGeometry featureGeometry = feature.geometry();
    geometry = featureGeometry.constGet();
    if ( !geometry )
    {
      QgsDebugMsgLevel( "geometry is null", 2 );
      return;
    }

    setPoints( mPoints, geometry );

    QgsFeatureMap &addedFeatures = mEditBuffer->mAddedFeatures;

    // change polygon to linestring
    Qgis::WkbType wkbType = QgsWkbTypes::flatType( geometry->wkbType() );
    if ( wkbType == Qgis::WkbType::Polygon )
    {
      QgsGeometry addedFeatureGeom = addedFeatures[fid].geometry();
      const QgsPolygon *polygon = dynamic_cast<const QgsPolygon *>( addedFeatureGeom.constGet() );
      if ( polygon )
      {
        QgsLineString *lineString = polygon->exteriorRing()->curveToLine();
        addedFeatures[fid].setGeometry( QgsGeometry( lineString ) );
      }
      // TODO: create also centroid and add it to undo
    }

    // add new category
    // TODO: add category also to boundary if user defined at least one attribute?
    if ( type != GV_BOUNDARY )
    {
      // TODO: redo of deleted new features - save new cats somewhere,
      // resetting fid probably is not possible because it is stored in undo commands and used in buffer maps

      // It may be that user manually entered cat value
      QgsFeatureMap &addedFeatures = mEditBuffer->mAddedFeatures;
      QgsFeature &feature = addedFeatures[fid];
      int catIndex = feature.fields().indexFromName( mLayer->keyColumnName() );
      if ( catIndex != -1 )
      {
        QVariant userCatVariant = feature.attributes().value( catIndex );
        if ( !userCatVariant.isNull() )
        {
          newCat = userCatVariant.toInt();
          QgsDebugMsgLevel( QString( "user defined newCat = %1" ).arg( newCat ), 2 );
        }
      }
      if ( newCat == 0 )
      {
        newCat = getNewCat();
      }
      QgsDebugMsgLevel( QString( "newCat = %1" ).arg( newCat ), 2 );
      Vect_cat_set( mCats, mLayerField, newCat );

      if ( mLayer->hasTable() )
      {
        addedFeatures[fid].setAttribute( mLayer->keyColumn(), newCat );
      }
      else
      {
        addedFeatures[fid].setAttribute( 0, newCat );
      }
      mLayer->map()->newCats()[fid] = newCat;
      QgsDebugMsgLevel( QString( "newCats[%1] = %2" ).arg( fid ).arg( newCat ), 2 );

      // Currently neither entering new cat nor changing existing cat is allowed
#if 0
      // There may be other new features with the same cat which we have to update
      const auto constKeys = addedFeatures.keys();
      for ( QgsFeatureId addedFid : constKeys )
      {
        if ( addedFid == fid )
        {
          continue;
        }
        int addedCat = mLayer->map()->newCats().value( addedFid ); // it should always exist
        QgsDebugMsgLevel( QString( "addedFid = %1 addedCat = %2" ).arg( addedFid ).arg( addedCat ), 2 );
        if ( addedCat == newCat )
        {
          QgsFeature addedFeature = addedFeatures[addedFid];
          // TODO: better to update form mLayer->attributes() ?
          for ( int i = 0; i < feature.fields()->size(); i++ )
          {
            if ( feature.fields()->field( i ).name() == QgsGrassVectorMap::topoSymbolFieldName() )
            {
              continue;
            }
            if ( feature.attributes().at( i ).isNull() )
            {
              continue;
            }
            addedFeature.setAttribute( i, feature.attributes().at( i ) );
          }
          addedFeatures[addedFid] = addedFeature;
        }
      }

      // Update all changed attributes
      QgsChangedAttributesMap &changedAttributes = const_cast<QgsChangedAttributesMap &>( mEditBuffer->changedAttributeValues() );
      const auto constKeys = changedAttributes.keys();
      for ( QgsFeatureId changedFid : constKeys )
      {
        int changedCat = QgsGrassFeatureIterator::catFromFid( changedFid );
        int realChangedCat = changedCat;
        if ( mLayer->map()->newCats().contains( changedFid ) )
        {
          realChangedCat = mLayer->map()->newCats().value( changedFid );
        }
        QgsDebugMsgLevel( QString( "changedFid = %1 changedCat = %2 realChangedCat = %3" )
                          .arg( changedFid ).arg( changedCat ).arg( realChangedCat ), 2 );
        if ( realChangedCat == newCat )
        {
          QgsAttributeMap attributeMap = changedAttributes[changedFid];
          const auto constKeys = attributeMap.keys();
          for ( int index : constKeys )
          {
            attributeMap[index] = feature.attributes().value( index );
          }
          changedAttributes[changedFid] = attributeMap;
        }
      }
#endif

      if ( mLayer->hasTable() )
      {
        QString error;
        // The record may exist if cat is manually defined by user (currently editing of cat column is disabled )
        bool recordExists = mLayer->recordExists( newCat, error );
        if ( !error.isEmpty() )
        {
          QgsGrass::warning( error );
        }
        else
        {
          error.clear();
          if ( !recordExists )
          {
            QgsDebugMsgLevel( "record does not exist", 2 );
            if ( mLayer->attributes().contains( newCat ) )
            {
              QgsDebugMsgLevel( "attributes exist -> reinsert", 2 );
              mLayer->reinsertAttributes( newCat, error );
            }
            else
            {
              mLayer->insertAttributes( newCat, feature, error );
            }
          }
          else
          {
            // Currently disabled
#if 0
            // Manual entry of cat is not currently allowed
            // TODO: open warning dialog?
            // For now we are expecting that user knows what he is doing.
            // We update existing record by non null values and set feature null values to existing values
            mLayer->updateAttributes( newCat, feature, error ); // also updates feature by existing non null attributes
#endif
          }
          if ( !error.isEmpty() )
          {
            QgsGrass::warning( error );
          }
        }
      }

      // update table
      emit dataChanged();
    }
  }
  else
  {
    QgsDebugMsgLevel( "undo of old deleted feature", 2 );
    int oldLid = lid;
    realLine = oldLid;
    int realCat = cat;
    int layerField = QgsGrassFeatureIterator::layerFromFid( fid );
    if ( mLayer->map()->newLids().contains( oldLid ) ) // if it was changed already
    {
      realLine = mLayer->map()->newLids().value( oldLid );
    }
    if ( mLayer->map()->newCats().contains( fid ) )
    {
      realCat = mLayer->map()->newCats().value( fid );
    }
    QgsDebugMsgLevel( QString( "fid = %1 lid = %2 realLine = %3 cat = %4 realCat = %5 layerField = %6" )
                      .arg( fid ).arg( lid ).arg( realLine ).arg( cat ).arg( realCat ).arg( layerField ), 2 );

    if ( realLine > 0 )
    {
      QgsDebugMsgLevel( QString( "reading realLine = %1" ).arg( realLine ), 2 );
      int realType = readLine( mPoints, mCats, realLine );
      if ( realType > 0 )
      {
        QgsDebugMsgLevel( QString( "the line exists realType = %1, add the cat to that line" ).arg( realType ), 2 );
        type = realType;
      }
      else // should not happen
      {
        QgsDebugError( "cannot read realLine" );
        return;
      }
    }
    else
    {
      QgsDebugMsgLevel( QString( "the line does not exist -> restore old geometry" ), 2 );
      const QgsAbstractGeometry *geometry = nullptr;

      // If it is not new feature, we should have the geometry in oldGeometries
      if ( mLayer->map()->oldGeometries().contains( lid ) )
      {
        geometry = mLayer->map()->oldGeometries().value( lid );
        type = mLayer->map()->oldTypes().value( lid );
      }
      else
      {
        QgsDebugError( "geometry of old, previously deleted feature not found" );
        return;
      }
      if ( !geometry )
      {
        QgsDebugMsgLevel( "geometry is null", 2 );
        return;
      }
      setPoints( mPoints, geometry );
    }

    QgsDebugMsgLevel( QString( "layerField = %1 realCat = %2" ).arg( layerField ).arg( realCat ), 2 );
    if ( realCat > 0 && layerField > 0 )
    {
      Vect_cat_set( mCats, layerField, realCat );

      // restore attributes
      QgsGrassVectorMapLayer *layer = mLayer;
      if ( layerField != mLayer->field() )
      {
        layer = otherEditLayer( layerField );
      }
      if ( !layer )
      {
        QgsDebugError( "Cannot get layer" );
      }
      else
      {
        QString error;
        bool recordExists = layer->recordExists( realCat, error );
        QgsDebugMsgLevel( QString( "recordExists = %1 error = %2" ).arg( recordExists ).arg( error ), 2 );
        if ( !recordExists && error.isEmpty() )
        {
          QgsDebugMsgLevel( "record does not exist -> restore attributes", 2 );
          error.clear();
          layer->reinsertAttributes( realCat, error );
          if ( !error.isEmpty() )
          {
            QgsGrass::warning( tr( "Cannot restore record with cat %1" ).arg( realCat ) );
          }
        }
      }
    }
  }

  QgsDebugMsgLevel( QString( "type = %1 mPoints->n_points = %2" ).arg( type ).arg( mPoints->n_points ), 2 );
  if ( type > 0 && mPoints->n_points > 0 )
  {
    int newLid = 0;
    mLayer->map()->lockReadWrite();
    if ( realLine > 0 )
    {
      newLid = rewriteLine( realLine, type, mPoints, mCats );
    }
    else
    {
      // TODO: use writeLine() and move setting oldLids, newLids there
      newLid = Vect_write_line( map(), type, mPoints, mCats );
      // fid may be new (negative) or old, if this is delete undo
      int oldLid = QgsGrassFeatureIterator::lidFromFid( fid );

      mLayer->map()->oldLids()[newLid] = oldLid;
      mLayer->map()->newLids()[oldLid] = newLid;
      QgsDebugMsgLevel( QString( "oldLid = %1 newLine = %2" ).arg( oldLid ).arg( newLid ), 2 );

      QgsDebugMsgLevel( QString( "oldLids : %1 -> %2" ).arg( newLid ).arg( oldLid ), 2 );
      mLayer->map()->oldLids()[newLid] = oldLid;
      QgsDebugMsgLevel( QString( "newLids : %1 -> %2" ).arg( oldLid ).arg( newLid ), 2 );
      mLayer->map()->newLids()[oldLid] = newLid;
    }
    mLayer->map()->unlockReadWrite();

    QgsDebugMsgLevel( QString( "newLine = %1" ).arg( newLid ), 2 );

    setAddedFeaturesSymbol();
  }
  QgsDebugMsgLevel( QString( "mLayer->cidxFieldIndex() = %1 cidxFieldNumCats() = %2" )
                    .arg( mLayer->cidxFieldIndex() ).arg( mLayer->cidxFieldNumCats() ), 2 );
}

void QgsGrassProvider::onFeatureDeleted( QgsFeatureId fid )
{
  if ( !mLayer )
    return;

  QgsDebugMsgLevel( QString( "fid = %1" ).arg( fid ), 2 );

  int oldLid = QgsGrassFeatureIterator::lidFromFid( fid );
  int cat = QgsGrassFeatureIterator::catFromFid( fid );
  int layerField = 0;
  if ( FID_IS_NEW( fid ) )
  {
    layerField = mLayerField;
  }
  else
  {
    layerField = QgsGrassFeatureIterator::layerFromFid( fid );
  }

  int realLine = oldLid;
  int realCat = cat;
  if ( mLayer->map()->newLids().contains( oldLid ) ) // if it was changed already
  {
    realLine = mLayer->map()->newLids().value( oldLid );
  }
  if ( mLayer->map()->newCats().contains( fid ) )
  {
    realCat = mLayer->map()->newCats().value( fid );
  }

  QgsDebugMsgLevel( QString( "fid = %1 oldLid = %2 realLine = %3 cat = %4 realCat = %5 layerField = %6" )
                    .arg( fid ).arg( oldLid ).arg( realLine ).arg( cat ).arg( realCat ).arg( layerField ), 2 );

  int type = 0;
  mLayer->map()->lockReadWrite();
  G_TRY
  {
    int type = readLine( mPoints, mCats, realLine );
    if ( type <= 0 )
    {
      QgsDebugError( "cannot read line" );
    }
    else
    {
      // store only the first original geometry if it is not new feature, changed geometries are stored in the buffer
      if ( oldLid > 0 && !mLayer->map()->oldGeometries().contains( oldLid ) )
      {
        QgsAbstractGeometry *geometry = mLayer->map()->lineGeometry( oldLid );
        if ( geometry )
        {
          QgsDebugMsgLevel( QString( "save old geometry of oldLid = %1" ).arg( oldLid ), 2 );
          mLayer->map()->oldGeometries().insert( oldLid, geometry );
          mLayer->map()->oldTypes().insert( oldLid, type );
        }
        else
        {
          QgsDebugError( QString( "cannot read geometry of oldLid = %1" ).arg( oldLid ) );
        }
      }

      if ( realCat > 0 && layerField > 0 )
      {
        if ( Vect_field_cat_del( mCats, layerField, realCat ) == 0 )
        {
          // should not happen
          QgsDebugError( "the line does not have old category" );
        }
      }
      QgsDebugMsgLevel( QString( "mCats->n_cats = %1" ).arg( mCats->n_cats ), 2 );
      if ( mCats->n_cats > 0 )
      {
        QgsDebugMsgLevel( "the line has more cats -> rewrite", 2 );
        int newLid = rewriteLine( realLine, type, mPoints, mCats );
        Q_UNUSED( newLid )
      }
      else
      {
        QgsDebugMsgLevel( "no more cats on the line -> delete", 2 );

        Vect_delete_line_function_pointer( map(), realLine );
        // oldLids are mapping to the very first, original version (used by undo)
        int oldestLid = oldLid;
        if ( mLayer->map()->oldLids().contains( oldLid ) )
        {
          oldestLid = mLayer->map()->oldLids().value( oldLid );
        }
        QgsDebugMsgLevel( QString( "oldLid = %1 oldestLid = %2" ).arg( oldLid ).arg( oldestLid ), 2 );
        QgsDebugMsgLevel( QString( "newLids : %1 -> 0" ).arg( oldestLid ), 2 );
        mLayer->map()->newLids()[oldestLid] = 0;
      }

      // Delete record if orphan
      if ( realCat > 0 && layerField > 0 )
      {
        QgsGrassVectorMapLayer *layer = mLayer;
        if ( layerField != mLayer->field() )
        {
          layer = otherEditLayer( layerField );
        }
        if ( !layer )
        {
          QgsDebugError( "Cannot get layer" );
        }
        else
        {
          QString error;
          bool orphan = layer->isOrphan( realCat, error );
          QgsDebugMsgLevel( QString( "orphan = %1 error = %2" ).arg( orphan ).arg( error ), 2 );
          if ( orphan && error.isEmpty() )
          {
            QgsDebugMsgLevel( QString( "realCat = %1 is orphan -> delete record" ).arg( realCat ), 2 );
            error.clear();
            layer->deleteAttribute( realCat, error );
            if ( !error.isEmpty() )
            {
              QgsGrass::warning( tr( "Cannot delete orphan record with cat %1" ).arg( realCat ) );
            }
          }
        }
      }
    }
  }
  G_CATCH( QgsGrass::Exception & e )
  {
    QgsDebugError( QString( "Cannot rewrite/delete line : %1" ).arg( e.what() ) );
  }
  mLayer->map()->unlockReadWrite();

  if ( type == GV_BOUNDARY || type == GV_CENTROID )
  {
    setAddedFeaturesSymbol();
  }
}

void QgsGrassProvider::onGeometryChanged( QgsFeatureId fid, const QgsGeometry &geom )
{
  if ( !mLayer )
    return;

  int oldLid = QgsGrassFeatureIterator::lidFromFid( fid );
  int realLine = oldLid;
  if ( mLayer->map()->newLids().contains( oldLid ) ) // if it was changed already
  {
    realLine = mLayer->map()->newLids().value( oldLid );
  }
  QgsDebugMsgLevel( QString( "fid = %1 oldLid = %2 realLine = %3" ).arg( fid ).arg( oldLid ).arg( realLine ), 2 );

  int type = readLine( mPoints, mCats, realLine );
  QgsDebugMsgLevel( QString( "type = %1 n_points = %2" ).arg( type ).arg( mPoints->n_points ), 2 );
  if ( type < 1 ) // error
  {
    return;
  }
  mLastType = type;

  // store only the first original geometry if it is not new feature, changed geometries are stored in the buffer
  if ( oldLid > 0 && !mLayer->map()->oldGeometries().contains( oldLid ) )
  {
    QgsAbstractGeometry *geometry = mLayer->map()->lineGeometry( oldLid );
    if ( geometry )
    {
      QgsDebugMsgLevel( QString( "save old geometry of oldLid = %1" ).arg( oldLid ), 2 );
      mLayer->map()->oldGeometries().insert( oldLid, geometry );
      mLayer->map()->oldTypes().insert( oldLid, type );
    }
    else
    {
      QgsDebugError( QString( "cannot read geometry of oldLid = %1" ).arg( oldLid ) );
    }
  }

  setPoints( mPoints, geom.constGet() );

  mLayer->map()->lockReadWrite();
  // Vect_rewrite_line may delete/write the line with a new id
  int newLid = rewriteLine( realLine, type, mPoints, mCats );
  Q_UNUSED( newLid )

  mLayer->map()->unlockReadWrite();

  if ( type == GV_BOUNDARY || type == GV_CENTROID )
  {
    setAddedFeaturesSymbol();
  }
}

void QgsGrassProvider::onAttributeValueChanged( QgsFeatureId fid, int idx, const QVariant &value )
{
  if ( !mLayer )
    return;

  QgsDebugMsgLevel( QString( "fid = %1 idx = %2 value = %3" ).arg( fid ).arg( idx ).arg( value.toString() ), 2 );

  int layerField = QgsGrassFeatureIterator::layerFromFid( fid );
  int cat = QgsGrassFeatureIterator::catFromFid( fid );
  QgsDebugMsgLevel( QString( "layerField = %1" ).arg( layerField ), 2 );

  if ( !FID_IS_NEW( fid ) && ( layerField > 0 && layerField != mLayerField ) )
  {
    QgsDebugMsgLevel( "changing attributes in different layer is not allowed", 2 );
    // reset the value
    QgsChangedAttributesMap &changedAttributes = mEditBuffer->mChangedAttributeValues;
    if ( idx == mLayer->keyColumn() )
    {
      // should not happen because cat field is not editable
      changedAttributes[fid][idx] = cat;
    }
    else
    {
      changedAttributes[fid][idx] = QgsGrassFeatureIterator::nonEditableValue( layerField );
    }
    // update table
    // TODO: This would be too slow with bulk updates (field calculator for example), causing update
    // of the whole table after each change. How to update single row?
    //emit dataChanged();
    return;
  }

  int oldLid = QgsGrassFeatureIterator::lidFromFid( fid );

  int realLine = oldLid;
  int realCat = cat;
  if ( mLayer->map()->newLids().contains( oldLid ) ) // if it was changed already
  {
    realLine = mLayer->map()->newLids().value( oldLid );
  }
  if ( mLayer->map()->newCats().contains( fid ) )
  {
    realCat = mLayer->map()->newCats().value( fid );
  }
  QgsDebugMsgLevel( QString( "fid = %1 oldLid = %2 realLine = %3 cat = %4 realCat = %5" )
                    .arg( fid ).arg( oldLid ).arg( realLine ).arg( cat ).arg( realCat ), 2 );

  // index is for current fields
  if ( idx < 0 || idx > mEditLayer->fields().size() )
  {
    QgsDebugError( "index out of range" );
    return;
  }
  QgsField field = mEditLayer->fields().at( idx );

  QgsDebugMsgLevel( "field.name() = " + field.name() + " keyColumnName() = " + mLayer->keyColumnName(), 2 );
  // TODO: Changing existing category is currently disabled (read only widget set on layer)
  //       because it makes it all too complicated
  if ( field.name() == mLayer->keyColumnName() )
  {
    // user changed category -> rewrite line
    QgsDebugMsgLevel( "cat changed -> rewrite line", 2 );
    int type = readLine( mPoints, mCats, realLine );
    if ( type <= 0 )
    {
      QgsDebugError( "cannot read line" );
    }
    else
    {
      if ( Vect_field_cat_del( mCats, mLayerField, realCat ) == 0 )
      {
        // should not happen
        QgsDebugError( "the line does not have old category" );
      }

      int newCat = value.toInt();
      QgsDebugMsgLevel( QString( "realCat = %1 newCat = %2" ).arg( realCat ).arg( newCat ), 2 );
      if ( newCat == 0 )
      {
        QgsDebugMsgLevel( "new category is 0", 2 );
        // TODO: remove category from line?
      }
      else
      {
        Vect_cat_set( mCats, mLayerField, newCat );
        mLayer->map()->lockReadWrite();
        int newLid = rewriteLine( realLine, type, mPoints, mCats );
        Q_UNUSED( newLid )
        mLayer->map()->newCats()[fid] = newCat;

        // TODO: - store the new cat somewhere for cats mapping
        //       - insert record if does not exist
        //       - update feature attributes by existing record?

        mLayer->map()->unlockReadWrite();
      }
    }
  }
  else
  {
    int undoIndex = mEditLayer->undoStack()->index();
    QgsDebugMsgLevel( QString( "undoIndex = %1" ).arg( undoIndex ), 2 );
    if ( realCat > 0 )
    {
      QString error;
      bool recordExists = mLayer->recordExists( realCat, error );
      if ( !error.isEmpty() )
      {
        QgsGrass::warning( error );
      }
      error.clear();
      mLayer->changeAttributeValue( realCat, field, value, error );
      if ( !error.isEmpty() )
      {
        QgsGrass::warning( error );
      }
      if ( !recordExists )
      {
        mLayer->map()->undoCommands()[undoIndex]
            << new QgsGrassUndoCommandChangeAttribute( this, fid, realLine, mLayerField, realCat, false, true );
      }
    }
    else
    {
      int newCat = getNewCat();
      QgsDebugMsgLevel( QString( "no cat -> add new cat %1 to line" ).arg( newCat ), 2 );
      int type = readLine( mPoints, mCats, realLine );
      if ( type <= 0 )
      {
        QgsDebugError( "cannot read line" );
      }
      else
      {
        Vect_cat_set( mCats, mLayerField, newCat );
        mLayer->map()->lockReadWrite();
        int newLid = rewriteLine( realLine, type, mPoints, mCats );
        Q_UNUSED( newLid )
        mLayer->map()->newCats()[fid] = newCat;

        QString error;
        bool recordExists = mLayer->recordExists( newCat, error );
        if ( !error.isEmpty() )
        {
          QgsGrass::warning( error );
        }
        error.clear();
        // it does insert new record if it doesn't exist
        mLayer->changeAttributeValue( newCat, field, value, error );
        if ( !error.isEmpty() )
        {
          QgsGrass::warning( error );
        }

        mLayer->map()->undoCommands()[undoIndex]
            << new QgsGrassUndoCommandChangeAttribute( this, fid, newLid, mLayerField, newCat, true, !recordExists );

        mLayer->map()->unlockReadWrite();
      }
    }
  }
}

void QgsGrassProvider::onAttributeAdded( int idx )
{
  if ( !mLayer )
    return;

  QgsDebugMsgLevel( QString( "idx = %1" ).arg( idx ), 2 );
  if ( idx < 0 || idx >= mEditLayer->fields().size() )
  {
    QgsDebugError( "index out of range" );
  }
  QString error;
  mLayer->addColumn( mEditLayer->fields().at( idx ), error );
  if ( !error.isEmpty() )
  {
    QgsDebugError( error );
    QgsGrass::warning( error );
    // TODO: remove the column somehow from the layer/buffer - undo?
  }
  else
  {
    mEditLayerFields = mEditLayer->fields();
    emit fieldsChanged();
  }
}

void QgsGrassProvider::onAttributeDeleted( int idx )
{
  if ( !mLayer )
    return;

  QgsDebugMsgLevel( QString( "idx = %1 mEditLayerFields.size() = %2" ).arg( idx ).arg( mEditLayerFields.size() ), 2 );
  // The field was already removed from mEditLayer->fields() -> using stored last version of fields
  if ( idx < 0 || idx >= mEditLayerFields.size() )
  {
    QgsDebugError( "index out of range" );
    return;
  }
  QgsField deletedField = mEditLayerFields.at( idx );
  QgsDebugMsgLevel( QString( "deletedField.name() = %1" ).arg( deletedField.name() ), 2 );

  QString error;
  mLayer->deleteColumn( deletedField, error );
  if ( !error.isEmpty() )
  {
    QgsDebugError( error );
    QgsGrass::warning( error );
    // TODO: get back the column somehow to the layer/buffer - undo?
  }
  else
  {
    mEditLayerFields = mEditLayer->fields();
    emit fieldsChanged();
  }
}

void QgsGrassProvider::setAddedFeaturesSymbol()
{
  if ( !mEditBuffer || !mLayer )
  {
    return;
  }
  QgsFeatureMap &features = mEditBuffer->mAddedFeatures;
  for ( auto it = features.constBegin(); it != features.constEnd(); ++it )
  {
    QgsFeature feature = it.value();
    if ( !feature.hasGeometry() )
    {
      continue;
    }
    int lid = QgsGrassFeatureIterator::lidFromFid( it.key() );
    int realLid = lid;
    if ( mLayer->map()->newLids().contains( lid ) )
    {
      realLid = mLayer->map()->newLids().value( lid );
    }
    QgsDebugMsgLevel( QString( "fid = %1 lid = %2 realLid = %3" ).arg( it.key() ).arg( lid ).arg( realLid ), 2 );
    QgsGrassVectorMap::TopoSymbol symbol = mLayer->map()->topoSymbol( realLid );
    // the feature may be without fields and set attribute by name does not work
    int index = mLayer->fields().indexFromName( QgsGrassVectorMap::topoSymbolFieldName() );
    feature.setAttribute( index, symbol );
    features[it.key()] = feature;
  }
}

void QgsGrassProvider::onUndoIndexChanged( int currentIndex )
{
  if ( !mLayer )
    return;

  QgsDebugMsgLevel( QString( "currentIndex = %1" ).arg( currentIndex ), 2 );
  // multiple commands maybe undone with single undoIndexChanged signal
  QList<int> indexes = mLayer->map()->undoCommands().keys();
  std::sort( indexes.begin(), indexes.end() );
  for ( int i = indexes.size() - 1; i >= 0; i-- )
  {
    int index = indexes[i];
    if ( index < currentIndex )
    {
      break;
    }
    QgsDebugMsgLevel( QString( "index = %1" ).arg( index ), 2 );
    if ( mLayer->map()->undoCommands().contains( index ) )
    {
      QgsDebugMsgLevel( QString( "%1 undo commands" ).arg( mLayer->map()->undoCommands()[index].size() ), 2 );

      for ( int j = 0; j < mLayer->map()->undoCommands()[index].size(); j++ )
      {
        mLayer->map()->undoCommands()[index][j]->undo();
        delete mLayer->map()->undoCommands()[index][j];
      }
      mLayer->map()->undoCommands().remove( index );
    }
  }
}

bool QgsGrassProvider::addAttributes( const QList<QgsField> &attributes )
{
  if ( !mLayer )
    return false;

  Q_UNUSED( attributes )
  // update fields because QgsVectorLayerEditBuffer::commitChanges() checks old /new fields count
  mLayer->updateFields();
  return true;
}

bool QgsGrassProvider::deleteAttributes( const QgsAttributeIds &attributes )
{
  if ( !mLayer )
    return false;

  Q_UNUSED( attributes )
  // update fields because QgsVectorLayerEditBuffer::commitChanges() checks old /new fields count
  mLayer->updateFields();
  return true;
}

void QgsGrassProvider::onBeforeCommitChanges()
{
  if ( !mLayer )
    return;

  mLayer->map()->clearUndoCommands();
}

void QgsGrassProvider::onBeforeRollBack()
{
}

void QgsGrassProvider::onEditingStopped()
{
  QgsVectorLayer *vectorLayer = qobject_cast<QgsVectorLayer *>( sender() );
  closeEdit( false, vectorLayer );
}

// -------------------------------------------------------------------------------

int QgsGrassProvider::cidxGetNumFields()
{
  return ( Vect_cidx_get_num_fields( map() ) );
}

int QgsGrassProvider::cidxGetFieldNumber( int idx )
{
  if ( idx < 0 || idx >= cidxGetNumFields() )
  {
    QgsDebugError( QString( "idx %1 out of range (0,%2)" ).arg( idx ).arg( cidxGetNumFields() - 1 ) );
    return 0;
  }
  return ( Vect_cidx_get_field_number( map(), idx ) );
}

int QgsGrassProvider::cidxGetMaxCat( int idx )
{
  QgsDebugMsgLevel( QString( "idx = %1" ).arg( idx ), 2 );
  if ( idx < 0 || idx >= cidxGetNumFields() )
  {
    QgsDebugError( QString( "idx %1 out of range (0,%2)" ).arg( idx ).arg( cidxGetNumFields() - 1 ) );
    return 0;
  }

  int ncats = Vect_cidx_get_num_cats_by_index( map(), idx );
  QgsDebugMsgLevel( QString( "ncats = %1" ).arg( ncats ), 2 );

  if ( ncats == 0 )
  {
    return 0;
  }

  int cat, type, id;
  Vect_cidx_get_cat_by_index( map(), idx, ncats - 1, &cat, &type, &id );

  return cat;
}

int QgsGrassProvider::getNewCat()
{
  if ( !mLayer )
    return 1;

  QgsDebugMsgLevel( QString( "get new cat for cidxFieldIndex() = %1" ).arg( mLayer->cidxFieldIndex() ), 2 );
  if ( mLayer->cidxFieldIndex() == -1 )
  {
    // No features with this field yet in map
    return 1;
  }
  else
  {
    return cidxGetMaxCat( mLayer->cidxFieldIndex() ) + 1;
  }
}

QgsGrassVectorMapLayer *QgsGrassProvider::openLayer() const
{
  if ( !mLayer )
    return nullptr;

  return mLayer->map()->openLayer( mLayerField );
}

struct Map_info *QgsGrassProvider::map() const
{
  Q_ASSERT( mLayer );
  // cppcheck-suppress assertWithSideEffect
  Q_ASSERT( mLayer->map() );
  // cppcheck-suppress assertWithSideEffect
  Q_ASSERT( mLayer->map()->map() );
  return mLayer->map()->map();
}

void QgsGrassProvider::setMapset()
{
  QgsGrass::setMapset( mGrassObject.gisdbase(), mGrassObject.location(), mGrassObject.mapset() );
}

QgsGrassVectorMapLayer *QgsGrassProvider::otherEditLayer( int layerField )
{
  const auto constMOtherEditLayers = mOtherEditLayers;
  for ( QgsGrassVectorMapLayer *layer : constMOtherEditLayers )
  {
    if ( layer->field() == layerField )
    {
      return layer;
    }
  }
  if ( !mLayer )
    return nullptr;

  QgsGrassVectorMapLayer *layer = mLayer->map()->openLayer( layerField );
  if ( layer )
  {
    layer->startEdit();
    mOtherEditLayers << layer;
  }
  return layer;
}

QString QgsGrassProvider::name() const
{
  return GRASS_KEY;
}

QString QgsGrassProvider::description() const
{
  return tr( "GRASS %1 vector provider" ).arg( GRASS_VERSION_MAJOR );
}

