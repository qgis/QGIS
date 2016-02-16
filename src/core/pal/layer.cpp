/*
 *   libpal - Automated Placement of Labels Library
 *
 *   Copyright (C) 2008 Maxence Laurent, MIS-TIC, HEIG-VD
 *                      University of Applied Sciences, Western Switzerland
 *                      http://www.hes-so.ch
 *
 *   Contact:
 *      maxence.laurent <at> heig-vd <dot> ch
 *    or
 *      eric.taillard <at> heig-vd <dot> ch
 *
 * This file is part of libpal.
 *
 * libpal is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * libpal is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with libpal.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "pal.h"
#include "layer.h"
#include "palexception.h"
#include "internalexception.h"
#include "feature.h"
#include "geomfunction.h"
#include "util.h"
#include "qgslabelingenginev2.h"
#include <cmath>
#include <vector>

using namespace pal;

Layer::Layer( QgsAbstractLabelProvider* provider, const QString& name, QgsPalLayerSettings::Placement arrangement, double defaultPriority, bool active, bool toLabel, Pal *pal, bool displayAll )
    : mProvider( provider )
    , mName( name )
    , pal( pal )
    , mObstacleType( QgsPalLayerSettings::PolygonInterior )
    , mActive( active )
    , mLabelLayer( toLabel )
    , mDisplayAll( displayAll )
    , mCentroidInside( false )
    , mFitInPolygon( false )
    , mArrangement( arrangement )
    , mArrangementFlags( nullptr )
    , mMode( LabelPerFeature )
    , mMergeLines( false )
    , mUpsidedownLabels( Upright )
{
  mFeatureIndex = new RTree<FeaturePart*, double, 2, double>();
  mObstacleIndex = new RTree<FeaturePart*, double, 2, double>();

  if ( defaultPriority < 0.0001 )
    mDefaultPriority = 0.0001;
  else if ( defaultPriority > 1.0 )
    mDefaultPriority = 1.0;
  else
    mDefaultPriority = defaultPriority;
}

Layer::~Layer()
{
  mMutex.lock();

  qDeleteAll( mFeatureParts );
  qDeleteAll( mObstacleParts );

  //should already be empty
  qDeleteAll( mConnectedHashtable );

  delete mFeatureIndex;
  delete mObstacleIndex;

  mMutex.unlock();
}

void Layer::setPriority( double priority )
{
  if ( priority >= 1.0 ) // low priority
    mDefaultPriority = 1.0;
  else if ( priority <= 0.0001 )
    mDefaultPriority = 0.0001; // high priority
  else
    mDefaultPriority = priority;
}

bool Layer::registerFeature( QgsLabelFeature* lf )
{
  if ( lf->size().width() < 0 || lf->size().height() < 0 )
    return false;

  mMutex.lock();

  if ( mHashtable.contains( lf->id() ) )
  {
    mMutex.unlock();
    //A feature with this id already exists. Don't throw an exception as sometimes,
    //the same feature is added twice (dateline split with otf-reprojection)
    return false;
  }

  // assign label feature to this PAL layer
  lf->setLayer( this );

  // Split MULTI GEOM and Collection in simple geometries

  bool addedFeature = false;

  double geom_size = -1, biggest_size = -1;
  FeaturePart* biggest_part = nullptr;

  // break the (possibly multi-part) geometry into simple geometries
  QLinkedList<const GEOSGeometry*>* simpleGeometries = Util::unmulti( lf->geometry() );
  if ( !simpleGeometries ) // unmulti() failed?
  {
    mMutex.unlock();
    throw InternalException::UnknownGeometry();
  }

  GEOSContextHandle_t geosctxt = geosContext();

  bool featureGeomIsObstacleGeom = !lf->obstacleGeometry();

  while ( !simpleGeometries->isEmpty() )
  {
    const GEOSGeometry* geom = simpleGeometries->takeFirst();

    // ignore invalid geometries (e.g. polygons with self-intersecting rings)
    if ( GEOSisValid_r( geosctxt, geom ) != 1 ) // 0=invalid, 1=valid, 2=exception
    {
      continue;
    }

    int type = GEOSGeomTypeId_r( geosctxt, geom );

    if ( type != GEOS_POINT && type != GEOS_LINESTRING && type != GEOS_POLYGON )
    {
      mMutex.unlock();
      throw InternalException::UnknownGeometry();
    }

    FeaturePart* fpart = new FeaturePart( lf, geom );

    // ignore invalid geometries
    if (( type == GEOS_LINESTRING && fpart->nbPoints < 2 ) ||
        ( type == GEOS_POLYGON && fpart->nbPoints < 3 ) )
    {
      delete fpart;
      continue;
    }

    // polygons: reorder coordinates
    if ( type == GEOS_POLYGON && GeomFunction::reorderPolygon( fpart->nbPoints, fpart->x, fpart->y ) != 0 )
    {
      delete fpart;
      continue;
    }

    // is the feature well defined?  TODO Check epsilon
    bool labelWellDefined = ( lf->size().width() > 0.0000001 && lf->size().height() > 0.0000001 );

    if ( lf->isObstacle() && featureGeomIsObstacleGeom )
    {
      //if we are not labelling the layer, only insert it into the obstacle list and avoid an
      //unnecessary copy
      if ( mLabelLayer && labelWellDefined )
      {
        addObstaclePart( new FeaturePart( *fpart ) );
      }
      else
      {
        addObstaclePart( fpart );
        fpart = nullptr;
      }
    }

    // feature has to be labeled?
    if ( !mLabelLayer || !labelWellDefined )
    {
      //nothing more to do for this part
      delete fpart;
      continue;
    }

    if ( mMode == LabelPerFeature && ( type == GEOS_POLYGON || type == GEOS_LINESTRING ) )
    {
      if ( type == GEOS_LINESTRING )
        GEOSLength_r( geosctxt, geom, &geom_size );
      else if ( type == GEOS_POLYGON )
        GEOSArea_r( geosctxt, geom, &geom_size );

      if ( geom_size > biggest_size )
      {
        biggest_size = geom_size;
        delete biggest_part; // safe with NULL part
        biggest_part = fpart;
      }
      else
      {
        delete fpart;
      }
      continue; // don't add the feature part now, do it later
    }

    // feature part is ready!
    addFeaturePart( fpart, lf->labelText() );
    addedFeature = true;
  }
  delete simpleGeometries;

  if ( !featureGeomIsObstacleGeom )
  {
    //do the same for the obstacle geometry
    simpleGeometries = Util::unmulti( lf->obstacleGeometry() );
    if ( !simpleGeometries ) // unmulti() failed?
    {
      mMutex.unlock();
      throw InternalException::UnknownGeometry();
    }

    while ( !simpleGeometries->isEmpty() )
    {
      const GEOSGeometry* geom = simpleGeometries->takeFirst();

      // ignore invalid geometries (e.g. polygons with self-intersecting rings)
      if ( GEOSisValid_r( geosctxt, geom ) != 1 ) // 0=invalid, 1=valid, 2=exception
      {
        continue;
      }

      int type = GEOSGeomTypeId_r( geosctxt, geom );

      if ( type != GEOS_POINT && type != GEOS_LINESTRING && type != GEOS_POLYGON )
      {
        mMutex.unlock();
        throw InternalException::UnknownGeometry();
      }

      FeaturePart* fpart = new FeaturePart( lf, geom );

      // ignore invalid geometries
      if (( type == GEOS_LINESTRING && fpart->nbPoints < 2 ) ||
          ( type == GEOS_POLYGON && fpart->nbPoints < 3 ) )
      {
        delete fpart;
        continue;
      }

      // polygons: reorder coordinates
      if ( type == GEOS_POLYGON && GeomFunction::reorderPolygon( fpart->nbPoints, fpart->x, fpart->y ) != 0 )
      {
        delete fpart;
        continue;
      }

      // feature part is ready!
      addObstaclePart( fpart );
    }
    delete simpleGeometries;
  }

  mMutex.unlock();

  // if using only biggest parts...
  if (( mMode == LabelPerFeature || lf->hasFixedPosition() ) && biggest_part )
  {
    addFeaturePart( biggest_part, lf->labelText() );
    addedFeature = true;
  }

  // add feature to layer if we have added something
  if ( addedFeature )
  {
    mHashtable.insert( lf->id(), lf );
  }

  return addedFeature; // true if we've added something
}


void Layer::addFeaturePart( FeaturePart* fpart, const QString& labelText )
{
  double bmin[2];
  double bmax[2];
  fpart->getBoundingBox( bmin, bmax );

  // add to list of layer's feature parts
  mFeatureParts << fpart;

  // add to r-tree for fast spatial access
  mFeatureIndex->Insert( bmin, bmax, fpart );

  // add to hashtable with equally named feature parts
  if ( mMergeLines && !labelText.isEmpty() )
  {
    QLinkedList< FeaturePart*>* lst;
    if ( !mConnectedHashtable.contains( labelText ) )
    {
      // entry doesn't exist yet
      lst = new QLinkedList<FeaturePart*>;
      mConnectedHashtable.insert( labelText, lst );
      mConnectedTexts << labelText;
    }
    else
    {
      lst = mConnectedHashtable.value( labelText );
    }
    lst->append( fpart ); // add to the list
  }
}

void Layer::addObstaclePart( FeaturePart* fpart )
{
  double bmin[2];
  double bmax[2];
  fpart->getBoundingBox( bmin, bmax );

  // add to list of layer's feature parts
  mObstacleParts.append( fpart );

  // add to obstacle r-tree
  mObstacleIndex->Insert( bmin, bmax, fpart );
}

static FeaturePart* _findConnectedPart( FeaturePart* partCheck, QLinkedList<FeaturePart*>* otherParts )
{
  // iterate in the rest of the parts with the same label
  QLinkedList<FeaturePart*>::const_iterator p = otherParts->constBegin();
  while ( p != otherParts->constEnd() )
  {
    if ( partCheck->isConnected( *p ) )
    {
      // stop checking for other connected parts
      return *p;
    }
    ++p;
  }

  return nullptr; // no connected part found...
}

void Layer::joinConnectedFeatures()
{
  // go through all label texts
  int connectedFeaturesId = 0;
  Q_FOREACH ( const QString& labelText, mConnectedTexts )
  {
    if ( !mConnectedHashtable.contains( labelText ) )
      continue; // shouldn't happen

    connectedFeaturesId++;

    QLinkedList<FeaturePart*>* parts = mConnectedHashtable.value( labelText );

    // go one-by-one part, try to merge
    while ( !parts->isEmpty() )
    {
      // part we'll be checking against other in this round
      FeaturePart* partCheck = parts->takeFirst();

      FeaturePart* otherPart = _findConnectedPart( partCheck, parts );
      if ( otherPart )
      {
        // remove partCheck from r-tree
        double bmin[2], bmax[2];
        partCheck->getBoundingBox( bmin, bmax );
        mFeatureIndex->Remove( bmin, bmax, partCheck );
        mFeatureParts.removeOne( partCheck );

        mConnectedFeaturesIds.insert( partCheck->featureId(), connectedFeaturesId );
        otherPart->getBoundingBox( bmin, bmax );

        // merge points from partCheck to p->item
        if ( otherPart->mergeWithFeaturePart( partCheck ) )
        {
          mConnectedFeaturesIds.insert( otherPart->featureId(), connectedFeaturesId );
          // reinsert p->item to r-tree (probably not needed)
          mFeatureIndex->Remove( bmin, bmax, otherPart );
          otherPart->getBoundingBox( bmin, bmax );
          mFeatureIndex->Insert( bmin, bmax, otherPart );
        }
        delete partCheck;
      }
    }

    // we're done processing feature parts with this particular label text
    delete parts;
    mConnectedHashtable.remove( labelText );
  }

  // we're done processing connected features

  //should be empty, but clear to be safe
  qDeleteAll( mConnectedHashtable );
  mConnectedHashtable.clear();

  mConnectedTexts.clear();
}

int Layer::connectedFeatureId( QgsFeatureId featureId ) const
{
  return mConnectedFeaturesIds.value( featureId, -1 );
}

void Layer::chopFeaturesAtRepeatDistance()
{
  GEOSContextHandle_t geosctxt = geosContext();
  QLinkedList<FeaturePart*> newFeatureParts;
  while ( !mFeatureParts.isEmpty() )
  {
    FeaturePart* fpart = mFeatureParts.takeFirst();
    const GEOSGeometry* geom = fpart->geos();
    double chopInterval = fpart->repeatDistance();
    if ( chopInterval != 0. && GEOSGeomTypeId_r( geosctxt, geom ) == GEOS_LINESTRING )
    {

      double bmin[2], bmax[2];
      fpart->getBoundingBox( bmin, bmax );
      mFeatureIndex->Remove( bmin, bmax, fpart );

      const GEOSCoordSequence *cs = GEOSGeom_getCoordSeq_r( geosctxt, geom );

      // get number of points
      unsigned int n;
      GEOSCoordSeq_getSize_r( geosctxt, cs, &n );

      // Read points
      std::vector<Point> points( n );
      for ( unsigned int i = 0; i < n; ++i )
      {
        GEOSCoordSeq_getX_r( geosctxt, cs, i, &points[i].x );
        GEOSCoordSeq_getY_r( geosctxt, cs, i, &points[i].y );
      }

      // Cumulative length vector
      std::vector<double> len( n, 0 );
      for ( unsigned int i = 1; i < n; ++i )
      {
        double dx = points[i].x - points[i - 1].x;
        double dy = points[i].y - points[i - 1].y;
        len[i] = len[i - 1] + std::sqrt( dx * dx + dy * dy );
      }

      // Walk along line
      unsigned int cur = 0;
      double lambda = 0;
      QVector<Point> part;
      for ( ;; )
      {
        lambda += chopInterval;
        for ( ; cur < n && lambda > len[cur]; ++cur )
        {
          part.push_back( points[cur] );
        }
        if ( cur >= n )
        {
          break;
        }
        double c = ( lambda - len[cur - 1] ) / ( len[cur] - len[cur - 1] );
        Point p;
        p.x = points[cur - 1].x + c * ( points[cur].x - points[cur - 1].x );
        p.y = points[cur - 1].y + c * ( points[cur].y - points[cur - 1].y );
        part.push_back( p );
        GEOSCoordSequence* cooSeq = GEOSCoordSeq_create_r( geosctxt, part.size(), 2 );
        for ( int i = 0; i < part.size(); ++i )
        {
          GEOSCoordSeq_setX_r( geosctxt, cooSeq, i, part[i].x );
          GEOSCoordSeq_setY_r( geosctxt, cooSeq, i, part[i].y );
        }

        GEOSGeometry* newgeom = GEOSGeom_createLineString_r( geosctxt, cooSeq );
        FeaturePart* newfpart = new FeaturePart( fpart->feature(), newgeom );
        newFeatureParts.append( newfpart );
        newfpart->getBoundingBox( bmin, bmax );
        mFeatureIndex->Insert( bmin, bmax, newfpart );
        part.clear();
        part.push_back( p );
      }
      // Create final part
      part.push_back( points[n - 1] );
      GEOSCoordSequence* cooSeq = GEOSCoordSeq_create_r( geosctxt, part.size(), 2 );
      for ( int i = 0; i < part.size(); ++i )
      {
        GEOSCoordSeq_setX_r( geosctxt, cooSeq, i, part[i].x );
        GEOSCoordSeq_setY_r( geosctxt, cooSeq, i, part[i].y );
      }

      GEOSGeometry* newgeom = GEOSGeom_createLineString_r( geosctxt, cooSeq );
      FeaturePart* newfpart = new FeaturePart( fpart->feature(), newgeom );
      newFeatureParts.append( newfpart );
      newfpart->getBoundingBox( bmin, bmax );
      mFeatureIndex->Insert( bmin, bmax, newfpart );
      delete fpart;
    }
    else
    {
      newFeatureParts.append( fpart );
    }
  }

  mFeatureParts = newFeatureParts;
}
