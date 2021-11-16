/***************************************************************************
                          QgsDualEdgeTriangulation.cpp
                          -------------------------
    copyright            : (C) 2004 by Marco Hugentobler
    email                : mhugent@geo.unizh.ch
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <QStack>
#include "qgsdualedgetriangulation.h"
#include <map>
#include "MathUtils.h"
#include "qgsgeometry.h"
#include "qgslogger.h"
#include "qgsvectorfilewriter.h"
#include "qgsinterpolator.h"

double leftOfTresh = 0;

static bool inCircle( const QgsPoint &testedPoint, const QgsPoint &point1, const QgsPoint &point2, const QgsPoint &point3 )
{
  const double x2 = point2.x() - point1.x();
  const double y2 = point2.y() - point1.y();
  const double x3 = point3.x() - point1.x();
  const double y3 = point3.y() - point1.y();

  const double denom = x2 * y3 - y2 * x3;
  double frac;

  if ( denom == 0 )
    return false;

  frac = ( x2 * ( x2 - x3 ) + y2 * ( y2 - y3 ) ) / denom;
  const double cx = ( x3 + frac * y3 ) / 2;
  const double cy = ( y3 - frac * x3 ) / 2;
  const double squaredRadius = ( cx * cx + cy * cy );
  const QgsPoint center( cx + point1.x(), cy + point1.y() );

  return  center.distanceSquared( testedPoint ) < squaredRadius ;
}

QgsDualEdgeTriangulation::~QgsDualEdgeTriangulation()
{
  //remove all the points
  if ( !mPointVector.isEmpty() )
  {
    for ( int i = 0; i < mPointVector.count(); i++ )
    {
      delete mPointVector[i];
    }
  }

  //remove all the HalfEdge
  if ( !mHalfEdge.isEmpty() )
  {
    for ( int i = 0; i < mHalfEdge.count(); i++ )
    {
      delete mHalfEdge[i];
    }
  }
}

void QgsDualEdgeTriangulation::performConsistencyTest()
{
  QgsDebugMsg( QStringLiteral( "performing consistency test" ) );

  for ( int i = 0; i < mHalfEdge.count(); i++ )
  {
    const int a = mHalfEdge[mHalfEdge[i]->getDual()]->getDual();
    const int b = mHalfEdge[mHalfEdge[mHalfEdge[i]->getNext()]->getNext()]->getNext();
    if ( i != a )
    {
      QgsDebugMsg( QStringLiteral( "warning, first test failed" ) );
    }
    if ( i != b )
    {
      QgsDebugMsg( QStringLiteral( "warning, second test failed" ) );
    }
  }
  QgsDebugMsg( QStringLiteral( "consistency test finished" ) );
}

void QgsDualEdgeTriangulation::addLine( const QVector<QgsPoint> &points, QgsInterpolator::SourceType lineType )
{
  int actpoint = -10;//number of the last point, which has been inserted from the line
  int currentpoint = -10;//number of the point, which is currently inserted from the line

  int i = 0;
  for ( const QgsPoint &point : points )
  {
    actpoint = addPoint( point );
    i++;
    if ( actpoint != -100 )
    {
      break;
    }
  }

  if ( actpoint == -100 )//no point of the line could be inserted
  {
    return;
  }

  for ( ; i < points.size(); ++i )
  {
    currentpoint = addPoint( points.at( i ) );
    if ( currentpoint != -100 && actpoint != -100 && currentpoint != actpoint )//-100 is the return value if the point could not be not inserted
    {
      insertForcedSegment( actpoint, currentpoint, lineType );
    }
    actpoint = currentpoint;
  }
}

int QgsDualEdgeTriangulation::addPoint( const QgsPoint &p )
{
  //first update the bounding box
  if ( mPointVector.isEmpty() )//update bounding box when the first point is inserted
  {
    mXMin = p.x();
    mYMin = p.y();
    mXMax = p.x();
    mYMax = p.y();
  }
  else //update bounding box else
  {
    mXMin = std::min( p.x(), mXMin );
    mXMax = std::max( p.x(), mXMax );
    mYMin = std::min( p.y(), mYMin );
    mYMax = std::max( p.y(), mYMax );
  }

  //then update mPointVector
  mPointVector.append( new QgsPoint( p ) );

  //then update the HalfEdgeStructure
  if ( mDimension == -1 )//insert the first point into the triangulation
  {
    const unsigned int zedge /* 0 */ = insertEdge( -10, -10, -1, false, false ); //edge pointing from p to the virtual point
    const unsigned int fedge /* 1 */ = insertEdge( static_cast<int>( zedge ), static_cast<int>( zedge ), 0, false, false ); //edge pointing from the virtual point to p
    ( mHalfEdge.at( zedge ) )->setDual( static_cast<int>( fedge ) );
    ( mHalfEdge.at( zedge ) )->setNext( static_cast<int>( fedge ) );
    mDimension = 0;
  }
  else if ( mDimension == 0 )//insert the second point into the triangulation
  {
    //test, if it is the same point as the first point
    if ( p.x() == mPointVector[0]->x() && p.y() == mPointVector[0]->y() )
    {
      //second point is the same as the first point
      removeLastPoint();
      return 0;
    }

    const unsigned int edgeFromPoint0ToPoint1 /* 2 */ = insertEdge( -10, -10, 1, false, false );//edge pointing from point 0 to point 1
    const unsigned int edgeFromPoint1ToPoint0 /* 3 */ = insertEdge( edgeFromPoint0ToPoint1, -10, 0, false, false ); //edge pointing from point 1 to point 0
    const unsigned int edgeFromVirtualToPoint1Side1 /* 4 */ = insertEdge( -10, -10, 1, false, false ); //edge pointing from the virtual point to point 1
    const unsigned int edgeFromPoint1ToVirtualSide1 /* 5 */ = insertEdge( edgeFromVirtualToPoint1Side1, 1, -1, false, false ); //edge pointing from point 1 to the virtual point
    const unsigned int edgeFromVirtualToPoint1Side2 /* 6 */ = insertEdge( -10, edgeFromPoint1ToPoint0, 1, false, false );
    const unsigned int edgeFromPoint1ToVirtualSide2 /* 7 */ = insertEdge( edgeFromVirtualToPoint1Side2, edgeFromVirtualToPoint1Side1, -1, false, false );
    const unsigned int edgeFromVirtualToPoint0Side2 /* 8 */ = insertEdge( -10, -10, 0, false, false );
    const unsigned int edgeFromPoint0ToVirtualSide2 /* 9 */ = insertEdge( edgeFromVirtualToPoint0Side2, edgeFromVirtualToPoint1Side2, -1, false, false );
    mHalfEdge.at( edgeFromPoint1ToPoint0 )->setNext( edgeFromPoint0ToVirtualSide2 );
    mHalfEdge.at( edgeFromPoint0ToPoint1 )->setDual( edgeFromPoint1ToPoint0 );
    mHalfEdge.at( edgeFromPoint0ToPoint1 )->setNext( edgeFromPoint1ToVirtualSide1 );
    mHalfEdge.at( edgeFromVirtualToPoint1Side1 )->setDual( edgeFromPoint1ToVirtualSide1 );
    mHalfEdge.at( edgeFromVirtualToPoint1Side1 )->setNext( edgeFromPoint1ToVirtualSide2 );
    mHalfEdge.at( 0 )->setNext( static_cast<int>( edgeFromVirtualToPoint0Side2 ) );
    mHalfEdge.at( 1 )->setNext( static_cast<int>( edgeFromPoint0ToPoint1 ) );
    mHalfEdge.at( edgeFromVirtualToPoint1Side2 )->setDual( edgeFromPoint1ToVirtualSide2 );
    mHalfEdge.at( edgeFromVirtualToPoint0Side2 )->setDual( edgeFromPoint0ToVirtualSide2 );
    mHalfEdge.at( edgeFromVirtualToPoint0Side2 )->setNext( 0 );
    mEdgeInside = 3;
    mEdgeOutside = edgeFromPoint0ToPoint1;
    mDimension = 1;
  }
  else if ( mDimension == 1 )
  {
    if ( mEdgeOutside < 0 || mHalfEdge[mEdgeOutside]->getPoint() < 0 || mHalfEdge[mHalfEdge[mEdgeOutside]->getDual()]->getPoint() < 0 )
      mEdgeOutside = firstEdgeOutSide();
    if ( mEdgeOutside < 0 || mHalfEdge[mEdgeOutside]->getPoint() < 0 || mHalfEdge[mHalfEdge[mEdgeOutside]->getDual()]->getPoint() < 0 )
      return -100;

    const double leftOfNumber = MathUtils::leftOf( p,  mPointVector[mHalfEdge[mHalfEdge[mEdgeOutside]->getDual()]->getPoint()], mPointVector[mHalfEdge[mEdgeOutside]->getPoint()] );
    if ( fabs( leftOfNumber ) <= leftOfTresh )
    {
      // new point colinear with existing points
      mDimension = 1;
      // First find the edge which has the point in or the closest edge if the new point is outside
      int closestEdge = -1;
      double distance = std::numeric_limits<double>::max();
      const int firstEdge = mEdgeOutside;
      do
      {
        const int point1 = mHalfEdge[mEdgeOutside]->getPoint();
        const int point2 = mHalfEdge[mHalfEdge[mEdgeOutside]->getDual()]->getPoint();
        const double distance1 = p.distance( *mPointVector[point1] );
        if ( distance1 <= leftOfTresh ) // point1 == new point
        {
          removeLastPoint();
          return point1;
        }
        const double distance2 = p.distance( *mPointVector[point2] );
        if ( distance2 <= leftOfTresh ) // point2 == new point
        {
          removeLastPoint();
          return point2;
        }

        const double edgeLength = mPointVector[point1]->distance( *mPointVector[point2] );

        if ( distance1 < edgeLength && distance2 < edgeLength )
        {
          //new point include in mEdgeOutside
          const int newPoint = mPointVector.count() - 1;

          //edges that do not change
          const int edgeFromNewPointToPoint1 = mEdgeOutside;
          const int edgeFromNewPointToPoint2 = mHalfEdge[mEdgeOutside]->getDual();
          //edges to modify
          const int edgeFromPoint1ToVirtualSide2 = mHalfEdge[edgeFromNewPointToPoint1]->getNext();
          const int edgeFromVirtualToPoint1Side1 = mHalfEdge[mHalfEdge[edgeFromNewPointToPoint2]->getNext()]->getNext();
          const int edgeFromPoint2ToVirtualSide1 = mHalfEdge[edgeFromNewPointToPoint2]->getNext();
          const int edgeFromVirtualToPoint2Side2 = mHalfEdge[mHalfEdge[edgeFromNewPointToPoint1]->getNext()]->getNext();
          //insert new edges
          const int edgeFromVirtualToNewPointSide1 = insertEdge( -10, edgeFromNewPointToPoint2, newPoint, false, false );
          const int edgeFromNewPointToVirtualSide1 = insertEdge( edgeFromVirtualToNewPointSide1, edgeFromVirtualToPoint1Side1, -1, false, false );
          const int edgeFromVirtualToNewPointSide2 = insertEdge( -10, edgeFromNewPointToPoint1, newPoint, false, false );
          const int edgeFromNewPointToVirtualSide2 = insertEdge( edgeFromVirtualToNewPointSide2, edgeFromVirtualToPoint2Side2, -1, false, false );
          const int edgeFromPoint1ToNewPoint = insertEdge( edgeFromNewPointToPoint1, edgeFromNewPointToVirtualSide1, newPoint, false, false );
          const int edgeFromPoint2ToNewPoint = insertEdge( edgeFromNewPointToPoint2, edgeFromNewPointToVirtualSide2, newPoint, false, false );
          mHalfEdge.at( edgeFromVirtualToNewPointSide1 )->setDual( edgeFromNewPointToVirtualSide1 );
          mHalfEdge.at( edgeFromVirtualToNewPointSide2 )->setDual( edgeFromNewPointToVirtualSide2 );
          //modify existing edges
          mHalfEdge.at( edgeFromPoint1ToVirtualSide2 )->setNext( edgeFromVirtualToNewPointSide2 );
          mHalfEdge.at( edgeFromVirtualToPoint1Side1 )->setNext( edgeFromPoint1ToNewPoint );
          mHalfEdge.at( edgeFromPoint2ToVirtualSide1 )->setNext( edgeFromVirtualToNewPointSide1 );
          mHalfEdge.at( edgeFromVirtualToPoint2Side2 )->setNext( edgeFromPoint2ToNewPoint );
          mHalfEdge.at( edgeFromNewPointToPoint1 )->setDual( edgeFromPoint1ToNewPoint );
          mHalfEdge.at( edgeFromNewPointToPoint2 )->setDual( edgeFromPoint2ToNewPoint );
          return newPoint;
        }
        else
        {
          if ( distance1 < distance )
          {
            closestEdge = mEdgeOutside;
            distance = distance1;
          }
          else if ( distance2 < distance )
          {
            closestEdge = mHalfEdge[mEdgeOutside]->getDual();
            distance = distance2;
          }
        }
        mEdgeOutside = mHalfEdge[mHalfEdge[mHalfEdge[mEdgeOutside]->getNext()]->getDual()]->getNext();
      }
      while ( mEdgeOutside != firstEdge && mHalfEdge[mEdgeOutside]->getPoint() != -1 && mHalfEdge[mHalfEdge[mEdgeOutside]->getDual()]->getPoint() != -1 );

      if ( closestEdge < 0 )
        return -100; //something gets wrong

      //add the new colinear point linking it to the extremity of closest edge
      const int extremPoint = mHalfEdge[closestEdge]->getPoint();
      const int newPoint = mPointVector.count() - 1;
      //edges that do not change
      const int edgeFromExtremeToOpposite = mHalfEdge[closestEdge]->getDual();
      //edges to modify
      const int edgeFromVirtualToExtremeSide1 = mHalfEdge[mHalfEdge[closestEdge]->getNext()]->getDual();
      const int edgeFromVirtualToExtremeSide2 = mHalfEdge[mHalfEdge[mHalfEdge[closestEdge]->getDual()]->getNext()]->getNext();
      const int edgeFromExtremeToVirtualSide2 = mHalfEdge[edgeFromVirtualToExtremeSide2]->getDual();
      //insert new edge
      const int edgeFromExtremeToNewPoint = insertEdge( -10, -10, newPoint, false, false );
      const int edgeFromNewPointToExtrem = insertEdge( edgeFromExtremeToNewPoint, edgeFromExtremeToVirtualSide2, extremPoint, false, false );
      const int edgeFromNewPointToVirtualSide1 = insertEdge( -10, edgeFromVirtualToExtremeSide1, -1, false, false );
      const int edgeFromVirtualToNewPointSide1 = insertEdge( edgeFromNewPointToVirtualSide1, -10, newPoint, false, false );
      const int edgeFromNewPointToVirtualSide2 = insertEdge( -10, edgeFromVirtualToNewPointSide1, -1, false, false );
      const int edgeFromVirtualToNewPointSide2 = insertEdge( edgeFromNewPointToVirtualSide2, edgeFromNewPointToExtrem, newPoint, false, false );
      mHalfEdge.at( edgeFromExtremeToNewPoint )->setDual( edgeFromNewPointToExtrem );
      mHalfEdge.at( edgeFromExtremeToNewPoint )->setNext( edgeFromNewPointToVirtualSide1 );
      mHalfEdge.at( edgeFromNewPointToVirtualSide1 )->setDual( edgeFromVirtualToNewPointSide1 );
      mHalfEdge.at( edgeFromNewPointToVirtualSide2 )->setDual( edgeFromVirtualToNewPointSide2 );
      mHalfEdge.at( edgeFromVirtualToNewPointSide1 )->setNext( edgeFromNewPointToVirtualSide2 );
      //modify existing edges
      mHalfEdge.at( edgeFromVirtualToExtremeSide1 )->setNext( edgeFromExtremeToNewPoint );
      mHalfEdge.at( edgeFromVirtualToExtremeSide2 )->setNext( edgeFromExtremeToOpposite );
      mHalfEdge.at( edgeFromExtremeToVirtualSide2 )->setNext( edgeFromVirtualToNewPointSide2 );

      return newPoint;
    }
    else if ( leftOfNumber >= leftOfTresh )
    {
      // new point on the right of mEdgeOutside
      mEdgeOutside = mHalfEdge[mEdgeOutside]->getDual();
    }
    mDimension = 2;
    const int newPoint = mPointVector.count() - 1;
    //buil the 2D dimension triangulation
    //First clock wise
    int cwEdge = mEdgeOutside;
    while ( mHalfEdge[mHalfEdge[mHalfEdge[mHalfEdge[cwEdge]->getNext()]->getDual()]->getNext()]->getPoint() != -1 )
    {
      mHalfEdge[mHalfEdge[ cwEdge ]->getNext()]->setPoint( newPoint );
      cwEdge = mHalfEdge[mHalfEdge[mHalfEdge[cwEdge]->getNext()]->getDual()]->getNext();
    }

    const int edgeFromLastCwPointToVirtualPoint = mHalfEdge[mHalfEdge[mHalfEdge[cwEdge]->getNext()]->getDual()]->getNext();
    const int edgeFromLastCwPointToNewPointPoint = mHalfEdge[ cwEdge ]->getNext();
    const int edgeFromNewPointPointToLastCwPoint = mHalfEdge[ edgeFromLastCwPointToNewPointPoint ]->getDual();
    //connect the new point
    const int edgeFromNewPointtoVirtualPoint = insertEdge( -10, -10, -1, false, false );
    const int edgeFromVirtualPointToNewPoint = insertEdge( edgeFromNewPointtoVirtualPoint, edgeFromNewPointPointToLastCwPoint, newPoint, false, false );
    mHalfEdge.at( edgeFromLastCwPointToNewPointPoint )->setPoint( newPoint );
    mHalfEdge.at( edgeFromNewPointtoVirtualPoint )->setDual( edgeFromVirtualPointToNewPoint );
    mHalfEdge.at( edgeFromLastCwPointToVirtualPoint )->setNext( edgeFromVirtualPointToNewPoint );

    //First counter clock wise
    int ccwEdge = mEdgeOutside;
    while ( mHalfEdge[mHalfEdge[mHalfEdge[mHalfEdge[mHalfEdge[ccwEdge]->getDual()]->getNext()]->getDual()]->getNext()]->getPoint() != -1 )
    {
      mHalfEdge[mHalfEdge[mHalfEdge[mHalfEdge[ ccwEdge ]->getNext()]->getNext()]->getDual()]->setPoint( newPoint );
      ccwEdge = mHalfEdge[mHalfEdge[mHalfEdge[mHalfEdge[mHalfEdge[ccwEdge]->getDual()]->getNext()]->getDual()]->getNext()]->getDual();
    }

    const int edgeToLastCcwPoint = mHalfEdge[mHalfEdge[mHalfEdge[ccwEdge]->getDual()]->getNext()]->getDual();
    const int edgeFromLastCcwPointToNewPoint = mHalfEdge[edgeToLastCcwPoint]->getNext();
    mHalfEdge.at( edgeFromNewPointtoVirtualPoint )->setNext( edgeToLastCcwPoint );
    mHalfEdge.at( edgeFromLastCcwPointToNewPoint )->setNext( edgeFromNewPointtoVirtualPoint );
    mHalfEdge.at( edgeFromLastCcwPointToNewPoint )->setPoint( newPoint );
  }
  else
  {
    const int number = baseEdgeOfTriangle( p );

    //point is outside the convex hull----------------------------------------------------
    if ( number == -10 )
    {
      unsigned int cwEdge = mEdgeOutside;//the last visible edge clockwise from mEdgeOutside
      unsigned int ccwEdge = mEdgeOutside;//the last visible edge counterclockwise from mEdgeOutside

      //mEdgeOutside is in each case visible
      mHalfEdge[mHalfEdge[mEdgeOutside]->getNext()]->setPoint( mPointVector.count() - 1 );

      //find cwEdge and replace the virtual point with the new point when necessary (equivalent to while the hull is not convex going clock wise)
      while ( MathUtils::leftOf( *mPointVector[ mHalfEdge[mHalfEdge[mHalfEdge[mHalfEdge[cwEdge]->getNext()]->getDual()]->getNext()]->getPoint()],
                                 &p, mPointVector[ mHalfEdge[cwEdge]->getPoint()] ) < ( -leftOfTresh ) )
      {
        //set the point number of the necessary edge to the actual point instead of the virtual point
        mHalfEdge[mHalfEdge[mHalfEdge[mHalfEdge[mHalfEdge[cwEdge]->getNext()]->getDual()]->getNext()]->getNext()]->setPoint( mPointVector.count() - 1 );
        //advance cwedge one edge further clockwise
        cwEdge = ( unsigned int )mHalfEdge[mHalfEdge[mHalfEdge[cwEdge]->getNext()]->getDual()]->getNext();
      }

      //build the necessary connections with the virtual point
      const unsigned int edge1 = insertEdge( mHalfEdge[cwEdge]->getNext(), -10, mHalfEdge[cwEdge]->getPoint(), false, false );//edge pointing from the new point to the last visible point clockwise
      const unsigned int edge2 = insertEdge( mHalfEdge[mHalfEdge[cwEdge]->getNext()]->getDual(), -10, -1, false, false );//edge pointing from the last visible point to the virtual point
      const unsigned int edge3 = insertEdge( -10, edge1, mPointVector.count() - 1, false, false );//edge pointing from the virtual point to new point

      //adjust the other pointers
      mHalfEdge[mHalfEdge[mHalfEdge[cwEdge]->getNext()]->getDual()]->setDual( edge2 );
      mHalfEdge[mHalfEdge[cwEdge]->getNext()]->setDual( edge1 );
      mHalfEdge[edge1]->setNext( edge2 );
      mHalfEdge[edge2]->setNext( edge3 );

      //find ccwedge and replace the virtual point with the new point when necessary
      while ( MathUtils::leftOf( *mPointVector[mHalfEdge[mHalfEdge[mHalfEdge[ccwEdge]->getNext()]->getNext()]->getPoint()], mPointVector[mPointVector.count() - 1], mPointVector[mHalfEdge[mHalfEdge[mHalfEdge[mHalfEdge[mHalfEdge[ccwEdge]->getNext()]->getNext()]->getDual()]->getNext()]->getPoint()] ) < ( -leftOfTresh ) )
      {
        //set the point number of the necessary edge to the actual point instead of the virtual point
        mHalfEdge[mHalfEdge[mHalfEdge[mHalfEdge[ccwEdge]->getNext()]->getNext()]->getDual()]->setPoint( mPointVector.count() - 1 );
        //advance ccwedge one edge further counterclockwise
        ccwEdge = mHalfEdge[mHalfEdge[mHalfEdge[mHalfEdge[mHalfEdge[ccwEdge]->getNext()]->getNext()]->getDual()]->getNext()]->getNext();
      }

      //build the necessary connections with the virtual point
      const unsigned int edge4 = insertEdge( mHalfEdge[mHalfEdge[ccwEdge]->getNext()]->getNext(), -10, mPointVector.count() - 1, false, false );//points from the last visible point counterclockwise to the new point
      const unsigned int edge5 = insertEdge( edge3, -10, -1, false, false );//points from the new point to the virtual point
      const unsigned int edge6 = insertEdge( mHalfEdge[mHalfEdge[mHalfEdge[ccwEdge]->getNext()]->getNext()]->getDual(), edge4, mHalfEdge[mHalfEdge[ccwEdge]->getDual()]->getPoint(), false, false );//points from the virtual point to the last visible point counterclockwise



      //adjust the other pointers
      mHalfEdge[mHalfEdge[mHalfEdge[mHalfEdge[ccwEdge]->getNext()]->getNext()]->getDual()]->setDual( edge6 );
      mHalfEdge[mHalfEdge[mHalfEdge[ccwEdge]->getNext()]->getNext()]->setDual( edge4 );
      mHalfEdge[edge4]->setNext( edge5 );
      mHalfEdge[edge5]->setNext( edge6 );
      mHalfEdge[edge3]->setDual( edge5 );

      //now test the HalfEdge at the former convex hull for swappint
      unsigned int index = ccwEdge;
      unsigned int toswap;
      while ( true )
      {
        toswap = index;
        index = mHalfEdge[mHalfEdge[mHalfEdge[index]->getNext()]->getDual()]->getNext();
        checkSwapRecursively( toswap, 0 );
        if ( toswap == cwEdge )
        {
          break;
        }
      }
    }
    else if ( number >= 0 ) //point is inside the convex hull------------------------------------------------------
    {
      const int nextnumber = mHalfEdge[number]->getNext();
      const int nextnextnumber = mHalfEdge[mHalfEdge[number]->getNext()]->getNext();

      //insert 6 new HalfEdges for the connections to the vertices of the triangle
      const unsigned int edge1 = insertEdge( -10, nextnumber, mHalfEdge[number]->getPoint(), false, false );
      const unsigned int edge2 = insertEdge( static_cast<int>( edge1 ), -10, mPointVector.count() - 1, false, false );
      const unsigned int edge3 = insertEdge( -10, nextnextnumber, mHalfEdge[nextnumber]->getPoint(), false, false );
      const unsigned int edge4 = insertEdge( static_cast<int>( edge3 ), static_cast<int>( edge1 ), mPointVector.count() - 1, false, false );
      const unsigned int edge5 = insertEdge( -10, number, mHalfEdge[nextnextnumber]->getPoint(), false, false );
      const unsigned int edge6 = insertEdge( static_cast<int>( edge5 ), static_cast<int>( edge3 ), mPointVector.count() - 1, false, false );


      mHalfEdge.at( edge1 )->setDual( static_cast<int>( edge2 ) );
      mHalfEdge.at( edge2 )->setNext( static_cast<int>( edge5 ) );
      mHalfEdge.at( edge3 )->setDual( static_cast<int>( edge4 ) );
      mHalfEdge.at( edge5 )->setDual( static_cast<int>( edge6 ) );
      mHalfEdge.at( number )->setNext( static_cast<int>( edge2 ) );
      mHalfEdge.at( nextnumber )->setNext( static_cast<int>( edge4 ) );
      mHalfEdge.at( nextnextnumber )->setNext( static_cast<int>( edge6 ) );

      //check, if there are swaps necessary
      checkSwapRecursively( number, 0 );
      checkSwapRecursively( nextnumber, 0 );
      checkSwapRecursively( nextnextnumber, 0 );
    }
    //the point is exactly on an existing edge (the number of the edge is stored in the variable 'mEdgeWithPoint'---------------
    else if ( number == -20 )
    {
      //point exactly on edge;

      //check if new point is the same than one extremity
      const int point1 = mHalfEdge[mEdgeWithPoint]->getPoint();
      const int point2 = mHalfEdge[mHalfEdge[mEdgeWithPoint]->getDual()]->getPoint();
      const double distance1 = p.distance( *mPointVector[point1] );
      if ( distance1 <= leftOfTresh ) // point1 == new point
      {
        removeLastPoint();
        return point1;
      }
      const double distance2 = p.distance( *mPointVector[point2] );
      if ( distance2 <= leftOfTresh ) // point2 == new point
      {
        removeLastPoint();
        return point2;
      }

      const int edgea = mEdgeWithPoint;
      const int edgeb = mHalfEdge[mEdgeWithPoint]->getDual();
      const int edgec = mHalfEdge[edgea]->getNext();
      const int edged = mHalfEdge[edgec]->getNext();
      const int edgee = mHalfEdge[edgeb]->getNext();
      const int edgef = mHalfEdge[edgee]->getNext();

      //insert the six new edges
      const int nedge1 = insertEdge( -10, mHalfEdge[edgea]->getNext(), mHalfEdge[edgea]->getPoint(), false, false );
      const int nedge2 = insertEdge( nedge1, -10, mPointVector.count() - 1, false, false );
      const int nedge3 = insertEdge( -10, edged, mHalfEdge[edgec]->getPoint(), false, false );
      const int nedge4 = insertEdge( nedge3, nedge1, mPointVector.count() - 1, false, false );
      const int nedge5 = insertEdge( -10, edgef, mHalfEdge[edgee]->getPoint(), false, false );
      const int nedge6 = insertEdge( nedge5, edgeb, mPointVector.count() - 1, false, false );

      //adjust the triangular structure
      mHalfEdge[nedge1]->setDual( nedge2 );
      mHalfEdge[nedge2]->setNext( nedge5 );
      mHalfEdge[nedge3]->setDual( nedge4 );
      mHalfEdge[nedge5]->setDual( nedge6 );
      mHalfEdge[edgea]->setPoint( mPointVector.count() - 1 );
      mHalfEdge[edgea]->setNext( nedge3 );
      mHalfEdge[edgec]->setNext( nedge4 );
      mHalfEdge[edgee]->setNext( nedge6 );
      mHalfEdge[edgef]->setNext( nedge2 );

      //swap edges if necessary
      checkSwapRecursively( edgec, 0 );
      checkSwapRecursively( edged, 0 );
      checkSwapRecursively( edgee, 0 );
      checkSwapRecursively( edgef, 0 );
    }

    else if ( number == -100 || number == -5 )//this means unknown problems or a numerical error occurred in 'baseEdgeOfTriangle'
    {
      //QgsDebugMsg( "point has not been inserted because of unknown problems" );
      removeLastPoint();
      return -100;
    }
    else if ( number == -25 )//this means that the point has already been inserted in the triangulation
    {
      //Take the higher z-Value in case of two equal points
      QgsPoint *newPoint = mPointVector[mPointVector.count() - 1];
      QgsPoint *existingPoint = mPointVector[mTwiceInsPoint];
      existingPoint->setZ( std::max( newPoint->z(), existingPoint->z() ) );

      removeLastPoint();
      return mTwiceInsPoint;
    }
  }

  return ( mPointVector.count() - 1 );
}

