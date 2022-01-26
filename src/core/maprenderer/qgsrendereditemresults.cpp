/***************************************************************************
  qgsrendereditemresults.cpp
  -------------------
   begin                : August 2021
   copyright            : (C) Nyall Dawson
   email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsrendereditemresults.h"
#include "qgsrendereditemdetails.h"
#include "qgsrendercontext.h"
#include "qgslogger.h"
#include "qgsrenderedannotationitemdetails.h"
#include "RTree.h"


///@cond PRIVATE
class QgsRenderedItemResultsSpatialIndex : public RTree<const QgsRenderedItemDetails *, float, 2, float>
{
  public:

    explicit QgsRenderedItemResultsSpatialIndex( const QgsRectangle &maxBounds )
      : mXMin( maxBounds.xMinimum() )
      , mYMin( maxBounds.yMinimum() )
      , mXRes( ( std::numeric_limits< float >::max() - 1 ) / ( maxBounds.xMaximum() - maxBounds.xMinimum() ) )
      , mYRes( ( std::numeric_limits< float >::max() - 1 ) / ( maxBounds.yMaximum() - maxBounds.yMinimum() ) )
      , mMaxBounds( maxBounds )
      , mUseScale( !maxBounds.isNull() )
    {}

    void insert( const QgsRenderedItemDetails *details, const QgsRectangle &bounds )
    {
      std::array< float, 4 > scaledBounds = scaleBounds( bounds );
      this->Insert(
      {
        scaledBounds[0], scaledBounds[ 1]
      },
      {
        scaledBounds[2], scaledBounds[3]
      },
      details );
    }

    /**
     * Performs an intersection check against the index, for data intersecting the specified \a bounds.
     *
     * The \a callback function will be called once for each matching data object encountered.
     */
    bool intersects( const QgsRectangle &bounds, const std::function< bool( const QgsRenderedItemDetails *details )> &callback ) const
    {
      std::array< float, 4 > scaledBounds = scaleBounds( bounds );
      this->Search(
      {
        scaledBounds[0], scaledBounds[ 1]
      },
      {
        scaledBounds[2], scaledBounds[3]
      },
      callback );
      return true;
    }

  private:
    double mXMin = 0;
    double mYMin = 0;
    double mXRes = 1;
    double mYRes = 1;
    QgsRectangle mMaxBounds;
    bool mUseScale = false;

    std::array<float, 4> scaleBounds( const QgsRectangle &bounds ) const
    {
      if ( mUseScale )
        return
      {
        static_cast< float >( ( std::max( bounds.xMinimum(), mMaxBounds.xMinimum() ) - mXMin ) / mXRes ),
        static_cast< float >( ( std::max( bounds.yMinimum(), mMaxBounds.yMinimum() ) - mYMin ) / mYRes ),
        static_cast< float >( ( std::min( bounds.xMaximum(), mMaxBounds.xMaximum() ) - mXMin ) / mXRes ),
        static_cast< float >( ( std::min( bounds.yMaximum(), mMaxBounds.yMaximum() ) - mYMin ) / mYRes )
      };
      else
        return
      {
        static_cast< float >( bounds.xMinimum() ),
        static_cast< float >( bounds.yMinimum() ),
        static_cast< float >( bounds.xMaximum() ),
        static_cast< float >( bounds.yMaximum() )
      };
    }
};
///@endcond

QgsRenderedItemResults::QgsRenderedItemResults( const QgsRectangle &extent )
  : mExtent( extent.buffered( std::max( extent.width(), extent.height() ) * 1000 ) ) // RTree goes crazy if we insert geometries outside the bounds, so buffer them right out to be safe
  , mAnnotationItemsIndex( std::make_unique< QgsRenderedItemResultsSpatialIndex >( mExtent ) )
{

}

QgsRenderedItemResults::~QgsRenderedItemResults() = default;

