/***************************************************************************
 *   Copyright (C) 2010 by Sergey Yakushev                                 *
 *   yakushevs <at> list.ru                                                *
 *                                                                         *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/

/**
 * \file qgslinevectorlayerdirector.cpp
 * \brief implementation of QgsLineVectorLayerDirector
 */

#include "qgslinevectorlayerdirector.h"
#include "qgsgraphbuilderintr.h"

// Qgis includes
#include <qgsvectorlayer.h>
#include <qgsmaplayerregistry.h>
#include <qgsvectordataprovider.h>
#include <qgspoint.h>
#include <qgsgeometry.h>
#include <qgsdistancearea.h>

// QT includes
#include <QString>
#include <QtAlgorithms>

//standard includes
#include <limits>
#include <algorithm>

class QgsPointCompare
{
  public:
    QgsPointCompare( double tolerance ) :
      mTolerance( tolerance )
    {  }
  
    bool operator()( const QgsPoint& p1, const QgsPoint& p2 ) const
    {
      if ( mTolerance <= 0 )
        return p1.x() == p2.x() ? p1.y() < p2.y() : p1.x() < p2.x();

      double tx1 = ceil( p1.x()/mTolerance );
      double tx2 = ceil( p2.x()/mTolerance );
      if ( tx1 == tx2 )
        return ceil( p1.y()/mTolerance ) < ceil( p2.y()/mTolerance );
      return tx1 < tx2;
    }
  
  private:
    double mTolerance;
};

