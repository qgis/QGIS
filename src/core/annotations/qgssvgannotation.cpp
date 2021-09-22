/***************************************************************************
                              qgssvgannotation.cpp
                              --------------------
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

#include "qgssvgannotation.h"

#include "qgsreadwritecontext.h"
#include "qgsproject.h"
#include "qgssymbollayerutils.h"

#include <QDomDocument>
#include <QDomElement>


QgsSvgAnnotation::QgsSvgAnnotation( QObject *parent )
  : QgsAnnotation( parent )
{

}

QgsSvgAnnotation *QgsSvgAnnotation::clone() const
{
  std::unique_ptr< QgsSvgAnnotation > c( new QgsSvgAnnotation() );
  copyCommonProperties( c.get() );
  c->setFilePath( mFilePath );
  return c.release();
}

void QgsSvgAnnotation::writeXml( QDomElement &elem, QDomDocument &doc, const QgsReadWriteContext &context ) const
{
  const QString filePath = QgsSymbolLayerUtils::svgSymbolPathToName( mFilePath, context.pathResolver() );
  QDomElement svgAnnotationElem = doc.createElement( QStringLiteral( "SVGAnnotationItem" ) );
  svgAnnotationElem.setAttribute( QStringLiteral( "file" ), filePath );
  _writeXml( svgAnnotationElem, doc, context );
  elem.appendChild( svgAnnotationElem );
}

void QgsSvgAnnotation::readXml( const QDomElement &itemElem, const QgsReadWriteContext &context )
{
  const QString filePath = QgsSymbolLayerUtils::svgSymbolNameToPath( itemElem.attribute( QStringLiteral( "file" ) ), context.pathResolver() );
  setFilePath( filePath );
  const QDomElement annotationElem = itemElem.firstChildElement( QStringLiteral( "AnnotationItem" ) );
  if ( !annotationElem.isNull() )
  {
    _readXml( annotationElem, context );
  }
}

void QgsSvgAnnotation::renderAnnotation( QgsRenderContext &context, QSizeF size ) const
{
  QPainter *painter = context.painter();
  if ( !painter )
  {
    return;
  }

  //keep width/height ratio of svg
  const QRect viewBox = mSvgRenderer.viewBox();
  if ( viewBox.isValid() )
  {
    const double widthRatio = size.width() / viewBox.width();
    const double heightRatio = size.height() / viewBox.height();
    double renderWidth = 0;
    double renderHeight = 0;
    if ( widthRatio <= heightRatio )
    {
      renderWidth = size.width();
      renderHeight = viewBox.height() * widthRatio;
    }
    else
    {
      renderHeight = size.height();
      renderWidth = viewBox.width() * heightRatio;
    }

    mSvgRenderer.render( painter, QRectF( 0, 0, renderWidth,
                                          renderHeight ) );
  }
}

void QgsSvgAnnotation::setFilePath( const QString &file )
{
  mFilePath = file;
  mSvgRenderer.load( mFilePath );
  emit appearanceChanged();
}
