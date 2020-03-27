/***************************************************************************
  qgsvectortilebasicrenderer.cpp
  --------------------------------------
  Date                 : March 2020
  Copyright            : (C) 2020 by Martin Dobias
  Email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsvectortilebasicrenderer.h"

#include "qgsapplication.h"
#include "qgscolorschemeregistry.h"
#include "qgsexpressioncontextutils.h"
#include "qgsfillsymbollayer.h"
#include "qgslinesymbollayer.h"
#include "qgsmarkersymbollayer.h"
#include "qgssymbollayerutils.h"
#include "qgsvectortileutils.h"


QgsVectorTileBasicRendererStyle::QgsVectorTileBasicRendererStyle( const QString &stName, const QString &laName, QgsWkbTypes::GeometryType geomType )
  : mStyleName( stName )
  , mLayerName( laName )
  , mGeometryType( geomType )
{
}

QgsVectorTileBasicRendererStyle::QgsVectorTileBasicRendererStyle( const QgsVectorTileBasicRendererStyle &other )
{
  operator=( other );
}

QgsVectorTileBasicRendererStyle &QgsVectorTileBasicRendererStyle::operator=( const QgsVectorTileBasicRendererStyle &other )
{
  mStyleName = other.mStyleName;
  mLayerName = other.mLayerName;
  mGeometryType = other.mGeometryType;
  mSymbol.reset( other.mSymbol ? other.mSymbol->clone() : nullptr );
  mEnabled = other.mEnabled;
  mExpression = other.mExpression;
  mMinZoomLevel = other.mMinZoomLevel;
  mMaxZoomLevel = other.mMaxZoomLevel;
  return *this;
}

QgsVectorTileBasicRendererStyle::~QgsVectorTileBasicRendererStyle() = default;

void QgsVectorTileBasicRendererStyle::setSymbol( QgsSymbol *sym )
{
  mSymbol.reset( sym );
}

void QgsVectorTileBasicRendererStyle::writeXml( QDomElement &elem, const QgsReadWriteContext &context ) const
{
  elem.setAttribute( "name", mStyleName );
  elem.setAttribute( "layer", mLayerName );
  elem.setAttribute( "geometry", mGeometryType );
  elem.setAttribute( "enabled", mEnabled ? "1" : "0" );
  elem.setAttribute( "expression", mExpression );
  elem.setAttribute( "min-zoom", mMinZoomLevel );
  elem.setAttribute( "max-zoom", mMaxZoomLevel );

  QDomDocument doc = elem.ownerDocument();
  QgsSymbolMap symbols;
  symbols[QStringLiteral( "0" )] = mSymbol.get();
  QDomElement symbolsElem = QgsSymbolLayerUtils::saveSymbols( symbols, QStringLiteral( "symbols" ), doc, context );
  elem.appendChild( symbolsElem );
}

void QgsVectorTileBasicRendererStyle::readXml( const QDomElement &elem, const QgsReadWriteContext &context )
{
  mStyleName = elem.attribute( "name" );
  mLayerName = elem.attribute( "layer" );
  mGeometryType = static_cast<QgsWkbTypes::GeometryType>( elem.attribute( "geometry" ).toInt() );
  mEnabled = elem.attribute( "enabled" ).toInt();
  mExpression = elem.attribute( "expression" );
  mMinZoomLevel = elem.attribute( "min-zoom" ).toInt();
  mMaxZoomLevel = elem.attribute( "max-zoom" ).toInt();

  mSymbol.reset();
  QDomElement symbolsElem = elem.firstChildElement( QStringLiteral( "symbols" ) );
  if ( !symbolsElem.isNull() )
  {
    QgsSymbolMap symbolMap = QgsSymbolLayerUtils::loadSymbols( symbolsElem, context );
    if ( !symbolMap.contains( QStringLiteral( "0" ) ) )
    {
      mSymbol.reset( symbolMap.take( QStringLiteral( "0" ) ) );
    }
  }
}

////////


QgsVectorTileBasicRenderer::QgsVectorTileBasicRenderer()
{
}

QString QgsVectorTileBasicRenderer::type() const
{
  return "basic";
}

QgsVectorTileBasicRenderer *QgsVectorTileBasicRenderer::clone() const
{
  QgsVectorTileBasicRenderer *r = new QgsVectorTileBasicRenderer;
  r->mStyles = mStyles;
  r->mStyles.detach();  // make a deep copy to make sure symbols get cloned
  return r;
}

void QgsVectorTileBasicRenderer::startRender( QgsRenderContext &context, int tileZoom, const QgsTileRange &tileRange )
{
  Q_UNUSED( context )
  Q_UNUSED( tileRange )
  // figure out required fields for different layers
  for ( const QgsVectorTileBasicRendererStyle &layerStyle : qgis::as_const( mStyles ) )
  {
    if ( layerStyle.isActive( tileZoom ) && !layerStyle.filterExpression().isEmpty() )
    {
      QgsExpression expr( layerStyle.filterExpression() );
      mRequiredFields[layerStyle.layerName()].unite( expr.referencedColumns() );
    }
  }
}

QMap<QString, QSet<QString> > QgsVectorTileBasicRenderer::usedAttributes( const QgsRenderContext & )
{
  return mRequiredFields;
}

void QgsVectorTileBasicRenderer::stopRender( QgsRenderContext &context )
{
  Q_UNUSED( context )
}

void QgsVectorTileBasicRenderer::renderTile( const QgsVectorTileRendererData &tile, QgsRenderContext &context )
{
  const QgsVectorTileFeatures tileData = tile.features();
  int zoomLevel = tile.id().zoomLevel();

  for ( const QgsVectorTileBasicRendererStyle &layerStyle : qgis::as_const( mStyles ) )
  {
    if ( !layerStyle.isActive( zoomLevel ) )
      continue;

    QgsFields fields = QgsVectorTileUtils::makeQgisFields( mRequiredFields[layerStyle.layerName()] );

    QgsExpressionContextScope *scope = new QgsExpressionContextScope( QObject::tr( "Layer" ) ); // will be deleted by popper
    scope->setFields( fields );
    QgsExpressionContextScopePopper popper( context.expressionContext(), scope );

    QgsExpression filterExpression( layerStyle.filterExpression() );
    filterExpression.prepare( &context.expressionContext() );

    QgsSymbol *sym = layerStyle.symbol();
    sym->startRender( context, QgsFields() );
    if ( layerStyle.layerName().isEmpty() )
    {
      // matching all layers
      for ( QString layerName : tileData.keys() )
      {
        for ( const QgsFeature &f : tileData[layerName] )
        {
          scope->setFeature( f );
          if ( filterExpression.isValid() && !filterExpression.evaluate( &context.expressionContext() ).toBool() )
            continue;

          if ( QgsWkbTypes::geometryType( f.geometry().wkbType() ) == layerStyle.geometryType() )
            sym->renderFeature( f, context );
        }
      }
    }
    else if ( tileData.contains( layerStyle.layerName() ) )
    {
      // matching one particular layer
      for ( const QgsFeature &f : tileData[layerStyle.layerName()] )
      {
        scope->setFeature( f );
        if ( filterExpression.isValid() && !filterExpression.evaluate( &context.expressionContext() ).toBool() )
          continue;

        if ( QgsWkbTypes::geometryType( f.geometry().wkbType() ) == layerStyle.geometryType() )
          sym->renderFeature( f, context );
      }
    }
    sym->stopRender( context );
  }
}

void QgsVectorTileBasicRenderer::writeXml( QDomElement &elem, const QgsReadWriteContext &context ) const
{
  QDomDocument doc = elem.ownerDocument();
  QDomElement elemStyles = doc.createElement( "styles" );
  for ( const QgsVectorTileBasicRendererStyle &layerStyle : mStyles )
  {
    QDomElement elemStyle = doc.createElement( "style" );
    layerStyle.writeXml( elemStyle, context );
    elemStyles.appendChild( elemStyle );
  }
  elem.appendChild( elemStyles );
}

void QgsVectorTileBasicRenderer::readXml( const QDomElement &elem, const QgsReadWriteContext &context )
{
  mStyles.clear();

  QDomElement elemStyles = elem.firstChildElement( "styles" );
  QDomElement elemStyle = elemStyles.firstChildElement( "style" );
  while ( !elemStyle.isNull() )
  {
    QgsVectorTileBasicRendererStyle layerStyle;
    layerStyle.readXml( elemStyle, context );
    mStyles.append( layerStyle );
  }
}

void QgsVectorTileBasicRenderer::setStyles( const QList<QgsVectorTileBasicRendererStyle> &styles )
{
  mStyles = styles;
}

QList<QgsVectorTileBasicRendererStyle> QgsVectorTileBasicRenderer::styles() const
{
  return mStyles;
}

QList<QgsVectorTileBasicRendererStyle> QgsVectorTileBasicRenderer::simpleStyleWithRandomColors()
{
  QColor polygonFillColor = QgsApplication::colorSchemeRegistry()->fetchRandomStyleColor();
  QColor polygonStrokeColor = polygonFillColor;
  polygonFillColor.setAlpha( 100 );
  double polygonStrokeWidth = DEFAULT_LINE_WIDTH;

  QColor lineStrokeColor = QgsApplication::colorSchemeRegistry()->fetchRandomStyleColor();
  double lineStrokeWidth = DEFAULT_LINE_WIDTH;

  QColor pointFillColor = QgsApplication::colorSchemeRegistry()->fetchRandomStyleColor();
  QColor pointStrokeColor = pointFillColor;
  pointFillColor.setAlpha( 100 );
  double pointSize = DEFAULT_POINT_SIZE;

  return simpleStyle( polygonFillColor, polygonStrokeColor, polygonStrokeWidth,
                      lineStrokeColor, lineStrokeWidth,
                      pointFillColor, pointStrokeColor, pointSize );
}

QList<QgsVectorTileBasicRendererStyle> QgsVectorTileBasicRenderer::simpleStyle(
  const QColor &polygonFillColor, const QColor &polygonStrokeColor, double polygonStrokeWidth,
  const QColor &lineStrokeColor, double lineStrokeWidth,
  const QColor &pointFillColor, const QColor &pointStrokeColor, double pointSize )
{
  QgsSimpleFillSymbolLayer *fillSymbolLayer = new QgsSimpleFillSymbolLayer();
  fillSymbolLayer->setFillColor( polygonFillColor );
  fillSymbolLayer->setStrokeColor( polygonStrokeColor );
  fillSymbolLayer->setStrokeWidth( polygonStrokeWidth );
  QgsFillSymbol *fillSymbol = new QgsFillSymbol( QgsSymbolLayerList() << fillSymbolLayer );

  QgsSimpleLineSymbolLayer *lineSymbolLayer = new QgsSimpleLineSymbolLayer;
  lineSymbolLayer->setColor( lineStrokeColor );
  lineSymbolLayer->setWidth( lineStrokeWidth );
  QgsLineSymbol *lineSymbol = new QgsLineSymbol( QgsSymbolLayerList() << lineSymbolLayer );

  QgsSimpleMarkerSymbolLayer *markerSymbolLayer = new QgsSimpleMarkerSymbolLayer;
  markerSymbolLayer->setFillColor( pointFillColor );
  markerSymbolLayer->setStrokeColor( pointStrokeColor );
  markerSymbolLayer->setSize( pointSize );
  QgsMarkerSymbol *markerSymbol = new QgsMarkerSymbol( QgsSymbolLayerList() << markerSymbolLayer );

  QgsVectorTileBasicRendererStyle st1( "polygons", QString(), QgsWkbTypes::PolygonGeometry );
  st1.setSymbol( fillSymbol );

  QgsVectorTileBasicRendererStyle st2( "lines", QString(), QgsWkbTypes::LineGeometry );
  st2.setSymbol( lineSymbol );

  QgsVectorTileBasicRendererStyle st3( "points", QString(), QgsWkbTypes::PointGeometry );
  st3.setSymbol( markerSymbol );

  QList<QgsVectorTileBasicRendererStyle> lst;
  lst << st1 << st2 << st3;
  return lst;
}
