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
#include <QTextCodec>

#include "qgsgrass.h"
#include "qgsgrassfeatureiterator.h"
#include "qgsgrassprovider.h"
#include "qgsgrassvectormap.h"

#include "qgsapplication.h"
#include "qgsgeometry.h"
#include "qgslogger.h"
#include "qgsmessagelog.h"

extern "C"
{
#include <grass/version.h>

#if GRASS_VERSION_MAJOR < 7
#include <grass/Vect.h>
#else
#include <grass/vector.h>
#define BOUND_BOX bound_box
#endif
}

#if GRASS_VERSION_MAJOR < 7
#else

void copy_boxlist_and_destroy( struct boxlist *blist, struct ilist * list )
{
  Vect_reset_list( list );
  for ( int i = 0; i < blist->n_values; i++ )
  {
    Vect_list_append( list, blist->id[i] );
  }
  Vect_destroy_boxlist( blist );
}

#define Vect_select_lines_by_box(map, box, type, list) \
  { \
    struct boxlist *blist = Vect_new_boxlist(0);\
    Vect_select_lines_by_box( (map), (box), (type), blist); \
    copy_boxlist_and_destroy( blist, (list) );\
  }
#define Vect_select_areas_by_box(map, box, list) \
  { \
    struct boxlist *blist = Vect_new_boxlist(0);\
    Vect_select_areas_by_box( (map), (box), blist); \
    copy_boxlist_and_destroy( blist, (list) );\
  }
#endif

//QMutex QgsGrassFeatureIterator::sMutex;

QgsGrassFeatureIterator::QgsGrassFeatureIterator( QgsGrassFeatureSource* source, bool ownSource, const QgsFeatureRequest& request )
    : QgsAbstractFeatureIteratorFromSource<QgsGrassFeatureSource>( source, ownSource, request )
    , mCanceled( false )
    , mNextCidx( 0 )
    , mNextLid( 1 )
{
  QgsDebugMsg( "entered" );

  // WARNING: the iterater cannot use mutex lock for its whole life, because QgsVectorLayerFeatureIterator is opening
  // multiple iterators if features are edited -> lock only critical sections

  // Create selection
  int size = 1 + qMax( Vect_get_num_lines( mSource->map() ), Vect_get_num_areas( mSource->map() ) );
  QgsDebugMsg( QString( "mSelection.resize(%1)" ).arg( size ) );
  mSelection.resize( size );

  if ( !request.filterRect().isNull() )
  {
    setSelectionRect( request.filterRect(), request.flags() & QgsFeatureRequest::ExactIntersect );
  }
  else
  {
    //no filter - use all features
    mSelection.fill( true );
  }

  connect( mSource->mLayer->map(), SIGNAL( cancelIterators() ), this, SLOT( cancel() ), Qt::DirectConnection );

  Qt::ConnectionType connectionType = Qt::DirectConnection;
  if ( mSource->mLayer->map()->thread() != thread() )
  {
    // Using BlockingQueuedConnection may be dangerous, if the iterator was later moved to maps thread, it would cause dead lock on emit closeIterators()
    QgsDebugMsg( "map and iterator are on different threads -> connect closeIterators() with BlockingQueuedConnection" );
    connectionType = Qt::BlockingQueuedConnection;
  }
  connect( mSource->mLayer->map(), SIGNAL( closeIterators() ), this, SLOT( doClose() ), connectionType );
}

QgsGrassFeatureIterator::~QgsGrassFeatureIterator()
{
  close();
}

void QgsGrassFeatureIterator::cancel()
{
  QgsDebugMsg( "cancel" );
  mCanceled = true;
}

void QgsGrassFeatureIterator::doClose()
{
  QgsDebugMsg( "doClose" );
  close();
}

