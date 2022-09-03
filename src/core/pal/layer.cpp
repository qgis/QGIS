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
#include "internalexception.h"
#include "feature.h"
#include "geomfunction.h"
#include "util.h"
#include "qgslabelingengine.h"
#include "qgslogger.h"

#include <cmath>
#include <vector>

using namespace pal;

Layer::Layer( QgsAbstractLabelProvider *provider, const QString &name, Qgis::LabelPlacement arrangement, double defaultPriority, bool active, bool toLabel, Pal *pal )
  : mProvider( provider )
  , mName( name )
  , mPal( pal )
  , mActive( active )
  , mLabelLayer( toLabel )
  , mArrangement( arrangement )
{
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

  qDeleteAll( mObstacleParts );

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
  std::unique_ptr<FeaturePart> biggestPart;

  // break the (possibly multi-part) geometry into simple geometries
  std::unique_ptr<QLinkedList<const GEOSGeometry *>> simpleGeometries( Util::unmulti( lf->geometry() ) );
  if ( !simpleGeometries ) // unmulti() failed?
  {
    throw InternalException::UnknownGeometry();
  }

  GEOSContextHandle_t geosctxt = QgsGeos::getGEOSHandler();

  const bool featureGeomIsObstacleGeom = lf->obstacleSettings().obstacleGeometry().isNull();

  while ( !simpleGeometries->isEmpty() )
  {
    const GEOSGeometry *geom = simpleGeometries->takeFirst();

    // ignore invalid geometries (e.g. polygons with self-intersecting rings)
    if ( GEOSisValid_r( geosctxt, geom ) != 1 ) // 0=invalid, 1=valid, 2=exception
    {
      continue;
    }

    const int type = GEOSGeomTypeId_r( geosctxt, geom );

    if ( type != GEOS_POINT && type != GEOS_LINESTRING && type != GEOS_POLYGON )
    {
      throw InternalException::UnknownGeometry();
    }

    std::unique_ptr<FeaturePart> fpart = std::make_unique<FeaturePart>( lf, geom );

    // ignore invalid geometries
    if ( ( type == GEOS_LINESTRING && fpart->nbPoints < 2 ) ||
         ( type == GEOS_POLYGON && fpart->nbPoints < 3 ) )
    {
      continue;
    }

    // polygons: reorder coordinates
    if ( type == GEOS_POLYGON && !GeomFunction::reorderPolygon( fpart->x, fpart->y ) )
    {
      continue;
    }

    // is the feature well defined?  TODO Check epsilon
    const bool labelWellDefined = ( lf->size().width() > 0.0000001 && lf->size().height() > 0.0000001 );

    if ( lf->obstacleSettings().isObstacle() && featureGeomIsObstacleGeom )
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
        geom_size = fpart->length();
      else if ( type == GEOS_POLYGON )
        geom_size = fpart->area();

      if ( geom_size > biggest_size )
      {
        biggest_size = geom_size;
        biggestPart = std::move( fpart );
      }
      // don't add the feature part now, do it later
    }
    else
    {
      // feature part is ready!
      addFeaturePart( std::move( fpart ), lf->labelText() );
      addedFeature = true;
    }
  }

  if ( lf->obstacleSettings().isObstacle() && !featureGeomIsObstacleGeom )
  {
    //do the same for the obstacle geometry
    const QgsGeometry obstacleGeometry = lf->obstacleSettings().obstacleGeometry();
    for ( auto it = obstacleGeometry.const_parts_begin(); it != obstacleGeometry.const_parts_end(); ++it )
    {
      geos::unique_ptr geom = QgsGeos::asGeos( *it );

      if ( !geom )
      {
        QgsDebugMsg( QStringLiteral( "Obstacle geometry passed to PAL labeling engine could not be converted to GEOS! %1" ).arg( ( *it )->asWkt() ) );
        continue;
      }

      // ignore invalid geometries (e.g. polygons with self-intersecting rings)
      if ( GEOSisValid_r( geosctxt, geom.get() ) != 1 ) // 0=invalid, 1=valid, 2=exception
      {
        // this shouldn't happen -- we have already checked this while registering the feature
        QgsDebugMsg( QStringLiteral( "Obstacle geometry passed to PAL labeling engine is not valid! %1" ).arg( ( *it )->asWkt() ) );
        continue;
      }

      const int type = GEOSGeomTypeId_r( geosctxt, geom.get() );

      if ( type != GEOS_POINT && type != GEOS_LINESTRING && type != GEOS_POLYGON )
      {
        throw InternalException::UnknownGeometry();
      }

      std::unique_ptr<FeaturePart> fpart = std::make_unique<FeaturePart>( lf, geom.get() );

      // ignore invalid geometries
      if ( ( type == GEOS_LINESTRING && fpart->nbPoints < 2 ) ||
           ( type == GEOS_POLYGON && fpart->nbPoints < 3 ) )
      {
        continue;
      }

      // polygons: reorder coordinates
      if ( type == GEOS_POLYGON && !GeomFunction::reorderPolygon( fpart->x, fpart->y ) )
      {
        continue;
      }

      mGeosObstacleGeometries.emplace_back( std::move( geom ) );

      // feature part is ready!
      addObstaclePart( fpart.release() );
    }
  }

  locker.unlock();

  // if using only biggest parts...
  if ( ( !lf->labelAllParts() || lf->hasFixedPosition() ) && biggestPart )
  {
    addFeaturePart( std::move( biggestPart ), lf->labelText() );
    addedFeature = true;
  }

  // add feature to layer if we have added something
  if ( addedFeature )
  {
    mHashtable.insert( lf->id(), lf );
  }

  return addedFeature; // true if we've added something
}


