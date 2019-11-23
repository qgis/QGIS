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
#include "qgslabelingengine.h"

#include <cmath>
#include <vector>

using namespace pal;

Layer::Layer( QgsAbstractLabelProvider *provider, const QString &name, QgsPalLayerSettings::Placement arrangement, double defaultPriority, bool active, bool toLabel, Pal *pal, bool displayAll )
  : mProvider( provider )
  , mName( name )
  , pal( pal )
  , mObstacleType( QgsPalLayerSettings::PolygonInterior )
  , mActive( active )
  , mLabelLayer( toLabel )
  , mDisplayAll( displayAll )
  , mCentroidInside( false )
  , mArrangement( arrangement )
  , mMergeLines( false )
  , mUpsidedownLabels( Upright )
{
  mFeatureIndex = new RTree<FeaturePart *, double, 2, double>();
  mObstacleIndex = new RTree<FeaturePart *, double, 2, double>();

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

bool Layer::registerFeature( QgsLabelFeature *lf )
{
  if ( lf->size().width() < 0 || lf->size().height() < 0 )
    return false;

  QMutexLocker locker( &mMutex );

  if ( mHashtable.contains( lf->id() ) )
  {
    //A feature with this id already exists. Don't throw an exception as sometimes,
    //the same feature is added twice (dateline split with otf-reprojection)
    return false;
  }

  // assign label feature to this PAL layer
  lf->setLayer( this );

  // Split MULTI GEOM and Collection in simple geometries

  bool addedFeature = false;

  double geom_size = -1, biggest_size = -1;
  std::unique_ptr<FeaturePart> biggest_part;

  // break the (possibly multi-part) geometry into simple geometries
  std::unique_ptr<QLinkedList<const GEOSGeometry *>> simpleGeometries( Util::unmulti( lf->geometry() ) );
  if ( !simpleGeometries ) // unmulti() failed?
  {
    throw InternalException::UnknownGeometry();
  }

  GEOSContextHandle_t geosctxt = QgsGeos::getGEOSHandler();

  bool featureGeomIsObstacleGeom = !lf->obstacleGeometry();

  while ( !simpleGeometries->isEmpty() )
  {
    const GEOSGeometry *geom = simpleGeometries->takeFirst();

    // ignore invalid geometries (e.g. polygons with self-intersecting rings)
    if ( GEOSisValid_r( geosctxt, geom ) != 1 ) // 0=invalid, 1=valid, 2=exception
    {
      continue;
    }

    int type = GEOSGeomTypeId_r( geosctxt, geom );

    if ( type != GEOS_POINT && type != GEOS_LINESTRING && type != GEOS_POLYGON )
    {
      throw InternalException::UnknownGeometry();
    }

    std::unique_ptr<FeaturePart> fpart = qgis::make_unique<FeaturePart>( lf, geom );

    // ignore invalid geometries
    if ( ( type == GEOS_LINESTRING && fpart->nbPoints < 2 ) ||
         ( type == GEOS_POLYGON && fpart->nbPoints < 3 ) )
    {
      continue;
    }

    // polygons: reorder coordinates
    if ( type == GEOS_POLYGON && GeomFunction::reorderPolygon( fpart->nbPoints, fpart->x, fpart->y ) != 0 )
    {
      continue;
    }

    // is the feature well defined?  TODO Check epsilon
    bool labelWellDefined = ( lf->size().width() > 0.0000001 && lf->size().height() > 0.0000001 );

    if ( lf->isObstacle() && featureGeomIsObstacleGeom )
    {
      //if we are not labeling the layer, only insert it into the obstacle list and avoid an
      //unnecessary copy
      if ( mLabelLayer && labelWellDefined )
      {
        addObstaclePart( new FeaturePart( *fpart ) );
      }
      else
      {
        addObstaclePart( fpart.release() );
      }
    }

    // feature has to be labeled?
    if ( !mLabelLayer || !labelWellDefined )
    {
      //nothing more to do for this part
      continue;
    }

    if ( !lf->labelAllParts() && ( type == GEOS_POLYGON || type == GEOS_LINESTRING ) )
    {
      if ( type == GEOS_LINESTRING )
        GEOSLength_r( geosctxt, geom, &geom_size );
      else if ( type == GEOS_POLYGON )
        GEOSArea_r( geosctxt, geom, &geom_size );

      if ( geom_size > biggest_size )
      {
        biggest_size = geom_size;
        biggest_part.reset( fpart.release() );
      }
      continue; // don't add the feature part now, do it later
    }

    // feature part is ready!
    addFeaturePart( fpart.release(), lf->labelText() );
    addedFeature = true;
  }

  if ( lf->isObstacle() && !featureGeomIsObstacleGeom )
  {
    //do the same for the obstacle geometry
    simpleGeometries.reset( Util::unmulti( lf->obstacleGeometry() ) );
    if ( !simpleGeometries ) // unmulti() failed?
    {
      throw InternalException::UnknownGeometry();
    }

    while ( !simpleGeometries->isEmpty() )
    {
      const GEOSGeometry *geom = simpleGeometries->takeFirst();

      // ignore invalid geometries (e.g. polygons with self-intersecting rings)
      if ( GEOSisValid_r( geosctxt, geom ) != 1 ) // 0=invalid, 1=valid, 2=exception
      {
        continue;
      }

      int type = GEOSGeomTypeId_r( geosctxt, geom );

      if ( type != GEOS_POINT && type != GEOS_LINESTRING && type != GEOS_POLYGON )
      {
        throw InternalException::UnknownGeometry();
      }

      std::unique_ptr<FeaturePart> fpart = qgis::make_unique<FeaturePart>( lf, geom );

      // ignore invalid geometries
      if ( ( type == GEOS_LINESTRING && fpart->nbPoints < 2 ) ||
           ( type == GEOS_POLYGON && fpart->nbPoints < 3 ) )
      {
        continue;
      }

      // polygons: reorder coordinates
      if ( type == GEOS_POLYGON && GeomFunction::reorderPolygon( fpart->nbPoints, fpart->x, fpart->y ) != 0 )
      {
        continue;
      }

      // feature part is ready!
      addObstaclePart( fpart.release() );
    }
  }

  locker.unlock();

  // if using only biggest parts...
  if ( ( !lf->labelAllParts() || lf->hasFixedPosition() ) && biggest_part )
  {
    addFeaturePart( biggest_part.release(), lf->labelText() );
    addedFeature = true;
  }

  // add feature to layer if we have added something
  if ( addedFeature )
  {
    mHashtable.insert( lf->id(), lf );
  }

  return addedFeature; // true if we've added something
}


void Layer::addFeaturePart( FeaturePart *fpart, const QString &labelText )
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
    mConnectedHashtable[ labelText ].append( fpart );
  }
}

