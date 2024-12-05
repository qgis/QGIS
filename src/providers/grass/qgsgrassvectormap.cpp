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

#include "qgslinestring.h"
#include "qgspolygon.h"
#include "qgspoint.h"

#include "qgslogger.h"
#include "qgsgrass.h"
#include "qgsgrassvectormap.h"
#include "moc_qgsgrassvectormap.cpp"
#include "qgsgrassvectormaplayer.h"
#include "qgsgrassundocommand.h"

extern "C"
{
#include <grass/version.h>
#if defined( _MSC_VER ) && defined( M_PI_4 )
#undef M_PI_4 //avoid redefinition warning
#endif
#include <grass/gprojects.h>
#include <grass/gis.h>
#include <grass/dbmi.h>
#include <grass/vector.h>
}

QgsGrassVectorMap::QgsGrassVectorMap( const QgsGrassObject &grassObject )
  : mGrassObject( grassObject )
  , mValid( false )
  , mOpen( false )
  , mFrozen( false )
  , mIsEdited( false )
  , mVersion( 0 )
  , mIs3d( false )
  , mOldNumLines( 0 )
{
  QgsDebugMsgLevel( "grassObject = " + grassObject.toString(), 2 );
  openMap();
  mOpen = true;
}

QgsGrassVectorMap::~QgsGrassVectorMap()
{
  QgsDebugMsgLevel( "grassObject = " + mGrassObject.toString(), 2 );
  // TODO close
  QgsGrass::vectDestroyMapStruct( mMap );
}

int QgsGrassVectorMap::userCount() const
{
  int count = 0;
  const auto constMLayers = mLayers;
  for ( QgsGrassVectorMapLayer *layer : constMLayers )
  {
    count += layer->userCount();
  }
  QgsDebugMsgLevel( QString( "count = %1" ).arg( count ), 2 );
  return count;
}

