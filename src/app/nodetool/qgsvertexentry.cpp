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
#include "qgsmaprenderer.h"

QgsVertexEntry::QgsVertexEntry( QgsMapCanvas *canvas, QgsMapLayer *layer, const QgsPointV2 &p, const QgsVertexId &vertexId, QString tooltip, QgsVertexMarker::IconType type, int penWidth )
    : mSelected( false )
    , mPoint( p )
    , mVertexId( vertexId )
    , mPenWidth( penWidth )
    , mToolTip( tooltip )
    , mType( type )
    , mMarker( 0 )
    , mCanvas( canvas )
    , mLayer( layer )
{
  placeMarker();
}

QgsVertexEntry::~QgsVertexEntry()
{
  delete mMarker;
}

void QgsVertexEntry::placeMarker()
{
  QgsPoint pm = mCanvas->mapSettings().layerToMapCoordinates( mLayer, pointV1() );

  if ( mCanvas->extent().contains( pm ) )
  {
    if ( !mMarker )
    {
      mMarker = new QgsVertexMarker( mCanvas );
      QColor c = mSelected ? QColor( Qt::blue ) : QColor( Qt::red );
      if ( mVertexId.type == QgsVertexId::CurveVertex )
      {
        mMarker->setIconType( QgsVertexMarker::ICON_CIRCLE );
      }
      else
      {
        mMarker->setIconType( mType );
      }
      mMarker->setColor( c );
      mMarker->setPenWidth( mPenWidth );
      mMarker->setToolTip( mToolTip );
    }

    mMarker->setCenter( pm );
    mMarker->update();
  }
  else if ( mMarker )
  {
    delete mMarker;
    mMarker = 0;
  }
}

void QgsVertexEntry::setSelected( bool selected )
{
  mSelected = selected;
  if ( mMarker )
  {
    QColor c = mSelected ? QColor( Qt::blue ) : QColor( Qt::red );
    mMarker->setColor( c );
    mMarker->update();
  }
}
