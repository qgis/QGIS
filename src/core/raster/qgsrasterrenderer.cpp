/***************************************************************************
                         qgsrasterrenderer.cpp
                         ---------------------
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

#include "qgsrasterrenderer.h"
#include "qgsrasterresampler.h"
#include "qgsrasterprojector.h"
#include "qgsrastertransparency.h"
#include "qgsrasterviewport.h"
#include "qgsmaptopixel.h"

//resamplers
#include "qgsbilinearrasterresampler.h"
#include "qgscubicrasterresampler.h"

#include <QCoreApplication>
#include <QDomDocument>
#include <QDomElement>
#include <QImage>
#include <QPainter>

#define tr( sourceText ) QCoreApplication::translate ( "QgsRasterRenderer", sourceText )

// Changing RGB components of NODATA_COLOR may break tests
const QRgb QgsRasterRenderer::NODATA_COLOR = qRgba( 0, 0, 0, 0 );

QgsRasterRenderer::QgsRasterRenderer( QgsRasterInterface* input, const QString& type )
    : QgsRasterInterface( input )
    , mType( type ), mOpacity( 1.0 ), mRasterTransparency( 0 )
    , mAlphaBand( -1 ) //, mInvertColor( false )
{
}

QgsRasterRenderer::~QgsRasterRenderer()
{
}

int QgsRasterRenderer::bandCount() const
{
  if ( mOn ) return 1;

  if ( mInput ) return mInput->bandCount();

  return 0;
}

QGis::DataType QgsRasterRenderer::dataType( int bandNo ) const
{
  QgsDebugMsg( "Entered" );

  if ( mOn ) return QGis::ARGB32_Premultiplied;

  if ( mInput ) return mInput->dataType( bandNo );

  return QGis::UnknownDataType;
}

bool QgsRasterRenderer::setInput( QgsRasterInterface* input )
{
  // Renderer can only work with numerical values in at least 1 band
  if ( !input ) return false;

  if ( !mOn )
  {
    // In off mode we can connect to anything
    mInput = input;
    return true;
  }

  for ( int i = 1; i <= input->bandCount(); i++ )
  {
    if ( !QgsRasterBlock::typeIsNumeric( input->dataType( i ) ) )
    {
      return false;
    }
  }
  mInput = input;
  return true;
}

bool QgsRasterRenderer::usesTransparency( ) const
{
  if ( !mInput )
  {
    return true;
  }
  // TODO: nodata per band
  //return ( mAlphaBand > 0 || ( mRasterTransparency && !mRasterTransparency->isEmpty( mInput->noDataValue( 1 ) ) ) || !doubleNear( mOpacity, 1.0 ) );
  return ( mAlphaBand > 0 || ( mRasterTransparency && !mRasterTransparency->isEmpty() ) || !doubleNear( mOpacity, 1.0 ) );
}

void QgsRasterRenderer::setRasterTransparency( QgsRasterTransparency* t )
{
  delete mRasterTransparency;
  mRasterTransparency = t;
}

void QgsRasterRenderer::_writeXML( QDomDocument& doc, QDomElement& rasterRendererElem ) const
{
  if ( rasterRendererElem.isNull() )
  {
    return;
  }

  rasterRendererElem.setAttribute( "type", mType );
  rasterRendererElem.setAttribute( "opacity", QString::number( mOpacity ) );
  rasterRendererElem.setAttribute( "alphaBand", mAlphaBand );
  //rasterRendererElem.setAttribute( "invertColor", mInvertColor );

  if ( mRasterTransparency )
  {
    mRasterTransparency->writeXML( doc, rasterRendererElem );
  }
}

void QgsRasterRenderer::readXML( const QDomElement& rendererElem )
{
  if ( rendererElem.isNull() )
  {
    return;
  }

  mType = rendererElem.attribute( "type" );
  mOpacity = rendererElem.attribute( "opacity", "1.0" ).toDouble();
  mAlphaBand = rendererElem.attribute( "alphaBand", "-1" ).toInt();
  //mInvertColor = rendererElem.attribute( "invertColor", "0" ).toInt();

  //todo: read mRasterTransparency
  QDomElement rasterTransparencyElem = rendererElem.firstChildElement( "rasterTransparency" );
  if ( !rasterTransparencyElem.isNull() )
  {
    delete mRasterTransparency;
    mRasterTransparency = new QgsRasterTransparency();
    mRasterTransparency->readXML( rasterTransparencyElem );
  }
}

QString QgsRasterRenderer::minMaxOriginName( int theOrigin )
{
  if ( theOrigin == MinMaxUnknown )
  {
    return "Unknown";
  }
  else if ( theOrigin == MinMaxUser )
  {
    return "User";
  }

  QString name;
  if ( theOrigin & MinMaxMinMax )
  {
    name += "MinMax";
  }
  else if ( theOrigin & MinMaxCumulativeCut )
  {
    name += "CumulativeCut";
  }
  else if ( theOrigin & MinMaxStdDev )
  {
    name += "StdDev";
  }

  if ( theOrigin & MinMaxFullExtent )
  {
    name += "FullExtent";
  }
  else if ( theOrigin & MinMaxSubExtent )
  {
    name += "SubExtent";
  }

  if ( theOrigin & MinMaxEstimated )
  {
    name += "Estimated";
  }
  else if ( theOrigin & MinMaxExact )
  {
    name += "Exact";
  }
  return name;
}

QString QgsRasterRenderer::minMaxOriginLabel( int theOrigin )
{
  if ( theOrigin == MinMaxUnknown )
  {
    return tr( "Unknown" );
  }
  else if ( theOrigin == MinMaxUser )
  {
    return tr( "User defined" );
  }

  QString name;
  if ( theOrigin & MinMaxEstimated )
  {
    name += tr( "Estimated" );
  }
  else if ( theOrigin & MinMaxExact )
  {
    name += tr( "Exact" );
  }

  name += " ";

  if ( theOrigin & MinMaxMinMax )
  {
    name += tr( "min / max" );
  }
  else if ( theOrigin & MinMaxCumulativeCut )
  {
    name += "cumulative cut";
  }
  else if ( theOrigin & MinMaxStdDev )
  {
    name += "standard deviation";
  }

  name += " " + tr( " of " ) + " ";

  if ( theOrigin & MinMaxFullExtent )
  {
    name += "full extent";
  }
  else if ( theOrigin & MinMaxSubExtent )
  {
    name += "sub extent";
  }

  name += ".";

  return name;
}

int QgsRasterRenderer::minMaxOriginFromName( QString theName )
{
  if ( theName.contains( "Unknown" ) )
  {
    return MinMaxUnknown;
  }
  else if ( theName.contains( "User" ) )
  {
    return MinMaxUser;
  }

  int origin = 0;

  if ( theName.contains( "MinMax" ) )
  {
    origin |= MinMaxMinMax;
  }
  else if ( theName.contains( "CumulativeCut" ) )
  {
    origin |= MinMaxCumulativeCut;
  }
  else if ( theName.contains( "StdDev" ) )
  {
    origin |= MinMaxStdDev;
  }

  if ( theName.contains( "FullExtent" ) )
  {
    origin |= MinMaxFullExtent;
  }
  else if ( theName.contains( "SubExtent" ) )
  {
    origin |= MinMaxSubExtent;
  }

  if ( theName.contains( "Estimated" ) )
  {
    origin |= MinMaxEstimated;
  }
  else if ( theName.contains( "Exact" ) )
  {
    origin |= MinMaxExact;
  }
  return origin;
}
