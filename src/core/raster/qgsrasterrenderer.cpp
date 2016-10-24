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
#include "qgsrastertransparency.h"

#include <QCoreApplication>
#include <QDomDocument>
#include <QDomElement>
#include <QImage>
#include <QPainter>

// See #9101 before any change of NODATA_COLOR!
const QRgb QgsRasterRenderer::NODATA_COLOR = qRgba( 0, 0, 0, 0 );

QgsRasterRenderer::QgsRasterRenderer( QgsRasterInterface* input, const QString& type )
    : QgsRasterInterface( input )
    , mType( type ), mOpacity( 1.0 ), mRasterTransparency( nullptr )
    , mAlphaBand( -1 ) //, mInvertColor( false )
{
}

QgsRasterRenderer::~QgsRasterRenderer()
{
  delete mRasterTransparency;
}

int QgsRasterRenderer::bandCount() const
{
  if ( mOn ) return 1;

  if ( mInput ) return mInput->bandCount();

  return 0;
}

Qgis::DataType QgsRasterRenderer::dataType( int bandNo ) const
{
  QgsDebugMsgLevel( "Entered", 4 );

  if ( mOn ) return Qgis::ARGB32_Premultiplied;

  if ( mInput ) return mInput->dataType( bandNo );

  return Qgis::UnknownDataType;
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

bool QgsRasterRenderer::usesTransparency() const
{
  if ( !mInput )
  {
    return true;
  }
  return ( mAlphaBand > 0 || ( mRasterTransparency && !mRasterTransparency->isEmpty() ) || !qgsDoubleNear( mOpacity, 1.0 ) );
}

void QgsRasterRenderer::setRasterTransparency( QgsRasterTransparency* t )
{
  delete mRasterTransparency;
  mRasterTransparency = t;
}

void QgsRasterRenderer::_writeXml( QDomDocument& doc, QDomElement& rasterRendererElem ) const
{
  if ( rasterRendererElem.isNull() )
  {
    return;
  }

  rasterRendererElem.setAttribute( QStringLiteral( "type" ), mType );
  rasterRendererElem.setAttribute( QStringLiteral( "opacity" ), QString::number( mOpacity ) );
  rasterRendererElem.setAttribute( QStringLiteral( "alphaBand" ), mAlphaBand );

  if ( mRasterTransparency )
  {
    mRasterTransparency->writeXml( doc, rasterRendererElem );
  }
}

void QgsRasterRenderer::readXml( const QDomElement& rendererElem )
{
  if ( rendererElem.isNull() )
  {
    return;
  }

  mType = rendererElem.attribute( QStringLiteral( "type" ) );
  mOpacity = rendererElem.attribute( QStringLiteral( "opacity" ), QStringLiteral( "1.0" ) ).toDouble();
  mAlphaBand = rendererElem.attribute( QStringLiteral( "alphaBand" ), QStringLiteral( "-1" ) ).toInt();

  QDomElement rasterTransparencyElem = rendererElem.firstChildElement( QStringLiteral( "rasterTransparency" ) );
  if ( !rasterTransparencyElem.isNull() )
  {
    delete mRasterTransparency;
    mRasterTransparency = new QgsRasterTransparency();
    mRasterTransparency->readXml( rasterTransparencyElem );
  }
}

void QgsRasterRenderer::copyCommonProperties( const QgsRasterRenderer* other )
{
  if ( !other )
    return;

  setOpacity( other->opacity() );
  setAlphaBand( other->alphaBand() );
  setRasterTransparency( other->rasterTransparency() ? new QgsRasterTransparency( *other->rasterTransparency() ) : nullptr );
}

QString QgsRasterRenderer::minMaxOriginName( int theOrigin )
{
  if ( theOrigin == MinMaxUnknown )
  {
    return QStringLiteral( "Unknown" );
  }
  else if ( theOrigin == MinMaxUser )
  {
    return QStringLiteral( "User" );
  }

  QString name;
  if ( theOrigin & MinMaxMinMax )
  {
    name += QLatin1String( "MinMax" );
  }
  else if ( theOrigin & MinMaxCumulativeCut )
  {
    name += QLatin1String( "CumulativeCut" );
  }
  else if ( theOrigin & MinMaxStdDev )
  {
    name += QLatin1String( "StdDev" );
  }

  if ( theOrigin & MinMaxFullExtent )
  {
    name += QLatin1String( "FullExtent" );
  }
  else if ( theOrigin & MinMaxSubExtent )
  {
    name += QLatin1String( "SubExtent" );
  }

  if ( theOrigin & MinMaxEstimated )
  {
    name += QLatin1String( "Estimated" );
  }
  else if ( theOrigin & MinMaxExact )
  {
    name += QLatin1String( "Exact" );
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

  QString label;
  QString est_exact;
  QString values;
  QString extent;

  if ( theOrigin & MinMaxEstimated )
  {
    est_exact = tr( "Estimated" );
  }
  else if ( theOrigin & MinMaxExact )
  {
    est_exact = tr( "Exact" );
  }

  if ( theOrigin & MinMaxMinMax )
  {
    values = tr( "min / max" );
  }
  else if ( theOrigin & MinMaxCumulativeCut )
  {
    values = tr( "cumulative cut" );
  }
  else if ( theOrigin & MinMaxStdDev )
  {
    values = tr( "standard deviation" );
  }

  if ( theOrigin & MinMaxFullExtent )
  {
    extent = tr( "full extent" );
  }
  else if ( theOrigin & MinMaxSubExtent )
  {
    extent = tr( "sub extent" );
  }

  label = QCoreApplication::translate( "QgsRasterRenderer", "%1 %2 of %3.",
                                       "min/max origin label in raster properties, where %1 - estimated/exact, %2 - values (min/max, stddev, etc.), %3 - extent" )
          .arg( est_exact,
                values,
                extent );
  return label;
}

int QgsRasterRenderer::minMaxOriginFromName( const QString& theName )
{
  if ( theName.contains( QLatin1String( "Unknown" ) ) )
  {
    return MinMaxUnknown;
  }
  else if ( theName.contains( QLatin1String( "User" ) ) )
  {
    return MinMaxUser;
  }

  int origin = 0;

  if ( theName.contains( QLatin1String( "MinMax" ) ) )
  {
    origin |= MinMaxMinMax;
  }
  else if ( theName.contains( QLatin1String( "CumulativeCut" ) ) )
  {
    origin |= MinMaxCumulativeCut;
  }
  else if ( theName.contains( QLatin1String( "StdDev" ) ) )
  {
    origin |= MinMaxStdDev;
  }

  if ( theName.contains( QLatin1String( "FullExtent" ) ) )
  {
    origin |= MinMaxFullExtent;
  }
  else if ( theName.contains( QLatin1String( "SubExtent" ) ) )
  {
    origin |= MinMaxSubExtent;
  }

  if ( theName.contains( QLatin1String( "Estimated" ) ) )
  {
    origin |= MinMaxEstimated;
  }
  else if ( theName.contains( QLatin1String( "Exact" ) ) )
  {
    origin |= MinMaxExact;
  }
  return origin;
}
