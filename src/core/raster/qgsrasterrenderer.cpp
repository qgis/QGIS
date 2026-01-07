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

#include <memory>

#include "qgscolorutils.h"
#include "qgslayertreemodellegendnode.h"
#include "qgsrastertransparency.h"
#include "qgssldexportcontext.h"

#include <QCoreApplication>
#include <QDomDocument>
#include <QDomElement>
#include <QImage>
#include <QPainter>

// See #9101 before any change of NODATA_COLOR!
const QRgb QgsRasterRenderer::NODATA_COLOR = qRgba( 0, 0, 0, 0 );

QgsRasterRenderer::QgsRasterRenderer( QgsRasterInterface *input, const QString &type )
  : QgsRasterInterface( input )
  , mType( type )
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

Qgis::DataType QgsRasterRenderer::dataType( int bandNo ) const
{
  QgsDebugMsgLevel( u"Entered"_s, 4 );

  if ( mOn ) return Qgis::DataType::ARGB32_Premultiplied;

  if ( mInput ) return mInput->dataType( bandNo );

  return Qgis::DataType::UnknownDataType;
}

Qgis::RasterRendererFlags QgsRasterRenderer::flags() const
{
  return Qgis::RasterRendererFlags();
}

bool QgsRasterRenderer::canCreateRasterAttributeTable() const
{
  return false;
}

bool QgsRasterRenderer::setInput( QgsRasterInterface *input )
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
    const Qgis::DataType bandType = input->dataType( i );
    // we always allow unknown data types to connect - otherwise invalid layers cannot setup
    // their original rendering pipe and this information is lost
    if ( bandType != Qgis::DataType::UnknownDataType && !QgsRasterBlock::typeIsNumeric( bandType ) )
    {
      return false;
    }
  }
  mInput = input;
  return true;
}

int QgsRasterRenderer::inputBand() const
{
  return -1;
}

bool QgsRasterRenderer::setInputBand( int )
{
  return false;
}

bool QgsRasterRenderer::usesTransparency() const
{
  if ( !mInput )
  {
    return true;
  }
  return ( mAlphaBand > 0 || ( mRasterTransparency && !mRasterTransparency->isEmpty() ) || !qgsDoubleNear( mOpacity, 1.0 ) );
}

void QgsRasterRenderer::setRasterTransparency( QgsRasterTransparency *t )
{
  mRasterTransparency.reset( t );

}

QList< QPair< QString, QColor > > QgsRasterRenderer::legendSymbologyItems() const
{
  return QList< QPair< QString, QColor > >();
}

QList<QgsLayerTreeModelLegendNode *> QgsRasterRenderer::createLegendNodes( QgsLayerTreeLayer *nodeLayer )
{
  QList<QgsLayerTreeModelLegendNode *> nodes;

  const QList< QPair< QString, QColor > > rasterItemList = legendSymbologyItems();
  if ( rasterItemList.isEmpty() )
    return nodes;

  // Paletted raster may have many colors, for example UInt16 may have 65536 colors
  // and it is very slow, so we limit max count
  int count = 0;
  const int max_count = 1000;

  for ( auto itemIt = rasterItemList.constBegin(); itemIt != rasterItemList.constEnd(); ++itemIt, ++count )
  {
    nodes << new QgsRasterSymbolLegendNode( nodeLayer, itemIt->second, itemIt->first );

    if ( count == max_count )
    {
      const QString label = tr( "following %n item(s) not displayed", nullptr, rasterItemList.size() - max_count );
      nodes << new QgsSimpleLegendNode( nodeLayer, label );
      break;
    }
  }

  return nodes;
}

void QgsRasterRenderer::_writeXml( QDomDocument &doc, QDomElement &rasterRendererElem ) const
{
  if ( rasterRendererElem.isNull() )
  {
    return;
  }

  rasterRendererElem.setAttribute( u"type"_s, mType );
  rasterRendererElem.setAttribute( u"opacity"_s, QString::number( mOpacity ) );
  rasterRendererElem.setAttribute( u"alphaBand"_s, mAlphaBand );
  rasterRendererElem.setAttribute( u"nodataColor"_s, mNodataColor.isValid() ? QgsColorUtils::colorToString( mNodataColor ) : QString() );

  if ( mRasterTransparency )
  {
    mRasterTransparency->writeXml( doc, rasterRendererElem );
  }

  QDomElement minMaxOriginElem = doc.createElement( u"minMaxOrigin"_s );
  mMinMaxOrigin.writeXml( doc, minMaxOriginElem );
  rasterRendererElem.appendChild( minMaxOriginElem );
}