int QgsDualEdgeTriangulation::baseEdgeOfPoint( int point )
{
  unsigned int actedge = mEdgeInside;//starting edge

  if ( mPointVector.count() < 4 || point == -1 || mDimension == 1 ) //at the beginning, mEdgeInside is not defined yet
  {
    int fromVirtualPoint = -1;
    //first find pointingedge(an edge pointing to p1, priority to edge that no come from virtual point)
    for ( int i = 0; i < mHalfEdge.count(); i++ )
    {
      if ( mHalfEdge[i]->getPoint() == point )//we found one
      {
        if ( mHalfEdge[mHalfEdge[i]->getDual()]->getPoint() != -1 )
          return i;
        else
          fromVirtualPoint = i;
      }
    }
    return fromVirtualPoint;
  }

  int control = 0;

  while ( true )//otherwise, start the search
  {
    control += 1;
    if ( control > 1000000 )
    {
      //QgsDebugMsg( QStringLiteral( "warning, endless loop" ) );

      //use the secure and slow method
      //qWarning( "******************warning, using the slow method in baseEdgeOfPoint****************************************" );
      for ( int i = 0; i < mHalfEdge.count(); i++ )
      {
        if ( mHalfEdge[i]->getPoint() == point && mHalfEdge[mHalfEdge[i]->getNext()]->getPoint() != -1 )//we found it
        {
          return i;
        }
      }
    }

    const int fromPoint = mHalfEdge[mHalfEdge[actedge]->getDual()]->getPoint();
    const int toPoint = mHalfEdge[actedge]->getPoint();

    if ( fromPoint == -1 || toPoint == -1 )//this would cause a crash. Therefore we use the slow method in this case
    {
      for ( int i = 0; i < mHalfEdge.count(); i++ )
      {
        if ( mHalfEdge[i]->getPoint() == point && mHalfEdge[mHalfEdge[i]->getNext()]->getPoint() != -1 )//we found it
        {
          mEdgeInside = i;
          return i;
        }
      }
    }

    const double leftOfNumber = MathUtils::leftOf( *mPointVector[point], mPointVector[mHalfEdge[mHalfEdge[actedge]->getDual()]->getPoint()], mPointVector[mHalfEdge[actedge]->getPoint()] );


    if ( mHalfEdge[actedge]->getPoint() == point && mHalfEdge[mHalfEdge[actedge]->getNext()]->getPoint() != -1 )//we found the edge
    {
      mEdgeInside = actedge;
      return actedge;
    }
    else if ( leftOfNumber <= 0.0 )
    {
      actedge = mHalfEdge[actedge]->getNext();
    }
    else
    {
      actedge = mHalfEdge[mHalfEdge[mHalfEdge[mHalfEdge[actedge]->getDual()]->getNext()]->getNext()]->getDual();
    }
  }
}

int QgsDualEdgeTriangulation::baseEdgeOfTriangle( const QgsPoint &point )
{
  unsigned int actEdge = mEdgeInside;//start with an edge which does not point to the virtual point
  if ( mHalfEdge.at( actEdge )->getPoint() < 0 )
    actEdge = mHalfEdge.at( mHalfEdge.at( mHalfEdge.at( actEdge )->getDual() )->getNext() )->getDual(); //get an real inside edge
  if ( mHalfEdge.at( mHalfEdge.at( actEdge )->getDual() )->getPoint() < 0 )
    actEdge = mHalfEdge.at( mHalfEdge.at( actEdge )->getNext() )->getDual();

  int counter = 0;//number of consecutive successful left-of-tests
  int nulls = 0;//number of left-of-tests, which returned 0. 1 means, that the point is on a line, 2 means that it is on an existing point
  int numInstabs = 0;//number of suspect left-of-tests due to 'leftOfTresh'
  int runs = 0;//counter for the number of iterations in the loop to prevent an endless loop
  int firstEndPoint = 0, secEndPoint = 0, thEndPoint = 0, fouEndPoint = 0; //four numbers of endpoints in cases when two left-of-test are 0

  while ( true )
  {
    if ( runs > MAX_BASE_ITERATIONS )//prevents endless loops
    {
      //QgsDebugMsg( "warning, probable endless loop detected" );
      return -100;
    }

    const double leftOfValue = MathUtils::leftOf( point, mPointVector.at( mHalfEdge.at( mHalfEdge.at( actEdge )->getDual() )->getPoint() ), mPointVector.at( mHalfEdge.at( actEdge )->getPoint() ) );

    if ( leftOfValue < ( -leftOfTresh ) )//point is on the left side
    {
      counter += 1;
      if ( counter == 3 )//three successful passes means that we have found the triangle
      {
        break;
      }
    }
    else if ( fabs( leftOfValue ) <= leftOfTresh ) //point is exactly in the line of the edge
    {
      if ( nulls == 0 )
      {
        //store the numbers of the two endpoints of the line
        firstEndPoint = mHalfEdge.at( mHalfEdge.at( actEdge )->getDual() )->getPoint();
        secEndPoint = mHalfEdge.at( actEdge )->getPoint();
      }
      else if ( nulls == 1 )
      {
        //store the numbers of the two endpoints of the line
        thEndPoint = mHalfEdge.at( mHalfEdge.at( actEdge )->getDual() )->getPoint();
        fouEndPoint = mHalfEdge.at( actEdge )->getPoint();
      }
      counter += 1;
      mEdgeWithPoint = actEdge;
      nulls += 1;
      if ( counter == 3 )//three successful passes means that we have found the triangle
      {
        break;
      }
    }
    else//point is on the right side
    {
      actEdge = mHalfEdge.at( actEdge )->getDual();
      counter = 1;
      nulls = 0;
      numInstabs = 0;
    }
    actEdge = mHalfEdge.at( actEdge )->getNext();
    if ( mHalfEdge.at( actEdge )->getPoint() == -1 )//the half edge points to the virtual point
    {
      if ( nulls == 1 )//point is exactly on the convex hull
      {
        return -20;
      }
      mEdgeOutside = ( unsigned int )mHalfEdge.at( mHalfEdge.at( actEdge )->getNext() )->getNext();
      mEdgeInside = mHalfEdge.at( mHalfEdge.at( mEdgeOutside )->getDual() )->getNext();
      return -10;//the point is outside the convex hull
    }
    runs++;
  }

  if ( numInstabs > 0 )//we hit an existing point or a numerical instability occurred
  {
    // QgsDebugMsg("numerical instability occurred");
    mUnstableEdge = actEdge;
    return -5;
  }

  if ( nulls == 2 )
  {
    //find out the number of the point, which has already been inserted
    if ( firstEndPoint == thEndPoint || firstEndPoint == fouEndPoint )
    {
      //firstendp is the number of the point which has been inserted twice
      mTwiceInsPoint = firstEndPoint;
      // QgsDebugMsg(QString("point nr %1 already inserted").arg(firstendp));
    }
    else if ( secEndPoint == thEndPoint || secEndPoint == fouEndPoint )
    {
      //secendp is the number of the point which has been inserted twice
      mTwiceInsPoint = secEndPoint;
      // QgsDebugMsg(QString("point nr %1 already inserted").arg(secendp));
    }

    return -25;//return the code for a point that is already contained in the triangulation
  }

  if ( nulls == 1 )//point is on an existing edge
  {
    return -20;
  }

  mEdgeInside = actEdge;

  int nr1, nr2, nr3;
  nr1 = mHalfEdge.at( actEdge )->getPoint();
  nr2 = mHalfEdge.at( mHalfEdge.at( actEdge )->getNext() )->getPoint();
  nr3 = mHalfEdge.at( mHalfEdge.at( mHalfEdge.at( actEdge )->getNext() )->getNext() )->getPoint();
  const double x1 = mPointVector.at( nr1 )->x();
  const double y1 = mPointVector.at( nr1 )->y();
  const double x2 = mPointVector.at( nr2 )->x();
  const double y2 = mPointVector.at( nr2 )->y();
  const double x3 = mPointVector.at( nr3 )->x();
  const double y3 = mPointVector.at( nr3 )->y();

  //now make sure that always the same edge is returned
  if ( x1 < x2 && x1 < x3 )//return the edge which points to the point with the lowest x-coordinate
  {
    return actEdge;
  }
  else if ( x2 < x1 && x2 < x3 )
  {
    return mHalfEdge.at( actEdge )->getNext();
  }
  else if ( x3 < x1 && x3 < x2 )
  {
    return mHalfEdge.at( mHalfEdge.at( actEdge )->getNext() )->getNext();
  }
  //in case two x-coordinates are the same, the edge pointing to the point with the lower y-coordinate is returned
  else if ( x1 == x2 )
  {
    if ( y1 < y2 )
    {
      return actEdge;
    }
    else if ( y2 < y1 )
    {
      return mHalfEdge.at( actEdge )->getNext();
    }
  }
  else if ( x2 == x3 )
  {
    if ( y2 < y3 )
    {
      return mHalfEdge.at( actEdge )->getNext();
    }
    else if ( y3 < y2 )
    {
      return mHalfEdge.at( mHalfEdge.at( actEdge )->getNext() )->getNext();
    }
  }
  else if ( x1 == x3 )
  {
    if ( y1 < y3 )
    {
      return actEdge;
    }
    else if ( y3 < y1 )
    {
      return mHalfEdge.at( mHalfEdge.at( actEdge )->getNext() )->getNext();
    }
  }
  return -100;//this means a bug happened
}

bool QgsDualEdgeTriangulation::calcNormal( double x, double y, QgsPoint &result )
{
  if ( mTriangleInterpolator )
  {
    return mTriangleInterpolator->calcNormVec( x, y, result );
  }
  else
  {
    QgsDebugMsg( QStringLiteral( "warning, null pointer" ) );
    return false;
  }
}

