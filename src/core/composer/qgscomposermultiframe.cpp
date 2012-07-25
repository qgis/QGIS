/***************************************************************************
                              qgscomposermultiframe.cpp
    ------------------------------------------------------------
    begin                : July 2012
    copyright            : (C) 2012 by Marco Hugentobler
    email                : marco dot hugentobler at sourcepole dot ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgscomposermultiframe.h"
#include "qgscomposerframe.h"

QgsComposerMultiFrame::QgsComposerMultiFrame( QgsComposition* c ): mComposition( c )
{
}

QgsComposerMultiFrame::QgsComposerMultiFrame(): mComposition( 0 )
{
}

QgsComposerMultiFrame::~QgsComposerMultiFrame()
{
}

void QgsComposerMultiFrame::recalculateFrameSizes()
{
  if ( mFrameItems.size() > 0 )
  {
    QSizeF size = totalSize();
    QgsComposerFrame* item = mFrameItems[0];
    item->setContentSection( QRectF( 0, 0, item->rect().width(), item->rect().height() ) );
  }
}

void QgsComposerMultiFrame::addFrame( QgsComposerFrame* frame )
{
  mFrameItems.push_back( frame );
  QObject::connect( frame, SIGNAL( sizeChanged() ), this, SLOT( recalculateFrameSizes() ) );
}
