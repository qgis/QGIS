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
  drawSource( painter );

}

void QgsComposerEffect::setCompositionMode( const QPainter::CompositionMode compositionMode )
{
  mCompositionMode = compositionMode;

  // force redraw with new composition mode
  update();
}