void Layer::addFeaturePart( std::unique_ptr<FeaturePart> fpart, const QString &labelText )
{
  // add to hashtable with equally named feature parts
  if ( mMergeLines && !labelText.isEmpty() )
  {
    mConnectedHashtable[ labelText ].append( fpart.get() );
  }

  // add to list of layer's feature parts
  mFeatureParts.emplace_back( std::move( fpart ) );
}

void Layer::addObstaclePart( FeaturePart *fpart )
{
  // add to list of layer's feature parts
  mObstacleParts.append( fpart );
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
    QVector<FeaturePart *> partsToMerge = it.value();

    // need to start with biggest parts first, to avoid merging in side branches before we've
    // merged the whole of the longest parts of the joined network
    std::sort( partsToMerge.begin(), partsToMerge.end(), []( FeaturePart * a, FeaturePart * b )
    {
      return a->length() > b->length();
    } );

    // go one-by-one part, try to merge
    while ( partsToMerge.count() > 1 )
    {
      connectedFeaturesId++;

      // part we'll be checking against other in this round
      FeaturePart *partToJoinTo = partsToMerge.takeFirst();
      mConnectedFeaturesIds.insert( partToJoinTo->featureId(), connectedFeaturesId );

      // loop through all other parts
      QVector< FeaturePart *> partsLeftToTryThisRound = partsToMerge;
      while ( !partsLeftToTryThisRound.empty() )
      {
        if ( FeaturePart *otherPart = _findConnectedPart( partToJoinTo, partsLeftToTryThisRound ) )
        {
          partsLeftToTryThisRound.removeOne( otherPart );
          if ( partToJoinTo->mergeWithFeaturePart( otherPart ) )
          {
            mConnectedFeaturesIds.insert( otherPart->featureId(), connectedFeaturesId );

            // otherPart was merged into partToJoinTo, so now we completely delete the redundant feature part which was merged in
            partsToMerge.removeAll( otherPart );
            const auto matchingPartIt = std::find_if( mFeatureParts.begin(), mFeatureParts.end(), [otherPart]( const std::unique_ptr< FeaturePart> &part ) { return part.get() == otherPart; } );
            Q_ASSERT( matchingPartIt != mFeatureParts.end() );
            mFeatureParts.erase( matchingPartIt );
          }
        }
        else
        {
          // no candidate parts remain which we could possibly merge in
          break;
        }
      }
    }
  }
  mConnectedHashtable.clear();

  // Expunge feature parts that are smaller than the minimum size required
  mFeatureParts.erase( std::remove_if( mFeatureParts.begin(), mFeatureParts.end(), []( const std::unique_ptr< FeaturePart > &part )
  {
    if ( part->feature()->minimumSize() != 0.0 && part->length() < part->feature()->minimumSize() )
    {
      return true;
    }
    return false;
  } ), mFeatureParts.end() );
}

