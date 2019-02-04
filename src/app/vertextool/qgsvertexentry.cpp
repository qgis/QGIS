/***************************************************************************
    qgsvertexentry.cpp  - entry for vertex of vertextool
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

#include "vertextool/qgsvertexentry.h"

#include "qgsguiutils.h"
#include "qgsmaplayer.h"
#include "qgsmapcanvas.h"

QgsVertexEntry::QgsVertexEntry( QgsMapCanvas *canvas, QgsMapLayer *layer, const QgsPoint &p, QgsVertexId vertexId, const QString &tooltip, QgsVertexMarker::IconType type, int penWidth )
  : mSelected( false )
  , mPoint( p )
  , mVertexId( vertexId )
  , mPenWidth( penWidth )
  , mToolTip( tooltip )
  , mType( type )
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
  QgsPointXY pm = mCanvas->mapSettings().layerToMapCoordinates( mLayer, pointV1() );

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
      mMarker->setIconSize( QgsGuiUtils::scaleIconSize( 10 ) );
      mMarker->setPenWidth( QgsGuiUtils::scaleIconSize( mPenWidth ) );
      mMarker->setToolTip( mToolTip );
    }

    mMarker->setCenter( pm );
    mMarker->update();
  }
  else if ( mMarker )
  {
    delete mMarker;
    mMarker = nullptr;
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
