/***************************************************************************
                         qgspalettedrasterrenderer.cpp
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

#include "qgspalettedrasterrenderer.h"
#include "qgsrastertransparency.h"
#include "qgsrasterviewport.h"

#include <QColor>
#include <QDomDocument>
#include <QDomElement>
#include <QImage>
#include <QVector>
#include <memory>

QgsPalettedRasterRenderer::QgsPalettedRasterRenderer( QgsRasterInterface *input, int bandNumber, const ClassData &classes )
  : QgsRasterRenderer( input, QStringLiteral( "paletted" ) )
  , mBand( bandNumber )
  , mClassData( classes )
{
  updateArrays();
}

QgsPalettedRasterRenderer::~QgsPalettedRasterRenderer()
{
  delete[] mColors;
  delete[] mIsNoData;
}

QgsPalettedRasterRenderer *QgsPalettedRasterRenderer::clone() const
{
  QgsPalettedRasterRenderer *renderer = new QgsPalettedRasterRenderer( nullptr, mBand, mClassData );
  renderer->copyCommonProperties( this );
  return renderer;
}

QgsRasterRenderer *QgsPalettedRasterRenderer::create( const QDomElement &elem, QgsRasterInterface *input )
{
  if ( elem.isNull() )
  {
    return nullptr;
  }

  int bandNumber = elem.attribute( QStringLiteral( "band" ), QStringLiteral( "-1" ) ).toInt();
  ClassData classData;

  QDomElement paletteElem = elem.firstChildElement( QStringLiteral( "colorPalette" ) );
  if ( !paletteElem.isNull() )
  {
    QDomNodeList paletteEntries = paletteElem.elementsByTagName( QStringLiteral( "paletteEntry" ) );

    QDomElement entryElem;
    int value;

    for ( int i = 0; i < paletteEntries.size(); ++i )
    {
      QColor color;
      QString label;
      entryElem = paletteEntries.at( i ).toElement();
      value = ( int )entryElem.attribute( QStringLiteral( "value" ), QStringLiteral( "0" ) ).toDouble();
      QgsDebugMsgLevel( entryElem.attribute( "color", "#000000" ), 4 );
      color = QColor( entryElem.attribute( QStringLiteral( "color" ), QStringLiteral( "#000000" ) ) );
      color.setAlpha( entryElem.attribute( QStringLiteral( "alpha" ), QStringLiteral( "255" ) ).toInt() );
      label = entryElem.attribute( QStringLiteral( "label" ) );
      classData.insert( value, Class( color, label ) );
    }
  }
  QgsPalettedRasterRenderer *r = new QgsPalettedRasterRenderer( input, bandNumber, classData );
  r->readXml( elem );
  return r;
}

QgsPalettedRasterRenderer::ClassData QgsPalettedRasterRenderer::classes() const
{
  return mClassData;
}

QString QgsPalettedRasterRenderer::label( int idx ) const
{
  if ( !mClassData.contains( idx ) )
    return QString();

  return mClassData.value( idx ).label;
}

void QgsPalettedRasterRenderer::setLabel( int idx, const QString &label )
{
  if ( !mClassData.contains( idx ) )
    return;

  mClassData[ idx ].label = label;
}

QgsRasterBlock *QgsPalettedRasterRenderer::block( int bandNo, QgsRectangle  const &extent, int width, int height, QgsRasterBlockFeedback *feedback )
{
  std::unique_ptr< QgsRasterBlock > outputBlock( new QgsRasterBlock() );
  if ( !mInput || mClassData.isEmpty() )
  {
    return outputBlock.release();
  }

  std::shared_ptr< QgsRasterBlock > inputBlock( mInput->block( bandNo, extent, width, height, feedback ) );

  if ( !inputBlock || inputBlock->isEmpty() )
  {
    QgsDebugMsg( "No raster data!" );
    return outputBlock.release();
  }

  double currentOpacity = mOpacity;

  //rendering is faster without considering user-defined transparency
  bool hasTransparency = usesTransparency();

  std::shared_ptr< QgsRasterBlock > alphaBlock;

  if ( mAlphaBand > 0 && mAlphaBand != mBand )
  {
    alphaBlock.reset( mInput->block( mAlphaBand, extent, width, height, feedback ) );
    if ( !alphaBlock || alphaBlock->isEmpty() )
    {
      return outputBlock.release();
    }
  }
  else if ( mAlphaBand == mBand )
  {
    alphaBlock = inputBlock;
  }

  if ( !outputBlock->reset( Qgis::ARGB32_Premultiplied, width, height ) )
  {
    return outputBlock.release();
  }

  QRgb myDefaultColor = NODATA_COLOR;

  //use direct data access instead of QgsRasterBlock::setValue
  //because of performance
  unsigned int *outputData = ( unsigned int * )( outputBlock->bits() );

  qgssize rasterSize = ( qgssize )width * height;
  for ( qgssize i = 0; i < rasterSize; ++i )
  {
    if ( inputBlock->isNoData( i ) )
    {
      outputData[i] = myDefaultColor;
      continue;
    }
    int val = ( int ) inputBlock->value( i );
    if ( val > mMaxColorIndex || mIsNoData[ val ] )
    {
      outputData[i] = myDefaultColor;
      continue;
    }

    if ( !hasTransparency )
    {
      outputData[i] = mColors[val];
    }
    else
    {
      currentOpacity = mOpacity;
      if ( mRasterTransparency )
      {
        currentOpacity = mRasterTransparency->alphaValue( val, mOpacity * 255 ) / 255.0;
      }
      if ( mAlphaBand > 0 )
      {
        currentOpacity *= alphaBlock->value( i ) / 255.0;
      }

      QRgb c = mColors[val];
      outputData[i] = qRgba( currentOpacity * qRed( c ), currentOpacity * qGreen( c ), currentOpacity * qBlue( c ), currentOpacity * qAlpha( c ) );
    }
  }

  return outputBlock.release();
}

void QgsPalettedRasterRenderer::writeXml( QDomDocument &doc, QDomElement &parentElem ) const
{
  if ( parentElem.isNull() )
  {
    return;
  }

  QDomElement rasterRendererElem = doc.createElement( QStringLiteral( "rasterrenderer" ) );
  _writeXml( doc, rasterRendererElem );

  rasterRendererElem.setAttribute( QStringLiteral( "band" ), mBand );
  QDomElement colorPaletteElem = doc.createElement( QStringLiteral( "colorPalette" ) );
  ClassData::const_iterator it = mClassData.constBegin();
  for ( ; it != mClassData.constEnd(); ++it )
  {
    QColor color = it.value().color;
    QDomElement colorElem = doc.createElement( QStringLiteral( "paletteEntry" ) );
    colorElem.setAttribute( QStringLiteral( "value" ), it.key() );
    colorElem.setAttribute( QStringLiteral( "color" ), color.name() );
    colorElem.setAttribute( QStringLiteral( "alpha" ), color.alpha() );
    if ( !it.value().label.isEmpty() )
    {
      colorElem.setAttribute( QStringLiteral( "label" ), it.value().label );
    }
    colorPaletteElem.appendChild( colorElem );
  }
  rasterRendererElem.appendChild( colorPaletteElem );

  parentElem.appendChild( rasterRendererElem );
}

void QgsPalettedRasterRenderer::legendSymbologyItems( QList< QPair< QString, QColor > > &symbolItems ) const
{
  QList< QPair< int, QPair< QString, QColor > > > legends;
  ClassData::const_iterator it = mClassData.constBegin();
  for ( ; it != mClassData.constEnd(); ++it )
  {
    QString lab = it.value().label.isEmpty() ? QString::number( it.key() ) : it.value().label;
    legends.push_back( qMakePair( it.key(), qMakePair( lab, it.value().color ) ) );
  }

  // sort by color value
  std::sort( legends.begin(), legends.end(),
             []( const QPair< int, QPair< QString, QColor > > &a, const QPair< int, QPair< QString, QColor > > &b ) -> bool
  {
    return a.first < b.first;
  } );

  QList< QPair< int, QPair< QString, QColor > > >::const_iterator lIt = legends.constBegin();
  for ( ; lIt != legends.constEnd(); ++lIt )
  {
    symbolItems << lIt->second;
  }
}

QList<int> QgsPalettedRasterRenderer::usesBands() const
{
  QList<int> bandList;
  if ( mBand != -1 )
  {
    bandList << mBand;
  }
  return bandList;
}

void QgsPalettedRasterRenderer::updateArrays()
{
  // find maximum color index
  ClassData::const_iterator mIt = mClassData.constBegin();
  for ( ; mIt != mClassData.constEnd(); ++mIt )
  {
    mMaxColorIndex = qMax( mMaxColorIndex, mIt.key() );
  }

  delete [] mColors;
  delete [] mIsNoData;
  mColors = new QRgb[mMaxColorIndex + 1];
  mIsNoData = new bool[mMaxColorIndex + 1];
  std::fill( mIsNoData, mIsNoData + mMaxColorIndex, true );

  int i = 0;
  ClassData::const_iterator it = mClassData.constBegin();
  for ( ; it != mClassData.constEnd(); ++it )
  {
    mColors[it.key()] = qPremultiply( it.value().color.rgba() );
    mIsNoData[it.key()] = false;
    i++;
  }
}