void QgsGrassFeatureIterator::setSelectionRect( const QgsRectangle& rect, bool useIntersect )
{
  QgsDebugMsg( QString( "useIntersect = %1 rect = %2" ).arg( useIntersect ).arg( rect.toString() ) );

  // TODO: selection of edited lines

  // Lock because functions using static/global variables are used
  // (e.g. static LocList in Vect_select_lines_by_box, global BranchBuf in RTreeGetBranches)
  QgsGrass::lock();

  mSelection.fill( false );

  BOUND_BOX box;
  box.N = rect.yMaximum(); box.S = rect.yMinimum();
  box.E = rect.xMaximum(); box.W = rect.xMinimum();
  box.T = PORT_DOUBLE_MAX; box.B = -PORT_DOUBLE_MAX;

  // Init structures
  struct ilist * list = Vect_new_list();

  if ( !useIntersect )
  { // select by bounding boxes only
    if ( mSource->mLayerType == QgsGrassProvider::POINT || mSource->mLayerType == QgsGrassProvider::CENTROID ||
         mSource->mLayerType == QgsGrassProvider::LINE || mSource->mLayerType == QgsGrassProvider::FACE ||
         mSource->mLayerType == QgsGrassProvider::BOUNDARY ||
         mSource->mLayerType == QgsGrassProvider::TOPO_POINT || mSource->mLayerType == QgsGrassProvider::TOPO_LINE ||
         mSource->mEditing )
    {
      QgsDebugMsg( "Vect_select_lines_by_box" );
      int type = mSource->mGrassType;
      if ( mSource->mEditing )
      {
        type = GV_POINTS | GV_LINES;
      }
      QgsDebugMsg( QString( "type = %1" ).arg( type ) );
      Vect_select_lines_by_box( mSource->map(), &box, type, list );
    }
    else if ( mSource->mLayerType == QgsGrassProvider::POLYGON )
    {
      Vect_select_areas_by_box( mSource->map(), &box, list );
    }
    else if ( mSource->mLayerType == QgsGrassProvider::TOPO_NODE )
    {
      Vect_select_nodes_by_box( mSource->map(), &box, list );
    }
  }
  else
  { // check intersection
    struct line_pnts *polygon = Vect_new_line_struct();

    // Using z coor -PORT_DOUBLE_MAX/PORT_DOUBLE_MAX we cover 3D, Vect_select_lines_by_polygon is
    // using dig_line_box to get the box, it is not perfect, Vect_select_lines_by_polygon
    // should clarify better how 2D/3D is treated
    Vect_append_point( polygon, rect.xMinimum(), rect.yMinimum(), -PORT_DOUBLE_MAX );
    Vect_append_point( polygon, rect.xMaximum(), rect.yMinimum(), PORT_DOUBLE_MAX );
    Vect_append_point( polygon, rect.xMaximum(), rect.yMaximum(), 0 );
    Vect_append_point( polygon, rect.xMinimum(), rect.yMaximum(), 0 );
    Vect_append_point( polygon, rect.xMinimum(), rect.yMinimum(), 0 );

    if ( mSource->mLayerType == QgsGrassProvider::POINT || mSource->mLayerType == QgsGrassProvider::CENTROID ||
         mSource->mLayerType == QgsGrassProvider::LINE || mSource->mLayerType == QgsGrassProvider::FACE ||
         mSource->mLayerType == QgsGrassProvider::BOUNDARY ||
         mSource->mLayerType == QgsGrassProvider::TOPO_POINT || mSource->mLayerType == QgsGrassProvider::TOPO_LINE ||
         mSource->mEditing )
    {
      QgsDebugMsg( "Vect_select_lines_by_polygon" );
      int type = mSource->mGrassType;
      if ( mSource->mEditing )
      {
        type = GV_POINTS | GV_LINES;
      }
      QgsDebugMsg( QString( "type = %1" ).arg( type ) );
      Vect_select_lines_by_polygon( mSource->map(), polygon, 0, NULL, type, list );
    }
    else if ( mSource->mLayerType == QgsGrassProvider::POLYGON )
    {
      Vect_select_areas_by_polygon( mSource->map(), polygon, 0, NULL, list );
    }
    else if ( mSource->mLayerType == QgsGrassProvider::TOPO_NODE )
    {
      // There is no Vect_select_nodes_by_polygon but for nodes it is the same as by box
      Vect_select_nodes_by_box( mSource->map(), &box, list );
    }

    Vect_destroy_line_struct( polygon );
  }
  for ( int i = 0; i < list->n_values; i++ )
  {
    int lid = list->value[i];
    if ( lid < 1 || lid >= mSelection.size() ) // should not happen
    {
      QgsDebugMsg( QString( "lid %1 out of range <1,%2>" ).arg( lid ).arg( mSelection.size() ) );
      continue;
    }
    mSelection.setBit( lid );
  }
  Vect_destroy_list( list );

  QgsDebugMsg( QString( " %1 features selected" ).arg( list->n_values ) );
  QgsGrass::unlock();
}

