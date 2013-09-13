/***************************************************************************
    qgsgrassfeatureiterator.cpp
    ---------------------
    begin                : Juli 2012
    copyright            : (C) 2012 by Martin Dobias
    email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <QObject>

#include "qgsgrassfeatureiterator.h"
#include "qgsgrassprovider.h"

#include "qgsapplication.h"
#include "qgslogger.h"
#include "qgsmessagelog.h"

extern "C"
{
#include <grass/Vect.h>
}

// from provider:
// - mMap, mMaps, mMapVersion
// - mLayers, mLayerId
// - mCidxFieldNumCats, mCidxFieldIndex
// - mGrassType, mQgisType
// - mapOutdated(), updateMap(), update()
// - attributesOutdated(), loadAttributes()
// - setFeatureAttributes()

QgsGrassFeatureIterator::QgsGrassFeatureIterator( QgsGrassProvider* p, const QgsFeatureRequest& request )
    : QgsAbstractFeatureIterator( request ), P( p )
{
  P->mActiveIterators << this;

  // check if outdated and update if necessary
  P->ensureUpdated();

  // Init structures
  mPoints = Vect_new_line_struct();
  mCats = Vect_new_cats_struct();
  mList = Vect_new_list();

  // Create selection array
  allocateSelection( P->mMap );
  resetSelection( 1 );

  if ( request.filterType() == QgsFeatureRequest::FilterRect )
  {
    setSelectionRect( request.filterRect(), request.flags() & QgsFeatureRequest::ExactIntersect );
  }
  else
  {
    // TODO: implement fast lookup by feature id

    //no filter - use all features
    resetSelection( 1 );
  }
}

void QgsGrassFeatureIterator::setSelectionRect( const QgsRectangle& rect, bool useIntersect )
{
  //apply selection rectangle
  resetSelection( 0 );

  BOUND_BOX box;
  box.N = rect.yMaximum(); box.S = rect.yMinimum();
  box.E = rect.xMaximum(); box.W = rect.xMinimum();
  box.T = PORT_DOUBLE_MAX; box.B = -PORT_DOUBLE_MAX;

  if ( !useIntersect )
  { // select by bounding boxes only
    if ( P->mLayerType == QgsGrassProvider::POINT || P->mLayerType == QgsGrassProvider::CENTROID ||
         P->mLayerType == QgsGrassProvider::LINE || P->mLayerType == QgsGrassProvider::FACE ||
         P->mLayerType == QgsGrassProvider::BOUNDARY ||
         P->mLayerType == QgsGrassProvider::TOPO_POINT || P->mLayerType == QgsGrassProvider::TOPO_LINE )
    {
      Vect_select_lines_by_box( P->mMap, &box, P->mGrassType, mList );
    }
    else if ( P->mLayerType == QgsGrassProvider::POLYGON )
    {
      Vect_select_areas_by_box( P->mMap, &box, mList );
    }
    else if ( P->mLayerType == QgsGrassProvider::TOPO_NODE )
    {
      Vect_select_nodes_by_box( P->mMap, &box, mList );
    }
  }
  else
  { // check intersection
    struct line_pnts *Polygon;

    Polygon = Vect_new_line_struct();

    // Using z coor -PORT_DOUBLE_MAX/PORT_DOUBLE_MAX we cover 3D, Vect_select_lines_by_polygon is
    // using dig_line_box to get the box, it is not perfect, Vect_select_lines_by_polygon
    // should clarify better how 2D/3D is treated
    Vect_append_point( Polygon, rect.xMinimum(), rect.yMinimum(), -PORT_DOUBLE_MAX );
    Vect_append_point( Polygon, rect.xMaximum(), rect.yMinimum(), PORT_DOUBLE_MAX );
    Vect_append_point( Polygon, rect.xMaximum(), rect.yMaximum(), 0 );
    Vect_append_point( Polygon, rect.xMinimum(), rect.yMaximum(), 0 );
    Vect_append_point( Polygon, rect.xMinimum(), rect.yMinimum(), 0 );

    if ( P->mLayerType == QgsGrassProvider::POINT || P->mLayerType == QgsGrassProvider::CENTROID ||
         P->mLayerType == QgsGrassProvider::LINE || P->mLayerType == QgsGrassProvider::FACE ||
         P->mLayerType == QgsGrassProvider::BOUNDARY ||
         P->mLayerType == QgsGrassProvider::TOPO_POINT || P->mLayerType == QgsGrassProvider::TOPO_LINE )
    {
      Vect_select_lines_by_polygon( P->mMap, Polygon, 0, NULL, P->mGrassType, mList );
    }
    else if ( P->mLayerType == QgsGrassProvider::POLYGON )
    {
      Vect_select_areas_by_polygon( P->mMap, Polygon, 0, NULL, mList );
    }
    else if ( P->mLayerType == QgsGrassProvider::TOPO_NODE )
    {
      // There is no Vect_select_nodes_by_polygon but for nodes it is the same as by box
      Vect_select_nodes_by_box( P->mMap, &box, mList );
    }

    Vect_destroy_line_struct( Polygon );
  }
  for ( int i = 0; i < mList->n_values; i++ )
  {
    if ( mList->value[i] <= mSelectionSize )
    {
      mSelection[mList->value[i]] = 1;
    }
    else
    {
      QgsDebugMsg( "Selected element out of range" );
    }
  }

}

QgsGrassFeatureIterator::~QgsGrassFeatureIterator()
{
  close();
}

bool QgsGrassFeatureIterator::nextFeature( QgsFeature& feature )
{
  if ( mClosed )
    return false;

  feature.setValid( false );
  int cat = -1, type = -1, id = -1;
  QgsFeatureId featureId = -1;

  QgsDebugMsgLevel( "entered.", 3 );

  if ( P->isEdited() || P->isFrozen() || !P->mValid )
  {
    close();
    return false;
  }

  // TODO: is this necessary? the same is checked below
  if ( !P->isTopoType()  && ( P->mCidxFieldIndex < 0 || mNextCidx >= P->mCidxFieldNumCats ) )
  {
    close();
    return false; // No features, no features in this layer
  }

  bool filterById = mRequest.filterType() == QgsFeatureRequest::FilterFid;

  // Get next line/area id
  int found = 0;
  while ( true )
  {
    QgsDebugMsgLevel( QString( "mNextTopoId = %1" ).arg( mNextTopoId ), 3 );
    if ( P->mLayerType == QgsGrassProvider::TOPO_POINT || P->mLayerType == QgsGrassProvider::TOPO_LINE )
    {
      if ( mNextTopoId > Vect_get_num_lines( P->mMap ) ) break;
      id = mNextTopoId;
      type = Vect_read_line( P->mMap, 0, 0, mNextTopoId++ );
      if ( !( type & P->mGrassType ) ) continue;
      featureId = id;
    }
    else if ( P->mLayerType == QgsGrassProvider::TOPO_NODE )
    {
      if ( mNextTopoId > Vect_get_num_nodes( P->mMap ) ) break;
      id = mNextTopoId;
      type = 0;
      mNextTopoId++;
      featureId = id;
    }
    else
    {
      if ( mNextCidx >= P->mCidxFieldNumCats ) break;

      Vect_cidx_get_cat_by_index( P->mMap, P->mCidxFieldIndex, mNextCidx++, &cat, &type, &id );
      // Warning: selection array is only of type line/area of current layer -> check type first
      if ( !( type & P->mGrassType ) )
        continue;

      // The 'id' is a unique id of a GRASS geometry object (point, line, area)
      // but it cannot be used as QgsFeatureId because one geometry object may
      // represent more features because it may have more categories.
      featureId = makeFeatureId( id, cat );
    }

    if ( filterById && featureId != mRequest.filterFid() )
      continue;

    // it is correct to use id with mSelection because mSelection is only used
    // for geometry selection
    if ( !mSelection[id] )
      continue;

    found = 1;
    break;
  }
  if ( !found )
  {
    close();
    return false; // No more features
  }
  QgsDebugMsgLevel( QString( "cat = %1 type = %2 id = %3 fatureId = %4" ).arg( cat ).arg( type ).arg( id ).arg( featureId ), 3 );

  feature.setFeatureId( featureId );
  feature.initAttributes( P->fields().count() );
  feature.setFields( &P->fields() ); // allow name-based attribute lookups

  if ( mRequest.flags() & QgsFeatureRequest::NoGeometry )
    feature.setGeometry( 0 );
  else
    setFeatureGeometry( feature, id, type );

  if ( ! P->isTopoType() )
  {
    if ( mRequest.flags() & QgsFeatureRequest::SubsetOfAttributes )
      P->setFeatureAttributes( P->mLayerId, cat, &feature, mRequest.subsetOfAttributes() );
    else
      P->setFeatureAttributes( P->mLayerId, cat, &feature );
  }
  else
  {
    feature.setAttribute( 0, id );
    if ( P->mLayerType == QgsGrassProvider::TOPO_POINT || P->mLayerType == QgsGrassProvider::TOPO_LINE )
    {
      feature.setAttribute( 1, P->primitiveTypeName( type ) );

      int node1, node2;
      Vect_get_line_nodes( P->mMap, id, &node1, &node2 );
      feature.setAttribute( 2, node1 );
      if ( P->mLayerType == QgsGrassProvider::TOPO_LINE )
      {
        feature.setAttribute( 3, node2 );
      }
    }

    if ( P->mLayerType == QgsGrassProvider::TOPO_LINE )
    {
      if ( type == GV_BOUNDARY )
      {
        int left, right;
        Vect_get_line_areas( P->mMap, id, &left, &right );
        feature.setAttribute( 4, left );
        feature.setAttribute( 5, right );
      }
    }
    else if ( P->mLayerType == QgsGrassProvider::TOPO_NODE )
    {
      QString lines;
      int nlines = Vect_get_node_n_lines( P->mMap, id );
      for ( int i = 0; i < nlines; i++ )
      {
        int line = Vect_get_node_line( P->mMap, id, i );
        if ( i > 0 ) lines += ",";
        lines += QString::number( line );
      }
      feature.setAttribute( 1, lines );
    }
  }

  feature.setValid( true );

  return true;
}




bool QgsGrassFeatureIterator::rewind()
{
  if ( mClosed )
    return false;

  if ( P->isEdited() || P->isFrozen() || !P->mValid )
    return false;

  // not sure if this really should be here
  P->ensureUpdated();

  mNextCidx = 0;
  mNextTopoId = 1;

  return true;
}

bool QgsGrassFeatureIterator::close()
{
  if ( mClosed )
    return false;

  P->mActiveIterators.remove( this );

  // finalization
  Vect_destroy_line_struct( mPoints );
  Vect_destroy_cats_struct( mCats );
  Vect_destroy_list( mList );

  free( mSelection );

  mClosed = true;
  return true;
}


//////////////////


void QgsGrassFeatureIterator::resetSelection( bool sel )
{
  QgsDebugMsg( "entered." );
  memset( mSelection, ( int ) sel, mSelectionSize );
  mNextCidx = 0;
  mNextTopoId = 1;
}


void QgsGrassFeatureIterator::allocateSelection( struct Map_info *map )
{
  int size;
  QgsDebugMsg( "entered." );

  int nlines = Vect_get_num_lines( map );
  int nareas = Vect_get_num_areas( map );

  if ( nlines > nareas )
  {
    size = nlines + 1;
  }
  else
  {
    size = nareas + 1;
  }
  QgsDebugMsg( QString( "nlines = %1 nareas = %2 size = %3" ).arg( nlines ).arg( nareas ).arg( size ) );

  mSelection = ( char * ) malloc( size );
  mSelectionSize = size;
}



void QgsGrassFeatureIterator::setFeatureGeometry( QgsFeature& feature, int id, int type )
{
  unsigned char *wkb;
  int wkbsize;

  // TODO int may be 64 bits (memcpy)
  if ( type & ( GV_POINTS | GV_LINES | GV_FACE ) || P->mLayerType == QgsGrassProvider::TOPO_NODE ) /* points or lines */
  {
    if ( P->mLayerType == QgsGrassProvider::TOPO_NODE )
    {
      double x, y, z;
      Vect_get_node_coor( P->mMap, id, &x, &y, &z );
      Vect_reset_line( mPoints );
      Vect_append_point( mPoints, x, y, z );
    }
    else
    {
      Vect_read_line( P->mMap, mPoints, 0, id );
    }
    int npoints = mPoints->n_points;

    if ( P->mLayerType == QgsGrassProvider::TOPO_NODE )
    {
      wkbsize = 1 + 4 + 2 * 8;
    }
    else if ( type & GV_POINTS )
    {
      wkbsize = 1 + 4 + 2 * 8;
    }
    else if ( type & GV_LINES )
    {
      wkbsize = 1 + 4 + 4 + npoints * 2 * 8;
    }
    else // GV_FACE
    {
      wkbsize = 1 + 4 + 4 + 4 + npoints * 2 * 8;
    }
    wkb = new unsigned char[wkbsize];
    unsigned char *wkbp = wkb;
    wkbp[0] = ( unsigned char ) QgsApplication::endian();
    wkbp += 1;

    /* WKB type */
    memcpy( wkbp, &P->mQgisType, 4 );
    wkbp += 4;

    /* Number of rings */
    if ( type & GV_FACE )
    {
      int nrings = 1;
      memcpy( wkbp, &nrings, 4 );
      wkbp += 4;
    }

    /* number of points */
    if ( type & ( GV_LINES | GV_FACE ) )
    {
      QgsDebugMsg( QString( "set npoints = %1" ).arg( npoints ) );
      memcpy( wkbp, &npoints, 4 );
      wkbp += 4;
    }

    for ( int i = 0; i < npoints; i++ )
    {
      memcpy( wkbp, &( mPoints->x[i] ), 8 );
      memcpy( wkbp + 8, &( mPoints->y[i] ), 8 );
      wkbp += 16;
    }
  }
  else   // GV_AREA
  {
    Vect_get_area_points( P->mMap, id, mPoints );
    int npoints = mPoints->n_points;

    wkbsize = 1 + 4 + 4 + 4 + npoints * 2 * 8; // size without islands
    wkb = new unsigned char[wkbsize];
    wkb[0] = ( unsigned char ) QgsApplication::endian();
    int offset = 1;

    /* WKB type */
    memcpy( wkb + offset, &P->mQgisType, 4 );
    offset += 4;

    /* Number of rings */
    int nisles = Vect_get_area_num_isles( P->mMap, id );
    int nrings = 1 + nisles;
    memcpy( wkb + offset, &nrings, 4 );
    offset += 4;

    /* Outer ring */
    memcpy( wkb + offset, &npoints, 4 );
    offset += 4;
    for ( int i = 0; i < npoints; i++ )
    {
      memcpy( wkb + offset, &( mPoints->x[i] ), 8 );
      memcpy( wkb + offset + 8, &( mPoints->y[i] ), 8 );
      offset += 16;
    }

    /* Isles */
    for ( int i = 0; i < nisles; i++ )
    {
      Vect_get_isle_points( P->mMap, Vect_get_area_isle( P->mMap, id, i ), mPoints );
      npoints = mPoints->n_points;

      // add space
      wkbsize += 4 + npoints * 2 * 8;
      wkb = ( unsigned char * ) realloc( wkb, wkbsize );

      memcpy( wkb + offset, &npoints, 4 );
      offset += 4;
      for ( int i = 0; i < npoints; i++ )
      {
        memcpy( wkb + offset, &( mPoints->x[i] ), 8 );
        memcpy( wkb + offset + 8, &( mPoints->y[i] ), 8 );
        offset += 16;
      }
    }
  }

  feature.setGeometryAndOwnership( wkb, wkbsize );
}

QgsFeatureId QgsGrassFeatureIterator::makeFeatureId( int grassId, int cat )
{
  // Because GRASS object id and category are both int and QgsFeatureId is qint64
  // we can create unique QgsFeatureId from GRASS id and cat
  return ( QgsFeatureId )grassId * 1000000000 + cat;
}
