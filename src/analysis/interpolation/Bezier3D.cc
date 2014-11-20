/***************************************************************************
                          Bezier3D.cc  -  description
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

#include "Bezier3D.h"
#include "qgslogger.h"


void Bezier3D::calcFirstDer( float t, Vector3D* v )
{
  if ( v && mControlPoly )
  {
    v->setX( 0 );
    v->setY( 0 );
    v->setZ( 0 );

    if ( mControlPoly->count() < 2 )
    {
      return;
    }

    for ( int n = 1; n <= int( mControlPoly->count() - 1 ); n++ )
    {
      double bernst = MathUtils::calcBernsteinPoly( mControlPoly->count() - 2, n - 1, t );
      v->setX( v->getX() + (( *mControlPoly )[n]->getX() - ( *mControlPoly )[n-1]->getX() )*bernst );
      v->setY( v->getY() + (( *mControlPoly )[n]->getY() - ( *mControlPoly )[n-1]->getY() )*bernst );
      v->setZ( v->getZ() + (( *mControlPoly )[n]->getZ() - ( *mControlPoly )[n-1]->getZ() )*bernst );
    }
    v->setX( v->getX()*( mControlPoly->count() - 1 ) );
    v->setY( v->getY()*( mControlPoly->count() - 1 ) );
    v->setZ( v->getZ()*( mControlPoly->count() - 1 ) );
  }

  else
  {
    QgsDebugMsg( "warning: null pointer" );
  }
}

void Bezier3D::calcPoint( float t, Point3D* p )
{

  if ( p && mControlPoly )
  {
    p->setX( 0 );
    p->setY( 0 );
    p->setZ( 0 );

    for ( int n = 1; n <= int( mControlPoly->count() ); n++ )
    {
      double bernst = MathUtils::calcBernsteinPoly( mControlPoly->count() - 1, n - 1, t );
      p->setX( p->getX() + ( *mControlPoly )[n-1]->getX()*bernst );
      p->setY( p->getY() + ( *mControlPoly )[n-1]->getY()*bernst );
      p->setZ( p->getZ() + ( *mControlPoly )[n-1]->getZ()*bernst );
    }
  }

  else
  {
    QgsDebugMsg( "warning: null pointer" );
  }
}

void Bezier3D::calcSecDer( float t, Vector3D* v )
{
  if ( v && mControlPoly )
  {
    v->setX( 0 );
    v->setY( 0 );
    v->setZ( 0 );

    if ( mControlPoly->count() < 3 )
    {
      return;
    }

    for ( int n = 1; n <= int( mControlPoly->count() - 2 ); n++ )
    {
      double bernst = MathUtils::calcBernsteinPoly( mControlPoly->count() - 3, n - 1, t );
      v->setX( v->getX() + (( *mControlPoly )[n+1]->getX() - 2*( *mControlPoly )[n]->getX() + ( *mControlPoly )[n-1]->getX() )*bernst );
      v->setY( v->getY() + (( *mControlPoly )[n+1]->getY() - 2*( *mControlPoly )[n]->getY() + ( *mControlPoly )[n-1]->getY() )*bernst );
      v->setZ( v->getZ() + (( *mControlPoly )[n+1]->getZ() - 2*( *mControlPoly )[n]->getZ() + ( *mControlPoly )[n-1]->getZ() )*bernst );
    }
    v->setX( v->getX()*MathUtils::faculty( mControlPoly->count() - 1 ) / MathUtils::faculty( mControlPoly->count() - 3 ) );
    v->setY( v->getY()*MathUtils::faculty( mControlPoly->count() - 1 ) / MathUtils::faculty( mControlPoly->count() - 3 ) );
    v->setZ( v->getZ()*MathUtils::faculty( mControlPoly->count() - 1 ) / MathUtils::faculty( mControlPoly->count() - 3 ) );
  }

  else
  {
    QgsDebugMsg( "warning: null pointer" );
  }
}


void Bezier3D::changeDirection()//does this work correctly? more testing is needed.
{
  if ( mControlPoly )
  {
    Point3D** pointer = new Point3D*[mControlPoly->count()];//create an array to temporarily store pointer to the control points
    for ( int i = 0; i < mControlPoly->count(); i++ )//store the points
    {
      pointer[i] = ( *mControlPoly )[i];
    }

    for ( int i = 0; i < mControlPoly->count(); i++ )
    {
      mControlPoly->insert( i, pointer[( mControlPoly->count()-1 )-i] );
    }
    delete [] pointer;
  }

  else
  {
    QgsDebugMsg( "warning: null pointer" );
  }
}
