bool QgsDualEdgeTriangulation::calcPoint( double x, double y, QgsPoint &result )
{
  if ( mTriangleInterpolator )
  {
    return mTriangleInterpolator->calcPoint( x, y, result );
  }
  else
  {
    QgsDebugMsg( QStringLiteral( "warning, null pointer" ) );
    return false;
  }
}

bool QgsDualEdgeTriangulation::checkSwapRecursively( unsigned int edge, unsigned int recursiveDeep )
{
  if ( swapPossible( edge ) )
  {
    QgsPoint *pta = mPointVector.at( mHalfEdge.at( edge )->getPoint() );
    QgsPoint *ptb = mPointVector.at( mHalfEdge.at( mHalfEdge.at( edge )->getNext() )->getPoint() );
    QgsPoint *ptc = mPointVector.at( mHalfEdge.at( mHalfEdge.at( mHalfEdge.at( edge )->getNext( ) )->getNext() )->getPoint() );
    QgsPoint *ptd = mPointVector.at( mHalfEdge.at( mHalfEdge.at( mHalfEdge.at( edge )->getDual() )->getNext() )->getPoint() );
    if ( inCircle( *ptd, *pta, *ptb, *ptc ) /*&& recursiveDeep < 100*/ ) //empty circle criterion violated
    {
      doSwapRecursively( edge, recursiveDeep );//swap the edge (recursive)
      return true;
    }
  }
  return false;
}

bool QgsDualEdgeTriangulation::isEdgeNeedSwap( unsigned int edge ) const
{
  if ( swapPossible( edge ) )
  {
    QgsPoint *pta = mPointVector[mHalfEdge[edge]->getPoint()];
    QgsPoint *ptb = mPointVector[mHalfEdge[mHalfEdge[edge]->getNext()]->getPoint()];
    QgsPoint *ptc = mPointVector[mHalfEdge[mHalfEdge[mHalfEdge[edge]->getNext()]->getNext()]->getPoint()];
    QgsPoint *ptd = mPointVector[mHalfEdge[mHalfEdge[mHalfEdge[edge]->getDual()]->getNext()]->getPoint()];
    return inCircle( *ptd, *pta, *ptb, *ptc );
  }

  return false;

}

void QgsDualEdgeTriangulation::doOnlySwap( unsigned int edge )
{
  const unsigned int edge1 = edge;
  const unsigned int edge2 = mHalfEdge[edge]->getDual();
  const unsigned int edge3 = mHalfEdge[edge]->getNext();
  const unsigned int edge4 = mHalfEdge[mHalfEdge[edge]->getNext()]->getNext();
  const unsigned int edge5 = mHalfEdge[mHalfEdge[edge]->getDual()]->getNext();
  const unsigned int edge6 = mHalfEdge[mHalfEdge[mHalfEdge[edge]->getDual()]->getNext()]->getNext();
  mHalfEdge[edge1]->setNext( edge4 );//set the necessary nexts
  mHalfEdge[edge2]->setNext( edge6 );
  mHalfEdge[edge3]->setNext( edge2 );
  mHalfEdge[edge4]->setNext( edge5 );
  mHalfEdge[edge5]->setNext( edge1 );
  mHalfEdge[edge6]->setNext( edge3 );
  mHalfEdge[edge1]->setPoint( mHalfEdge[edge3]->getPoint() );//change the points to which edge1 and edge2 point
  mHalfEdge[edge2]->setPoint( mHalfEdge[edge5]->getPoint() );
}

void QgsDualEdgeTriangulation::doSwapRecursively( unsigned int edge, unsigned int recursiveDeep )
{
  const unsigned int edge1 = edge;
  const unsigned int edge2 = mHalfEdge.at( edge )->getDual();
  const unsigned int edge3 = mHalfEdge.at( edge )->getNext();
  const unsigned int edge4 = mHalfEdge.at( mHalfEdge.at( edge )->getNext() )->getNext();
  const unsigned int edge5 = mHalfEdge.at( mHalfEdge.at( edge )->getDual() )->getNext();
  const unsigned int edge6 = mHalfEdge.at( mHalfEdge.at( mHalfEdge.at( edge )->getDual() )->getNext() )->getNext();
  mHalfEdge.at( edge1 )->setNext( edge4 );//set the necessary nexts
  mHalfEdge.at( edge2 )->setNext( edge6 );
  mHalfEdge.at( edge3 )->setNext( edge2 );
  mHalfEdge.at( edge4 )->setNext( edge5 );
  mHalfEdge.at( edge5 )->setNext( edge1 );
  mHalfEdge.at( edge6 )->setNext( edge3 );
  mHalfEdge.at( edge1 )->setPoint( mHalfEdge.at( edge3 )->getPoint() );//change the points to which edge1 and edge2 point
  mHalfEdge.at( edge2 )->setPoint( mHalfEdge.at( edge5 )->getPoint() );
  recursiveDeep++;

  if ( recursiveDeep < 100 )
  {
    checkSwapRecursively( edge3, recursiveDeep );
    checkSwapRecursively( edge6, recursiveDeep );
    checkSwapRecursively( edge4, recursiveDeep );
    checkSwapRecursively( edge5, recursiveDeep );
  }
  else
  {
    QStack<int> edgesToSwap;
    edgesToSwap.push( edge3 );
    edgesToSwap.push( edge6 );
    edgesToSwap.push( edge4 );
    edgesToSwap.push( edge5 );
    int loopCount = 0;
    while ( !edgesToSwap.isEmpty() && loopCount < 10000 )
    {
      loopCount++;
      const unsigned int e1 = edgesToSwap.pop();
      if ( isEdgeNeedSwap( e1 ) )
      {
        const unsigned int e2 = mHalfEdge.at( e1 )->getDual();
        const unsigned int e3 = mHalfEdge.at( e1 )->getNext();
        const unsigned int e4 = mHalfEdge.at( mHalfEdge.at( e1 )->getNext() )->getNext();
        const unsigned int e5 = mHalfEdge.at( mHalfEdge.at( e1 )->getDual() )->getNext();
        const unsigned int e6 = mHalfEdge.at( mHalfEdge.at( mHalfEdge.at( e1 )->getDual() )->getNext() )->getNext();
        mHalfEdge.at( e1 )->setNext( e4 );//set the necessary nexts
        mHalfEdge.at( e2 )->setNext( e6 );
        mHalfEdge.at( e3 )->setNext( e2 );
        mHalfEdge.at( e4 )->setNext( e5 );
        mHalfEdge.at( e5 )->setNext( e1 );
        mHalfEdge.at( e6 )->setNext( e3 );
        mHalfEdge.at( e1 )->setPoint( mHalfEdge.at( e3 )->getPoint() );//change the points to which edge1 and edge2 point
        mHalfEdge.at( e2 )->setPoint( mHalfEdge.at( e5 )->getPoint() );

        edgesToSwap.push( e3 );
        edgesToSwap.push( e6 );
        edgesToSwap.push( e4 );
        edgesToSwap.push( e5 );
      }
    }
  }

}

#if 0
void DualEdgeTriangulation::draw( QPainter *p, double xlowleft, double ylowleft, double xupright, double yupright, double width, double height ) const
{
  //if mPointVector is empty, there is nothing to do
  if ( mPointVector.isEmpty() )
  {
    return;
  }

  p->setPen( mEdgeColor );

  bool *control = new bool[mHalfEdge.count()];//controllarray that no edge is painted twice
  bool *control2 = new bool[mHalfEdge.count()];//controllarray for the flat triangles

  for ( unsigned int i = 0; i <= mHalfEdge.count() - 1; i++ )
  {
    control[i] = false;
    control2[i] = false;
  }

  if ( ( ( xupright - xlowleft ) / width ) > ( ( yupright - ylowleft ) / height ) )
  {
    double lowerborder = -( height * ( xupright - xlowleft ) / width - yupright );//real world coordinates of the lower widget border. This is useful to know because of the HalfEdge bounding box test
    for ( unsigned int i = 0; i < mHalfEdge.count() - 1; i++ )
    {
      if ( mHalfEdge[i]->getPoint() == -1 || mHalfEdge[mHalfEdge[i]->getDual()]->getPoint() == -1 )
      {continue;}

      //check, if the edge belongs to a flat triangle, remove this later
      if ( !control2[i] )
      {
        double p1, p2, p3;
        if ( mHalfEdge[i]->getPoint() != -1 && mHalfEdge[mHalfEdge[i]->getNext()]->getPoint() != -1 && mHalfEdge[mHalfEdge[mHalfEdge[i]->getNext()]->getNext()]->getPoint() != -1 )
        {
          p1 = mPointVector[mHalfEdge[i]->getPoint()]->getZ();
          p2 = mPointVector[mHalfEdge[mHalfEdge[i]->getNext()]->getPoint()]->getZ();
          p3 = mPointVector[mHalfEdge[mHalfEdge[mHalfEdge[i]->getNext()]->getNext()]->getPoint()]->getZ();
          if ( p1 == p2 && p2 == p3 && halfEdgeBBoxTest( i, xlowleft, lowerborder, xupright, yupright ) && halfEdgeBBoxTest( mHalfEdge[i]->getNext(), xlowleft, lowerborder, xupright, yupright ) && halfEdgeBBoxTest( mHalfEdge[mHalfEdge[i]->getNext()]->getNext(), xlowleft, lowerborder, xupright, yupright ) )//draw the triangle
          {
            QPointArray pa( 3 );
            pa.setPoint( 0, ( mPointVector[mHalfEdge[i]->getPoint()]->getX() - xlowleft ) / ( xupright - xlowleft )*width, ( yupright - mPointVector[mHalfEdge[i]->getPoint()]->getY() ) / ( xupright - xlowleft )*width );
            pa.setPoint( 1, ( mPointVector[mHalfEdge[mHalfEdge[i]->getNext()]->getPoint()]->getX() - xlowleft ) / ( xupright - xlowleft )*width, ( yupright - mPointVector[mHalfEdge[mHalfEdge[i]->getNext()]->getPoint()]->getY() ) / ( xupright - xlowleft )*width );
            pa.setPoint( 2, ( mPointVector[mHalfEdge[mHalfEdge[mHalfEdge[i]->getNext()]->getNext()]->getPoint()]->getX() - xlowleft ) / ( xupright - xlowleft )*width, ( yupright - mPointVector[mHalfEdge[mHalfEdge[mHalfEdge[i]->getNext()]->getNext()]->getPoint()]->getY() ) / ( xupright - xlowleft )*width );
            QColor c( 255, 0, 0 );
            p->setBrush( c );
            p->drawPolygon( pa );
          }
        }

        control2[i] = true;
        control2[mHalfEdge[i]->getNext()] = true;
        control2[mHalfEdge[mHalfEdge[i]->getNext()]->getNext()] = true;
      }//end of the section, which has to be removed later

      if ( control[i] )//check, if edge has already been drawn
      {continue;}

      //draw the edge;
      if ( halfEdgeBBoxTest( i, xlowleft, lowerborder, xupright, yupright ) )//only draw the halfedge if its bounding box intersects the painted area
      {
        if ( mHalfEdge[i]->getBreak() )//change the color it the edge is a breakline
        {
          p->setPen( mBreakEdgeColor );
        }
        else if ( mHalfEdge[i]->getForced() )//change the color if the edge is forced
        {
          p->setPen( mForcedEdgeColor );
        }


        p->drawLine( ( mPointVector[mHalfEdge[i]->getPoint()]->getX() - xlowleft ) / ( xupright - xlowleft )*width, ( yupright - mPointVector[mHalfEdge[i]->getPoint()]->getY() ) / ( xupright - xlowleft )*width, ( mPointVector[mHalfEdge[mHalfEdge[i]->getDual()]->getPoint()]->getX() - xlowleft ) / ( xupright - xlowleft )*width, ( yupright - mPointVector[mHalfEdge[mHalfEdge[i]->getDual()]->getPoint()]->getY() ) / ( xupright - xlowleft )*width );

        if ( mHalfEdge[i]->getForced() )
        {
          p->setPen( mEdgeColor );
        }


      }
      control[i] = true;
      control[mHalfEdge[i]->getDual()] = true;
    }
  }
  else
  {
    double rightborder = width * ( yupright - ylowleft ) / height + xlowleft;//real world coordinates of the right widget border. This is useful to know because of the HalfEdge bounding box test
    for ( unsigned int i = 0; i < mHalfEdge.count() - 1; i++ )
    {
      if ( mHalfEdge[i]->getPoint() == -1 || mHalfEdge[mHalfEdge[i]->getDual()]->getPoint() == -1 )
      {continue;}

      //check, if the edge belongs to a flat triangle, remove this section later
      if ( !control2[i] )
      {
        double p1, p2, p3;
        if ( mHalfEdge[i]->getPoint() != -1 && mHalfEdge[mHalfEdge[i]->getNext()]->getPoint() != -1 && mHalfEdge[mHalfEdge[mHalfEdge[i]->getNext()]->getNext()]->getPoint() != -1 )
        {
          p1 = mPointVector[mHalfEdge[i]->getPoint()]->getZ();
          p2 = mPointVector[mHalfEdge[mHalfEdge[i]->getNext()]->getPoint()]->getZ();
          p3 = mPointVector[mHalfEdge[mHalfEdge[mHalfEdge[i]->getNext()]->getNext()]->getPoint()]->getZ();
          if ( p1 == p2 && p2 == p3 && halfEdgeBBoxTest( i, xlowleft, ylowleft, rightborder, yupright ) && halfEdgeBBoxTest( mHalfEdge[i]->getNext(), xlowleft, ylowleft, rightborder, yupright ) && halfEdgeBBoxTest( mHalfEdge[mHalfEdge[i]->getNext()]->getNext(), xlowleft, ylowleft, rightborder, yupright ) )//draw the triangle
          {
            QPointArray pa( 3 );
            pa.setPoint( 0, ( mPointVector[mHalfEdge[i]->getPoint()]->getX() - xlowleft ) / ( yupright - ylowleft )*height, ( yupright - mPointVector[mHalfEdge[i]->getPoint()]->getY() ) / ( yupright - ylowleft )*height );
            pa.setPoint( 1, ( mPointVector[mHalfEdge[mHalfEdge[i]->getNext()]->getPoint()]->getX() - xlowleft ) / ( yupright - ylowleft )*height, ( yupright - mPointVector[mHalfEdge[mHalfEdge[i]->getNext()]->getPoint()]->getY() ) / ( yupright - ylowleft )*height );
            pa.setPoint( 2, ( mPointVector[mHalfEdge[mHalfEdge[mHalfEdge[i]->getNext()]->getNext()]->getPoint()]->getX() - xlowleft ) / ( yupright - ylowleft )*height, ( yupright - mPointVector[mHalfEdge[mHalfEdge[mHalfEdge[i]->getNext()]->getNext()]->getPoint()]->getY() ) / ( yupright - ylowleft )*height );
            QColor c( 255, 0, 0 );
            p->setBrush( c );
            p->drawPolygon( pa );
          }
        }

        control2[i] = true;
        control2[mHalfEdge[i]->getNext()] = true;
        control2[mHalfEdge[mHalfEdge[i]->getNext()]->getNext()] = true;
      }//end of the section, which has to be removed later


      if ( control[i] )//check, if edge has already been drawn
      {continue;}

      //draw the edge
      if ( halfEdgeBBoxTest( i, xlowleft, ylowleft, rightborder, yupright ) )//only draw the edge if its bounding box intersects with the painted area
      {
        if ( mHalfEdge[i]->getBreak() )//change the color if the edge is a breakline
        {
          p->setPen( mBreakEdgeColor );
        }
        else if ( mHalfEdge[i]->getForced() )//change the color if the edge is forced
        {
          p->setPen( mForcedEdgeColor );
        }

        p->drawLine( ( mPointVector[mHalfEdge[i]->getPoint()]->getX() - xlowleft ) / ( yupright - ylowleft )*height, ( yupright - mPointVector[mHalfEdge[i]->getPoint()]->getY() ) / ( yupright - ylowleft )*height, ( mPointVector[mHalfEdge[mHalfEdge[i]->getDual()]->getPoint()]->getX() - xlowleft ) / ( yupright - ylowleft )*height, ( yupright - mPointVector[mHalfEdge[mHalfEdge[i]->getDual()]->getPoint()]->getY() ) / ( yupright - ylowleft )*height );

        if ( mHalfEdge[i]->getForced() )
        {
          p->setPen( mEdgeColor );
        }

      }
      control[i] = true;
      control[mHalfEdge[i]->getDual()] = true;
    }
  }

  delete[] control;
  delete[] control2;
}
#endif

int QgsDualEdgeTriangulation::oppositePoint( int p1, int p2 )
{

  //first find a half edge which points to p2
  const int firstedge = baseEdgeOfPoint( p2 );

  //then find the edge which comes from p1 or print an error message if there isn't any
  int theedge = -10;
  int nextnextedge = firstedge;
  int edge, nextedge;
  do
  {
    edge = mHalfEdge[nextnextedge]->getDual();
    if ( mHalfEdge[edge]->getPoint() == p1 )
    {
      theedge = nextnextedge;
      break;
    }//we found the edge
    nextedge = mHalfEdge[edge]->getNext();
    nextnextedge = mHalfEdge[nextedge]->getNext();
  }
  while ( nextnextedge != firstedge );

  if ( theedge == -10 )//there is no edge between p1 and p2
  {
    //QgsDebugMsg( QStringLiteral( "warning, error: the points are: %1 and %2" ).arg( p1 ).arg( p2 ) );
    return -10;
  }

  //finally find the opposite point
  return mHalfEdge[mHalfEdge[mHalfEdge[theedge]->getDual()]->getNext()]->getPoint();

}

QList<int> QgsDualEdgeTriangulation::surroundingTriangles( int pointno )
{
  const int firstedge = baseEdgeOfPoint( pointno );

  QList<int> vlist;
  if ( firstedge == -1 )//an error occurred
  {
    return vlist;
  }

  int actedge = firstedge;
  int edge, nextedge, nextnextedge;
  do
  {
    edge = mHalfEdge[actedge]->getDual();
    vlist.append( mHalfEdge[edge]->getPoint() );//add the number of the endpoint of the first edge to the value list
    nextedge = mHalfEdge[edge]->getNext();
    vlist.append( mHalfEdge[nextedge]->getPoint() );//add the number of the endpoint of the second edge to the value list
    nextnextedge = mHalfEdge[nextedge]->getNext();
    vlist.append( mHalfEdge[nextnextedge]->getPoint() );//add the number of endpoint of the third edge to the value list
    if ( mHalfEdge[nextnextedge]->getBreak() )//add, whether the third edge is a breakline or not
    {
      vlist.append( -10 );
    }
    else
    {
      vlist.append( -20 );
    }
    actedge = nextnextedge;
  }
  while ( nextnextedge != firstedge );

  return vlist;

}

