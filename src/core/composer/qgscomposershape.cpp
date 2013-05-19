/***************************************************************************
                         qgscomposershape.cpp
                         ----------------------
    begin                : November 2009
    copyright            : (C) 2009 by Marco Hugentobler
    email                : marco@hugis.net
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgscomposershape.h"
#include <QPainter>

QgsComposerShape::QgsComposerShape( QgsComposition* composition ): QgsComposerItem( composition ), mShape( Ellipse )
{
  setFrameEnabled( true );
}

QgsComposerShape::QgsComposerShape( qreal x, qreal y, qreal width, qreal height, QgsComposition* composition ): QgsComposerItem( x, y, width, height, composition ), mShape( Ellipse )
{
  setSceneRect( QRectF( x, y, width, height ) );
  setFrameEnabled( true );
}

QgsComposerShape::~QgsComposerShape()
{

}

void QgsComposerShape::paint( QPainter* painter, const QStyleOptionGraphicsItem* itemStyle, QWidget* pWidget )
{
  Q_UNUSED( itemStyle );
  Q_UNUSED( pWidget );
  if ( !painter )
  {
    return;
  }
  drawBackground( painter );
  drawFrame( painter );

  if ( isSelected() )
  {
    drawSelectionBoxes( painter );
  }
}


void QgsComposerShape::drawShape( QPainter* p )
{

  p->save();
  p->setRenderHint( QPainter::Antialiasing );

  p->translate( rect().width() / 2.0, rect().height() / 2.0 );
  p->rotate( mRotation );
  p->translate( -rect().width() / 2.0, -rect().height() / 2.0 );

  switch ( mShape )
  {
    case Ellipse:
      p->drawEllipse( QRectF( 0, 0 , rect().width(), rect().height() ) );
      break;
    case Rectangle:
      p->drawRect( QRectF( 0, 0 , rect().width(), rect().height() ) );
      break;
    case Triangle:
      QPolygonF triangle;
      triangle << QPointF( 0, rect().height() );
      triangle << QPointF( rect().width() , rect().height() );
      triangle << QPointF( rect().width() / 2.0, 0 );
      p->drawPolygon( triangle );
      break;
  }
  p->restore();

}


void QgsComposerShape::drawFrame( QPainter* p )
{
  if ( mFrame && p )
  {
    p->setPen( pen() );
    p->setBrush( Qt::NoBrush );
    p->setRenderHint( QPainter::Antialiasing, true );
    drawShape( p );
  }
}

void QgsComposerShape::drawBackground( QPainter* p )
{
  if ( mBackground && p )
  {
    p->setBrush( brush() );//this causes a problem in atlas generation
    p->setPen( Qt::NoPen );
    p->setRenderHint( QPainter::Antialiasing, true );
    drawShape( p );
  }
}


bool QgsComposerShape::writeXML( QDomElement& elem, QDomDocument & doc ) const
{
  QDomElement composerShapeElem = doc.createElement( "ComposerShape" );
  composerShapeElem.setAttribute( "shapeType", mShape );
  elem.appendChild( composerShapeElem );
  return _writeXML( composerShapeElem, doc );
}

bool QgsComposerShape::readXML( const QDomElement& itemElem, const QDomDocument& doc )
{
  mShape = QgsComposerShape::Shape( itemElem.attribute( "shapeType", "0" ).toInt() );

  //restore general composer item properties
  QDomNodeList composerItemList = itemElem.elementsByTagName( "ComposerItem" );
  if ( composerItemList.size() > 0 )
  {
    QDomElement composerItemElem = composerItemList.at( 0 ).toElement();
    _readXML( composerItemElem, doc );
  }
  emit itemChanged();
  return true;
}


void QgsComposerShape::setRotation( double r )
{
  //adapt rectangle size
  double width = rect().width();
  double height = rect().height();
  sizeChangedByRotation( width, height );

  //adapt scene rect to have the same center and the new width / height
  double x = transform().dx() + rect().width() / 2.0 - width / 2.0;
  double y = transform().dy() + rect().height() / 2.0 - height / 2.0;
  QgsComposerItem::setSceneRect( QRectF( x, y, width, height ) );

  QgsComposerItem::setRotation( r );
}

void QgsComposerShape::setSceneRect( const QRectF& rectangle )
{


  //consider to change size of the shape if the rectangle changes width and/or height
  if ( rectangle.width() != rect().width() || rectangle.height() != rect().height() )
  {
    double newShapeWidth = rectangle.width();
    double newShapeHeight = rectangle.height();
    imageSizeConsideringRotation( newShapeWidth, newShapeHeight );
  }

  QgsComposerItem::setSceneRect( rectangle );
}