QList<QgsRenderedItemDetails *> QgsRenderedItemResults::renderedItems() const
{
  QList< QgsRenderedItemDetails * > res;
  for ( const auto &it : mDetails )
  {
    std::transform( it.second.begin(), it.second.end(), std::back_inserter( res ), []( const auto & detail )
    {
      return detail.get();
    } );
  }
  return res;
}

QList<const QgsRenderedAnnotationItemDetails *> QgsRenderedItemResults::renderedAnnotationItemsInBounds( const QgsRectangle &bounds ) const
{
  QList<const QgsRenderedAnnotationItemDetails *> res;

  mAnnotationItemsIndex->intersects( bounds, [&res]( const QgsRenderedItemDetails * details )->bool
  {
    res << qgis::down_cast< const QgsRenderedAnnotationItemDetails * >( details );
    return true;
  } );
  return res;
}

void QgsRenderedItemResults::appendResults( const QList<QgsRenderedItemDetails *> &results, const QgsRenderContext &context )
{
  QgsCoordinateTransform layerToMapTransform = context.coordinateTransform();
  layerToMapTransform.setBallparkTransformsAreAppropriate( true );
  for ( QgsRenderedItemDetails *details : results )
  {
    try
    {
      const QgsRectangle transformedBounds = layerToMapTransform.transformBoundingBox( details->boundingBox() );
      details->setBoundingBox( transformedBounds );
    }
    catch ( QgsCsException & )
    {
      QgsDebugMsg( QStringLiteral( "Could not transform rendered item's bounds to map CRS" ) );
    }

    if ( QgsRenderedAnnotationItemDetails *annotationDetails = dynamic_cast< QgsRenderedAnnotationItemDetails * >( details ) )
      mAnnotationItemsIndex->insert( annotationDetails, annotationDetails->boundingBox() );


    mDetails[ details->layerId() ].emplace_back( std::unique_ptr< QgsRenderedItemDetails >( details ) );
  }
}

void QgsRenderedItemResults::transferResults( QgsRenderedItemResults *other, const QStringList &layerIds )
{
  for ( const QString &layerId : layerIds )
  {
    auto otherLayerIt = other->mDetails.find( layerId );
    if ( otherLayerIt == other->mDetails.end() )
      continue;

    std::vector< std::unique_ptr< QgsRenderedItemDetails > > &source = otherLayerIt->second;

    for ( std::unique_ptr< QgsRenderedItemDetails > &details : source )
    {
      if ( QgsRenderedAnnotationItemDetails *annotationDetails = dynamic_cast< QgsRenderedAnnotationItemDetails * >( details.get() ) )
        mAnnotationItemsIndex->insert( annotationDetails, annotationDetails->boundingBox() );

      mDetails[layerId].emplace_back( std::move( details ) );
    }

    other->mDetails.erase( otherLayerIt );
  }
}

void QgsRenderedItemResults::transferResults( QgsRenderedItemResults *other )
{
  for ( auto layerIt = other->mDetails.begin(); layerIt != other->mDetails.end(); ++layerIt )
  {
    std::vector< std::unique_ptr< QgsRenderedItemDetails > > &dest = mDetails[layerIt->first];
    dest.reserve( layerIt->second.size() );
    for ( auto it = layerIt->second.begin(); it != layerIt->second.end(); ++it )
    {
      if ( QgsRenderedAnnotationItemDetails *annotationDetails = dynamic_cast< QgsRenderedAnnotationItemDetails * >( ( *it ).get() ) )
        mAnnotationItemsIndex->insert( annotationDetails, annotationDetails->boundingBox() );

      dest.emplace_back( std::move( *it ) );
    }
  }
  other->mDetails.clear();
}

void QgsRenderedItemResults::eraseResultsFromLayers( const QStringList &layerIds )
{
  for ( const QString &layerId : layerIds )
  {
    auto it = mDetails.find( layerId );
    if ( it != mDetails.end() )
      mDetails.erase( it );
  }
}


