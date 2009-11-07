/***************************************************************************
                          DualEdgeTriangulation.cc  -  description
                             -------------------
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


#include "DualEdgeTriangulation.h"
#include <map>
#include "qgsgeometry.h"
#include "qgslogger.h"
#include "qgsvectorfilewriter.h"

double leftOfTresh = 0.00000001;

DualEdgeTriangulation::~DualEdgeTriangulation()
{
  //remove all the points
  if ( mPointVector.count() > 0 )
  {
    for ( int i = 0; i < mPointVector.count();i++ )
    {
      delete mPointVector[i];
    }
  }

  //remove all the HalfEdge
  if ( mHalfEdge.count() > 0 )
  {
    for ( int i = 0; i < mHalfEdge.count();i++ )
    {
      delete mHalfEdge[i];
    }
  }
}

void DualEdgeTriangulation::performConsistencyTest()
{
  QgsDebugMsg( "performing consistency test" );

  for ( int i = 0;i < mHalfEdge.count();i++ )
  {
    int a = mHalfEdge[mHalfEdge[i]->getDual()]->getDual();
    int b = mHalfEdge[mHalfEdge[mHalfEdge[i]->getNext()]->getNext()]->getNext();
    if ( i != a )
    {
      QgsDebugMsg( "warning, first test failed" );
    }
    if ( i != b )
    {
      QgsDebugMsg( "warning, second test failed" );
    }
  }
  QgsDebugMsg( "consistency test finished" );
}

void DualEdgeTriangulation::addLine( Line3D* line, bool breakline )
{
  int actpoint = -10;//number of the last point, which has been inserted from the line
  int currentpoint = -10;//number of the point, which is currently inserted from the line
  if ( line )
  {
    //first, find the first point
    unsigned int i;
    line->goToBegin();

    for ( i = 0;i < line->getSize();i++ )
    {
      line->goToNext();
      actpoint = mDecorator->addPoint( line->getPoint() );
      if ( actpoint != -100 )
      {
        i++;
        break;
      }
    }

    if ( actpoint == -100 )//no point of the line could be inserted
    {
      delete line;
      return;
    }

    for ( ;i < line->getSize();i++ )
    {
      line->goToNext();
      currentpoint = mDecorator->addPoint( line->getPoint() );
      if ( currentpoint != -100 && actpoint != -100 && currentpoint != actpoint )//-100 is the return value if the point could not be not inserted
      {
        insertForcedSegment( actpoint, currentpoint, breakline );
      }
      actpoint = currentpoint;
    }
  }
  delete line;
}

int DualEdgeTriangulation::addPoint( Point3D* p )
{
  if ( p )
  {
// QgsDebugMsg( QString("inserting point %1,%2//%3//%4").arg(mPointVector.count()).arg(p->getX()).arg(p->getY()).arg(p->getZ()));

    //first update the bounding box
    if ( mPointVector.count() == 0 )//update bounding box when the first point is inserted
    {
      xMin = ( *p ).getX();
      yMin = ( *p ).getY();
      xMax = ( *p ).getX();
      yMax = ( *p ).getY();
    }

    else//update bounding box else
    {
      if (( *p ).getX() < xMin )
      {
        xMin = ( *p ).getX();
      }
      else if (( *p ).getX() > xMax )
      {
        xMax = ( *p ).getX();
      }

      if (( *p ).getY() < yMin )
      {
        yMin = ( *p ).getY();
      }
      else if (( *p ).getY() > yMax )
      {
        yMax = ( *p ).getY();
      }
    }

    //then update mPointVector
    mPointVector.append( p );

    //then update the HalfEdgeStructure
    if ( mPointVector.count() == 1 )//insert the first point into the triangulation
    {
      unsigned int zedge = insertEdge( -10, -10, -1, false, false );//edge pointing from p to the virtual point
      unsigned int fedge = insertEdge(( int )zedge, ( int )zedge, 0, false, false );//edge pointing from the virtual point to p
      ( mHalfEdge.at( zedge ) )->setDual(( int )fedge );
      ( mHalfEdge.at( zedge ) )->setNext(( int )fedge );

    }

    else if ( mPointVector.count() == 2 )//insert the second point into the triangulation
    {
      //test, if it is the same point as the first point
      if ( p->getX() == mPointVector[0]->getX() && p->getY() == mPointVector[0]->getY() )
      {
        QgsDebugMsg( "second point is the same as the first point, it thus has not been inserted" );
        Point3D* p = mPointVector[1];
        mPointVector.remove( 1 );
        delete p;
        return -100;
      }

      unsigned int sedge = insertEdge( -10, -10, 1, false, false );//edge pointing from point 0 to point 1
      unsigned int tedge = insertEdge(( int )sedge, 0, 0, false, false );//edge pointing from point 1 to point 0
      unsigned int foedge = insertEdge( -10, 4, 1, false, false );//edge pointing from the virtual point to point 1
      unsigned int fiedge = insertEdge(( int )foedge, 1, -1, false, false );//edge pointing from point 2 to the virtual point
      mHalfEdge.at( sedge )->setDual(( int )tedge );
      mHalfEdge.at( sedge )->setNext(( int )fiedge );
      mHalfEdge.at( foedge )->setDual(( int )fiedge );
      mHalfEdge.at( foedge )->setNext(( int )tedge );
      mHalfEdge.at( 0 )->setNext(( int )foedge );
      mHalfEdge.at( 1 )->setNext(( int )sedge );

      mEdgeInside = 3;
    }

    else if ( mPointVector.count() == 3 )//insert the third point into the triangulation
    {
      //we first have to decide, if the third point is to the left or to the right of the line through p0 and p1
      double number = MathUtils::leftOf( p, mPointVector[0], mPointVector[1] );
      if ( number < -leftOfTresh )//p is on the left side
      {
        //insert six new edges
        unsigned int edgea = insertEdge( -10, -10, 2, false, false );//edge pointing from point1 to point2
        unsigned int edgeb = insertEdge(( int )edgea, 5, 1, false, false );//edge pointing from point2 to point1
        unsigned int edgec = insertEdge( -10, ( int )edgeb, 2, false, false );//edge pointing from the virtual point to p2
        unsigned int edged = insertEdge( -10, 2, 0, false, false );//edge pointing from point2 to point0
        unsigned int edgee = insertEdge(( int )edged, -10, 2, false, false );//edge pointing from point0 to point2
        unsigned int edgef = insertEdge(( int )edgec, 1, -1, false, false );//edge pointing from point2 to the virtual point
        mHalfEdge.at( edgea )->setDual(( int )edgeb );
        mHalfEdge.at( edgea )->setNext(( int )edged );
        mHalfEdge.at( edgec )->setDual(( int )edgef );
        mHalfEdge.at( edged )->setDual(( int )edgee );
        mHalfEdge.at( edgee )->setNext(( int )edgef );
        mHalfEdge.at( 5 )->setNext(( int )edgec );
        mHalfEdge.at( 1 )->setNext(( int )edgee );
        mHalfEdge.at( 2 )->setNext(( int )edgea );
      }

      else if ( number > leftOfTresh )//p is on the right side
      {
        //insert six new edges
        unsigned int edgea = insertEdge( -10, -10, 2, false, false );//edge pointing from p0 to p2
        unsigned int edgeb = insertEdge(( int )edgea, 0, 0, false, false );//edge pointing from p2 to p0
        unsigned int edgec = insertEdge( -10, ( int )edgeb, 2, false, false );//edge pointing from the virtual point to p2
        unsigned int edged = insertEdge( -10, 3, 1, false, false );//edge pointing from p2 to p1
        unsigned int edgee = insertEdge(( int )edged, -10, 2, false, false );//edge pointing from p1 to p2
        unsigned int edgef = insertEdge(( int )edgec, 4, -1, false, false );//edge pointing from p2 to the virtual point
        mHalfEdge.at( edgea )->setDual(( int )edgeb );
        mHalfEdge.at( edgea )->setNext(( int )edged );
        mHalfEdge.at( edgec )->setDual(( int )edgef );
        mHalfEdge.at( edged )->setDual(( int )edgee );
        mHalfEdge.at( edgee )->setNext(( int )edgef );
        mHalfEdge.at( 0 )->setNext(( int )edgec );
        mHalfEdge.at( 4 )->setNext(( int )edgee );
        mHalfEdge.at( 3 )->setNext(( int )edgea );
      }

      else//p is in a line with p0 and p1
      {
        mPointVector.remove( mPointVector.count() - 1 );
        QgsDebugMsg( "error: third point is on the same line as the first and the second point. It thus has not been inserted into the triangulation" );
        return -100;
      }
    }

    else if ( mPointVector.count() >= 4 )//insert an arbitrary point into the triangulation
    {
      int number = baseEdgeOfTriangle( p );

      //point is outside the convex hull----------------------------------------------------
      if ( number == -10 )
      {
        unsigned int cwedge = mEdgeOutside;//the last visible edge clockwise from mEdgeOutside
        unsigned int ccwedge = mEdgeOutside;//the last visible edge counterclockwise from mEdgeOutside

        //mEdgeOutside is in each case visible
        mHalfEdge[mHalfEdge[mEdgeOutside]->getNext()]->setPoint( mPointVector.count() - 1 );

        //find cwedge and replace the virtual point with the new point when necessary
        while ( MathUtils::leftOf( mPointVector[( unsigned int ) mHalfEdge[mHalfEdge[mHalfEdge[mHalfEdge[cwedge]->getNext()]->getDual()]->getNext()]->getPoint()], p, mPointVector[( unsigned int ) mHalfEdge[cwedge]->getPoint()] ) < ( -leftOfTresh ) )
        {
          //set the point number of the necessary edge to the actual point instead of the virtual point
          mHalfEdge[mHalfEdge[mHalfEdge[mHalfEdge[mHalfEdge[cwedge]->getNext()]->getDual()]->getNext()]->getNext()]->setPoint( mPointVector.count() - 1 );
          //advance cwedge one edge further clockwise
          cwedge = ( unsigned int )mHalfEdge[mHalfEdge[mHalfEdge[cwedge]->getNext()]->getDual()]->getNext();
        }

        //build the necessary connections with the virtual point
        unsigned int edge1 = insertEdge( mHalfEdge[cwedge]->getNext(), -10, mHalfEdge[cwedge]->getPoint(), false, false );//edge pointing from the new point to the last visible point clockwise
        unsigned int edge2 = insertEdge( mHalfEdge[mHalfEdge[cwedge]->getNext()]->getDual(), -10, -1, false, false );//edge pointing from the last visible point to the virtual point
        unsigned int edge3 = insertEdge( -10, edge1, mPointVector.count() - 1, false, false );//edge pointing from the virtual point to new point

        //adjust the other pointers
        mHalfEdge[mHalfEdge[mHalfEdge[cwedge]->getNext()]->getDual()]->setDual( edge2 );
        mHalfEdge[mHalfEdge[cwedge]->getNext()]->setDual( edge1 );
        mHalfEdge[edge1]->setNext( edge2 );
        mHalfEdge[edge2]->setNext( edge3 );



        //find ccwedge and replace the virtual point with the new point when necessary
        while ( MathUtils::leftOf( mPointVector[mHalfEdge[mHalfEdge[mHalfEdge[ccwedge]->getNext()]->getNext()]->getPoint()], mPointVector[mPointVector.count()-1], mPointVector[mHalfEdge[mHalfEdge[mHalfEdge[mHalfEdge[mHalfEdge[ccwedge]->getNext()]->getNext()]->getDual()]->getNext()]->getPoint()] ) < ( -leftOfTresh ) )
        {
          //set the point number of the necessary edge to the actual point instead of the virtual point
          mHalfEdge[mHalfEdge[mHalfEdge[mHalfEdge[ccwedge]->getNext()]->getNext()]->getDual()]->setPoint( mPointVector.count() - 1 );
          //advance ccwedge one edge further counterclockwise
          ccwedge = mHalfEdge[mHalfEdge[mHalfEdge[mHalfEdge[mHalfEdge[ccwedge]->getNext()]->getNext()]->getDual()]->getNext()]->getNext();
        }

        //build the necessary connections with the virtual point
        unsigned int edge4 = insertEdge( mHalfEdge[mHalfEdge[ccwedge]->getNext()]->getNext(), -10, mPointVector.count() - 1, false, false );//points from the last visible point counterclockwise to the new point
        unsigned int edge5 = insertEdge( edge3, -10, -1, false, false );//points from the new point to the virtual point
        unsigned int edge6 = insertEdge( mHalfEdge[mHalfEdge[mHalfEdge[ccwedge]->getNext()]->getNext()]->getDual(), edge4, mHalfEdge[mHalfEdge[ccwedge]->getDual()]->getPoint(), false, false );//points from the virtual point to the last visible point counterclockwise



        //adjust the other pointers
        mHalfEdge[mHalfEdge[mHalfEdge[mHalfEdge[ccwedge]->getNext()]->getNext()]->getDual()]->setDual( edge6 );
        mHalfEdge[mHalfEdge[mHalfEdge[ccwedge]->getNext()]->getNext()]->setDual( edge4 );
        mHalfEdge[edge4]->setNext( edge5 );
        mHalfEdge[edge5]->setNext( edge6 );
        mHalfEdge[edge3]->setDual( edge5 );

        //now test the HalfEdge at the former convex hull for swappint
        unsigned int index = ccwedge;
        unsigned int toswap;
        while ( true )
        {
          toswap = index;
          index = mHalfEdge[mHalfEdge[mHalfEdge[index]->getNext()]->getDual()]->getNext();
          checkSwap( toswap );
          if ( toswap == cwedge )
          {
            break;
          }
        }
      }

      //point is inside the convex hull------------------------------------------------------


      else if ( number >= 0 )
      {
        int nextnumber = mHalfEdge[number]->getNext();
        int nextnextnumber = mHalfEdge[mHalfEdge[number]->getNext()]->getNext();

        //insert 6 new HalfEdges for the connections to the vertices of the triangle
        unsigned int edge1 = insertEdge( -10, nextnumber, mHalfEdge[number]->getPoint(), false, false );
        unsigned int edge2 = insertEdge(( int )edge1, -10, mPointVector.count() - 1, false, false );
        unsigned int edge3 = insertEdge( -10, nextnextnumber, mHalfEdge[nextnumber]->getPoint(), false, false );
        unsigned int edge4 = insertEdge(( int )edge3, ( int )edge1, mPointVector.count() - 1, false, false );
        unsigned int edge5 = insertEdge( -10, number, mHalfEdge[nextnextnumber]->getPoint(), false, false );
        unsigned int edge6 = insertEdge(( int )edge5, ( int )edge3, mPointVector.count() - 1, false, false );


        mHalfEdge.at( edge1 )->setDual(( int )edge2 );
        mHalfEdge.at( edge2 )->setNext(( int )edge5 );
        mHalfEdge.at( edge3 )->setDual(( int )edge4 );
        mHalfEdge.at( edge5 )->setDual(( int )edge6 );
        mHalfEdge.at( number )->setNext(( int )edge2 );
        mHalfEdge.at( nextnumber )->setNext(( int )edge4 );
        mHalfEdge.at( nextnextnumber )->setNext(( int )edge6 );

        //check, if there are swaps necessary
        checkSwap( number );
        checkSwap( nextnumber );
        checkSwap( nextnextnumber );
      }

      //the point is exactly on an existing edge (the number of the edge is stored in the variable 'mEdgeWithPoint'---------------
      else if ( number == -20 )
      {
        int edgea = mEdgeWithPoint;
        int edgeb = mHalfEdge[mEdgeWithPoint]->getDual();
        int edgec = mHalfEdge[edgea]->getNext();
        int edged = mHalfEdge[edgec]->getNext();
        int edgee = mHalfEdge[edgeb]->getNext();
        int edgef = mHalfEdge[edgee]->getNext();

        //insert the six new edges
        int nedge1 = insertEdge( -10, mHalfEdge[edgea]->getNext(), mHalfEdge[edgea]->getPoint(), false, false );
        int nedge2 = insertEdge( nedge1, -10, mPointVector.count() - 1, false, false );
        int nedge3 = insertEdge( -10, edged, mHalfEdge[edgec]->getPoint(), false, false );
        int nedge4 = insertEdge( nedge3, nedge1, mPointVector.count() - 1, false, false );
        int nedge5 = insertEdge( -10, edgef, mHalfEdge[edgee]->getPoint(), false, false );
        int nedge6 = insertEdge( nedge5, edgeb, mPointVector.count() - 1, false, false );

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
        checkSwap( edgec );
        checkSwap( edged );
        checkSwap( edgee );
        checkSwap( edgef );
      }

      else if ( number == -100 || number == -5 )//this means unknown problems or a numerical error occured in 'baseEdgeOfTriangle'
      {
        // QgsDebugMsg("point has not been inserted because of unknown problems");
        Point3D* p = mPointVector[mPointVector.count()-1];
        mPointVector.remove( mPointVector.count() - 1 );
        delete p;
        return -100;
      }
      else if ( number == -25 )//this means that the point has already been inserted in the triangulation
      {
        //Take the higher z-Value in case of two equal points
        Point3D* newPoint = mPointVector[mPointVector.count()-1];
        Point3D* existingPoint = mPointVector[mTwiceInsPoint];
        existingPoint->setZ( std::max( newPoint->getZ(), existingPoint->getZ() ) );

        mPointVector.remove( mPointVector.count() - 1 );
        delete newPoint;
        return mTwiceInsPoint;
      }
    }

    return ( mPointVector.count() - 1 );
  }
  else
  {
    QgsDebugMsg( "warning: null pointer" );
    return -100;
  }
}

int DualEdgeTriangulation::baseEdgeOfPoint( int point )
{
  unsigned int actedge = mEdgeInside;//starting edge

  if ( mPointVector.count() < 4 || point == -1 )//at the beginning, mEdgeInside is not defined yet
  {
    //first find pointingedge(an edge pointing to p1)
    for ( int i = 0;i < mHalfEdge.count();i++ )
    {
      if ( mHalfEdge[i]->getPoint() == point )//we found it
      {
        return i;
      }
    }
  }

  int control = 0;

  while ( true )//otherwise, start the search
  {
    control += 1;
    if ( control > 1000000 )
    {
      // QgsDebugMsg( "warning, endless loop" );

      //use the secure and slow method
      for ( int i = 0;i < mHalfEdge.count();i++ )
      {
        if ( mHalfEdge[i]->getPoint() == point && mHalfEdge[mHalfEdge[i]->getNext()]->getPoint() != -1 )//we found it
        {
          return i;
        }
      }
    }

    int frompoint = mHalfEdge[mHalfEdge[actedge]->getDual()]->getPoint();
    int topoint = mHalfEdge[actedge]->getPoint();

    if ( frompoint == -1 || topoint == -1 )//this would cause a crash. Therefore we use the slow method in this case
    {
      for ( int i = 0;i < mHalfEdge.count();i++ )
      {
        if ( mHalfEdge[i]->getPoint() == point && mHalfEdge[mHalfEdge[i]->getNext()]->getPoint() != -1 )//we found it
        {
          return i;
        }
      }
    }

    double leftofnumber = MathUtils::leftOf( mPointVector[point], mPointVector[mHalfEdge[mHalfEdge[actedge]->getDual()]->getPoint()], mPointVector[mHalfEdge[actedge]->getPoint()] );


    if ( mHalfEdge[actedge]->getPoint() == point && mHalfEdge[mHalfEdge[actedge]->getNext()]->getPoint() != -1 )//we found the edge
    {
      return actedge;
      break;
    }

    else if ( leftofnumber <= 0 )
    {
      actedge = mHalfEdge[actedge]->getNext();
    }

    else if ( leftofnumber > 0 )
    {
      actedge = mHalfEdge[mHalfEdge[mHalfEdge[mHalfEdge[actedge]->getDual()]->getNext()]->getNext()]->getDual();
    }
  }
}

int DualEdgeTriangulation::baseEdgeOfTriangle( Point3D* point )
{
  unsigned int actedge = mEdgeInside;//start with an edge which does not point to the virtual point (usualy number 3)
  int counter = 0;//number of consecutive successful left-of-tests
  int nulls = 0;//number of left-of-tests, which returned 0. 1 means, that the point is on a line, 2 means that it is on an existing point
  int numinstabs = 0;//number of suspect left-of-tests due to 'leftOfTresh'
  int runs = 0;//counter for the number of iterations in the loop to prevent an endless loop
  int firstendp = 0, secendp = 0, thendp = 0, fouendp = 0; //four numbers of endpoints in cases when two left-of-test are 0

  while ( true )
  {
    if ( runs > nBaseOfRuns )//prevents endless loops
    {
      // QgsDebugMsg("warning, probable endless loop detected");
      return -100;
    }

    double leftofvalue = MathUtils::leftOf( point, mPointVector[mHalfEdge[mHalfEdge[actedge]->getDual()]->getPoint()], mPointVector[mHalfEdge[actedge]->getPoint()] );

    if ( leftofvalue < ( -leftOfTresh ) )//point is on the left side
    {
      counter += 1;
      if ( counter == 3 )//three successful passes means that we have found the triangle
      {
        break;
      }
    }

    else if ( leftofvalue == 0 )//point is exactly in the line of the edge
    {
      if ( nulls == 0 )
      {
        //store the numbers of the two endpoints of the line
        firstendp = mHalfEdge[mHalfEdge[actedge]->getDual()]->getPoint();
        secendp = mHalfEdge[actedge]->getPoint();
      }
      else if ( nulls == 1 )
      {
        //store the numbers of the two endpoints of the line
        thendp = mHalfEdge[mHalfEdge[actedge]->getDual()]->getPoint();
        fouendp = mHalfEdge[actedge]->getPoint();
      }
      counter += 1;
      mEdgeWithPoint = actedge;
      nulls += 1;
      if ( counter == 3 )//three successful passes means that we have found the triangle
      {
        break;
      }
    }

    else if ( leftofvalue < leftOfTresh )//numerical problems
    {
      counter += 1;
      numinstabs += 1;
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
        return -20;
      }
      mEdgeOutside = ( unsigned int )mHalfEdge[mHalfEdge[actedge]->getNext()]->getNext();
      mEdgeInside = mHalfEdge[mHalfEdge[mEdgeOutside]->getDual()]->getNext();
      return -10;//the point is outside the convex hull
    }
    runs++;
  }

  if ( numinstabs > 0 )//we hit an existing point or a numerical instability occured
  {
    // QgsDebugMsg("numerical instability occured");
    mUnstableEdge = actedge;
    return -5;
  }

  if ( nulls == 2 )
  {
    //find out the number of the point, which has already been inserted
    if ( firstendp == thendp || firstendp == fouendp )
    {
      //firstendp is the number of the point which has been inserted twice
      mTwiceInsPoint = firstendp;
      // QgsDebugMsg(QString("point nr %1 already inserted").arg(firstendp));
    }
    else if ( secendp == thendp || secendp == fouendp )
    {
      //secendp is the number of the point which has been inserted twice
      mTwiceInsPoint = secendp;
      // QgsDebugMsg(QString("point nr %1 already inserted").arg(secendp));
    }

    return -25;//return the code for a point that is already contained in the triangulation
  }

  if ( nulls == 1 )//point is on an existing edge
  {
    return -20;
  }

  mEdgeInside = actedge;

  int nr1, nr2, nr3;
  nr1 = mHalfEdge[actedge]->getPoint();
  nr2 = mHalfEdge[mHalfEdge[actedge]->getNext()]->getPoint();
  nr3 = mHalfEdge[mHalfEdge[mHalfEdge[actedge]->getNext()]->getNext()]->getPoint();
  double x1 = mPointVector[nr1]->getX();
  double y1 = mPointVector[nr1]->getY();
  double x2 = mPointVector[nr2]->getX();
  double y2 = mPointVector[nr2]->getY();
  double x3 = mPointVector[nr3]->getX();
  double y3 = mPointVector[nr3]->getY();

  //now make sure that always the same edge is returned
  if ( x1 < x2 && x1 < x3 )//return the edge which points to the point with the lowest x-coordinate
  {
    return actedge;
  }
  else if ( x2 < x1 && x2 < x3 )
  {
    return mHalfEdge[actedge]->getNext();
  }
  else if ( x3 < x1 && x3 < x2 )
  {
    return mHalfEdge[mHalfEdge[actedge]->getNext()]->getNext();
  }
  //in case two x-coordinates are the same, the edge pointing to the point with the lower y-coordinate is returned
  else if ( x1 == x2 )
  {
    if ( y1 < y2 )
    {
      return actedge;
    }
    else if ( y2 < y1 )
    {
      return mHalfEdge[actedge]->getNext();
    }
  }
  else if ( x2 == x3 )
  {
    if ( y2 < y3 )
    {
      return mHalfEdge[actedge]->getNext();
    }
    else if ( y3 < y2 )
    {
      return mHalfEdge[mHalfEdge[actedge]->getNext()]->getNext();
    }
  }
  else if ( x1 == x3 )
  {
    if ( y1 < y3 )
    {
      return actedge;
    }
    else if ( y3 < y1 )
    {
      return mHalfEdge[mHalfEdge[actedge]->getNext()]->getNext();
    }
  }
  return -100;//this means a bug happened
}

bool DualEdgeTriangulation::calcNormal( double x, double y, Vector3D* result )
{
  if ( result && mTriangleInterpolator )
  {
    if ( !mTriangleInterpolator->calcNormVec( x, y, result ) )
      {return false;}
    return true;
  }
  else
  {
    QgsDebugMsg( "warning, null pointer" );
    return false;
  }
}

bool DualEdgeTriangulation::calcPoint( double x, double y, Point3D* result )
{
  if ( result && mTriangleInterpolator )
  {
    if ( !mTriangleInterpolator->calcPoint( x, y, result ) )
      {return false;}
    return true;
  }
  else
  {
    QgsDebugMsg( "warning, null pointer" );
    return false;
  }
}

bool DualEdgeTriangulation::checkSwap( unsigned int edge )
{
  if ( swapPossible( edge ) )
  {
    Point3D* pta = mPointVector[mHalfEdge[edge]->getPoint()];
    Point3D* ptb = mPointVector[mHalfEdge[mHalfEdge[edge]->getNext()]->getPoint()];
    Point3D* ptc = mPointVector[mHalfEdge[mHalfEdge[mHalfEdge[edge]->getNext()]->getNext()]->getPoint()];
    Point3D* ptd = mPointVector[mHalfEdge[mHalfEdge[mHalfEdge[edge]->getDual()]->getNext()]->getPoint()];

    if ( MathUtils::inCircle( ptd, pta, ptb, ptc ) )//empty circle criterion violated
    {
      doSwap( edge );//swap the edge (recursiv)
      return true;
    }
  }
  return false;
}

void DualEdgeTriangulation::doOnlySwap( unsigned int edge )
{
  unsigned int edge1 = edge;
  unsigned int edge2 = mHalfEdge[edge]->getDual();
  unsigned int edge3 = mHalfEdge[edge]->getNext();
  unsigned int edge4 = mHalfEdge[mHalfEdge[edge]->getNext()]->getNext();
  unsigned int edge5 = mHalfEdge[mHalfEdge[edge]->getDual()]->getNext();
  unsigned int edge6 = mHalfEdge[mHalfEdge[mHalfEdge[edge]->getDual()]->getNext()]->getNext();
  mHalfEdge[edge1]->setNext( edge4 );//set the necessary nexts
  mHalfEdge[edge2]->setNext( edge6 );
  mHalfEdge[edge3]->setNext( edge2 );
  mHalfEdge[edge4]->setNext( edge5 );
  mHalfEdge[edge5]->setNext( edge1 );
  mHalfEdge[edge6]->setNext( edge3 );
  mHalfEdge[edge1]->setPoint( mHalfEdge[edge3]->getPoint() );//change the points to which edge1 and edge2 point
  mHalfEdge[edge2]->setPoint( mHalfEdge[edge5]->getPoint() );
}

void DualEdgeTriangulation::doSwap( unsigned int edge )
{
  unsigned int edge1 = edge;
  unsigned int edge2 = mHalfEdge[edge]->getDual();
  unsigned int edge3 = mHalfEdge[edge]->getNext();
  unsigned int edge4 = mHalfEdge[mHalfEdge[edge]->getNext()]->getNext();
  unsigned int edge5 = mHalfEdge[mHalfEdge[edge]->getDual()]->getNext();
  unsigned int edge6 = mHalfEdge[mHalfEdge[mHalfEdge[edge]->getDual()]->getNext()]->getNext();
  mHalfEdge[edge1]->setNext( edge4 );//set the necessary nexts
  mHalfEdge[edge2]->setNext( edge6 );
  mHalfEdge[edge3]->setNext( edge2 );
  mHalfEdge[edge4]->setNext( edge5 );
  mHalfEdge[edge5]->setNext( edge1 );
  mHalfEdge[edge6]->setNext( edge3 );
  mHalfEdge[edge1]->setPoint( mHalfEdge[edge3]->getPoint() );//change the points to which edge1 and edge2 point
  mHalfEdge[edge2]->setPoint( mHalfEdge[edge5]->getPoint() );
  checkSwap( edge3 );
  checkSwap( edge6 );
  checkSwap( edge4 );
  checkSwap( edge5 );
}

#if 0
void DualEdgeTriangulation::draw( QPainter* p, double xlowleft, double ylowleft, double xupright, double yupright, double width, double height ) const
{
  //if mPointVector is empty, there is nothing to do
  if ( mPointVector.count() == 0 )
  {
    return;
  }

  p->setPen( mEdgeColor );

  bool* control = new bool[mHalfEdge.count()];//controllarray that no edge is painted twice
  bool* control2 = new bool[mHalfEdge.count()];//controllarray for the flat triangles

  for ( unsigned int i = 0;i <= mHalfEdge.count() - 1;i++ )
  {
    control[i] = false;
    control2[i] = false;
  }

  if ((( xupright - xlowleft ) / width ) > (( yupright - ylowleft ) / height ) )
  {
    double lowerborder = -( height * ( xupright - xlowleft ) / width - yupright );//real world coordinates of the lower widget border. This is useful to know because of the HalfEdge bounding box test
    for ( unsigned int i = 0;i < mHalfEdge.count() - 1;i++ )
    {
      if ( mHalfEdge[i]->getPoint() == -1 || mHalfEdge[mHalfEdge[i]->getDual()]->getPoint() == -1 )
        {continue;}

      //check, if the edge belongs to a flat triangle, remove this later
      if ( control2[i] == false )
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

      if ( control[i] == true )//check, if edge has already been drawn
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


        p->drawLine(( mPointVector[mHalfEdge[i]->getPoint()]->getX() - xlowleft ) / ( xupright - xlowleft )*width, ( yupright - mPointVector[mHalfEdge[i]->getPoint()]->getY() ) / ( xupright - xlowleft )*width, ( mPointVector[mHalfEdge[mHalfEdge[i]->getDual()]->getPoint()]->getX() - xlowleft ) / ( xupright - xlowleft )*width, ( yupright - mPointVector[mHalfEdge[mHalfEdge[i]->getDual()]->getPoint()]->getY() ) / ( xupright - xlowleft )*width );

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
    for ( unsigned int i = 0;i < mHalfEdge.count() - 1;i++ )
    {
      if ( mHalfEdge[i]->getPoint() == -1 || mHalfEdge[mHalfEdge[i]->getDual()]->getPoint() == -1 )
        {continue;}

      //check, if the edge belongs to a flat triangle, remove this section later
      if ( control2[i] == false )
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


      if ( control[i] == true )//check, if edge has already been drawn
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

        p->drawLine(( mPointVector[mHalfEdge[i]->getPoint()]->getX() - xlowleft ) / ( yupright - ylowleft )*height, ( yupright - mPointVector[mHalfEdge[i]->getPoint()]->getY() ) / ( yupright - ylowleft )*height, ( mPointVector[mHalfEdge[mHalfEdge[i]->getDual()]->getPoint()]->getX() - xlowleft ) / ( yupright - ylowleft )*height, ( yupright - mPointVector[mHalfEdge[mHalfEdge[i]->getDual()]->getPoint()]->getY() ) / ( yupright - ylowleft )*height );

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

int DualEdgeTriangulation::getOppositePoint( int p1, int p2 )
{

  //first find a half edge which points to p2
  int firstedge = baseEdgeOfPoint( p2 );

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
    QgsDebugMsg( QString( "warning, error: the points are: %1 and %2" ).arg( p1 ).arg( p2 ) );
    return -10;
  }

  //finally find the opposite point
  return mHalfEdge[mHalfEdge[mHalfEdge[theedge]->getDual()]->getNext()]->getPoint();

}

QList<int>* DualEdgeTriangulation::getSurroundingTriangles( int pointno )
{
  int firstedge = baseEdgeOfPoint( pointno );

  if ( firstedge == -1 )//an error occured
  {
    return 0;
  }

  QList<int>* vlist = new QList<int>();//create the value list on the heap

  int actedge = firstedge;
  int edge, nextedge, nextnextedge;
  do
  {
    edge = mHalfEdge[actedge]->getDual();
    vlist->append( mHalfEdge[edge]->getPoint() );//add the number of the endpoint of the first edge to the value list
    nextedge = mHalfEdge[edge]->getNext();
    vlist->append( mHalfEdge[nextedge]->getPoint() );//add the number of the endpoint of the second edge to the value list
    nextnextedge = mHalfEdge[nextedge]->getNext();
    vlist->append( mHalfEdge[nextnextedge]->getPoint() );//add the number of endpoint of the third edge to the value list
    if ( mHalfEdge[nextnextedge]->getBreak() == true )//add, whether the third edge is a breakline or not
    {
      vlist->append( -10 );
    }
    else
    {
      vlist->append( -20 );
    }
    actedge = nextnextedge;
  }
  while ( nextnextedge != firstedge );

  return vlist;

}

bool DualEdgeTriangulation::getTriangle( double x, double y, Point3D* p1, int* n1, Point3D* p2, int* n2, Point3D* p3, int* n3 )
{
  if ( mPointVector.size() < 3 )
  {
    return false;
  }

  if ( p1 && p2 && p3 )
  {
    Point3D point( x, y, 0 );
    int edge = baseEdgeOfTriangle( &point );
    if ( edge == -10 )//the point is outside the convex hull
    {
      QgsDebugMsg( "edge outside the convex hull" );
      return false;
    }

    else if ( edge >= 0 )//the point is inside the convex hull
    {
      int ptnr1 = mHalfEdge[edge]->getPoint();
      int ptnr2 = mHalfEdge[mHalfEdge[edge]->getNext()]->getPoint();
      int ptnr3 = mHalfEdge[mHalfEdge[mHalfEdge[edge]->getNext()]->getNext()]->getPoint();
      p1->setX( mPointVector[ptnr1]->getX() );
      p1->setY( mPointVector[ptnr1]->getY() );
      p1->setZ( mPointVector[ptnr1]->getZ() );
      p2->setX( mPointVector[ptnr2]->getX() );
      p2->setY( mPointVector[ptnr2]->getY() );
      p2->setZ( mPointVector[ptnr2]->getZ() );
      p3->setX( mPointVector[ptnr3]->getX() );
      p3->setY( mPointVector[ptnr3]->getY() );
      p3->setZ( mPointVector[ptnr3]->getZ() );
      ( *n1 ) = ptnr1;
      ( *n2 ) = ptnr2;
      ( *n3 ) = ptnr3;
      return true;
    }
    else if ( edge == -20 )//the point is exactly on an edge
    {
      int ptnr1 = mHalfEdge[mEdgeWithPoint]->getPoint();
      int ptnr2 = mHalfEdge[mHalfEdge[mEdgeWithPoint]->getNext()]->getPoint();
      int ptnr3 = mHalfEdge[mHalfEdge[mHalfEdge[mEdgeWithPoint]->getNext()]->getNext()]->getPoint();
      if ( ptnr1 == -1 || ptnr2 == -1 || ptnr3 == -1 )
      {
        return false;
      }
      p1->setX( mPointVector[ptnr1]->getX() );
      p1->setY( mPointVector[ptnr1]->getY() );
      p1->setZ( mPointVector[ptnr1]->getZ() );
      p2->setX( mPointVector[ptnr2]->getX() );
      p2->setY( mPointVector[ptnr2]->getY() );
      p2->setZ( mPointVector[ptnr2]->getZ() );
      p3->setX( mPointVector[ptnr3]->getX() );
      p3->setY( mPointVector[ptnr3]->getY() );
      p3->setZ( mPointVector[ptnr3]->getZ() );
      ( *n1 ) = ptnr1;
      ( *n2 ) = ptnr2;
      ( *n3 ) = ptnr3;
      return true;
    }
    else if ( edge == -25 )//x and y are the coordinates of an existing point
    {
      int edge1 = baseEdgeOfPoint( mTwiceInsPoint );
      int edge2 = mHalfEdge[edge1]->getNext();
      int edge3 = mHalfEdge[edge2]->getNext();
      int ptnr1 = mHalfEdge[edge1]->getPoint();
      int ptnr2 = mHalfEdge[edge2]->getPoint();
      int ptnr3 = mHalfEdge[edge3]->getPoint();
      p1->setX( mPointVector[ptnr1]->getX() );
      p1->setY( mPointVector[ptnr1]->getY() );
      p1->setZ( mPointVector[ptnr1]->getZ() );
      p2->setX( mPointVector[ptnr2]->getX() );
      p2->setY( mPointVector[ptnr2]->getY() );
      p2->setZ( mPointVector[ptnr2]->getZ() );
      p3->setX( mPointVector[ptnr3]->getX() );
      p3->setY( mPointVector[ptnr3]->getY() );
      p3->setZ( mPointVector[ptnr3]->getZ() );
      ( *n1 ) = ptnr1;
      ( *n2 ) = ptnr2;
      ( *n3 ) = ptnr3;
      return true;
    }
    else if ( edge == -5 )//numerical problems in 'baseEdgeOfTriangle'
    {
      int ptnr1 = mHalfEdge[mUnstableEdge]->getPoint();
      int ptnr2 = mHalfEdge[mHalfEdge[mUnstableEdge]->getNext()]->getPoint();
      int ptnr3 = mHalfEdge[mHalfEdge[mHalfEdge[mUnstableEdge]->getNext()]->getNext()]->getPoint();
      if ( ptnr1 == -1 || ptnr2 == -1 || ptnr3 == -1 )
      {
        return false;
      }
      p1->setX( mPointVector[ptnr1]->getX() );
      p1->setY( mPointVector[ptnr1]->getY() );
      p1->setZ( mPointVector[ptnr1]->getZ() );
      p2->setX( mPointVector[ptnr2]->getX() );
      p2->setY( mPointVector[ptnr2]->getY() );
      p2->setZ( mPointVector[ptnr2]->getZ() );
      p3->setX( mPointVector[ptnr3]->getX() );
      p3->setY( mPointVector[ptnr3]->getY() );
      p3->setZ( mPointVector[ptnr3]->getZ() );
      ( *n1 ) = ptnr1;
      ( *n2 ) = ptnr2;
      ( *n3 ) = ptnr3;
      return true;
    }
    else//problems
    {
      QgsDebugMsg( QString( "problem: the edge is: %1" ).arg( edge ) );
      return false;
    }
  }

  else
  {
    QgsDebugMsg( "warning, null pointer" );
    return false;
  }
}

bool DualEdgeTriangulation::getTriangle( double x, double y, Point3D* p1, Point3D* p2, Point3D* p3 )
{
  if ( mPointVector.size() < 3 )
  {
    return false;
  }

  if ( p1 && p2 && p3 )
  {
    Point3D point( x, y, 0 );
    int edge = baseEdgeOfTriangle( &point );
    if ( edge == -10 )//the point is outside the convex hull
    {
      QgsDebugMsg( "edge outside the convex hull" );
      return false;
    }
    else if ( edge >= 0 )//the point is inside the convex hull
    {
      int ptnr1 = mHalfEdge[edge]->getPoint();
      int ptnr2 = mHalfEdge[mHalfEdge[edge]->getNext()]->getPoint();
      int ptnr3 = mHalfEdge[mHalfEdge[mHalfEdge[edge]->getNext()]->getNext()]->getPoint();
      p1->setX( mPointVector[ptnr1]->getX() );
      p1->setY( mPointVector[ptnr1]->getY() );
      p1->setZ( mPointVector[ptnr1]->getZ() );
      p2->setX( mPointVector[ptnr2]->getX() );
      p2->setY( mPointVector[ptnr2]->getY() );
      p2->setZ( mPointVector[ptnr2]->getZ() );
      p3->setX( mPointVector[ptnr3]->getX() );
      p3->setY( mPointVector[ptnr3]->getY() );
      p3->setZ( mPointVector[ptnr3]->getZ() );
      return true;
    }
    else if ( edge == -20 )//the point is exactly on an edge
    {
      int ptnr1 = mHalfEdge[mEdgeWithPoint]->getPoint();
      int ptnr2 = mHalfEdge[mHalfEdge[mEdgeWithPoint]->getNext()]->getPoint();
      int ptnr3 = mHalfEdge[mHalfEdge[mHalfEdge[mEdgeWithPoint]->getNext()]->getNext()]->getPoint();
      if ( ptnr1 == -1 || ptnr2 == -1 || ptnr3 == -1 )
      {
        return false;
      }
      p1->setX( mPointVector[ptnr1]->getX() );
      p1->setY( mPointVector[ptnr1]->getY() );
      p1->setZ( mPointVector[ptnr1]->getZ() );
      p2->setX( mPointVector[ptnr2]->getX() );
      p2->setY( mPointVector[ptnr2]->getY() );
      p2->setZ( mPointVector[ptnr2]->getZ() );
      p3->setX( mPointVector[ptnr3]->getX() );
      p3->setY( mPointVector[ptnr3]->getY() );
      p3->setZ( mPointVector[ptnr3]->getZ() );
      return true;
    }
    else if ( edge == -25 )//x and y are the coordinates of an existing point
    {
      int edge1 = baseEdgeOfPoint( mTwiceInsPoint );
      int edge2 = mHalfEdge[edge1]->getNext();
      int edge3 = mHalfEdge[edge2]->getNext();
      int ptnr1 = mHalfEdge[edge1]->getPoint();
      int ptnr2 = mHalfEdge[edge2]->getPoint();
      int ptnr3 = mHalfEdge[edge3]->getPoint();
      if ( ptnr1 == -1 || ptnr2 == -1 || ptnr3 == -1 )
      {
        return false;
      }
      p1->setX( mPointVector[ptnr1]->getX() );
      p1->setY( mPointVector[ptnr1]->getY() );
      p1->setZ( mPointVector[ptnr1]->getZ() );
      p2->setX( mPointVector[ptnr2]->getX() );
      p2->setY( mPointVector[ptnr2]->getY() );
      p2->setZ( mPointVector[ptnr2]->getZ() );
      p3->setX( mPointVector[ptnr3]->getX() );
      p3->setY( mPointVector[ptnr3]->getY() );
      p3->setZ( mPointVector[ptnr3]->getZ() );
      return true;
    }
    else if ( edge == -5 )//numerical problems in 'baseEdgeOfTriangle'
    {
      int ptnr1 = mHalfEdge[mUnstableEdge]->getPoint();
      int ptnr2 = mHalfEdge[mHalfEdge[mUnstableEdge]->getNext()]->getPoint();
      int ptnr3 = mHalfEdge[mHalfEdge[mHalfEdge[mUnstableEdge]->getNext()]->getNext()]->getPoint();
      if ( ptnr1 == -1 || ptnr2 == -1 || ptnr3 == -1 )
      {
        return false;
      }
      p1->setX( mPointVector[ptnr1]->getX() );
      p1->setY( mPointVector[ptnr1]->getY() );
      p1->setZ( mPointVector[ptnr1]->getZ() );
      p2->setX( mPointVector[ptnr2]->getX() );
      p2->setY( mPointVector[ptnr2]->getY() );
      p2->setZ( mPointVector[ptnr2]->getZ() );
      p3->setX( mPointVector[ptnr3]->getX() );
      p3->setY( mPointVector[ptnr3]->getY() );
      p3->setZ( mPointVector[ptnr3]->getZ() );
      return true;
    }
    else//problems
    {
      QgsDebugMsg( QString( "problems: the edge is: %1" ).arg( edge ) );
      return false;
    }
  }

  else
  {
    QgsDebugMsg( "warning, null pointer" );
    return false;
  }
}

unsigned int DualEdgeTriangulation::insertEdge( int dual, int next, int point, bool mbreak, bool forced )
{
  HalfEdge* edge = new HalfEdge( dual, next, point, mbreak, forced );
  mHalfEdge.append( edge );
  return mHalfEdge.count() - 1;

}

int DualEdgeTriangulation::insertForcedSegment( int p1, int p2, bool breakline )
{
  if ( p1 == p2 )
  {
    return 0;
  }

  //list with (half of) the crossed edges
  QList<int> crossedEdges;

  //an edge pointing to p1
  int pointingedge = 0;

  pointingedge = baseEdgeOfPoint( p1 );

  if ( pointingedge == -1 )
  {
    return -100;//return an error code
  }

  //go around p1 and find out, if the segment already exists and if not, which is the first cutted edge
  int actedge = mHalfEdge[pointingedge]->getDual();
  //number to prevent endless loops
  int control = 0;

  while ( true )//if it's an endless loop, something went wrong
  {
    control += 1;
    if ( control > 17000 )
    {
      QgsDebugMsg( "warning, endless loop" );
      return -100;//return an error code
    }

    if ( mHalfEdge[actedge]->getPoint() == -1 )//actedge points to the virtual point
    {
      actedge = mHalfEdge[mHalfEdge[mHalfEdge[actedge]->getNext()]->getNext()]->getDual();
      continue;
    }

    //test, if actedge is already the forced edge
    if ( mHalfEdge[actedge]->getPoint() == p2 )
    {
      mHalfEdge[actedge]->setForced( true );
      mHalfEdge[actedge]->setBreak( breakline );
      mHalfEdge[mHalfEdge[actedge]->getDual()]->setForced( true );
      mHalfEdge[mHalfEdge[actedge]->getDual()]->setBreak( breakline );
      return actedge;
    }

    //test, if the forced segment is a multiple of actedge and if the direction is the same
    else if ( /*lines are parallel*/( mPointVector[p2]->getY() - mPointVector[p1]->getY() ) / ( mPointVector[mHalfEdge[actedge]->getPoint()]->getY() - mPointVector[p1]->getY() ) == ( mPointVector[p2]->getX() - mPointVector[p1]->getX() ) / ( mPointVector[mHalfEdge[actedge]->getPoint()]->getX() - mPointVector[p1]->getX() ) && (( mPointVector[p2]->getY() - mPointVector[p1]->getY() ) >= 0 ) == (( mPointVector[mHalfEdge[actedge]->getPoint()]->getY() - mPointVector[p1]->getY() ) > 0 ) && (( mPointVector[p2]->getX() - mPointVector[p1]->getX() ) >= 0 ) == (( mPointVector[mHalfEdge[actedge]->getPoint()]->getX() - mPointVector[p1]->getX() ) > 0 ) )
    {
      //mark actedge and Dual(actedge) as forced, reset p1 and start the method from the beginning
      mHalfEdge[actedge]->setForced( true );
      mHalfEdge[actedge]->setBreak( breakline );
      mHalfEdge[mHalfEdge[actedge]->getDual()]->setForced( true );
      mHalfEdge[mHalfEdge[actedge]->getDual()]->setBreak( breakline );
      int a = insertForcedSegment( mHalfEdge[actedge]->getPoint(), p2, breakline );
      return a;
    }

    //test, if the forced segment intersects Next(actedge)
    if ( mHalfEdge[mHalfEdge[actedge]->getNext()]->getPoint() == -1 )//intersection with line to the virtual point makes no sense
    {
      actedge = mHalfEdge[mHalfEdge[mHalfEdge[actedge]->getNext()]->getNext()]->getDual();
      continue;
    }
    else if ( MathUtils::lineIntersection( mPointVector[p1], mPointVector[p2], mPointVector[mHalfEdge[mHalfEdge[actedge]->getNext()]->getPoint()], mPointVector[mHalfEdge[mHalfEdge[mHalfEdge[actedge]->getNext()]->getDual()]->getPoint()] ) )
    {
      if ( mHalfEdge[mHalfEdge[actedge]->getNext()]->getForced() && mForcedCrossBehaviour == Triangulation::SnappingType_VERTICE )//if the crossed edge is a forced edge, we have to snap the forced line to the next node
      {
        Point3D crosspoint;
        int p3, p4;
        p3 = mHalfEdge[mHalfEdge[actedge]->getNext()]->getPoint();
        p4 = mHalfEdge[mHalfEdge[mHalfEdge[actedge]->getNext()]->getDual()]->getPoint();
        MathUtils::lineIntersection( mPointVector[p1], mPointVector[p2], mPointVector[p3], mPointVector[p4], &crosspoint );
        double dista = sqrt(( crosspoint.getX() - mPointVector[p3]->getX() ) * ( crosspoint.getX() - mPointVector[p3]->getX() ) + ( crosspoint.getY() - mPointVector[p3]->getY() ) * ( crosspoint.getY() - mPointVector[p3]->getY() ) );
        double distb = sqrt(( crosspoint.getX() - mPointVector[p4]->getX() ) * ( crosspoint.getX() - mPointVector[p4]->getX() ) + ( crosspoint.getY() - mPointVector[p4]->getY() ) * ( crosspoint.getY() - mPointVector[p4]->getY() ) );
        if ( dista <= distb )
        {
          insertForcedSegment( p1, p3, breakline );
          int e = insertForcedSegment( p3, p2, breakline );
          return e;
        }
        else if ( distb <= dista )
        {
          insertForcedSegment( p1, p4, breakline );
          int e = insertForcedSegment( p4, p2, breakline );
          return e;
        }
      }
      else if ( mHalfEdge[mHalfEdge[actedge]->getNext()]->getForced() && mForcedCrossBehaviour == Triangulation::INSERT_VERTICE )//if the crossed edge is a forced edge, we have to insert a new vertice on this edge
      {
        Point3D crosspoint;
        int p3, p4;
        p3 = mHalfEdge[mHalfEdge[actedge]->getNext()]->getPoint();
        p4 = mHalfEdge[mHalfEdge[mHalfEdge[actedge]->getNext()]->getDual()]->getPoint();
        MathUtils::lineIntersection( mPointVector[p1], mPointVector[p2], mPointVector[p3], mPointVector[p4], &crosspoint );
        double distpart = sqrt(( crosspoint.getX() - mPointVector[p4]->getX() ) * ( crosspoint.getX() - mPointVector[p4]->getX() ) + ( crosspoint.getY() - mPointVector[p4]->getY() ) * ( crosspoint.getY() - mPointVector[p4]->getY() ) );
        double disttot = sqrt(( mPointVector[p3]->getX() - mPointVector[p4]->getX() ) * ( mPointVector[p3]->getX() - mPointVector[p4]->getX() ) + ( mPointVector[p3]->getY() - mPointVector[p4]->getY() ) * ( mPointVector[p3]->getY() - mPointVector[p4]->getY() ) );
        float frac = distpart / disttot;

        if ( frac == 0 || frac == 1 )//just in case MathUtils::lineIntersection does not work as expected
        {
          if ( frac == 0 )
          {
            //mark actedge and Dual(actedge) as forced, reset p1 and start the method from the beginning
            mHalfEdge[actedge]->setForced( true );
            mHalfEdge[actedge]->setBreak( breakline );
            mHalfEdge[mHalfEdge[actedge]->getDual()]->setForced( true );
            mHalfEdge[mHalfEdge[actedge]->getDual()]->setBreak( breakline );
            int a = insertForcedSegment( p4, p2, breakline );
            return a;
          }
          else if ( frac == 1 )
          {
            //mark actedge and Dual(actedge) as forced, reset p1 and start the method from the beginning
            mHalfEdge[actedge]->setForced( true );
            mHalfEdge[actedge]->setBreak( breakline );
            mHalfEdge[mHalfEdge[actedge]->getDual()]->setForced( true );
            mHalfEdge[mHalfEdge[actedge]->getDual()]->setBreak( breakline );
            if ( p3 != p2 )
            {
              int a = insertForcedSegment( p3, p2, breakline );
              return a;
            }
            else
            {
              return actedge;
            }
          }

        }


        else
        {
          int newpoint = splitHalfEdge( mHalfEdge[actedge]->getNext(), frac );
          insertForcedSegment( p1, newpoint, breakline );
          int e = insertForcedSegment( newpoint, p2, breakline );
          return e;
        }
      }

      //add the first HalfEdge to the list of crossed edges
      crossedEdges.append( mHalfEdge[actedge]->getNext() );
      break;
    }
    actedge = mHalfEdge[mHalfEdge[mHalfEdge[actedge]->getNext()]->getNext()]->getDual();
  }

  //we found the first edge, terminated the method or called the method with other points. Lets search for all the other crossed edges

  while ( true )//if its an endless loop, something went wrong.
  {
    if ( MathUtils::lineIntersection( mPointVector[mHalfEdge[mHalfEdge[crossedEdges.last()]->getDual()]->getPoint()], mPointVector[mHalfEdge[mHalfEdge[mHalfEdge[crossedEdges.last()]->getDual()]->getNext()]->getPoint()], mPointVector[p1], mPointVector[p2] ) )
    {
      if ( mHalfEdge[mHalfEdge[mHalfEdge[crossedEdges.last()]->getDual()]->getNext()]->getForced() && mForcedCrossBehaviour == Triangulation::SnappingType_VERTICE )//if the crossed edge is a forced edge and mForcedCrossBehaviour is SnappingType_VERTICE, we have to snap the forced line to the next node
      {
        Point3D crosspoint;
        int p3, p4;
        p3 = mHalfEdge[mHalfEdge[crossedEdges.last()]->getDual()]->getPoint();
        p4 = mHalfEdge[mHalfEdge[mHalfEdge[crossedEdges.last()]->getDual()]->getNext()]->getPoint();
        MathUtils::lineIntersection( mPointVector[p1], mPointVector[p2], mPointVector[p3], mPointVector[p4], &crosspoint );
        double dista = sqrt(( crosspoint.getX() - mPointVector[p3]->getX() ) * ( crosspoint.getX() - mPointVector[p3]->getX() ) + ( crosspoint.getY() - mPointVector[p3]->getY() ) * ( crosspoint.getY() - mPointVector[p3]->getY() ) );
        double distb = sqrt(( crosspoint.getX() - mPointVector[p4]->getX() ) * ( crosspoint.getX() - mPointVector[p4]->getX() ) + ( crosspoint.getY() - mPointVector[p4]->getY() ) * ( crosspoint.getY() - mPointVector[p4]->getY() ) );
        if ( dista <= distb )
        {
          insertForcedSegment( p1, p3, breakline );
          int e = insertForcedSegment( p3, p2, breakline );
          return e;
        }
        else if ( distb <= dista )
        {
          insertForcedSegment( p1, p4, breakline );
          int e = insertForcedSegment( p4, p2, breakline );
          return e;
        }
      }
      else if ( mHalfEdge[mHalfEdge[mHalfEdge[crossedEdges.last()]->getDual()]->getNext()]->getForced() && mForcedCrossBehaviour == Triangulation::INSERT_VERTICE )//if the crossed edge is a forced edge, we have to insert a new vertice on this edge
      {
        Point3D crosspoint;
        int p3, p4;
        p3 = mHalfEdge[mHalfEdge[crossedEdges.last()]->getDual()]->getPoint();
        p4 = mHalfEdge[mHalfEdge[mHalfEdge[crossedEdges.last()]->getDual()]->getNext()]->getPoint();
        MathUtils::lineIntersection( mPointVector[p1], mPointVector[p2], mPointVector[p3], mPointVector[p4], &crosspoint );
        double distpart = sqrt(( crosspoint.getX() - mPointVector[p3]->getX() ) * ( crosspoint.getX() - mPointVector[p3]->getX() ) + ( crosspoint.getY() - mPointVector[p3]->getY() ) * ( crosspoint.getY() - mPointVector[p3]->getY() ) );
        double disttot = sqrt(( mPointVector[p3]->getX() - mPointVector[p4]->getX() ) * ( mPointVector[p3]->getX() - mPointVector[p4]->getX() ) + ( mPointVector[p3]->getY() - mPointVector[p4]->getY() ) * ( mPointVector[p3]->getY() - mPointVector[p4]->getY() ) );
        float frac = distpart / disttot;
        if ( frac == 0 || frac == 1 )
        {
          break;//seems that a roundoff error occured. We found the endpoint
        }
        int newpoint = splitHalfEdge( mHalfEdge[mHalfEdge[crossedEdges.last()]->getDual()]->getNext(), frac );
        insertForcedSegment( p1, newpoint, breakline );
        int e = insertForcedSegment( newpoint, p2, breakline );
        return e;
      }

      crossedEdges.append( mHalfEdge[mHalfEdge[crossedEdges.last()]->getDual()]->getNext() );
      continue;
    }
    else if ( MathUtils::lineIntersection( mPointVector[mHalfEdge[mHalfEdge[mHalfEdge[crossedEdges.last()]->getDual()]->getNext()]->getPoint()], mPointVector[mHalfEdge[mHalfEdge[mHalfEdge[mHalfEdge[crossedEdges.last()]->getDual()]->getNext()]->getNext()]->getPoint()], mPointVector[p1], mPointVector[p2] ) )
    {
      if ( mHalfEdge[mHalfEdge[mHalfEdge[mHalfEdge[crossedEdges.last()]->getDual()]->getNext()]->getNext()]->getForced() && mForcedCrossBehaviour == Triangulation::SnappingType_VERTICE )//if the crossed edge is a forced edge and mForcedCrossBehaviour is SnappingType_VERTICE, we have to snap the forced line to the next node
      {
        Point3D crosspoint;
        int p3, p4;
        p3 = mHalfEdge[mHalfEdge[mHalfEdge[crossedEdges.last()]->getDual()]->getNext()]->getPoint();
        p4 = mHalfEdge[mHalfEdge[mHalfEdge[mHalfEdge[crossedEdges.last()]->getDual()]->getNext()]->getNext()]->getPoint();
        MathUtils::lineIntersection( mPointVector[p1], mPointVector[p2], mPointVector[p3], mPointVector[p4], &crosspoint );
        double dista = sqrt(( crosspoint.getX() - mPointVector[p3]->getX() ) * ( crosspoint.getX() - mPointVector[p3]->getX() ) + ( crosspoint.getY() - mPointVector[p3]->getY() ) * ( crosspoint.getY() - mPointVector[p3]->getY() ) );
        double distb = sqrt(( crosspoint.getX() - mPointVector[p4]->getX() ) * ( crosspoint.getX() - mPointVector[p4]->getX() ) + ( crosspoint.getY() - mPointVector[p4]->getY() ) * ( crosspoint.getY() - mPointVector[p4]->getY() ) );
        if ( dista <= distb )
        {
          insertForcedSegment( p1, p3, breakline );
          int e = insertForcedSegment( p3, p2, breakline );
          return e;
        }
        else if ( distb <= dista )
        {
          insertForcedSegment( p1, p4, breakline );
          int e = insertForcedSegment( p4, p2, breakline );
          return e;
        }
      }
      else if ( mHalfEdge[mHalfEdge[mHalfEdge[mHalfEdge[crossedEdges.last()]->getDual()]->getNext()]->getNext()]->getForced() && mForcedCrossBehaviour == Triangulation::INSERT_VERTICE )//if the crossed edge is a forced edge, we have to insert a new vertice on this edge
      {
        Point3D crosspoint;
        int p3, p4;
        p3 = mHalfEdge[mHalfEdge[mHalfEdge[crossedEdges.last()]->getDual()]->getNext()]->getPoint();
        p4 = mHalfEdge[mHalfEdge[mHalfEdge[mHalfEdge[crossedEdges.last()]->getDual()]->getNext()]->getNext()]->getPoint();
        MathUtils::lineIntersection( mPointVector[p1], mPointVector[p2], mPointVector[p3], mPointVector[p4], &crosspoint );
        double distpart = sqrt(( crosspoint.getX() - mPointVector[p3]->getX() ) * ( crosspoint.getX() - mPointVector[p3]->getX() ) + ( crosspoint.getY() - mPointVector[p3]->getY() ) * ( crosspoint.getY() - mPointVector[p3]->getY() ) );
        double disttot = sqrt(( mPointVector[p3]->getX() - mPointVector[p4]->getX() ) * ( mPointVector[p3]->getX() - mPointVector[p4]->getX() ) + ( mPointVector[p3]->getY() - mPointVector[p4]->getY() ) * ( mPointVector[p3]->getY() - mPointVector[p4]->getY() ) );
        float frac = distpart / disttot;
        if ( frac == 0 || frac == 1 )
        {
          break;//seems that a roundoff error occured. We found the endpoint
        }
        int newpoint = splitHalfEdge( mHalfEdge[mHalfEdge[mHalfEdge[crossedEdges.last()]->getDual()]->getNext()]->getNext(), frac );
        insertForcedSegment( p1, newpoint, breakline );
        int e = insertForcedSegment( newpoint, p2, breakline );
        return e;
      }

      crossedEdges.append( mHalfEdge[mHalfEdge[mHalfEdge[crossedEdges.last()]->getDual()]->getNext()]->getNext() );
      continue;
    }
    else//forced edge terminates
    {
      break;
    }
  }

  //set the flags 'forced' and 'break' to false for every edge and dualedge of 'crossEdges'
  QList<int>::const_iterator iter;
  for ( iter = crossedEdges.constBegin();iter != crossedEdges.constEnd();++iter )
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
  int firstedge = freelist.first();//edge pointing from p1 to p2
  mHalfEdge[firstedge]->setForced( true );
  mHalfEdge[firstedge]->setBreak( breakline );
  leftPolygon.append( firstedge );
  int dualfirstedge = mHalfEdge[freelist.first()]->getDual();//edge pointing from p2 to p1
  mHalfEdge[dualfirstedge]->setForced( true );
  mHalfEdge[dualfirstedge]->setBreak( breakline );
  rightPolygon.append( dualfirstedge );
  freelist.pop_front();//delete the first entry from the freelist

  //finish the polygon on the left side
  int actpointl = p2;
  QList<int>::const_iterator leftiter; //todo: is there a better way to set an iterator to the last list element?
  leftiter = crossedEdges.constEnd();
  --leftiter;
  while ( true )
  {
    int newpoint = mHalfEdge[mHalfEdge[mHalfEdge[mHalfEdge[( *leftiter )]->getDual()]->getNext()]->getNext()]->getPoint();
    if ( newpoint != actpointl )
    {
      //insert the edge into the leftPolygon
      actpointl = newpoint;
      int theedge = mHalfEdge[mHalfEdge[mHalfEdge[( *leftiter )]->getDual()]->getNext()]->getNext();
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
  for ( rightiter = crossedEdges.constBegin();rightiter != crossedEdges.constEnd();++rightiter )
  {
    int newpoint = mHalfEdge[mHalfEdge[mHalfEdge[( *rightiter )]->getNext()]->getNext()]->getPoint();
    if ( newpoint != actpointr )
    {
      //insert the edge into the right polygon
      actpointr = newpoint;
      int theedge = mHalfEdge[mHalfEdge[( *rightiter )]->getNext()]->getNext();
      rightPolygon.append( theedge );
    }
  }


  //insert the last element into rightPolygon
  rightPolygon.append( mHalfEdge[mHalfEdge[crossedEdges.last()]->getDual()]->getNext() );
  mHalfEdge[rightPolygon.last()]->setNext( dualfirstedge );//set 'Next' of the last edge to dualfirstedge

  //set the necessary nexts of leftPolygon(exept the first)
  int actedgel = leftPolygon[1];
  leftiter = leftPolygon.constBegin(); leftiter += 2;
  for ( ;leftiter != leftPolygon.constEnd();++leftiter )
  {
    mHalfEdge[actedgel]->setNext(( *leftiter ) );
    actedgel = ( *leftiter );
  }

  //set all the necessary nexts of rightPolygon
  int actedger = rightPolygon[1];
  rightiter = rightPolygon.constBegin(); rightiter += 2;
  for ( ;rightiter != rightPolygon.constEnd();++rightiter )
  {
    mHalfEdge[actedger]->setNext(( *rightiter ) );
    actedger = ( *( rightiter ) );
  }


  //setNext and setPoint for the forced edge because this would disturb the building of 'leftpoly' and 'rightpoly' otherwise
  mHalfEdge[leftPolygon.first()]->setNext(( *( ++( leftiter = leftPolygon.begin() ) ) ) );
  mHalfEdge[leftPolygon.first()]->setPoint( p2 );
  mHalfEdge[leftPolygon.last()]->setNext( firstedge );
  mHalfEdge[rightPolygon.first()]->setNext(( *( ++( rightiter = rightPolygon.begin() ) ) ) );
  mHalfEdge[rightPolygon.first()]->setPoint( p1 );
  mHalfEdge[rightPolygon.last()]->setNext( dualfirstedge );

  triangulatePolygon( &leftPolygon, &freelist, firstedge );
  triangulatePolygon( &rightPolygon, &freelist, dualfirstedge );

  //optimisation of the new edges
  for ( iter = crossedEdges.begin();iter != crossedEdges.end();++iter )
  {
    checkSwap(( *( iter ) ) );
  }

  return leftPolygon.first();
}

void DualEdgeTriangulation::setForcedCrossBehaviour( Triangulation::forcedCrossBehaviour b )
{
  mForcedCrossBehaviour = b;
}

void DualEdgeTriangulation::setEdgeColor( int r, int g, int b )
{
  mEdgeColor.setRgb( r, g, b );
}

void DualEdgeTriangulation::setForcedEdgeColor( int r, int g, int b )
{
  mForcedEdgeColor.setRgb( r, g, b );
}

void DualEdgeTriangulation::setBreakEdgeColor( int r, int g, int b )
{
  mBreakEdgeColor.setRgb( r, g, b );
}

void DualEdgeTriangulation::setTriangleInterpolator( TriangleInterpolator* interpolator )
{
  mTriangleInterpolator = interpolator;
}

void DualEdgeTriangulation::eliminateHorizontalTriangles()
{
  QgsDebugMsg( "am in eliminateHorizontalTriangles" );
  double minangle = 0;//minimum angle for swapped triangles. If triangles generated by a swap would have a minimum angle (in degrees) below that value, the swap will not be done.

  while ( true )
  {
    bool swaped = false;//flag which allows to exit the loop
    bool* control = new bool[mHalfEdge.count()];//controlarray

    for ( int i = 0;i <= mHalfEdge.count() - 1;i++ )
    {
      control[i] = false;
    }


    for ( int i = 0;i <= mHalfEdge.count() - 1;i++ )
    {
      if ( control[i] == true )//edge has already been examined
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
      el1 = mPointVector[p1]->getZ();
      el2 = mPointVector[p2]->getZ();
      el3 = mPointVector[p3]->getZ();

      if ( el1 == el2 && el2 == el3 )//we found a horizonal triangle
      {
        //swap edges if it is possible, if it would remove the horizontal triangle and if the minimum angle generated by the swap is high enough
        if ( swapPossible(( uint )e1 ) && mPointVector[mHalfEdge[mHalfEdge[mHalfEdge[e1]->getDual()]->getNext()]->getPoint()]->getZ() != el1 && swapMinAngle( e1 ) > minangle )
        {
          doOnlySwap(( uint )e1 );
          swaped = true;
        }
        else if ( swapPossible(( uint )e2 ) && mPointVector[mHalfEdge[mHalfEdge[mHalfEdge[e2]->getDual()]->getNext()]->getPoint()]->getZ() != el2 && swapMinAngle( e2 ) > minangle )
        {
          doOnlySwap(( uint )e2 );
          swaped = true;
        }
        else if ( swapPossible(( uint )e3 ) && mPointVector[mHalfEdge[mHalfEdge[mHalfEdge[e3]->getDual()]->getNext()]->getPoint()]->getZ() != el3 && swapMinAngle( e3 ) > minangle )
        {
          doOnlySwap(( uint )e3 );
          swaped = true;
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
    if ( swaped == false )
    {
      delete[] control;
      break;
    }
    delete[] control;
  }

  QgsDebugMsg( "end of method" );
}

void DualEdgeTriangulation::ruppertRefinement()
{
  //minimum angle
  double mintol = 17;//refinement stops after the minimum angle reached this tolerance

  //data structures
  std::map<int, double> edge_angle;//search tree with the edge number as key
  std::multimap<double, int> angle_edge;//multimap (map with not unique keys) with angle as key
  std::set<int> dontexamine;//search tree containing the edges which do not have to be examined (because of numerical problems)


  //first, go through all the forced edges and subdivide if they are encroached by a point
  bool stop = false;//flag to ensure that the for-loop is repeated until no half edge is splitted any more

  while ( stop == false )
  {
    stop = true;
    int nhalfedges = mHalfEdge.count();

    for ( int i = 0;i < nhalfedges - 1;i++ )
    {
      int next = mHalfEdge[i]->getNext();
      int nextnext = mHalfEdge[next]->getNext();

      if ( mHalfEdge[next]->getPoint() != -1 && ( mHalfEdge[i]->getForced() || mHalfEdge[mHalfEdge[mHalfEdge[i]->getDual()]->getNext()]->getPoint() == -1 ) )//check for encroached points on forced segments and segments on the inner side of the convex hull, but don't consider edges on the outer side of the convex hull
      {
        if ( !(( mHalfEdge[next]->getForced() || edgeOnConvexHull( next ) ) || ( mHalfEdge[nextnext]->getForced() || edgeOnConvexHull( nextnext ) ) ) )//don't consider triangles where all three edges are forced edges or hull edges
        {
          //test for encroachment
          while ( MathUtils::inDiametral( mPointVector[mHalfEdge[mHalfEdge[i]->getDual()]->getPoint()], mPointVector[mHalfEdge[i]->getPoint()], mPointVector[mHalfEdge[next]->getPoint()] ) )
          {
            //split segment
            int pointno = splitHalfEdge( i, 0.5 );
            Q_UNUSED( pointno );
            stop = false;
          }
        }
      }
    }
  }

  //examine the triangulation for angles below the minimum and insert the edges into angle_edge and edge_angle, except the small angle is between forced segments or convex hull edges
  double angle;//angle between edge i and the consecutive edge
  int p1, p2, p3;//numbers of the triangle points
  for ( int i = 0;i < mHalfEdge.count() - 1;i++ )
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


    if (( mHalfEdge[i]->getForced() == true || edgeOnConvexHull( i ) ) && ( mHalfEdge[mHalfEdge[i]->getNext()]->getForced() == true || edgeOnConvexHull( mHalfEdge[i]->getNext() ) ) )
    {
      twoforcededges = true;
    }
    else
    {
      twoforcededges = false;
    }

    if ( angle < mintol && !twoforcededges )
    {
      edge_angle.insert( std::make_pair( i, angle ) );
      angle_edge.insert( std::make_pair( angle, i ) );
    }
  }

  //debugging: print out all the angles below the minimum for a test
  for ( std::multimap<double, int>::const_iterator it = angle_edge.begin();it != angle_edge.end();++it )
  {
    QgsDebugMsg( QString( "angle: %1" ).arg( it->first ) );
  }


  double minangle = 0;//actual minimum angle
  int minedge;//first edge adjacent to the minimum angle
  int minedgenext;
  int minedgenextnext;

  Point3D circumcenter;

  while ( !edge_angle.empty() )
  {
    minangle = angle_edge.begin()->first;
    QgsDebugMsg( QString( "minangle: %1" ).arg( minangle ) );
    minedge = angle_edge.begin()->second;
    minedgenext = mHalfEdge[minedge]->getNext();
    minedgenextnext = mHalfEdge[minedgenext]->getNext();

    //calculate the circumcenter
    if ( !MathUtils::circumcenter( mPointVector[mHalfEdge[minedge]->getPoint()], mPointVector[mHalfEdge[minedgenext]->getPoint()], mPointVector[mHalfEdge[minedgenextnext]->getPoint()], &circumcenter ) )
    {
      QgsDebugMsg( "warning, calculation of circumcenter failed" );
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

    if ( !pointInside( circumcenter.getX(), circumcenter.getY() ) )
    {
      //put all three edges to dontexamine and remove them from the other maps
      QgsDebugMsg( QString( "put circumcenter %1//%2 on dontexamine list because it is outside the convex hull" )
                   .arg( circumcenter.getX() ).arg( circumcenter.getY() ) );
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
    for ( int i = 0;i < numhalfedges;i++ )
    {
      if ( mHalfEdge[i]->getForced() || edgeOnConvexHull( i ) )
      {
        if ( MathUtils::inDiametral( mPointVector[mHalfEdge[i]->getPoint()], mPointVector[mHalfEdge[mHalfEdge[i]->getDual()]->getPoint()], &circumcenter ) )
        {
          encroached = true;
          //split segment
          QgsDebugMsg( "segment split" );
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

            if (( mHalfEdge[ed1]->getForced() == true || edgeOnConvexHull( ed1 ) ) && ( mHalfEdge[ed2]->getForced() == true || edgeOnConvexHull( ed2 ) ) )
            {
              twoforcededges1 = true;
            }
            else
            {
              twoforcededges1 = false;
            }

            if (( mHalfEdge[ed2]->getForced() == true || edgeOnConvexHull( ed2 ) ) && ( mHalfEdge[ed3]->getForced() == true || edgeOnConvexHull( ed3 ) ) )
            {
              twoforcededges2 = true;
            }
            else
            {
              twoforcededges2 = false;
            }

            if (( mHalfEdge[ed3]->getForced() == true || edgeOnConvexHull( ed3 ) ) && ( mHalfEdge[ed1]->getForced() == true || edgeOnConvexHull( ed1 ) ) )
            {
              twoforcededges3 = true;
            }
            else
            {
              twoforcededges3 = false;
            }

            //update the settings related to ed1
            std::set<int>::iterator ed1iter = dontexamine.find( ed1 );
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
            std::set<int>::iterator ed2iter = dontexamine.find( ed2 );
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
            std::set<int>::iterator ed3iter = dontexamine.find( ed3 );
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
    std::set<int> influenceedges;//begin fast method
    int baseedge = baseEdgeOfTriangle( &circumcenter );
    if ( baseedge == -5 )//a numerical instability occured or the circumcenter already exists in the triangulation
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

    evaluateInfluenceRegion( &circumcenter, baseedge, &influenceedges );
    evaluateInfluenceRegion( &circumcenter, mHalfEdge[baseedge]->getNext(), &influenceedges );
    evaluateInfluenceRegion( &circumcenter, mHalfEdge[mHalfEdge[baseedge]->getNext()]->getNext(), &influenceedges );

    for ( std::set<int>::iterator it = influenceedges.begin();it != influenceedges.end();++it )
    {
      if (( mHalfEdge[*it]->getForced() == true || edgeOnConvexHull( *it ) ) && MathUtils::inDiametral( mPointVector[mHalfEdge[*it]->getPoint()], mPointVector[mHalfEdge[mHalfEdge[*it]->getDual()]->getPoint()], &circumcenter ) )
      {
        //split segment
        QgsDebugMsg( "segment split" );
        int pointno = splitHalfEdge( *it, 0.5 );
        encroached = true;

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
          bool twoforcededges1, twoforcededges2, twoforcededges3;//flag to decide, if edges should be added to the maps. Do not add them if true



          if (( mHalfEdge[ed1]->getForced() == true || edgeOnConvexHull( ed1 ) ) && ( mHalfEdge[ed2]->getForced() == true || edgeOnConvexHull( ed2 ) ) )
          {
            twoforcededges1 = true;
          }
          else
          {
            twoforcededges1 = false;
          }

          if (( mHalfEdge[ed2]->getForced() == true || edgeOnConvexHull( ed2 ) ) && ( mHalfEdge[ed3]->getForced() == true || edgeOnConvexHull( ed3 ) ) )
          {
            twoforcededges2 = true;
          }
          else
          {
            twoforcededges2 = false;
          }

          if (( mHalfEdge[ed3]->getForced() == true || edgeOnConvexHull( ed3 ) ) && ( mHalfEdge[ed1]->getForced() == true || edgeOnConvexHull( ed1 ) ) )
          {
            twoforcededges3 = true;
          }
          else
          {
            twoforcededges3 = false;
          }


          //update the settings related to ed1
          std::set<int>::iterator ed1iter = dontexamine.find( ed1 );
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
          std::set<int>::iterator ed2iter = dontexamine.find( ed2 );
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
          std::set<int>::iterator ed3iter = dontexamine.find( ed3 );
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
    } //end fast method


    if ( encroached == true )
    {
      continue;
    }

    /*******otherwise, try to add the circumcenter to the triangulation************************************************************************************************/

    Point3D* p = new Point3D();
    mDecorator->calcPoint( circumcenter.getX(), circumcenter.getY(), p );
    int pointno = mDecorator->addPoint( p );

    if ( pointno == -100 || pointno == mTwiceInsPoint )
    {
      if ( pointno == -100 )
      {
        QgsDebugMsg( QString( "put circumcenter %1//%2 on dontexamine list because of numerical instabilities" )
                     .arg( circumcenter.getX() ).arg( circumcenter.getY() ) );
      }
      else if ( pointno == mTwiceInsPoint )
      {
        QgsDebugMsg( QString( "put circumcenter %1//%2 on dontexamine list because it is already inserted" )
                     .arg( circumcenter.getX() ).arg( circumcenter.getY() ) );
        //test, if the point is present in the triangulation
        bool flag = false;
        for ( int i = 0;i < mPointVector.count();i++ )
        {
          if ( mPointVector[i]->getX() == circumcenter.getX() && mPointVector[i]->getY() == circumcenter.getY() )
          {
            flag = true;
          }
        }
        if ( flag == false )
        {
          QgsDebugMsg( "point is not present in the triangulation" );
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
      QgsDebugMsg( "circumcenter added" );

      //update the maps
      //go around the inserted point and make changes for every half edge
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

        //todo: put all three edges on the dontexamine list if two edges are forced or convex hull edges
        bool twoforcededges1, twoforcededges2, twoforcededges3;

        if (( mHalfEdge[ed1]->getForced() == true || edgeOnConvexHull( ed1 ) ) && ( mHalfEdge[ed2]->getForced() == true || edgeOnConvexHull( ed2 ) ) )
        {
          twoforcededges1 = true;
        }
        else
        {
          twoforcededges1 = false;
        }

        if (( mHalfEdge[ed2]->getForced() == true || edgeOnConvexHull( ed2 ) ) && ( mHalfEdge[ed3]->getForced() == true || edgeOnConvexHull( ed3 ) ) )
        {
          twoforcededges2 = true;
        }
        else
        {
          twoforcededges2 = false;
        }

        if (( mHalfEdge[ed3]->getForced() == true || edgeOnConvexHull( ed3 ) ) && ( mHalfEdge[ed1]->getForced() == true || edgeOnConvexHull( ed1 ) ) )
        {
          twoforcededges3 = true;
        }
        else
        {
          twoforcededges3 = false;
        }


        //update the settings related to ed1
        std::set<int>::iterator ed1iter = dontexamine.find( ed1 );
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
        std::set<int>::iterator ed2iter = dontexamine.find( ed2 );
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
        std::set<int>::iterator ed3iter = dontexamine.find( ed3 );
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

#if 0
  //debugging: print out all edge of dontexamine
  for ( std::set<int>::iterator it = dontexamine.begin();it != dontexamine.end();++it )
  {
    QgsDebugMsg( QString( "edge nr. %1 is in dontexamine" ).arg( *it ) );
  }
#endif
}


bool DualEdgeTriangulation::swapPossible( unsigned int edge )
{
  //test, if edge belongs to a forced edge
  if ( mHalfEdge[edge]->getForced() == true )
  {
    return false;
  }

  //test, if the edge is on the convex hull or is connected to the virtual point
  if ( mHalfEdge[edge]->getPoint() == -1 || mHalfEdge[mHalfEdge[edge]->getNext()]->getPoint() == -1 || mHalfEdge[mHalfEdge[mHalfEdge[edge]->getDual()]->getNext()]->getPoint() == -1 || mHalfEdge[mHalfEdge[edge]->getDual()]->getPoint() == -1 )
  {
    return false;
  }
  //then, test, if the edge is in the middle of a not convex quad
  Point3D* pta = mPointVector[mHalfEdge[edge]->getPoint()];
  Point3D* ptb = mPointVector[mHalfEdge[mHalfEdge[edge]->getNext()]->getPoint()];
  Point3D* ptc = mPointVector[mHalfEdge[mHalfEdge[mHalfEdge[edge]->getNext()]->getNext()]->getPoint()];
  Point3D* ptd = mPointVector[mHalfEdge[mHalfEdge[mHalfEdge[edge]->getDual()]->getNext()]->getPoint()];
  if ( MathUtils::leftOf( ptc, pta, ptb ) > leftOfTresh )
  {
    return false;
  }
  else if ( MathUtils::leftOf( ptd, ptb, ptc ) > leftOfTresh )
  {
    return false;
  }
  else if ( MathUtils::leftOf( pta, ptc, ptd ) > leftOfTresh )
  {
    return false;
  }
  else if ( MathUtils::leftOf( ptb, ptd, pta ) > leftOfTresh )
  {
    return false;
  }
  return true;
}

void DualEdgeTriangulation::triangulatePolygon( QList<int>* poly, QList<int>* free, int mainedge )
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
      int inserta = free->first();//take an edge from the freelist
      int insertb = mHalfEdge[inserta]->getDual();
      free->pop_front();

      mHalfEdge[inserta]->setNext(( poly->at( 1 ) ) );
      mHalfEdge[inserta]->setPoint( mHalfEdge[mainedge]->getPoint() );
      mHalfEdge[insertb]->setNext( nextdistedge );
      mHalfEdge[insertb]->setPoint( mHalfEdge[distedge]->getPoint() );
      mHalfEdge[distedge]->setNext( inserta );
      mHalfEdge[mainedge]->setNext( insertb );

      QList<int> polya;
      for ( iterator = ( ++( poly->constBegin() ) );( *iterator ) != nextdistedge;++iterator )
      {
        polya.append(( *iterator ) );
      }
      polya.prepend( inserta );

#if 0
      //print out all the elements of polya for a test
      for ( iterator = polya.begin();iterator != polya.end();++iterator )
      {
        QgsDebugMsg( *iterator );
      }
#endif

      triangulatePolygon( &polya, free, inserta );
    }

    else if ( distedge == ( *( ++poly->begin() ) ) )//the nearest point is connected to the beginpoint of mainedge
    {
      int inserta = free->first();//take an edge from the freelist
      int insertb = mHalfEdge[inserta]->getDual();
      free->pop_front();

      mHalfEdge[inserta]->setNext(( poly->at( 2 ) ) );
      mHalfEdge[inserta]->setPoint( mHalfEdge[distedge]->getPoint() );
      mHalfEdge[insertb]->setNext( mainedge );
      mHalfEdge[insertb]->setPoint( mHalfEdge[mHalfEdge[mainedge]->getDual()]->getPoint() );
      mHalfEdge[distedge]->setNext( insertb );
      mHalfEdge[( *( --poly->end() ) )]->setNext( inserta );

      QList<int> polya;
      iterator = poly->constBegin(); iterator += 2;
      while ( iterator != poly->constEnd() )
      {
        polya.append(( *iterator ) );
        ++iterator;
      }
      polya.prepend( inserta );

      triangulatePolygon( &polya, free, inserta );
    }

    else//the nearest point is not connected to an endpoint of mainedge
    {
      int inserta = free->first();//take an edge from the freelist
      int insertb = mHalfEdge[inserta]->getDual();
      free->pop_front();

      int insertc = free->first();
      int insertd = mHalfEdge[insertc]->getDual();
      free->pop_front();

      mHalfEdge[inserta]->setNext(( poly->at( 1 ) ) );
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

      for ( iterator = ++( poly->constBegin() );( *iterator ) != nextdistedge;++iterator )
      {
        polya.append(( *iterator ) );
      }
      polya.prepend( inserta );


      while ( iterator != poly->constEnd() )
      {
        polyb.append(( *iterator ) );
        ++iterator;
      }
      polyb.prepend( insertc );

      triangulatePolygon( &polya, free, inserta );
      triangulatePolygon( &polyb, free, insertc );
    }
  }

  else
  {
    QgsDebugMsg( "warning, null pointer" );
  }

}

bool DualEdgeTriangulation::pointInside( double x, double y )
{
  Point3D point( x, y, 0 );
  unsigned int actedge = mEdgeInside;//start with an edge which does not point to the virtual point
  int counter = 0;//number of consecutive successful left-of-tests
  int nulls = 0;//number of left-of-tests, which returned 0. 1 means, that the point is on a line, 2 means that it is on an existing point
  int numinstabs = 0;//number of suspect left-of-tests due to 'leftOfTresh'
  int runs = 0;//counter for the number of iterations in the loop to prevent an endless loop

  while ( true )
  {
    if ( runs > nBaseOfRuns )//prevents endless loops
    {
      QgsDebugMsg( QString( "warning, instability detected: Point coordinates: %1//%2" ).arg( x ).arg( y ) );
      return false;
    }

    if ( MathUtils::leftOf( &point, mPointVector[mHalfEdge[mHalfEdge[actedge]->getDual()]->getPoint()], mPointVector[mHalfEdge[actedge]->getPoint()] ) < ( -leftOfTresh ) )//point is on the left side
    {
      counter += 1;
      if ( counter == 3 )//three successful passes means that we have found the triangle
      {
        break;
      }
    }

    else if ( MathUtils::leftOf( &point, mPointVector[mHalfEdge[mHalfEdge[actedge]->getDual()]->getPoint()], mPointVector[mHalfEdge[actedge]->getPoint()] ) == 0 )//point is exactly in the line of the edge
    {
      counter += 1;
      mEdgeWithPoint = actedge;
      nulls += 1;
      if ( counter == 3 )//three successful passes means that we have found the triangle
      {
        break;
      }
    }
    else if ( MathUtils::leftOf( &point, mPointVector[mHalfEdge[mHalfEdge[actedge]->getDual()]->getPoint()], mPointVector[mHalfEdge[actedge]->getPoint()] ) < leftOfTresh )//numerical problems
    {
      counter += 1;
      numinstabs += 1;
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
  if ( numinstabs > 0 )//a numerical instability occured
  {
    QgsDebugMsg( "numerical instabilities" );
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
  QProgressBar* edgebar = new QProgressBar();//use a progress dialog so that it is not boring for the user
  edgebar->setCaption( "Reading edges..." );
  edgebar->setTotalSteps( numberofhalfedges / 2 );
  edgebar->setMinimumWidth( 400 );
  edgebar->move( 500, 500 );
  edgebar->show();

  for ( int i = 0;i < numberofhalfedges / 2;i++ )
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

    HalfEdge* hf1 = new HalfEdge();
    hf1->setDual( nr2 );
    hf1->setNext( next1 );
    hf1->setPoint( point1 );
    hf1->setBreak( break1 );
    hf1->setForced( forced1 );

    HalfEdge* hf2 = new HalfEdge();
    hf2->setDual( nr1 );
    hf2->setNext( next2 );
    hf2->setPoint( point2 );
    hf2->setBreak( break2 );
    hf2->setForced( forced2 );

    // QgsDebugMsg( QString( "inserting half edge pair %1" ).arg( i ) );
    mHalfEdge.insert( nr1, hf1 );
    mHalfEdge.insert( nr2, hf2 );

  }

  edgebar->setProgress( numberofhalfedges / 2 );
  delete edgebar;

  //set mEdgeInside to a reasonable value
  for ( int i = 0;i < numberofhalfedges;i++ )
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

  QProgressBar* pointbar = new QProgressBar();
  pointbar->setCaption( "Reading points..." );
  pointbar->setTotalSteps( numberofpoints );
  pointbar->setMinimumWidth( 400 );
  pointbar->move( 500, 500 );
  pointbar->show();


  double x, y, z;
  for ( int i = 0;i < numberofpoints;i++ )
  {
    if ( i % 1000 == 0 )
    {
      pointbar->setProgress( i );
    }

    textstream >> x;
    textstream >> y;
    textstream >> z;

    Point3D* p = new Point3D( x, y, z );

    // QgsDebugMsg( QString( "inserting point %1" ).arg( i ) );
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
    QMessageBox::warning( 0, "warning", "File could not be written", QMessageBox::Ok, QMessageBox::NoButton, QMessageBox::NoButton );
    return false;
  }

  QTextStream outstream( &outputfile );
  outstream.precision( 9 );

  //export the edges. Attention, dual edges must be adjacent in the TAFF-file
  outstream << "TRIA" << std::endl << std::flush;
  outstream << "NEDG " << mHalfEdge.count() << std::endl << std::flush;
  outstream << "PANO 1" << std::endl << std::flush;
  outstream << "DATA ";

  bool* cont = new bool[mHalfEdge.count()];
  for ( unsigned int i = 0;i <= mHalfEdge.count() - 1;i++ )
  {
    cont[i] = false;
  }

  for ( unsigned int i = 0;i < mHalfEdge.count();i++ )
  {
    if ( cont[i] == true )
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
  outstream << "NPTS " << this->getNumberOfPoints() << std::endl << std::flush;
  outstream << "PATT 3" << std::endl << std::flush;
  outstream << "DATA ";

  for ( int i = 0;i < this->getNumberOfPoints();i++ )
  {
    Point3D* p = mPointVector[i];
    outstream << p->getX() << " " << p->getY() << " " << p->getZ() << " ";
  }
  outstream << std::endl << std::flush;
  outstream << std::endl << std::flush;

  return true;
}
#endif //0

bool DualEdgeTriangulation::swapEdge( double x, double y )
{
  Point3D p( x, y, 0 );
  int edge1 = baseEdgeOfTriangle( &p );
  if ( edge1 >= 0 )
  {
    int edge2, edge3;
    Point3D* point1;
    Point3D* point2;
    Point3D* point3;
    edge2 = mHalfEdge[edge1]->getNext();
    edge3 = mHalfEdge[edge2]->getNext();
    point1 = getPoint( mHalfEdge[edge1]->getPoint() );
    point2 = getPoint( mHalfEdge[edge2]->getPoint() );
    point3 = getPoint( mHalfEdge[edge3]->getPoint() );
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

QList<int>* DualEdgeTriangulation::getPointsAroundEdge( double x, double y )
{
  Point3D p( x, y, 0 );
  int p1, p2, p3, p4;
  int edge1 = baseEdgeOfTriangle( &p );
  if ( edge1 >= 0 )
  {
    int edge2, edge3;
    Point3D* point1;
    Point3D* point2;
    Point3D* point3;
    edge2 = mHalfEdge[edge1]->getNext();
    edge3 = mHalfEdge[edge2]->getNext();
    point1 = getPoint( mHalfEdge[edge1]->getPoint() );
    point2 = getPoint( mHalfEdge[edge2]->getPoint() );
    point3 = getPoint( mHalfEdge[edge3]->getPoint() );
    if ( point1 && point2 && point3 )
    {
      double dist1, dist2, dist3;
      dist1 = MathUtils::distPointFromLine( &p, point3, point1 );
      dist2 = MathUtils::distPointFromLine( &p, point1, point2 );
      dist3 = MathUtils::distPointFromLine( &p, point2, point3 );
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
      else if ( dist3 <= dist1 && dist3 <= dist2 )
      {
        p1 = mHalfEdge[edge3]->getPoint();
        p2 = mHalfEdge[mHalfEdge[edge3]->getNext()]->getPoint();
        p3 = mHalfEdge[mHalfEdge[edge3]->getDual()]->getPoint();
        p4 = mHalfEdge[mHalfEdge[mHalfEdge[edge3]->getDual()]->getNext()]->getPoint();
      }
      QList<int>* list = new QList<int>();
      list->append( p1 );
      list->append( p2 );
      list->append( p3 );
      list->append( p4 );
      return list;
    }
    else
    {
      QgsDebugMsg( "warning: null pointer" );
      return 0;
    }
  }
  else
  {
    QgsDebugMsg( "Edge number negative" );
    return 0;
  }
}

bool DualEdgeTriangulation::saveAsShapefile( const QString& fileName ) const
{
  QString shapeFileName = fileName;

  QgsFieldMap fields;
  fields.insert( 0, QgsField( "type", QVariant::String, "String" ) );

  // add the extension if not present
  if ( shapeFileName.indexOf( ".shp" ) == -1 )
  {
    shapeFileName += ".shp";
  }

  //delete already existing files
  if ( QFile::exists( shapeFileName ) )
  {
    if ( !QgsVectorFileWriter::deleteShapeFile( shapeFileName ) )
    {
      return false;
    }
  }

  QgsVectorFileWriter writer( shapeFileName, "Utf-8", fields, QGis::WKBLineString, 0 );
  if ( writer.hasError() != QgsVectorFileWriter::NoError )
  {
    return false;
  }

  bool *alreadyVisitedEdges = new bool[mHalfEdge.size()];
  if ( !alreadyVisitedEdges )
  {
    QgsDebugMsg( "out of memory" );
    return false;
  }

  for ( int i = 0; i < mHalfEdge.size(); ++i )
  {
    alreadyVisitedEdges[i] = false;
  }

  for ( int i = 0; i < mHalfEdge.size(); ++i )
  {
    HalfEdge* currentEdge = mHalfEdge[i];
    if ( currentEdge->getPoint() != -1 && mHalfEdge[currentEdge->getDual()]->getPoint() != -1 && !alreadyVisitedEdges[currentEdge->getDual()] )
    {
      QgsFeature edgeLineFeature;

      //geometry
      Point3D* p1 = mPointVector[currentEdge->getPoint()];
      Point3D* p2 = mPointVector[mHalfEdge[currentEdge->getDual()]->getPoint()];
      QgsPolyline lineGeom;
      lineGeom.push_back( QgsPoint( p1->getX(), p1->getY() ) );
      lineGeom.push_back( QgsPoint( p2->getX(), p2->getY() ) );
      QgsGeometry* geom = QgsGeometry::fromPolyline( lineGeom );
      edgeLineFeature.setGeometry( geom );

      //attributes
      QString attributeString;
      if ( currentEdge->getForced() )
      {
        if ( currentEdge->getBreak() )
        {
          attributeString = "break line";
        }
        else
        {
          attributeString = "structure line";
        }
      }
      edgeLineFeature.addAttribute( 0, attributeString );

      writer.addFeature( edgeLineFeature );
    }
    alreadyVisitedEdges[i] = true;
  }

  delete [] alreadyVisitedEdges;

  return true;
}

double DualEdgeTriangulation::swapMinAngle( int edge ) const
{
  Point3D* p1 = getPoint( mHalfEdge[edge]->getPoint() );
  Point3D* p2 = getPoint( mHalfEdge[mHalfEdge[edge]->getNext()]->getPoint() );
  Point3D* p3 = getPoint( mHalfEdge[mHalfEdge[edge]->getDual()]->getPoint() );
  Point3D* p4 = getPoint( mHalfEdge[mHalfEdge[mHalfEdge[edge]->getDual()]->getNext()]->getPoint() );

  //search for the minimum angle (it is important, which directions the lines have!)
  double minangle;
  double angle1 = MathUtils::angle( p1, p2, p4, p2 );
  minangle = angle1;
  double angle2 = MathUtils::angle( p3, p2, p4, p2 );
  if ( angle2 < minangle )
  {
    minangle = angle2;
  }
  double angle3 = MathUtils::angle( p2, p3, p4, p3 );
  if ( angle3 < minangle )
  {
    minangle = angle3;
  }
  double angle4 = MathUtils::angle( p3, p4, p2, p4 );
  if ( angle4 < minangle )
  {
    minangle = angle4;
  }
  double angle5 = MathUtils::angle( p2, p4, p1, p4 );
  if ( angle5 < minangle )
  {
    minangle = angle5;
  }
  double angle6 = MathUtils::angle( p4, p1, p2, p1 );
  if ( angle6 < minangle )
  {
    minangle = angle6;
  }

  return minangle;
}

int DualEdgeTriangulation::splitHalfEdge( int edge, float position )
{
  //just a short test if position is between 0 and 1
  if ( position < 0 || position > 1 )
  {
    QgsDebugMsg( "warning, position is not between 0 and 1" );
  }

  //create the new point on the heap
  Point3D* p = new Point3D( mPointVector[mHalfEdge[edge]->getPoint()]->getX()*position + mPointVector[mHalfEdge[mHalfEdge[edge]->getDual()]->getPoint()]->getX()*( 1 - position ), mPointVector[mHalfEdge[edge]->getPoint()]->getY()*position + mPointVector[mHalfEdge[mHalfEdge[edge]->getDual()]->getPoint()]->getY()*( 1 - position ), 0 );

  //calculate the z-value of the point to insert
  Point3D zvaluepoint;
  mDecorator->calcPoint( p->getX(), p->getY(), &zvaluepoint );
  p->setZ( zvaluepoint.getZ() );

  //insert p into mPointVector
  if ( mPointVector.count() >= mPointVector.size() )
  {
    mPointVector.resize( mPointVector.count() + 1 );
  }
  QgsDebugMsg( QString( "inserting point nr. %1, %2//%3//%4" ).arg( mPointVector.count() ).arg( p->getX() ).arg( p->getY() ).arg( p->getZ() ) );
  mPointVector.insert( mPointVector.count(), p );

  //insert the six new halfedges
  int dualedge = mHalfEdge[edge]->getDual();
  int edge1 = insertEdge( -10, -10, mPointVector.count() - 1, false, false );
  int edge2 = insertEdge( edge1, mHalfEdge[mHalfEdge[edge]->getNext()]->getNext(), mHalfEdge[mHalfEdge[edge]->getNext()]->getPoint(), false, false );
  int edge3 = insertEdge( -10, mHalfEdge[mHalfEdge[dualedge]->getNext()]->getNext(), mHalfEdge[mHalfEdge[dualedge]->getNext()]->getPoint(), false, false );
  int edge4 = insertEdge( edge3, dualedge, mPointVector.count() - 1, false, false );
  int edge5 = insertEdge( -10, mHalfEdge[edge]->getNext(), mHalfEdge[edge]->getPoint(), mHalfEdge[edge]->getBreak(), mHalfEdge[edge]->getForced() );
  int edge6 = insertEdge( edge5, edge3, mPointVector.count() - 1, mHalfEdge[dualedge]->getBreak(), mHalfEdge[dualedge]->getForced() );
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

  //test four times recursively for swaping
  checkSwap( mHalfEdge[edge5]->getNext() );
  checkSwap( mHalfEdge[edge2]->getNext() );
  checkSwap( mHalfEdge[dualedge]->getNext() );
  checkSwap( mHalfEdge[edge3]->getNext() );

  mDecorator->addPoint( new Point3D( p->getX(), p->getY(), 0 ) );//dirty hack to enforce update of decorators

  return mPointVector.count() - 1;
}

bool DualEdgeTriangulation::edgeOnConvexHull( int edge )
{
  return ( mHalfEdge[mHalfEdge[edge]->getNext()]->getPoint() == -1 || mHalfEdge[mHalfEdge[mHalfEdge[edge]->getDual()]->getNext()]->getPoint() == -1 );
}

void DualEdgeTriangulation::evaluateInfluenceRegion( Point3D* point, int edge, std::set<int>* set )
{
  if ( set->find( edge ) == set->end() )
  {
    set->insert( edge );
  }
  else//prevent endless loops
  {
    return;
  }

  if ( mHalfEdge[edge]->getForced() == false && !edgeOnConvexHull( edge ) )
  {
    //test, if point is in the circle through both endpoints of edge and the endpoint of edge->dual->next->point
    if ( MathUtils::inCircle( point, mPointVector[mHalfEdge[mHalfEdge[edge]->getDual()]->getPoint()], mPointVector[mHalfEdge[edge]->getPoint()], mPointVector[mHalfEdge[mHalfEdge[edge]->getNext()]->getPoint()] ) )
    {
      evaluateInfluenceRegion( point, mHalfEdge[mHalfEdge[edge]->getDual()]->getNext(), set );
      evaluateInfluenceRegion( point, mHalfEdge[mHalfEdge[mHalfEdge[edge]->getDual()]->getNext()]->getNext(), set );
    }
  }
}