int Layer::connectedFeatureId( QgsFeatureId featureId ) const
{
  return mConnectedFeaturesIds.value( featureId, -1 );
}

void Layer::chopFeaturesAtRepeatDistance()
{
  GEOSContextHandle_t geosctxt = QgsGeos::getGEOSHandler();
  std::deque< std::unique_ptr< FeaturePart > > newFeatureParts;
  while ( !mFeatureParts.empty() )
  {
    std::unique_ptr< FeaturePart > fpart = std::move( mFeatureParts.front() );
    mFeatureParts.pop_front();

    const GEOSGeometry *geom = fpart->geos();
    double chopInterval = fpart->repeatDistance();

    // whether we CAN chop
    bool canChop = false;
    double featureLen = 0;
    if ( chopInterval != 0. && GEOSGeomTypeId_r( geosctxt, geom ) == GEOS_LINESTRING )
    {
      featureLen = fpart->length();
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
      const GEOSCoordSequence *cs = GEOSGeom_getCoordSeq_r( geosctxt, geom );

      // get number of points
      unsigned int n;
      GEOSCoordSeq_getSize_r( geosctxt, cs, &n );

      // Read points
      std::vector<Point> points( n );
      for ( unsigned int i = 0; i < n; ++i )
      {
        GEOSCoordSeq_getXY_r( geosctxt, cs, i, &points[i].x, &points[i].y );
      }

      // Cumulative length vector
      std::vector<double> len( n, 0 );
      for ( unsigned int i = 1; i < n; ++i )
      {
        const double dx = points[i].x - points[i - 1].x;
        const double dy = points[i].y - points[i - 1].y;
        len[i] = len[i - 1] + std::sqrt( dx * dx + dy * dy );
      }

      // Walk along line
      unsigned int cur = 0;
      double lambda = 0;
      std::vector<Point> part;

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
          GEOSCoordSequence *cooSeq = GEOSCoordSeq_create_r( geosctxt, static_cast< unsigned int >( part.size() ), 2 );
          for ( unsigned int i = 0; i < part.size(); ++i )
          {
            GEOSCoordSeq_setXY_r( geosctxt, cooSeq, i, part[i].x, part[i].y );
          }
          GEOSGeometry *newgeom = GEOSGeom_createLineString_r( geosctxt, cooSeq );
          std::unique_ptr< FeaturePart > newfpart = std::make_unique< FeaturePart >( fpart->feature(), newgeom );
          repeatParts.push_back( newfpart.get() );
          newFeatureParts.emplace_back( std::move( newfpart ) );
          break;
        }
        const double c = ( lambda - len[cur - 1] ) / ( len[cur] - len[cur - 1] );
        Point p;
        p.x = points[cur - 1].x + c * ( points[cur].x - points[cur - 1].x );
        p.y = points[cur - 1].y + c * ( points[cur].y - points[cur - 1].y );
        part.push_back( p );
        GEOSCoordSequence *cooSeq = GEOSCoordSeq_create_r( geosctxt, static_cast< unsigned int >( part.size() ), 2 );
        for ( std::size_t i = 0; i < part.size(); ++i )
        {
          GEOSCoordSeq_setXY_r( geosctxt, cooSeq, i, part[i].x, part[i].y );
        }

        GEOSGeometry *newgeom = GEOSGeom_createLineString_r( geosctxt, cooSeq );
        std::unique_ptr< FeaturePart > newfpart = std::make_unique< FeaturePart >( fpart->feature(), newgeom );
        repeatParts.push_back( newfpart.get() );
        newFeatureParts.emplace_back( std::move( newfpart ) );
        part.clear();
        part.push_back( p );
      }

      for ( FeaturePart *partPtr : repeatParts )
        partPtr->setTotalRepeats( repeatParts.count() );
    }
    else
    {
      newFeatureParts.emplace_back( std::move( fpart ) );
    }
  }

  mFeatureParts = std::move( newFeatureParts );
}


template class QgsGenericSpatialIndex<pal::FeaturePart>;
