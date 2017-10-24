/***************************************************************************
  qgssnapindicator.cpp
  --------------------------------------
  Date                 : October 2017
  Copyright            : (C) 2017 by Martin Dobias
  Email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgssnapindicator.h"

#include "qgsvertexmarker.h"


QgsSnapIndicator::QgsSnapIndicator( QgsMapCanvas *canvas )
  : mCanvas( canvas )
{
}

QgsSnapIndicator::~QgsSnapIndicator()
{
}

void QgsSnapIndicator::setMatch( const QgsPointLocator::Match &match )
{
  mMatch = match;

  if ( !mMatch.isValid() )
  {
    mSnappingMarker.reset();
  }
  else
  {
    if ( !mSnappingMarker )
    {
      mSnappingMarker.reset( new QgsVertexMarker( mCanvas ) );
      mSnappingMarker->setIconType( QgsVertexMarker::ICON_CROSS );
      mSnappingMarker->setColor( Qt::magenta );
      mSnappingMarker->setPenWidth( 3 );
    }
    mSnappingMarker->setCenter( match.point() );
  }
}

void QgsSnapIndicator::setVisible( bool visible )
{
  mSnappingMarker->setVisible( visible );
}

bool QgsSnapIndicator::isVisible() const
{
  return mSnappingMarker->isVisible();
}