void Layer::addObstaclePart( FeaturePart *fpart )
{
  double bmin[2];
  double bmax[2];
  fpart->getBoundingBox( bmin, bmax );

  // add to list of layer's feature parts
  mObstacleParts.append( fpart );

  // add to obstacle r-tree
  mObstacleIndex->Insert( bmin, bmax, fpart );
}

static FeaturePart *_findConnectedPart( FeaturePart *partCheck, const QVector<FeaturePart *> &otherParts )
{
  // iterate in the rest of the parts with the same label
  auto it = otherParts.constBegin();
  while ( it != otherParts.constEnd() )
  {
    if ( partCheck->isConnected( *it ) )
    {
      // stop checking for other connected parts
      return *it;
    }
    ++it;
  }

  return nullptr; // no connected part found...
}

void Layer::joinConnectedFeatures()
{
  // go through all label texts
  int connectedFeaturesId = 0;
  for ( auto it = mConnectedHashtable.constBegin(); it != mConnectedHashtable.constEnd(); ++it )
  {
    QVector<FeaturePart *> parts = it.value();
    connectedFeaturesId++;

    // need to start with biggest parts first, to avoid merging in side branches before we've
    // merged the whole of the longest parts of the joined network
    std::sort( parts.begin(), parts.end(), []( FeaturePart * a, FeaturePart * b )
    {
      return a->length() > b->length();
    } );

    // go one-by-one part, try to merge
    while ( parts.count() > 1 )
    {
      // part we'll be checking against other in this round
      FeaturePart *partCheck = parts.takeFirst();

      FeaturePart *otherPart = _findConnectedPart( partCheck, parts );
      if ( otherPart )
      {
        // remove partCheck from r-tree
        double checkpartBMin[2], checkpartBMax[2];
        partCheck->getBoundingBox( checkpartBMin, checkpartBMax );

        double otherPartBMin[2], otherPartBMax[2];
        otherPart->getBoundingBox( otherPartBMin, otherPartBMax );

        // merge points from partCheck to p->item
        if ( otherPart->mergeWithFeaturePart( partCheck ) )
        {
          // remove the parts we are joining from the index
          mFeatureIndex->Remove( checkpartBMin, checkpartBMax, partCheck );
          mFeatureIndex->Remove( otherPartBMin, otherPartBMax, otherPart );

          // reinsert merged line to r-tree (probably not needed)
          otherPart->getBoundingBox( otherPartBMin, otherPartBMax );
          mFeatureIndex->Insert( otherPartBMin, otherPartBMax, otherPart );

          mConnectedFeaturesIds.insert( partCheck->featureId(), connectedFeaturesId );
          mConnectedFeaturesIds.insert( otherPart->featureId(), connectedFeaturesId );

          mFeatureParts.removeOne( partCheck );
          delete partCheck;
        }
      }
    }
  }
  mConnectedHashtable.clear();
}

