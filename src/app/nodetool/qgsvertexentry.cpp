/***************************************************************************
    qgsvertexentry.cpp  - entry for vertex of nodetool
    ---------------------
    begin                : April 2009
    copyright            : (C) 2009 by Richard Kostecky
    email                : csf dot kostej at mail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "nodetool/qgsvertexentry.h"

#include <qgsmaprenderer.h>

QgsVertexEntry::QgsVertexEntry( QgsMapCanvas *canvas, QgsMapLayer *layer, QgsPoint p, int originalIndex, QString tooltip, QgsVertexMarker::IconType type, int penWidth )
    : mSelected( false )
    , mEquals( -1 )
    , mInRubberBand( false )
    , mRubberBandNr( 0 )
    , mOriginalIndex( originalIndex )
    , mPenWidth( penWidth )
    , mToolTip( tooltip )
    , mType( type )
    , mMarker( 0 )
    , mCanvas( canvas )
    , mLayer( layer )
{
  setCenter( p );
}

QgsVertexEntry::~QgsVertexEntry()
{
  if ( mMarker )
  {
    delete mMarker;
    mMarker = 0;
  }
}

void QgsVertexEntry::setCenter( QgsPoint p )
{
  mPoint = p;
  p = mCanvas->mapRenderer()->layerToMapCoordinates( mLayer, p );

  if ( mCanvas->extent().contains( p ) )
  {
    if ( !mMarker )
    {
      mMarker = new QgsVertexMarker( mCanvas );
      mMarker->setIconType( mType );
      mMarker->setColor( mSelected ? Qt::blue : Qt::red );
      mMarker->setPenWidth( mPenWidth );

      if ( !mToolTip.isEmpty() )
        mMarker->setToolTip( mToolTip );
    }

    mMarker->setCenter( p );
  }
  else if ( mMarker )
  {
    delete mMarker;
    mMarker = 0;
  }
}

void QgsVertexEntry::moveCenter( double x, double y )
{
  mPoint += QgsVector( x, y );
  setCenter( mPoint );
}

void QgsVertexEntry::setSelected( bool selected )
{
  mSelected = selected;
  if ( mMarker )
  {
    mMarker->setColor( mSelected ? Qt::blue : Qt::red );
  }
}

void QgsVertexEntry::setRubberBandValues( bool inRubberBand, int rubberBandNr, int indexInRubberBand )
{
  mIndex        = indexInRubberBand;
  mInRubberBand = inRubberBand;
  mRubberBandNr = rubberBandNr;
}

void QgsVertexEntry::update()
{
  if ( mMarker )
    mMarker->update();
}
