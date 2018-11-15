/***************************************************************************
  qgslinevectorlayerdirector.cpp
  --------------------------------------
  Date                 : 2010-10-20
  Copyright            : (C) 2010 by Yakushev Sergey
  Email                : YakushevS@list.ru
****************************************************************************
*                                                                          *
*   This program is free software; you can redistribute it and/or modify   *
*   it under the terms of the GNU General Public License as published by   *
*   the Free Software Foundation; either version 2 of the License, or      *
*   (at your option) any later version.                                    *
*                                                                          *
***************************************************************************/

/**
 * \file qgsvectorlayerdirector.cpp
 * \brief implementation of QgsVectorLayerDirector
 */

#include "qgsvectorlayerdirector.h"
#include "qgsgraphbuilderinterface.h"

#include "qgsfeatureiterator.h"
#include "qgsfeaturesource.h"
#include "qgsvectordataprovider.h"
#include "qgspoint.h"
#include "qgsgeometry.h"
#include "qgsdistancearea.h"
#include "qgswkbtypes.h"

#include <QString>
#include <QtAlgorithms>

#include <spatialindex/SpatialIndex.h>

using namespace SpatialIndex;

struct TiePointInfo
{
  TiePointInfo() = default;
  TiePointInfo( int additionalPointId, QgsFeatureId featureId, const QgsPointXY &start, const QgsPointXY &end )
    : additionalPointId( additionalPointId )
    , mNetworkFeatureId( featureId )
    , mFirstPoint( start )
    , mLastPoint( end )
  {}

  int additionalPointId = -1;
  QgsPointXY mTiedPoint;
  double mLength = std::numeric_limits<double>::max();
  QgsFeatureId mNetworkFeatureId = -1;
  QgsPointXY mFirstPoint;
  QgsPointXY mLastPoint;
};

QgsVectorLayerDirector::QgsVectorLayerDirector( QgsFeatureSource *source,
    int directionFieldId,
    const QString &directDirectionValue,
    const QString &reverseDirectionValue,
    const QString &bothDirectionValue,
    const Direction defaultDirection )
  : mSource( source )
  , mDirectionFieldId( directionFieldId )
  , mDirectDirectionValue( directDirectionValue )
  , mReverseDirectionValue( reverseDirectionValue )
  , mBothDirectionValue( bothDirectionValue )
  , mDefaultDirection( defaultDirection )
{
}

QString QgsVectorLayerDirector::name() const
{
  return QStringLiteral( "Vector line" );
}

QgsAttributeList QgsVectorLayerDirector::requiredAttributes() const
{
  QSet< int > attrs;

  if ( mDirectionFieldId != -1 )
    attrs.insert( mDirectionFieldId );

  for ( const QgsNetworkStrategy *strategy : mStrategies )
  {
    attrs.unite( strategy->requiredAttributes() );
  }
  return attrs.toList();
}

QgsVectorLayerDirector::Direction QgsVectorLayerDirector::directionForFeature( const QgsFeature &feature ) const
{
  if ( mDirectionFieldId < 0 )
    return mDefaultDirection;

  QString str = feature.attribute( mDirectionFieldId ).toString();
  if ( str == mBothDirectionValue )
  {
    return Direction::DirectionBoth;
  }
  else if ( str == mDirectDirectionValue )
  {
    return Direction::DirectionForward;
  }
  else if ( str == mReverseDirectionValue )
  {
    return Direction::DirectionBackward;
  }
  else
  {
    return mDefaultDirection;
  }
}

///@cond PRIVATE
class QgsNetworkVisitor : public SpatialIndex::IVisitor
{
  public:
    explicit QgsNetworkVisitor( QVector< int > &pointIndexes )
      : mPoints( pointIndexes ) {}

    void visitNode( const INode &n ) override
    { Q_UNUSED( n ); }

    void visitData( const IData &d ) override
    {
      mPoints.append( d.getIdentifier() );
    }

    void visitData( std::vector<const IData *> &v ) override
    { Q_UNUSED( v ); }

  private:
    QVector< int > &mPoints;
};

///@endcond

std::unique_ptr< SpatialIndex::ISpatialIndex > createVertexSpatialIndex( SpatialIndex::IStorageManager &storageManager )
{
  // R-Tree parameters
  double fillFactor = 0.7;
  unsigned long indexCapacity = 10;
  unsigned long leafCapacity = 10;
  unsigned long dimension = 2;
  RTree::RTreeVariant variant = RTree::RV_RSTAR;

  // create R-tree
  SpatialIndex::id_type indexId;
  std::unique_ptr< SpatialIndex::ISpatialIndex > iRTree( RTree::createNewRTree( storageManager, fillFactor, indexCapacity,
      leafCapacity, dimension, variant, indexId ) );
  return iRTree;
}