QRgb QgsRasterRenderer::renderColorForNodataPixel() const
{
  if ( !mNodataColor.isValid() )
    return NODATA_COLOR;
  else
    return mNodataColor.rgba();
}

void QgsRasterRenderer::readXml( const QDomElement &rendererElem )
{
  if ( rendererElem.isNull() )
  {
    return;
  }

  mType = rendererElem.attribute( u"type"_s );
  mOpacity = rendererElem.attribute( u"opacity"_s, u"1.0"_s ).toDouble();
  mAlphaBand = rendererElem.attribute( u"alphaBand"_s, u"-1"_s ).toInt();
  const QString colorEncoded = rendererElem.attribute( u"nodataColor"_s );
  mNodataColor = !colorEncoded.isEmpty() ? QgsColorUtils::colorFromString( colorEncoded ) : QColor();

  const QDomElement rasterTransparencyElem = rendererElem.firstChildElement( u"rasterTransparency"_s );
  if ( !rasterTransparencyElem.isNull() )
  {
    mRasterTransparency = std::make_unique<QgsRasterTransparency>( );

    mRasterTransparency->readXml( rasterTransparencyElem );
  }

  const QDomElement minMaxOriginElem = rendererElem.firstChildElement( u"minMaxOrigin"_s );
  if ( !minMaxOriginElem.isNull() )
  {
    mMinMaxOrigin.readXml( minMaxOriginElem );
  }
}

void QgsRasterRenderer::copyCommonProperties( const QgsRasterRenderer *other, bool copyMinMaxOrigin )
{
  if ( !other )
    return;

  setOpacity( other->opacity() );
  setAlphaBand( other->alphaBand() );
  setRasterTransparency( other->rasterTransparency() ? new QgsRasterTransparency( *other->rasterTransparency() ) : nullptr );
  setNodataColor( other->nodataColor() );
  if ( copyMinMaxOrigin )
    setMinMaxOrigin( other->minMaxOrigin() );
}

void QgsRasterRenderer::toSld( QDomDocument &doc, QDomElement &element, const QVariantMap &props ) const
{
  QgsSldExportContext context;
  context.setExtraProperties( props );
  toSld( doc, element, context );
}

bool QgsRasterRenderer::toSld( QDomDocument &doc, QDomElement &element, QgsSldExportContext & ) const
{
  QDomElement rasterSymbolizerElem = doc.createElement( u"sld:RasterSymbolizer"_s );
  element.appendChild( rasterSymbolizerElem );

  // add opacity only is different from default
  if ( !qgsDoubleNear( opacity(), 1.0 ) )
  {
    QDomElement opacityElem = doc.createElement( u"sld:Opacity"_s );
    opacityElem.appendChild( doc.createTextNode( QString::number( opacity() ) ) );
    rasterSymbolizerElem.appendChild( opacityElem );
  }
  return true;
}

bool QgsRasterRenderer::accept( QgsStyleEntityVisitorInterface * ) const
{
  return true;
}

bool QgsRasterRenderer::needsRefresh( const QgsRectangle &extent ) const
{
  if ( mLastRectangleUsedByRefreshContrastEnhancementIfNeeded != extent &&
       mMinMaxOrigin.limits() != Qgis::RasterRangeLimit::NotSet &&
       mMinMaxOrigin.extent() == Qgis::RasterRangeExtent::UpdatedCanvas )
  {
    return true;
  }

  return false;
}

bool QgsRasterRenderer::refresh( const QgsRectangle &extent, const QList<double> &, const QList<double> &, bool forceRefresh )
{
  if ( !needsRefresh( extent ) && !forceRefresh )
  {
    return false;
  }

  mLastRectangleUsedByRefreshContrastEnhancementIfNeeded = extent;
  return true;
}