int Layer::connectedFeatureId( QgsFeatureId featureId ) const
{
  return mConnectedFeaturesIds.value( featureId, -1 );
}

void Layer::chopFeaturesAtRepeatDistance()
{
  GEOSContextHandle_t geosctxt = QgsGeos::getGEOSHandler();
  QLinkedList<FeaturePart *> newFeatureParts;
  while ( !mFeatureParts.isEmpty() )
  {
    std::unique_ptr< FeaturePart > fpart( mFeatureParts.takeFirst() );
    const GEOSGeometry *geom = fpart->geos();
    double chopInterval = fpart->repeatDistance();

    // whether we CAN chop
    bool canChop = false;
    double featureLen = 0;
    if ( chopInterval != 0. && GEOSGeomTypeId_r( geosctxt, geom ) == GEOS_LINESTRING )
    {
      ( void )GEOSLength_r( geosctxt, geom, &featureLen );
      if ( featureLen > chopInterval )
        canChop = true;
    }

    // whether we SHOULD chop
    bool shouldChop = canChop;
    int possibleSegments = 0;
    if ( canChop )
    {
      // never chop into segments smaller than required for the actual label text
      chopInterval *= std::ceil( fpart->getLabelWidth() / fpart->repeatDistance() );

      // now work out how many full segments we could chop this line into
      possibleSegments = static_cast< int >( std::floor( featureLen / chopInterval ) );

      // ... and use this to work out the actual chop distance for this line. Otherwise, we risk the
      // situation of:
      // 1. Line length of 3cm
      // 2. Repeat distance of 2cm
      // 3. Label size is 1.5 cm
      //
      //      2cm    1cm
      // /--Label--/----/
      //
      // i.e. the labels would be off center and gravitate toward line starts
      chopInterval = featureLen / possibleSegments;

      shouldChop = possibleSegments > 1;
    }

    if ( shouldChop )
    {
      double bmin[2], bmax[2];
      fpart->getBoundingBox( bmin, bmax );
      mFeatureIndex->Remove( bmin, bmax, fpart.get() );

      const GEOSCoordSequence *cs = GEOSGeom_getCoordSeq_r( geosctxt, geom );

      // get number of points
      unsigned int n;
      GEOSCoordSeq_getSize_r( geosctxt, cs, &n );

      // Read points
      std::vector<Point> points( n );
      for ( unsigned int i = 0; i < n; ++i )
      {
#if GEOS_VERSION_MAJOR>3 || GEOS_VERSION_MINOR>=8
        GEOSCoordSeq_getXY_r( geosctxt, cs, i, &points[i].x, &points[i].y );
#else
        GEOSCoordSeq_getX_r( geosctxt, cs, i, &points[i].x );
        GEOSCoordSeq_getY_r( geosctxt, cs, i, &points[i].y );
#endif
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

      QList<FeaturePart *> repeatParts;
      repeatParts.reserve( possibleSegments );

      for ( int segment = 0; segment < possibleSegments; segment++ )
      {
        lambda += chopInterval;
        for ( ; cur < n && lambda > len[cur]; ++cur )
        {
          part.push_back( points[cur] );
        }
        if ( cur >= n )
        {
          // Create final part
          GEOSCoordSequence *cooSeq = GEOSCoordSeq_create_r( geosctxt, part.size(), 2 );
          for ( int i = 0; i < part.size(); ++i )
          {
#if GEOS_VERSION_MAJOR>3 || GEOS_VERSION_MINOR>=8
            GEOSCoordSeq_setXY_r( geosctxt, cooSeq, i, part[i].x, part[i].y );
#else
            GEOSCoordSeq_setX_r( geosctxt, cooSeq, i, part[i].x );
            GEOSCoordSeq_setY_r( geosctxt, cooSeq, i, part[i].y );
#endif
          }
          GEOSGeometry *newgeom = GEOSGeom_createLineString_r( geosctxt, cooSeq );
          FeaturePart *newfpart = new FeaturePart( fpart->feature(), newgeom );
          newFeatureParts.append( newfpart );
          newfpart->getBoundingBox( bmin, bmax );
          mFeatureIndex->Insert( bmin, bmax, newfpart );
          repeatParts.push_back( newfpart );

          break;
        }
        double c = ( lambda - len[cur - 1] ) / ( len[cur] - len[cur - 1] );
        Point p;
        p.x = points[cur - 1].x + c * ( points[cur].x - points[cur - 1].x );
        p.y = points[cur - 1].y + c * ( points[cur].y - points[cur - 1].y );
        part.push_back( p );
        GEOSCoordSequence *cooSeq = GEOSCoordSeq_create_r( geosctxt, part.size(), 2 );
        for ( int i = 0; i < part.size(); ++i )
        {
#if GEOS_VERSION_MAJOR>3 || GEOS_VERSION_MINOR>=8
          GEOSCoordSeq_setXY_r( geosctxt, cooSeq, i, part[i].x, part[i].y );
#else
          GEOSCoordSeq_setX_r( geosctxt, cooSeq, i, part[i].x );
          GEOSCoordSeq_setY_r( geosctxt, cooSeq, i, part[i].y );
#endif
        }

        GEOSGeometry *newgeom = GEOSGeom_createLineString_r( geosctxt, cooSeq );
        FeaturePart *newfpart = new FeaturePart( fpart->feature(), newgeom );
        newFeatureParts.append( newfpart );
        newfpart->getBoundingBox( bmin, bmax );
        mFeatureIndex->Insert( bmin, bmax, newfpart );
        part.clear();
        part.push_back( p );
        repeatParts.push_back( newfpart );
      }

      for ( FeaturePart *part : repeatParts )
        part->setTotalRepeats( repeatParts.count() );
    }
    else
    {
      newFeatureParts.append( fpart.release() );
    }
  }

  mFeatureParts = newFeatureParts;
}
