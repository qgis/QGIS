/***************************************************************************
                            qgsgrassvectormap.cpp
                             -------------------
    begin                : September, 2015
    copyright            : (C) 2015 by Radim Blazek
    email                : radim.blazek@gmail.com
 ***************************************************************************/
/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <QFileInfo>
#include <QMessageBox>

#include "qgslinestringv2.h"
#include "qgspolygonv2.h"
#include "qgspointv2.h"

#include "qgslogger.h"
#include "qgsgeometry.h"

#include "qgsgrass.h"
#include "qgsgrassvectormap.h"
#include "qgsgrassvectormaplayer.h"
#include "qgsgrassundocommand.h"

extern "C"
{
#include <grass/version.h>
#if defined(_MSC_VER) && defined(M_PI_4)
#undef M_PI_4 //avoid redefinition warning
#endif
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

QgsGrassVectorMap::QgsGrassVectorMap( const QgsGrassObject & grassObject )
    : mGrassObject( grassObject )
    , mValid( false )
    , mOpen( false )
    , mFrozen( false )
    , mIsEdited( false )
    , mVersion( 0 )
    , mMap( 0 )
    , mIs3d( false )
    , mOldNumLines( 0 )
{
  QgsDebugMsg( "grassObject = " + grassObject.toString() );
  openMap();
  mOpen = true;
}

QgsGrassVectorMap::~QgsGrassVectorMap()
{
  QgsDebugMsg( "grassObject = " + mGrassObject.toString() );
  // TODO close
  QgsGrass::vectDestroyMapStruct( mMap );
}

int QgsGrassVectorMap::userCount() const
{
  int count = 0;
  Q_FOREACH ( QgsGrassVectorMapLayer *layer, mLayers )
  {
    count += layer->userCount();
  }
  QgsDebugMsg( QString( "count = %1" ).arg( count ) );
  return count;
}

bool QgsGrassVectorMap::open()
{
  QgsDebugMsg( toString() );
  if ( mOpen )
  {
    QgsDebugMsg( "already open" );
    return true;
  }
  lockOpenClose();
  bool result = openMap();
  mOpen = true;
  unlockOpenClose();
  return result;
}

void QgsGrassVectorMap::close()
{
  QgsDebugMsg( toString() );
  if ( !mOpen )
  {
    QgsDebugMsg( "is not open" );
    return;
  }
  lockOpenClose();
  closeAllIterators(); // blocking
  closeMap();
  mOpen = false;
  unlockOpenClose();
  return;
}

bool QgsGrassVectorMap::openMap()
{
  // TODO: refresh layers (reopen)
  QgsDebugMsg( toString() );
  QgsGrass::lock();
  QgsGrass::setLocation( mGrassObject.gisdbase(), mGrassObject.location() );

  // Find the vector
  const char *ms = G_find_vector2( mGrassObject.name().toUtf8().data(),  mGrassObject.mapset().toUtf8().data() );

  if ( !ms )
  {
    QgsDebugMsg( "Cannot find GRASS vector" );
    QgsGrass::unlock();
    return false;
  }

  // Read the time of vector dir before Vect_open_old, because it may take long time (when the vector
  // could be owerwritten)
  QFileInfo di( mGrassObject.mapsetPath() + "/vector/" + mGrassObject.name() );
  mLastModified = di.lastModified();

  di.setFile( mGrassObject.mapsetPath() + "/vector/" + mGrassObject.name() + "/dbln" );
  mLastAttributesModified = di.lastModified();

  mMap = QgsGrass::vectNewMapStruct();
  // Do we have topology and cidx (level2)
  int level = -1;
  G_TRY
  {
    Vect_set_open_level( 2 );
    level = Vect_open_old_head( mMap, mGrassObject.name().toUtf8().data(), mGrassObject.mapset().toUtf8().data() );
    Vect_close( mMap );
  }
  G_CATCH( QgsGrass::Exception &e )
  {
    QgsGrass::warning( e );
    level = -1;
  }

  if ( level == -1 )
  {
    QgsDebugMsg( "Cannot open GRASS vector head" );
    QgsGrass::unlock();
    return false;
  }
  else if ( level == 1 )
  {
    QMessageBox::StandardButton ret = QMessageBox::question( 0, "Warning",
                                      QObject::tr( "GRASS vector map %1 does not have topology. Build topology?" ).arg( mGrassObject.name() ),
                                      QMessageBox::Ok | QMessageBox::Cancel );

    if ( ret == QMessageBox::Cancel )
    {
      QgsGrass::unlock();
      return false;
    }
  }

  // Open vector
  G_TRY
  {
    Vect_set_open_level( level );
    Vect_open_old( mMap, mGrassObject.name().toUtf8().data(), mGrassObject.mapset().toUtf8().data() );
  }
  G_CATCH( QgsGrass::Exception &e )
  {
    QgsGrass::warning( QString( "Cannot open GRASS vector: %1" ).arg( e.what() ) );
    QgsGrass::unlock();
    return false;
  }

  if ( level == 1 )
  {
    G_TRY
    {
#if defined(GRASS_VERSION_MAJOR) && defined(GRASS_VERSION_MINOR) && \
    ( ( GRASS_VERSION_MAJOR == 6 && GRASS_VERSION_MINOR >= 4 ) || GRASS_VERSION_MAJOR > 6 )
      Vect_build( mMap );
#else
      Vect_build( mMap, stderr );
#endif
    }
    G_CATCH( QgsGrass::Exception &e )
    {
      QgsGrass::warning( QString( "Cannot build topology: %1" ).arg( e.what() ) );
      QgsGrass::unlock();
      return false;
    }
  }
  QgsDebugMsg( "GRASS map successfully opened" );

  mIs3d = Vect_is_3d( mMap );

  QgsGrass::unlock();
  mValid = true;
  return true;
}

bool QgsGrassVectorMap::startEdit()
{
  QgsDebugMsg( toString() );

  lockOpenClose();

  closeAllIterators(); // blocking

  // TODO: Can it still happen? QgsGrassVectorMapStore singleton is used now.
#if 0
  // Check number of maps (the problem may appear if static variables are not shared - runtime linker)
  if ( mMaps.size() == 0 )
  {
    QMessageBox::warning( 0, "Warning", "No maps opened in mMaps, probably problem in runtime linking, "
                          "static variables are not shared by provider and plugin." );
    return false;
  }
#endif

  /* Close map */
  mValid = false;

  QgsGrass::lock();
  // Mapset must be set before Vect_close()
  QgsGrass::setMapset( mGrassObject.gisdbase(), mGrassObject.location(), mGrassObject.mapset() );

  int level = -1;
  G_TRY
  {
    Vect_close( mMap );
    Vect_set_open_level( 2 );
    level = Vect_open_update( mMap, mGrassObject.name().toUtf8().data(), mGrassObject.mapset().toUtf8().data() );
    if ( level < 2 )
    {
      QgsDebugMsg( "Cannot open GRASS vector for update on level 2." );
    }
  }
  G_CATCH( QgsGrass::Exception &e )
  {
    Q_UNUSED( e );
    QgsDebugMsg( QString( "Cannot open GRASS vector for update: %1" ).arg( e.what() ) );
  }

  if ( level < 2 )
  {
    // reopen vector for reading
    G_TRY
    {
      Vect_set_open_level( 2 );
      level = Vect_open_old( mMap, mGrassObject.name().toUtf8().data(), mGrassObject.mapset().toUtf8().data() );
      if ( level < 2 )
      {
        QgsDebugMsg( QString( "Cannot reopen GRASS vector: %1" ).arg( QgsGrass::errorMessage() ) );
      }
    }
    G_CATCH( QgsGrass::Exception &e )
    {
      Q_UNUSED( e );
      QgsDebugMsg( QString( "Cannot reopen GRASS vector: %1" ).arg( e.what() ) );
    }

    if ( level >= 2 )
    {
      mValid = true;
    }
    QgsGrass::unlock();
    unlockOpenClose();
    return false;
  }
  Vect_set_category_index_update( mMap );

  // Write history
  Vect_hist_command( mMap );

  mOldNumLines = Vect_get_num_lines( mMap );
  QgsDebugMsg( QString( "Vector successfully reopened for update mOldNumLines = %1" ).arg( mOldNumLines ) );

  mIsEdited = true;

  mValid = true;
  printDebug();

  QgsGrass::unlock();
  unlockOpenClose();
  emit dataChanged();
  return true;
}

bool QgsGrassVectorMap::closeEdit( bool newMap )
{
  Q_UNUSED( newMap );
  QgsDebugMsg( toString() );
  if ( !mValid || !mIsEdited )
  {
    return false;
  }

  // mValid = false; // close() is checking mValid

  lockOpenClose();
  closeAllIterators(); // blocking

  QgsGrass::lock();

  mOldLids.clear();
  mNewLids.clear();
  mOldGeometries.clear();
  mNewCats.clear();
  clearUndoCommands();

  // Mapset must be set before Vect_close()
  QgsGrass::setMapset( mGrassObject.gisdbase(), mGrassObject.location(), mGrassObject.mapset() );

#if defined(GRASS_VERSION_MAJOR) && defined(GRASS_VERSION_MINOR) && \
    ( ( GRASS_VERSION_MAJOR == 6 && GRASS_VERSION_MINOR >= 4 ) || GRASS_VERSION_MAJOR > 6 )
  Vect_build_partial( mMap, GV_BUILD_NONE );
  Vect_build( mMap );
#else
  Vect_build_partial( mMap, GV_BUILD_NONE, NULL );
  Vect_build( mMap, stderr );
#endif

  // TODO?
#if 0
  // If a new map was created close the map and return
  if ( newMap )
  {
    QgsDebugMsg( QString( "mLayers.size() = %1" ).arg( mLayers.size() ) );
    mUpdate = false;
    // Map must be set as valid otherwise it is not closed and topo is not written
    mValid = true;
    // TODO refresh layers ?
    //closeLayer( mLayerId );
    QgsGrass::unlock();
    unlockOpenClose();
    return true;
  }
#endif

  mIsEdited = false;
  QgsGrass::unlock();
  closeAllIterators(); // blocking

  closeMap();
  openMap();
  reloadLayers();
  mVersion++;
  unlockOpenClose();

  emit dataChanged();
  QgsDebugMsg( "edit closed" );
  return mValid;
}

void QgsGrassVectorMap::clearUndoCommands()
{
  Q_FOREACH ( int index, mUndoCommands.keys() )
  {
    Q_FOREACH ( QgsGrassUndoCommand *command, mUndoCommands[index] )
    {
      delete command;
    }
  }
  mUndoCommands.clear();
}

QgsGrassVectorMapLayer * QgsGrassVectorMap::openLayer( int field )
{
  QgsDebugMsg( QString( "%1 field = %2" ).arg( toString() ).arg( field ) );

  // There are 2 locks on openLayer(), it must be locked when the map is being opened/closed/updated
  // but that lock must not block closeLayer() because close/update map closes first all iterators
  // which call closeLayer() and using single lock would result in dead lock.

  lockOpenCloseLayer();
  lockOpenClose();
  QgsGrassVectorMapLayer *layer = 0;
  // Check if this layer is already open
  Q_FOREACH ( QgsGrassVectorMapLayer *l, mLayers )
  {
    if ( l->field() == field )
    {
      QgsDebugMsg( "Layer exists" );
      layer = l;
      if ( layer->userCount() == 0 )
      {
        layer->load();
      }
    }
  }

  if ( !layer )
  {
    layer = new QgsGrassVectorMapLayer( this, field ) ;
    layer->load();
    mLayers << layer;
  }

  layer->addUser();
  unlockOpenClose();
  unlockOpenCloseLayer();
  return layer;
}

void QgsGrassVectorMap::reloadLayers()
{
  Q_FOREACH ( QgsGrassVectorMapLayer *l, mLayers )
  {
    l->load();
  }
}

void QgsGrassVectorMap::closeLayer( QgsGrassVectorMapLayer * layer )
{
  if ( !layer )
  {
    return;
  }

  QgsDebugMsg( QString( "Close layer %1 usersCount = %2" ).arg( toString() ).arg( layer->userCount() ) );

  lockOpenCloseLayer();
  layer->removeUser();

  if ( layer->userCount() == 0 )   // No more users, free sources
  {
    QgsDebugMsg( "No more users -> clear" );
    layer->clear();
  }

  QgsDebugMsg( QString( "%1 map users" ).arg( userCount() ) );
  if ( userCount() == 0 )
  {
    QgsDebugMsg( "No more map users -> close" );
    // Once was probably causing dead lock; move to QgsGrassVectorMapStore?
    close();
  }

  QgsDebugMsg( "layer closed" );
  unlockOpenCloseLayer();
}

void QgsGrassVectorMap::closeMap()
{
  QgsDebugMsg( toString() );
  QgsGrass::lock();
  if ( !mValid )
  {
    QgsDebugMsg( "map is not valid" );
  }
  else
  {
    // Mapset must be set before Vect_close()
    QgsGrass::setMapset( mGrassObject.gisdbase(), mGrassObject.location(), mGrassObject.mapset() );

    G_TRY
    {
      Vect_close( mMap );
      QgsDebugMsg( "map closed" );
    }
    G_CATCH( QgsGrass::Exception &e )
    {
      QgsDebugMsg( "Vect_close failed:" + QString( e.what() ) );
    }
  }
  QgsGrass::vectDestroyMapStruct( mMap );
  mMap = 0;
  mOldNumLines = 0;
  mValid = false;
  QgsGrass::unlock();
}

void QgsGrassVectorMap::update()
{
  QgsDebugMsg( toString() );
  lockOpenClose();
  closeAllIterators(); // blocking
  closeMap();
  openMap();
  reloadLayers();
  unlockOpenClose();
  emit dataChanged();
}

bool QgsGrassVectorMap::mapOutdated()
{

  QString dp = mGrassObject.mapsetPath() + "/vector/" + mGrassObject.name();
  QFileInfo di( dp );

  if ( mLastModified < di.lastModified() )
  {
    // If the cidx file has been deleted, the map is currently being modified
    // by an external tool. Do not update until the cidx file has been recreated.
    if ( !QFileInfo( dp + "/cidx" ).exists() )
    {
      QgsDebugMsg( "The map is being modified and is unavailable : " + mGrassObject.toString() );
      return false;
    }
    QgsDebugMsg( "The map was modified : " + mGrassObject.toString() );
    return true;
  }
  return false;
}

bool QgsGrassVectorMap::attributesOutdated()
{

  QString dp = mGrassObject.mapsetPath() + "/vector/" + mGrassObject.name() + "/dbln";
  QFileInfo di( dp );

  if ( mLastAttributesModified < di.lastModified() )
  {
    QgsDebugMsg( "The attributes of the layer were modified : " + mGrassObject.toString() );

    return true;
  }
  return false;
}

int QgsGrassVectorMap::numLines()
{

  return ( Vect_get_num_lines( mMap ) );
}

int QgsGrassVectorMap::numAreas()
{

  return ( Vect_get_num_areas( mMap ) );
}

QString QgsGrassVectorMap::toString()
{
  return mGrassObject.mapsetPath() + "/" +  mGrassObject.name();
}

void QgsGrassVectorMap::printDebug()
{
  if ( !mValid || !mMap )
  {
    QgsDebugMsg( "map not valid" );
    return;
  }
  G_TRY
  {
#ifdef QGISDEBUG
    int ncidx = Vect_cidx_get_num_fields( mMap );
    QgsDebugMsg( QString( "ncidx = %1" ).arg( ncidx ) );

    for ( int i = 0; i < ncidx; i++ )
    {
      int layer = Vect_cidx_get_field_number( mMap, i );
      int ncats = Vect_cidx_get_num_cats_by_index( mMap, i );
      QgsDebugMsg( QString( "i = %1 layer = %2 ncats = %3" ).arg( i ).arg( layer ).arg( ncats ) );
    }
#endif
  }
  G_CATCH( QgsGrass::Exception &e )
  {
    Q_UNUSED( e )
    QgsDebugMsg( "Cannot read info from map: " + QString( e.what() ) );
  }
}

void QgsGrassVectorMap::lockOpenClose()
{
  QgsDebugMsg( "lockOpenClose" );
  mOpenCloseMutex.lock();
}

void QgsGrassVectorMap::unlockOpenClose()
{
  QgsDebugMsg( "unlockOpenClose" );
  mOpenCloseMutex.unlock();
}

void QgsGrassVectorMap::lockOpenCloseLayer()
{
  QgsDebugMsg( "lockOpenCloseLayer" );
  mOpenCloseLayerMutex.lock();
}

void QgsGrassVectorMap::unlockOpenCloseLayer()
{
  QgsDebugMsg( "unlockOpenCloseLayer" );
  mOpenCloseLayerMutex.unlock();
}

void QgsGrassVectorMap::lockReadWrite()
{
  if ( isEdited() )
  {
    QgsDebugMsgLevel( "lockReadWrite", 3 );
    mReadWriteMutex.lock();
  }
}

void QgsGrassVectorMap::unlockReadWrite()
{
  if ( isEdited() )
  {
    QgsDebugMsgLevel( "unlockReadWrite", 3 );
    mReadWriteMutex.unlock();
  }
}

QgsAbstractGeometryV2 * QgsGrassVectorMap::lineGeometry( int id )
{
  QgsDebugMsgLevel( QString( "id = %1" ).arg( id ), 3 );
  if ( !Vect_line_alive( mMap, id ) ) // should not happen (update mode!)?
  {
    QgsDebugMsg( QString( "line %1 is dead" ).arg( id ) );
    return 0;
  }

  struct line_pnts *points = Vect_new_line_struct();

  int type = Vect_read_line( mMap, points, 0, id );
  QgsDebugMsgLevel( QString( "type = %1 n_points = %2" ).arg( type ).arg( points->n_points ), 3 );
  if ( points->n_points == 0 )
  {
    Vect_destroy_line_struct( points );
    return 0;
  }

  QgsPointSequenceV2 pointList;
  pointList.reserve( points->n_points );
  for ( int i = 0; i < points->n_points; i++ )
  {
    pointList << QgsPointV2( is3d() ? QgsWKBTypes::PointZ : QgsWKBTypes::Point, points->x[i], points->y[i], points->z[i] );
  }

  Vect_destroy_line_struct( points );

  if ( type & GV_POINTS )
  {
    return pointList.first().clone();
  }
  else if ( type & GV_LINES )
  {
    QgsLineStringV2 * line = new QgsLineStringV2();
    line->setPoints( pointList );
    return line;
  }
  else if ( type & GV_FACE )
  {
    QgsPolygonV2 * polygon = new QgsPolygonV2();
    QgsLineStringV2 * ring = new QgsLineStringV2();
    ring->setPoints( pointList );
    polygon->setExteriorRing( ring );
    return polygon;
  }

  QgsDebugMsg( QString( "unknown type = %1" ).arg( type ) );
  return 0;
}

QgsAbstractGeometryV2 * QgsGrassVectorMap::nodeGeometry( int id )
{
  QgsDebugMsgLevel( QString( "id = %1" ).arg( id ), 3 );
  double x, y, z;
  Vect_get_node_coor( mMap, id, &x, &y, &z );
  return new QgsPointV2( is3d() ? QgsWKBTypes::PointZ : QgsWKBTypes::Point, x, y, z );
}

QgsAbstractGeometryV2 * QgsGrassVectorMap::areaGeometry( int id )
{
  QgsDebugMsgLevel( QString( "id = %1" ).arg( id ), 3 );
  QgsPolygonV2 * polygon = new QgsPolygonV2();

  struct line_pnts *points = Vect_new_line_struct();
  QgsDebugMsgLevel( QString( "points= %1" ).arg(( long )points ), 3 );
  // Vect_get_area_points and Vect_get_isle_pointsis using static variable -> lock
  // TODO: Faster to lock the whole feature iterator? Maybe only for areas?
  QgsGrass::lock();
  Vect_get_area_points( mMap, id, points );

  QgsPointSequenceV2 pointList;
  pointList.reserve( points->n_points );
  for ( int i = 0; i < points->n_points; i++ )
  {
    pointList << QgsPointV2( is3d() ? QgsWKBTypes::PointZ : QgsWKBTypes::Point, points->x[i], points->y[i], points->z[i] );
  }

  QgsLineStringV2 * ring = new QgsLineStringV2();
  ring->setPoints( pointList );
  polygon->setExteriorRing( ring );

  int nIsles = Vect_get_area_num_isles( mMap, id );
  for ( int i = 0; i < nIsles; i++ )
  {
    pointList.clear();
    int isle = Vect_get_area_isle( mMap, id, i );
    Vect_get_isle_points( mMap, isle, points );

    pointList.reserve( points->n_points );
    for ( int i = 0; i < points->n_points; i++ )
    {
      pointList <<  QgsPointV2( is3d() ? QgsWKBTypes::PointZ : QgsWKBTypes::Point, points->x[i], points->y[i], points->z[i] );
    }
    ring = new QgsLineStringV2();
    ring->setPoints( pointList );
    polygon->addInteriorRing( ring );
  }
  QgsGrass::unlock();
  Vect_destroy_line_struct( points );
  return polygon;
}

void QgsGrassVectorMap::closeAllIterators()
{
  QgsDebugMsg( toString() );
  // cancel and close all iterator
  // Iterators must be connected properly, otherwise may it result in dead lock!
  emit cancelIterators(); // non blocking
  emit closeIterators(); // blocking
  QgsDebugMsg( "iterators closed" );
}

//------------------------------------ QgsGrassVectorMapStore ------------------------------------
QgsGrassVectorMapStore * QgsGrassVectorMapStore::mStore = 0;

QgsGrassVectorMapStore::QgsGrassVectorMapStore()
{
}

QgsGrassVectorMapStore::~QgsGrassVectorMapStore()
{
}

QgsGrassVectorMapStore *QgsGrassVectorMapStore::instance()
{
  static QgsGrassVectorMapStore instance;
  if ( mStore )
  {
    return mStore;
  }
  return &instance;
}

QgsGrassVectorMap * QgsGrassVectorMapStore::openMap( const QgsGrassObject & grassObject )
{
  QgsDebugMsg( "grassObject = " + grassObject.toString() );

  mMutex.lock();
  QgsGrassVectorMap *map = 0;

  // Check if this map is already open
  Q_FOREACH ( QgsGrassVectorMap *m, mMaps )
  {
    if ( m->grassObject() == grassObject )
    {
      QgsDebugMsg( "The map already exists" );
      map = m;
      if ( !map->isOpen() )
      {
        map->open();
      }
    }
  }

  if ( !map )
  {
    map = new QgsGrassVectorMap( grassObject );
    mMaps << map;
  }

  mMutex.unlock();
  return map;
}

QgsGrassVectorMap::TopoSymbol QgsGrassVectorMap::topoSymbol( int lid )
{
  int type = Vect_read_line( mMap, 0, 0, lid );

  TopoSymbol symbol = TopoUndefined;
  if ( type == GV_POINT )
  {
    symbol = TopoPoint;
  }
  else if ( type == GV_CENTROID )
  {
    int area = Vect_get_centroid_area( mMap, lid );
    if ( area == 0 )
      symbol = TopoCentroidOut;
    else if ( area > 0 )
      symbol = TopoCentroidIn;
    else
      symbol = TopoCentroidDupl; /* area < 0 */
  }
  else if ( type == GV_LINE )
  {
    symbol = TopoLine;
  }
  else if ( type == GV_BOUNDARY )
  {
    int left, right;
    Vect_get_line_areas( mMap, lid, &left, &right );
    if ( left != 0 && right != 0 )
    {
      symbol = TopoBoundaryOk;
    }
    else if ( left == 0 && right == 0 )
    {
      symbol = TopoBoundaryError;
    }
    else if ( left == 0 )
    {
      symbol = TopoBoundaryErrorLeft;
    }
    else if ( right == 0 )
    {
      symbol = TopoBoundaryErrorRight;
    }
  }
  QgsDebugMsgLevel( QString( "lid = %1 type = %2 symbol = %3" ).arg( lid ).arg( type ).arg( symbol ), 3 );
  return symbol;
}