bool QgsDualEdgeTriangulation::triangleVertices( double x, double y, QgsPoint &p1, int &n1, QgsPoint &p2, int &n2, QgsPoint &p3, int &n3 )
{
  if ( mPointVector.size() < 3 )
  {
    return false;
  }

  const QgsPoint point( x, y, 0 );
  const int edge = baseEdgeOfTriangle( point );
  if ( edge == -10 )//the point is outside the convex hull
  {
    return false;
  }

  else if ( edge >= 0 )//the point is inside the convex hull
  {
    const int ptnr1 = mHalfEdge[edge]->getPoint();
    const int ptnr2 = mHalfEdge[mHalfEdge[edge]->getNext()]->getPoint();
    const int ptnr3 = mHalfEdge[mHalfEdge[mHalfEdge[edge]->getNext()]->getNext()]->getPoint();
    p1.setX( mPointVector[ptnr1]->x() );
    p1.setY( mPointVector[ptnr1]->y() );
    p1.setZ( mPointVector[ptnr1]->z() );
    p2.setX( mPointVector[ptnr2]->x() );
    p2.setY( mPointVector[ptnr2]->y() );
    p2.setZ( mPointVector[ptnr2]->z() );
    p3.setX( mPointVector[ptnr3]->x() );
    p3.setY( mPointVector[ptnr3]->y() );
    p3.setZ( mPointVector[ptnr3]->z() );
    n1 = ptnr1;
    n2 = ptnr2;
    n3 = ptnr3;
    return true;
  }
  else if ( edge == -20 )//the point is exactly on an edge
  {
    const int ptnr1 = mHalfEdge[mEdgeWithPoint]->getPoint();
    const int ptnr2 = mHalfEdge[mHalfEdge[mEdgeWithPoint]->getNext()]->getPoint();
    const int ptnr3 = mHalfEdge[mHalfEdge[mHalfEdge[mEdgeWithPoint]->getNext()]->getNext()]->getPoint();
    if ( ptnr1 == -1 || ptnr2 == -1 || ptnr3 == -1 )
    {
      return false;
    }
    p1.setX( mPointVector[ptnr1]->x() );
    p1.setY( mPointVector[ptnr1]->y() );
    p1.setZ( mPointVector[ptnr1]->z() );
    p2.setX( mPointVector[ptnr2]->x() );
    p2.setY( mPointVector[ptnr2]->y() );
    p2.setZ( mPointVector[ptnr2]->z() );
    p3.setX( mPointVector[ptnr3]->x() );
    p3.setY( mPointVector[ptnr3]->y() );
    p3.setZ( mPointVector[ptnr3]->z() );
    n1 = ptnr1;
    n2 = ptnr2;
    n3 = ptnr3;
    return true;
  }
  else if ( edge == -25 )//x and y are the coordinates of an existing point
  {
    const int edge1 = baseEdgeOfPoint( mTwiceInsPoint );
    const int edge2 = mHalfEdge[edge1]->getNext();
    const int edge3 = mHalfEdge[edge2]->getNext();
    const int ptnr1 = mHalfEdge[edge1]->getPoint();
    const int ptnr2 = mHalfEdge[edge2]->getPoint();
    const int ptnr3 = mHalfEdge[edge3]->getPoint();
    p1.setX( mPointVector[ptnr1]->x() );
    p1.setY( mPointVector[ptnr1]->y() );
    p1.setZ( mPointVector[ptnr1]->z() );
    p2.setX( mPointVector[ptnr2]->x() );
    p2.setY( mPointVector[ptnr2]->y() );
    p2.setZ( mPointVector[ptnr2]->z() );
    p3.setX( mPointVector[ptnr3]->x() );
    p3.setY( mPointVector[ptnr3]->y() );
    p3.setZ( mPointVector[ptnr3]->z() );
    n1 = ptnr1;
    n2 = ptnr2;
    n3 = ptnr3;
    return true;
  }
  else if ( edge == -5 )//numerical problems in 'baseEdgeOfTriangle'
  {
    const int ptnr1 = mHalfEdge[mUnstableEdge]->getPoint();
    const int ptnr2 = mHalfEdge[mHalfEdge[mUnstableEdge]->getNext()]->getPoint();
    const int ptnr3 = mHalfEdge[mHalfEdge[mHalfEdge[mUnstableEdge]->getNext()]->getNext()]->getPoint();
    if ( ptnr1 == -1 || ptnr2 == -1 || ptnr3 == -1 )
    {
      return false;
    }
    p1.setX( mPointVector[ptnr1]->x() );
    p1.setY( mPointVector[ptnr1]->y() );
    p1.setZ( mPointVector[ptnr1]->z() );
    p2.setX( mPointVector[ptnr2]->x() );
    p2.setY( mPointVector[ptnr2]->y() );
    p2.setZ( mPointVector[ptnr2]->z() );
    p3.setX( mPointVector[ptnr3]->x() );
    p3.setY( mPointVector[ptnr3]->y() );
    p3.setZ( mPointVector[ptnr3]->z() );
    n1 = ptnr1;
    n2 = ptnr2;
    n3 = ptnr3;
    return true;
  }
  else//problems
  {
    //QgsDebugMsg( QStringLiteral( "problem: the edge is: %1" ).arg( edge ) );
    return false;
  }
}

bool QgsDualEdgeTriangulation::triangleVertices( double x, double y, QgsPoint &p1, QgsPoint &p2, QgsPoint &p3 )
{
  if ( mPointVector.size() < 3 )
  {
    return false;
  }

  const QgsPoint point( x, y, 0 );
  const int edge = baseEdgeOfTriangle( point );
  if ( edge == -10 )//the point is outside the convex hull
  {
    return false;
  }
  else if ( edge >= 0 )//the point is inside the convex hull
  {
    const int ptnr1 = mHalfEdge[edge]->getPoint();
    const int ptnr2 = mHalfEdge[mHalfEdge[edge]->getNext()]->getPoint();
    const int ptnr3 = mHalfEdge[mHalfEdge[mHalfEdge[edge]->getNext()]->getNext()]->getPoint();
    p1.setX( mPointVector[ptnr1]->x() );
    p1.setY( mPointVector[ptnr1]->y() );
    p1.setZ( mPointVector[ptnr1]->z() );
    p2.setX( mPointVector[ptnr2]->x() );
    p2.setY( mPointVector[ptnr2]->y() );
    p2.setZ( mPointVector[ptnr2]->z() );
    p3.setX( mPointVector[ptnr3]->x() );
    p3.setY( mPointVector[ptnr3]->y() );
    p3.setZ( mPointVector[ptnr3]->z() );
    return true;
  }
  else if ( edge == -20 )//the point is exactly on an edge
  {
    const int ptnr1 = mHalfEdge[mEdgeWithPoint]->getPoint();
    const int ptnr2 = mHalfEdge[mHalfEdge[mEdgeWithPoint]->getNext()]->getPoint();
    const int ptnr3 = mHalfEdge[mHalfEdge[mHalfEdge[mEdgeWithPoint]->getNext()]->getNext()]->getPoint();
    if ( ptnr1 == -1 || ptnr2 == -1 || ptnr3 == -1 )
    {
      return false;
    }
    p1.setX( mPointVector[ptnr1]->x() );
    p1.setY( mPointVector[ptnr1]->y() );
    p1.setZ( mPointVector[ptnr1]->z() );
    p2.setX( mPointVector[ptnr2]->x() );
    p2.setY( mPointVector[ptnr2]->y() );
    p2.setZ( mPointVector[ptnr2]->z() );
    p3.setX( mPointVector[ptnr3]->x() );
    p3.setY( mPointVector[ptnr3]->y() );
    p3.setZ( mPointVector[ptnr3]->z() );
    return true;
  }
  else if ( edge == -25 )//x and y are the coordinates of an existing point
  {
    const int edge1 = baseEdgeOfPoint( mTwiceInsPoint );
    const int edge2 = mHalfEdge[edge1]->getNext();
    const int edge3 = mHalfEdge[edge2]->getNext();
    const int ptnr1 = mHalfEdge[edge1]->getPoint();
    const int ptnr2 = mHalfEdge[edge2]->getPoint();
    const int ptnr3 = mHalfEdge[edge3]->getPoint();
    if ( ptnr1 == -1 || ptnr2 == -1 || ptnr3 == -1 )
    {
      return false;
    }
    p1.setX( mPointVector[ptnr1]->x() );
    p1.setY( mPointVector[ptnr1]->y() );
    p1.setZ( mPointVector[ptnr1]->z() );
    p2.setX( mPointVector[ptnr2]->x() );
    p2.setY( mPointVector[ptnr2]->y() );
    p2.setZ( mPointVector[ptnr2]->z() );
    p3.setX( mPointVector[ptnr3]->x() );
    p3.setY( mPointVector[ptnr3]->y() );
    p3.setZ( mPointVector[ptnr3]->z() );
    return true;
  }
  else if ( edge == -5 )//numerical problems in 'baseEdgeOfTriangle'
  {
    const int ptnr1 = mHalfEdge[mUnstableEdge]->getPoint();
    const int ptnr2 = mHalfEdge[mHalfEdge[mUnstableEdge]->getNext()]->getPoint();
    const int ptnr3 = mHalfEdge[mHalfEdge[mHalfEdge[mUnstableEdge]->getNext()]->getNext()]->getPoint();
    if ( ptnr1 == -1 || ptnr2 == -1 || ptnr3 == -1 )
    {
      return false;
    }
    p1.setX( mPointVector[ptnr1]->x() );
    p1.setY( mPointVector[ptnr1]->y() );
    p1.setZ( mPointVector[ptnr1]->z() );
    p2.setX( mPointVector[ptnr2]->x() );
    p2.setY( mPointVector[ptnr2]->y() );
    p2.setZ( mPointVector[ptnr2]->z() );
    p3.setX( mPointVector[ptnr3]->x() );
    p3.setY( mPointVector[ptnr3]->y() );
    p3.setZ( mPointVector[ptnr3]->z() );
    return true;
  }
  else//problems
  {
    return false;
  }
}

unsigned int QgsDualEdgeTriangulation::insertEdge( int dual, int next, int point, bool mbreak, bool forced )
{
  HalfEdge *edge = new HalfEdge( dual, next, point, mbreak, forced );
  mHalfEdge.append( edge );
  return mHalfEdge.count() - 1;

}

static bool altitudeTriangleIsSmall( const QgsPoint &pointBase1, const QgsPoint &pointBase2, const QgsPoint &pt3, double tolerance )
{
  // Compare the altitude of the triangle defined by base points and a third point with tolerance. Return true if the altitude < tolerance
  const double x1 = pointBase1.x();
  const double y1 = pointBase1.y();
  const double x2 = pointBase2.x();
  const double y2 = pointBase2.y();
  const double X = pt3.x();
  const double Y = pt3.y();
  QgsPoint projectedPoint;

  double nx, ny; //normal vector

  nx = y2 - y1;
  ny = -( x2 - x1 );

  double t;
  t = ( X * ny - Y * nx - x1 * ny + y1 * nx ) / ( ( x2 - x1 ) * ny - ( y2 - y1 ) * nx );
  projectedPoint.setX( x1 + t * ( x2 - x1 ) );
  projectedPoint.setY( y1 + t * ( y2 - y1 ) );

  return pt3.distance( projectedPoint ) < tolerance;
}

