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
#include "qgsfillsymbol.h"
#include "qgslinesymbol.h"
#include "qgsmarkersymbol.h"

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
  elem.setAttribute( QStringLiteral( "name" ), mStyleName );
  elem.setAttribute( QStringLiteral( "layer" ), mLayerName );
  elem.setAttribute( QStringLiteral( "geometry" ), mGeometryType );
  elem.setAttribute( QStringLiteral( "enabled" ), mEnabled ? QStringLiteral( "1" ) : QStringLiteral( "0" ) );
  elem.setAttribute( QStringLiteral( "expression" ), mExpression );
  elem.setAttribute( QStringLiteral( "min-zoom" ), mMinZoomLevel );
  elem.setAttribute( QStringLiteral( "max-zoom" ), mMaxZoomLevel );

  QDomDocument doc = elem.ownerDocument();
  QgsSymbolMap symbols;
  symbols[QStringLiteral( "0" )] = mSymbol.get();
  QDomElement symbolsElem = QgsSymbolLayerUtils::saveSymbols( symbols, QStringLiteral( "symbols" ), doc, context );
  elem.appendChild( symbolsElem );
}

void QgsVectorTileBasicRendererStyle::readXml( const QDomElement &elem, const QgsReadWriteContext &context )
{
  mStyleName = elem.attribute( QStringLiteral( "name" ) );
  mLayerName = elem.attribute( QStringLiteral( "layer" ) );
  mGeometryType = static_cast<QgsWkbTypes::GeometryType>( elem.attribute( QStringLiteral( "geometry" ) ).toInt() );
  mEnabled = elem.attribute( QStringLiteral( "enabled" ) ).toInt();
  mExpression = elem.attribute( QStringLiteral( "expression" ) );
  mMinZoomLevel = elem.attribute( QStringLiteral( "min-zoom" ) ).toInt();
  mMaxZoomLevel = elem.attribute( QStringLiteral( "max-zoom" ) ).toInt();

  mSymbol.reset();
  QDomElement symbolsElem = elem.firstChildElement( QStringLiteral( "symbols" ) );
  if ( !symbolsElem.isNull() )
  {
    QgsSymbolMap symbolMap = QgsSymbolLayerUtils::loadSymbols( symbolsElem, context );
    if ( symbolMap.contains( QStringLiteral( "0" ) ) )
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
  return QStringLiteral( "basic" );
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
  for ( const QgsVectorTileBasicRendererStyle &layerStyle : std::as_const( mStyles ) )
  {
    if ( layerStyle.isActive( tileZoom ) )
    {
      if ( !layerStyle.filterExpression().isEmpty() )
      {
        QgsExpression expr( layerStyle.filterExpression() );
        mRequiredFields[layerStyle.layerName()].unite( expr.referencedColumns() );
      }
      if ( auto *lSymbol = layerStyle.symbol() )
      {
        mRequiredFields[layerStyle.layerName()].unite( lSymbol->usedAttributes( context ) );
      }
    }
  }
}

QMap<QString, QSet<QString> > QgsVectorTileBasicRenderer::usedAttributes( const QgsRenderContext & )
{
  return mRequiredFields;
}

QSet<QString> QgsVectorTileBasicRenderer::requiredLayers( QgsRenderContext &, int tileZoom ) const
{
  QSet< QString > res;
  for ( const QgsVectorTileBasicRendererStyle &layerStyle : std::as_const( mStyles ) )
  {
    if ( layerStyle.isActive( tileZoom ) )
    {
      res.insert( layerStyle.layerName() );
    }
  }
  return res;
}

void QgsVectorTileBasicRenderer::stopRender( QgsRenderContext &context )
{
  Q_UNUSED( context )
}

void QgsVectorTileBasicRenderer::renderTile( const QgsVectorTileRendererData &tile, QgsRenderContext &context )
{
  const QgsVectorTileFeatures tileData = tile.features();
  int zoomLevel = tile.id().zoomLevel();

  for ( const QgsVectorTileBasicRendererStyle &layerStyle : std::as_const( mStyles ) )
  {
    if ( !layerStyle.isActive( zoomLevel ) || !layerStyle.symbol() )
      continue;

    QgsExpressionContextScope *scope = new QgsExpressionContextScope( QObject::tr( "Layer" ) ); // will be deleted by popper
    scope->setFields( tile.fields()[layerStyle.layerName()] );
    QgsExpressionContextScopePopper popper( context.expressionContext(), scope );

    QgsExpression filterExpression( layerStyle.filterExpression() );
    filterExpression.prepare( &context.expressionContext() );

    QgsSymbol *sym = layerStyle.symbol();
    sym->startRender( context, QgsFields() );
    if ( layerStyle.layerName() == QLatin1String( "background" ) )
    {
      QgsFillSymbol *fillSym = dynamic_cast<QgsFillSymbol *>( sym );
      if ( fillSym )
        fillSym->renderPolygon( tile.tilePolygon(), nullptr, nullptr, context );
    }
    else if ( layerStyle.layerName().isEmpty() )
    {
      // matching all layers
      for ( QString layerName : tileData.keys() )
      {
        for ( const QgsFeature &f : tileData[layerName] )
        {
          scope->setFeature( f );
          if ( filterExpression.isValid() && !filterExpression.evaluate( &context.expressionContext() ).toBool() )
            continue;

          const QgsWkbTypes::GeometryType featureType = QgsWkbTypes::geometryType( f.geometry().wkbType() );
          if ( featureType == layerStyle.geometryType() )
          {
            sym->renderFeature( f, context );
          }
          else if ( featureType == QgsWkbTypes::PolygonGeometry && layerStyle.geometryType() == QgsWkbTypes::LineGeometry )
          {
            // be tolerant and permit rendering polygons with a line layer style, as some style definitions use this approach
            // to render the polygon borders only
            QgsFeature exterior = f;
            exterior.setGeometry( QgsGeometry( f.geometry().constGet()->boundary() ) );
            sym->renderFeature( exterior, context );
          }
          else if ( featureType == QgsWkbTypes::PolygonGeometry && layerStyle.geometryType() == QgsWkbTypes::PointGeometry )
          {
            // be tolerant and permit rendering polygons with a point layer style, as some style definitions use this approach
            // to render the polygon center
            QgsFeature centroid = f;
            const QgsRectangle boundingBox = f.geometry().boundingBox();
            centroid.setGeometry( f.geometry().poleOfInaccessibility( std::min( boundingBox.width(), boundingBox.height() ) / 20 ) );
            sym->renderFeature( centroid, context );
          }
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

        const QgsWkbTypes::GeometryType featureType = QgsWkbTypes::geometryType( f.geometry().wkbType() );
        if ( featureType == layerStyle.geometryType() )
        {
          sym->renderFeature( f, context );
        }
        else if ( featureType == QgsWkbTypes::PolygonGeometry && layerStyle.geometryType() == QgsWkbTypes::LineGeometry )
        {
          // be tolerant and permit rendering polygons with a line layer style, as some style definitions use this approach
          // to render the polygon borders only
          QgsFeature exterior = f;
          exterior.setGeometry( QgsGeometry( f.geometry().constGet()->boundary() ) );
          sym->renderFeature( exterior, context );
        }
        else if ( featureType == QgsWkbTypes::PolygonGeometry && layerStyle.geometryType() == QgsWkbTypes::PointGeometry )
        {
          // be tolerant and permit rendering polygons with a point layer style, as some style definitions use this approach
          // to render the polygon center
          QgsFeature centroid = f;
          const QgsRectangle boundingBox = f.geometry().boundingBox();
          centroid.setGeometry( f.geometry().poleOfInaccessibility( std::min( boundingBox.width(), boundingBox.height() ) / 20 ) );
          sym->renderFeature( centroid, context );
        }
      }
    }
    sym->stopRender( context );
  }
}

void QgsVectorTileBasicRenderer::writeXml( QDomElement &elem, const QgsReadWriteContext &context ) const
{
  QDomDocument doc = elem.ownerDocument();
  QDomElement elemStyles = doc.createElement( QStringLiteral( "styles" ) );
  for ( const QgsVectorTileBasicRendererStyle &layerStyle : mStyles )
  {
    QDomElement elemStyle = doc.createElement( QStringLiteral( "style" ) );
    layerStyle.writeXml( elemStyle, context );
    elemStyles.appendChild( elemStyle );
  }
  elem.appendChild( elemStyles );
}

void QgsVectorTileBasicRenderer::readXml( const QDomElement &elem, const QgsReadWriteContext &context )
{
  mStyles.clear();

  QDomElement elemStyles = elem.firstChildElement( QStringLiteral( "styles" ) );
  QDomElement elemStyle = elemStyles.firstChildElement( QStringLiteral( "style" ) );
  while ( !elemStyle.isNull() )
  {
    QgsVectorTileBasicRendererStyle layerStyle;
    layerStyle.readXml( elemStyle, context );
    mStyles.append( layerStyle );
    elemStyle = elemStyle.nextSiblingElement( QStringLiteral( "style" ) );
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

  QgsVectorTileBasicRendererStyle st1( QStringLiteral( "Polygons" ), QString(), QgsWkbTypes::PolygonGeometry );
  st1.setFilterExpression( QStringLiteral( "geometry_type($geometry)='Polygon'" ) );
  st1.setSymbol( fillSymbol );

  QgsVectorTileBasicRendererStyle st2( QStringLiteral( "Lines" ), QString(), QgsWkbTypes::LineGeometry );
  st2.setFilterExpression( QStringLiteral( "geometry_type($geometry)='Line'" ) );
  st2.setSymbol( lineSymbol );

  QgsVectorTileBasicRendererStyle st3( QStringLiteral( "Points" ), QString(), QgsWkbTypes::PointGeometry );
  st3.setFilterExpression( QStringLiteral( "geometry_type($geometry)='Point'" ) );
  st3.setSymbol( markerSymbol );

  QList<QgsVectorTileBasicRendererStyle> lst;
  lst << st1 << st2 << st3;
  return lst;
}
