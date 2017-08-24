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

/** \ingroup analysis
 * \class QgsPointCompare
 */
class QgsPointCompare
{
  public:
    explicit QgsPointCompare( double tolerance )
      : mTolerance( tolerance )
    {  }

    bool operator()( const QgsPointXY &p1, const QgsPointXY &p2 ) const
    {
      if ( mTolerance <= 0 )
        return p1.x() == p2.x() ? p1.y() < p2.y() : p1.x() < p2.x();

      double tx1 = std::ceil( p1.x() / mTolerance );
      double tx2 = std::ceil( p2.x() / mTolerance );
      if ( tx1 == tx2 )
        return std::ceil( p1.y() / mTolerance ) < std::ceil( p2.y() / mTolerance );
      return tx1 < tx2;
    }

  private:
    double mTolerance;
};

template <typename RandIter, typename Type, typename CompareOp > RandIter my_binary_search( RandIter begin, RandIter end, Type val, CompareOp comp )
{
  // result if not found
  RandIter not_found = end;

  while ( true )
  {
    RandIter avg = begin + ( end - begin ) / 2;
    if ( begin == avg || end == avg )
    {
      if ( !comp( *begin, val ) && !comp( val, *begin ) )
        return begin;
      if ( !comp( *end, val ) && !comp( val, *end ) )
        return end;

      return not_found;
    }
    if ( comp( val, *avg ) )
      end = avg;
    else if ( comp( *avg, val ) )
      begin = avg;
    else
      return avg;
  }

  return not_found;
}

struct TiePointInfo
{
  QgsPointXY mTiedPoint;
  double mLength;
  QgsPointXY mFirstPoint;
  QgsPointXY mLastPoint;
};

bool TiePointInfoCompare( const TiePointInfo &a, const TiePointInfo &b )
{
  if ( a.mFirstPoint == b.mFirstPoint )
    return a.mLastPoint.x() == b.mLastPoint.x() ? a.mLastPoint.y() < b.mLastPoint.y() : a.mLastPoint.x() < b.mLastPoint.x();

  return a.mFirstPoint.x() == b.mFirstPoint.x() ? a.mFirstPoint.y() < b.mFirstPoint.y() : a.mFirstPoint.x() < b.mFirstPoint.x();
}

QgsVectorLayerDirector::QgsVectorLayerDirector( QgsFeatureSource *source,
    int directionFieldId,
    const QString &directDirectionValue,
    const QString &reverseDirectionValue,
    const QString &bothDirectionValue,
    const Direction defaultDirection
                                              )
{
  mSource                 = source;
  mDirectionFieldId       = directionFieldId;
  mDirectDirectionValue   = directDirectionValue;
  mReverseDirectionValue  = reverseDirectionValue;
  mDefaultDirection       = defaultDirection;
  mBothDirectionValue     = bothDirectionValue;
}

QString QgsVectorLayerDirector::name() const
{
  return QStringLiteral( "Vector line" );
}