int QgsDualEdgeTriangulation::insertForcedSegment( int p1, int p2, QgsInterpolator::SourceType segmentType )
{
  if ( p1 == p2 )
  {
    return 0;
  }

  QgsPoint *point1 = mPointVector.at( p1 );
  QgsPoint *point2 = mPointVector.at( p2 );

  //list with (half of) the crossed edges
  QList<int> crossedEdges;

  //an edge pointing to p1
  int pointingEdge = 0;

  pointingEdge = baseEdgeOfPoint( p1 );

  if ( pointingEdge == -1 )
  {
    return -100;//return an error code
  }

  //go around p1 and find out, if the segment already exists and if not, which is the first cutted edge
  int actEdge = mHalfEdge[pointingEdge]->getDual();
  const int firstActEdge = actEdge;
  //number to prevent endless loops
  int control = 0;

  do //if it's an endless loop, something went wrong
  {
    control += 1;
    if ( control > 100 )
    {
      //QgsDebugMsg( QStringLiteral( "warning, endless loop" ) );
      return -100;//return an error code
    }

    if ( mHalfEdge[actEdge]->getPoint() == -1 )//actEdge points to the virtual point
    {
      actEdge = mHalfEdge[mHalfEdge[mHalfEdge[actEdge]->getNext()]->getNext()]->getDual();
      continue;
    }

    //test, if actEdge is already the forced edge
    if ( mHalfEdge[actEdge]->getPoint() == p2 )
    {
      mHalfEdge[actEdge]->setForced( true );
      mHalfEdge[actEdge]->setBreak( segmentType == QgsInterpolator::SourceBreakLines );
      mHalfEdge[mHalfEdge[actEdge]->getDual()]->setForced( true );
      mHalfEdge[mHalfEdge[actEdge]->getDual()]->setBreak( segmentType == QgsInterpolator::SourceBreakLines );
      return actEdge;
    }

    //test, if the forced segment is a multiple of actEdge and if the direction is the same
    if ( /*lines are parallel*/( point2->y() - point1->y() ) / ( mPointVector[mHalfEdge[actEdge]->getPoint()]->y() - point1->y() ) == ( point2->x() - point1->x() ) / ( mPointVector[mHalfEdge[actEdge]->getPoint()]->x() - point1->x() )
                               && ( ( point2->y() - point1->y() ) >= 0 ) == ( ( mPointVector[mHalfEdge[actEdge]->getPoint()]->y() - point1->y() ) > 0 )
                               && ( ( point2->x() - point1->x() ) >= 0 ) == ( ( mPointVector[mHalfEdge[actEdge]->getPoint()]->x() - point1->x() ) > 0 ) )
    {
      //mark actedge and Dual(actedge) as forced, reset p1 and start the method from the beginning
      mHalfEdge[actEdge]->setForced( true );
      mHalfEdge[actEdge]->setBreak( segmentType == QgsInterpolator::SourceBreakLines );
      mHalfEdge[mHalfEdge[actEdge]->getDual()]->setForced( true );
      mHalfEdge[mHalfEdge[actEdge]->getDual()]->setBreak( segmentType == QgsInterpolator::SourceBreakLines );
      const int a = insertForcedSegment( mHalfEdge[actEdge]->getPoint(), p2, segmentType );
      return a;
    }

    //test, if the forced segment intersects Next(actEdge)
    const int oppositeEdge = mHalfEdge[actEdge]->getNext();

    if ( mHalfEdge[oppositeEdge]->getPoint() == -1 || mHalfEdge[mHalfEdge[oppositeEdge]->getDual()]->getPoint() == -1 ) //intersection with line to the virtual point makes no sense
    {
      actEdge = mHalfEdge[mHalfEdge[oppositeEdge]->getNext()]->getDual(); //continue with the next edge around p1
      continue;
    }

    QgsPoint *oppositePoint1 = mPointVector[mHalfEdge[oppositeEdge]->getPoint()];
    QgsPoint *oppositePoint2 = mPointVector[mHalfEdge[mHalfEdge[oppositeEdge]->getDual()]->getPoint()];

    if ( altitudeTriangleIsSmall( *oppositePoint1, *oppositePoint2, *point1, oppositePoint1->distance( *oppositePoint2 ) / 500 ) )
    {
      // to much risks to do something, go away
      return -100;
    }

    if ( MathUtils::lineIntersection( point1,
                                      point2,
                                      mPointVector[mHalfEdge[oppositeEdge]->getPoint()],
                                      mPointVector[mHalfEdge[mHalfEdge[oppositeEdge]->getDual()]->getPoint()] ) )
    {
      if ( mHalfEdge[mHalfEdge[actEdge]->getNext()]->getForced() && mForcedCrossBehavior == QgsTriangulation::SnappingTypeVertex )//if the crossed edge is a forced edge, we have to snap the forced line to the next node
      {
        QgsPoint crosspoint( 0, 0, 0 );
        int p3, p4;
        p3 = mHalfEdge[mHalfEdge[actEdge]->getNext()]->getPoint();
        p4 = mHalfEdge[mHalfEdge[mHalfEdge[actEdge]->getNext()]->getDual()]->getPoint();
        MathUtils::lineIntersection( point1, point2, mPointVector[p3], mPointVector[p4], &crosspoint );
        const double dista = std::sqrt( ( crosspoint.x() - mPointVector[p3]->x() ) * ( crosspoint.x() - mPointVector[p3]->x() ) + ( crosspoint.y() - mPointVector[p3]->y() ) * ( crosspoint.y() - mPointVector[p3]->y() ) );
        const double distb = std::sqrt( ( crosspoint.x() - mPointVector[p4]->x() ) * ( crosspoint.x() - mPointVector[p4]->x() ) + ( crosspoint.y() - mPointVector[p4]->y() ) * ( crosspoint.y() - mPointVector[p4]->y() ) );
        if ( dista <= distb )
        {
          insertForcedSegment( p1, p3, segmentType );
          const int e = insertForcedSegment( p3, p2, segmentType );
          return e;
        }
        else if ( distb <= dista )
        {
          insertForcedSegment( p1, p4, segmentType );
          const int e = insertForcedSegment( p4, p2, segmentType );
          return e;
        }
      }

      if ( mHalfEdge[mHalfEdge[actEdge]->getNext()]->getForced() && mForcedCrossBehavior == QgsTriangulation::InsertVertex )//if the crossed edge is a forced edge, we have to insert a new vertice on this edge
      {
        QgsPoint crosspoint( 0, 0, 0 );
        int p3, p4;
        p3 = mHalfEdge[mHalfEdge[actEdge]->getNext()]->getPoint();
        p4 = mHalfEdge[mHalfEdge[mHalfEdge[actEdge]->getNext()]->getDual()]->getPoint();
        MathUtils::lineIntersection( mPointVector[p1], mPointVector[p2], mPointVector[p3], mPointVector[p4], &crosspoint );
        const double distpart = std::sqrt( ( crosspoint.x() - mPointVector[p4]->x() ) * ( crosspoint.x() - mPointVector[p4]->x() ) + ( crosspoint.y() - mPointVector[p4]->y() ) * ( crosspoint.y() - mPointVector[p4]->y() ) );
        const double disttot = std::sqrt( ( mPointVector[p3]->x() - mPointVector[p4]->x() ) * ( mPointVector[p3]->x() - mPointVector[p4]->x() ) + ( mPointVector[p3]->y() - mPointVector[p4]->y() ) * ( mPointVector[p3]->y() - mPointVector[p4]->y() ) );
        const float frac = distpart / disttot;

        if ( frac == 0 || frac == 1 )//just in case MathUtils::lineIntersection does not work as expected
        {
          if ( frac == 0 )
          {
            //mark actEdge and Dual(actEdge) as forced, reset p1 and start the method from the beginning
            mHalfEdge[actEdge]->setForced( true );
            mHalfEdge[actEdge]->setBreak( segmentType == QgsInterpolator::SourceBreakLines );
            mHalfEdge[mHalfEdge[actEdge]->getDual()]->setForced( true );
            mHalfEdge[mHalfEdge[actEdge]->getDual()]->setBreak( segmentType == QgsInterpolator::SourceBreakLines );
            const int a = insertForcedSegment( p4, p2, segmentType );
            return a;
          }
          else if ( frac == 1 )
          {
            //mark actEdge and Dual(actEdge) as forced, reset p1 and start the method from the beginning
            mHalfEdge[actEdge]->setForced( true );
            mHalfEdge[actEdge]->setBreak( segmentType == QgsInterpolator::SourceBreakLines );
            mHalfEdge[mHalfEdge[actEdge]->getDual()]->setForced( true );
            mHalfEdge[mHalfEdge[actEdge]->getDual()]->setBreak( segmentType == QgsInterpolator::SourceBreakLines );
            if ( p3 != p2 )
            {
              const int a = insertForcedSegment( p3, p2, segmentType );
              return a;
            }
            else
            {
              return actEdge;
            }
          }

        }
        else
        {
          const int newpoint = splitHalfEdge( mHalfEdge[actEdge]->getNext(), frac );
          insertForcedSegment( p1, newpoint, segmentType );
          const int e = insertForcedSegment( newpoint, p2, segmentType );
          return e;
        }
      }

      //add the first HalfEdge to the list of crossed edges
      crossedEdges.append( oppositeEdge );
      break;
    }
    actEdge = mHalfEdge[mHalfEdge[oppositeEdge]->getNext()]->getDual(); //continue with the next edge around p1
  }
  while ( actEdge != firstActEdge );

  if ( crossedEdges.isEmpty() ) //nothing found due to rounding error, better to go away!
    return -100;
  int lastEdgeOppositePointIndex = mHalfEdge[mHalfEdge[mHalfEdge[crossedEdges.last()]->getDual()]->getNext()]->getPoint();

  //we found the first edge, terminated the method or called the method with other points. Lets search for all the other crossed edges
  while ( lastEdgeOppositePointIndex != p2 )
  {
    QgsPoint *lastEdgePoint1 = mPointVector[mHalfEdge[mHalfEdge[crossedEdges.last()]->getDual()]->getPoint()];
    QgsPoint *lastEdgePoint2 = mPointVector[mHalfEdge[mHalfEdge[mHalfEdge[mHalfEdge[crossedEdges.last()]->getDual()]->getNext()]->getNext()]->getPoint()];
    QgsPoint *lastEdgeOppositePoint = mPointVector[lastEdgeOppositePointIndex];

    if ( MathUtils::lineIntersection( lastEdgePoint1, lastEdgeOppositePoint,
                                      point1, point2 ) )
    {
      if ( mHalfEdge[mHalfEdge[mHalfEdge[crossedEdges.last()]->getDual()]->getNext()]->getForced() && mForcedCrossBehavior == QgsTriangulation::SnappingTypeVertex )//if the crossed edge is a forced edge and mForcedCrossBehavior is SnappingType_VERTICE, we have to snap the forced line to the next node
      {
        QgsPoint crosspoint( 0, 0, 0 );
        int p3, p4;
        p3 = mHalfEdge[mHalfEdge[crossedEdges.last()]->getDual()]->getPoint();
        p4 = mHalfEdge[mHalfEdge[mHalfEdge[crossedEdges.last()]->getDual()]->getNext()]->getPoint();
        MathUtils::lineIntersection( point1, point2, mPointVector[p3], mPointVector[p4], &crosspoint );
        const double dista = std::sqrt( ( crosspoint.x() - mPointVector[p3]->x() ) * ( crosspoint.x() - mPointVector[p3]->x() ) + ( crosspoint.y() - mPointVector[p3]->y() ) * ( crosspoint.y() - mPointVector[p3]->y() ) );
        const double distb = std::sqrt( ( crosspoint.x() - mPointVector[p4]->x() ) * ( crosspoint.x() - mPointVector[p4]->x() ) + ( crosspoint.y() - mPointVector[p4]->y() ) * ( crosspoint.y() - mPointVector[p4]->y() ) );
        if ( dista <= distb )
        {
          insertForcedSegment( p1, p3, segmentType );
          const int e = insertForcedSegment( p3, p2, segmentType );
          return e;
        }
        else if ( distb <= dista )
        {
          insertForcedSegment( p1, p4, segmentType );
          const int e = insertForcedSegment( p4, p2, segmentType );
          return e;
        }
      }
      else if ( mHalfEdge[mHalfEdge[mHalfEdge[crossedEdges.last()]->getDual()]->getNext()]->getForced() && mForcedCrossBehavior == QgsTriangulation::InsertVertex )//if the crossed edge is a forced edge, we have to insert a new vertice on this edge
      {
        QgsPoint crosspoint( 0, 0, 0 );
        int p3, p4;
        p3 = mHalfEdge[mHalfEdge[crossedEdges.last()]->getDual()]->getPoint();
        p4 = mHalfEdge[mHalfEdge[mHalfEdge[crossedEdges.last()]->getDual()]->getNext()]->getPoint();
        MathUtils::lineIntersection( point1, point2, mPointVector[p3], mPointVector[p4], &crosspoint );
        const double distpart = std::sqrt( ( crosspoint.x() - mPointVector[p3]->x() ) * ( crosspoint.x() - mPointVector[p3]->x() ) + ( crosspoint.y() - mPointVector[p3]->y() ) * ( crosspoint.y() - mPointVector[p3]->y() ) );
        const double disttot = std::sqrt( ( mPointVector[p3]->x() - mPointVector[p4]->x() ) * ( mPointVector[p3]->x() - mPointVector[p4]->x() ) + ( mPointVector[p3]->y() - mPointVector[p4]->y() ) * ( mPointVector[p3]->y() - mPointVector[p4]->y() ) );
        const float frac = distpart / disttot;
        if ( frac == 0 || frac == 1 )
        {
          break;//seems that a roundoff error occurred. We found the endpoint
        }
        const int newpoint = splitHalfEdge( mHalfEdge[mHalfEdge[crossedEdges.last()]->getDual()]->getNext(), frac );
        insertForcedSegment( p1, newpoint, segmentType );
        const int e = insertForcedSegment( newpoint, p2, segmentType );
        return e;
      }

      crossedEdges.append( mHalfEdge[mHalfEdge[crossedEdges.last()]->getDual()]->getNext() );
    }
    else if ( MathUtils::lineIntersection( lastEdgePoint2, lastEdgeOppositePoint,
                                           point1, point2 ) )
    {
      if ( mHalfEdge[mHalfEdge[mHalfEdge[mHalfEdge[crossedEdges.last()]->getDual()]->getNext()]->getNext()]->getForced() && mForcedCrossBehavior == QgsTriangulation::SnappingTypeVertex )//if the crossed edge is a forced edge and mForcedCrossBehavior is SnappingType_VERTICE, we have to snap the forced line to the next node
      {
        QgsPoint crosspoint( 0, 0, 0 );
        int p3, p4;
        p3 = mHalfEdge[mHalfEdge[mHalfEdge[crossedEdges.last()]->getDual()]->getNext()]->getPoint();
        p4 = mHalfEdge[mHalfEdge[mHalfEdge[mHalfEdge[crossedEdges.last()]->getDual()]->getNext()]->getNext()]->getPoint();
        MathUtils::lineIntersection( mPointVector[p1], mPointVector[p2], mPointVector[p3], mPointVector[p4], &crosspoint );
        const double dista = std::sqrt( ( crosspoint.x() - mPointVector[p3]->x() ) * ( crosspoint.x() - mPointVector[p3]->x() ) + ( crosspoint.y() - mPointVector[p3]->y() ) * ( crosspoint.y() - mPointVector[p3]->y() ) );
        const double distb = std::sqrt( ( crosspoint.x() - mPointVector[p4]->x() ) * ( crosspoint.x() - mPointVector[p4]->x() ) + ( crosspoint.y() - mPointVector[p4]->y() ) * ( crosspoint.y() - mPointVector[p4]->y() ) );
        if ( dista <= distb )
        {
          insertForcedSegment( p1, p3, segmentType );
          const int e = insertForcedSegment( p3, p2, segmentType );
          return e;
        }
        else if ( distb <= dista )
        {
          insertForcedSegment( p1, p4, segmentType );
          const int e = insertForcedSegment( p4, p2, segmentType );
          return e;
        }
      }
      else if ( mHalfEdge[mHalfEdge[mHalfEdge[mHalfEdge[crossedEdges.last()]->getDual()]->getNext()]->getNext()]->getForced() && mForcedCrossBehavior == QgsTriangulation::InsertVertex )//if the crossed edge is a forced edge, we have to insert a new vertice on this edge
      {
        QgsPoint crosspoint( 0, 0, 0 );
        int p3, p4;
        p3 = mHalfEdge[mHalfEdge[mHalfEdge[crossedEdges.last()]->getDual()]->getNext()]->getPoint();
        p4 = mHalfEdge[mHalfEdge[mHalfEdge[mHalfEdge[crossedEdges.last()]->getDual()]->getNext()]->getNext()]->getPoint();
        MathUtils::lineIntersection( point1, point2, mPointVector[p3], mPointVector[p4], &crosspoint );
        const double distpart = std::sqrt( ( crosspoint.x() - mPointVector[p3]->x() ) * ( crosspoint.x() - mPointVector[p3]->x() ) + ( crosspoint.y() - mPointVector[p3]->y() ) * ( crosspoint.y() - mPointVector[p3]->y() ) );
        const double disttot = std::sqrt( ( mPointVector[p3]->x() - mPointVector[p4]->x() ) * ( mPointVector[p3]->x() - mPointVector[p4]->x() ) + ( mPointVector[p3]->y() - mPointVector[p4]->y() ) * ( mPointVector[p3]->y() - mPointVector[p4]->y() ) );
        const float frac = distpart / disttot;
        if ( frac == 0 || frac == 1 )
        {
          break;//seems that a roundoff error occurred. We found the endpoint
        }
        const int newpoint = splitHalfEdge( mHalfEdge[mHalfEdge[mHalfEdge[crossedEdges.last()]->getDual()]->getNext()]->getNext(), frac );
        insertForcedSegment( p1, newpoint, segmentType );
        const int e = insertForcedSegment( newpoint, p2, segmentType );
        return e;
      }

      crossedEdges.append( mHalfEdge[mHalfEdge[mHalfEdge[crossedEdges.last()]->getDual()]->getNext()]->getNext() );
    }
    else
    {
      //no intersection found, surely due to rounding error or something else wrong, better to give up!
      return -100;
    }
    lastEdgeOppositePointIndex = mHalfEdge[mHalfEdge[mHalfEdge[crossedEdges.last()]->getDual()]->getNext()]->getPoint();
  }

  // Last check before construct the breakline
  QgsPoint *lastEdgePoint1 = mPointVector[mHalfEdge[mHalfEdge[crossedEdges.last()]->getDual()]->getPoint()];
  QgsPoint *lastEdgePoint2 = mPointVector[mHalfEdge[mHalfEdge[mHalfEdge[mHalfEdge[crossedEdges.last()]->getDual()]->getNext()]->getNext()]->getPoint()];
  QgsPoint *lastEdgeOppositePoint = mPointVector[lastEdgeOppositePointIndex];
  if ( altitudeTriangleIsSmall( *lastEdgePoint1, *lastEdgePoint2, *lastEdgeOppositePoint, lastEdgePoint1->distance( *lastEdgePoint2 ) / 500 ) )
    return -100;

  //set the flags 'forced' and 'break' to false for every edge and dualedge of 'crossEdges'
  QList<int>::const_iterator iter;
  for ( iter = crossedEdges.constBegin(); iter != crossedEdges.constEnd(); ++iter )
  {
    mHalfEdge[( *( iter ) )]->setForced( false );
    mHalfEdge[( *( iter ) )]->setBreak( false );
    mHalfEdge[mHalfEdge[( *( iter ) )]->getDual()]->setForced( false );
    mHalfEdge[mHalfEdge[( *( iter ) )]->getDual()]->setBreak( false );
  }

  //crossed edges is filled, now the two polygons to be retriangulated can be build

  QList<int> freelist = crossedEdges;//copy the list with the crossed edges to remove the edges already reused

  //create the left polygon as a list of the numbers of the halfedges
  QList<int> leftPolygon;
  QList<int> rightPolygon;

  //insert the forced edge and enter the corresponding halfedges as the first edges in the left and right polygons. The nexts and points are set later because of the algorithm to build two polygons from 'crossedEdges'
  const int firstedge = freelist.first();//edge pointing from p1 to p2
  mHalfEdge[firstedge]->setForced( true );
  mHalfEdge[firstedge]->setBreak( segmentType == QgsInterpolator::SourceBreakLines );
  leftPolygon.append( firstedge );
  const int dualfirstedge = mHalfEdge[freelist.first()]->getDual();//edge pointing from p2 to p1
  mHalfEdge[dualfirstedge]->setForced( true );
  mHalfEdge[dualfirstedge]->setBreak( segmentType == QgsInterpolator::SourceBreakLines );
  rightPolygon.append( dualfirstedge );
  freelist.pop_front();//delete the first entry from the freelist

  //finish the polygon on the left side
  int actpointl = p2;
  QList<int>::const_iterator leftiter; //todo: is there a better way to set an iterator to the last list element?
  leftiter = crossedEdges.constEnd();
  --leftiter;
  while ( true )
  {
    const int newpoint = mHalfEdge[mHalfEdge[mHalfEdge[mHalfEdge[( *leftiter )]->getDual()]->getNext()]->getNext()]->getPoint();
    if ( newpoint != actpointl )
    {
      //insert the edge into the leftPolygon
      actpointl = newpoint;
      const int theedge = mHalfEdge[mHalfEdge[mHalfEdge[( *leftiter )]->getDual()]->getNext()]->getNext();
      leftPolygon.append( theedge );
    }
    if ( leftiter == crossedEdges.constBegin() )
    {break;}
    --leftiter;
  }

  //insert the last element into leftPolygon
  leftPolygon.append( mHalfEdge[crossedEdges.first()]->getNext() );

  //finish the polygon on the right side
  QList<int>::const_iterator rightiter;
  int actpointr = p1;
  for ( rightiter = crossedEdges.constBegin(); rightiter != crossedEdges.constEnd(); ++rightiter )
  {
    const int newpoint = mHalfEdge[mHalfEdge[mHalfEdge[( *rightiter )]->getNext()]->getNext()]->getPoint();
    if ( newpoint != actpointr )
    {
      //insert the edge into the right polygon
      actpointr = newpoint;
      const int theedge = mHalfEdge[mHalfEdge[( *rightiter )]->getNext()]->getNext();
      rightPolygon.append( theedge );
    }
  }


  //insert the last element into rightPolygon
  rightPolygon.append( mHalfEdge[mHalfEdge[crossedEdges.last()]->getDual()]->getNext() );
  mHalfEdge[rightPolygon.last()]->setNext( dualfirstedge );//set 'Next' of the last edge to dualfirstedge

  //set the necessary nexts of leftPolygon(except the first)
  int actedgel = leftPolygon[1];
  leftiter = leftPolygon.constBegin();
  leftiter += 2;
  for ( ; leftiter != leftPolygon.constEnd(); ++leftiter )
  {
    mHalfEdge[actedgel]->setNext( ( *leftiter ) );
    actedgel = ( *leftiter );
  }

  //set all the necessary nexts of rightPolygon
  int actedger = rightPolygon[1];
  rightiter = rightPolygon.constBegin();
  rightiter += 2;
  for ( ; rightiter != rightPolygon.constEnd(); ++rightiter )
  {
    mHalfEdge[actedger]->setNext( ( *rightiter ) );
    actedger = ( *( rightiter ) );
  }


  //setNext and setPoint for the forced edge because this would disturb the building of 'leftpoly' and 'rightpoly' otherwise
  mHalfEdge[leftPolygon.first()]->setNext( ( *( ++( leftiter = leftPolygon.constBegin() ) ) ) );
  mHalfEdge[leftPolygon.first()]->setPoint( p2 );
  mHalfEdge[leftPolygon.last()]->setNext( firstedge );
  mHalfEdge[rightPolygon.first()]->setNext( ( *( ++( rightiter = rightPolygon.constBegin() ) ) ) );
  mHalfEdge[rightPolygon.first()]->setPoint( p1 );
  mHalfEdge[rightPolygon.last()]->setNext( dualfirstedge );

  triangulatePolygon( &leftPolygon, &freelist, firstedge );
  triangulatePolygon( &rightPolygon, &freelist, dualfirstedge );

  //optimisation of the new edges
  for ( iter = crossedEdges.constBegin(); iter != crossedEdges.constEnd(); ++iter )
  {
    checkSwapRecursively( ( *( iter ) ), 0 );
  }

  return leftPolygon.first();
}

void QgsDualEdgeTriangulation::setForcedCrossBehavior( QgsTriangulation::ForcedCrossBehavior b )
{
  mForcedCrossBehavior = b;
}

void QgsDualEdgeTriangulation::setTriangleInterpolator( TriangleInterpolator *interpolator )
{
  mTriangleInterpolator = interpolator;
}

void QgsDualEdgeTriangulation::eliminateHorizontalTriangles()
{
  //QgsDebugMsg( QStringLiteral( "am in eliminateHorizontalTriangles" ) );
  const double minangle = 0;//minimum angle for swapped triangles. If triangles generated by a swap would have a minimum angle (in degrees) below that value, the swap will not be done.

  while ( true )
  {
    bool swapped = false;//flag which allows exiting the loop
    bool *control = new bool[mHalfEdge.count()];//controlarray

    for ( int i = 0; i <= mHalfEdge.count() - 1; i++ )
    {
      control[i] = false;
    }


    for ( int i = 0; i <= mHalfEdge.count() - 1; i++ )
    {
      if ( control[i] )//edge has already been examined
      {
        continue;
      }

      int e1, e2, e3;//numbers of the three edges
      e1 = i;
      e2 = mHalfEdge[e1]->getNext();
      e3 = mHalfEdge[e2]->getNext();

      int p1, p2, p3;//numbers of the three points
      p1 = mHalfEdge[e1]->getPoint();
      p2 = mHalfEdge[e2]->getPoint();
      p3 = mHalfEdge[e3]->getPoint();

      //skip the iteration, if one point is the virtual point
      if ( p1 == -1 || p2 == -1 || p3 == -1 )
      {
        control[e1] = true;
        control[e2] = true;
        control[e3] = true;
        continue;
      }

      double el1, el2, el3;//elevations of the points
      el1 = mPointVector[p1]->z();
      el2 = mPointVector[p2]->z();
      el3 = mPointVector[p3]->z();

      if ( el1 == el2 && el2 == el3 )//we found a horizontal triangle
      {
        //swap edges if it is possible, if it would remove the horizontal triangle and if the minimum angle generated by the swap is high enough
        if ( swapPossible( ( uint )e1 ) && mPointVector[mHalfEdge[mHalfEdge[mHalfEdge[e1]->getDual()]->getNext()]->getPoint()]->z() != el1 && swapMinAngle( e1 ) > minangle )
        {
          doOnlySwap( ( uint )e1 );
          swapped = true;
        }
        else if ( swapPossible( ( uint )e2 ) && mPointVector[mHalfEdge[mHalfEdge[mHalfEdge[e2]->getDual()]->getNext()]->getPoint()]->z() != el2 && swapMinAngle( e2 ) > minangle )
        {
          doOnlySwap( ( uint )e2 );
          swapped = true;
        }
        else if ( swapPossible( ( uint )e3 ) && mPointVector[mHalfEdge[mHalfEdge[mHalfEdge[e3]->getDual()]->getNext()]->getPoint()]->z() != el3 && swapMinAngle( e3 ) > minangle )
        {
          doOnlySwap( ( uint )e3 );
          swapped = true;
        }
        control[e1] = true;
        control[e2] = true;
        control[e3] = true;
        continue;
      }
      else//no horizontal triangle, go to the next one
      {
        control[e1] = true;
        control[e2] = true;
        control[e3] = true;
        continue;
      }
    }
    if ( !swapped )
    {
      delete[] control;
      break;
    }
    delete[] control;
  }

  //QgsDebugMsg( QStringLiteral( "end of method" ) );
}

