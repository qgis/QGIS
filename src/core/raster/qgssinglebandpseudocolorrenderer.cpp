/***************************************************************************
                         qgssinglebandpseudocolorrenderer.cpp
                         ------------------------------------
    begin                : January 2012
    copyright            : (C) 2012 by Marco Hugentobler
    email                : marco at sourcepole dot ch
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgssinglebandpseudocolorrenderer.h"
#include "qgsrastershader.h"
#include "qgsrastertransparency.h"
#include "qgsrasterviewport.h"
#include <QDomDocument>
#include <QDomElement>
#include <QImage>

QgsSingleBandPseudoColorRenderer::QgsSingleBandPseudoColorRenderer( QgsRasterInterface* input, int band, QgsRasterShader* shader ):
    QgsRasterRenderer( input, "singlebandpseudocolor" ), mShader( shader ), mBand( band )
{
}

QgsSingleBandPseudoColorRenderer::~QgsSingleBandPseudoColorRenderer()
{
  delete mShader;
}

void QgsSingleBandPseudoColorRenderer::setShader( QgsRasterShader* shader )
{
  delete mShader;
  mShader = shader;
}

QgsRasterRenderer* QgsSingleBandPseudoColorRenderer::create( const QDomElement& elem, QgsRasterInterface* input )
{
  if ( elem.isNull() )
  {
    return 0;
  }

  int band = elem.attribute( "band", "-1" ).toInt();
  QgsRasterShader* shader = 0;
  QDomElement rasterShaderElem = elem.firstChildElement( "rastershader" );
  if ( !rasterShaderElem.isNull() )
  {
    shader = new QgsRasterShader();
    shader->readXML( rasterShaderElem );
  }
  QgsRasterRenderer* r = new QgsSingleBandPseudoColorRenderer( input, band, shader );
  r->readXML( elem );
  return r;
}

void * QgsSingleBandPseudoColorRenderer::readBlock( int bandNo, QgsRectangle  const & extent, int width, int height )
{
  Q_UNUSED( bandNo );
  if ( !mInput || !mShader )
  {
    return 0;
  }

  QgsRasterInterface::DataType transparencyType = QgsRasterInterface::UnknownDataType;
  if ( mAlphaBand > 0 )
  {
    transparencyType = ( QgsRasterInterface::DataType )mInput->dataType( mAlphaBand );
  }

  void* transparencyData = 0;
  double currentOpacity = mOpacity;
  QgsRasterInterface::DataType rasterType = ( QgsRasterInterface::DataType )mInput->dataType( mBand );

  void* rasterData = mInput->block( mBand, extent, width, height );

  int red, green, blue;
  QRgb myDefaultColor = qRgba( 255, 255, 255, 0 );

  //rendering is faster without considering user-defined transparency
  bool hasTransparency = usesTransparency();

  if ( mAlphaBand > 0 && mAlphaBand != mBand )
  {
    transparencyData = mInput->block( mAlphaBand, extent, width, height );
  }
  else if ( mAlphaBand == mBand )
  {
    transparencyData = rasterData;
  }

  //create image
  QImage img( width, height, QImage::Format_ARGB32_Premultiplied );
  QRgb* imageScanLine = 0;
  double val = 0;

  int currentRasterPos = 0;
  for ( int i = 0; i < height; ++i )
  {
    imageScanLine = ( QRgb* )( img.scanLine( i ) );
    for ( int j = 0; j < width; ++j )
    {
      val = readValue( rasterData, rasterType, currentRasterPos );
      if ( !mShader->shade( val, &red, &green, &blue ) )
      {
        imageScanLine[j] = myDefaultColor;
        ++currentRasterPos;
        continue;
      }

      if ( !hasTransparency )
      {
        imageScanLine[j] = qRgba( red, green, blue, 255 );
      }
      else
      {
        //opacity
        currentOpacity = mOpacity;
        if ( mRasterTransparency )
        {
          currentOpacity = mRasterTransparency->alphaValue( val, mOpacity * 255 ) / 255.0;
        }
        if ( mAlphaBand > 0 )
        {
          currentOpacity *= ( readValue( transparencyData, transparencyType, currentRasterPos ) / 255.0 );
        }

        imageScanLine[j] = qRgba( currentOpacity * red, currentOpacity * green, currentOpacity * blue, currentOpacity * 255 );
      }
      ++currentRasterPos;
    }
  }

  VSIFree( rasterData );

  void * data = VSIMalloc( img.byteCount() );
  return memcpy( data, img.bits(), img.byteCount() );
}

void QgsSingleBandPseudoColorRenderer::writeXML( QDomDocument& doc, QDomElement& parentElem ) const
{
  if ( parentElem.isNull() )
  {
    return;
  }

  QDomElement rasterRendererElem = doc.createElement( "rasterrenderer" );
  _writeXML( doc, rasterRendererElem );
  rasterRendererElem.setAttribute( "band", mBand );
  if ( mShader )
  {
    mShader->writeXML( doc, rasterRendererElem ); //todo: include color ramp items directly in this renderer
  }
  parentElem.appendChild( rasterRendererElem );
}

void QgsSingleBandPseudoColorRenderer::legendSymbologyItems( QList< QPair< QString, QColor > >& symbolItems ) const
{
  if ( mShader )
  {
    QgsRasterShaderFunction* shaderFunction = mShader->rasterShaderFunction();
    if ( shaderFunction )
    {
      shaderFunction->legendSymbologyItems( symbolItems );
    }
  }
}

QList<int> QgsSingleBandPseudoColorRenderer::usesBands() const
{
  QList<int> bandList;
  if ( mBand != -1 )
  {
    bandList << mBand;
  }
  return bandList;
}