bool QgsGrassFeatureIterator::fetchFeature( QgsFeature& feature )
{
  QgsDebugMsgLevel( "entered", 3 );
  if ( mClosed )
  {
    return false;
  }
  if ( mCanceled )
  {
    QgsDebugMsg( "iterator is canceled -> close" );
    close();
    return false;
  }

  feature.setValid( false );

  // TODO: locking each feature is too expensive. What happens with map structures if lines are
  // written/rewritten/deleted? Can be read simultaneously?
  mSource->mLayer->map()->lockReadWrite(); // locks only in editing mode
  bool filterById = mRequest.filterType() == QgsFeatureRequest::FilterFid;
  int cat = 0;
  int type = 0;
  int lid = 0;
  QgsFeatureId featureId = 0;

  if ( mSource->mEditing )
  {
    QgsDebugMsg( "newLids:" );
    foreach ( int oldLid, mSource->mLayer->map()->newLids().keys() )
    {
      QgsDebugMsg( QString( "%1 -> %2" ).arg( oldLid ).arg( mSource->mLayer->map()->newLids().value( oldLid ) ) );
    }
  }

  if ( filterById )
  {
    featureId = mRequest.filterFid();
    lid = lidFormFid( mRequest.filterFid() );

    if ( mSource->mLayer->map()->newLids().contains( lid ) )
    {
      lid = mSource->mLayer->map()->newLids().value( lid );
      QgsDebugMsg( QString( "line %1 rewritten -> real lid = %2" ).arg( lidFormFid( mRequest.filterFid() ) ).arg( lid ) );
    }
    if ( !Vect_line_alive( mSource->map(), lid ) )
    {
      close();
      mSource->mLayer->map()->unlockReadWrite();
      return false;
    }
    type = Vect_read_line( mSource->map(), 0, 0, lid );

    // TODO real cat when line/cat was rewritten?!
    cat = catFormFid( mRequest.filterFid() );
    QgsDebugMsg( QString( "lid = %1 cat = %2" ).arg( lid ).arg( cat ) );
  }
  else
  {
    // Get next line/area id
    while ( true )
    {
      // TODO: if selection is used, go only through the list of selected values
      cat = 0;
      type = 0;
      lid = 0;
      QgsDebugMsgLevel( QString( "mNextLid = %1 mNextCidx = %2 numLines() = %3" ).arg( mNextLid ).arg( mNextCidx ).arg( mSource->mLayer->map()->numLines() ), 3 );
      if ( mSource->mEditing )
      {
        // TODO should be numLines before editing started (?), but another layer
        // where editing started later mest have different, because its buffer does not have previous changes
        // -> editing of more layers must be synchronized or not allowed
        //if ( mNextLid > mSource->mLayer->map()->numOldLines() )
        if ( mNextLid > mSource->mLayer->map()->numLines() )
        {
          QgsDebugMsgLevel( "mNextLid > numLines()", 3 );
          break;
        }

        int realLid = mNextLid;
        if ( mSource->mLayer->map()->newLids().contains( mNextLid ) )
        {
          realLid = mSource->mLayer->map()->newLids().value( mNextLid );
          QgsDebugMsg( QString( "line %1 rewritten ->  realLid = %2" ).arg( mNextLid ).arg( realLid ) );
        }

        if ( !Vect_line_alive( mSource->map(), realLid ) ) // should not be necessary for rewritten lines
        {
          mNextLid++;
          continue;
        }

        struct line_cats *cats = Vect_new_cats_struct();
        int tmpType = Vect_read_line( mSource->map(), 0, cats, realLid );
        if ( cats->n_cats == 0 )
        {
          lid = realLid;
          type = tmpType;
          cat = 0;
          featureId = makeFeatureId( mNextLid, cat );
          mNextLid++;
        }
        else
        {
          if ( mNextCidx >= cats->n_cats )
          {
            mNextCidx = 0;
            mNextLid++;
            continue;
          }
          else
          {
            // Show only cats of currently edited layer
            if ( cats->field[mNextCidx] != mSource->mLayer->field() )
            {
              mNextCidx++;
              continue;
            }
            else
            {
              lid = realLid;
              type = tmpType;
              cat = cats->cat[mNextCidx];
              featureId = makeFeatureId( mNextLid, cat );
              mNextCidx++;
            }
          }
        }
        Vect_destroy_cats_struct( cats );
      }
      else if ( mSource->mLayerType == QgsGrassProvider::TOPO_POINT || mSource->mLayerType == QgsGrassProvider::TOPO_LINE )
      {
        if ( mNextLid > Vect_get_num_lines( mSource->map() ) )
        {
          break;
        }
        lid = mNextLid;
        type = Vect_read_line( mSource->map(), 0, 0, mNextLid++ );
        if ( !( type & mSource->mGrassType ) )
        {
          continue;
        }
        featureId = lid;
      }
      else if ( mSource->mLayerType == QgsGrassProvider::TOPO_NODE )
      {
        if ( mNextLid > Vect_get_num_nodes( mSource->map() ) )
        {
          break;
        }
        lid = mNextLid;
        type = 0;
        mNextLid++;
        featureId = lid;
      }
      else // standard layer
      {
        if ( mNextCidx >= mSource->mCidxFieldNumCats )
        {
          break;
        }
        int tmpLid, tmpType, tmpCat;
        Vect_cidx_get_cat_by_index( mSource->map(), mSource->mCidxFieldIndex, mNextCidx++, &tmpCat, &tmpType, &tmpLid );
        // Warning: selection array is only of type line/area of current layer -> check type first
        if ( !( tmpType & mSource->mGrassType ) )
        {
          continue;
        }

        // The 'id' is a unique id of a GRASS geometry object (point, line, area)
        // but it cannot be used as QgsFeatureId because one geometry object may
        // represent more features because it may have more categories.
        lid = tmpLid;
        cat = tmpCat;
        type = tmpType;
        featureId = makeFeatureId( lid, cat );
      }

      // TODO: fix selection for mEditing
      //if ( !mSource->mEditing && !mSelection[id] )
      if ( lid < 1 || lid >= mSelection.size() || !mSelection[lid] )
      {
        QgsDebugMsgLevel( QString( "lid = %1 not in selection" ).arg( lid ), 3 );
        continue;
      }
      else
      {
        QgsDebugMsgLevel( QString( "lid = %1 in selection" ).arg( lid ), 3 );
      }
      break;
    }
  }
  if ( lid == 0 || lid > mSource->mLayer->map()->numLines() )
  {
    QgsDebugMsg( QString( "lid = %1 -> close" ).arg( lid ) );
    close();
    mSource->mLayer->map()->unlockReadWrite();
    return false; // No more features
  }
  if ( type == 0 ) // should not happen
  {
    QgsDebugMsg( "unknown type" );
    close();
    mSource->mLayer->map()->unlockReadWrite();
    return false;
  }
  QgsDebugMsgLevel( QString( "lid = %1 type = %2 cat = %3 fatureId = %4" ).arg( lid ).arg( type ).arg( cat ).arg( featureId ), 3 );

  feature.setFeatureId( featureId );
  //feature.initAttributes( mSource->mFields.count() );
  QgsDebugMsgLevel( QString( "mSource->mFields.size() = %1" ).arg( mSource->mFields.size() ), 3 );
  feature.setFields( mSource->mFields ); // allow name-based attribute lookups

  if ( !( mRequest.flags() & QgsFeatureRequest::NoGeometry ) )
  {
    // TODO ???
#if 0
    // Changed geometry are always read from cache
    if ( mSource->mEditing && mSource->mChangedFeatures.contains( mRequest.filterFid() ) )
    {
      QgsDebugMsg( QString( "filterById = %1 mRequest.filterFid() = %2 mSource->mChangedFeatures.size() = %3" ).arg( filterById ).arg( mRequest.filterFid() ).arg( mSource->mChangedFeatures.size() ) );
      QgsFeature f = mSource->mChangedFeatures.value( mRequest.filterFid() );
      QgsDebugMsg( QString( "return features from mChangedFeatures id = %1" ).arg( f.id() ) );
      feature.setFeatureId( f.id() );
      feature.initAttributes( mSource->mFields.count() );
      feature.setFields( &( mSource->mFields ) ); // allow name-based attribute lookups
      feature.setAttributes( f.attributes() );
      feature.setGeometry( new QgsGeometry( *( f.geometry() ) ) );
      feature.setValid( true );
      mSource->mLayer->map()->unlockReadWrite();
      return true;
    }
    else
    {
    }
#endif
    setFeatureGeometry( feature, lid, type );
  }

  if ( !QgsGrassProvider::isTopoType( mSource->mLayerType ) )
  {
    QgsGrassProvider::TopoSymbol symbol = QgsGrassProvider::TopoUndefined;
    if ( mSource->mEditing )
    {
      symbol = topoSymbol( lid, type );
    }

    if ( mRequest.flags() & QgsFeatureRequest::SubsetOfAttributes )
      setFeatureAttributes( cat, &feature, mRequest.subsetOfAttributes(), symbol );
    else
      setFeatureAttributes( cat, &feature, symbol );
  }
  else
  {
    feature.setAttribute( 0, lid );
#if GRASS_VERSION_MAJOR < 7
    if ( mSource->mLayerType == QgsGrassProvider::TOPO_POINT || mSource->mLayerType == QgsGrassProvider::TOPO_LINE )
#else
    /* No more topo points in GRASS 7 */
    if ( mSource->mLayerType == QgsGrassProvider::TOPO_LINE )
#endif
    {
      feature.setAttribute( 1, QgsGrass::vectorTypeName( type ) );

      int node1, node2;;
      close();
      Vect_get_line_nodes( mSource->map(), lid, &node1, &node2 );
      feature.setAttribute( 2, node1 );
      if ( mSource->mLayerType == QgsGrassProvider::TOPO_LINE )
      {
        feature.setAttribute( 3, node2 );
      }
    }

    if ( mSource->mLayerType == QgsGrassProvider::TOPO_LINE )
    {
      if ( type == GV_BOUNDARY )
      {
        int left, right;
        Vect_get_line_areas( mSource->map(), lid, &left, &right );
        feature.setAttribute( 4, left );
        feature.setAttribute( 5, right );
      }
    }
    else if ( mSource->mLayerType == QgsGrassProvider::TOPO_NODE )
    {
      QString lines;
      int nlines = Vect_get_node_n_lines( mSource->map(), lid );
      for ( int i = 0; i < nlines; i++ )
      {
        int line = Vect_get_node_line( mSource->map(), lid, i );  QgsDebugMsg( "cancel" );
        if ( i > 0 ) lines += ",";
        lines += QString::number( line );
      }
      feature.setAttribute( 1, lines );
    }
  }
  feature.setValid( true );
  mSource->mLayer->map()->unlockReadWrite();

  return true;
}




