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

QgsComposerFrame::QgsComposerFrame( QgsComposition* c, QgsComposerMultiFrame* mf, qreal x, qreal y, qreal width, qreal height )
  : QgsComposerItem( x, y, width, height, c )
  , mMultiFrame( mf )
{
}

QgsComposerFrame::QgsComposerFrame()
  : QgsComposerItem( 0, 0, 0, 0, 0 )
  , mMultiFrame( 0 )
{
}

QgsComposerFrame::~QgsComposerFrame()
{
}

bool QgsComposerFrame::writeXML( QDomElement& elem, QDomDocument & doc ) const
{
  QDomElement frameElem = doc.createElement( "ComposerFrame" );
  frameElem.setAttribute( "sectionX", QString::number( mSection.x() ) );
  frameElem.setAttribute( "sectionY", QString::number( mSection.y() ) );
  frameElem.setAttribute( "sectionWidth", QString::number( mSection.width() ) );
  frameElem.setAttribute( "sectionHeight", QString::number( mSection.height() ) );
  elem.appendChild( frameElem );
  return _writeXML( frameElem, doc );
}

bool QgsComposerFrame::readXML( const QDomElement& itemElem, const QDomDocument& doc )
{
  double x = itemElem.attribute( "sectionX" ).toDouble();
  double y = itemElem.attribute( "sectionY" ).toDouble();
  double width = itemElem.attribute( "sectionWidth" ).toDouble();
  double height = itemElem.attribute( "sectionHeight" ).toDouble();
  mSection = QRectF( x, y, width, height );
  QDomElement composerItem = itemElem.firstChildElement( "ComposerItem" );
  if ( composerItem.isNull() )
  {
    return false;
  }
  return _readXML( composerItem, doc );
}

void QgsComposerFrame::paint( QPainter* painter, const QStyleOptionGraphicsItem* itemStyle, QWidget* pWidget )
{
  Q_UNUSED( itemStyle );
  Q_UNUSED( pWidget );

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

void QgsComposerFrame::beginItemCommand( const QString& text )
{
  if ( mComposition )
  {
    mComposition->beginMultiFrameCommand( multiFrame(), text );
  }
}

void QgsComposerFrame::endItemCommand()
{
  if ( mComposition )
  {
    mComposition->endMultiFrameCommand();
  }
}