void QgsDualEdgeTriangulation::ruppertRefinement()
{
  //minimum angle
  const double mintol = 17;//refinement stops after the minimum angle reached this tolerance

  //data structures
  std::map<int, double> edge_angle;//search tree with the edge number as key
  std::multimap<double, int> angle_edge;//multimap (map with not unique keys) with angle as key
  QSet<int> dontexamine;//search tree containing the edges which do not have to be examined (because of numerical problems)


  //first, go through all the forced edges and subdivide if they are encroached by a point
  bool stop = false;//flag to ensure that the for-loop is repeated until no half edge is split any more

  while ( !stop )
  {
    stop = true;
    const int nhalfedges = mHalfEdge.count();

    for ( int i = 0; i < nhalfedges - 1; i++ )
    {
      const int next = mHalfEdge[i]->getNext();
      const int nextnext = mHalfEdge[next]->getNext();

      if ( mHalfEdge[next]->getPoint() != -1 && ( mHalfEdge[i]->getForced() || mHalfEdge[mHalfEdge[mHalfEdge[i]->getDual()]->getNext()]->getPoint() == -1 ) )//check for encroached points on forced segments and segments on the inner side of the convex hull, but don't consider edges on the outer side of the convex hull
      {
        if ( !( ( mHalfEdge[next]->getForced() || edgeOnConvexHull( next ) ) || ( mHalfEdge[nextnext]->getForced() || edgeOnConvexHull( nextnext ) ) ) ) //don't consider triangles where all three edges are forced edges or hull edges
        {
          //test for encroachment
          while ( MathUtils::inDiametral( mPointVector[mHalfEdge[mHalfEdge[i]->getDual()]->getPoint()], mPointVector[mHalfEdge[i]->getPoint()], mPointVector[mHalfEdge[next]->getPoint()] ) )
          {
            //split segment
            const int pointno = splitHalfEdge( i, 0.5 );
            Q_UNUSED( pointno )
            stop = false;
          }
        }
      }
    }
  }

  //examine the triangulation for angles below the minimum and insert the edges into angle_edge and edge_angle, except the small angle is between forced segments or convex hull edges
  double angle;//angle between edge i and the consecutive edge
  int p1, p2, p3;//numbers of the triangle points
  for ( int i = 0; i < mHalfEdge.count() - 1; i++ )
  {
    p1 = mHalfEdge[mHalfEdge[i]->getDual()]->getPoint();
    p2 = mHalfEdge[i]->getPoint();
    p3 = mHalfEdge[mHalfEdge[i]->getNext()]->getPoint();

    if ( p1 == -1 || p2 == -1 || p3 == -1 )//don't consider triangles with the virtual point
    {
      continue;
    }
    angle = MathUtils::angle( mPointVector[p1], mPointVector[p2], mPointVector[p3], mPointVector[p2] );

    bool twoforcededges;//flag to decide, if edges should be added to the maps. Do not add them if true


    twoforcededges = ( mHalfEdge[i]->getForced() || edgeOnConvexHull( i ) ) && ( mHalfEdge[mHalfEdge[i]->getNext()]->getForced() || edgeOnConvexHull( mHalfEdge[i]->getNext() ) );

    if ( angle < mintol && !twoforcededges )
    {
      edge_angle.insert( std::make_pair( i, angle ) );
      angle_edge.insert( std::make_pair( angle, i ) );
    }
  }

  //debugging: print out all the angles below the minimum for a test
  for ( std::multimap<double, int>::const_iterator it = angle_edge.begin(); it != angle_edge.end(); ++it )
  {
    QgsDebugMsg( QStringLiteral( "angle: %1" ).arg( it->first ) );
  }


  double minangle = 0;//actual minimum angle
  int minedge;//first edge adjacent to the minimum angle
  int minedgenext;
  int minedgenextnext;

  QgsPoint circumcenter( 0, 0, 0 );

  while ( !edge_angle.empty() )
  {
    minangle = angle_edge.begin()->first;
    QgsDebugMsg( QStringLiteral( "minangle: %1" ).arg( minangle ) );
    minedge = angle_edge.begin()->second;
    minedgenext = mHalfEdge[minedge]->getNext();
    minedgenextnext = mHalfEdge[minedgenext]->getNext();

    //calculate the circumcenter
    if ( !MathUtils::circumcenter( mPointVector[mHalfEdge[minedge]->getPoint()], mPointVector[mHalfEdge[minedgenext]->getPoint()], mPointVector[mHalfEdge[minedgenextnext]->getPoint()], &circumcenter ) )
    {
      QgsDebugMsg( QStringLiteral( "warning, calculation of circumcenter failed" ) );
      //put all three edges to dontexamine and remove them from the other maps
      dontexamine.insert( minedge );
      edge_angle.erase( minedge );
      std::multimap<double, int>::iterator minedgeiter = angle_edge.find( minangle );
      while ( minedgeiter->second != minedge )
      {
        ++minedgeiter;
      }
      angle_edge.erase( minedgeiter );
      continue;
    }

    if ( !pointInside( circumcenter.x(), circumcenter.y() ) )
    {
      //put all three edges to dontexamine and remove them from the other maps
      QgsDebugMsg( QStringLiteral( "put circumcenter %1//%2 on dontexamine list because it is outside the convex hull" )
                   .arg( circumcenter.x() ).arg( circumcenter.y() ) );
      dontexamine.insert( minedge );
      edge_angle.erase( minedge );
      std::multimap<double, int>::iterator minedgeiter = angle_edge.find( minangle );
      while ( minedgeiter->second != minedge )
      {
        ++minedgeiter;
      }
      angle_edge.erase( minedgeiter );
      continue;
    }

    /*******find out, if any forced segment or convex hull segment is in the influence region of the circumcenter. In this case, split the segment********************/

    bool encroached = false;

#if 0 //slow version
    int numhalfedges = mHalfEdge.count();//begin slow version
    for ( int i = 0; i < numhalfedges; i++ )
    {
      if ( mHalfEdge[i]->getForced() || edgeOnConvexHull( i ) )
      {
        if ( MathUtils::inDiametral( mPointVector[mHalfEdge[i]->getPoint()], mPointVector[mHalfEdge[mHalfEdge[i]->getDual()]->getPoint()], &circumcenter ) )
        {
          encroached = true;
          //split segment
          QgsDebugMsg( QStringLiteral( "segment split" ) );
          int pointno = splitHalfEdge( i, 0.5 );

          //update dontexmine, angle_edge, edge_angle
          int pointingedge = baseEdgeOfPoint( pointno );

          int actedge = pointingedge;
          int ed1, ed2, ed3;//numbers of the three edges
          int pt1, pt2, pt3;//numbers of the three points
          double angle1, angle2, angle3;

          do
          {
            ed1 = mHalfEdge[actedge]->getDual();
            pt1 = mHalfEdge[ed1]->getPoint();
            ed2 = mHalfEdge[ed1]->getNext();
            pt2 = mHalfEdge[ed2]->getPoint();
            ed3 = mHalfEdge[ed2]->getNext();
            pt3 = mHalfEdge[ed3]->getPoint();
            actedge = ed3;

            if ( pt1 == -1 || pt2 == -1 || pt3 == -1 )//don't consider triangles with the virtual point
            {
              continue;
            }

            angle1 = MathUtils::angle( mPointVector[pt3], mPointVector[pt1], mPointVector[pt2], mPointVector[pt1] );
            angle2 = MathUtils::angle( mPointVector[pt1], mPointVector[pt2], mPointVector[pt3], mPointVector[pt2] );
            angle3 = MathUtils::angle( mPointVector[pt2], mPointVector[pt3], mPointVector[pt1], mPointVector[pt3] );

            //don't put the edges on the maps if two segments are forced or on a hull
            bool twoforcededges1, twoforcededges2, twoforcededges3;//flag to indicate, if angle1, angle2 and angle3 are between forced edges or hull edges

            if ( ( mHalfEdge[ed1]->getForced() || edgeOnConvexHull( ed1 ) ) && ( mHalfEdge[ed2]->getForced() || edgeOnConvexHull( ed2 ) ) )
            {
              twoforcededges1 = true;
            }
            else
            {
              twoforcededges1 = false;
            }

            if ( ( mHalfEdge[ed2]->getForced() || edgeOnConvexHull( ed2 ) ) && ( mHalfEdge[ed3]->getForced() || edgeOnConvexHull( ed3 ) ) )
            {
              twoforcededges2 = true;
            }
            else
            {
              twoforcededges2 = false;
            }

            if ( ( mHalfEdge[ed3]->getForced() || edgeOnConvexHull( ed3 ) ) && ( mHalfEdge[ed1]->getForced() || edgeOnConvexHull( ed1 ) ) )
            {
              twoforcededges3 = true;
            }
            else
            {
              twoforcededges3 = false;
            }

            //update the settings related to ed1
            QSet<int>::iterator ed1iter = dontexamine.find( ed1 );
            if ( ed1iter != dontexamine.end() )
            {
              //edge number is on dontexamine list
              dontexamine.erase( ed1iter );
            }
            else
            {
              //test, if it is on edge_angle and angle_edge and erase them if yes
              std::map<int, double>::iterator tempit1;
              tempit1 = edge_angle.find( ed1 );
              if ( tempit1 != edge_angle.end() )
              {
                //erase the entries
                double angle = tempit1->second;
                edge_angle.erase( ed1 );
                std::multimap<double, int>::iterator tempit2 = angle_edge.lower_bound( angle );
                while ( tempit2->second != ed1 )
                {
                  ++tempit2;
                }
                angle_edge.erase( tempit2 );
              }
            }

            if ( angle1 < mintol && !twoforcededges1 )
            {
              edge_angle.insert( std::make_pair( ed1, angle1 ) );
              angle_edge.insert( std::make_pair( angle1, ed1 ) );
            }

            //update the settings related to ed2
            QSet<int>::iterator ed2iter = dontexamine.find( ed2 );
            if ( ed2iter != dontexamine.end() )
            {
              //edge number is on dontexamine list
              dontexamine.erase( ed2iter );
            }
            else
            {
              //test, if it is on edge_angle and angle_edge and erase them if yes
              std::map<int, double>::iterator tempit1;
              tempit1 = edge_angle.find( ed2 );
              if ( tempit1 != edge_angle.end() )
              {
                //erase the entries
                double angle = tempit1->second;
                edge_angle.erase( ed2 );
                std::multimap<double, int>::iterator tempit2 = angle_edge.lower_bound( angle );
                while ( tempit2->second != ed2 )
                {
                  ++tempit2;
                }
                angle_edge.erase( tempit2 );
              }
            }

            if ( angle2 < mintol && !twoforcededges2 )
            {
              edge_angle.insert( std::make_pair( ed2, angle2 ) );
              angle_edge.insert( std::make_pair( angle2, ed2 ) );
            }

            //update the settings related to ed3
            QSet<int>::iterator ed3iter = dontexamine.find( ed3 );
            if ( ed3iter != dontexamine.end() )
            {
              //edge number is on dontexamine list
              dontexamine.erase( ed3iter );
            }
            else
            {
              //test, if it is on edge_angle and angle_edge and erase them if yes
              std::map<int, double>::iterator tempit1;
              tempit1 = edge_angle.find( ed3 );
              if ( tempit1 != edge_angle.end() )
              {
                //erase the entries
                double angle = tempit1->second;
                edge_angle.erase( ed3 );
                std::multimap<double, int>::iterator tempit2 = angle_edge.lower_bound( angle );
                while ( tempit2->second != ed3 )
                {
                  ++tempit2;
                }
                angle_edge.erase( tempit2 );
              }
            }

            if ( angle3 < mintol && !twoforcededges3 )
            {
              edge_angle.insert( std::make_pair( ed3, angle3 ) );
              angle_edge.insert( std::make_pair( angle3, ed3 ) );
            }


          }
          while ( actedge != pointingedge );
        }
      }
    }
#endif //end slow version


    //fast version. Maybe this does not work
    QSet<int> influenceedges;//begin fast method
    int baseedge = baseEdgeOfTriangle( circumcenter );
    if ( baseedge == -5 )//a numerical instability occurred or the circumcenter already exists in the triangulation
    {
      //delete minedge from edge_angle and minangle from angle_edge
      edge_angle.erase( minedge );
      std::multimap<double, int>::iterator minedgeiter = angle_edge.find( minangle );
      while ( minedgeiter->second != minedge )
      {
        ++minedgeiter;
      }
      angle_edge.erase( minedgeiter );
      continue;
    }
    else if ( baseedge == -25 )//circumcenter already exists in the triangulation
    {
      //delete minedge from edge_angle and minangle from angle_edge
      edge_angle.erase( minedge );
      std::multimap<double, int>::iterator minedgeiter = angle_edge.find( minangle );
      while ( minedgeiter->second != minedge )
      {
        ++minedgeiter;
      }
      angle_edge.erase( minedgeiter );
      continue;
    }
    else if ( baseedge == -20 )
    {
      baseedge = mEdgeWithPoint;
    }

    evaluateInfluenceRegion( &circumcenter, baseedge, influenceedges );
    evaluateInfluenceRegion( &circumcenter, mHalfEdge[baseedge]->getNext(), influenceedges );
    evaluateInfluenceRegion( &circumcenter, mHalfEdge[mHalfEdge[baseedge]->getNext()]->getNext(), influenceedges );

    for ( QSet<int>::iterator it = influenceedges.begin(); it != influenceedges.end(); ++it )
    {
      if ( ( mHalfEdge[*it]->getForced() || edgeOnConvexHull( *it ) ) && MathUtils::inDiametral( mPointVector[mHalfEdge[*it]->getPoint()], mPointVector[mHalfEdge[mHalfEdge[*it]->getDual()]->getPoint()], &circumcenter ) )
      {
        //split segment
        QgsDebugMsg( QStringLiteral( "segment split" ) );
        const int pointno = splitHalfEdge( *it, 0.5 );
        encroached = true;

        //update dontexmine, angle_edge, edge_angle
        const int pointingedge = baseEdgeOfPoint( pointno );

        int actedge = pointingedge;
        int ed1, ed2, ed3;//numbers of the three edges
        int pt1, pt2, pt3;//numbers of the three points
        double angle1, angle2, angle3;

        do
        {
          ed1 = mHalfEdge[actedge]->getDual();
          pt1 = mHalfEdge[ed1]->getPoint();
          ed2 = mHalfEdge[ed1]->getNext();
          pt2 = mHalfEdge[ed2]->getPoint();
          ed3 = mHalfEdge[ed2]->getNext();
          pt3 = mHalfEdge[ed3]->getPoint();
          actedge = ed3;

          if ( pt1 == -1 || pt2 == -1 || pt3 == -1 )//don't consider triangles with the virtual point
          {
            continue;
          }

          angle1 = MathUtils::angle( mPointVector[pt3], mPointVector[pt1], mPointVector[pt2], mPointVector[pt1] );
          angle2 = MathUtils::angle( mPointVector[pt1], mPointVector[pt2], mPointVector[pt3], mPointVector[pt2] );
          angle3 = MathUtils::angle( mPointVector[pt2], mPointVector[pt3], mPointVector[pt1], mPointVector[pt3] );

          //don't put the edges on the maps if two segments are forced or on a hull
          bool twoforcededges1, twoforcededges2, twoforcededges3;//flag to decide, if edges should be added to the maps. Do not add them if true



          twoforcededges1 = ( mHalfEdge[ed1]->getForced() || edgeOnConvexHull( ed1 ) ) && ( mHalfEdge[ed2]->getForced() || edgeOnConvexHull( ed2 ) );

          twoforcededges2 = ( mHalfEdge[ed2]->getForced() || edgeOnConvexHull( ed2 ) ) && ( mHalfEdge[ed3]->getForced() || edgeOnConvexHull( ed3 ) );

          twoforcededges3 = ( mHalfEdge[ed3]->getForced() || edgeOnConvexHull( ed3 ) ) && ( mHalfEdge[ed1]->getForced() || edgeOnConvexHull( ed1 ) );


          //update the settings related to ed1
          const QSet<int>::iterator ed1iter = dontexamine.find( ed1 );
          if ( ed1iter != dontexamine.end() )
          {
            //edge number is on dontexamine list
            dontexamine.erase( ed1iter );
          }
          else
          {
            //test, if it is on edge_angle and angle_edge and erase them if yes
            std::map<int, double>::iterator tempit1;
            tempit1 = edge_angle.find( ed1 );
            if ( tempit1 != edge_angle.end() )
            {
              //erase the entries
              const double angle = tempit1->second;
              edge_angle.erase( ed1 );
              std::multimap<double, int>::iterator tempit2 = angle_edge.lower_bound( angle );
              while ( tempit2->second != ed1 )
              {
                ++tempit2;
              }
              angle_edge.erase( tempit2 );
            }
          }

          if ( angle1 < mintol && !twoforcededges1 )
          {
            edge_angle.insert( std::make_pair( ed1, angle1 ) );
            angle_edge.insert( std::make_pair( angle1, ed1 ) );
          }

          //update the settings related to ed2
          const QSet<int>::iterator ed2iter = dontexamine.find( ed2 );
          if ( ed2iter != dontexamine.end() )
          {
            //edge number is on dontexamine list
            dontexamine.erase( ed2iter );
          }
          else
          {
            //test, if it is on edge_angle and angle_edge and erase them if yes
            std::map<int, double>::iterator tempit1;
            tempit1 = edge_angle.find( ed2 );
            if ( tempit1 != edge_angle.end() )
            {
              //erase the entries
              const double angle = tempit1->second;
              edge_angle.erase( ed2 );
              std::multimap<double, int>::iterator tempit2 = angle_edge.lower_bound( angle );
              while ( tempit2->second != ed2 )
              {
                ++tempit2;
              }
              angle_edge.erase( tempit2 );
            }
          }

          if ( angle2 < mintol && !twoforcededges2 )
          {
            edge_angle.insert( std::make_pair( ed2, angle2 ) );
            angle_edge.insert( std::make_pair( angle2, ed2 ) );
          }

          //update the settings related to ed3
          const QSet<int>::iterator ed3iter = dontexamine.find( ed3 );
          if ( ed3iter != dontexamine.end() )
          {
            //edge number is on dontexamine list
            dontexamine.erase( ed3iter );
          }
          else
          {
            //test, if it is on edge_angle and angle_edge and erase them if yes
            std::map<int, double>::iterator tempit1;
            tempit1 = edge_angle.find( ed3 );
            if ( tempit1 != edge_angle.end() )
            {
              //erase the entries
              const double angle = tempit1->second;
              edge_angle.erase( ed3 );
              std::multimap<double, int>::iterator tempit2 = angle_edge.lower_bound( angle );
              while ( tempit2->second != ed3 )
              {
                ++tempit2;
              }
              angle_edge.erase( tempit2 );
            }
          }

          if ( angle3 < mintol && !twoforcededges3 )
          {
            edge_angle.insert( std::make_pair( ed3, angle3 ) );
            angle_edge.insert( std::make_pair( angle3, ed3 ) );
          }


        }
        while ( actedge != pointingedge );
      }
    } //end fast method


    if ( encroached )
    {
      continue;
    }

    /*******otherwise, try to add the circumcenter to the triangulation************************************************************************************************/

    QgsPoint p( 0, 0, 0 );
    calcPoint( circumcenter.x(), circumcenter.y(), p );
    const int pointno = addPoint( p );

    if ( pointno == -100 || pointno == mTwiceInsPoint )
    {
      if ( pointno == -100 )
      {
        QgsDebugMsg( QStringLiteral( "put circumcenter %1//%2 on dontexamine list because of numerical instabilities" )
                     .arg( circumcenter.x() ).arg( circumcenter.y() ) );
      }
      else if ( pointno == mTwiceInsPoint )
      {
        QgsDebugMsg( QStringLiteral( "put circumcenter %1//%2 on dontexamine list because it is already inserted" )
                     .arg( circumcenter.x() ).arg( circumcenter.y() ) );
        //test, if the point is present in the triangulation
        bool flag = false;
        for ( int i = 0; i < mPointVector.count(); i++ )
        {
          if ( mPointVector[i]->x() == circumcenter.x() && mPointVector[i]->y() == circumcenter.y() )
          {
            flag = true;
          }
        }
        if ( !flag )
        {
          QgsDebugMsg( QStringLiteral( "point is not present in the triangulation" ) );
        }
      }
      //put all three edges to dontexamine and remove them from the other maps
      dontexamine.insert( minedge );
      edge_angle.erase( minedge );
      std::multimap<double, int>::iterator minedgeiter = angle_edge.lower_bound( minangle );
      while ( minedgeiter->second != minedge )
      {
        ++minedgeiter;
      }
      angle_edge.erase( minedgeiter );
      continue;
    }
    else//insertion successful
    {
      QgsDebugMsg( QStringLiteral( "circumcenter added" ) );

      //update the maps
      //go around the inserted point and make changes for every half edge
      const int pointingedge = baseEdgeOfPoint( pointno );

      int actedge = pointingedge;
      int ed1, ed2, ed3;//numbers of the three edges
      int pt1, pt2, pt3;//numbers of the three points
      double angle1, angle2, angle3;

      do
      {
        ed1 = mHalfEdge[actedge]->getDual();
        pt1 = mHalfEdge[ed1]->getPoint();
        ed2 = mHalfEdge[ed1]->getNext();
        pt2 = mHalfEdge[ed2]->getPoint();
        ed3 = mHalfEdge[ed2]->getNext();
        pt3 = mHalfEdge[ed3]->getPoint();
        actedge = ed3;

        if ( pt1 == -1 || pt2 == -1 || pt3 == -1 )//don't consider triangles with the virtual point
        {
          continue;
        }

        angle1 = MathUtils::angle( mPointVector[pt3], mPointVector[pt1], mPointVector[pt2], mPointVector[pt1] );
        angle2 = MathUtils::angle( mPointVector[pt1], mPointVector[pt2], mPointVector[pt3], mPointVector[pt2] );
        angle3 = MathUtils::angle( mPointVector[pt2], mPointVector[pt3], mPointVector[pt1], mPointVector[pt3] );

        //todo: put all three edges on the dontexamine list if two edges are forced or convex hull edges
        bool twoforcededges1, twoforcededges2, twoforcededges3;

        twoforcededges1 = ( mHalfEdge[ed1]->getForced() || edgeOnConvexHull( ed1 ) ) && ( mHalfEdge[ed2]->getForced() || edgeOnConvexHull( ed2 ) );

        twoforcededges2 = ( mHalfEdge[ed2]->getForced() || edgeOnConvexHull( ed2 ) ) && ( mHalfEdge[ed3]->getForced() || edgeOnConvexHull( ed3 ) );

        twoforcededges3 = ( mHalfEdge[ed3]->getForced() || edgeOnConvexHull( ed3 ) ) && ( mHalfEdge[ed1]->getForced() || edgeOnConvexHull( ed1 ) );


        //update the settings related to ed1
        const QSet<int>::iterator ed1iter = dontexamine.find( ed1 );
        if ( ed1iter != dontexamine.end() )
        {
          //edge number is on dontexamine list
          dontexamine.erase( ed1iter );
        }
        else
        {
          //test, if it is on edge_angle and angle_edge and erase them if yes
          std::map<int, double>::iterator tempit1;
          tempit1 = edge_angle.find( ed1 );
          if ( tempit1 != edge_angle.end() )
          {
            //erase the entries
            const double angle = tempit1->second;
            edge_angle.erase( ed1 );
            std::multimap<double, int>::iterator tempit2 = angle_edge.lower_bound( angle );
            while ( tempit2->second != ed1 )
            {
              ++tempit2;
            }
            angle_edge.erase( tempit2 );
          }
        }

        if ( angle1 < mintol && !twoforcededges1 )
        {
          edge_angle.insert( std::make_pair( ed1, angle1 ) );
          angle_edge.insert( std::make_pair( angle1, ed1 ) );
        }


        //update the settings related to ed2
        const QSet<int>::iterator ed2iter = dontexamine.find( ed2 );
        if ( ed2iter != dontexamine.end() )
        {
          //edge number is on dontexamine list
          dontexamine.erase( ed2iter );
        }
        else
        {
          //test, if it is on edge_angle and angle_edge and erase them if yes
          std::map<int, double>::iterator tempit1;
          tempit1 = edge_angle.find( ed2 );
          if ( tempit1 != edge_angle.end() )
          {
            //erase the entries
            const double angle = tempit1->second;
            edge_angle.erase( ed2 );
            std::multimap<double, int>::iterator tempit2 = angle_edge.lower_bound( angle );
            while ( tempit2->second != ed2 )
            {
              ++tempit2;
            }
            angle_edge.erase( tempit2 );
          }
        }

        if ( angle2 < mintol && !twoforcededges2 )
        {
          edge_angle.insert( std::make_pair( ed2, angle2 ) );
          angle_edge.insert( std::make_pair( angle2, ed2 ) );
        }

        //update the settings related to ed3
        const QSet<int>::iterator ed3iter = dontexamine.find( ed3 );
        if ( ed3iter != dontexamine.end() )
        {
          //edge number is on dontexamine list
          dontexamine.erase( ed3iter );
        }
        else
        {
          //test, if it is on edge_angle and angle_edge and erase them if yes
          std::map<int, double>::iterator tempit1;
          tempit1 = edge_angle.find( ed3 );
          if ( tempit1 != edge_angle.end() )
          {
            //erase the entries
            const double angle = tempit1->second;
            edge_angle.erase( ed3 );
            std::multimap<double, int>::iterator tempit2 = angle_edge.lower_bound( angle );
            while ( tempit2->second != ed3 )
            {
              ++tempit2;
            }
            angle_edge.erase( tempit2 );
          }
        }

        if ( angle3 < mintol && !twoforcededges3 )
        {
          edge_angle.insert( std::make_pair( ed3, angle3 ) );
          angle_edge.insert( std::make_pair( angle3, ed3 ) );
        }


      }
      while ( actedge != pointingedge );
    }
  }

#if 0
  //debugging: print out all edge of dontexamine
  for ( QSet<int>::iterator it = dontexamine.begin(); it != dontexamine.end(); ++it )
  {
    QgsDebugMsg( QStringLiteral( "edge nr. %1 is in dontexamine" ).arg( *it ) );
  }
#endif
}