int findClosestVertex( const QgsPointXY &point, SpatialIndex::ISpatialIndex *index, double tolerance )
{
  QVector< int > matching;
  QgsNetworkVisitor visitor( matching );

  double pt1[2] = { point.x() - tolerance, point.y() - tolerance },
                  pt2[2] = { point.x() + tolerance, point.y() + tolerance };
  SpatialIndex::Region searchRegion( pt1, pt2, 2 );

  index->intersectsWithQuery( searchRegion, visitor );

  return matching.empty() ? -1 : matching.at( 0 );
}

void QgsVectorLayerDirector::makeGraph( QgsGraphBuilderInterface *builder, const QVector< QgsPointXY > &additionalPoints,
                                        QVector< QgsPointXY > &snappedPoints, QgsFeedback *feedback ) const
{
  long featureCount = mSource->featureCount() * 2;
  int step = 0;

  QgsCoordinateTransform ct;
  ct.setSourceCrs( mSource->sourceCrs() );
  if ( builder->coordinateTransformationEnabled() )
  {
    ct.setDestinationCrs( builder->destinationCrs() );
  }

  // clear existing snapped points list, and resize to length of provided additional points
  snappedPoints = QVector< QgsPointXY >( additionalPoints.size(), QgsPointXY( 0.0, 0.0 ) );
  // tie points = snapped location of specified additional points to network lines
  QVector< TiePointInfo > additionalTiePoints( additionalPoints.size() );

  // graph's vertices = all vertices in graph, with vertices within builder's tolerance collapsed together
  QVector< QgsPointXY > graphVertices;

  // spatial index for graph vertices
  std::unique_ptr< SpatialIndex::IStorageManager > iStorage( StorageManager::createNewMemoryStorageManager() );
  std::unique_ptr< SpatialIndex::ISpatialIndex > iRTree = createVertexSpatialIndex( *iStorage );

  double tolerance = std::max( builder->topologyTolerance(), 1e-10 );
  auto findPointWithinTolerance = [&iRTree, tolerance]( const QgsPointXY & point )->int
  {
    return findClosestVertex( point, iRTree.get(), tolerance );
  };
  auto addPointToIndex = [&iRTree]( const QgsPointXY & point, int index )
  {
    double coords[] = {point.x(), point.y()};
    iRTree->insertData( 0, nullptr, SpatialIndex::Point( coords, 2 ), index );
  };

  // first iteration - get all nodes from network, and snap additional points to network
  QgsFeatureIterator fit = mSource->getFeatures( QgsFeatureRequest().setNoAttributes() );
  QgsFeature feature;
  while ( fit.nextFeature( feature ) )
  {
    if ( feedback && feedback->isCanceled() )
      return;

    QgsMultiPolylineXY mpl;
    if ( QgsWkbTypes::flatType( feature.geometry().wkbType() ) == QgsWkbTypes::MultiLineString )
      mpl = feature.geometry().asMultiPolyline();
    else if ( QgsWkbTypes::flatType( feature.geometry().wkbType() ) == QgsWkbTypes::LineString )
      mpl.push_back( feature.geometry().asPolyline() );

    for ( const QgsPolylineXY &line : qgis::as_const( mpl ) )
    {
      QgsPointXY pt1, pt2;
      bool isFirstPoint = true;
      for ( const QgsPointXY &point : line )
      {
        pt2 = ct.transform( point );

        int pt2Idx = findPointWithinTolerance( pt2 ) ;
        if ( pt2Idx == -1 )
        {
          // no vertex already exists within tolerance - add to points, and index
          addPointToIndex( pt2, graphVertices.count() );
          graphVertices.push_back( pt2 );
        }
        else
        {
          // vertex already exists within tolerance - use that
          pt2 = graphVertices.at( pt2Idx );
        }

        if ( !isFirstPoint )
        {
          // check if this line segment is a candidate for being closest to each additional point
          int i = 0;
          for ( const QgsPointXY &additionalPoint : additionalPoints )
          {

            QgsPointXY snappedPoint;
            double thisSegmentClosestDist = std::numeric_limits<double>::max();
            if ( pt1 == pt2 )
            {
              thisSegmentClosestDist = additionalPoint.sqrDist( pt1 );
              snappedPoint = pt1;
            }
            else
            {
              thisSegmentClosestDist = additionalPoint.sqrDistToSegment( pt1.x(), pt1.y(),
                                       pt2.x(), pt2.y(), snappedPoint, 0 );
            }

            if ( thisSegmentClosestDist < additionalTiePoints[ i ].mLength )
            {
              // found a closer segment for this additional point
              TiePointInfo info( i, feature.id(), pt1, pt2 );
              info.mLength = thisSegmentClosestDist;
              info.mTiedPoint = snappedPoint;

              additionalTiePoints[ i ] = info;
              snappedPoints[ i ] = info.mTiedPoint;
            }
            i++;
          }
        }
        pt1 = pt2;
        isFirstPoint = false;
      }
    }
    if ( feedback )
      feedback->setProgress( 100.0 * static_cast< double >( ++step ) / featureCount );
  }

  // build a hash of feature ids to tie points which depend on this feature
  QHash< QgsFeatureId, QList< int > > tiePointNetworkFeatures;
  int i = 0;
  for ( TiePointInfo &info : additionalTiePoints )
  {
    tiePointNetworkFeatures[ info.mNetworkFeatureId ] << i;
    i++;
  }

  // add tied point to graph
  for ( int i = 0; i < snappedPoints.size(); ++i )
  {
    // check index to see if vertex exists within tolerance of tie point
    const QgsPointXY point = snappedPoints.at( i );
    int ptIdx = findPointWithinTolerance( point );
    if ( ptIdx == -1 )
    {
      // no vertex already within tolerance, add to index and network vertices
      addPointToIndex( point, graphVertices.count() );
      graphVertices.push_back( point );
    }
    else
    {
      // otherwise snap tie point to vertex
      snappedPoints[ i ] = graphVertices.at( ptIdx );
    }
  }
  // also need to update tie points - they need to be matched for snapped points
  for ( int i = 0; i < additionalTiePoints.count(); ++i )
  {
    additionalTiePoints[ i ].mTiedPoint = snappedPoints.at( additionalTiePoints.at( i ).additionalPointId );
  }


  // begin graph construction

  // add vertices to graph
  {
    int i = 0;
    for ( const QgsPointXY &point : graphVertices )
    {
      builder->addVertex( i, point );
      i++;
    }
  }

  fit = mSource->getFeatures( QgsFeatureRequest().setSubsetOfAttributes( requiredAttributes() ) );
  while ( fit.nextFeature( feature ) )
  {
    if ( feedback && feedback->isCanceled() )
      return;

    Direction direction = directionForFeature( feature );

    // begin features segments and add arc to the Graph;
    QgsMultiPolylineXY mpl;
    if ( QgsWkbTypes::flatType( feature.geometry().wkbType() ) == QgsWkbTypes::MultiLineString )
      mpl = feature.geometry().asMultiPolyline();
    else if ( QgsWkbTypes::flatType( feature.geometry().wkbType() ) == QgsWkbTypes::LineString )
      mpl.push_back( feature.geometry().asPolyline() );

    for ( const QgsPolylineXY &line : qgis::as_const( mpl ) )
    {
      QgsPointXY pt1, pt2;

      bool isFirstPoint = true;
      for ( const QgsPointXY &point : line )
      {
        pt2 = ct.transform( point );
        int pPt2idx = findPointWithinTolerance( pt2 );
        Q_ASSERT_X( pPt2idx >= 0, "QgsVectorLayerDirectory::makeGraph", "encountered a vertex which was not present in graph" );
        pt2 = graphVertices.at( pPt2idx );

        if ( !isFirstPoint )
        {
          QMap< double, QgsPointXY > pointsOnArc;
          pointsOnArc[ 0.0 ] = pt1;
          pointsOnArc[ pt1.sqrDist( pt2 )] = pt2;

          const QList< int > tiePointsForCurrentFeature = tiePointNetworkFeatures.value( feature.id() );
          for ( int tiePointIdx : tiePointsForCurrentFeature )
          {
            const TiePointInfo &t = additionalTiePoints.at( tiePointIdx );
            if ( t.mFirstPoint == pt1 && t.mLastPoint == pt2 )
            {
              pointsOnArc[ pt1.sqrDist( t.mTiedPoint )] = t.mTiedPoint;
            }
          }

          QgsPointXY arcPt1;
          QgsPointXY arcPt2;
          int pt1idx = -1;
          int pt2idx = -1;
          bool isFirstPoint = true;
          for ( auto arcPointIt = pointsOnArc.constBegin(); arcPointIt != pointsOnArc.constEnd(); ++arcPointIt )
          {
            arcPt2 = arcPointIt.value();

            pt2idx = findPointWithinTolerance( arcPt2 );
            Q_ASSERT_X( pt2idx >= 0, "QgsVectorLayerDirectory::makeGraph", "encountered a vertex which was not present in graph" );
            arcPt2 = graphVertices.at( pt2idx );

            if ( !isFirstPoint && arcPt1 != arcPt2 )
            {
              double distance = builder->distanceArea()->measureLine( arcPt1, arcPt2 );
              QVector< QVariant > prop;
              prop.reserve( mStrategies.size() );
              for ( QgsNetworkStrategy *strategy : mStrategies )
              {
                prop.push_back( strategy->cost( distance, feature ) );
              }

              if ( direction == Direction::DirectionForward ||
                   direction == Direction::DirectionBoth )
              {
                builder->addEdge( pt1idx, arcPt1, pt2idx, arcPt2, prop );
              }
              if ( direction == Direction::DirectionBackward ||
                   direction == Direction::DirectionBoth )
              {
                builder->addEdge( pt2idx, arcPt2, pt1idx, arcPt1, prop );
              }
            }
            pt1idx = pt2idx;
            arcPt1 = arcPt2;
            isFirstPoint = false;
          }
        }
        pt1 = pt2;
        isFirstPoint = false;
      }
    }
    if ( feedback )
    {
      feedback->setProgress( 100.0 * static_cast< double >( ++step ) / featureCount );
    }

  }
}