void QgsVectorLayerDirector::makeGraph( QgsGraphBuilderInterface *builder, const QVector< QgsPointXY > &additionalPoints,
                                        QVector< QgsPointXY > &snappedPoints, QgsFeedback *feedback ) const
{
  int featureCount = ( int ) mSource->featureCount() * 2;
  int step = 0;

  QgsCoordinateTransform ct;
  ct.setSourceCrs( mSource->sourceCrs() );
  if ( builder->coordinateTransformationEnabled() )
  {
    ct.setDestinationCrs( builder->destinationCrs() );
  }
  else
  {
    ct.setDestinationCrs( mSource->sourceCrs() );
  }

  snappedPoints = QVector< QgsPointXY >( additionalPoints.size(), QgsPointXY( 0.0, 0.0 ) );

  TiePointInfo tmpInfo;
  tmpInfo.mLength = std::numeric_limits<double>::infinity();

  QVector< TiePointInfo > pointLengthMap( additionalPoints.size(), tmpInfo );
  QVector< TiePointInfo >::iterator pointLengthIt;

  //Graph's points;
  QVector< QgsPointXY > points;

  QgsFeatureIterator fit = mSource->getFeatures( QgsFeatureRequest().setSubsetOfAttributes( QgsAttributeList() ) );

  // begin: tie points to the graph
  QgsAttributeList la;
  QgsFeature feature;
  while ( fit.nextFeature( feature ) )
  {
    if ( feedback && feedback->isCanceled() )
    {
      return;
    }

    QgsMultiPolyline mpl;
    if ( QgsWkbTypes::flatType( feature.geometry().geometry()->wkbType() ) == QgsWkbTypes::MultiLineString )
      mpl = feature.geometry().asMultiPolyline();
    else if ( QgsWkbTypes::flatType( feature.geometry().geometry()->wkbType() ) == QgsWkbTypes::LineString )
      mpl.push_back( feature.geometry().asPolyline() );

    QgsMultiPolyline::iterator mplIt;
    for ( mplIt = mpl.begin(); mplIt != mpl.end(); ++mplIt )
    {
      QgsPointXY pt1, pt2;
      bool isFirstPoint = true;
      QgsPolyline::iterator pointIt;
      for ( pointIt = mplIt->begin(); pointIt != mplIt->end(); ++pointIt )
      {
        pt2 = ct.transform( *pointIt );
        points.push_back( pt2 );

        if ( !isFirstPoint )
        {
          int i = 0;
          for ( i = 0; i != additionalPoints.size(); ++i )
          {
            TiePointInfo info;
            if ( pt1 == pt2 )
            {
              info.mLength = additionalPoints[ i ].sqrDist( pt1 );
              info.mTiedPoint = pt1;
            }
            else
            {
              info.mLength = additionalPoints[ i ].sqrDistToSegment( pt1.x(), pt1.y(),
                             pt2.x(), pt2.y(), info.mTiedPoint );
            }

            if ( pointLengthMap[ i ].mLength > info.mLength )
            {
              Q_UNUSED( info.mTiedPoint );
              info.mFirstPoint = pt1;
              info.mLastPoint = pt2;

              pointLengthMap[ i ] = info;
              snappedPoints[ i ] = info.mTiedPoint;
            }
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
  // end: tie points to graph

  // add tied point to graph
  int i = 0;
  for ( i = 0; i < snappedPoints.size(); ++i )
  {
    if ( snappedPoints[ i ] != QgsPointXY( 0.0, 0.0 ) )
    {
      points.push_back( snappedPoints [ i ] );
    }
  }

  QgsPointCompare pointCompare( builder->topologyTolerance() );

  std::sort( points.begin(), points.end(), pointCompare );
  QVector< QgsPointXY >::iterator tmp = std::unique( points.begin(), points.end() );
  points.resize( tmp - points.begin() );

  for ( i = 0; i < points.size(); ++i )
    builder->addVertex( i, points[ i ] );

  for ( i = 0; i < snappedPoints.size() ; ++i )
    snappedPoints[ i ] = *( my_binary_search( points.begin(), points.end(), snappedPoints[ i ], pointCompare ) );

  std::sort( pointLengthMap.begin(), pointLengthMap.end(), TiePointInfoCompare );

  {
    // fill attribute list 'la'
    QgsAttributeList tmpAttr;
    if ( mDirectionFieldId != -1 )
    {
      tmpAttr.push_back( mDirectionFieldId );
    }

    QList< QgsNetworkStrategy * >::const_iterator it;
    QgsAttributeList::const_iterator it2;

    for ( it = mStrategies.begin(); it != mStrategies.end(); ++it )
    {
      QgsAttributeList tmp = ( *it )->requiredAttributes();
      for ( it2 = tmp.begin(); it2 != tmp.end(); ++it2 )
      {
        tmpAttr.push_back( *it2 );
      }
    }
    std::sort( tmpAttr.begin(), tmpAttr.end() );

    int lastAttrId = -1;
    for ( it2 = tmpAttr.begin(); it2 != tmpAttr.end(); ++it2 )
    {
      if ( *it2 == lastAttrId )
      {
        continue;
      }

      la.push_back( *it2 );

      lastAttrId = *it2;
    }
  } // end fill attribute list 'la'

  // begin graph construction
  fit = mSource->getFeatures( QgsFeatureRequest().setSubsetOfAttributes( la ) );
  while ( fit.nextFeature( feature ) )
  {
    if ( feedback && feedback->isCanceled() )
    {
      return;
    }

    Direction directionType = mDefaultDirection;

    // What direction have feature?
    QString str = feature.attribute( mDirectionFieldId ).toString();
    if ( str == mBothDirectionValue )
    {
      directionType = Direction::DirectionBoth;
    }
    else if ( str == mDirectDirectionValue )
    {
      directionType = Direction::DirectionForward;
    }
    else if ( str == mReverseDirectionValue )
    {
      directionType = Direction::DirectionBackward;
    }

    // begin features segments and add arc to the Graph;
    QgsMultiPolyline mpl;
    if ( QgsWkbTypes::flatType( feature.geometry().geometry()->wkbType() ) == QgsWkbTypes::MultiLineString )
      mpl = feature.geometry().asMultiPolyline();
    else if ( QgsWkbTypes::flatType( feature.geometry().geometry()->wkbType() ) == QgsWkbTypes::LineString )
      mpl.push_back( feature.geometry().asPolyline() );

    QgsMultiPolyline::iterator mplIt;
    for ( mplIt = mpl.begin(); mplIt != mpl.end(); ++mplIt )
    {
      QgsPointXY pt1, pt2;

      bool isFirstPoint = true;
      QgsPolyline::iterator pointIt;
      for ( pointIt = mplIt->begin(); pointIt != mplIt->end(); ++pointIt )
      {
        pt2 = ct.transform( *pointIt );

        if ( !isFirstPoint )
        {
          QMap< double, QgsPointXY > pointsOnArc;
          pointsOnArc[ 0.0 ] = pt1;
          pointsOnArc[ pt1.sqrDist( pt2 )] = pt2;

          TiePointInfo t;
          t.mFirstPoint = pt1;
          t.mLastPoint  = pt2;
          t.mLength = 0.0;
          pointLengthIt = my_binary_search( pointLengthMap.begin(), pointLengthMap.end(), t, TiePointInfoCompare );

          if ( pointLengthIt != pointLengthMap.end() )
          {
            QVector< TiePointInfo >::iterator it;
            for ( it = pointLengthIt; it - pointLengthMap.begin() >= 0; --it )
            {
              if ( it->mFirstPoint == pt1 && it->mLastPoint == pt2 )
              {
                pointsOnArc[ pt1.sqrDist( it->mTiedPoint )] = it->mTiedPoint;
              }
            }
            for ( it = pointLengthIt + 1; it != pointLengthMap.end(); ++it )
            {
              if ( it->mFirstPoint == pt1 && it->mLastPoint == pt2 )
              {
                pointsOnArc[ pt1.sqrDist( it->mTiedPoint )] = it->mTiedPoint;
              }
            }
          }

          QMap< double, QgsPointXY >::iterator pointsIt;
          QgsPointXY pt1;
          QgsPointXY pt2;
          int pt1idx = -1, pt2idx = -1;
          bool isFirstPoint = true;
          for ( pointsIt = pointsOnArc.begin(); pointsIt != pointsOnArc.end(); ++pointsIt )
          {
            pt2 = *pointsIt;
            tmp = my_binary_search( points.begin(), points.end(), pt2, pointCompare );
            pt2 = *tmp;
            pt2idx = tmp - points.begin();

            if ( !isFirstPoint && pt1 != pt2 )
            {
              double distance = builder->distanceArea()->measureLine( pt1, pt2 );
              QVector< QVariant > prop;
              QList< QgsNetworkStrategy * >::const_iterator it;
              for ( it = mStrategies.begin(); it != mStrategies.end(); ++it )
              {
                prop.push_back( ( *it )->cost( distance, feature ) );
              }

              if ( directionType == Direction::DirectionForward ||
                   directionType == Direction::DirectionBoth )
              {
                builder->addEdge( pt1idx, pt1, pt2idx, pt2, prop );
              }
              if ( directionType == Direction::DirectionBackward ||
                   directionType == Direction::DirectionBoth )
              {
                builder->addEdge( pt2idx, pt2, pt1idx, pt1, prop );
              }
            }
            pt1idx = pt2idx;
            pt1 = pt2;
            isFirstPoint = false;
          }
        } // if ( !isFirstPoint )
        pt1 = pt2;
        isFirstPoint = false;
      } // for (it = pl.begin(); it != pl.end(); ++it)
    }
    if ( feedback )
    {
      feedback->setProgress( 100.0 * static_cast< double >( ++step ) / featureCount );
    }

  } // while( mSource->nextFeature(feature) )
} // makeGraph( QgsGraphBuilderInterface *builder, const QVector< QgsPointXY >& additionalPoints, QVector< QgsPointXY >& tiedPoint )
