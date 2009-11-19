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
  initBrushAndPen();
}

QgsComposerShape::QgsComposerShape( qreal x, qreal y, qreal width, qreal height, QgsComposition* composition ): QgsComposerItem( x, y, width, height, composition ), mShape( Ellipse )
{
  setSceneRect( QRectF( x, y, width, height ) );
  initBrushAndPen();
}

QgsComposerShape::~QgsComposerShape()
{

}

void QgsComposerShape::paint( QPainter* painter, const QStyleOptionGraphicsItem* itemStyle, QWidget* pWidget )
{
  double width = rect().width();
  double height = rect().height();
  imageSizeConsideringRotation( width, height );

  painter->save();
  painter->setRenderHint( QPainter::Antialiasing );
  painter->setPen( mPen );
  painter->setBrush( mBrush );

  painter->translate( rect().width() / 2.0, rect().height() / 2.0 );
  painter->rotate( mRotation );
  painter->translate( -width / 2.0, -height / 2.0 );

  double halfPenWidth = mPen.widthF() / 2.0;

  switch ( mShape )
  {
    case Ellipse:
      painter->drawEllipse( QRectF( halfPenWidth, halfPenWidth , width - mPen.widthF(), height - mPen.widthF() ) );
      break;
    case Rectangle:
      painter->drawRect( QRectF( halfPenWidth, halfPenWidth , width - mPen.widthF(), height - mPen.widthF() ) );
      break;
    case Triangle:
      QPolygonF triangle;
      triangle << QPointF( halfPenWidth, height - halfPenWidth );
      triangle << QPointF( width - halfPenWidth, height - halfPenWidth );
      triangle << QPointF( width / 2.0, halfPenWidth );
      painter->drawPolygon( triangle );
      break;
  }

  painter->restore();

  drawFrame( painter );
  if ( isSelected() )
  {
    drawSelectionBoxes( painter );
  }
}

bool QgsComposerShape::writeXML( QDomElement& elem, QDomDocument & doc ) const
{
  QDomElement composerShapeElem = doc.createElement( "ComposerShape" );
  composerShapeElem.setAttribute( "shapeType", mShape );
  composerShapeElem.setAttribute( "outlineWidth", mPen.widthF() );
  composerShapeElem.setAttribute( "transparentFill", mBrush.style() == Qt::NoBrush );
  QDomElement outlineColorElem = doc.createElement( "OutlineColor" );
  outlineColorElem.setAttribute( "red", mPen.color().red() );
  outlineColorElem.setAttribute( "green", mPen.color().green() );
  outlineColorElem.setAttribute( "blue", mPen.color().blue() );
  composerShapeElem.appendChild( outlineColorElem );
  QDomElement fillColorElem = doc.createElement( "FillColor" );
  fillColorElem.setAttribute( "red", mBrush.color().red() );
  fillColorElem.setAttribute( "green", mBrush.color().green() );
  fillColorElem.setAttribute( "blue", mBrush.color().blue() );
  composerShapeElem.appendChild( fillColorElem );
  elem.appendChild( composerShapeElem );
  return _writeXML( composerShapeElem, doc );
}

bool QgsComposerShape::readXML( const QDomElement& itemElem, const QDomDocument& doc )
{
  mShape = QgsComposerShape::Shape( itemElem.attribute( "shapeType", "0" ).toInt() );
  mPen.setWidthF( itemElem.attribute( "outlineWidth", "0.4" ).toDouble() );

  //transparent fill
  bool transparent = itemElem.attribute( "transparentFill", "1" ).toInt() == 1;
  if ( transparent )
  {
    mBrush.setStyle( Qt::NoBrush );
  }
  else
  {
    mBrush.setStyle( Qt::SolidPattern );
  }

  //outline color
  QDomNodeList outlineColorList = itemElem.elementsByTagName( "OutlineColor" );
  if ( outlineColorList.size() > 0 )
  {
    QDomElement outlineColorElem = outlineColorList.at( 0 ).toElement();
    int penRed = outlineColorElem.attribute( "red", "0" ).toInt();
    int penGreen = outlineColorElem.attribute( "green", "0" ).toInt();
    int penBlue = outlineColorElem.attribute( "blue", "0" ).toInt();
    mPen.setColor( QColor( penRed, penGreen, penBlue ) );
  }

  //fill color
  QDomNodeList fillNodeList = itemElem.elementsByTagName( "FillColor" );
  if ( fillNodeList.size() > 0 )
  {
    QDomElement fillColorElem = fillNodeList.at( 0 ).toElement();
    int brushRed = fillColorElem.attribute( "red", "0" ).toInt();
    int brushGreen = fillColorElem.attribute( "green", "0" ).toInt();
    int brushBlue = fillColorElem.attribute( "blue", "0" ).toInt();
    mBrush.setColor( QColor( brushRed, brushGreen, brushBlue ) );
  }


  //restore general composer item properties
  QDomNodeList composerItemList = itemElem.elementsByTagName( "ComposerItem" );
  if ( composerItemList.size() > 0 )
  {
    QDomElement composerItemElem = composerItemList.at( 0 ).toElement();
    _readXML( composerItemElem, doc );
  }
  return true;
}

void QgsComposerShape::setLineWidth( double width )
{
  mPen.setWidthF( width );
}

double QgsComposerShape::lineWidth() const
{
  return mPen.widthF();
}

void QgsComposerShape::setOutlineColor( const QColor& color )
{
  mPen.setColor( color );
}

QColor QgsComposerShape::outlineColor() const
{
  return mPen.color();
}

void QgsComposerShape::setFillColor( const QColor& color )
{
  mBrush.setColor( color );
}

QColor QgsComposerShape::fillColor() const
{
  return mBrush.color();
}

bool QgsComposerShape::transparentFill() const
{
  return mBrush.style() == Qt::NoBrush;
}

void QgsComposerShape::setTransparentFill( bool transparent )
{
  if ( transparent )
  {
    mBrush.setStyle( Qt::NoBrush );
  }
  else
  {
    mBrush.setStyle( Qt::SolidPattern );
  }
}

void QgsComposerShape::initBrushAndPen()
{
  mPen.setColor( QColor( 0, 0, 0 ) );
  mPen.setWidthF( 1 );
  mPen.setJoinStyle( Qt::RoundJoin );
  mBrush.setColor( QColor( 0, 0, 0 ) );
  mBrush.setStyle( Qt::NoBrush );
}