bool QgsGrassFeatureIterator::rewind()
{
  if ( mClosed )
  {
    QgsDebugMsg( "closed" );
    return false;
  }

  mNextCidx = 0;
  mNextLid = 1;

  return true;
}

bool QgsGrassFeatureIterator::close()
{
  QgsDebugMsg( "entered" );
  if ( mClosed )
  {
    QgsDebugMsg( "already closed" );
    return false;
  }

  iteratorClosed();

  mClosed = true;
  QgsDebugMsg( "closed" );
  //sMutex.unlock();
  return true;
}

void QgsGrassFeatureIterator::setFeatureGeometry( QgsFeature& feature, int id, int type )
{
  QgsDebugMsgLevel( QString( "id = %1 type = %2" ).arg( id ).arg( type ), 3 );

  QgsAbstractGeometryV2 *geometry = 0;
  if ( type & ( GV_POINTS | GV_LINES | GV_FACE ) )
  {
    if ( mSource->mEditing )
    {
      // Use original geometry because QgsVectorLayerUndoCommandChangeGeometry::undo() needs that
      if ( mSource->mLayer->map()->oldLids().contains( id ) )
      {
        int oldLid = mSource->mLayer->map()->oldLids().value( id );
        QgsDebugMsg( QString( "oldLid = %1 -> use old geometry" ).arg( oldLid ) );
        if ( mSource->mLayer->map()->oldGeometries().contains( oldLid ) )
        {
          geometry = mSource->mLayer->map()->oldGeometries().value( oldLid )->clone();
        }
        else
        {
          QgsDebugMsg( "geometry not found in oldGeometries" );
        }
      }
    }
    if ( !geometry )
    {
      geometry = mSource->mLayer->map()->lineGeometry( id );
    }
  }
  else if ( mSource->mLayerType == QgsGrassProvider::TOPO_NODE )
  {
    geometry = mSource->mLayer->map()->nodeGeometry( id );
  }
  else if ( type == GV_AREA )
  {
    geometry = mSource->mLayer->map()->areaGeometry( id );
  }
  else
  {
    QgsDebugMsg( QString( "unknown type = %1" ).arg( type ) );
  }
  feature.setGeometry( new QgsGeometry( geometry ) );
}

