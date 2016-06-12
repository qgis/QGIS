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
  box.N = rect.yMaximum();
  box.S = rect.yMinimum();
  box.E = rect.xMaximum();
  box.W = rect.xMinimum();
  box.T = PORT_DOUBLE_MAX;
  box.B = -PORT_DOUBLE_MAX;

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
  QgsAbstractGeometryV2 *oldGeometry = 0;
  int cidxFieldIndex = mSource->mLayer->cidxFieldIndex();

#ifdef QGISDEBUG
  if ( mSource->mEditing )
  {
    QgsDebugMsgLevel( "newLids:", 3 );
    Q_FOREACH ( int oldLid, mSource->mLayer->map()->newLids().keys() )
    {
      QgsDebugMsgLevel( QString( "%1 -> %2" ).arg( oldLid ).arg( mSource->mLayer->map()->newLids().value( oldLid ) ), 3 );
    }
  }
#endif

  if ( filterById )
  {
    featureId = mRequest.filterFid();
    lid = lidFromFid( mRequest.filterFid() );
    QgsDebugMsg( QString( "featureId = %1 lid = %2" ).arg( featureId ).arg( lid ) );

    if ( mSource->mEditing )
    {
      // Undo needs the oldes version of geometry, but we also need topo_symbol, so we must read
      // topo_symbol from map if the newLine exists  read
      if ( mSource->mLayer->map()->oldGeometries().contains( lid ) )
      {
        QgsDebugMsg( QString( "use old geometry for lid = %1" ).arg( lid ) );
        oldGeometry = mSource->mLayer->map()->oldGeometries().value( lid );
        if ( !oldGeometry )
        {
          QgsDebugMsg( "oldGeometry is null" );
        }
      }
      if ( mSource->mLayer->map()->newLids().contains( lid ) )
      {
        // newLid may be 0 if line was deleted, in such case use only the old geometry, topo_symbol cannot be read
        int newLid = mSource->mLayer->map()->newLids().value( lid );
        QgsDebugMsg( QString( "use newLid = %1 -> lid = %2" ).arg( newLid ).arg( lid ) );
        lid = newLid;
      }
    }

    if ( lid > 0 )
    {
      if ( !Vect_line_alive( mSource->map(), lid ) )
      {
        close();
        mSource->mLayer->map()->unlockReadWrite();
        return false;
      }
      type = Vect_read_line( mSource->map(), 0, 0, lid );

      // TODO real cat when line/cat was rewritten?!
      cat = catFromFid( mRequest.filterFid() );
      QgsDebugMsg( QString( "lid = %1 cat = %2" ).arg( lid ).arg( cat ) );
    }
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
      QgsDebugMsgLevel( QString( "mNextLid = %1 mNextCidx = %2 numLines() = %3 cidxFieldIndex() = %4 cidxFieldNumCats() = %5" )
                        .arg( mNextLid ).arg( mNextCidx ).arg( mSource->mLayer->map()->numLines() )
                        .arg( mSource->mLayer->cidxFieldIndex() ).arg( mSource->mLayer->cidxFieldNumCats() ), 3 );
      if ( mSource->mEditing )
      {
        // TODO should be numLines before editing started (?), but another layer
        // where editing started later mest have different, because its buffer does not have previous changes
        // -> editing of more layers must be synchronized or not allowed
        if ( mNextLid > mSource->mLayer->map()->numLines() )
        {
          QgsDebugMsgLevel( "mNextLid > numLines()", 3 );
          break;
        }

        int oldLid = mNextLid;
        if ( mSource->mLayer->map()->oldLids().contains( mNextLid ) )
        {
          oldLid = mSource->mLayer->map()->oldLids().value( mNextLid );
          QgsDebugMsg( QString( "mNextLid = %1 -> oldLid = %2" ).arg( mNextLid ).arg( oldLid ) );
        }

        if ( oldLid < 0 )
        {
          QgsDebugMsg( QString( "skip new feature oldLid = %1" ).arg( oldLid ) );
          mNextCidx = 0;
          mNextLid++;
          continue;
        }

        if ( !Vect_line_alive( mSource->map(), mNextLid ) ) // should not be necessary for rewritten lines
        {
          mNextCidx = 0;
          mNextLid++;
          continue;
        }

        struct line_cats *cats = Vect_new_cats_struct();
        int tmpType = Vect_read_line( mSource->map(), 0, cats, mNextLid );
        if ( cats->n_cats == 0 )
        {
          lid = mNextLid;
          type = tmpType;
          cat = 0;
          int layer = 0;
          featureId = makeFeatureId( oldLid, cat, layer );
          mNextCidx = 0;
          mNextLid++;
        }
        else
        {
          if ( mNextCidx >= cats->n_cats )
          {
            mNextCidx = 0;
            mNextLid++;
            Vect_destroy_cats_struct( cats );
            continue;
          }
          else
          {
            lid = mNextLid;
            type = tmpType;
            cat = cats->cat[mNextCidx];
            int layer = cats->field[mNextCidx];
            QgsDebugMsgLevel( QString( "lid = %1 layer = %2 cat = %3" ).arg( lid ).arg( layer ).arg( cat ), 3 );
            featureId = makeFeatureId( oldLid, cat, layer );
            mNextCidx++;
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
        QgsDebugMsgLevel( "standard layer", 3 );
        if ( mNextCidx >= mSource->mLayer->cidxFieldNumCats() )
        {
          break;
        }
        int tmpLid, tmpType, tmpCat;

        int numFields = Vect_cidx_get_num_fields( mSource->map() );
        if ( cidxFieldIndex < 0 || cidxFieldIndex >= numFields )
        {
          QgsDebugMsg( QString( "cidxFieldIndex %1 out of range (0,%2)" ).arg( cidxFieldIndex ).arg( numFields - 1 ) );
          break;
        }
#if 0
        // debug
        Vect_topo_dump( mSource->map(), stderr );
        Vect_cidx_dump( mSource->map(), stderr );
#endif
        Vect_cidx_get_cat_by_index( mSource->map(), cidxFieldIndex, mNextCidx++, &tmpCat, &tmpType, &tmpLid );
        // Warning: selection array is only of type line/area of current layer -> check type first
        if ( !( tmpType & mSource->mGrassType ) )
        {
          QgsDebugMsgLevel( QString( "tmpType = %1 does not match mGrassType = %2" ).arg( tmpType ).arg( mSource->mGrassType ), 3 );
          continue;
        }

        // The 'id' is a unique id of a GRASS geometry object (point, line, area)
        // but it cannot be used as QgsFeatureId because one geometry object may
        // represent more features because it may have more categories.
        lid = tmpLid;
        cat = tmpCat;
        type = tmpType;
        QgsDebugMsgLevel( QString( "lid = %1 field = %2 cat = %3 type= %4" )
                          .arg( lid ).arg( mSource->mLayer->field() ).arg( cat ).arg( type ), 3 );
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
  if ( !oldGeometry )
  {
    int numLinesOrAreas = ( mSource->mGrassType == GV_AREA && !mSource->mEditing ) ?
                          mSource->mLayer->map()->numAreas() : mSource->mLayer->map()->numLines();
    if ( lid == 0 || lid > numLinesOrAreas )
    {
      QgsDebugMsg( QString( "lid = %1 > numLinesOrAreas = %2 -> close" ).arg( lid ).arg( numLinesOrAreas ) );
      close();
      mSource->mLayer->map()->unlockReadWrite();
      return false; // No more features
    }
    if ( type == 0 && mSource->mLayerType != QgsGrassProvider::TOPO_NODE )
    {
      QgsDebugMsg( "unknown type" );
      close();
      mSource->mLayer->map()->unlockReadWrite();
      return false;
    }
  }
  QgsDebugMsgLevel( QString( "lid = %1 type = %2 cat = %3 featureId = %4" ).arg( lid ).arg( type ).arg( cat ).arg( featureId ), 3 );

  feature.setFeatureId( featureId );
  //feature.initAttributes( mSource->mFields.count() );
  QgsDebugMsgLevel( QString( "mSource->mFields.size() = %1" ).arg( mSource->mFields.size() ), 3 );
  feature.setFields( mSource->mFields ); // allow name-based attribute lookups

  if ( !( mRequest.flags() & QgsFeatureRequest::NoGeometry ) )
  {
    if ( oldGeometry )
    {
      feature.setGeometry( new QgsGeometry( oldGeometry->clone() ) );
    }
    else
    {
      setFeatureGeometry( feature, lid, type );
    }
  }

  if ( !QgsGrassProvider::isTopoType( mSource->mLayerType ) )
  {
    QgsGrassVectorMap::TopoSymbol symbol = QgsGrassVectorMap::TopoUndefined;
    if ( mSource->mEditing && lid > 0 )
    {
      symbol = mSource->mLayer->map()->topoSymbol( lid );
    }

    if ( mRequest.flags() & QgsFeatureRequest::SubsetOfAttributes )
      setFeatureAttributes( cat, &feature, mRequest.subsetOfAttributes(), symbol );
    else
      setFeatureAttributes( cat, &feature, symbol );
  }
  else
  {
    feature.initAttributes( mSource->mFields.size() );
    feature.setAttribute( 0, lid );
#if GRASS_VERSION_MAJOR < 7
    if ( mSource->mLayerType == QgsGrassProvider::TOPO_POINT || mSource->mLayerType == QgsGrassProvider::TOPO_LINE )
#else
    /* No more topo points in GRASS 7 */
    if ( mSource->mLayerType == QgsGrassProvider::TOPO_LINE )
#endif
    {
      feature.setAttribute( 1, QgsGrass::vectorTypeName( type ) );

      int node1, node2;
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
        int line = Vect_get_node_line( mSource->map(), lid, i );
        QgsDebugMsg( "cancel" );
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
    geometry = mSource->mLayer->map()->lineGeometry( id );
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

QgsFeatureId QgsGrassFeatureIterator::makeFeatureId( int grassId, int cat, int layer )
{
  // Because GRASS object id and category are both int and QgsFeatureId is qint64
  // we can create unique QgsFeatureId from GRASS id and cat.
  // Max  supported layer number is 92 (max 64bit int is 9,223,372,036,854,775,807).
  QgsFeatureId fid = ( QgsFeatureId )layer * 100000000000000000 + ( QgsFeatureId )grassId * 1000000000 + cat;
  QgsDebugMsgLevel( QString( "grassId = %1 cat = %2 layer = %3 fid = %4" ).arg( grassId ).arg( cat ).arg( layer ).arg( fid ), 3 );
  return fid;
}

int QgsGrassFeatureIterator::layerFromFid( QgsFeatureId fid )
{
  if ( FID_IS_NEW( fid ) )
  {
    return 0;
  }
  return fid / 100000000000000000;
}

int QgsGrassFeatureIterator::lidFromFid( QgsFeatureId fid )
{
  // New features have negative fid, we take such fid as (still negative) lid
  // it is only used for mapping from fid to real lid
  if ( FID_IS_NEW( fid ) )
  {
    return fid;
  }
  qint64 lidCat = fid % 100000000000000000;
  return lidCat / 1000000000;
}

int QgsGrassFeatureIterator::catFromFid( QgsFeatureId fid )
{
  if ( FID_IS_NEW( fid ) )
  {
    // TODO: keep track of cats for new features
    return 0;
  }
  return fid % 1000000000;
}

QVariant QgsGrassFeatureIterator::nonEditableValue( int layerNumber )
{
  if ( layerNumber > 0 )
  {
    return tr( "<not editable (layer %1)>" ).arg( layerNumber );
  }
  else
  {
    // attributes of features without cat (layer = 0) may be edited -> cat created
    return QVariant();
  }
}

void QgsGrassFeatureIterator::setFeatureAttributes( int cat, QgsFeature *feature, QgsGrassVectorMap::TopoSymbol symbol )
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

void QgsGrassFeatureIterator::setFeatureAttributes( int cat, QgsFeature *feature, const QgsAttributeList& attlist, QgsGrassVectorMap::TopoSymbol symbol )
{
  QgsDebugMsgLevel( QString( "setFeatureAttributes cat = %1 symbol = %2" ).arg( cat ).arg( symbol ), 3 );
  feature->initAttributes( mSource->mLayer->fields().size() );

  bool isEditedLayer = true;
  int layerNumber = 0;
  if ( mSource->mEditing )
  {
    layerNumber = layerFromFid( feature->id() );
    if ( layerNumber != mSource->mLayer->field() )
    {
      isEditedLayer = false;
    }
  }

  for ( QgsAttributeList::const_iterator iter = attlist.begin(); iter != attlist.end(); ++iter )
  {
    if ( *iter == mSource->mSymbolAttributeIndex )
    {
      continue;
    }
    QVariant value;
    if ( isEditedLayer )
    {
      value =  mSource->mLayer->attribute( cat, *iter );
      if ( value.type() == QVariant::ByteArray )
      {
        value = QVariant( mSource->mEncoding->toUnicode( value.toByteArray() ) );
      }
    }
    else
    {
      // We are setting cat of different layer in cat column of this layer
      if ( *iter == mSource->mLayer->keyColumn() )
      {
        value = QVariant( cat );
      }
      else if ( layerNumber > 0 )
      {
        value = nonEditableValue( layerNumber );
      }
    }
    QgsDebugMsgLevel( QString( "iter = %1 value = %2" ).arg( *iter ).arg( value.toString() ), 3 );

    feature->setAttribute( *iter, value );
  }


  if ( mSource->mEditing && attlist.contains( mSource->mSymbolAttributeIndex ) )
  {
    QgsDebugMsgLevel( QString( "set attribute %1 to symbol %2" ).arg( mSource->mSymbolAttributeIndex ).arg( symbol ), 3 );
    feature->setAttribute( mSource->mSymbolAttributeIndex, QVariant( symbol ) );
  }
}

//  ------------------ QgsGrassFeatureSource ------------------
QgsGrassFeatureSource::QgsGrassFeatureSource( const QgsGrassProvider* p )
    : mLayer( p->openLayer() )
    , mLayerType( p->mLayerType )
    , mGrassType( p->mGrassType )
    , mQgisType( p->mQgisType )
    , mFields( p->fields() )
    , mEncoding( p->mEncoding )
    , mEditing( p->mEditBuffer )
{
  Q_ASSERT( mLayer );

  mSymbolAttributeIndex = mFields.indexFromName( QgsGrassVectorMap::topoSymbolFieldName() );
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
