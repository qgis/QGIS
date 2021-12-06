/***************************************************************************
  qgsattributetablerenderer.cpp - QgsAttributeTableRenderer

 ---------------------
 begin                : 6.12.2021
 copyright            : (C) 2021 by Alessandro Pasotti
 email                : elpaso at itopen dot it
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "qgsattributetablerenderer.h"
#include "qgsrasterinterface.h"

QgsAttributeTableRenderer::QgsAttributeTableRenderer( QgsRasterInterface *input, int band, QgsRasterShader *shader )
  : QgsRasterRenderer( input, QStringLiteral( "attributetable" ) )
  , mShader( shader )
  , mBand( band )
{

}

QgsAttributeTableRenderer *QgsAttributeTableRenderer::clone() const
{
  return new QgsAttributeTableRenderer( input(), band() );
}

QgsRasterRenderer *QgsAttributeTableRenderer::create( const QDomElement &elem, QgsRasterInterface *input )
{
  if ( elem.isNull() )
  {
    return nullptr;
  }

  const int band = elem.attribute( QStringLiteral( "band" ), QStringLiteral( "-1" ) ).toInt();

  QgsAttributeTableRenderer *renderer = new QgsAttributeTableRenderer( input, band );

  renderer->readXml( elem );

  return renderer;
}

QgsRasterBlock *QgsAttributeTableRenderer::block( int bandNo, const QgsRectangle &extent, int width, int height, QgsRasterBlockFeedback *feedback )
{
  return nullptr;
}

void QgsAttributeTableRenderer::writeXml( QDomDocument &doc, QDomElement &parentElem ) const
{

}

QList<QPair<QString, QColor> > QgsAttributeTableRenderer::legendSymbologyItems() const
{
  return QList<QPair<QString, QColor> >();
}

QList<QgsLayerTreeModelLegendNode *> QgsAttributeTableRenderer::createLegendNodes( QgsLayerTreeLayer *nodeLayer )
{

}

QList<int> QgsAttributeTableRenderer::usesBands() const
{
  return QList<int>() << band();
}

void QgsAttributeTableRenderer::toSld( QDomDocument &doc, QDomElement &element, const QVariantMap &props ) const
{

}

bool QgsAttributeTableRenderer::accept( QgsStyleEntityVisitorInterface *visitor ) const
{
  return false;
}