QgsFeatureId QgsGrassFeatureIterator::makeFeatureId( int grassId, int cat )
{
  // Because GRASS object id and category are both int and QgsFeatureId is qint64
  // we can create unique QgsFeatureId from GRASS id and cat
  return ( QgsFeatureId )grassId * 1000000000 + cat;
}

int QgsGrassFeatureIterator::lidFormFid( QgsFeatureId fid )
{
  return fid / 1000000000;
}

int QgsGrassFeatureIterator::catFormFid( QgsFeatureId fid )
{
  return fid % 1000000000;
}

QgsGrassProvider::TopoSymbol QgsGrassFeatureIterator::topoSymbol( int lid, int type )
{
  QgsGrassProvider::TopoSymbol symbol = QgsGrassProvider::TopoUndefined;
  if ( type == GV_POINT )
  {
    symbol = QgsGrassProvider::TopoPoint;
  }
  else if ( type == GV_CENTROID )
  {
    int area = Vect_get_centroid_area( mSource->map(), lid );
    if ( area == 0 )
      symbol = QgsGrassProvider::TopoCentroidOut;
    else if ( area > 0 )
      symbol = QgsGrassProvider::TopoCentroidIn;
    else
      symbol = QgsGrassProvider::TopoCentroidDupl; /* area < 0 */
  }
  else if ( type == GV_LINE )
  {
    symbol = QgsGrassProvider::TopoLine;
  }
  else if ( type == GV_BOUNDARY )
  {
    int left, right;
    Vect_get_line_areas( mSource->map(), lid, &left, &right );
    if ( left != 0 && right != 0 )
    {
      symbol = QgsGrassProvider::TopoBoundary2;
    }
    else if ( left == 0 && right == 0 )
    {
      symbol = QgsGrassProvider::TopoBoundary0;
    }
    else
    {
      symbol = QgsGrassProvider::TopoBoundary1;
    }
  }
  QgsDebugMsgLevel( QString( "lid = %1 type = %2 symbol = %3" ).arg( lid ).arg( type ).arg( symbol ), 3 );
  return symbol;
}

