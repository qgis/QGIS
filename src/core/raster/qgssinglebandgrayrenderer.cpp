/***************************************************************************
                         qgssinglebandgrayrenderer.cpp
                         -----------------------------
    begin                : December 2011
    copyright            : (C) 2011 by Marco Hugentobler
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

#include "qgssinglebandgrayrenderer.h"
#include "qgscontrastenhancement.h"
#include "qgsrastertransparency.h"
#include <QDomDocument>
#include <QDomElement>
#include <QImage>

QgsSingleBandGrayRenderer::QgsSingleBandGrayRenderer( QgsRasterInterface* input, int grayBand ):
    QgsRasterRenderer( input, "singlebandgray" ), mGrayBand( grayBand ), mContrastEnhancement( 0 )
{
}

QgsSingleBandGrayRenderer::~QgsSingleBandGrayRenderer()
{
  delete mContrastEnhancement;
}

QgsRasterRenderer* QgsSingleBandGrayRenderer::create( const QDomElement& elem, QgsRasterInterface* input )
{
  if ( elem.isNull() )
  {
    return 0;
  }

  int grayBand = elem.attribute( "grayBand", "-1" ).toInt();
  QgsSingleBandGrayRenderer* r = new QgsSingleBandGrayRenderer( input, grayBand );
  r->readXML( elem );

  QDomElement contrastEnhancementElem = elem.firstChildElement( "contrastEnhancement" );
  if ( !contrastEnhancementElem.isNull() )
  {
    QgsContrastEnhancement* ce = new QgsContrastEnhancement(( QgsContrastEnhancement::QgsRasterDataType )(
          input->dataType( grayBand ) ) ) ;
    ce->readXML( contrastEnhancementElem );
    r->setContrastEnhancement( ce );
  }
  return r;
}

void QgsSingleBandGrayRenderer::setContrastEnhancement( QgsContrastEnhancement* ce )
{
  delete mContrastEnhancement;
  mContrastEnhancement = ce;
}

void * QgsSingleBandGrayRenderer::readBlock( int bandNo, QgsRectangle  const & extent, int width, int height )
{
  Q_UNUSED( bandNo );
  if ( !mInput )
  {
    return 0;
  }

  QgsRasterInterface::DataType rasterType = ( QgsRasterInterface::DataType )mInput->dataType( mGrayBand );
  QgsRasterInterface::DataType alphaType = QgsRasterInterface::UnknownDataType;
  if ( mAlphaBand > 0 )
  {
    alphaType = ( QgsRasterInterface::DataType )mInput->dataType( mAlphaBand );
  }

  void* rasterData = mInput->block( mGrayBand, extent, width, height );
  if ( !rasterData ) return 0;

  void* alphaData = 0;
  double currentAlpha = mOpacity;
  int grayVal;
  QRgb myDefaultColor = qRgba( 0, 0, 0, 0 );

  if ( mAlphaBand > 0 && mGrayBand != mAlphaBand )
  {
    alphaData = mInput->block( mAlphaBand, extent, width, height );
    if ( !alphaData )
    {
      free( rasterData );
      return 0;
    }
  }
  else if ( mAlphaBand > 0 )
  {
    alphaData = rasterData;
  }

  QImage *img = createImage( width, height, QImage::Format_ARGB32_Premultiplied );
  QRgb* imageScanLine = 0;
  int currentRasterPos = 0;

  for ( int i = 0; i < height; ++i )
  {
    imageScanLine = ( QRgb* )( img->scanLine( i ) );
    for ( int j = 0; j < width; ++j )
    {
      grayVal = readValue( rasterData, rasterType, currentRasterPos );

      //alpha
      currentAlpha = mOpacity;
      if ( mRasterTransparency )
      {
        currentAlpha = mRasterTransparency->alphaValue( grayVal, mOpacity * 255 ) / 255.0;
      }
      if ( mAlphaBand > 0 )
      {
        currentAlpha *= ( readValue( alphaData, alphaType, currentRasterPos ) / 255.0 );
      }

      if ( mContrastEnhancement )
      {
        if ( !mContrastEnhancement->isValueInDisplayableRange( grayVal ) )
        {
          imageScanLine[ j ] = myDefaultColor;
          ++currentRasterPos;
          continue;
        }
        grayVal = mContrastEnhancement->enhanceContrast( grayVal );
      }

      if ( mInvertColor )
      {
        grayVal = 255 - grayVal;
      }

      if ( doubleNear( currentAlpha, 1.0 ) )
      {
        imageScanLine[j] = qRgba( grayVal, grayVal, grayVal, 255 );
      }
      else
      {
        imageScanLine[j] = qRgba( currentAlpha * grayVal, currentAlpha * grayVal, currentAlpha * grayVal, currentAlpha * 255 );
      }
      ++currentRasterPos;
    }
  }

  free( rasterData );
  if ( mAlphaBand > 0 && mGrayBand != mAlphaBand )
  {
    free( alphaData );
  }

  void * data = ( void * )img->bits();
  delete img;
  return data; // OK, the image was created with extraneous data
}

void QgsSingleBandGrayRenderer::writeXML( QDomDocument& doc, QDomElement& parentElem ) const
{
  if ( parentElem.isNull() )
  {
    return;
  }

  QDomElement rasterRendererElem = doc.createElement( "rasterrenderer" );
  _writeXML( doc, rasterRendererElem );

  rasterRendererElem.setAttribute( "grayBand", mGrayBand );
  if ( mContrastEnhancement )
  {
    QDomElement contrastElem = doc.createElement( "contrastEnhancement" );
    mContrastEnhancement->writeXML( doc, contrastElem );
    rasterRendererElem.appendChild( contrastElem );
  }
  parentElem.appendChild( rasterRendererElem );
}

void QgsSingleBandGrayRenderer::legendSymbologyItems( QList< QPair< QString, QColor > >& symbolItems ) const
{
  if ( mContrastEnhancement && mContrastEnhancement->contrastEnhancementAlgorithm() != QgsContrastEnhancement::NoEnhancement )
  {
    symbolItems.push_back( qMakePair( QString::number( mContrastEnhancement->minimumValue() ), QColor( 0, 0, 0 ) ) );
    symbolItems.push_back( qMakePair( QString::number( mContrastEnhancement->maximumValue() ), QColor( 255, 255, 255 ) ) );
  }
}

QList<int> QgsSingleBandGrayRenderer::usesBands() const
{
  QList<int> bandList;
  if ( mGrayBand != -1 )
  {
    bandList << mGrayBand;
  }
  return bandList;
}