bool QgsGrassVectorMap::open()
{
  QgsDebugMsgLevel( toString(), 2 );
  if ( mOpen )
  {
    QgsDebugMsgLevel( "already open", 2 );
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
  QgsDebugMsgLevel( toString(), 2 );
  if ( !mOpen )
  {
    QgsDebugMsgLevel( "is not open", 2 );
    return;
  }
  lockOpenClose();
  closeAllIterators(); // blocking
  closeMap();
  mOpen = false;
  unlockOpenClose();
}

bool QgsGrassVectorMap::openMap()
{
  // TODO: refresh layers (reopen)
  QgsDebugMsgLevel( toString(), 2 );
  QgsGrass::lock();
  QgsGrass::setLocation( mGrassObject.gisdbase(), mGrassObject.location() );

  // Find the vector
  const char *ms = G_find_vector2( mGrassObject.name().toUtf8().constData(), mGrassObject.mapset().toUtf8().constData() );

  if ( !ms )
  {
    QgsDebugError( "Cannot find GRASS vector" );
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
    level = Vect_open_old_head( mMap, mGrassObject.name().toUtf8().constData(), mGrassObject.mapset().toUtf8().constData() );
    Vect_close( mMap );
  }
  G_CATCH( QgsGrass::Exception & e )
  {
    QgsGrass::warning( e );
    level = -1;
  }

  if ( level == -1 )
  {
    QgsDebugError( "Cannot open GRASS vector head" );
    QgsGrass::unlock();
    return false;
  }
  else if ( level == 1 )
  {
    QMessageBox::StandardButton ret = QMessageBox::question( nullptr, QStringLiteral( "Warning" ), QObject::tr( "GRASS vector map %1 does not have topology. Build topology?" ).arg( mGrassObject.name() ), QMessageBox::Ok | QMessageBox::Cancel );

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
    Vect_open_old( mMap, mGrassObject.name().toUtf8().constData(), mGrassObject.mapset().toUtf8().constData() );
  }
  G_CATCH( QgsGrass::Exception & e )
  {
    QgsGrass::warning( QStringLiteral( "Cannot open GRASS vector: %1" ).arg( e.what() ) );
    QgsGrass::unlock();
    return false;
  }

  if ( level == 1 )
  {
    G_TRY
    {
      Vect_build( mMap );
    }
    G_CATCH( QgsGrass::Exception & e )
    {
      QgsGrass::warning( QStringLiteral( "Cannot build topology: %1" ).arg( e.what() ) );
      QgsGrass::unlock();
      return false;
    }
  }
  QgsDebugMsgLevel( "GRASS map successfully opened", 2 );

  mIs3d = Vect_is_3d( mMap );

  QgsGrass::unlock();
  mValid = true;
  return true;
}

bool QgsGrassVectorMap::startEdit()
{
  QgsDebugMsgLevel( toString(), 2 );

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
    level = Vect_open_update( mMap, mGrassObject.name().toUtf8().constData(), mGrassObject.mapset().toUtf8().constData() );
    if ( level < 2 )
    {
      QgsDebugError( "Cannot open GRASS vector for update on level 2." );
    }
  }
  G_CATCH( QgsGrass::Exception & e )
  {
    Q_UNUSED( e )
    QgsDebugError( QString( "Cannot open GRASS vector for update: %1" ).arg( e.what() ) );
  }

  if ( level < 2 )
  {
    // reopen vector for reading
    G_TRY
    {
      Vect_set_open_level( 2 );
      level = Vect_open_old( mMap, mGrassObject.name().toUtf8().constData(), mGrassObject.mapset().toUtf8().constData() );
      if ( level < 2 )
      {
        QgsDebugError( QString( "Cannot reopen GRASS vector: %1" ).arg( QgsGrass::errorMessage() ) );
      }
    }
    G_CATCH( QgsGrass::Exception & e )
    {
      Q_UNUSED( e )
      QgsDebugError( QString( "Cannot reopen GRASS vector: %1" ).arg( e.what() ) );
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
  QgsDebugMsgLevel( QString( "Vector successfully reopened for update mOldNumLines = %1" ).arg( mOldNumLines ), 2 );

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
  Q_UNUSED( newMap )
  QgsDebugMsgLevel( toString(), 2 );
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

  Vect_build_partial( mMap, GV_BUILD_NONE );
  Vect_build( mMap );

  // TODO?
#if 0
  // If a new map was created close the map and return
  if ( newMap )
  {
    QgsDebugMsgLevel( QString( "mLayers.size() = %1" ).arg( mLayers.size() ), 2 );
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
  QgsDebugMsgLevel( "edit closed", 2 );
  return mValid;
}

void QgsGrassVectorMap::clearUndoCommands()
{
  for ( auto it = mUndoCommands.constBegin(); it != mUndoCommands.constEnd(); ++it )
  {
    const auto constValue = it.value();
    for ( QgsGrassUndoCommand *command : constValue )
    {
      delete command;
    }
  }
  mUndoCommands.clear();
}

QgsGrassVectorMapLayer *QgsGrassVectorMap::openLayer( int field )
{
  QgsDebugMsgLevel( QString( "%1 field = %2" ).arg( toString() ).arg( field ), 2 );

  // There are 2 locks on openLayer(), it must be locked when the map is being opened/closed/updated
  // but that lock must not block closeLayer() because close/update map closes first all iterators
  // which call closeLayer() and using single lock would result in dead lock.

  lockOpenCloseLayer();
  lockOpenClose();
  QgsGrassVectorMapLayer *layer = nullptr;
  // Check if this layer is already open
  const auto constMLayers = mLayers;
  for ( QgsGrassVectorMapLayer *l : constMLayers )
  {
    if ( l->field() == field )
    {
      QgsDebugMsgLevel( "Layer exists", 2 );
      layer = l;
      if ( layer->userCount() == 0 )
      {
        layer->load();
      }
    }
  }

  if ( !layer )
  {
    layer = new QgsGrassVectorMapLayer( this, field );
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
  const auto constMLayers = mLayers;
  for ( QgsGrassVectorMapLayer *l : constMLayers )
  {
    l->load();
  }
}

void QgsGrassVectorMap::closeLayer( QgsGrassVectorMapLayer *layer )
{
  if ( !layer )
  {
    return;
  }

  QgsDebugMsgLevel( QString( "Close layer %1 usersCount = %2" ).arg( toString() ).arg( layer->userCount() ), 2 );

  lockOpenCloseLayer();
  layer->removeUser();

  if ( layer->userCount() == 0 ) // No more users, free sources
  {
    QgsDebugMsgLevel( "No more users -> clear", 2 );
    layer->clear();
  }

  QgsDebugMsgLevel( QString( "%1 map users" ).arg( userCount() ), 2 );
  if ( userCount() == 0 )
  {
    QgsDebugMsgLevel( "No more map users -> close", 2 );
    // Once was probably causing dead lock; move to QgsGrassVectorMapStore?
    close();
  }

  QgsDebugMsgLevel( "layer closed", 2 );
  unlockOpenCloseLayer();
}

void QgsGrassVectorMap::closeMap()
{
  QgsDebugMsgLevel( toString(), 2 );
  QgsGrass::lock();
  if ( !mValid )
  {
    QgsDebugError( "map is not valid" );
  }
  else
  {
    // Mapset must be set before Vect_close()
    QgsGrass::setMapset( mGrassObject.gisdbase(), mGrassObject.location(), mGrassObject.mapset() );

    G_TRY
    {
      Vect_close( mMap );
      QgsDebugMsgLevel( "map closed", 2 );
    }
    G_CATCH( QgsGrass::Exception & e )
    {
      QgsDebugError( "Vect_close failed:" + QString( e.what() ) );
    }
  }
  QgsGrass::vectDestroyMapStruct( mMap );
  mMap = nullptr;
  mOldNumLines = 0;
  mValid = false;
  QgsGrass::unlock();
}

void QgsGrassVectorMap::update()
{
  QgsDebugMsgLevel( toString(), 2 );
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
    if ( !QFileInfo::exists( dp + "/cidx" ) )
    {
      QgsDebugMsgLevel( "The map is being modified and is unavailable : " + mGrassObject.toString(), 2 );
      return false;
    }
    QgsDebugMsgLevel( "The map was modified : " + mGrassObject.toString(), 2 );
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
    QgsDebugMsgLevel( "The attributes of the layer were modified : " + mGrassObject.toString(), 2 );

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
  return mGrassObject.mapsetPath() + "/" + mGrassObject.name();
}

void QgsGrassVectorMap::printDebug()
{
  if ( !mValid || !mMap )
  {
    QgsDebugError( "map not valid" );
    return;
  }
  G_TRY
  {
#ifdef QGISDEBUG
    int ncidx = Vect_cidx_get_num_fields( mMap );
    QgsDebugMsgLevel( QString( "ncidx = %1" ).arg( ncidx ), 2 );

    for ( int i = 0; i < ncidx; i++ )
    {
      int layer = Vect_cidx_get_field_number( mMap, i );
      int ncats = Vect_cidx_get_num_cats_by_index( mMap, i );
      QgsDebugMsgLevel( QString( "i = %1 layer = %2 ncats = %3" ).arg( i ).arg( layer ).arg( ncats ), 2 );
    }
#endif
  }
  G_CATCH( QgsGrass::Exception & e )
  {
    Q_UNUSED( e )
    QgsDebugError( "Cannot read info from map: " + QString( e.what() ) );
  }
}

void QgsGrassVectorMap::lockOpenClose()
{
  QgsDebugMsgLevel( "lockOpenClose", 2 );
  mOpenCloseMutex.lock();
}

void QgsGrassVectorMap::unlockOpenClose()
{
  QgsDebugMsgLevel( "unlockOpenClose", 2 );
  mOpenCloseMutex.unlock();
}

void QgsGrassVectorMap::lockOpenCloseLayer()
{
  QgsDebugMsgLevel( "lockOpenCloseLayer", 2 );
  mOpenCloseLayerMutex.lock();
}

void QgsGrassVectorMap::unlockOpenCloseLayer()
{
  QgsDebugMsgLevel( "unlockOpenCloseLayer", 2 );
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

QgsAbstractGeometry *QgsGrassVectorMap::lineGeometry( int id )
{
  QgsDebugMsgLevel( QString( "id = %1" ).arg( id ), 3 );
  if ( !Vect_line_alive( mMap, id ) ) // should not happen (update mode!)?
  {
    QgsDebugMsgLevel( QString( "line %1 is dead" ).arg( id ), 2 );
    return nullptr;
  }

  struct line_pnts *points = Vect_new_line_struct();

  int type = Vect_read_line( mMap, points, nullptr, id );
  QgsDebugMsgLevel( QString( "type = %1 n_points = %2" ).arg( type ).arg( points->n_points ), 3 );
  if ( points->n_points == 0 )
  {
    Vect_destroy_line_struct( points );
    return nullptr;
  }

  QgsPointSequence pointList;
  pointList.reserve( points->n_points );
  for ( int i = 0; i < points->n_points; i++ )
  {
    pointList << QgsPoint( is3d() ? Qgis::WkbType::PointZ : Qgis::WkbType::Point, points->x[i], points->y[i], points->z[i] );
  }

  Vect_destroy_line_struct( points );

  if ( type & GV_POINTS )
  {
    return pointList.first().clone();
  }
  else if ( type & GV_LINES )
  {
    QgsLineString *line = new QgsLineString();
    line->setPoints( pointList );
    return line;
  }
  else if ( type & GV_FACE )
  {
    QgsPolygon *polygon = new QgsPolygon();
    QgsLineString *ring = new QgsLineString();
    ring->setPoints( pointList );
    polygon->setExteriorRing( ring );
    return polygon;
  }

  QgsDebugError( QString( "unknown type = %1" ).arg( type ) );
  return nullptr;
}

QgsAbstractGeometry *QgsGrassVectorMap::nodeGeometry( int id )
{
  QgsDebugMsgLevel( QString( "id = %1" ).arg( id ), 3 );
  double x, y, z;
  Vect_get_node_coor( mMap, id, &x, &y, &z );
  return new QgsPoint( is3d() ? Qgis::WkbType::PointZ : Qgis::WkbType::Point, x, y, z );
}

QgsAbstractGeometry *QgsGrassVectorMap::areaGeometry( int id )
{
  QgsDebugMsgLevel( QString( "id = %1" ).arg( id ), 3 );
  QgsPolygon *polygon = new QgsPolygon();

  struct line_pnts *points = Vect_new_line_struct();
  QgsDebugMsgLevel( QString( "points= %1" ).arg( ( quint64 ) points ), 3 );
  // Vect_get_area_points and Vect_get_isle_pointsis using static variable -> lock
  // TODO: Faster to lock the whole feature iterator? Maybe only for areas?
  QgsGrass::lock();
  Vect_get_area_points( mMap, id, points );

  QgsPointSequence pointList;
  pointList.reserve( points->n_points );
  for ( int i = 0; i < points->n_points; i++ )
  {
    pointList << QgsPoint( is3d() ? Qgis::WkbType::PointZ : Qgis::WkbType::Point, points->x[i], points->y[i], points->z[i] );
  }

  QgsLineString *ring = new QgsLineString();
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
      pointList << QgsPoint( is3d() ? Qgis::WkbType::PointZ : Qgis::WkbType::Point, points->x[i], points->y[i], points->z[i] );
    }
    ring = new QgsLineString();
    ring->setPoints( pointList );
    polygon->addInteriorRing( ring );
  }
  QgsGrass::unlock();
  Vect_destroy_line_struct( points );
  return polygon;
}

void QgsGrassVectorMap::closeAllIterators()
{
  QgsDebugMsgLevel( toString(), 2 );
  // cancel and close all iterator
  // Iterators must be connected properly, otherwise may it result in dead lock!
  emit cancelIterators(); // non blocking
  emit closeIterators();  // blocking
  QgsDebugMsgLevel( "iterators closed", 2 );
}

//------------------------------------ QgsGrassVectorMapStore ------------------------------------
QgsGrassVectorMapStore *QgsGrassVectorMapStore::sStore = nullptr;

QgsGrassVectorMapStore *QgsGrassVectorMapStore::instance()
{
  static QgsGrassVectorMapStore sInstance;
  if ( sStore )
  {
    return sStore;
  }
  return &sInstance;
}

QgsGrassVectorMap *QgsGrassVectorMapStore::openMap( const QgsGrassObject &grassObject )
{
  QgsDebugMsgLevel( "grassObject = " + grassObject.toString(), 2 );

  mMutex.lock();
  QgsGrassVectorMap *map = nullptr;

  // Check if this map is already open
  const auto constMMaps = mMaps;
  for ( QgsGrassVectorMap *m : constMMaps )
  {
    if ( m->grassObject() == grassObject )
    {
      QgsDebugMsgLevel( "The map already exists", 2 );
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
  int type = Vect_read_line( mMap, nullptr, nullptr, lid );

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