void QgsGrassFeatureIterator::setFeatureAttributes( int cat, QgsFeature *feature, QgsGrassProvider::TopoSymbol symbol )
{
  QgsDebugMsgLevel( QString( "setFeatureAttributes cat = %1" ).arg( cat ), 3 );
  QgsAttributeList attlist;
  int nFields =  mSource->mLayer->fields().size();
  if ( nFields > 0 )
  {
    for ( int i = 0; i <  mSource->mLayer->fields().size(); i++ )
    {
      attlist << i;
    }
  }
  else
  {
    attlist << 0;
  }
  return setFeatureAttributes( cat, feature, attlist, symbol );
}

void QgsGrassFeatureIterator::setFeatureAttributes( int cat, QgsFeature *feature, const QgsAttributeList& attlist, QgsGrassProvider::TopoSymbol symbol )
{
  QgsDebugMsgLevel( QString( "setFeatureAttributes cat = %1 symbol = %2" ).arg( cat ).arg( symbol ), 3 );
  int nFields = mSource->mLayer->fields().size();
  int nAttributes = nFields;
  if ( mSource->mEditing )
  {
    //nAttributes += 1;
  }
  feature->initAttributes( nAttributes );
  if ( mSource->mLayer->hasTable() )
  {
    for ( QgsAttributeList::const_iterator iter = attlist.begin(); iter != attlist.end(); ++iter )
    {
      if ( !mSource->mLayer->attributes().contains( cat ) )
      {
        QgsDebugMsgLevel( QString( "cat %1 not found in attributes" ).arg( cat ), 3 );
      }
      QVariant value = mSource->mLayer->attributes().value( cat ).value( *iter );
      if ( value.type() == QVariant::ByteArray )
      {
        value = QVariant( mSource->mEncoding->toUnicode( value.toByteArray() ) );
      }
      QgsDebugMsgLevel( QString( "iter = %1 value = %2" ).arg( *iter ).arg( value.toString() ), 3 );
      feature->setAttribute( *iter, value );
    }
  }
  else if ( attlist.contains( 0 ) ) // no table and first attribute requested -> add cat
  {
    QgsDebugMsgLevel( QString( "no table, set attribute 0 to cat %1" ).arg( cat ), 3 );
    feature->setAttribute( 0, QVariant( cat ) );
  }
  else
  {
    QgsDebugMsgLevel( "no table, cat not requested", 3 );
  }
  if ( mSource->mEditing )
  {
    // append topo_symbol
    int idx = nAttributes - 1;
    QgsDebugMsgLevel( QString( "set attribute %1 to symbol %2" ).arg( idx ).arg( symbol ), 3 );
    //feature->setAttribute( 0, QVariant( symbol ) ); // debug
    feature->setAttribute( idx, QVariant( symbol ) );
  }
}

