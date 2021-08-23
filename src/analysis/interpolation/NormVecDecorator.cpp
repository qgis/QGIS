/***************************************************************************
                             NormVecDecorator.cpp
                             --------------------
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

#include <QApplication>

#include "qgsfeedback.h"
#include "qgslogger.h"
#include "qgsfields.h"
#include "qgspoint.h"

#include "NormVecDecorator.h"


NormVecDecorator::~NormVecDecorator()
{
  //remove all the normals
  if ( !mNormVec->isEmpty() )
  {
    for ( int i = 0; i < mNormVec->count(); i++ )
    {
      delete ( *mNormVec )[i];
    }
  }

  delete mNormVec;
  delete mPointState;
  delete mTIN;
}

int NormVecDecorator::addPoint( const QgsPoint &p )
{
  if ( mTIN )
  {
    int pointno;
    pointno = mTIN->addPoint( p );

    if ( pointno == -100 )//a numerical error occurred
    {
// QgsDebugMsg("warning, numerical error");
      return -100;
    }

    //recalculate the necessary normals
    if ( alreadyestimated )
    {
      estimateFirstDerivative( pointno );
      //update also the neighbours of the new point
      const QList<int> list = mTIN->surroundingTriangles( pointno );
      auto it = list.constBegin();//iterate through the list and analyze it
      while ( it != list.constEnd() )
      {
        int point;
        point = *it;
        if ( point != -1 )
        {
          estimateFirstDerivative( point );
        }
        ++it;
        ++it;
        ++it;
        ++it;
      }
    }
    return pointno;
  }

  return -1;
}

bool NormVecDecorator::calcNormal( double x, double y, QgsPoint &result )
{
  if ( !alreadyestimated )
  {
    estimateFirstDerivatives();
    alreadyestimated = true;
  }

  if ( mInterpolator )
  {
    const bool b = mInterpolator->calcNormVec( x, y, result );
    return b;
  }
  else
  {
    QgsDebugMsg( QStringLiteral( "warning, null pointer" ) );
    return false;
  }
}

bool NormVecDecorator::calcNormalForPoint( double x, double y, int pointIndex, Vector3D *result )
{
  if ( !alreadyestimated )
  {
    estimateFirstDerivatives();
    alreadyestimated = true;
  }

  if ( result )
  {
    int numberofbreaks = 0;//number of breaklines around the point
    int ffirstbp = -1000; //numbers of the points related to the first breakline
    int lfirstbp = -1000;
    bool pointfound = false;//is set to true, if the triangle with the point in is found
    int numberofruns = 0;//number of runs of the loop. This integer can be used to prevent endless loops
    const int limit = 100000;//ater this number of iterations, the method is terminated

    result->setX( 0 );
    result->setY( 0 );
    result->setZ( 0 );

    const QList<int> vlist = surroundingTriangles( pointIndex );//get the value list
    if ( vlist.empty() )//an error occurred in 'getSurroundingTriangles'
    {
      return false;
    }

    if ( ( ( vlist.count() ) % 4 ) != 0 ) //number of items in vlist has to be a multiple of 4
    {
      QgsDebugMsg( QStringLiteral( "warning, wrong number of items in vlist" ) );
      return false;
    }

    auto it = vlist.constBegin();

    bool firstrun;

    while ( true )//endless loop to analyze vlist
    {
      numberofruns++;
      if ( numberofruns > limit )
      {
        QgsDebugMsg( QStringLiteral( "warning, a probable endless loop is detected" ) );
        return false;
      }

      int p1, p2, p3, line;
      firstrun = false;//flag which tells, if it is the first run with a breakline
      p1 = ( *it );
      ++it;
      p2 = ( *it );
      ++it;
      p3 = ( *it );
      ++it;
      line = ( *it );


      if ( numberofbreaks > 0 )
      {

        if ( p1 != -1 && p2 != -1 && p3 != -1 )
        {
          if ( MathUtils::pointInsideTriangle( x, y, point( p1 ), point( p2 ), point( p3 ) ) )
          {
            pointfound = true;
          }

          Vector3D addvec( 0, 0, 0 );
          MathUtils::normalFromPoints( point( p1 ), point( p2 ), point( p3 ), &addvec );
          result->setX( result->getX() + addvec.getX() );
          result->setY( result->getY() + addvec.getY() );
          result->setZ( result->getZ() + addvec.getZ() );
        }
      }

      if ( line == -10 )//we found a breakline
      {

        if ( numberofbreaks == 0 )//it is the first breakline
        {
          firstrun = true;
          ffirstbp = p2;//set the marks to recognize the breakline later
          lfirstbp = p3;
        }

        if ( p2 == ffirstbp && p3 == lfirstbp && !firstrun )//we are back at the first breakline
        {
          if ( !pointfound )//the point with coordinates x, y was in no triangle
          {
            QgsDebugMsg( QStringLiteral( "warning: point (x,y) was in no triangle" ) );
            return false;
          }
          result->standardise();
          break;
        }

        if ( numberofbreaks > 0 && pointfound )//we found the second break line and the point is between the first and the second
        {
          result->standardise();
          numberofbreaks++;//to make the distinction between endpoints and points on a breakline easier
          break;
        }

        result->setX( 0 );
        result->setY( 0 );
        result->setZ( 0 );
        numberofbreaks++;
      }

      ++it;
      if ( it == vlist.constEnd() )//restart at the beginning of the loop
      {
        it = vlist.constBegin();
      }
    }
    return true;
  }
  else
  {
    QgsDebugMsg( QStringLiteral( "warning, null pointer" ) );
    return false;
  }

}

bool NormVecDecorator::calcPoint( double x, double y, QgsPoint &result )
{

  if ( !alreadyestimated )
  {
    estimateFirstDerivatives();
    alreadyestimated = true;
  }

  if ( mInterpolator )
  {
    const bool b = mInterpolator->calcPoint( x, y, result );
    return b;
  }
  else
  {
    QgsDebugMsg( QStringLiteral( "warning, null pointer" ) );
    return false;
  }
}

bool NormVecDecorator::getTriangle( double x, double y, QgsPoint &p1, Vector3D *v1, QgsPoint &p2, Vector3D *v2, QgsPoint &p3, Vector3D *v3 )
{
  if ( v1 && v2 && v3 )
  {
    int nr1 = 0;
    int nr2 = 0;
    int nr3 = 0;

    if ( TriDecorator::triangleVertices( x, y, p1, nr1, p2, nr2, p3, nr3 ) )//everything alright
    {
      if ( ( *mNormVec )[ nr1 ] && ( *mNormVec )[ nr2 ] && ( *mNormVec )[ nr3 ] )
      {
        v1->setX( ( *mNormVec )[ nr1 ]->getX() );
        v1->setY( ( *mNormVec )[nr1 ]->getY() );
        v1->setZ( ( *mNormVec )[nr1 ]->getZ() );

        v2->setX( ( *mNormVec )[nr2 ]->getX() );
        v2->setY( ( *mNormVec )[nr2 ]->getY() );
        v2->setZ( ( *mNormVec )[nr2 ]->getZ() );

        v3->setX( ( *mNormVec )[nr3 ]->getX() );
        v3->setY( ( *mNormVec )[nr3 ]->getY() );
        v3->setZ( ( *mNormVec )[nr3 ]->getZ() );
      }
      else
      {
        QgsDebugMsg( QStringLiteral( "warning, null pointer" ) );
        return false;
      }
      return true;
    }
    else
    {
      return false;
    }
  }

  else
  {
    QgsDebugMsg( QStringLiteral( "warning, null pointer" ) );
    return false;
  }
}

NormVecDecorator::PointState NormVecDecorator::getState( int pointno ) const
{
  if ( pointno >= 0 )
  {
    return mPointState->at( pointno );
  }
  else
  {
    QgsDebugMsg( QStringLiteral( "warning, number below 0" ) );
    return mPointState->at( 0 );//just to avoid a compiler warning
  }
}


bool NormVecDecorator::getTriangle( double x, double y, QgsPoint &p1, int &ptn1, Vector3D *v1, PointState *state1, QgsPoint &p2, int &ptn2, Vector3D *v2, PointState *state2, QgsPoint &p3, int &ptn3, Vector3D *v3, PointState *state3 )
{
  if ( v1 && v2 && v3 && state1 && state2 && state3 )
  {
    if ( TriDecorator::triangleVertices( x, y, p1, ptn1, p2, ptn2, p3, ptn3 ) )//everything alright
    {
      v1->setX( ( *mNormVec )[( ptn1 )]->getX() );
      v1->setY( ( *mNormVec )[( ptn1 )]->getY() );
      v1->setZ( ( *mNormVec )[( ptn1 )]->getZ() );

      ( *state1 ) = ( *mPointState )[( ptn1 )];

      v2->setX( ( *mNormVec )[( ptn2 )]->getX() );
      v2->setY( ( *mNormVec )[( ptn2 )]->getY() );
      v2->setZ( ( *mNormVec )[( ptn2 )]->getZ() );

      ( *state2 ) = ( *mPointState )[( ptn2 )];

      v3->setX( ( *mNormVec )[( ptn3 )]->getX() );
      v3->setY( ( *mNormVec )[( ptn3 )]->getY() );
      v3->setZ( ( *mNormVec )[( ptn3 )]->getZ() );

      ( *state3 ) = ( *mPointState )[( ptn3 )];

      return true;
    }
    else
    {
      return false;
    }

  }
  else
  {
    QgsDebugMsg( QStringLiteral( "warning, null pointer" ) );
    return false;
  }
}

bool NormVecDecorator::estimateFirstDerivative( int pointno )
{
  if ( pointno == -1 )
  {
    return false;
  }

  Vector3D part;
  Vector3D total;
  total.setX( 0 );
  total.setY( 0 );
  total.setZ( 0 );
  int numberofbreaks = 0;//number of counted breaklines
  double weights = 0;//sum of the weights
  double currentweight = 0;//current weight
  PointState status;

  const QList<int> vlist = surroundingTriangles( pointno );//get the value list

  if ( vlist.empty() )
  {
    //something went wrong in getSurroundingTriangles, set the normal to (0,0,0)
    if ( mNormVec->size() <= mNormVec->count() )//allocate more memory if necessary
    {
      QgsDebugMsg( QStringLiteral( "resizing mNormVec from %1 to %2" ).arg( mNormVec->size() ).arg( mNormVec->size() + 1 ) );
      mNormVec->resize( mNormVec->size() + 1 );
    }

    //todo:resize mNormVec if necessary

    if ( !( ( *mNormVec )[pointno] ) ) //insert a pointer to a Vector3D, if there is none at this position
    {
      Vector3D *vec = new Vector3D( total.getX(), total.getY(), total.getZ() );
      mNormVec->insert( pointno, vec );
    }
    else
    {
      ( *mNormVec )[pointno]->setX( 0 );
      ( *mNormVec )[pointno]->setY( 0 );
      ( *mNormVec )[pointno]->setZ( 0 );
    }
    return false;
  }

  if ( ( vlist.count() % 4 ) != 0 ) //number of items in vlist has to be a multiple of 4
  {
    QgsDebugMsg( QStringLiteral( "warning, wrong number of items in vlist" ) );
    return false;
  }

  auto it = vlist.constBegin();//iterate through the list and analyze it
  while ( it != vlist.constEnd() )
  {
    int p1, p2, p3, flag;
    part.setX( 0 );
    part.setY( 0 );
    part.setZ( 0 );

    currentweight = 0;

    p1 = ( *it );
    ++it;
    p2 = ( *it );
    ++it;
    p3 = ( *it );
    ++it;
    flag = ( *it );

    if ( flag == -10 )//we found a breakline.
    {
      numberofbreaks++;
    }

    if ( p1 != -1 && p2 != -1 && p3 != -1 )//don't calculate normal, if a point is a virtual point
    {
      MathUtils::normalFromPoints( point( p1 ), point( p2 ), point( p3 ), &part );
      const double dist1 = point( p3 )->distance3D( *point( p1 ) );
      const double dist2 = point( p3 )->distance3D( *point( p2 ) );
      //don't add the normal if the triangle is horizontal
      if ( ( point( p1 )->z() != point( p2 )->z() ) || ( point( p1 )->z() != point( p3 )->z() ) )
      {
        currentweight = 1 / ( dist1 * dist1 * dist2 * dist2 );
        total.setX( total.getX() + part.getX()*currentweight );
        total.setY( total.getY() + part.getY()*currentweight );
        total.setZ( total.getZ() + part.getZ()*currentweight );
        weights += currentweight;
      }
    }
    ++it;
  }

  if ( total.getX() == 0 && total.getY() == 0 && total.getZ() == 0 )//we have a point surrounded by horizontal triangles
  {
    total.setZ( 1 );
  }
  else
  {
    total.setX( total.getX() / weights );
    total.setY( total.getY() / weights );
    total.setZ( total.getZ() / weights );
    total.standardise();
  }


  if ( numberofbreaks == 0 )
  {
    status = Normal;
  }
  else if ( numberofbreaks == 1 )
  {
    status = EndPoint;
  }
  else
  {
    status = BreakLine;
  }

  //insert the new calculated vector
  if ( mNormVec->size() <= mNormVec->count() )//allocate more memory if necessary
  {
    mNormVec->resize( mNormVec->size() + 1 );
  }

  if ( !( ( *mNormVec )[pointno] ) ) //insert a pointer to a Vector3D, if there is none at this position
  {
    Vector3D *vec = new Vector3D( total.getX(), total.getY(), total.getZ() );
    mNormVec->insert( pointno, vec );
  }
  else
  {
    ( *mNormVec )[pointno]->setX( total.getX() );
    ( *mNormVec )[pointno]->setY( total.getY() );
    ( *mNormVec )[pointno]->setZ( total.getZ() );
  }

  //insert the new status

  if ( pointno >= mPointState->size() )
  {
    mPointState->resize( mPointState->size() + 1 );
  }

  ( *mPointState )[pointno] = status;

  return true;
}

//weighted method of little
bool NormVecDecorator::estimateFirstDerivatives( QgsFeedback *feedback )
{
  const int numberPoints = pointsCount();
  for ( int i = 0; i < numberPoints; i++ )
  {
    if ( feedback )
    {
      feedback->setProgress( 100.0 * static_cast< double >( i ) / numberPoints );
    }
    estimateFirstDerivative( i );
  }

  return true;
}

void NormVecDecorator::eliminateHorizontalTriangles()
{
  if ( mTIN )
  {
    if ( alreadyestimated )
    {
      mTIN->eliminateHorizontalTriangles();
      estimateFirstDerivatives();
    }
    else
    {
      mTIN->eliminateHorizontalTriangles();
    }
  }
  else
  {
    QgsDebugMsg( QStringLiteral( "warning, null pointer" ) );
  }
}

void NormVecDecorator::setState( int pointno, PointState s )
{
  if ( pointno >= 0 )
  {
    ( *mPointState )[pointno] = s;
  }
  else
  {
    QgsDebugMsg( QStringLiteral( "warning, pointno>0" ) );
  }
}

bool NormVecDecorator::swapEdge( double x, double y )
{
  if ( mTIN )
  {
    bool b = false;
    if ( alreadyestimated )
    {
      const QList<int> list = pointsAroundEdge( x, y );
      if ( !list.empty() )
      {
        b = mTIN->swapEdge( x, y );
        for ( const int i : list )
        {
          estimateFirstDerivative( i );
        }
      }
    }
    else
    {
      b = mTIN->swapEdge( x, y );
    }
    return b;
  }
  else
  {
    QgsDebugMsg( QStringLiteral( "warning, null pointer" ) );
    return false;
  }
}

bool NormVecDecorator::saveTriangulation( QgsFeatureSink *sink, QgsFeedback *feedback ) const
{
  if ( !mTIN )
  {
    return false;
  }
  return mTIN->saveTriangulation( sink, feedback );
}

QgsMesh NormVecDecorator::triangulationToMesh( QgsFeedback *feedback ) const
{
  if ( !mTIN )
  {
    return QgsMesh();
  }
  return mTIN->triangulationToMesh( feedback );
}

