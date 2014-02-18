/***************************************************************************
                           qgscomposereffect.cpp
                             -------------------
    begin                : March 2013
    copyright            : (C) 2013 by Nyall Dawson
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

#include "qgscomposereffect.h"

QgsComposerEffect::QgsComposerEffect()
    : mCompositionMode( QPainter::CompositionMode_SourceOver )
{
}

QgsComposerEffect::~QgsComposerEffect()
{
}

void QgsComposerEffect::draw( QPainter *painter )
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

void QgsComposerEffect::setCompositionMode( const QPainter::CompositionMode &compositionMode )
{
  mCompositionMode = compositionMode;

  // force redraw with new composition mode
  update();
}



