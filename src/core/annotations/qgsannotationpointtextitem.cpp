/***************************************************************************
    qgsannotationpointtextitem.cpp
    ----------------
    begin                : August 2020
    copyright            : (C) 2020 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsannotationpointtextitem.h"
#include "qgstextrenderer.h"

QgsAnnotationPointTextItem::QgsAnnotationPointTextItem( const QString &text, QgsPointXY point )
  : QgsAnnotationItem()
  , mText( text )
  , mPoint( point )
{

}

QgsAnnotationPointTextItem::~QgsAnnotationPointTextItem() = default;

QString QgsAnnotationPointTextItem::type() const
{
  return QStringLiteral( "pointtext" );
}

void QgsAnnotationPointTextItem::render( QgsRenderContext &context, QgsFeedback * )
{
  QPointF pt;
  if ( context.coordinateTransform().isValid() )
  {
    double x = mPoint.x();
    double y = mPoint.y();
    double z = 0.0;
    context.coordinateTransform().transformInPlace( x, y, z );
    pt = QPointF( x, y );
  }
  else
    pt = mPoint.toQPointF();

  context.mapToPixel().transformInPlace( pt.rx(), pt.ry() );

  QgsTextRenderer::drawText( pt, mAngle * M_PI / 180.0,
                             QgsTextRenderer::convertQtHAlignment( mAlignment ),
                             mText.split( '\n' ), context, mTextFormat );
}

bool QgsAnnotationPointTextItem::writeXml( QDomElement &element, QDomDocument &document, const QgsReadWriteContext &context ) const
{
  element.setAttribute( QStringLiteral( "x" ), qgsDoubleToString( mPoint.x() ) );
  element.setAttribute( QStringLiteral( "y" ), qgsDoubleToString( mPoint.y() ) );
  element.setAttribute( QStringLiteral( "text" ), mText );
  element.setAttribute( QStringLiteral( "zIndex" ), zIndex() );
  element.setAttribute( QStringLiteral( "angle" ), qgsDoubleToString( mAngle ) );
  element.setAttribute( QStringLiteral( "alignment" ), QString::number( mAlignment ) );

  QDomElement textFormatElem = document.createElement( QStringLiteral( "pointTextFormat" ) );
  textFormatElem.appendChild( mTextFormat.writeXml( document, context ) );
  element.appendChild( textFormatElem );

  return true;
}

QgsAnnotationPointTextItem *QgsAnnotationPointTextItem::create()
{
  return new QgsAnnotationPointTextItem( QString(), QgsPointXY() );
}

bool QgsAnnotationPointTextItem::readXml( const QDomElement &element, const QgsReadWriteContext &context )
{
  const double x = element.attribute( QStringLiteral( "x" ) ).toDouble();
  const double y = element.attribute( QStringLiteral( "y" ) ).toDouble();
  mPoint = QgsPointXY( x, y );
  mText = element.attribute( QStringLiteral( "text" ) );
  mAngle = element.attribute( QStringLiteral( "angle" ) ).toDouble();
  mAlignment = static_cast< Qt::Alignment >( element.attribute( QStringLiteral( "alignment" ) ).toInt() );
  setZIndex( element.attribute( QStringLiteral( "zIndex" ) ).toInt() );

  const QDomElement textFormatElem = element.firstChildElement( QStringLiteral( "pointTextFormat" ) );
  if ( !textFormatElem.isNull() )
  {
    QDomNodeList textFormatNodeList = textFormatElem.elementsByTagName( QStringLiteral( "text-style" ) );
    QDomElement textFormatElem = textFormatNodeList.at( 0 ).toElement();
    mTextFormat.readXml( textFormatElem, context );
  }

  return true;
}

QgsAnnotationPointTextItem *QgsAnnotationPointTextItem::clone()
{
  std::unique_ptr< QgsAnnotationPointTextItem > item = std::make_unique< QgsAnnotationPointTextItem >( mText, mPoint );
  item->setFormat( mTextFormat );
  item->setAngle( mAngle );
  item->setAlignment( mAlignment );
  item->setZIndex( zIndex() );
  return item.release();
}

QgsRectangle QgsAnnotationPointTextItem::boundingBox() const
{
  return QgsRectangle( mPoint.x(), mPoint.y(), mPoint.x(), mPoint.y() );
}

QgsTextFormat QgsAnnotationPointTextItem::format() const
{
  return mTextFormat;
}

void QgsAnnotationPointTextItem::setFormat( const QgsTextFormat &format )
{
  mTextFormat = format;
}

Qt::Alignment QgsAnnotationPointTextItem::alignment() const
{
  return mAlignment;
}

void QgsAnnotationPointTextItem::setAlignment( Qt::Alignment alignment )
{
  mAlignment = alignment;
}
