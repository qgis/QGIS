/***************************************************************************
                             qgslayouteffect.cpp
                             -------------------
    begin                : October 2017
    copyright            : (C) 2017 by Nyall Dawson
    email                : nyall.dawson@gmail.com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <QPainter>

#include "qgslayouteffect.h"
#include "moc_qgslayouteffect.cpp"

void QgsLayoutEffect::draw( QPainter *painter )
{
  QPoint offset;
  QPixmap pixmap;

  // Set desired composition mode then draw source
  painter->setCompositionMode( mCompositionMode );

  if ( mCompositionMode == QPainter::CompositionMode_SourceOver )
  {
    // Normal (sourceover) blending, do faster drawSource operation
    drawSource( painter );
    return;
  }

  // Otherwise, draw using pixmap so QPrinter output works as expected
  if ( sourceIsPixmap() )
  {
    // No point in drawing in device coordinates (pixmap will be scaled anyways).
    pixmap = sourcePixmap( Qt::LogicalCoordinates, &offset );
  }
  else
  {
    // Draw pixmap in device coordinates to avoid pixmap scaling;
    pixmap = sourcePixmap( Qt::DeviceCoordinates, &offset );
    painter->setWorldTransform( QTransform() );
  }

  painter->drawPixmap( offset, pixmap );
}

void QgsLayoutEffect::setCompositionMode( QPainter::CompositionMode compositionMode )
{
  mCompositionMode = compositionMode;

  // force redraw with new composition mode
  update();
}



