/***************************************************************************
                             Bezier3D.cpp
                             ------------
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
#include "Vector3D.h"
#include "MathUtils.h"

void Bezier3D::calcFirstDer( float t, Vector3D *v )
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
      const double bernst = MathUtils::calcBernsteinPoly( mControlPoly->count() - 2, n - 1, t );
      v->setX( v->getX() + ( ( *mControlPoly )[n]->x() - ( *mControlPoly )[n - 1]->x() )*bernst );
      v->setY( v->getY() + ( ( *mControlPoly )[n]->y() - ( *mControlPoly )[n - 1]->y() )*bernst );
      v->setZ( v->getZ() + ( ( *mControlPoly )[n]->z() - ( *mControlPoly )[n - 1]->z() )*bernst );
    }
    v->setX( v->getX() * ( mControlPoly->count() - 1 ) );
    v->setY( v->getY() * ( mControlPoly->count() - 1 ) );
    v->setZ( v->getZ() * ( mControlPoly->count() - 1 ) );
  }

  else
  {
    QgsDebugMsg( QStringLiteral( "warning: null pointer" ) );
  }
}

void Bezier3D::calcPoint( float t, QgsPoint *p )
{

  if ( p && mControlPoly )
  {
    p->setX( 0 );
    p->setY( 0 );
    p->setZ( 0 );

    for ( int n = 1; n <= int( mControlPoly->count() ); n++ )
    {
      const double bernst = MathUtils::calcBernsteinPoly( mControlPoly->count() - 1, n - 1, t );
      p->setX( p->x() + ( *mControlPoly )[n - 1]->x()*bernst );
      p->setY( p->y() + ( *mControlPoly )[n - 1]->y()*bernst );
      p->setZ( p->z() + ( *mControlPoly )[n - 1]->z()*bernst );
    }
  }

  else
  {
    QgsDebugMsg( QStringLiteral( "warning: null pointer" ) );
  }
}

void Bezier3D::calcSecDer( float t, Vector3D *v )
{
  if ( v && mControlPoly )
  {
    v->setX( 0 );
    v->setY( 0 );
    v->setZ( 0 );

    const int nodes = mControlPoly->count();
    if ( nodes < 3 )
    {
      return;
    }

    for ( int n = 1; n <= int( nodes - 2 ); n++ )
    {
      const double bernst = MathUtils::calcBernsteinPoly( nodes - 3, n - 1, t );
      v->setX( v->getX() + ( ( *mControlPoly )[n + 1]->x() - 2 * ( *mControlPoly )[n]->x() + ( *mControlPoly )[n - 1]->x() )*bernst );
      v->setY( v->getY() + ( ( *mControlPoly )[n + 1]->y() - 2 * ( *mControlPoly )[n]->y() + ( *mControlPoly )[n - 1]->y() )*bernst );
      v->setZ( v->getZ() + ( ( *mControlPoly )[n + 1]->z() - 2 * ( *mControlPoly )[n]->z() + ( *mControlPoly )[n - 1]->z() )*bernst );
    }
    v->setX( v->getX()*MathUtils::faculty( nodes - 1 ) / MathUtils::faculty( nodes - 3 ) );
    v->setY( v->getY()*MathUtils::faculty( nodes - 1 ) / MathUtils::faculty( nodes - 3 ) );
    v->setZ( v->getZ()*MathUtils::faculty( nodes - 1 ) / MathUtils::faculty( nodes - 3 ) );
  }

  else
  {
    QgsDebugMsg( QStringLiteral( "warning: null pointer" ) );
  }
}


void Bezier3D::changeDirection()//does this work correctly? more testing is needed.
{
  if ( mControlPoly )
  {
    QgsPoint **pointer = new QgsPoint*[mControlPoly->count()];//create an array to temporarily store pointer to the control points
    for ( int i = 0; i < mControlPoly->count(); i++ )//store the points
    {
      pointer[i] = ( *mControlPoly )[i];
    }

    for ( int i = 0; i < mControlPoly->count(); i++ )
    {
      mControlPoly->insert( i, pointer[( mControlPoly->count() - 1 ) - i] );
    }
    delete [] pointer;
  }

  else
  {
    QgsDebugMsg( QStringLiteral( "warning: null pointer" ) );
  }
}
































