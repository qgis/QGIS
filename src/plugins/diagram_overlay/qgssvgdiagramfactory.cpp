/***************************************************************************
                         qgssvgdiagramfactory.cpp  -  description
                         ------------------------
    begin                : November 2007
    copyright            : (C) 2007 by Marco Hugentobler
    email                : marco dot hugentobler at karto dot baug dot ethz dot ch
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgssvgdiagramfactory.h"
#include "qgsrendercontext.h"
#include <QImage>
#include <QPainter>
#include <QDomNode>

QgsSVGDiagramFactory::QgsSVGDiagramFactory(): QgsDiagramFactory()
{

}

QgsSVGDiagramFactory::~QgsSVGDiagramFactory()
{

}

QImage* QgsSVGDiagramFactory::createDiagram( int size, const QgsFeature& f, const QgsRenderContext& renderContext ) const
{
  Q_UNUSED( f );
  //check default size
  QSize defaultSize = mRenderer.defaultSize();
  qreal scaleFactor;
  int imageWidth, imageHeight;

  //size parameter applies to maximum of width, height
  if ( defaultSize.width() >= defaultSize.height() )
  {
    scaleFactor = (( double )size * diagramSizeScaleFactor( renderContext ) * renderContext.rasterScaleFactor() ) / defaultSize.width();
  }
  else
  {
    scaleFactor = (( double )size * diagramSizeScaleFactor( renderContext ) * renderContext.rasterScaleFactor() ) / defaultSize.height();
  }

  imageWidth = ( int )( defaultSize.width() * scaleFactor );
  imageHeight = ( int )( defaultSize.height() * scaleFactor );
  QImage* diagramImage = new QImage( QSize( imageWidth, imageHeight ), QImage::Format_ARGB32_Premultiplied );
  diagramImage->fill( qRgba( 0, 0, 0, 0 ) ); //transparent background

  QPainter p;
  p.begin( diagramImage );
  p.setRenderHint( QPainter::Antialiasing );
  //p.scale(scaleFactor, scaleFactor);

  //render image
  mRenderer.render( &p );

  p.end();
  return diagramImage;
}

int QgsSVGDiagramFactory::getDiagramDimensions( int size, const QgsFeature& f, const QgsRenderContext& context, int& width, int& height ) const
{
  Q_UNUSED( f );
  double scaleFactor;
  QSize defaultSize = mRenderer.defaultSize();
  //size parameter applies to maximum of width, height
  if ( defaultSize.width() >= defaultSize.height() )
  {
    scaleFactor = (( double )size * diagramSizeScaleFactor( context ) * context.rasterScaleFactor() ) / defaultSize.width();
  }
  else
  {
    scaleFactor = (( double )size * diagramSizeScaleFactor( context ) * context.rasterScaleFactor() ) / defaultSize.height();
  }
  width = ( int )( defaultSize.width() * scaleFactor );
  height = ( int )( defaultSize.height() * scaleFactor );
  return 0;
}

bool QgsSVGDiagramFactory::setSVGData( const QByteArray& data, const QString& filePath )
{
  mSvgFilePath = filePath;
  return mRenderer.load( data );
}

bool QgsSVGDiagramFactory::writeXML( QDomNode& overlay_node, QDomDocument& doc ) const
{
  QDomElement factoryElem = doc.createElement( "factory" );
  factoryElem.setAttribute( "type", "svg" );
  //add size units as an attribute to the factory element
  writeSizeUnits( factoryElem, doc );

  QDomElement svgPathElem = doc.createElement( "svgPath" );
  QDomText svgPathText = doc.createTextNode( mSvgFilePath );
  svgPathElem.appendChild( svgPathText );
  factoryElem.appendChild( svgPathElem );
  overlay_node.appendChild( factoryElem );

  return true;
}

bool QgsSVGDiagramFactory::readXML( const QDomNode& factoryNode )
{
  QDomElement factoryElem = factoryNode.toElement();
  if ( factoryElem.isNull() )
  {
    return false;
  }

  //size units
  readSizeUnits( factoryElem );

  //get <svgPath> element
  QDomElement svgPathElem = factoryElem.namedItem( "svgPath" ).toElement();
  if ( svgPathElem.isNull() )
  {
    return false;
  }

  QString svgFilePath = svgPathElem.text();
  if ( !mRenderer.load( svgFilePath ) )
  {
    return false;
  }
  mSvgFilePath = svgFilePath;

  return true;
}