template <typename RandIter, typename Type, typename CompareOp > RandIter my_binary_search( RandIter begin, RandIter end, Type val, CompareOp comp)
{
  // result if not found
  RandIter not_found = end;

  while ( true )
  {
    RandIter avg = begin + (end-begin)/2;
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

QgsLineVectorLayerDirector::QgsLineVectorLayerDirector( const QString& layerId,
    int directionFieldId,
    const QString& directDirectionValue,
    const QString& reverseDirectionValue,
    const QString& bothDirectionValue,
    int defaultDirection
    )
{
  mLayerId                = layerId;
  mDirectionFieldId       = directionFieldId;
  mDirectDirectionValue   = directDirectionValue;
  mReverseDirectionValue  = reverseDirectionValue;
  mDefaultDirection       = defaultDirection;
  mBothDirectionValue     = bothDirectionValue;
}

QgsLineVectorLayerDirector::~QgsLineVectorLayerDirector()
{

}

QString QgsLineVectorLayerDirector::name() const
{
  return QString( "Vector line" );
}

void QgsLineVectorLayerDirector::makeGraph( QgsGraphBuilderInterface *builder, const QVector< QgsPoint >& additionalPoints,
    QVector< QgsPoint >& tiedPoint ) const
{
  QgsVectorLayer *vl = myLayer();

  if ( vl == NULL )
    return;

  int featureCount = ( int ) vl->featureCount() * 2;
  int step = 0;

  QgsCoordinateTransform ct;
  ct.setSourceCrs( vl->crs() );
  if ( builder->coordinateTransformationEnabled() )
  {
    ct.setDestCRS( builder->destinationCrs() );
  }else
  {
    ct.setDestCRS( vl->crs() );
  }
 
  tiedPoint = QVector< QgsPoint >( additionalPoints.size(), QgsPoint( 0.0, 0.0 ) );
  
  TiePointInfo tmpInfo;
  tmpInfo.mLength = std::numeric_limits<double>::infinity();

  QVector< TiePointInfo > pointLengthMap( additionalPoints.size(), tmpInfo );
  QVector< TiePointInfo >::iterator pointLengthIt;
  
  //Graph's points;
  QVector< QgsPoint > points;

  // begin: tie points to the graph
  QgsAttributeList la;
  vl->select( la );
  QgsFeature feature;
  while ( vl->nextFeature( feature ) )
  {
    QgsPoint pt1, pt2;
    bool isFirstPoint = true;
    QgsPolyline pl = feature.geometry()->asPolyline();
    QgsPolyline::iterator pointIt;
    for ( pointIt = pl.begin(); pointIt != pl.end(); ++pointIt )
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
          }else
          {
            info.mLength = additionalPoints[ i ].sqrDistToSegment( pt1.x(), pt1.y(), 
                pt2.x(), pt2.y(), info.mTiedPoint );
          }

          if ( pointLengthMap[ i ].mLength > info.mLength )
          {
            info.mTiedPoint = info.mTiedPoint ;
            info.mFirstPoint = pt1;
            info.mLastPoint = pt2;

            pointLengthMap[ i ] = info;
            tiedPoint[ i ] = info.mTiedPoint;
          }
        }
      }
      pt1 = pt2;
      isFirstPoint = false;
    }
    emit buildProgress( ++step, featureCount );
  }
  // end: tie points to graph

  // add tied point to graph 
  int i = 0;
  for ( i = 0; i < tiedPoint.size(); ++i )
  {
    if ( tiedPoint[ i ] != QgsPoint( 0.0, 0.0 ) )
    {
      points.push_back( tiedPoint [ i ] );
    }
  }
  
  QgsPointCompare pointCompare( builder->topologyTolerance() );

  qSort( points.begin(), points.end(), pointCompare );
  QVector< QgsPoint >::iterator tmp = std::unique( points.begin(), points.end() ); 
  points.resize( tmp - points.begin() );
  
  for (i=0;i<points.size();++i)
    builder->addVertex( i, points[ i ] );
  
  for ( i = 0; i < tiedPoint.size() ; ++i)
    tiedPoint[ i ] = *(my_binary_search( points.begin(), points.end(), tiedPoint[ i ], pointCompare ) );

  { // fill attribute list 'la'
    QgsAttributeList tmpAttr;
    if ( mDirectionFieldId != -1 )
    {
      tmpAttr.push_back( mDirectionFieldId );
    }

    QList< QgsArcProperter* >::const_iterator it;
    QgsAttributeList::const_iterator it2;

    for ( it = mProperterList.begin(); it != mProperterList.end(); ++it )
    {
      QgsAttributeList tmp = (*it)->requiredAttributes();
      for ( it2 = tmp.begin(); it2 != tmp.end(); ++it2 )
      {
        tmpAttr.push_back( *it2 );
      }
    }
    qSort( tmpAttr.begin(), tmpAttr.end() );
    
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
  vl->select( la );
  while ( vl->nextFeature( feature ) )
  {
    QgsAttributeMap attr = feature.attributeMap();
    int directionType = mDefaultDirection;
    QgsAttributeMap::const_iterator it;
    // What direction have feature?
    for ( it = attr.constBegin(); it != attr.constEnd(); ++it )
    {
      if ( it.key() != mDirectionFieldId )
      {
        continue;
      }
      QString str = it.value().toString();
      if ( str == mBothDirectionValue )
      {
        directionType = 3;
      }
      else if ( str == mDirectDirectionValue )
      {
        directionType = 1;
      }
      else if ( str == mReverseDirectionValue )
      {
        directionType = 2;
      }
    }

    // begin features segments and add arc to the Graph;
    QgsPoint pt1, pt2;

    bool isFirstPoint = true;
    QgsPolyline pl = feature.geometry()->asPolyline();
    QgsPolyline::iterator pointIt;
    for ( pointIt = pl.begin(); pointIt != pl.end(); ++pointIt )
    {
      pt2 = ct.transform( *pointIt );
      
      if ( !isFirstPoint )
      {
        std::map< double, QgsPoint > pointsOnArc;
        pointsOnArc[ 0.0 ] = pt1;
        pointsOnArc[ pt1.sqrDist( pt2 )] = pt2;

        for ( pointLengthIt = pointLengthMap.begin(); pointLengthIt != pointLengthMap.end(); ++pointLengthIt )
        {
          if ( pointLengthIt->mFirstPoint == pt1 && pointLengthIt->mLastPoint == pt2 )
          {
            QgsPoint tiedPoint = pointLengthIt->mTiedPoint;
            pointsOnArc[ pt1.sqrDist( tiedPoint )] = tiedPoint;
          }
        }

        std::map< double, QgsPoint >::iterator pointsIt;
        QgsPoint pt1;
        QgsPoint pt2;
        int pt1idx = -1, pt2idx = -1;
        bool isFirstPoint = true;
        for ( pointsIt = pointsOnArc.begin(); pointsIt != pointsOnArc.end(); ++pointsIt )
        {
          pt2 = pointsIt->second;
          tmp = my_binary_search( points.begin(), points.end(), pt2, pointCompare );
          pt2 = *tmp;
          pt2idx = tmp - points.begin();

          if ( !isFirstPoint && pt1 != pt2 )
          {
            double distance = builder->distanceArea()->measureLine( pt1, pt2 );
            QVector< QVariant > prop;
            QList< QgsArcProperter* >::const_iterator it;
            for ( it = mProperterList.begin(); it != mProperterList.end(); ++it )
            {
              prop.push_back( (*it)->property( distance, feature ) );
            }
      
            if ( directionType == 1 ||
                 directionType == 3 )
            {
              builder->addArc( pt1idx, pt1, pt2idx, pt2, prop );
            }
            if ( directionType == 2 ||
                 directionType == 3 )
            {
              builder->addArc( pt2idx, pt2, pt1idx, pt1, prop );
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
    emit buildProgress( ++step, featureCount );
  } // while( vl->nextFeature(feature) )
} // makeGraph( QgsGraphBuilderInterface *builder, const QVector< QgsPoint >& additionalPoints, QVector< QgsPoint >& tiedPoint )

QgsVectorLayer* QgsLineVectorLayerDirector::myLayer() const
{
  QMap <QString, QgsMapLayer*> m = QgsMapLayerRegistry::instance()->mapLayers();
  QMap <QString, QgsMapLayer*>::const_iterator it = m.find( mLayerId );
  if ( it == m.end() )
  {
    return NULL;
  }
  // return NULL if it.value() isn't QgsVectorLayer()
  return dynamic_cast<QgsVectorLayer*>( it.value() );
}
