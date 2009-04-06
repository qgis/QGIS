/***************************************************************************
                         qgsoverlayobject.cpp  -  description
                         ------------------------------
    begin                : January 2007
    copyright            : (C) 2007 by Marco Hugentobler
    email                : marco dot hugentobler at karto dot baug dot ethz dot ch
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsoverlayobject.h"
#include "qgsgeometry.h"

QgsOverlayObject::QgsOverlayObject( int width, int height, double rotation, QgsGeometry* geometry ): mWidth( width ), mHeight( height ), mRotation( rotation ), mGeometry( geometry )
{

}

QgsOverlayObject::~QgsOverlayObject()
{
  delete mGeometry;
}

QgsOverlayObject::QgsOverlayObject( const QgsOverlayObject& other ): mWidth( other.width() ), mHeight( other.height() ), mPositions( other.positions() ), mRotation( other.rotation() )
{
  mGeometry = new QgsGeometry( *( other.geometry() ) );
}

QgsOverlayObject& QgsOverlayObject::operator=( const QgsOverlayObject & other )
{
  mWidth = other.width();
  mHeight = other.height();
  mPositions = other.positions();
  mRotation = other.rotation();
  mGeometry = new QgsGeometry( *( other.geometry() ) );
  return *this;
}

GEOSGeometry* QgsOverlayObject::getGeosGeometry()
{
  if ( !mGeometry )
  {
    return 0;
  }

  return mGeometry->asGeos();
}

void QgsOverlayObject::addPosition( const QgsPoint& position )
{
  mPositions.push_back( position );
}

void QgsOverlayObject::setGeometry( QgsGeometry* g )
{
  delete mGeometry;
  mGeometry = g;
}

QgsPoint QgsOverlayObject::position() const
{
  if ( mPositions.size() > 0 )
  {
    return mPositions.at( 0 );
  }
  else
  {
    return QgsPoint( 0.0, 0.0 );
  }
}
