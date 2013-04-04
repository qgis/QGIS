/***************************************************************************
                              qgssvgannotationitem.cpp
                              ------------------------
  begin                : November, 2012
  copyright            : (C) 2012 by Marco Hugentobler
  email                : marco dot hugentobler at sourcepole dot ch
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgssvgannotationitem.h"
#include "qgsproject.h"
#include <QDomDocument>
#include <QDomElement>


QgsSvgAnnotationItem::QgsSvgAnnotationItem( QgsMapCanvas* canvas ): QgsAnnotationItem( canvas )
{

}

QgsSvgAnnotationItem::~QgsSvgAnnotationItem()
{

}

void QgsSvgAnnotationItem::writeXML( QDomDocument& doc ) const
{
  QDomElement documentElem = doc.documentElement();
  if ( documentElem.isNull() )
  {
    return;
  }

  QDomElement svgAnnotationElem = doc.createElement( "SVGAnnotationItem" );
  svgAnnotationElem.setAttribute( "file", QgsProject::instance()->writePath( mFilePath ) );
  _writeXML( doc, svgAnnotationElem );
  documentElem.appendChild( svgAnnotationElem );
}

void QgsSvgAnnotationItem::readXML( const QDomDocument& doc, const QDomElement& itemElem )
{
  QString filePath = QgsProject::instance()->readPath( itemElem.attribute( "file" ) );
  setFilePath( filePath );
  QDomElement annotationElem = itemElem.firstChildElement( "AnnotationItem" );
  if ( !annotationElem.isNull() )
  {
    _readXML( doc, annotationElem );
  }
}

void QgsSvgAnnotationItem::paint( QPainter* painter )
{
  if ( !painter )
  {
    return;
  }

  drawFrame( painter );
  if ( mMapPositionFixed )
  {
    drawMarkerSymbol( painter );
  }

  //keep width/height ratio of svg
  QRect viewBox = mSvgRenderer.viewBox();
  if ( viewBox.isValid() )
  {
    double widthRatio = mFrameSize.width() / viewBox.width();
    double heightRatio = mFrameSize.height() / viewBox.height();
    double renderWidth = 0;
    double renderHeight = 0;
    if ( widthRatio <= heightRatio )
    {
      renderWidth = mFrameSize.width();
      renderHeight = viewBox.height() * mFrameSize.width() / viewBox.width() ;
    }
    else
    {
      renderHeight = mFrameSize.height();
      renderWidth = viewBox.width() * mFrameSize.height() / viewBox.height() ;
    }

    mSvgRenderer.render( painter, QRectF( mOffsetFromReferencePoint.x(), mOffsetFromReferencePoint.y(), renderWidth,
                                          renderHeight ) );
  }
  if ( isSelected() )
  {
    drawSelectionBoxes( painter );
  }
}

void QgsSvgAnnotationItem::setFilePath( const QString& file )
{
  mFilePath = file;
  mSvgRenderer.load( mFilePath );
}
