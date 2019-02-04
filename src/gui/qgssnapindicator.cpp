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

#include "qgsguiutils.h"
#include "qgsmapcanvas.h"
#include "qgssettings.h"
#include "qgsvectorlayer.h"
#include "qgsvertexmarker.h"

#include <QToolTip>


QgsSnapIndicator::QgsSnapIndicator( QgsMapCanvas *canvas )
  : mCanvas( canvas )
{
}

QgsSnapIndicator::~QgsSnapIndicator() = default;

void QgsSnapIndicator::setMatch( const QgsPointLocator::Match &match )
{
  mMatch = match;

  if ( !mMatch.isValid() )
  {
    mSnappingMarker.reset();
    QToolTip::hideText();
  }
  else
  {
    if ( !mSnappingMarker )
    {
      mSnappingMarker.reset( new QgsVertexMarker( mCanvas ) );
      mSnappingMarker->setIconSize( QgsGuiUtils::scaleIconSize( 10 ) );
      mSnappingMarker->setPenWidth( QgsGuiUtils::scaleIconSize( 3 ) );
    }

    QgsSettings s;

    QColor color = s.value( QStringLiteral( "/qgis/digitizing/snap_color" ), QColor( Qt::magenta ) ).value<QColor>();
    mSnappingMarker->setColor( color );

    int iconType;
    if ( match.hasVertex() )
    {
      if ( match.layer() )
        iconType = QgsVertexMarker::ICON_BOX;  // vertex snap
      else
        iconType = QgsVertexMarker::ICON_X;  // intersection snap
    }
    else  // must be segment snap
    {
      iconType = QgsVertexMarker::ICON_DOUBLE_TRIANGLE;
    }

    mSnappingMarker->setIconType( iconType );

    mSnappingMarker->setCenter( match.point() );

    // tooltip
    if ( s.value( QStringLiteral( "/qgis/digitizing/snap_tooltip" ), false ).toBool() )
    {
      QPoint ptCanvas = mSnappingMarker->toCanvasCoordinates( match.point() ).toPoint();
      QPoint ptGlobal = mCanvas->mapToGlobal( ptCanvas );
      QRect rect( ptCanvas.x(), ptCanvas.y(), 1, 1 );  // area where is the tooltip valid
      QString layerName = match.layer() ? match.layer()->name() : QString();
      QToolTip::showText( ptGlobal, layerName, mCanvas, rect );
    }
  }
}

void QgsSnapIndicator::setVisible( bool visible )
{
  if ( mSnappingMarker )
    mSnappingMarker->setVisible( visible );
}

bool QgsSnapIndicator::isVisible() const
{
  if ( mSnappingMarker )
    return mSnappingMarker->isVisible();

  return false;
}