bool QgsDualEdgeTriangulation::swapPossible( unsigned int edge ) const
{
  //test, if edge belongs to a forced edge
  if ( mHalfEdge[edge]->getForced() )
  {
    return false;
  }

  //test, if the edge is on the convex hull or is connected to the virtual point
  if ( mHalfEdge[edge]->getPoint() == -1 || mHalfEdge[mHalfEdge[edge]->getNext()]->getPoint() == -1 || mHalfEdge[mHalfEdge[mHalfEdge[edge]->getDual()]->getNext()]->getPoint() == -1 || mHalfEdge[mHalfEdge[edge]->getDual()]->getPoint() == -1 )
  {
    return false;
  }
  //then, test, if the edge is in the middle of a not convex quad
  QgsPoint *pta = mPointVector[mHalfEdge[edge]->getPoint()];
  QgsPoint *ptb = mPointVector[mHalfEdge[mHalfEdge[edge]->getNext()]->getPoint()];
  QgsPoint *ptc = mPointVector[mHalfEdge[mHalfEdge[mHalfEdge[edge]->getNext()]->getNext()]->getPoint()];
  QgsPoint *ptd = mPointVector[mHalfEdge[mHalfEdge[mHalfEdge[edge]->getDual()]->getNext()]->getPoint()];
  if ( MathUtils::leftOf( *ptc, pta, ptb ) > leftOfTresh )
  {
    return false;
  }
  else if ( MathUtils::leftOf( *ptd, ptb, ptc ) > leftOfTresh )
  {
    return false;
  }
  else if ( MathUtils::leftOf( *pta, ptc, ptd ) > leftOfTresh )
  {
    return false;
  }
  else if ( MathUtils::leftOf( *ptb, ptd, pta ) > leftOfTresh )
  {
    return false;
  }
  return true;
}

void QgsDualEdgeTriangulation::triangulatePolygon( QList<int> *poly, QList<int> *free, int mainedge )
{
  if ( poly && free )
  {
    if ( poly->count() == 3 )//polygon is already a triangle
    {
      return;
    }

    //search for the edge pointing on the closest point(distedge) and for the next(nextdistedge)
    QList<int>::const_iterator iterator = ++( poly->constBegin() );//go to the second edge
    double distance = MathUtils::distPointFromLine( mPointVector[mHalfEdge[( *iterator )]->getPoint()], mPointVector[mHalfEdge[mHalfEdge[mainedge]->getDual()]->getPoint()], mPointVector[mHalfEdge[mainedge]->getPoint()] );
    int distedge = ( *iterator );
    int nextdistedge = mHalfEdge[( *iterator )]->getNext();
    ++iterator;

    while ( iterator != --( poly->constEnd() ) )
    {
      if ( MathUtils::distPointFromLine( mPointVector[mHalfEdge[( *iterator )]->getPoint()], mPointVector[mHalfEdge[mHalfEdge[mainedge]->getDual()]->getPoint()], mPointVector[mHalfEdge[mainedge]->getPoint()] ) < distance )
      {
        distedge = ( *iterator );
        nextdistedge = mHalfEdge[( *iterator )]->getNext();
        distance = MathUtils::distPointFromLine( mPointVector[mHalfEdge[( *iterator )]->getPoint()], mPointVector[mHalfEdge[mHalfEdge[mainedge]->getDual()]->getPoint()], mPointVector[mHalfEdge[mainedge]->getPoint()] );
      }
      ++iterator;
    }

    if ( nextdistedge == ( *( --poly->end() ) ) )//the nearest point is connected to the endpoint of mainedge
    {
      const int inserta = free->first();//take an edge from the freelist
      const int insertb = mHalfEdge[inserta]->getDual();
      free->pop_front();

      mHalfEdge[inserta]->setNext( ( poly->at( 1 ) ) );
      mHalfEdge[inserta]->setPoint( mHalfEdge[mainedge]->getPoint() );
      mHalfEdge[insertb]->setNext( nextdistedge );
      mHalfEdge[insertb]->setPoint( mHalfEdge[distedge]->getPoint() );
      mHalfEdge[distedge]->setNext( inserta );
      mHalfEdge[mainedge]->setNext( insertb );

      QList<int> polya;
      for ( iterator = ( ++( poly->constBegin() ) ); ( *iterator ) != nextdistedge; ++iterator )
      {
        polya.append( ( *iterator ) );
      }
      polya.prepend( inserta );

#if 0
      //print out all the elements of polya for a test
      for ( iterator = polya.begin(); iterator != polya.end(); ++iterator )
      {
        QgsDebugMsg( *iterator );
      }
#endif

      triangulatePolygon( &polya, free, inserta );
    }

    else if ( distedge == ( *( ++poly->begin() ) ) )//the nearest point is connected to the beginpoint of mainedge
    {
      const int inserta = free->first();//take an edge from the freelist
      const int insertb = mHalfEdge[inserta]->getDual();
      free->pop_front();

      mHalfEdge[inserta]->setNext( ( poly->at( 2 ) ) );
      mHalfEdge[inserta]->setPoint( mHalfEdge[distedge]->getPoint() );
      mHalfEdge[insertb]->setNext( mainedge );
      mHalfEdge[insertb]->setPoint( mHalfEdge[mHalfEdge[mainedge]->getDual()]->getPoint() );
      mHalfEdge[distedge]->setNext( insertb );
      mHalfEdge[( *( --poly->end() ) )]->setNext( inserta );

      QList<int> polya;
      iterator = poly->constBegin();
      iterator += 2;
      while ( iterator != poly->constEnd() )
      {
        polya.append( ( *iterator ) );
        ++iterator;
      }
      polya.prepend( inserta );

      triangulatePolygon( &polya, free, inserta );
    }

    else//the nearest point is not connected to an endpoint of mainedge
    {
      const int inserta = free->first();//take an edge from the freelist
      const int insertb = mHalfEdge[inserta]->getDual();
      free->pop_front();

      const int insertc = free->first();
      const int insertd = mHalfEdge[insertc]->getDual();
      free->pop_front();

      mHalfEdge[inserta]->setNext( ( poly->at( 1 ) ) );
      mHalfEdge[inserta]->setPoint( mHalfEdge[mainedge]->getPoint() );
      mHalfEdge[insertb]->setNext( insertd );
      mHalfEdge[insertb]->setPoint( mHalfEdge[distedge]->getPoint() );
      mHalfEdge[insertc]->setNext( nextdistedge );
      mHalfEdge[insertc]->setPoint( mHalfEdge[distedge]->getPoint() );
      mHalfEdge[insertd]->setNext( mainedge );
      mHalfEdge[insertd]->setPoint( mHalfEdge[mHalfEdge[mainedge]->getDual()]->getPoint() );

      mHalfEdge[distedge]->setNext( inserta );
      mHalfEdge[mainedge]->setNext( insertb );
      mHalfEdge[( *( --poly->end() ) )]->setNext( insertc );

      //build two new polygons for recursive triangulation
      QList<int> polya;
      QList<int> polyb;

      for ( iterator = ++( poly->constBegin() ); ( *iterator ) != nextdistedge; ++iterator )
      {
        polya.append( ( *iterator ) );
      }
      polya.prepend( inserta );


      while ( iterator != poly->constEnd() )
      {
        polyb.append( ( *iterator ) );
        ++iterator;
      }
      polyb.prepend( insertc );

      triangulatePolygon( &polya, free, inserta );
      triangulatePolygon( &polyb, free, insertc );
    }
  }

  else
  {
    QgsDebugMsg( QStringLiteral( "warning, null pointer" ) );
  }

}

bool QgsDualEdgeTriangulation::pointInside( double x, double y )
{
  const QgsPoint point( x, y, 0 );
  unsigned int actedge = mEdgeInside;//start with an edge which does not point to the virtual point
  int counter = 0;//number of consecutive successful left-of-tests
  int nulls = 0;//number of left-of-tests, which returned 0. 1 means, that the point is on a line, 2 means that it is on an existing point
  int numinstabs = 0;//number of suspect left-of-tests due to 'leftOfTresh'
  int runs = 0;//counter for the number of iterations in the loop to prevent an endless loop

  while ( true )
  {
    if ( runs > MAX_BASE_ITERATIONS )//prevents endless loops
    {
      QgsDebugMsg( QStringLiteral( "warning, instability detected: Point coordinates: %1//%2" ).arg( x ).arg( y ) );
      return false;
    }

    if ( MathUtils::leftOf( point, mPointVector[mHalfEdge[mHalfEdge[actedge]->getDual()]->getPoint()], mPointVector[mHalfEdge[actedge]->getPoint()] ) < ( -leftOfTresh ) )//point is on the left side
    {
      counter += 1;
      if ( counter == 3 )//three successful passes means that we have found the triangle
      {
        break;
      }
    }

    else if ( fabs( MathUtils::leftOf( point, mPointVector[mHalfEdge[mHalfEdge[actedge]->getDual()]->getPoint()], mPointVector[mHalfEdge[actedge]->getPoint()] ) ) <= leftOfTresh ) //point is exactly in the line of the edge
    {
      counter += 1;
      mEdgeWithPoint = actedge;
      nulls += 1;
      if ( counter == 3 )//three successful passes means that we have found the triangle
      {
        break;
      }
    }

    else//point is on the right side
    {
      actedge = mHalfEdge[actedge]->getDual();
      counter = 1;
      nulls = 0;
      numinstabs = 0;
    }

    actedge = mHalfEdge[actedge]->getNext();
    if ( mHalfEdge[actedge]->getPoint() == -1 )//the half edge points to the virtual point
    {
      if ( nulls == 1 )//point is exactly on the convex hull
      {
        return true;
      }
      mEdgeOutside = ( unsigned int )mHalfEdge[mHalfEdge[actedge]->getNext()]->getNext();
      return false;//the point is outside the convex hull
    }
    runs++;
  }

  if ( nulls == 2 )//we hit an existing point
  {
    return true;
  }
  if ( numinstabs > 0 )//a numerical instability occurred
  {
    QgsDebugMsg( QStringLiteral( "numerical instabilities" ) );
    return true;
  }

  if ( nulls == 1 )//point is on an existing edge
  {
    return true;
  }
  mEdgeInside = actedge;
  return true;
}

#if 0
bool DualEdgeTriangulation::readFromTAFF( QString filename )
{
  QApplication::setOverrideCursor( QCursor( Qt::WaitCursor ) );//change the cursor

  QFile file( filename );//the file to be read
  file.open( IO_Raw | IO_ReadOnly );
  QBuffer buffer( file.readAll() );//the buffer to copy the file to
  file.close();

  QTextStream textstream( &buffer );
  buffer.open( IO_ReadOnly );

  QString buff;
  int numberofhalfedges;
  int numberofpoints;

  //edge section
  while ( buff.mid( 0, 4 ) != "TRIA" )//search for the TRIA-section
  {
    buff = textstream.readLine();
  }
  while ( buff.mid( 0, 4 ) != "NEDG" )
  {
    buff = textstream.readLine();
  }
  numberofhalfedges = buff.section( ' ', 1, 1 ).toInt();
  mHalfEdge.resize( numberofhalfedges );


  while ( buff.mid( 0, 4 ) != "DATA" )//search for the data section
  {
    textstream >> buff;
  }

  int nr1, nr2, dual1, dual2, point1, point2, next1, next2, fo1, fo2, b1, b2;
  bool forced1, forced2, break1, break2;


  //import all the DualEdges
  QProgressBar *edgebar = new QProgressBar();//use a progress dialog so that it is not boring for the user
  edgebar->setCaption( tr( "Reading edges…" ) );
  edgebar->setTotalSteps( numberofhalfedges / 2 );
  edgebar->setMinimumWidth( 400 );
  edgebar->move( 500, 500 );
  edgebar->show();

  for ( int i = 0; i < numberofhalfedges / 2; i++ )
  {
    if ( i % 1000 == 0 )
    {
      edgebar->setProgress( i );
    }

    textstream >> nr1;
    textstream >> point1;
    textstream >> next1;
    textstream >> fo1;

    if ( fo1 == 0 )
    {
      forced1 = false;
    }
    else
    {
      forced1 = true;
    }

    textstream >> b1;

    if ( b1 == 0 )
    {
      break1 = false;
    }
    else
    {
      break1 = true;
    }

    textstream >> nr2;
    textstream >> point2;
    textstream >> next2;
    textstream >> fo2;

    if ( fo2 == 0 )
    {
      forced2 = false;
    }
    else
    {
      forced2 = true;
    }

    textstream >> b2;

    if ( b2 == 0 )
    {
      break2 = false;
    }
    else
    {
      break2 = true;
    }

    HalfEdge *hf1 = new HalfEdge();
    hf1->setDual( nr2 );
    hf1->setNext( next1 );
    hf1->setPoint( point1 );
    hf1->setBreak( break1 );
    hf1->setForced( forced1 );

    HalfEdge *hf2 = new HalfEdge();
    hf2->setDual( nr1 );
    hf2->setNext( next2 );
    hf2->setPoint( point2 );
    hf2->setBreak( break2 );
    hf2->setForced( forced2 );

    // QgsDebugMsg( QStringLiteral( "inserting half edge pair %1" ).arg( i ) );
    mHalfEdge.insert( nr1, hf1 );
    mHalfEdge.insert( nr2, hf2 );

  }

  edgebar->setProgress( numberofhalfedges / 2 );
  delete edgebar;

  //set mEdgeInside to a reasonable value
  for ( int i = 0; i < numberofhalfedges; i++ )
  {
    int a, b, c, d;
    a = mHalfEdge[i]->getPoint();
    b = mHalfEdge[mHalfEdge[i]->getDual()]->getPoint();
    c = mHalfEdge[mHalfEdge[i]->getNext()]->getPoint();
    d = mHalfEdge[mHalfEdge[mHalfEdge[i]->getDual()]->getNext()]->getPoint();
    if ( a != -1 && b != -1 && c != -1 && d != -1 )
    {
      mEdgeInside = i;
      break;
    }
  }

  //point section
  while ( buff.mid( 0, 4 ) != "POIN" )
  {
    buff = textstream.readLine();
    QgsDebugMsg( buff );
  }
  while ( buff.mid( 0, 4 ) != "NPTS" )
  {
    buff = textstream.readLine();
    QgsDebugMsg( buff );
  }
  numberofpoints = buff.section( ' ', 1, 1 ).toInt();
  mPointVector.resize( numberofpoints );

  while ( buff.mid( 0, 4 ) != "DATA" )
  {
    textstream >> buff;
  }

  QProgressBar *pointbar = new QProgressBar();
  pointbar->setCaption( tr( "Reading points…" ) );
  pointbar->setTotalSteps( numberofpoints );
  pointbar->setMinimumWidth( 400 );
  pointbar->move( 500, 500 );
  pointbar->show();


  double x, y, z;
  for ( int i = 0; i < numberofpoints; i++ )
  {
    if ( i % 1000 == 0 )
    {
      pointbar->setProgress( i );
    }

    textstream >> x;
    textstream >> y;
    textstream >> z;

    QgsPoint *p = new QgsPoint( x, y, z );

    // QgsDebugMsg( QStringLiteral( "inserting point %1" ).arg( i ) );
    mPointVector.insert( i, p );

    if ( i == 0 )
    {
      xMin = x;
      xMax = x;
      yMin = y;
      yMax = y;
    }
    else
    {
      //update the bounding box
      if ( x < xMin )
      {
        xMin = x;
      }
      else if ( x > xMax )
      {
        xMax = x;
      }

      if ( y < yMin )
      {
        yMin = y;
      }
      else if ( y > yMax )
      {
        yMax = y;
      }
    }
  }

  pointbar->setProgress( numberofpoints );
  delete pointbar;
  QApplication::restoreOverrideCursor();

}