//  ------------------ QgsGrassFeatureSource ------------------
QgsGrassFeatureSource::QgsGrassFeatureSource( const QgsGrassProvider* p )
    : mLayer( p->openLayer() )
    , mLayerType( p->mLayerType )
    , mGrassType( p->mGrassType )
    , mQgisType( p->mQgisType )
    , mCidxFieldIndex( p->mCidxFieldIndex )
    , mCidxFieldNumCats( p->mCidxFieldNumCats )
    , mFields( p->fields() )
    , mEncoding( p->mEncoding )
    , mEditing( p->mEditBuffer )
{

  Q_ASSERT( mLayer );
#if 0
  if ( mEditing )
  {
    mFields.clear();
    mFields.append( QgsField( "topo_symbol", QVariant::Int, "int" ) );
  }
#endif
}

QgsGrassFeatureSource::~QgsGrassFeatureSource()
{
  mLayer->close();
}

QgsFeatureIterator QgsGrassFeatureSource::getFeatures( const QgsFeatureRequest& request )
{
  QgsDebugMsg( "QgsGrassFeatureSource::getFeatures" );
  return QgsFeatureIterator( new QgsGrassFeatureIterator( this, false, request ) );
}

struct Map_info* QgsGrassFeatureSource::map()
{
  return  mLayer->map()->map();
}
