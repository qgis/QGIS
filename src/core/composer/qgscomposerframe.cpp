/***************************************************************************
                              qgscomposerframe.cpp
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

#include "qgscomposerframe.h"
#include "qgscomposermultiframe.h"

QgsComposerFrame::QgsComposerFrame( QgsComposition* c, QgsComposerMultiFrame* mf, double x, double y, double width, double height ):
    QgsComposerItem( x, y, width, height, c ), mMultiFrame( mf )
{
}

QgsComposerFrame::~QgsComposerFrame()
{
}

bool QgsComposerFrame::writeXML( QDomElement& elem, QDomDocument & doc ) const
{
  return false; //_writeXML( element, doc );
}

bool QgsComposerFrame::readXML( const QDomElement& itemElem, const QDomDocument& doc )
{
  return false; //_readXML( element, doc )
}

void QgsComposerFrame::paint( QPainter* painter, const QStyleOptionGraphicsItem* itemStyle, QWidget* pWidget )
{
  if ( !painter )
  {
    return;
  }

  drawBackground( painter );
  if ( mMultiFrame )
  {
    mMultiFrame->render( painter, mSection );
  }

  drawFrame( painter );
  if ( isSelected() )
  {
    drawSelectionBoxes( painter );
  }
}
