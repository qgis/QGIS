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
#include "qgssettingsregistrycore.h"
#include "qgsvectorlayer.h"
#include "qgsvertexmarker.h"
#include "qgssettingsentryimpl.h"

#include <QToolTip>


QgsSnapIndicator::QgsSnapIndicator( QgsMapCanvas *canvas )
  : mCanvas( canvas )
{
  mSnappingMarker.setParentOwner( canvas );
}

QgsSnapIndicator::~QgsSnapIndicator() = default;

void QgsSnapIndicator::setMatch( const QgsPointLocator::Match &match )
{
  mMatch = match;

  if ( !mMatch.isValid() )
  {
    if ( mSnappingMarker )
    {
      mCanvas->scene()->removeItem( mSnappingMarker );
      mSnappingMarker.reset(); // need to delete since QGraphicsSene::removeItem transfers back ownership
    }
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

    const QColor color = QgsSettingsRegistryCore::settingsDigitizingSnapColor->value();
    mSnappingMarker->setColor( color );

    int iconType;
    if ( match.hasLineEndpoint() )
    {
      iconType = QgsVertexMarker::ICON_INVERTED_TRIANGLE; // line endpoint snap
    }
    else if ( match.hasVertex() )
    {
      if ( match.layer() )
        iconType = QgsVertexMarker::ICON_BOX; // vertex snap
      else
        iconType = QgsVertexMarker::ICON_X; // intersection snap
    }
    else if ( match.hasMiddleSegment() )
    {
      iconType = QgsVertexMarker::ICON_TRIANGLE; // middle snap
    }
    else if ( match.hasCentroid() )
    {
      iconType = QgsVertexMarker::ICON_CIRCLE; // centroid snap
    }
    else if ( match.hasArea() )
    {
      iconType = QgsVertexMarker::ICON_RHOMBUS; // area snap
    }
    else // must be segment snap
    {
      iconType = QgsVertexMarker::ICON_DOUBLE_TRIANGLE;
    }

    mSnappingMarker->setIconType( iconType );

    mSnappingMarker->setCenter( match.point() );

    // tooltip
    if ( QgsSettingsRegistryCore::settingsDigitizingSnapTooltip->value() )
    {
      const QPoint ptCanvas = mSnappingMarker->toCanvasCoordinates( match.point() ).toPoint();
      const QPoint ptGlobal = mCanvas->mapToGlobal( ptCanvas );
      const QRect rect( ptCanvas.x(), ptCanvas.y(), 1, 1 ); // area where is the tooltip valid
      const QString layerName = match.layer() ? match.layer()->name() : QString();
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