bool DualEdgeTriangulation::saveToTAFF( QString filename ) const
{
  QFile outputfile( filename );
  if ( !outputfile.open( IO_WriteOnly ) )
  {
    QMessageBox::warning( 0, tr( "Warning" ), tr( "File could not be written." ), QMessageBox::Ok, QMessageBox::NoButton, QMessageBox::NoButton );
    return false;
  }

  QTextStream outstream( &outputfile );
  outstream.precision( 9 );

  //export the edges. Attention, dual edges must be adjacent in the TAFF-file
  outstream << "TRIA" << std::endl << std::flush;
  outstream << "NEDG " << mHalfEdge.count() << std::endl << std::flush;
  outstream << "PANO 1" << std::endl << std::flush;
  outstream << "DATA ";

  bool *cont = new bool[mHalfEdge.count()];
  for ( unsigned int i = 0; i <= mHalfEdge.count() - 1; i++ )
  {
    cont[i] = false;
  }

  for ( unsigned int i = 0; i < mHalfEdge.count(); i++ )
  {
    if ( cont[i] )
    {
      continue;
    }

    int dual = mHalfEdge[i]->getDual();
    outstream << i << " " << mHalfEdge[i]->getPoint() << " " << mHalfEdge[i]->getNext() << " " << mHalfEdge[i]->getForced() << " " << mHalfEdge[i]->getBreak() << " ";
    outstream << dual << " " << mHalfEdge[dual]->getPoint() << " " << mHalfEdge[dual]->getNext() << " " << mHalfEdge[dual]->getForced() << " " << mHalfEdge[dual]->getBreak() << " ";
    cont[i] = true;
    cont[dual] = true;
  }
  outstream << std::endl << std::flush;
  outstream << std::endl << std::flush;

  delete[] cont;

  //export the points to the file
  outstream << "POIN" << std::endl << std::flush;
  outstream << "NPTS " << getNumberOfPoints() << std::endl << std::flush;
  outstream << "PATT 3" << std::endl << std::flush;
  outstream << "DATA ";

  for ( int i = 0; i < getNumberOfPoints(); i++ )
  {
    QgsPoint *p = mPointVector[i];
    outstream << p->getX() << " " << p->getY() << " " << p->getZ() << " ";
  }
  outstream << std::endl << std::flush;
  outstream << std::endl << std::flush;

  return true;
}
#endif //0

bool QgsDualEdgeTriangulation::swapEdge( double x, double y )
{
  QgsPoint p( x, y, 0 );
  const int edge1 = baseEdgeOfTriangle( p );
  if ( edge1 >= 0 )
  {
    int edge2, edge3;
    QgsPoint *point1 = nullptr;
    QgsPoint *point2 = nullptr;
    QgsPoint *point3 = nullptr;
    edge2 = mHalfEdge[edge1]->getNext();
    edge3 = mHalfEdge[edge2]->getNext();
    point1 = point( mHalfEdge[edge1]->getPoint() );
    point2 = point( mHalfEdge[edge2]->getPoint() );
    point3 = point( mHalfEdge[edge3]->getPoint() );
    if ( point1 && point2 && point3 )
    {
      //find out the closest edge to the point and swap this edge
      double dist1, dist2, dist3;
      dist1 = MathUtils::distPointFromLine( &p, point3, point1 );
      dist2 = MathUtils::distPointFromLine( &p, point1, point2 );
      dist3 = MathUtils::distPointFromLine( &p, point2, point3 );
      if ( dist1 <= dist2 && dist1 <= dist3 )
      {
        //qWarning("edge "+QString::number(edge1)+" is closest");
        if ( swapPossible( edge1 ) )
        {
          doOnlySwap( edge1 );
        }
      }
      else if ( dist2 <= dist1 && dist2 <= dist3 )
      {
        //qWarning("edge "+QString::number(edge2)+" is closest");
        if ( swapPossible( edge2 ) )
        {
          doOnlySwap( edge2 );
        }
      }
      else if ( dist3 <= dist1 && dist3 <= dist2 )
      {
        //qWarning("edge "+QString::number(edge3)+" is closest");
        if ( swapPossible( edge3 ) )
        {
          doOnlySwap( edge3 );
        }
      }
      return true;
    }
    else
    {
      // QgsDebugMsg("warning: null pointer");
      return false;
    }
  }
  else
  {
    // QgsDebugMsg("Edge number negative");
    return false;
  }
}

QList<int> QgsDualEdgeTriangulation::pointsAroundEdge( double x, double y )
{
  QgsPoint p( x, y, 0 );
  QList<int> list;
  const int edge1 = baseEdgeOfTriangle( p );
  if ( edge1 >= 0 )
  {
    const int edge2 = mHalfEdge[edge1]->getNext();
    const int edge3 = mHalfEdge[edge2]->getNext();
    QgsPoint *point1 = point( mHalfEdge[edge1]->getPoint() );
    QgsPoint *point2 = point( mHalfEdge[edge2]->getPoint() );
    QgsPoint *point3 = point( mHalfEdge[edge3]->getPoint() );
    if ( point1 && point2 && point3 )
    {
      int p1, p2, p3, p4;
      const double dist1 = MathUtils::distPointFromLine( &p, point3, point1 );
      const double dist2 = MathUtils::distPointFromLine( &p, point1, point2 );
      const double dist3 = MathUtils::distPointFromLine( &p, point2, point3 );
      if ( dist1 <= dist2 && dist1 <= dist3 )
      {
        p1 = mHalfEdge[edge1]->getPoint();
        p2 = mHalfEdge[mHalfEdge[edge1]->getNext()]->getPoint();
        p3 = mHalfEdge[mHalfEdge[edge1]->getDual()]->getPoint();
        p4 = mHalfEdge[mHalfEdge[mHalfEdge[edge1]->getDual()]->getNext()]->getPoint();
      }
      else if ( dist2 <= dist1 && dist2 <= dist3 )
      {
        p1 = mHalfEdge[edge2]->getPoint();
        p2 = mHalfEdge[mHalfEdge[edge2]->getNext()]->getPoint();
        p3 = mHalfEdge[mHalfEdge[edge2]->getDual()]->getPoint();
        p4 = mHalfEdge[mHalfEdge[mHalfEdge[edge2]->getDual()]->getNext()]->getPoint();
      }
      else /* if ( dist3 <= dist1 && dist3 <= dist2 ) */
      {
        p1 = mHalfEdge[edge3]->getPoint();
        p2 = mHalfEdge[mHalfEdge[edge3]->getNext()]->getPoint();
        p3 = mHalfEdge[mHalfEdge[edge3]->getDual()]->getPoint();
        p4 = mHalfEdge[mHalfEdge[mHalfEdge[edge3]->getDual()]->getNext()]->getPoint();
      }


      list.append( p1 );
      list.append( p2 );
      list.append( p3 );
      list.append( p4 );
    }
    else
    {
      QgsDebugMsg( QStringLiteral( "warning: null pointer" ) );
    }
  }
  else
  {
    QgsDebugMsg( QStringLiteral( "Edge number negative" ) );
  }
  return list;
}

bool QgsDualEdgeTriangulation::saveTriangulation( QgsFeatureSink *sink, QgsFeedback *feedback ) const
{
  if ( !sink )
  {
    return false;
  }

  bool *alreadyVisitedEdges = new bool[mHalfEdge.size()];
  if ( !alreadyVisitedEdges )
  {
    QgsDebugMsg( QStringLiteral( "out of memory" ) );
    return false;
  }

  for ( int i = 0; i < mHalfEdge.size(); ++i )
  {
    alreadyVisitedEdges[i] = false;
  }

  for ( int i = 0; i < mHalfEdge.size(); ++i )
  {
    if ( feedback && feedback->isCanceled() )
      break;

    HalfEdge *currentEdge = mHalfEdge[i];
    if ( currentEdge->getPoint() != -1 && mHalfEdge[currentEdge->getDual()]->getPoint() != -1 && !alreadyVisitedEdges[currentEdge->getDual()] )
    {
      QgsFeature edgeLineFeature;

      //geometry
      QgsPoint *p1 = mPointVector[currentEdge->getPoint()];
      QgsPoint *p2 = mPointVector[mHalfEdge[currentEdge->getDual()]->getPoint()];
      QgsPolylineXY lineGeom;
      lineGeom.push_back( QgsPointXY( p1->x(), p1->y() ) );
      lineGeom.push_back( QgsPointXY( p2->x(), p2->y() ) );
      edgeLineFeature.setGeometry( QgsGeometry::fromPolylineXY( lineGeom ) );
      edgeLineFeature.initAttributes( 1 );

      //attributes
      QString attributeString;
      if ( currentEdge->getForced() )
      {
        if ( currentEdge->getBreak() )
        {
          attributeString = QStringLiteral( "break line" );
        }
        else
        {
          attributeString = QStringLiteral( "structure line" );
        }
      }
      edgeLineFeature.setAttribute( 0, attributeString );

      sink->addFeature( edgeLineFeature, QgsFeatureSink::FastInsert );
    }
    alreadyVisitedEdges[i] = true;
  }

  delete [] alreadyVisitedEdges;

  return !feedback || !feedback->isCanceled();
}

QgsMesh QgsDualEdgeTriangulation::triangulationToMesh( QgsFeedback *feedback ) const
{
  if ( feedback )
    feedback->setProgress( 0 );

  QVector< bool> edgeToTreat( mHalfEdge.count(), true );
  QHash<HalfEdge *, int > edgesHash;
  for ( int i = 0; i < mHalfEdge.count(); ++i )
  {
    edgesHash.insert( mHalfEdge[i], i );
  }

  QgsMesh mesh;
  for ( const QgsPoint *point : mPointVector )
  {
    mesh.vertices.append( *point );
  }

  const int edgeCount = edgeToTreat.count();
  for ( int i = 0 ; i < edgeCount; ++i )
  {
    bool containVirtualPoint = false;
    if ( edgeToTreat[i] )
    {
      HalfEdge *currentEdge = mHalfEdge[i];
      HalfEdge *firstEdge = currentEdge;
      QgsMeshFace face;
      do
      {
        edgeToTreat[edgesHash.value( currentEdge )] = false;
        face.append( currentEdge->getPoint() );
        containVirtualPoint |= currentEdge->getPoint() == -1;
        currentEdge = mHalfEdge.at( currentEdge->getNext() );
      }
      while ( currentEdge != firstEdge && !containVirtualPoint && ( !feedback || !feedback->isCanceled() ) );
      if ( !containVirtualPoint )
        mesh.faces.append( face );
    }
    if ( feedback )
    {
      feedback->setProgress( ( 100 *  i ) / edgeCount ) ;
    }
  }

  return mesh;
}

double QgsDualEdgeTriangulation::swapMinAngle( int edge ) const
{
  QgsPoint *p1 = point( mHalfEdge[edge]->getPoint() );
  QgsPoint *p2 = point( mHalfEdge[mHalfEdge[edge]->getNext()]->getPoint() );
  QgsPoint *p3 = point( mHalfEdge[mHalfEdge[edge]->getDual()]->getPoint() );
  QgsPoint *p4 = point( mHalfEdge[mHalfEdge[mHalfEdge[edge]->getDual()]->getNext()]->getPoint() );

  //search for the minimum angle (it is important, which directions the lines have!)
  double minangle;
  const double angle1 = MathUtils::angle( p1, p2, p4, p2 );
  minangle = angle1;
  const double angle2 = MathUtils::angle( p3, p2, p4, p2 );
  if ( angle2 < minangle )
  {
    minangle = angle2;
  }
  const double angle3 = MathUtils::angle( p2, p3, p4, p3 );
  if ( angle3 < minangle )
  {
    minangle = angle3;
  }
  const double angle4 = MathUtils::angle( p3, p4, p2, p4 );
  if ( angle4 < minangle )
  {
    minangle = angle4;
  }
  const double angle5 = MathUtils::angle( p2, p4, p1, p4 );
  if ( angle5 < minangle )
  {
    minangle = angle5;
  }
  const double angle6 = MathUtils::angle( p4, p1, p2, p1 );
  if ( angle6 < minangle )
  {
    minangle = angle6;
  }

  return minangle;
}

int QgsDualEdgeTriangulation::splitHalfEdge( int edge, float position )
{
  //just a short test if position is between 0 and 1
  if ( position < 0 || position > 1 )
  {
    QgsDebugMsg( QStringLiteral( "warning, position is not between 0 and 1" ) );
  }

  //create the new point on the heap
  QgsPoint *p = new QgsPoint( mPointVector[mHalfEdge[edge]->getPoint()]->x()*position + mPointVector[mHalfEdge[mHalfEdge[edge]->getDual()]->getPoint()]->x() * ( 1 - position ), mPointVector[mHalfEdge[edge]->getPoint()]->y()*position + mPointVector[mHalfEdge[mHalfEdge[edge]->getDual()]->getPoint()]->y() * ( 1 - position ), 0 );

  //calculate the z-value of the point to insert
  QgsPoint zvaluepoint( 0, 0, 0 );
  calcPoint( p->x(), p->y(), zvaluepoint );
  p->setZ( zvaluepoint.z() );

  //insert p into mPointVector
  if ( mPointVector.count() >= mPointVector.size() )
  {
    mPointVector.resize( mPointVector.count() + 1 );
  }
  QgsDebugMsg( QStringLiteral( "inserting point nr. %1, %2//%3//%4" ).arg( mPointVector.count() ).arg( p->x() ).arg( p->y() ).arg( p->z() ) );
  mPointVector.insert( mPointVector.count(), p );

  //insert the six new halfedges
  const int dualedge = mHalfEdge[edge]->getDual();
  const int edge1 = insertEdge( -10, -10, mPointVector.count() - 1, false, false );
  const int edge2 = insertEdge( edge1, mHalfEdge[mHalfEdge[edge]->getNext()]->getNext(), mHalfEdge[mHalfEdge[edge]->getNext()]->getPoint(), false, false );
  const int edge3 = insertEdge( -10, mHalfEdge[mHalfEdge[dualedge]->getNext()]->getNext(), mHalfEdge[mHalfEdge[dualedge]->getNext()]->getPoint(), false, false );
  const int edge4 = insertEdge( edge3, dualedge, mPointVector.count() - 1, false, false );
  const int edge5 = insertEdge( -10, mHalfEdge[edge]->getNext(), mHalfEdge[edge]->getPoint(), mHalfEdge[edge]->getBreak(), mHalfEdge[edge]->getForced() );
  const int edge6 = insertEdge( edge5, edge3, mPointVector.count() - 1, mHalfEdge[dualedge]->getBreak(), mHalfEdge[dualedge]->getForced() );
  mHalfEdge[edge1]->setDual( edge2 );
  mHalfEdge[edge1]->setNext( edge5 );
  mHalfEdge[edge3]->setDual( edge4 );
  mHalfEdge[edge5]->setDual( edge6 );

  //adjust the already existing halfedges
  mHalfEdge[mHalfEdge[edge]->getNext()]->setNext( edge1 );
  mHalfEdge[mHalfEdge[dualedge]->getNext()]->setNext( edge4 );
  mHalfEdge[edge]->setNext( edge2 );
  mHalfEdge[edge]->setPoint( mPointVector.count() - 1 );
  mHalfEdge[mHalfEdge[edge3]->getNext()]->setNext( edge6 );

  //test four times recursively for swapping
  checkSwapRecursively( mHalfEdge[edge5]->getNext(), 0 );
  checkSwapRecursively( mHalfEdge[edge2]->getNext(), 0 );
  checkSwapRecursively( mHalfEdge[dualedge]->getNext(), 0 );
  checkSwapRecursively( mHalfEdge[edge3]->getNext(), 0 );

  addPoint( QgsPoint( p->x(), p->y(), 0 ) );//dirty hack to enforce update of decorators

  return mPointVector.count() - 1;
}

bool QgsDualEdgeTriangulation::edgeOnConvexHull( int edge )
{
  return ( mHalfEdge[mHalfEdge[edge]->getNext()]->getPoint() == -1 || mHalfEdge[mHalfEdge[mHalfEdge[edge]->getDual()]->getNext()]->getPoint() == -1 );
}

void QgsDualEdgeTriangulation::evaluateInfluenceRegion( QgsPoint *point, int edge, QSet<int> &set )
{
  if ( set.find( edge ) == set.end() )
  {
    set.insert( edge );
  }
  else//prevent endless loops
  {
    return;
  }

  if ( !mHalfEdge[edge]->getForced() && !edgeOnConvexHull( edge ) )
  {
    //test, if point is in the circle through both endpoints of edge and the endpoint of edge->dual->next->point
    if ( inCircle( *point, *mPointVector[mHalfEdge[mHalfEdge[edge]->getDual()]->getPoint()], *mPointVector[mHalfEdge[edge]->getPoint()], *mPointVector[mHalfEdge[mHalfEdge[edge]->getNext()]->getPoint()] ) )
    {
      evaluateInfluenceRegion( point, mHalfEdge[mHalfEdge[edge]->getDual()]->getNext(), set );
      evaluateInfluenceRegion( point, mHalfEdge[mHalfEdge[mHalfEdge[edge]->getDual()]->getNext()]->getNext(), set );
    }
  }
}

int QgsDualEdgeTriangulation::firstEdgeOutSide()
{
  int edge = 0;
  while ( ( mHalfEdge[mHalfEdge[edge]->getNext()]->getPoint() != -1 ||
            mHalfEdge[edge]->getPoint() == -1 || mHalfEdge[mHalfEdge[edge]->getDual()]->getPoint() == -1 ) &&
          edge < mHalfEdge.count() )
  {
    edge++;
  }

  if ( edge >= mHalfEdge.count() )
    return -1;
  else
    return edge;
}

void QgsDualEdgeTriangulation::removeLastPoint()
{
  if ( mPointVector.isEmpty() )
    return;
  QgsPoint *p = mPointVector.takeLast();
  delete p;
}
