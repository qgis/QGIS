/***************************************************************************
                              qgstextannotationitem.cpp
                              ------------------------
  begin                : February 9, 2010
  copyright            : (C) 2010 by Marco Hugentobler
  email                : marco dot hugentobler at hugis dot net
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgstextannotationitem.h"
#include <QDomDocument>
#include <QPainter>

QgsTextAnnotationItem::QgsTextAnnotationItem( QgsMapCanvas* canvas ): QgsAnnotationItem( canvas ), mDocument( new QTextDocument( QObject::tr( "QGIS rocks!" ) ) )
{
  mDocument->setUseDesignMetrics( true );
}

QgsTextAnnotationItem::~QgsTextAnnotationItem()
{
  delete mDocument;
}

QTextDocument* QgsTextAnnotationItem::document() const
{
  if ( !mDocument )
  {
    return 0;
  }

  return mDocument->clone();
}

void QgsTextAnnotationItem::setDocument( const QTextDocument* doc )
{
  delete mDocument;
  mDocument = doc->clone();
}

void QgsTextAnnotationItem::paint( QPainter * painter )
{
  if ( !painter || !mDocument )
  {
    return;
  }

  drawFrame( painter );
  if ( mMapPositionFixed )
  {
    drawMarkerSymbol( painter );
  }
  double frameWidth = mFrameBorderWidth;
  mDocument->setTextWidth( mFrameSize.width() );

  painter->save();
  painter->translate( mOffsetFromReferencePoint.x() + frameWidth / 2.0, mOffsetFromReferencePoint.y() + \
                      frameWidth / 2.0 );

  //draw text document
  mDocument->drawContents( painter, QRectF( 0, 0, mFrameSize.width() - frameWidth / 2.0, mFrameSize.height() - frameWidth / 2.0 ) );
  painter->restore();
  if ( isSelected() )
  {
    drawSelectionBoxes( painter );
  }
}

void QgsTextAnnotationItem::writeXML( QDomDocument& doc ) const
{
  QDomElement documentElem = doc.documentElement();
  if ( documentElem.isNull() )
  {
    return;
  }
  QDomElement annotationElem = doc.createElement( "TextAnnotationItem" );
  if ( mDocument )
  {
    annotationElem.setAttribute( "document", mDocument->toHtml() );
  }
  _writeXML( doc, annotationElem );
  documentElem.appendChild( annotationElem );
}

void QgsTextAnnotationItem::readXML( const QDomDocument& doc, const QDomElement& itemElem )
{
  delete mDocument;
  mDocument = new QTextDocument;
  mDocument->setHtml( itemElem.attribute( "document", "<html>QGIS rocks!</html>" ) );
  QDomElement annotationElem = itemElem.firstChildElement( "AnnotationItem" );
  if ( !annotationElem.isNull() )
  {
    _readXML( doc, annotationElem );
  }
}
