/***************************************************************************
  qgsmapboxglstyleconverter.cpp
  --------------------------------------
  Date                 : September 2020
  Copyright            : (C) 2020 by Nyall Dawson
  Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/


/*
 * Ported from original work by Martin Dobias, and extended by the MapTiler team!
 */

#include "qgsmapboxglstyleconverter.h"

#include "qgis.h"
#include "qgsapplication.h"
#include "qgsblureffect.h"
#include "qgseffectstack.h"
#include "qgsfillsymbol.h"
#include "qgsfillsymbollayer.h"
#include "qgsfontmanager.h"
#include "qgsfontutils.h"
#include "qgsjsonutils.h"
#include "qgslinesymbol.h"
#include "qgslinesymbollayer.h"
#include "qgslogger.h"
#include "qgsmarkersymbol.h"
#include "qgsmarkersymbollayer.h"
#include "qgspainteffect.h"
#include "qgsproviderregistry.h"
#include "qgsrasterlayer.h"
#include "qgsrasterpipe.h"
#include "qgssymbollayer.h"
#include "qgssymbollayerutils.h"
#include "qgstextbackgroundsettings.h"
#include "qgsvectortilebasiclabeling.h"
#include "qgsvectortilebasicrenderer.h"

#include <QBuffer>
#include <QRegularExpression>

#include "moc_qgsmapboxglstyleconverter.cpp"

QgsMapBoxGlStyleConverter::QgsMapBoxGlStyleConverter()
{
}

QgsMapBoxGlStyleConverter::Result QgsMapBoxGlStyleConverter::convert( const QVariantMap &style, QgsMapBoxGlStyleConversionContext *context )
{
  mError.clear();
  mWarnings.clear();

  if ( style.contains( u"sources"_s ) )
  {
    parseSources( style.value( u"sources"_s ).toMap(), context );
  }

  if ( style.contains( u"layers"_s ) )
  {
    parseLayers( style.value( u"layers"_s ).toList(), context );
  }
  else
  {
    mError = QObject::tr( "Could not find layers list in JSON" );
    return NoLayerList;
  }
  return Success;
}

QgsMapBoxGlStyleConverter::Result QgsMapBoxGlStyleConverter::convert( const QString &style, QgsMapBoxGlStyleConversionContext *context )
{
  return convert( QgsJsonUtils::parseJson( style ).toMap(), context );
}

QgsMapBoxGlStyleConverter::~QgsMapBoxGlStyleConverter()
{
  qDeleteAll( mSources );
}

void QgsMapBoxGlStyleConverter::parseLayers( const QVariantList &layers, QgsMapBoxGlStyleConversionContext *context )
{
  std::unique_ptr< QgsMapBoxGlStyleConversionContext > tmpContext;
  if ( !context )
  {
    tmpContext = std::make_unique< QgsMapBoxGlStyleConversionContext >();
    context = tmpContext.get();
  }

  QList<QgsVectorTileBasicRendererStyle> rendererStyles;
  QList<QgsVectorTileBasicLabelingStyle> labelingStyles;

  QgsVectorTileBasicRendererStyle rendererBackgroundStyle;
  bool hasRendererBackgroundStyle = false;

  for ( const QVariant &layer : layers )
  {
    const QVariantMap jsonLayer = layer.toMap();

    const QString layerType = jsonLayer.value( u"type"_s ).toString();
    if ( layerType == "background"_L1 )
    {
      hasRendererBackgroundStyle = parseFillLayer( jsonLayer, rendererBackgroundStyle, *context, true );
      if ( hasRendererBackgroundStyle )
      {
        rendererBackgroundStyle.setStyleName( layerType );
        rendererBackgroundStyle.setLayerName( layerType );
        rendererBackgroundStyle.setFilterExpression( QString() );
        rendererBackgroundStyle.setEnabled( true );
      }
      continue;
    }

    const QString styleId = jsonLayer.value( u"id"_s ).toString();
    context->setLayerId( styleId );

    if ( layerType.compare( "raster"_L1, Qt::CaseInsensitive ) == 0 )
    {
      QgsMapBoxGlStyleRasterSubLayer raster( styleId, jsonLayer.value( u"source"_s ).toString() );
      const QVariantMap jsonPaint = jsonLayer.value( u"paint"_s ).toMap();
      if ( jsonPaint.contains( u"raster-opacity"_s ) )
      {
        const QVariant jsonRasterOpacity = jsonPaint.value( u"raster-opacity"_s );
        double defaultOpacity = 1;
        raster.dataDefinedProperties().setProperty( QgsRasterPipe::Property::RendererOpacity, parseInterpolateByZoom( jsonRasterOpacity.toMap(), *context, 100, &defaultOpacity ) );
      }

      mRasterSubLayers.append( raster );
      continue;
    }

    const QString layerName = jsonLayer.value( u"source-layer"_s ).toString();

    const int minZoom = jsonLayer.value( u"minzoom"_s, u"-1"_s ).toInt();

    // WARNING -- the QGIS renderers for vector tiles treat maxzoom different to the MapBox Style Specifications.
    // from the MapBox Specifications:
    //
    // "The maximum zoom level for the layer. At zoom levels equal to or greater than the maxzoom, the layer will be hidden."
    //
    // However the QGIS styles will be hidden if the zoom level is GREATER THAN (not equal to) maxzoom.
    // Accordingly we need to subtract 1 from the maxzoom value in the JSON:
    int maxZoom = jsonLayer.value( u"maxzoom"_s, u"-1"_s ).toInt();
    if ( maxZoom != -1 )
      maxZoom--;

    QString visibilyStr;
    if ( jsonLayer.contains( u"visibility"_s ) )
    {
      visibilyStr = jsonLayer.value( u"visibility"_s ).toString();
    }
    else if ( jsonLayer.contains( u"layout"_s ) && jsonLayer.value( u"layout"_s ).userType() == QMetaType::Type::QVariantMap )
    {
      const QVariantMap jsonLayout = jsonLayer.value( u"layout"_s ).toMap();
      visibilyStr = jsonLayout.value( u"visibility"_s ).toString();
    }

    const bool enabled = visibilyStr != "none"_L1;

    QString filterExpression;
    if ( jsonLayer.contains( u"filter"_s ) )
    {
      filterExpression = parseExpression( jsonLayer.value( u"filter"_s ).toList(), *context );
    }

    QgsVectorTileBasicRendererStyle rendererStyle;
    QgsVectorTileBasicLabelingStyle labelingStyle;

    bool hasRendererStyle = false;
    bool hasLabelingStyle = false;
    if ( layerType == "fill"_L1 )
    {
      hasRendererStyle = parseFillLayer( jsonLayer, rendererStyle, *context );
    }
    else if ( layerType == "line"_L1 )
    {
      hasRendererStyle = parseLineLayer( jsonLayer, rendererStyle, *context );
    }
    else if ( layerType == "circle"_L1 )
    {
      hasRendererStyle = parseCircleLayer( jsonLayer, rendererStyle, *context );
    }
    else if ( layerType == "symbol"_L1 )
    {
      parseSymbolLayer( jsonLayer, rendererStyle, hasRendererStyle, labelingStyle, hasLabelingStyle, *context );
    }
    else
    {
      mWarnings << QObject::tr( "%1: Skipping unknown layer type %2" ).arg( context->layerId(), layerType );
      QgsDebugError( mWarnings.constLast() );
      continue;
    }

    if ( hasRendererStyle )
    {
      rendererStyle.setStyleName( styleId );
      rendererStyle.setLayerName( layerName );
      rendererStyle.setFilterExpression( filterExpression );
      rendererStyle.setMinZoomLevel( minZoom );
      rendererStyle.setMaxZoomLevel( maxZoom );
      rendererStyle.setEnabled( enabled );
      rendererStyles.append( rendererStyle );
    }

    if ( hasLabelingStyle )
    {
      labelingStyle.setStyleName( styleId );
      labelingStyle.setLayerName( layerName );
      labelingStyle.setFilterExpression( filterExpression );
      labelingStyle.setMinZoomLevel( minZoom );
      labelingStyle.setMaxZoomLevel( maxZoom );
      labelingStyle.setEnabled( enabled );
      labelingStyles.append( labelingStyle );
    }

    mWarnings.append( context->warnings() );
    context->clearWarnings();
  }

  if ( hasRendererBackgroundStyle )
    rendererStyles.prepend( rendererBackgroundStyle );

  auto renderer = std::make_unique< QgsVectorTileBasicRenderer >();
  renderer->setStyles( rendererStyles );
  mRenderer = std::move( renderer );

  auto labeling = std::make_unique< QgsVectorTileBasicLabeling >();
  labeling->setStyles( labelingStyles );
  mLabeling = std::move( labeling );
}

bool QgsMapBoxGlStyleConverter::parseFillLayer( const QVariantMap &jsonLayer, QgsVectorTileBasicRendererStyle &style, QgsMapBoxGlStyleConversionContext &context, bool isBackgroundStyle )
{
  const QVariantMap jsonPaint = jsonLayer.value( u"paint"_s ).toMap();

  QgsPropertyCollection ddProperties;
  QgsPropertyCollection ddRasterProperties;

  bool colorIsDataDefined = false;

  std::unique_ptr< QgsSymbol > symbol( std::make_unique< QgsFillSymbol >() );

  // fill color
  QColor fillColor;
  if ( jsonPaint.contains( isBackgroundStyle ? u"background-color"_s : u"fill-color"_s ) )
  {
    const QVariant jsonFillColor = jsonPaint.value( isBackgroundStyle ? u"background-color"_s : u"fill-color"_s );
    switch ( jsonFillColor.userType() )
    {
      case QMetaType::Type::QVariantMap:
        ddProperties.setProperty( QgsSymbolLayer::Property::FillColor, parseInterpolateColorByZoom( jsonFillColor.toMap(), context, &fillColor ) );
        break;

      case QMetaType::Type::QVariantList:
      case QMetaType::Type::QStringList:
        colorIsDataDefined = true;
        ddProperties.setProperty( QgsSymbolLayer::Property::FillColor, parseValueList( jsonFillColor.toList(), PropertyType::Color, context, 1, 255, &fillColor ) );
        break;

      case QMetaType::Type::QString:
        fillColor = parseColor( jsonFillColor.toString(), context );
        break;

      default:
      {
        context.pushWarning( QObject::tr( "%1: Skipping unsupported fill-color type (%2)" ).arg( context.layerId(), QMetaType::typeName( static_cast<QMetaType::Type>( jsonFillColor.userType() ) ) ) );
        break;
      }
    }
  }
  else
  {
    // defaults to #000000
    fillColor = QColor( 0, 0, 0 );
  }

  QColor fillOutlineColor;
  if ( !isBackgroundStyle )
  {
    if ( !jsonPaint.contains( u"fill-outline-color"_s ) )
    {
      if ( fillColor.isValid() )
        fillOutlineColor = fillColor;

      // match fill color data defined property when active
      if ( ddProperties.isActive( QgsSymbolLayer::Property::FillColor ) )
        ddProperties.setProperty( QgsSymbolLayer::Property::StrokeColor,  ddProperties.property( QgsSymbolLayer::Property::FillColor ) );
    }
    else
    {
      const QVariant jsonFillOutlineColor = jsonPaint.value( u"fill-outline-color"_s );
      switch ( jsonFillOutlineColor.userType() )
      {
        case QMetaType::Type::QVariantMap:
          ddProperties.setProperty( QgsSymbolLayer::Property::StrokeColor, parseInterpolateColorByZoom( jsonFillOutlineColor.toMap(), context, &fillOutlineColor ) );
          break;

        case QMetaType::Type::QVariantList:
        case QMetaType::Type::QStringList:
          ddProperties.setProperty( QgsSymbolLayer::Property::StrokeColor, parseValueList( jsonFillOutlineColor.toList(), PropertyType::Color, context, 1, 255, &fillOutlineColor ) );
          break;

        case QMetaType::Type::QString:
          fillOutlineColor = parseColor( jsonFillOutlineColor.toString(), context );
          break;

        default:
          context.pushWarning( QObject::tr( "%1: Skipping unsupported fill-outline-color type (%2)" ).arg( context.layerId(), QMetaType::typeName( static_cast<QMetaType::Type>( jsonFillOutlineColor.userType() ) ) ) );
          break;
      }
    }
  }

  double fillOpacity = -1.0;
  double rasterOpacity = -1.0;
  if ( jsonPaint.contains( isBackgroundStyle ? u"background-opacity"_s : u"fill-opacity"_s ) )
  {
    const QVariant jsonFillOpacity = jsonPaint.value( isBackgroundStyle ? u"background-opacity"_s : u"fill-opacity"_s );
    switch ( jsonFillOpacity.userType() )
    {
      case QMetaType::Type::Int:
      case QMetaType::Type::LongLong:
      case QMetaType::Type::Double:
        fillOpacity = jsonFillOpacity.toDouble();
        rasterOpacity = fillOpacity;
        break;

      case QMetaType::Type::QVariantMap:
        if ( ddProperties.isActive( QgsSymbolLayer::Property::FillColor ) )
        {
          symbol->setDataDefinedProperty( QgsSymbol::Property::Opacity, parseInterpolateByZoom( jsonFillOpacity.toMap(), context, 100 ) );
        }
        else
        {
          ddProperties.setProperty( QgsSymbolLayer::Property::FillColor, parseInterpolateOpacityByZoom( jsonFillOpacity.toMap(), fillColor.isValid() ? fillColor.alpha() : 255, &context ) );
          ddProperties.setProperty( QgsSymbolLayer::Property::StrokeColor, parseInterpolateOpacityByZoom( jsonFillOpacity.toMap(), fillOutlineColor.isValid() ? fillOutlineColor.alpha() : 255, &context ) );
          ddRasterProperties.setProperty( QgsSymbolLayer::Property::Opacity, parseInterpolateByZoom( jsonFillOpacity.toMap(), context, 100, &rasterOpacity ) );
        }
        break;

      case QMetaType::Type::QVariantList:
      case QMetaType::Type::QStringList:
        if ( ddProperties.isActive( QgsSymbolLayer::Property::FillColor ) )
        {
          symbol->setDataDefinedProperty( QgsSymbol::Property::Opacity, parseValueList( jsonFillOpacity.toList(), PropertyType::Numeric, context, 100, 100 ) );
        }
        else
        {
          ddProperties.setProperty( QgsSymbolLayer::Property::FillColor, parseValueList( jsonFillOpacity.toList(), PropertyType::Opacity, context, 1, fillColor.isValid() ? fillColor.alpha() : 255 ) );
          ddProperties.setProperty( QgsSymbolLayer::Property::StrokeColor, parseValueList( jsonFillOpacity.toList(), PropertyType::Opacity, context, 1, fillOutlineColor.isValid() ? fillOutlineColor.alpha() : 255 ) );
          ddRasterProperties.setProperty( QgsSymbolLayer::Property::Opacity, parseValueList( jsonFillOpacity.toList(), PropertyType::Numeric, context, 100, 255, nullptr, &rasterOpacity ) );
        }
        break;

      default:
        context.pushWarning( QObject::tr( "%1: Skipping unsupported fill-opacity type (%2)" ).arg( context.layerId(), QMetaType::typeName( static_cast<QMetaType::Type>( jsonFillOpacity.userType() ) ) ) );
        break;
    }
  }

  // fill-translate
  QPointF fillTranslate;
  if ( jsonPaint.contains( u"fill-translate"_s ) )
  {
    const QVariant jsonFillTranslate = jsonPaint.value( u"fill-translate"_s );
    switch ( jsonFillTranslate.userType() )
    {

      case QMetaType::Type::QVariantMap:
        ddProperties.setProperty( QgsSymbolLayer::Property::Offset, parseInterpolatePointByZoom( jsonFillTranslate.toMap(), context, context.pixelSizeConversionFactor(), &fillTranslate ) );
        break;

      case QMetaType::Type::QVariantList:
      case QMetaType::Type::QStringList:
        fillTranslate = QPointF( jsonFillTranslate.toList().value( 0 ).toDouble() * context.pixelSizeConversionFactor(),
                                 jsonFillTranslate.toList().value( 1 ).toDouble() * context.pixelSizeConversionFactor() );
        break;

      default:
        context.pushWarning( QObject::tr( "%1: Skipping unsupported fill-translate type (%2)" ).arg( context.layerId(), QMetaType::typeName( static_cast<QMetaType::Type>( jsonFillTranslate.userType() ) ) ) );
        break;
    }
  }

  QgsSimpleFillSymbolLayer *fillSymbol = dynamic_cast< QgsSimpleFillSymbolLayer * >( symbol->symbolLayer( 0 ) );
  Q_ASSERT( fillSymbol ); // should not fail since QgsFillSymbol() constructor instantiates a QgsSimpleFillSymbolLayer

  // set render units
  symbol->setOutputUnit( context.targetUnit() );
  fillSymbol->setOutputUnit( context.targetUnit() );

  if ( !fillTranslate.isNull() )
  {
    fillSymbol->setOffset( fillTranslate );
  }
  fillSymbol->setOffsetUnit( context.targetUnit() );

  if ( jsonPaint.contains( isBackgroundStyle ? u"background-pattern"_s : u"fill-pattern"_s ) )
  {
    // get fill-pattern to set sprite

    const QVariant fillPatternJson = jsonPaint.value( isBackgroundStyle ? u"background-pattern"_s : u"fill-pattern"_s );

    // fill-pattern disabled dillcolor
    fillColor = QColor();
    fillOutlineColor = QColor();

    // fill-pattern can be String or Object
    // String: {"fill-pattern": "dash-t"}
    // Object: {"fill-pattern":{"stops":[[11,"wetland8"],[12,"wetland16"]]}}

    QSize spriteSize;
    QString spriteProperty, spriteSizeProperty;
    const QString sprite = retrieveSpriteAsBase64WithProperties( fillPatternJson, context, spriteSize, spriteProperty, spriteSizeProperty );
    if ( !sprite.isEmpty() )
    {
      // when fill-pattern exists, set and insert QgsRasterFillSymbolLayer
      QgsRasterFillSymbolLayer *rasterFill = new QgsRasterFillSymbolLayer();
      rasterFill->setImageFilePath( sprite );
      rasterFill->setWidth( spriteSize.width() );
      rasterFill->setSizeUnit( context.targetUnit() );
      rasterFill->setCoordinateMode( Qgis::SymbolCoordinateReference::Viewport );

      if ( rasterOpacity >= 0 )
      {
        rasterFill->setOpacity( rasterOpacity );
      }

      if ( !spriteProperty.isEmpty() )
      {
        ddRasterProperties.setProperty( QgsSymbolLayer::Property::File, QgsProperty::fromExpression( spriteProperty ) );
        ddRasterProperties.setProperty( QgsSymbolLayer::Property::Width, QgsProperty::fromExpression( spriteSizeProperty ) );
      }

      rasterFill->setDataDefinedProperties( ddRasterProperties );
      symbol->appendSymbolLayer( rasterFill );
    }
  }

  fillSymbol->setDataDefinedProperties( ddProperties );

  if ( fillOpacity != -1 )
  {
    symbol->setOpacity( fillOpacity );
  }

  // some complex logic here!
  // by default a MapBox fill style will share the same stroke color as the fill color.
  // This is generally desirable and the 1px stroke can help to hide boundaries between features which
  // would otherwise be visible due to antialiasing effects.
  // BUT if the outline color is semi-transparent, then drawing the stroke will result in a double rendering
  // of strokes for adjacent polygons, resulting in visible seams between tiles. Accordingly, we only
  // set the stroke color if it's a completely different color to the fill (ie the style designer explicitly
  // wants a visible stroke) OR the stroke color is opaque and the double-rendering artifacts aren't an issue
  if ( fillOutlineColor.isValid() && ( fillOutlineColor.alpha() == 255 || fillOutlineColor != fillColor ) )
  {
    // mapbox fill strokes are always 1 px wide
    fillSymbol->setStrokeWidth( 0 );
    fillSymbol->setStrokeColor( fillOutlineColor );
  }
  else
  {
    fillSymbol->setStrokeStyle( Qt::NoPen );
  }

  if ( fillColor.isValid() )
  {
    fillSymbol->setFillColor( fillColor );
  }
  else if ( colorIsDataDefined )
  {
    fillSymbol->setFillColor( QColor( Qt::transparent ) );
  }
  else
  {
    fillSymbol->setBrushStyle( Qt::NoBrush );
  }

  style.setGeometryType( Qgis::GeometryType::Polygon );
  style.setSymbol( symbol.release() );
  return true;
}

bool QgsMapBoxGlStyleConverter::parseLineLayer( const QVariantMap &jsonLayer, QgsVectorTileBasicRendererStyle &style, QgsMapBoxGlStyleConversionContext &context )
{
  if ( !jsonLayer.contains( u"paint"_s ) )
  {
    context.pushWarning( QObject::tr( "%1: Style has no paint property, skipping" ).arg( context.layerId() ) );
    return false;
  }

  QgsPropertyCollection ddProperties;
  QString rasterLineSprite;

  const QVariantMap jsonPaint = jsonLayer.value( u"paint"_s ).toMap();
  if ( jsonPaint.contains( u"line-pattern"_s ) )
  {
    const QVariant jsonLinePattern = jsonPaint.value( u"line-pattern"_s );
    switch ( jsonLinePattern.userType() )
    {
      case QMetaType::Type::QVariantMap:
      case QMetaType::Type::QString:
      {
        QSize spriteSize;
        QString spriteProperty, spriteSizeProperty;
        rasterLineSprite = retrieveSpriteAsBase64WithProperties( jsonLinePattern, context, spriteSize, spriteProperty, spriteSizeProperty );
        ddProperties.setProperty( QgsSymbolLayer::Property::File, QgsProperty::fromExpression( spriteProperty ) );
        break;
      }

      case QMetaType::Type::QVariantList:
      case QMetaType::Type::QStringList:
      default:
        break;
    }

    if ( rasterLineSprite.isEmpty() )
    {
      // unsupported line-pattern definition, moving on
      context.pushWarning( QObject::tr( "%1: Skipping unsupported line-pattern property" ).arg( context.layerId() ) );
      return false;
    }
  }

  // line color
  QColor lineColor;
  if ( jsonPaint.contains( u"line-color"_s ) )
  {
    const QVariant jsonLineColor = jsonPaint.value( u"line-color"_s );
    switch ( jsonLineColor.userType() )
    {
      case QMetaType::Type::QVariantMap:
        ddProperties.setProperty( QgsSymbolLayer::Property::FillColor, parseInterpolateColorByZoom( jsonLineColor.toMap(), context, &lineColor ) );
        ddProperties.setProperty( QgsSymbolLayer::Property::StrokeColor, ddProperties.property( QgsSymbolLayer::Property::FillColor ) );
        break;

      case QMetaType::Type::QVariantList:
      case QMetaType::Type::QStringList:
        ddProperties.setProperty( QgsSymbolLayer::Property::FillColor, parseValueList( jsonLineColor.toList(), PropertyType::Color, context, 1, 255, &lineColor ) );
        ddProperties.setProperty( QgsSymbolLayer::Property::StrokeColor, ddProperties.property( QgsSymbolLayer::Property::FillColor ) );
        break;

      case QMetaType::Type::QString:
        lineColor = parseColor( jsonLineColor.toString(), context );
        break;

      default:
        context.pushWarning( QObject::tr( "%1: Skipping unsupported line-color type (%2)" ).arg( context.layerId(), QMetaType::typeName( static_cast<QMetaType::Type>( jsonLineColor.userType() ) ) ) );
        break;
    }
  }
  else
  {
    // defaults to #000000
    lineColor = QColor( 0, 0, 0 );
  }


  double lineWidth = 1.0 * context.pixelSizeConversionFactor();
  QgsProperty lineWidthProperty;
  if ( jsonPaint.contains( u"line-width"_s ) )
  {
    const QVariant jsonLineWidth = jsonPaint.value( u"line-width"_s );
    switch ( jsonLineWidth.userType() )
    {
      case QMetaType::Type::Int:
      case QMetaType::Type::LongLong:
      case QMetaType::Type::Double:
        lineWidth = jsonLineWidth.toDouble() * context.pixelSizeConversionFactor();
        break;

      case QMetaType::Type::QVariantMap:
      {
        lineWidth = -1;
        lineWidthProperty = parseInterpolateByZoom( jsonLineWidth.toMap(), context, context.pixelSizeConversionFactor(), &lineWidth );
        ddProperties.setProperty( QgsSymbolLayer::Property::StrokeWidth, lineWidthProperty );
        // set symbol layer visibility depending on line width since QGIS displays line with 0 width as hairlines
        QgsProperty layerEnabledProperty = QgsProperty( lineWidthProperty );
        layerEnabledProperty.setExpressionString( u"(%1) > 0"_s.arg( lineWidthProperty.expressionString() ) );
        ddProperties.setProperty( QgsSymbolLayer::Property::LayerEnabled, layerEnabledProperty );
        break;
      }

      case QMetaType::Type::QVariantList:
      case QMetaType::Type::QStringList:
      {
        lineWidthProperty = parseValueList( jsonLineWidth.toList(), PropertyType::Numeric, context, context.pixelSizeConversionFactor(), 255, nullptr, &lineWidth );
        ddProperties.setProperty( QgsSymbolLayer::Property::StrokeWidth, lineWidthProperty );
        // set symbol layer visibility depending on line width since QGIS displays line with 0 width as hairlines
        QgsProperty layerEnabledProperty = QgsProperty( lineWidthProperty );
        layerEnabledProperty.setExpressionString( u"(%1) > 0"_s.arg( lineWidthProperty.expressionString() ) );
        ddProperties.setProperty( QgsSymbolLayer::Property::LayerEnabled, layerEnabledProperty );
        break;
      }

      default:
        context.pushWarning( QObject::tr( "%1: Skipping unsupported fill-width type (%2)" ).arg( context.layerId(), QMetaType::typeName( static_cast<QMetaType::Type>( jsonLineWidth.userType() ) ) ) );
        break;
    }
  }

  double lineOffset = 0.0;
  if ( jsonPaint.contains( u"line-offset"_s ) )
  {
    const QVariant jsonLineOffset = jsonPaint.value( u"line-offset"_s );
    switch ( jsonLineOffset.userType() )
    {
      case QMetaType::Type::Int:
      case QMetaType::Type::LongLong:
      case QMetaType::Type::Double:
        lineOffset = -jsonLineOffset.toDouble() * context.pixelSizeConversionFactor();
        break;

      case QMetaType::Type::QVariantMap:
        lineWidth = -1;
        ddProperties.setProperty( QgsSymbolLayer::Property::Offset, parseInterpolateByZoom( jsonLineOffset.toMap(), context, context.pixelSizeConversionFactor() * -1, &lineOffset ) );
        break;

      case QMetaType::Type::QVariantList:
      case QMetaType::Type::QStringList:
        ddProperties.setProperty( QgsSymbolLayer::Property::Offset, parseValueList( jsonLineOffset.toList(), PropertyType::Numeric, context, context.pixelSizeConversionFactor() * -1, 255, nullptr, &lineOffset ) );
        break;

      default:
        context.pushWarning( QObject::tr( "%1: Skipping unsupported line-offset type (%2)" ).arg( context.layerId(), QMetaType::typeName( static_cast<QMetaType::Type>( jsonLineOffset.userType() ) ) ) );
        break;
    }
  }

  double lineOpacity = -1.0;
  QgsProperty lineOpacityProperty;
  if ( jsonPaint.contains( u"line-opacity"_s ) )
  {
    const QVariant jsonLineOpacity = jsonPaint.value( u"line-opacity"_s );
    switch ( jsonLineOpacity.userType() )
    {
      case QMetaType::Type::Int:
      case QMetaType::Type::LongLong:
      case QMetaType::Type::Double:
        lineOpacity = jsonLineOpacity.toDouble();
        break;

      case QMetaType::Type::QVariantMap:
        if ( ddProperties.isActive( QgsSymbolLayer::Property::StrokeColor ) )
        {
          double defaultValue = 1.0;
          lineOpacityProperty = parseInterpolateByZoom( jsonLineOpacity.toMap(), context, 100, &defaultValue );
        }
        else
        {
          ddProperties.setProperty( QgsSymbolLayer::Property::StrokeColor, parseInterpolateOpacityByZoom( jsonLineOpacity.toMap(), lineColor.isValid() ? lineColor.alpha() : 255, &context ) );
        }
        break;

      case QMetaType::Type::QVariantList:
      case QMetaType::Type::QStringList:
        if ( ddProperties.isActive( QgsSymbolLayer::Property::StrokeColor ) )
        {
          double defaultValue = 1.0;
          QColor invalidColor;
          lineOpacityProperty = parseValueList( jsonLineOpacity.toList(), PropertyType::Numeric, context, 100, 255, &invalidColor, &defaultValue );
        }
        else
        {
          ddProperties.setProperty( QgsSymbolLayer::Property::StrokeColor, parseValueList( jsonLineOpacity.toList(), PropertyType::Opacity, context, 1, lineColor.isValid() ? lineColor.alpha() : 255 ) );
        }
        break;

      default:
        context.pushWarning( QObject::tr( "%1: Skipping unsupported line-opacity type (%2)" ).arg( context.layerId(), QMetaType::typeName( static_cast<QMetaType::Type>( jsonLineOpacity.userType() ) ) ) );
        break;
    }
  }

  QVector< double > dashVector;
  if ( jsonPaint.contains( u"line-dasharray"_s ) )
  {
    const QVariant jsonLineDashArray = jsonPaint.value( u"line-dasharray"_s );
    switch ( jsonLineDashArray.userType() )
    {
      case QMetaType::Type::QVariantMap:
      {
        QString arrayExpression;
        if ( !lineWidthProperty.asExpression().isEmpty() )
        {
          arrayExpression = u"array_to_string(array_foreach(%1,@element * (%2)), ';')"_s // skip-keyword-check
                            .arg( parseArrayStops( jsonLineDashArray.toMap().value( u"stops"_s ).toList(), context, 1 ),
                                  lineWidthProperty.asExpression() );
        }
        else
        {
          arrayExpression = u"array_to_string(%1, ';')"_s.arg( parseArrayStops( jsonLineDashArray.toMap().value( u"stops"_s ).toList(), context, lineWidth ) );
        }
        ddProperties.setProperty( QgsSymbolLayer::Property::CustomDash, QgsProperty::fromExpression( arrayExpression ) );

        const QVariantList dashSource = jsonLineDashArray.toMap().value( u"stops"_s ).toList().first().toList().value( 1 ).toList();
        for ( const QVariant &v : dashSource )
        {
          dashVector << v.toDouble() * lineWidth;
        }
        break;
      }

      case QMetaType::Type::QVariantList:
      case QMetaType::Type::QStringList:
      {
        const QVariantList dashSource = jsonLineDashArray.toList();

        if ( dashSource.at( 0 ).userType() == QMetaType::Type::QString )
        {
          QgsProperty property = parseValueList( dashSource, PropertyType::NumericArray, context, 1, 255, nullptr, nullptr );
          if ( !lineWidthProperty.asExpression().isEmpty() )
          {
            property = QgsProperty::fromExpression( u"array_to_string(array_foreach(%1,@element * (%2)), ';')"_s // skip-keyword-check
                                                    .arg( property.asExpression(), lineWidthProperty.asExpression() ) );
          }
          else
          {
            property = QgsProperty::fromExpression( u"array_to_string(%1, ';')"_s.arg( property.asExpression() ) );
          }
          ddProperties.setProperty( QgsSymbolLayer::Property::CustomDash, property );
        }
        else
        {
          QVector< double > rawDashVectorSizes;
          rawDashVectorSizes.reserve( dashSource.size() );
          for ( const QVariant &v : dashSource )
          {
            rawDashVectorSizes << v.toDouble();
          }

          // handle non-compliant dash vector patterns
          if ( rawDashVectorSizes.size() == 1 )
          {
            // match behavior of MapBox style rendering -- if a user makes a line dash array with one element, it's ignored
            rawDashVectorSizes.clear();
          }
          else if ( rawDashVectorSizes.size() % 2 == 1 )
          {
            // odd number of dash pattern sizes -- this isn't permitted by Qt/QGIS, but isn't explicitly blocked by the MapBox specs
            // MapBox seems to add the extra dash element to the first dash size
            rawDashVectorSizes[0] = rawDashVectorSizes[0] + rawDashVectorSizes[rawDashVectorSizes.size() - 1];
            rawDashVectorSizes.resize( rawDashVectorSizes.size() - 1 );
          }

          if ( !rawDashVectorSizes.isEmpty() && ( !lineWidthProperty.asExpression().isEmpty() ) )
          {
            QStringList dashArrayStringParts;
            dashArrayStringParts.reserve( rawDashVectorSizes.size() );
            for ( double v : std::as_const( rawDashVectorSizes ) )
            {
              dashArrayStringParts << qgsDoubleToString( v );
            }

            QString arrayExpression = u"array_to_string(array_foreach(array(%1),@element * (%2)), ';')"_s // skip-keyword-check
                                      .arg( dashArrayStringParts.join( ',' ),
                                            lineWidthProperty.asExpression() );
            ddProperties.setProperty( QgsSymbolLayer::Property::CustomDash, QgsProperty::fromExpression( arrayExpression ) );
          }

          // dash vector sizes for QGIS symbols must be multiplied by the target line width
          for ( double v : std::as_const( rawDashVectorSizes ) )
          {
            dashVector << v *lineWidth;
          }
        }
        break;
      }

      default:
        context.pushWarning( QObject::tr( "%1: Skipping unsupported line-dasharray type (%2)" ).arg( context.layerId(), QMetaType::typeName( static_cast<QMetaType::Type>( jsonLineDashArray.userType() ) ) ) );
        break;
    }
  }

  Qt::PenCapStyle penCapStyle = Qt::FlatCap;
  Qt::PenJoinStyle penJoinStyle = Qt::MiterJoin;
  if ( jsonLayer.contains( u"layout"_s ) )
  {
    const QVariantMap jsonLayout = jsonLayer.value( u"layout"_s ).toMap();
    if ( jsonLayout.contains( u"line-cap"_s ) )
    {
      penCapStyle = parseCapStyle( jsonLayout.value( u"line-cap"_s ).toString() );
    }
    if ( jsonLayout.contains( u"line-join"_s ) )
    {
      penJoinStyle = parseJoinStyle( jsonLayout.value( u"line-join"_s ).toString() );
    }
  }

  std::unique_ptr< QgsSymbol > symbol( std::make_unique< QgsLineSymbol >() );
  symbol->setOutputUnit( context.targetUnit() );

  if ( !rasterLineSprite.isEmpty() )
  {
    QgsRasterLineSymbolLayer *lineSymbol = new QgsRasterLineSymbolLayer( rasterLineSprite );
    lineSymbol->setOutputUnit( context.targetUnit() );
    lineSymbol->setPenCapStyle( penCapStyle );
    lineSymbol->setPenJoinStyle( penJoinStyle );
    lineSymbol->setDataDefinedProperties( ddProperties );
    lineSymbol->setOffset( lineOffset );
    lineSymbol->setOffsetUnit( context.targetUnit() );

    if ( lineOpacity != -1 )
    {
      symbol->setOpacity( lineOpacity );
    }
    if ( !lineOpacityProperty.asExpression().isEmpty() )
    {
      QgsPropertyCollection ddProperties;
      ddProperties.setProperty( QgsSymbol::Property::Opacity, lineOpacityProperty );
      symbol->setDataDefinedProperties( ddProperties );
    }
    if ( lineWidth != -1 )
    {
      lineSymbol->setWidth( lineWidth );
    }
    symbol->changeSymbolLayer( 0, lineSymbol );
  }
  else
  {
    QgsSimpleLineSymbolLayer *lineSymbol = dynamic_cast< QgsSimpleLineSymbolLayer * >( symbol->symbolLayer( 0 ) );
    Q_ASSERT( lineSymbol ); // should not fail since QgsLineSymbol() constructor instantiates a QgsSimpleLineSymbolLayer

    // set render units
    lineSymbol->setOutputUnit( context.targetUnit() );
    lineSymbol->setPenCapStyle( penCapStyle );
    lineSymbol->setPenJoinStyle( penJoinStyle );
    lineSymbol->setDataDefinedProperties( ddProperties );
    lineSymbol->setOffset( lineOffset );
    lineSymbol->setOffsetUnit( context.targetUnit() );

    if ( lineOpacity != -1 )
    {
      symbol->setOpacity( lineOpacity );
    }
    if ( !lineOpacityProperty.asExpression().isEmpty() )
    {
      QgsPropertyCollection ddProperties;
      ddProperties.setProperty( QgsSymbol::Property::Opacity, lineOpacityProperty );
      symbol->setDataDefinedProperties( ddProperties );
    }
    if ( lineColor.isValid() )
    {
      lineSymbol->setColor( lineColor );
    }
    if ( lineWidth != -1 )
    {
      lineSymbol->setWidth( lineWidth );
    }
    if ( !dashVector.empty() )
    {
      lineSymbol->setUseCustomDashPattern( true );
      lineSymbol->setCustomDashVector( dashVector );
    }
  }

  style.setGeometryType( Qgis::GeometryType::Line );
  style.setSymbol( symbol.release() );
  return true;
}

bool QgsMapBoxGlStyleConverter::parseCircleLayer( const QVariantMap &jsonLayer, QgsVectorTileBasicRendererStyle &style, QgsMapBoxGlStyleConversionContext &context )
{
  if ( !jsonLayer.contains( u"paint"_s ) )
  {
    context.pushWarning( QObject::tr( "%1: Style has no paint property, skipping" ).arg( context.layerId() ) );
    return false;
  }

  const QVariantMap jsonPaint = jsonLayer.value( u"paint"_s ).toMap();
  QgsPropertyCollection ddProperties;

  // circle color
  QColor circleFillColor;
  if ( jsonPaint.contains( u"circle-color"_s ) )
  {
    const QVariant jsonCircleColor = jsonPaint.value( u"circle-color"_s );
    switch ( jsonCircleColor.userType() )
    {
      case QMetaType::Type::QVariantMap:
        ddProperties.setProperty( QgsSymbolLayer::Property::FillColor, parseInterpolateColorByZoom( jsonCircleColor.toMap(), context, &circleFillColor ) );
        break;

      case QMetaType::Type::QVariantList:
      case QMetaType::Type::QStringList:
        ddProperties.setProperty( QgsSymbolLayer::Property::FillColor, parseValueList( jsonCircleColor.toList(), PropertyType::Color, context, 1, 255, &circleFillColor ) );
        break;

      case QMetaType::Type::QString:
        circleFillColor = parseColor( jsonCircleColor.toString(), context );
        break;

      default:
        context.pushWarning( QObject::tr( "%1: Skipping unsupported circle-color type (%2)" ).arg( context.layerId(), QMetaType::typeName( static_cast<QMetaType::Type>( jsonCircleColor.userType() ) ) ) );
        break;
    }
  }
  else
  {
    // defaults to #000000
    circleFillColor = QColor( 0, 0, 0 );
  }

  // circle radius
  double circleDiameter = 10.0;
  if ( jsonPaint.contains( u"circle-radius"_s ) )
  {
    const QVariant jsonCircleRadius = jsonPaint.value( u"circle-radius"_s );
    switch ( jsonCircleRadius.userType() )
    {
      case QMetaType::Type::Int:
      case QMetaType::Type::LongLong:
      case QMetaType::Type::Double:
        circleDiameter = jsonCircleRadius.toDouble() * context.pixelSizeConversionFactor() * 2;
        break;

      case QMetaType::Type::QVariantMap:
        circleDiameter = -1;
        ddProperties.setProperty( QgsSymbolLayer::Property::Width, parseInterpolateByZoom( jsonCircleRadius.toMap(), context, context.pixelSizeConversionFactor() * 2, &circleDiameter ) );
        break;

      case QMetaType::Type::QVariantList:
      case QMetaType::Type::QStringList:
        ddProperties.setProperty( QgsSymbolLayer::Property::Width, parseValueList( jsonCircleRadius.toList(), PropertyType::Numeric, context, context.pixelSizeConversionFactor() * 2, 255, nullptr, &circleDiameter ) );
        break;

      default:
        context.pushWarning( QObject::tr( "%1: Skipping unsupported circle-radius type (%2)" ).arg( context.layerId(), QMetaType::typeName( static_cast<QMetaType::Type>( jsonCircleRadius.userType() ) ) ) );
        break;
    }
  }

  double circleOpacity = -1.0;
  if ( jsonPaint.contains( u"circle-opacity"_s ) )
  {
    const QVariant jsonCircleOpacity = jsonPaint.value( u"circle-opacity"_s );
    switch ( jsonCircleOpacity.userType() )
    {
      case QMetaType::Type::Int:
      case QMetaType::Type::LongLong:
      case QMetaType::Type::Double:
        circleOpacity = jsonCircleOpacity.toDouble();
        break;

      case QMetaType::Type::QVariantMap:
        ddProperties.setProperty( QgsSymbolLayer::Property::FillColor, parseInterpolateOpacityByZoom( jsonCircleOpacity.toMap(), circleFillColor.isValid() ? circleFillColor.alpha() : 255, &context ) );
        break;

      case QMetaType::Type::QVariantList:
      case QMetaType::Type::QStringList:
        ddProperties.setProperty( QgsSymbolLayer::Property::FillColor, parseValueList( jsonCircleOpacity.toList(), PropertyType::Opacity, context, 1, circleFillColor.isValid() ? circleFillColor.alpha() : 255 ) );
        break;

      default:
        context.pushWarning( QObject::tr( "%1: Skipping unsupported circle-opacity type (%2)" ).arg( context.layerId(), QMetaType::typeName( static_cast<QMetaType::Type>( jsonCircleOpacity.userType() ) ) ) );
        break;
    }
  }
  if ( ( circleOpacity != -1 ) && circleFillColor.isValid() )
  {
    circleFillColor.setAlphaF( circleOpacity );
  }

  // circle stroke color
  QColor circleStrokeColor;
  if ( jsonPaint.contains( u"circle-stroke-color"_s ) )
  {
    const QVariant jsonCircleStrokeColor = jsonPaint.value( u"circle-stroke-color"_s );
    switch ( jsonCircleStrokeColor.userType() )
    {
      case QMetaType::Type::QVariantMap:
        ddProperties.setProperty( QgsSymbolLayer::Property::StrokeColor, parseInterpolateColorByZoom( jsonCircleStrokeColor.toMap(), context, &circleStrokeColor ) );
        break;

      case QMetaType::Type::QVariantList:
      case QMetaType::Type::QStringList:
        ddProperties.setProperty( QgsSymbolLayer::Property::StrokeColor, parseValueList( jsonCircleStrokeColor.toList(), PropertyType::Color, context, 1, 255, &circleStrokeColor ) );
        break;

      case QMetaType::Type::QString:
        circleStrokeColor = parseColor( jsonCircleStrokeColor.toString(), context );
        break;

      default:
        context.pushWarning( QObject::tr( "%1: Skipping unsupported circle-stroke-color type (%2)" ).arg( context.layerId(), QMetaType::typeName( static_cast<QMetaType::Type>( jsonCircleStrokeColor.userType() ) ) ) );
        break;
    }
  }

  // circle stroke width
  double circleStrokeWidth = -1.0;
  if ( jsonPaint.contains( u"circle-stroke-width"_s ) )
  {
    const QVariant circleStrokeWidthJson = jsonPaint.value( u"circle-stroke-width"_s );
    switch ( circleStrokeWidthJson.userType() )
    {
      case QMetaType::Type::Int:
      case QMetaType::Type::LongLong:
      case QMetaType::Type::Double:
        circleStrokeWidth = circleStrokeWidthJson.toDouble() * context.pixelSizeConversionFactor();
        break;

      case QMetaType::Type::QVariantMap:
        circleStrokeWidth = -1.0;
        ddProperties.setProperty( QgsSymbolLayer::Property::StrokeWidth, parseInterpolateByZoom( circleStrokeWidthJson.toMap(), context, context.pixelSizeConversionFactor(), &circleStrokeWidth ) );
        break;

      case QMetaType::Type::QVariantList:
      case QMetaType::Type::QStringList:
        ddProperties.setProperty( QgsSymbolLayer::Property::StrokeWidth, parseValueList( circleStrokeWidthJson.toList(), PropertyType::Numeric, context, context.pixelSizeConversionFactor(), 255, nullptr, &circleStrokeWidth ) );
        break;

      default:
        context.pushWarning( QObject::tr( "%1: Skipping unsupported circle-stroke-width type (%2)" ).arg( context.layerId(), QMetaType::typeName( static_cast<QMetaType::Type>( circleStrokeWidthJson.userType() ) ) ) );
        break;
    }
  }

  double circleStrokeOpacity = -1.0;
  if ( jsonPaint.contains( u"circle-stroke-opacity"_s ) )
  {
    const QVariant jsonCircleStrokeOpacity = jsonPaint.value( u"circle-stroke-opacity"_s );
    switch ( jsonCircleStrokeOpacity.userType() )
    {
      case QMetaType::Type::Int:
      case QMetaType::Type::LongLong:
      case QMetaType::Type::Double:
        circleStrokeOpacity = jsonCircleStrokeOpacity.toDouble();
        break;

      case QMetaType::Type::QVariantMap:
        ddProperties.setProperty( QgsSymbolLayer::Property::StrokeColor, parseInterpolateOpacityByZoom( jsonCircleStrokeOpacity.toMap(), circleStrokeColor.isValid() ? circleStrokeColor.alpha() : 255, &context ) );
        break;

      case QMetaType::Type::QVariantList:
      case QMetaType::Type::QStringList:
        ddProperties.setProperty( QgsSymbolLayer::Property::StrokeColor, parseValueList( jsonCircleStrokeOpacity.toList(), PropertyType::Opacity, context, 1, circleStrokeColor.isValid() ? circleStrokeColor.alpha() : 255 ) );
        break;

      default:
        context.pushWarning( QObject::tr( "%1: Skipping unsupported circle-stroke-opacity type (%2)" ).arg( context.layerId(), QMetaType::typeName( static_cast<QMetaType::Type>( jsonCircleStrokeOpacity.userType() ) ) ) );
        break;
    }
  }
  if ( ( circleStrokeOpacity != -1 ) && circleStrokeColor.isValid() )
  {
    circleStrokeColor.setAlphaF( circleStrokeOpacity );
  }

  // translate
  QPointF circleTranslate;
  if ( jsonPaint.contains( u"circle-translate"_s ) )
  {
    const QVariant jsonCircleTranslate = jsonPaint.value( u"circle-translate"_s );
    switch ( jsonCircleTranslate.userType() )
    {

      case QMetaType::Type::QVariantMap:
        ddProperties.setProperty( QgsSymbolLayer::Property::Offset, parseInterpolatePointByZoom( jsonCircleTranslate.toMap(), context, context.pixelSizeConversionFactor(), &circleTranslate ) );
        break;

      case QMetaType::Type::QVariantList:
      case QMetaType::Type::QStringList:
        circleTranslate = QPointF( jsonCircleTranslate.toList().value( 0 ).toDouble() * context.pixelSizeConversionFactor(),
                                   jsonCircleTranslate.toList().value( 1 ).toDouble() * context.pixelSizeConversionFactor() );
        break;

      default:
        context.pushWarning( QObject::tr( "%1: Skipping unsupported circle-translate type (%2)" ).arg( context.layerId(), QMetaType::typeName( static_cast<QMetaType::Type>( jsonCircleTranslate.userType() ) ) ) );
        break;
    }
  }

  std::unique_ptr< QgsSymbol > symbol( std::make_unique< QgsMarkerSymbol >() );
  QgsSimpleMarkerSymbolLayer *markerSymbolLayer = dynamic_cast< QgsSimpleMarkerSymbolLayer * >( symbol->symbolLayer( 0 ) );
  Q_ASSERT( markerSymbolLayer );

  // set render units
  symbol->setOutputUnit( context.targetUnit() );
  symbol->setDataDefinedProperties( ddProperties );

  if ( !circleTranslate.isNull() )
  {
    markerSymbolLayer->setOffset( circleTranslate );
    markerSymbolLayer->setOffsetUnit( context.targetUnit() );
  }

  if ( circleFillColor.isValid() )
  {
    markerSymbolLayer->setFillColor( circleFillColor );
  }
  if ( circleDiameter != -1 )
  {
    markerSymbolLayer->setSize( circleDiameter );
    markerSymbolLayer->setSizeUnit( context.targetUnit() );
  }
  if ( circleStrokeColor.isValid() )
  {
    markerSymbolLayer->setStrokeColor( circleStrokeColor );
  }
  if ( circleStrokeWidth != -1 )
  {
    markerSymbolLayer->setStrokeWidth( circleStrokeWidth );
    markerSymbolLayer->setStrokeWidthUnit( context.targetUnit() );
  }

  style.setGeometryType( Qgis::GeometryType::Point );
  style.setSymbol( symbol.release() );
  return true;
}

void QgsMapBoxGlStyleConverter::parseSymbolLayer( const QVariantMap &jsonLayer, QgsVectorTileBasicRendererStyle &renderer, bool &hasRenderer, QgsVectorTileBasicLabelingStyle &labelingStyle, bool &hasLabeling, QgsMapBoxGlStyleConversionContext &context )
{
  hasLabeling = false;
  hasRenderer = false;

  if ( !jsonLayer.contains( u"layout"_s ) )
  {
    context.pushWarning( QObject::tr( "%1: Style layer has no layout property, skipping" ).arg( context.layerId() ) );
    return;
  }
  const QVariantMap jsonLayout = jsonLayer.value( u"layout"_s ).toMap();
  if ( !jsonLayout.contains( u"text-field"_s ) )
  {
    hasRenderer = parseSymbolLayerAsRenderer( jsonLayer, renderer, context );
    return;
  }

  const QVariantMap jsonPaint = jsonLayer.value( u"paint"_s ).toMap();

  QgsPropertyCollection ddLabelProperties;

  double textSize = 16.0 * context.pixelSizeConversionFactor();
  QgsProperty textSizeProperty;
  if ( jsonLayout.contains( u"text-size"_s ) )
  {
    const QVariant jsonTextSize = jsonLayout.value( u"text-size"_s );
    switch ( jsonTextSize.userType() )
    {
      case QMetaType::Type::Int:
      case QMetaType::Type::LongLong:
      case QMetaType::Type::Double:
        textSize = jsonTextSize.toDouble() * context.pixelSizeConversionFactor();
        break;

      case QMetaType::Type::QVariantMap:
        textSize = -1;
        textSizeProperty = parseInterpolateByZoom( jsonTextSize.toMap(), context, context.pixelSizeConversionFactor(), &textSize );

        break;

      case QMetaType::Type::QVariantList:
      case QMetaType::Type::QStringList:
        textSize = -1;
        textSizeProperty = parseValueList( jsonTextSize.toList(), PropertyType::Numeric, context, context.pixelSizeConversionFactor(), 255, nullptr, &textSize );
        break;

      default:
        context.pushWarning( QObject::tr( "%1: Skipping unsupported text-size type (%2)" ).arg( context.layerId(), QMetaType::typeName( static_cast<QMetaType::Type>( jsonTextSize.userType() ) ) ) );
        break;
    }

    if ( textSizeProperty )
    {
      ddLabelProperties.setProperty( QgsPalLayerSettings::Property::Size, textSizeProperty );
    }
  }

  // a rough average of ems to character count conversion for a variety of fonts
  constexpr double EM_TO_CHARS = 2.0;

  double textMaxWidth = -1;
  if ( jsonLayout.contains( u"text-max-width"_s ) )
  {
    const QVariant jsonTextMaxWidth = jsonLayout.value( u"text-max-width"_s );
    switch ( jsonTextMaxWidth.userType() )
    {
      case QMetaType::Type::Int:
      case QMetaType::Type::LongLong:
      case QMetaType::Type::Double:
        textMaxWidth = jsonTextMaxWidth.toDouble() * EM_TO_CHARS;
        break;

      case QMetaType::Type::QVariantMap:
        ddLabelProperties.setProperty( QgsPalLayerSettings::Property::AutoWrapLength, parseInterpolateByZoom( jsonTextMaxWidth.toMap(), context, EM_TO_CHARS, &textMaxWidth ) );
        break;

      case QMetaType::Type::QVariantList:
      case QMetaType::Type::QStringList:
        ddLabelProperties.setProperty( QgsPalLayerSettings::Property::AutoWrapLength, parseValueList( jsonTextMaxWidth.toList(), PropertyType::Numeric, context, EM_TO_CHARS, 255, nullptr, &textMaxWidth ) );
        break;

      default:
        context.pushWarning( QObject::tr( "%1: Skipping unsupported text-max-width type (%2)" ).arg( context.layerId(), QMetaType::typeName( static_cast<QMetaType::Type>( jsonTextMaxWidth.userType() ) ) ) );
        break;
    }
  }
  else
  {
    // defaults to 10
    textMaxWidth = 10 * EM_TO_CHARS;
  }

  double textLetterSpacing = -1;
  if ( jsonLayout.contains( u"text-letter-spacing"_s ) )
  {
    const QVariant jsonTextLetterSpacing = jsonLayout.value( u"text-letter-spacing"_s );
    switch ( jsonTextLetterSpacing.userType() )
    {
      case QMetaType::Type::Int:
      case QMetaType::Type::LongLong:
      case QMetaType::Type::Double:
        textLetterSpacing = jsonTextLetterSpacing.toDouble();
        break;

      case QMetaType::Type::QVariantMap:
        ddLabelProperties.setProperty( QgsPalLayerSettings::Property::FontLetterSpacing, parseInterpolateByZoom( jsonTextLetterSpacing.toMap(), context, 1, &textLetterSpacing ) );
        break;

      case QMetaType::Type::QVariantList:
      case QMetaType::Type::QStringList:
        ddLabelProperties.setProperty( QgsPalLayerSettings::Property::FontLetterSpacing, parseValueList( jsonTextLetterSpacing.toList(), PropertyType::Numeric, context, 1, 255, nullptr, &textLetterSpacing ) );
        break;

      default:
        context.pushWarning( QObject::tr( "%1: Skipping unsupported text-letter-spacing type (%2)" ).arg( context.layerId(), QMetaType::typeName( static_cast<QMetaType::Type>( jsonTextLetterSpacing.userType() ) ) ) );
        break;
    }
  }

  QFont textFont;
  bool foundFont = false;
  QString fontName;
  QString fontStyleName;

  if ( jsonLayout.contains( u"text-font"_s ) )
  {
    auto splitFontFamily = []( const QString & fontName, QString & family, QString & style ) -> bool
    {
      QString matchedFamily;
      const QStringList textFontParts = fontName.split( ' ' );
      for ( int i = textFontParts.size() - 1; i >= 1; --i )
      {
        const QString candidateFontFamily = textFontParts.mid( 0, i ).join( ' ' );
        const QString candidateFontStyle = textFontParts.mid( i ).join( ' ' );

        const QString processedFontFamily = QgsApplication::fontManager()->processFontFamilyName( candidateFontFamily );
        if ( QgsFontUtils::fontFamilyHasStyle( processedFontFamily, candidateFontStyle ) )
        {
          family = processedFontFamily;
          style = candidateFontStyle;
          return true;
        }
        else if ( QgsApplication::fontManager()->tryToDownloadFontFamily( processedFontFamily, matchedFamily ) )
        {
          if ( processedFontFamily == matchedFamily )
          {
            family = processedFontFamily;
            style = candidateFontStyle;
          }
          else
          {
            family = matchedFamily;
            style = processedFontFamily;
            style.replace( matchedFamily, QString() );
            style = style.trimmed();
            if ( !style.isEmpty() && !candidateFontStyle.isEmpty() )
            {
              style += u" %1"_s.arg( candidateFontStyle );
            }
          }
          return true;
        }
      }

      const QString processedFontFamily = QgsApplication::fontManager()->processFontFamilyName( fontName );
      if ( QFontDatabase().hasFamily( processedFontFamily ) )
      {
        // the json isn't following the spec correctly!!
        family = processedFontFamily;
        style.clear();
        return true;
      }
      else if ( QgsApplication::fontManager()->tryToDownloadFontFamily( processedFontFamily, matchedFamily ) )
      {
        family = matchedFamily;
        style.clear();
        return true;
      }
      return false;
    };

    const QVariant jsonTextFont = jsonLayout.value( u"text-font"_s );
    if ( jsonTextFont.userType() != QMetaType::Type::QVariantList && jsonTextFont.userType() != QMetaType::Type::QStringList && jsonTextFont.userType() != QMetaType::Type::QString
         && jsonTextFont.userType() != QMetaType::Type::QVariantMap )
    {
      context.pushWarning( QObject::tr( "%1: Skipping unsupported text-font type (%2)" ).arg( context.layerId(), QMetaType::typeName( static_cast<QMetaType::Type>( jsonTextFont.userType() ) ) ) );
    }
    else
    {
      switch ( jsonTextFont.userType() )
      {
        case QMetaType::Type::QVariantList:
        case QMetaType::Type::QStringList:
          fontName = jsonTextFont.toList().value( 0 ).toString();
          break;

        case QMetaType::Type::QString:
          fontName = jsonTextFont.toString();
          break;

        case QMetaType::Type::QVariantMap:
        {
          QString familyCaseString = u"CASE "_s;
          QString styleCaseString = u"CASE "_s;
          QString fontFamily;
          const QVariantList stops = jsonTextFont.toMap().value( u"stops"_s ).toList();

          bool error = false;
          for ( int i = 0; i < stops.length() - 1; ++i )
          {
            // bottom zoom and value
            const QVariant bz = stops.value( i ).toList().value( 0 );
            const QString bv = stops.value( i ).toList().value( 1 ).userType() == QMetaType::Type::QString ? stops.value( i ).toList().value( 1 ).toString() : stops.value( i ).toList().value( 1 ).toList().value( 0 ).toString();
            if ( bz.userType() == QMetaType::Type::QVariantList || bz.userType() == QMetaType::Type::QStringList )
            {
              context.pushWarning( QObject::tr( "%1: Expressions in interpolation function are not supported, skipping." ).arg( context.layerId() ) );
              error = true;
              break;
            }

            // top zoom
            const QVariant tz = stops.value( i + 1 ).toList().value( 0 );
            if ( tz.userType() == QMetaType::Type::QVariantList || tz.userType() == QMetaType::Type::QStringList )
            {
              context.pushWarning( QObject::tr( "%1: Expressions in interpolation function are not supported, skipping." ).arg( context.layerId() ) );
              error = true;
              break;
            }

            if ( splitFontFamily( bv, fontFamily, fontStyleName ) )
            {
              familyCaseString += QStringLiteral( "WHEN @vector_tile_zoom > %1 AND @vector_tile_zoom <= %2 "
                                                  "THEN %3 " ).arg( bz.toString(),
                                                      tz.toString(),
                                                      QgsExpression::quotedValue( fontFamily ) );
              styleCaseString += QStringLiteral( "WHEN @vector_tile_zoom > %1 AND @vector_tile_zoom <= %2 "
                                                 "THEN %3 " ).arg( bz.toString(),
                                                     tz.toString(),
                                                     QgsExpression::quotedValue( fontStyleName ) );
            }
            else
            {
              context.pushWarning( QObject::tr( "%1: Referenced font %2 is not available on system" ).arg( context.layerId(), bv ) );
            }
          }
          if ( error )
            break;

          const QString bv = stops.constLast().toList().value( 1 ).userType() == QMetaType::Type::QString ? stops.constLast().toList().value( 1 ).toString() : stops.constLast().toList().value( 1 ).toList().value( 0 ).toString();
          if ( splitFontFamily( bv, fontFamily, fontStyleName ) )
          {
            familyCaseString += u"ELSE %1 END"_s.arg( QgsExpression::quotedValue( fontFamily ) );
            styleCaseString += u"ELSE %1 END"_s.arg( QgsExpression::quotedValue( fontStyleName ) );
          }
          else
          {
            context.pushWarning( QObject::tr( "%1: Referenced font %2 is not available on system" ).arg( context.layerId(), bv ) );
          }

          ddLabelProperties.setProperty( QgsPalLayerSettings::Property::Family, QgsProperty::fromExpression( familyCaseString ) );
          ddLabelProperties.setProperty( QgsPalLayerSettings::Property::FontStyle, QgsProperty::fromExpression( styleCaseString ) );

          foundFont = true;
          fontName = fontFamily;

          break;
        }

        default:
          break;
      }

      QString fontFamily;
      if ( splitFontFamily( fontName, fontFamily, fontStyleName ) )
      {
        textFont = QgsFontUtils::createFont( fontFamily );
        if ( !fontStyleName.isEmpty() )
          textFont.setStyleName( fontStyleName );
        foundFont = true;
      }
    }
  }
  else
  {
    // Defaults to ["Open Sans Regular","Arial Unicode MS Regular"].
    if ( QgsFontUtils::fontFamilyHasStyle( u"Open Sans"_s, u"Regular"_s ) )
    {
      fontName = u"Open Sans"_s;
      textFont = QgsFontUtils::createFont( fontName );
      textFont.setStyleName( u"Regular"_s );
      fontStyleName = u"Regular"_s;
      foundFont = true;
    }
    else if ( QgsFontUtils::fontFamilyHasStyle( u"Arial Unicode MS"_s, u"Regular"_s ) )
    {
      fontName = u"Arial Unicode MS"_s;
      textFont = QgsFontUtils::createFont( fontName );
      textFont.setStyleName( u"Regular"_s );
      fontStyleName = u"Regular"_s;
      foundFont = true;
    }
    else
    {
      fontName = u"Open Sans, Arial Unicode MS"_s;
    }
  }
  if ( !foundFont && !fontName.isEmpty() )
  {
    context.pushWarning( QObject::tr( "%1: Referenced font %2 is not available on system" ).arg( context.layerId(), fontName ) );
  }

  // text color
  QColor textColor;
  if ( jsonPaint.contains( u"text-color"_s ) )
  {
    const QVariant jsonTextColor = jsonPaint.value( u"text-color"_s );
    switch ( jsonTextColor.userType() )
    {
      case QMetaType::Type::QVariantMap:
        ddLabelProperties.setProperty( QgsPalLayerSettings::Property::Color, parseInterpolateColorByZoom( jsonTextColor.toMap(), context, &textColor ) );
        break;

      case QMetaType::Type::QVariantList:
      case QMetaType::Type::QStringList:
        ddLabelProperties.setProperty( QgsPalLayerSettings::Property::Color, parseValueList( jsonTextColor.toList(), PropertyType::Color, context, 1, 255, &textColor ) );
        break;

      case QMetaType::Type::QString:
        textColor = parseColor( jsonTextColor.toString(), context );
        break;

      default:
        context.pushWarning( QObject::tr( "%1: Skipping unsupported text-color type (%2)" ).arg( context.layerId(), QMetaType::typeName( static_cast<QMetaType::Type>( jsonTextColor.userType() ) ) ) );
        break;
    }
  }
  else
  {
    // defaults to #000000
    textColor = QColor( 0, 0, 0 );
  }

  // buffer color
  QColor bufferColor( 0, 0, 0, 0 );
  if ( jsonPaint.contains( u"text-halo-color"_s ) )
  {
    const QVariant jsonBufferColor = jsonPaint.value( u"text-halo-color"_s );
    switch ( jsonBufferColor.userType() )
    {
      case QMetaType::Type::QVariantMap:
        ddLabelProperties.setProperty( QgsPalLayerSettings::Property::BufferColor, parseInterpolateColorByZoom( jsonBufferColor.toMap(), context, &bufferColor ) );
        break;

      case QMetaType::Type::QVariantList:
      case QMetaType::Type::QStringList:
        ddLabelProperties.setProperty( QgsPalLayerSettings::Property::BufferColor, parseValueList( jsonBufferColor.toList(), PropertyType::Color, context, 1, 255, &bufferColor ) );
        break;

      case QMetaType::Type::QString:
        bufferColor = parseColor( jsonBufferColor.toString(), context );
        break;

      default:
        context.pushWarning( QObject::tr( "%1: Skipping unsupported text-halo-color type (%2)" ).arg( context.layerId(), QMetaType::typeName( static_cast<QMetaType::Type>( jsonBufferColor.userType() ) ) ) );
        break;
    }
  }

  double bufferSize = 0.0;
  // the pixel based text buffers appear larger when rendered on the web - so automatically scale
  // them up when converting to a QGIS style
  // (this number is based on trial-and-error comparisons only!)
  constexpr double BUFFER_SIZE_SCALE = 2.0;
  if ( jsonPaint.contains( u"text-halo-width"_s ) )
  {
    const QVariant jsonHaloWidth = jsonPaint.value( u"text-halo-width"_s );
    QString bufferSizeDataDefined;
    switch ( jsonHaloWidth.userType() )
    {
      case QMetaType::Type::Int:
      case QMetaType::Type::LongLong:
      case QMetaType::Type::Double:
        bufferSize = jsonHaloWidth.toDouble() * context.pixelSizeConversionFactor() * BUFFER_SIZE_SCALE;
        break;

      case QMetaType::Type::QVariantMap:
        bufferSize = 1;
        bufferSizeDataDefined = parseInterpolateByZoom( jsonHaloWidth.toMap(), context, context.pixelSizeConversionFactor() * BUFFER_SIZE_SCALE, &bufferSize ).asExpression();
        break;

      case QMetaType::Type::QVariantList:
      case QMetaType::Type::QStringList:
        bufferSize = 1;
        bufferSizeDataDefined = parseValueList( jsonHaloWidth.toList(), PropertyType::Numeric, context, context.pixelSizeConversionFactor() * BUFFER_SIZE_SCALE, 255, nullptr, &bufferSize ).asExpression();
        break;

      default:
        context.pushWarning( QObject::tr( "%1: Skipping unsupported text-halo-width type (%2)" ).arg( context.layerId(), QMetaType::typeName( static_cast<QMetaType::Type>( jsonHaloWidth.userType() ) ) ) );
        break;
    }

    // from the specs halo should not be larger than 1/4 of the text-size
    // https://docs.mapbox.com/style-spec/reference/layers/#paint-symbol-text-halo-width
    if ( bufferSize > 0 )
    {
      if ( textSize > 0 && bufferSizeDataDefined.isEmpty() )
      {
        bufferSize = std::min( bufferSize, textSize * BUFFER_SIZE_SCALE / 4 );
      }
      else if ( textSize > 0 && !bufferSizeDataDefined.isEmpty() )
      {
        bufferSizeDataDefined = u"min(%1/4, %2)"_s.arg( textSize * BUFFER_SIZE_SCALE ).arg( bufferSizeDataDefined );
        ddLabelProperties.setProperty( QgsPalLayerSettings::Property::BufferSize, QgsProperty::fromExpression( bufferSizeDataDefined ) );
      }
      else if ( !bufferSizeDataDefined.isEmpty() )
      {
        bufferSizeDataDefined = u"min(%1*%2/4, %3)"_s
                                .arg( textSizeProperty.asExpression() )
                                .arg( BUFFER_SIZE_SCALE )
                                .arg( bufferSizeDataDefined );
        ddLabelProperties.setProperty( QgsPalLayerSettings::Property::BufferSize, QgsProperty::fromExpression( bufferSizeDataDefined ) );
      }
      else if ( bufferSizeDataDefined.isEmpty() )
      {
        bufferSizeDataDefined = u"min(%1*%2/4, %3)"_s
                                .arg( textSizeProperty.asExpression() )
                                .arg( BUFFER_SIZE_SCALE )
                                .arg( bufferSize );
        ddLabelProperties.setProperty( QgsPalLayerSettings::Property::BufferSize, QgsProperty::fromExpression( bufferSizeDataDefined ) );
      }
    }
  }

  double haloBlurSize = 0;
  if ( jsonPaint.contains( u"text-halo-blur"_s ) )
  {
    const QVariant jsonTextHaloBlur = jsonPaint.value( u"text-halo-blur"_s );
    switch ( jsonTextHaloBlur.userType() )
    {
      case QMetaType::Type::Int:
      case QMetaType::Type::LongLong:
      case QMetaType::Type::Double:
      {
        haloBlurSize = jsonTextHaloBlur.toDouble() * context.pixelSizeConversionFactor();
        break;
      }

      default:
        context.pushWarning( QObject::tr( "%1: Skipping unsupported text-halo-blur type (%2)" ).arg( context.layerId(), QMetaType::typeName( static_cast<QMetaType::Type>( jsonTextHaloBlur.userType() ) ) ) );
        break;
    }
  }

  QgsTextFormat format;
  format.setSizeUnit( context.targetUnit() );
  if ( textColor.isValid() )
    format.setColor( textColor );
  if ( textSize >= 0 )
    format.setSize( textSize );
  if ( foundFont )
  {
    format.setFont( textFont );
    if ( !fontStyleName.isEmpty() )
      format.setNamedStyle( fontStyleName );
  }
  if ( textLetterSpacing > 0 )
  {
    QFont f = format.font();
    f.setLetterSpacing( QFont::AbsoluteSpacing, textLetterSpacing );
    format.setFont( f );
  }

  if ( bufferSize > 0 )
  {
    // Color and opacity are separate components in QGIS
    const double opacity = bufferColor.alphaF();
    bufferColor.setAlphaF( 1.0 );

    format.buffer().setEnabled( true );
    format.buffer().setSize( bufferSize );
    format.buffer().setSizeUnit( context.targetUnit() );
    format.buffer().setColor( bufferColor );
    format.buffer().setOpacity( opacity );

    if ( haloBlurSize > 0 )
    {
      QgsEffectStack *stack = new QgsEffectStack();
      QgsBlurEffect *blur = new QgsBlurEffect() ;
      blur->setEnabled( true );
      blur->setBlurUnit( context.targetUnit() );
      blur->setBlurLevel( haloBlurSize );
      blur->setBlurMethod( QgsBlurEffect::StackBlur );
      stack->appendEffect( blur );
      stack->setEnabled( true );
      format.buffer().setPaintEffect( stack );
    }
  }

  QgsPalLayerSettings labelSettings;

  if ( textMaxWidth > 0 )
  {
    labelSettings.autoWrapLength = textMaxWidth;
  }

  // convert field name
  if ( jsonLayout.contains( u"text-field"_s ) )
  {
    const QVariant jsonTextField = jsonLayout.value( u"text-field"_s );
    switch ( jsonTextField.userType() )
    {
      case QMetaType::Type::QString:
      {
        labelSettings.fieldName = processLabelField( jsonTextField.toString(), labelSettings.isExpression );
        break;
      }

      case QMetaType::Type::QVariantList:
      case QMetaType::Type::QStringList:
      {
        const QVariantList textFieldList = jsonTextField.toList();
        /*
         * e.g.
         *     "text-field": ["format",
         *                    "foo", { "font-scale": 1.2 },
         *                    "bar", { "font-scale": 0.8 }
         * ]
         */
        if ( textFieldList.size() > 2 && textFieldList.at( 0 ).toString() == "format"_L1 )
        {
          QStringList parts;
          for ( int i = 1; i < textFieldList.size(); ++i )
          {
            bool isExpression = false;
            const QString part = processLabelField( textFieldList.at( i ).toString(), isExpression );
            if ( !isExpression )
              parts << QgsExpression::quotedColumnRef( part );
            else
              parts << part;
            // TODO -- we could also translate font color, underline, overline, strikethrough to HTML tags!
            i += 1;
          }
          labelSettings.fieldName = u"concat(%1)"_s.arg( parts.join( ',' ) );
          labelSettings.isExpression = true;
        }
        else
        {
          /*
           * e.g.
           *     "text-field": ["to-string", ["get", "name"]]
           */
          labelSettings.fieldName = parseExpression( textFieldList, context );
          labelSettings.isExpression = true;
        }
        break;
      }

      case QMetaType::Type::QVariantMap:
      {
        const QVariantList stops = jsonTextField.toMap().value( u"stops"_s ).toList();
        if ( !stops.empty() )
        {
          labelSettings.fieldName = parseLabelStops( stops, context );
          labelSettings.isExpression = true;
        }
        else
        {
          context.pushWarning( QObject::tr( "%1: Skipping unsupported text-field dictionary" ).arg( context.layerId() ) );
        }
        break;
      }

      default:
        context.pushWarning( QObject::tr( "%1: Skipping unsupported text-field type (%2)" ).arg( context.layerId(), QMetaType::typeName( static_cast<QMetaType::Type>( jsonTextField.userType() ) ) ) );
        break;
    }
  }

  if ( jsonLayout.contains( u"text-rotate"_s ) )
  {
    const QVariant jsonTextRotate = jsonLayout.value( u"text-rotate"_s );
    switch ( jsonTextRotate.userType() )
    {
      case QMetaType::Type::Double:
      case QMetaType::Type::Int:
      {
        labelSettings.angleOffset = jsonTextRotate.toDouble();
        break;
      }

      case QMetaType::Type::QVariantList:
      case QMetaType::Type::QStringList:
      {
        const QgsProperty property = parseValueList( jsonTextRotate.toList(), PropertyType::Numeric, context );
        ddLabelProperties.setProperty( QgsPalLayerSettings::Property::LabelRotation, property );
        break;
      }

      case QMetaType::Type::QVariantMap:
      {
        QVariantMap rotateMap = jsonTextRotate.toMap();
        if ( rotateMap.contains( u"property"_s ) && rotateMap[u"type"_s].toString() == "identity"_L1 )
        {
          const QgsProperty property = QgsProperty::fromExpression( rotateMap[u"property"_s].toString() );
          ddLabelProperties.setProperty( QgsPalLayerSettings::Property::LabelRotation, property );
        }
        else
          context.pushWarning( QObject::tr( "%1: Skipping unsupported text-rotate map content (%2)" ).arg( context.layerId(), QString( QJsonDocument::fromVariant( rotateMap ).toJson() ) ) );
        break;
      }

      default:
        context.pushWarning( QObject::tr( "%1: Skipping unsupported text-rotate type (%2)" ).arg( context.layerId(), QMetaType::typeName( static_cast<QMetaType::Type>( jsonTextRotate.userType() ) ) ) );
        break;
    }
  }

  if ( jsonLayout.contains( u"text-transform"_s ) )
  {
    const QString textTransform = jsonLayout.value( u"text-transform"_s ).toString();
    if ( textTransform == "uppercase"_L1 )
    {
      labelSettings.fieldName = u"upper(%1)"_s.arg( labelSettings.isExpression ? labelSettings.fieldName : QgsExpression::quotedColumnRef( labelSettings.fieldName ) );
    }
    else if ( textTransform == "lowercase"_L1 )
    {
      labelSettings.fieldName = u"lower(%1)"_s.arg( labelSettings.isExpression ? labelSettings.fieldName : QgsExpression::quotedColumnRef( labelSettings.fieldName ) );
    }
    labelSettings.isExpression = true;
  }

  labelSettings.placement = Qgis::LabelPlacement::OverPoint;
  Qgis::GeometryType geometryType = Qgis::GeometryType::Point;
  if ( jsonLayout.contains( u"symbol-placement"_s ) )
  {
    const QString symbolPlacement = jsonLayout.value( u"symbol-placement"_s ).toString();
    if ( symbolPlacement == "line"_L1 )
    {
      labelSettings.placement = Qgis::LabelPlacement::Curved;
      labelSettings.lineSettings().setPlacementFlags( Qgis::LabelLinePlacementFlag::OnLine );
      geometryType = Qgis::GeometryType::Line;

      if ( jsonLayout.contains( u"text-rotation-alignment"_s ) )
      {
        const QString textRotationAlignment = jsonLayout.value( u"text-rotation-alignment"_s ).toString();
        if ( textRotationAlignment == "viewport"_L1 )
        {
          labelSettings.placement = Qgis::LabelPlacement::Horizontal;
        }
      }

      if ( labelSettings.placement == Qgis::LabelPlacement::Curved )
      {
        QPointF textOffset;
        QgsProperty textOffsetProperty;
        if ( jsonLayout.contains( u"text-offset"_s ) )
        {
          const QVariant jsonTextOffset = jsonLayout.value( u"text-offset"_s );

          // units are ems!
          switch ( jsonTextOffset.userType() )
          {
            case QMetaType::Type::QVariantMap:
              textOffsetProperty = parseInterpolatePointByZoom( jsonTextOffset.toMap(), context, !textSizeProperty ? textSize : 1.0, &textOffset );
              if ( !textSizeProperty )
              {
                ddLabelProperties.setProperty( QgsPalLayerSettings::Property::LabelDistance, u"abs(array_get(%1,1))-%2"_s.arg( textOffsetProperty.asExpression() ).arg( textSize ) );
              }
              else
              {
                ddLabelProperties.setProperty( QgsPalLayerSettings::Property::LabelDistance, u"with_variable('text_size',%2,abs(array_get(%1,1))*@text_size-@text_size)"_s.arg( textOffsetProperty.asExpression(), textSizeProperty.asExpression() ) );
              }
              ddLabelProperties.setProperty( QgsPalLayerSettings::Property::LinePlacementOptions, u"if(array_get(%1,1)>0,'BL','AL')"_s.arg( textOffsetProperty.asExpression() ) );
              break;

            case QMetaType::Type::QVariantList:
            case QMetaType::Type::QStringList:
              textOffset = QPointF( jsonTextOffset.toList().value( 0 ).toDouble() * textSize,
                                    jsonTextOffset.toList().value( 1 ).toDouble() * textSize );
              break;

            default:
              context.pushWarning( QObject::tr( "%1: Skipping unsupported text-offset type (%2)" ).arg( context.layerId(), QMetaType::typeName( static_cast<QMetaType::Type>( jsonTextOffset.userType() ) ) ) );
              break;
          }

          if ( !textOffset.isNull() )
          {
            labelSettings.distUnits = context.targetUnit();
            labelSettings.dist = std::abs( textOffset.y() ) - textSize;
            labelSettings.lineSettings().setPlacementFlags( textOffset.y() > 0.0 ? Qgis::LabelLinePlacementFlag::BelowLine : Qgis::LabelLinePlacementFlag::AboveLine );
            if ( textSizeProperty && !textOffsetProperty )
            {
              ddLabelProperties.setProperty( QgsPalLayerSettings::Property::LabelDistance, u"with_variable('text_size',%2,%1*@text_size-@text_size)"_s.arg( std::abs( textOffset.y() / textSize ) ).arg( textSizeProperty.asExpression() ) );
            }
          }
        }

        if ( textOffset.isNull() )
        {
          labelSettings.lineSettings().setPlacementFlags( Qgis::LabelLinePlacementFlag::OnLine );
        }
      }
    }
  }

  if ( jsonLayout.contains( u"text-justify"_s ) )
  {
    const QVariant jsonTextJustify = jsonLayout.value( u"text-justify"_s );

    // default is center
    QString textAlign = u"center"_s;

    const QVariantMap conversionMap
    {
      { u"left"_s, u"left"_s },
      { u"center"_s, u"center"_s },
      { u"right"_s, u"right"_s },
      { u"auto"_s, u"follow"_s }
    };

    switch ( jsonTextJustify.userType() )
    {
      case QMetaType::Type::QString:
        textAlign = jsonTextJustify.toString();
        break;

      case QMetaType::Type::QVariantList:
        ddLabelProperties.setProperty( QgsPalLayerSettings::Property::OffsetQuad, QgsProperty::fromExpression( parseStringStops( jsonTextJustify.toList(), context, conversionMap, &textAlign ) ) );
        break;

      case QMetaType::Type::QVariantMap:
        ddLabelProperties.setProperty( QgsPalLayerSettings::Property::OffsetQuad, parseInterpolateStringByZoom( jsonTextJustify.toMap(), context, conversionMap, &textAlign ) );
        break;

      default:
        context.pushWarning( QObject::tr( "%1: Skipping unsupported text-justify type (%2)" ).arg( context.layerId(), QMetaType::typeName( static_cast<QMetaType::Type>( jsonTextJustify.userType() ) ) ) );
        break;
    }

    if ( textAlign == "left"_L1 )
      labelSettings.multilineAlign = Qgis::LabelMultiLineAlignment::Left;
    else if ( textAlign == "right"_L1 )
      labelSettings.multilineAlign = Qgis::LabelMultiLineAlignment::Right;
    else if ( textAlign == "center"_L1 )
      labelSettings.multilineAlign = Qgis::LabelMultiLineAlignment::Center;
    else if ( textAlign == "follow"_L1 )
      labelSettings.multilineAlign = Qgis::LabelMultiLineAlignment::FollowPlacement;
  }
  else
  {
    labelSettings.multilineAlign = Qgis::LabelMultiLineAlignment::Center;
  }

  if ( labelSettings.placement == Qgis::LabelPlacement::OverPoint )
  {
    if ( jsonLayout.contains( u"text-anchor"_s ) )
    {
      const QVariant jsonTextAnchor = jsonLayout.value( u"text-anchor"_s );
      QString textAnchor;

      const QVariantMap conversionMap
      {
        { u"center"_s, 4 },
        { u"left"_s, 5 },
        { u"right"_s, 3 },
        { u"top"_s, 7 },
        { u"bottom"_s, 1 },
        { u"top-left"_s, 8 },
        { u"top-right"_s, 6 },
        { u"bottom-left"_s, 2 },
        { u"bottom-right"_s, 0 },
      };

      switch ( jsonTextAnchor.userType() )
      {
        case QMetaType::Type::QString:
          textAnchor = jsonTextAnchor.toString();
          break;

        case QMetaType::Type::QVariantList:
          ddLabelProperties.setProperty( QgsPalLayerSettings::Property::OffsetQuad, QgsProperty::fromExpression( parseStringStops( jsonTextAnchor.toList(), context, conversionMap, &textAnchor ) ) );
          break;

        case QMetaType::Type::QVariantMap:
          ddLabelProperties.setProperty( QgsPalLayerSettings::Property::OffsetQuad, parseInterpolateStringByZoom( jsonTextAnchor.toMap(), context, conversionMap, &textAnchor ) );
          break;

        default:
          context.pushWarning( QObject::tr( "%1: Skipping unsupported text-anchor type (%2)" ).arg( context.layerId(), QMetaType::typeName( static_cast<QMetaType::Type>( jsonTextAnchor.userType() ) ) ) );
          break;
      }

      if ( textAnchor == "center"_L1 )
        labelSettings.pointSettings().setQuadrant( Qgis::LabelQuadrantPosition::Over );
      else if ( textAnchor == "left"_L1 )
        labelSettings.pointSettings().setQuadrant( Qgis::LabelQuadrantPosition::Right );
      else if ( textAnchor == "right"_L1 )
        labelSettings.pointSettings().setQuadrant( Qgis::LabelQuadrantPosition::Left );
      else if ( textAnchor == "top"_L1 )
        labelSettings.pointSettings().setQuadrant( Qgis::LabelQuadrantPosition::Below );
      else if ( textAnchor == "bottom"_L1 )
        labelSettings.pointSettings().setQuadrant( Qgis::LabelQuadrantPosition::Above );
      else if ( textAnchor == "top-left"_L1 )
        labelSettings.pointSettings().setQuadrant( Qgis::LabelQuadrantPosition::BelowRight );
      else if ( textAnchor == "top-right"_L1 )
        labelSettings.pointSettings().setQuadrant( Qgis::LabelQuadrantPosition::BelowLeft );
      else if ( textAnchor == "bottom-left"_L1 )
        labelSettings.pointSettings().setQuadrant( Qgis::LabelQuadrantPosition::AboveRight );
      else if ( textAnchor == "bottom-right"_L1 )
        labelSettings.pointSettings().setQuadrant( Qgis::LabelQuadrantPosition::AboveLeft );
    }

    QPointF textOffset;
    if ( jsonLayout.contains( u"text-offset"_s ) )
    {
      const QVariant jsonTextOffset = jsonLayout.value( u"text-offset"_s );

      // units are ems!
      switch ( jsonTextOffset.userType() )
      {
        case QMetaType::Type::QVariantMap:
          ddLabelProperties.setProperty( QgsPalLayerSettings::Property::OffsetXY, parseInterpolatePointByZoom( jsonTextOffset.toMap(), context, textSize, &textOffset ) );
          break;

        case QMetaType::Type::QVariantList:
        case QMetaType::Type::QStringList:
          textOffset = QPointF( jsonTextOffset.toList().value( 0 ).toDouble() * textSize,
                                jsonTextOffset.toList().value( 1 ).toDouble() * textSize );
          break;

        default:
          context.pushWarning( QObject::tr( "%1: Skipping unsupported text-offset type (%2)" ).arg( context.layerId(), QMetaType::typeName( static_cast<QMetaType::Type>( jsonTextOffset.userType() ) ) ) );
          break;
      }

      if ( !textOffset.isNull() )
      {
        labelSettings.offsetUnits = context.targetUnit();
        labelSettings.xOffset = textOffset.x();
        labelSettings.yOffset = textOffset.y();
      }
    }
  }

  if ( jsonLayout.contains( u"icon-image"_s ) &&
       ( labelSettings.placement == Qgis::LabelPlacement::Horizontal || labelSettings.placement == Qgis::LabelPlacement::Curved ) )
  {
    QSize spriteSize;
    QString spriteProperty, spriteSizeProperty;
    const QString sprite = retrieveSpriteAsBase64WithProperties( jsonLayout.value( u"icon-image"_s ), context, spriteSize, spriteProperty, spriteSizeProperty );
    if ( !sprite.isEmpty() )
    {
      double size = 1.0;
      if ( jsonLayout.contains( u"icon-size"_s ) )
      {
        QgsProperty property;
        const QVariant jsonIconSize = jsonLayout.value( u"icon-size"_s );
        switch ( jsonIconSize.userType() )
        {
          case QMetaType::Type::Int:
          case QMetaType::Type::LongLong:
          case QMetaType::Type::Double:
          {
            size = jsonIconSize.toDouble();
            if ( !spriteSizeProperty.isEmpty() )
            {
              ddLabelProperties.setProperty( QgsPalLayerSettings::Property::ShapeSizeX,
                                             QgsProperty::fromExpression( u"with_variable('marker_size',%1,%2*@marker_size)"_s.arg( spriteSizeProperty ).arg( size ) ) );
            }
            break;
          }

          case QMetaType::Type::QVariantMap:
            property = parseInterpolateByZoom( jsonIconSize.toMap(), context, 1, &size );
            break;

          case QMetaType::Type::QVariantList:
          case QMetaType::Type::QStringList:
            property = parseValueList( jsonIconSize.toList(), PropertyType::Numeric, context );
            break;
          default:
            context.pushWarning( QObject::tr( "%1: Skipping non-implemented icon-size type (%2)" ).arg( context.layerId(), QMetaType::typeName( static_cast<QMetaType::Type>( jsonIconSize.userType() ) ) ) );
            break;
        }

        if ( !property.expressionString().isEmpty() )
        {
          if ( !spriteSizeProperty.isEmpty() )
          {
            ddLabelProperties.setProperty( QgsPalLayerSettings::Property::ShapeSizeX,
                                           QgsProperty::fromExpression( u"with_variable('marker_size',%1,(%2)*@marker_size)"_s.arg( spriteSizeProperty ).arg( property.expressionString() ) ) );
          }
          else
          {
            ddLabelProperties.setProperty( QgsPalLayerSettings::Property::ShapeSizeX,
                                           QgsProperty::fromExpression( u"(%2)*%1"_s.arg( spriteSize.width() ).arg( property.expressionString() ) ) );
          }
        }
      }

      QgsRasterMarkerSymbolLayer *markerLayer = new QgsRasterMarkerSymbolLayer( );
      markerLayer->setPath( sprite );
      markerLayer->setSize( spriteSize.width() );
      markerLayer->setSizeUnit( context.targetUnit() );

      if ( !spriteProperty.isEmpty() )
      {
        QgsPropertyCollection markerDdProperties;
        markerDdProperties.setProperty( QgsSymbolLayer::Property::Name, QgsProperty::fromExpression( spriteProperty ) );
        markerLayer->setDataDefinedProperties( markerDdProperties );
      }

      QgsTextBackgroundSettings backgroundSettings;
      backgroundSettings.setEnabled( true );
      backgroundSettings.setType( QgsTextBackgroundSettings::ShapeMarkerSymbol );
      backgroundSettings.setSize( spriteSize * size );
      backgroundSettings.setSizeUnit( context.targetUnit() );
      backgroundSettings.setSizeType( QgsTextBackgroundSettings::SizeFixed );
      backgroundSettings.setMarkerSymbol( new QgsMarkerSymbol( QgsSymbolLayerList() << markerLayer ) );
      format.setBackground( backgroundSettings );
    }
  }

  if ( textSize >= 0 )
  {
    // TODO -- this probably needs revisiting -- it was copied from the MapTiler code, but may be wrong...
    labelSettings.priority = std::min( textSize / ( context.pixelSizeConversionFactor() * 3 ), 10.0 );
  }

  labelSettings.setFormat( format );

  // use a low obstacle weight for layers by default -- we'd rather have more labels for these layers, even if placement isn't ideal
  labelSettings.obstacleSettings().setFactor( 0.1 );

  labelSettings.setDataDefinedProperties( ddLabelProperties );

  labelingStyle.setGeometryType( geometryType );
  labelingStyle.setLabelSettings( labelSettings );

  hasLabeling = true;

  hasRenderer = parseSymbolLayerAsRenderer( jsonLayer, renderer, context );
}

bool QgsMapBoxGlStyleConverter::parseSymbolLayerAsRenderer( const QVariantMap &jsonLayer, QgsVectorTileBasicRendererStyle &rendererStyle, QgsMapBoxGlStyleConversionContext &context )
{
  if ( !jsonLayer.contains( u"layout"_s ) )
  {
    context.pushWarning( QObject::tr( "%1: Style layer has no layout property, skipping" ).arg( context.layerId() ) );
    return false;
  }
  const QVariantMap jsonLayout = jsonLayer.value( u"layout"_s ).toMap();

  if ( jsonLayout.value( u"symbol-placement"_s ).toString() == "line"_L1 && !jsonLayout.contains( u"text-field"_s ) )
  {
    QgsPropertyCollection ddProperties;

    double spacing = -1.0;
    if ( jsonLayout.contains( u"symbol-spacing"_s ) )
    {
      const QVariant jsonSpacing = jsonLayout.value( u"symbol-spacing"_s );
      switch ( jsonSpacing.userType() )
      {
        case QMetaType::Type::Int:
        case QMetaType::Type::LongLong:
        case QMetaType::Type::Double:
          spacing = jsonSpacing.toDouble() * context.pixelSizeConversionFactor();
          break;

        case QMetaType::Type::QVariantMap:
          ddProperties.setProperty( QgsSymbolLayer::Property::Interval, parseInterpolateByZoom( jsonSpacing.toMap(), context, context.pixelSizeConversionFactor(), &spacing ) );
          break;

        case QMetaType::Type::QVariantList:
        case QMetaType::Type::QStringList:
          ddProperties.setProperty( QgsSymbolLayer::Property::Interval, parseValueList( jsonSpacing.toList(), PropertyType::Numeric, context, context.pixelSizeConversionFactor(), 255, nullptr, &spacing ) );
          break;

        default:
          context.pushWarning( QObject::tr( "%1: Skipping unsupported symbol-spacing type (%2)" ).arg( context.layerId(), QMetaType::typeName( static_cast<QMetaType::Type>( jsonSpacing.userType() ) ) ) );
          break;
      }
    }
    else
    {
      // defaults to 250
      spacing = 250 * context.pixelSizeConversionFactor();
    }

    bool rotateMarkers = true;
    if ( jsonLayout.contains( u"icon-rotation-alignment"_s ) )
    {
      const QString alignment = jsonLayout.value( u"icon-rotation-alignment"_s ).toString();
      if ( alignment == "map"_L1 || alignment == "auto"_L1 )
      {
        rotateMarkers = true;
      }
      else if ( alignment == "viewport"_L1 )
      {
        rotateMarkers = false;
      }
    }

    QgsPropertyCollection markerDdProperties;
    double rotation = 0.0;
    if ( jsonLayout.contains( u"icon-rotate"_s ) )
    {
      const QVariant jsonIconRotate = jsonLayout.value( u"icon-rotate"_s );
      switch ( jsonIconRotate.userType() )
      {
        case QMetaType::Type::Int:
        case QMetaType::Type::LongLong:
        case QMetaType::Type::Double:
          rotation = jsonIconRotate.toDouble();
          break;

        case QMetaType::Type::QVariantMap:
          markerDdProperties.setProperty( QgsSymbolLayer::Property::Angle, parseInterpolateByZoom( jsonIconRotate.toMap(), context, context.pixelSizeConversionFactor(), &rotation ) );
          break;

        case QMetaType::Type::QVariantList:
        case QMetaType::Type::QStringList:
          markerDdProperties.setProperty( QgsSymbolLayer::Property::Angle, parseValueList( jsonIconRotate.toList(), PropertyType::Numeric, context, context.pixelSizeConversionFactor(), 255, nullptr, &rotation ) );
          break;

        default:
          context.pushWarning( QObject::tr( "%1: Skipping unsupported icon-rotate type (%2)" ).arg( context.layerId(), QMetaType::typeName( static_cast<QMetaType::Type>( jsonIconRotate.userType() ) ) ) );
          break;
      }
    }

    QgsMarkerLineSymbolLayer *lineSymbol = new QgsMarkerLineSymbolLayer( rotateMarkers, spacing > 0 ? spacing : 1 );
    lineSymbol->setOutputUnit( context.targetUnit() );
    lineSymbol->setDataDefinedProperties( ddProperties );
    if ( spacing < 1 )
    {
      // if spacing isn't specified, it's a central point marker only
      lineSymbol->setPlacements( Qgis::MarkerLinePlacement::CentralPoint );
    }

    QgsRasterMarkerSymbolLayer *markerLayer = new QgsRasterMarkerSymbolLayer( );
    QSize spriteSize;
    QString spriteProperty, spriteSizeProperty;
    const QString sprite = retrieveSpriteAsBase64WithProperties( jsonLayout.value( u"icon-image"_s ), context, spriteSize, spriteProperty, spriteSizeProperty );
    if ( !sprite.isNull() )
    {
      markerLayer->setPath( sprite );
      markerLayer->setSize( spriteSize.width() );
      markerLayer->setSizeUnit( context.targetUnit() );

      if ( !spriteProperty.isEmpty() )
      {
        markerDdProperties.setProperty( QgsSymbolLayer::Property::Name, QgsProperty::fromExpression( spriteProperty ) );
        markerDdProperties.setProperty( QgsSymbolLayer::Property::Width, QgsProperty::fromExpression( spriteSizeProperty ) );
      }
    }

    if ( jsonLayout.contains( u"icon-size"_s ) )
    {
      const QVariant jsonIconSize = jsonLayout.value( u"icon-size"_s );
      double size = 1.0;
      QgsProperty property;
      switch ( jsonIconSize.userType() )
      {
        case QMetaType::Type::Int:
        case QMetaType::Type::LongLong:
        case QMetaType::Type::Double:
        {
          size = jsonIconSize.toDouble();
          if ( !spriteSizeProperty.isEmpty() )
          {
            markerDdProperties.setProperty( QgsSymbolLayer::Property::Width,
                                            QgsProperty::fromExpression( u"with_variable('marker_size',%1,%2*@marker_size)"_s.arg( spriteSizeProperty ).arg( size ) ) );
          }
          break;
        }

        case QMetaType::Type::QVariantMap:
          property = parseInterpolateByZoom( jsonIconSize.toMap(), context, 1, &size );
          break;

        case QMetaType::Type::QVariantList:
        case QMetaType::Type::QStringList:
          property = parseValueList( jsonIconSize.toList(), PropertyType::Numeric, context );
          break;
        default:
          context.pushWarning( QObject::tr( "%1: Skipping non-implemented icon-size type (%2)" ).arg( context.layerId(), QMetaType::typeName( static_cast<QMetaType::Type>( jsonIconSize.userType() ) ) ) );
          break;
      }
      markerLayer->setSize( size * spriteSize.width() );
      if ( !property.expressionString().isEmpty() )
      {
        if ( !spriteSizeProperty.isEmpty() )
        {
          markerDdProperties.setProperty( QgsSymbolLayer::Property::Width,
                                          QgsProperty::fromExpression( u"with_variable('marker_size',%1,(%2)*@marker_size)"_s.arg( spriteSizeProperty ).arg( property.expressionString() ) ) );
        }
        else
        {
          markerDdProperties.setProperty( QgsSymbolLayer::Property::Width,
                                          QgsProperty::fromExpression( u"(%2)*%1"_s.arg( spriteSize.width() ).arg( property.expressionString() ) ) );
        }
      }
    }

    markerLayer->setDataDefinedProperties( markerDdProperties );
    markerLayer->setAngle( rotation );
    lineSymbol->setSubSymbol( new QgsMarkerSymbol( QgsSymbolLayerList() << markerLayer ) );

    std::unique_ptr< QgsSymbol > symbol = std::make_unique< QgsLineSymbol >( QgsSymbolLayerList() << lineSymbol );

    // set render units
    symbol->setOutputUnit( context.targetUnit() );
    lineSymbol->setOutputUnit( context.targetUnit() );

    rendererStyle.setGeometryType( Qgis::GeometryType::Line );
    rendererStyle.setSymbol( symbol.release() );
    return true;
  }
  else if ( jsonLayout.contains( u"icon-image"_s ) )
  {
    const QVariantMap jsonPaint = jsonLayer.value( u"paint"_s ).toMap();

    QSize spriteSize;
    QString spriteProperty, spriteSizeProperty;
    const QString sprite = retrieveSpriteAsBase64WithProperties( jsonLayout.value( u"icon-image"_s ), context, spriteSize, spriteProperty, spriteSizeProperty );
    if ( !sprite.isEmpty() || !spriteProperty.isEmpty() )
    {
      QgsRasterMarkerSymbolLayer *rasterMarker = new QgsRasterMarkerSymbolLayer( );
      rasterMarker->setPath( sprite );
      rasterMarker->setSize( spriteSize.width() );
      rasterMarker->setSizeUnit( context.targetUnit() );

      QgsPropertyCollection markerDdProperties;
      if ( !spriteProperty.isEmpty() )
      {
        markerDdProperties.setProperty( QgsSymbolLayer::Property::Name, QgsProperty::fromExpression( spriteProperty ) );
        markerDdProperties.setProperty( QgsSymbolLayer::Property::Width, QgsProperty::fromExpression( spriteSizeProperty ) );
      }

      if ( jsonLayout.contains( u"icon-size"_s ) )
      {
        const QVariant jsonIconSize = jsonLayout.value( u"icon-size"_s );
        double size = 1.0;
        QgsProperty property;
        switch ( jsonIconSize.userType() )
        {
          case QMetaType::Type::Int:
          case QMetaType::Type::LongLong:
          case QMetaType::Type::Double:
          {
            size = jsonIconSize.toDouble();
            if ( !spriteSizeProperty.isEmpty() )
            {
              markerDdProperties.setProperty( QgsSymbolLayer::Property::Width,
                                              QgsProperty::fromExpression( u"with_variable('marker_size',%1,%2*@marker_size)"_s.arg( spriteSizeProperty ).arg( size ) ) );
            }
            break;
          }

          case QMetaType::Type::QVariantMap:
            property = parseInterpolateByZoom( jsonIconSize.toMap(), context, 1, &size );
            break;

          case QMetaType::Type::QVariantList:
          case QMetaType::Type::QStringList:
            property = parseValueList( jsonIconSize.toList(), PropertyType::Numeric, context );
            break;
          default:
            context.pushWarning( QObject::tr( "%1: Skipping non-implemented icon-size type (%2)" ).arg( context.layerId(), QMetaType::typeName( static_cast<QMetaType::Type>( jsonIconSize.userType() ) ) ) );
            break;
        }
        rasterMarker->setSize( size * spriteSize.width() );
        if ( !property.expressionString().isEmpty() )
        {
          if ( !spriteSizeProperty.isEmpty() )
          {
            markerDdProperties.setProperty( QgsSymbolLayer::Property::Width,
                                            QgsProperty::fromExpression( u"with_variable('marker_size',%1,(%2)*@marker_size)"_s.arg( spriteSizeProperty ).arg( property.expressionString() ) ) );
          }
          else
          {
            markerDdProperties.setProperty( QgsSymbolLayer::Property::Width,
                                            QgsProperty::fromExpression( u"(%2)*%1"_s.arg( spriteSize.width() ).arg( property.expressionString() ) ) );
          }
        }
      }

      double rotation = 0.0;
      if ( jsonLayout.contains( u"icon-rotate"_s ) )
      {
        const QVariant jsonIconRotate = jsonLayout.value( u"icon-rotate"_s );
        switch ( jsonIconRotate.userType() )
        {
          case QMetaType::Type::Int:
          case QMetaType::Type::LongLong:
          case QMetaType::Type::Double:
            rotation = jsonIconRotate.toDouble();
            break;

          case QMetaType::Type::QVariantMap:
            markerDdProperties.setProperty( QgsSymbolLayer::Property::Angle, parseInterpolateByZoom( jsonIconRotate.toMap(), context, context.pixelSizeConversionFactor(), &rotation ) );
            break;

          case QMetaType::Type::QVariantList:
          case QMetaType::Type::QStringList:
            markerDdProperties.setProperty( QgsSymbolLayer::Property::Angle, parseValueList( jsonIconRotate.toList(), PropertyType::Numeric, context, context.pixelSizeConversionFactor(), 255, nullptr, &rotation ) );
            break;

          default:
            context.pushWarning( QObject::tr( "%1: Skipping unsupported icon-rotate type (%2)" ).arg( context.layerId(), QMetaType::typeName( static_cast<QMetaType::Type>( jsonIconRotate.userType() ) ) ) );
            break;
        }
      }

      double iconOpacity = -1.0;
      if ( jsonPaint.contains( u"icon-opacity"_s ) )
      {
        const QVariant jsonIconOpacity = jsonPaint.value( u"icon-opacity"_s );
        switch ( jsonIconOpacity.userType() )
        {
          case QMetaType::Type::Int:
          case QMetaType::Type::LongLong:
          case QMetaType::Type::Double:
            iconOpacity = jsonIconOpacity.toDouble();
            break;

          case QMetaType::Type::QVariantMap:
            markerDdProperties.setProperty( QgsSymbolLayer::Property::Opacity, parseInterpolateByZoom( jsonIconOpacity.toMap(), context, 100, &iconOpacity ) );
            break;

          case QMetaType::Type::QVariantList:
          case QMetaType::Type::QStringList:
            markerDdProperties.setProperty( QgsSymbolLayer::Property::Opacity, parseValueList( jsonIconOpacity.toList(), PropertyType::Numeric, context, 100, 255, nullptr, &iconOpacity ) );
            break;

          default:
            context.pushWarning( QObject::tr( "%1: Skipping unsupported icon-opacity type (%2)" ).arg( context.layerId(), QMetaType::typeName( static_cast<QMetaType::Type>( jsonIconOpacity.userType() ) ) ) );
            break;
        }
      }

      rasterMarker->setDataDefinedProperties( markerDdProperties );
      rasterMarker->setAngle( rotation );
      if ( iconOpacity >= 0 )
        rasterMarker->setOpacity( iconOpacity );

      QgsMarkerSymbol *markerSymbol = new QgsMarkerSymbol( QgsSymbolLayerList() << rasterMarker );
      rendererStyle.setSymbol( markerSymbol );
      rendererStyle.setGeometryType( Qgis::GeometryType::Point );
      return true;
    }
  }

  return false;
}

QgsProperty QgsMapBoxGlStyleConverter::parseInterpolateColorByZoom( const QVariantMap &json, QgsMapBoxGlStyleConversionContext &context, QColor *defaultColor )
{
  const double base = json.value( u"base"_s, u"1"_s ).toDouble();
  const QVariantList stops = json.value( u"stops"_s ).toList();
  if ( stops.empty() )
    return QgsProperty();

  QString caseString = u"CASE "_s;
  const QString colorComponent( "color_part(%1,'%2')" );

  for ( int i = 0; i < stops.length() - 1; ++i )
  {
    // step bottom zoom
    const QString bz = stops.at( i ).toList().value( 0 ).toString();
    // step top zoom
    const QString tz = stops.at( i + 1 ).toList().value( 0 ).toString();

    const QVariant bcVariant = stops.at( i ).toList().value( 1 );
    const QVariant tcVariant = stops.at( i + 1 ).toList().value( 1 );

    const QColor bottomColor = parseColor( bcVariant.toString(), context );
    const QColor topColor = parseColor( tcVariant.toString(), context );

    if ( i == 0 && bottomColor.isValid() )
    {
      int bcHue;
      int bcSat;
      int bcLight;
      int bcAlpha;
      colorAsHslaComponents( bottomColor, bcHue, bcSat, bcLight, bcAlpha );
      caseString += u"WHEN @vector_tile_zoom < %1 THEN color_hsla(%2, %3, %4, %5) "_s
                    .arg( bz ).arg( bcHue ).arg( bcSat ).arg( bcLight ).arg( bcAlpha );
    }

    if ( bottomColor.isValid() && topColor.isValid() )
    {
      int bcHue;
      int bcSat;
      int bcLight;
      int bcAlpha;
      colorAsHslaComponents( bottomColor, bcHue, bcSat, bcLight, bcAlpha );
      int tcHue;
      int tcSat;
      int tcLight;
      int tcAlpha;
      colorAsHslaComponents( topColor, tcHue, tcSat, tcLight, tcAlpha );
      caseString += QStringLiteral( "WHEN @vector_tile_zoom >= %1 AND @vector_tile_zoom < %2 THEN color_hsla("
                                    "%3, %4, %5, %6) " ).arg( bz, tz,
                                        interpolateExpression( bz.toDouble(), tz.toDouble(), bcHue, tcHue, base, 1, &context ),
                                        interpolateExpression( bz.toDouble(), tz.toDouble(), bcSat, tcSat, base, 1, &context ),
                                        interpolateExpression( bz.toDouble(), tz.toDouble(), bcLight, tcLight, base, 1, &context ),
                                        interpolateExpression( bz.toDouble(), tz.toDouble(), bcAlpha, tcAlpha, base, 1, &context ) );
    }
    else
    {
      const QString bottomColorExpr = parseColorExpression( bcVariant, context );
      const QString topColorExpr = parseColorExpression( tcVariant, context );

      caseString += QStringLiteral( "WHEN @vector_tile_zoom >= %1 AND @vector_tile_zoom < %2 THEN color_hsla("
                                    "%3, %4, %5, %6) " ).arg( bz, tz,
                                        interpolateExpression( bz.toDouble(), tz.toDouble(), colorComponent.arg( bottomColorExpr ).arg( "hsl_hue" ), colorComponent.arg( topColorExpr ).arg( "hsl_hue" ), base, 1, &context ),
                                        interpolateExpression( bz.toDouble(), tz.toDouble(), colorComponent.arg( bottomColorExpr ).arg( "hsl_saturation" ), colorComponent.arg( topColorExpr ).arg( "hsl_saturation" ), base, 1, &context ),
                                        interpolateExpression( bz.toDouble(), tz.toDouble(), colorComponent.arg( bottomColorExpr ).arg( "lightness" ), colorComponent.arg( topColorExpr ).arg( "lightness" ), base, 1, &context ),
                                        interpolateExpression( bz.toDouble(), tz.toDouble(), colorComponent.arg( bottomColorExpr ).arg( "alpha" ), colorComponent.arg( topColorExpr ).arg( "alpha" ), base, 1, &context ) );
    }
  }

  // top color
  const QString tz = stops.last().toList().value( 0 ).toString();
  const QVariant tcVariant = stops.last().toList().value( 1 );
  QColor topColor;
  if ( tcVariant.userType() == QMetaType::Type::QString )
  {
    topColor = parseColor( tcVariant, context );
    if ( topColor.isValid() )
    {
      int tcHue;
      int tcSat;
      int tcLight;
      int tcAlpha;
      colorAsHslaComponents( topColor, tcHue, tcSat, tcLight, tcAlpha );
      caseString += QStringLiteral( "WHEN @vector_tile_zoom >= %1 THEN color_hsla(%2, %3, %4, %5) "
                                    "ELSE color_hsla(%2, %3, %4, %5) END" ).arg( tz ).arg( tcHue ).arg( tcSat ).arg( tcLight ).arg( tcAlpha );
    }
  }
  else if ( tcVariant.userType() == QMetaType::QVariantList )
  {
    const QString topColorExpr = parseColorExpression( tcVariant, context );

    caseString += QStringLiteral( "WHEN @vector_tile_zoom >= %1 THEN color_hsla(%2, %3, %4, %5) "
                                  "ELSE color_hsla(%2, %3, %4, %5) END" ).arg( tz )
                  .arg( colorComponent.arg( topColorExpr ).arg( "hsl_hue" ) ).arg( colorComponent.arg( topColorExpr ).arg( "hsl_saturation" ) ).arg( colorComponent.arg( topColorExpr ).arg( "lightness" ) ).arg( colorComponent.arg( topColorExpr ).arg( "alpha" ) );
  }

  if ( !stops.empty() && defaultColor )
    *defaultColor = parseColor( stops.value( 0 ).toList().value( 1 ).toString(), context );

  return QgsProperty::fromExpression( caseString );
}

QgsProperty QgsMapBoxGlStyleConverter::parseInterpolateByZoom( const QVariantMap &json, QgsMapBoxGlStyleConversionContext &context, double multiplier, double *defaultNumber )
{
  const double base = json.value( u"base"_s, u"1"_s ).toDouble();
  const QVariantList stops = json.value( u"stops"_s ).toList();
  if ( stops.empty() )
    return QgsProperty();

  QString scaleExpression;
  if ( stops.size() <= 2 )
  {
    scaleExpression = interpolateExpression(
                        stops.value( 0 ).toList().value( 0 ).toDouble(), // zoomMin
                        stops.last().toList().value( 0 ).toDouble(), // zoomMax
                        stops.value( 0 ).toList().value( 1 ), // valueMin
                        stops.last().toList().value( 1 ), // valueMax
                        base, multiplier, &context );
  }
  else
  {
    scaleExpression = parseStops( base, stops, multiplier, context );
  }

  if ( !stops.empty() && defaultNumber )
    *defaultNumber = stops.value( 0 ).toList().value( 1 ).toDouble() * multiplier;

  return QgsProperty::fromExpression( scaleExpression );
}

QgsProperty QgsMapBoxGlStyleConverter::parseInterpolateOpacityByZoom( const QVariantMap &json, int maxOpacity, QgsMapBoxGlStyleConversionContext *contextPtr )
{
  QgsMapBoxGlStyleConversionContext context;
  if ( contextPtr )
  {
    context = *contextPtr;
  }
  const double base = json.value( u"base"_s, u"1"_s ).toDouble();
  const QVariantList stops = json.value( u"stops"_s ).toList();
  if ( stops.empty() )
    return QgsProperty();

  QString scaleExpression;
  if ( stops.length() <= 2 )
  {
    const QVariant bv = stops.value( 0 ).toList().value( 1 );
    const QVariant tv = stops.last().toList().value( 1 );
    double bottom = 0.0;
    double top = 0.0;
    const bool numeric = numericArgumentsOnly( bv, tv, bottom, top );
    scaleExpression = u"set_color_part(@symbol_color, 'alpha', %1)"_s
                      .arg( interpolateExpression(
                              stops.value( 0 ).toList().value( 0 ).toDouble(),
                              stops.last().toList().value( 0 ).toDouble(),
                              numeric ? QString::number( bottom * maxOpacity ) : QString( "(%1) * %2" ).arg( parseValue( bv, context ) ).arg( maxOpacity ),
                              numeric ? QString::number( top * maxOpacity ) : QString( "(%1) * %2" ).arg( parseValue( tv, context ) ).arg( maxOpacity ), base, 1, &context ) );
  }
  else
  {
    scaleExpression = parseOpacityStops( base, stops, maxOpacity, context );
  }
  return QgsProperty::fromExpression( scaleExpression );
}

QString QgsMapBoxGlStyleConverter::parseOpacityStops( double base, const QVariantList &stops, int maxOpacity, QgsMapBoxGlStyleConversionContext &context )
{
  QString caseString = u"CASE WHEN @vector_tile_zoom < %1 THEN set_color_part(@symbol_color, 'alpha', %2)"_s
                       .arg( stops.value( 0 ).toList().value( 0 ).toString() )
                       .arg( stops.value( 0 ).toList().value( 1 ).toDouble() * maxOpacity );

  for ( int i = 0; i < stops.size() - 1; ++i )
  {
    const QVariant bv = stops.value( i ).toList().value( 1 );
    const QVariant tv = stops.value( i + 1 ).toList().value( 1 );
    double bottom = 0.0;
    double top = 0.0;
    const bool numeric = numericArgumentsOnly( bv, tv, bottom, top );

    caseString += QStringLiteral( " WHEN @vector_tile_zoom >= %1 AND @vector_tile_zoom < %2 "
                                  "THEN set_color_part(@symbol_color, 'alpha', %3)" )
                  .arg( stops.value( i ).toList().value( 0 ).toString(),
                        stops.value( i + 1 ).toList().value( 0 ).toString(),
                        interpolateExpression(
                          stops.value( i ).toList().value( 0 ).toDouble(),
                          stops.value( i + 1 ).toList().value( 0 ).toDouble(),
                          numeric ? QString::number( bottom * maxOpacity ) : QString( "(%1) * %2" ).arg( parseValue( bv, context ) ).arg( maxOpacity ),
                          numeric ? QString::number( top * maxOpacity ) : QString( "(%1) * %2" ).arg( parseValue( tv, context ) ).arg( maxOpacity ),
                          base, 1, &context ) );
  }


  bool numeric = false;
  const QVariant vv = stops.last().toList().value( 1 );
  double dv = vv.toDouble( &numeric );

  caseString += QStringLiteral( " WHEN @vector_tile_zoom >= %1 "
                                "THEN set_color_part(@symbol_color, 'alpha', %2) END" ).arg(
                  stops.last().toList().value( 0 ).toString(),
                  numeric ? QString::number( dv * maxOpacity ) : QString( "(%1) * %2" ).arg( parseValue( vv, context ) ).arg( maxOpacity )
                );
  return caseString;
}

QgsProperty QgsMapBoxGlStyleConverter::parseInterpolatePointByZoom( const QVariantMap &json, QgsMapBoxGlStyleConversionContext &context, double multiplier, QPointF *defaultPoint )
{
  const double base = json.value( u"base"_s, u"1"_s ).toDouble();
  const QVariantList stops = json.value( u"stops"_s ).toList();
  if ( stops.empty() )
    return QgsProperty();

  QString scaleExpression;
  if ( stops.size() <= 2 )
  {
    scaleExpression = u"array(%1,%2)"_s.arg( interpolateExpression( stops.value( 0 ).toList().value( 0 ).toDouble(),
                      stops.last().toList().value( 0 ).toDouble(),
                      stops.value( 0 ).toList().value( 1 ).toList().value( 0 ),
                      stops.last().toList().value( 1 ).toList().value( 0 ), base, multiplier, &context ),
                      interpolateExpression( stops.value( 0 ).toList().value( 0 ).toDouble(),
                          stops.last().toList().value( 0 ).toDouble(),
                          stops.value( 0 ).toList().value( 1 ).toList().value( 1 ),
                          stops.last().toList().value( 1 ).toList().value( 1 ), base, multiplier, &context )
                                           );
  }
  else
  {
    scaleExpression = parsePointStops( base, stops, context, multiplier );
  }

  if ( !stops.empty() && defaultPoint )
    *defaultPoint = QPointF( stops.value( 0 ).toList().value( 1 ).toList().value( 0 ).toDouble() * multiplier,
                             stops.value( 0 ).toList().value( 1 ).toList().value( 1 ).toDouble() * multiplier );

  return QgsProperty::fromExpression( scaleExpression );
}

QgsProperty QgsMapBoxGlStyleConverter::parseInterpolateStringByZoom( const QVariantMap &json, QgsMapBoxGlStyleConversionContext &context,
    const QVariantMap &conversionMap, QString *defaultString )
{
  const QVariantList stops = json.value( u"stops"_s ).toList();
  if ( stops.empty() )
    return QgsProperty();

  const QString scaleExpression = parseStringStops( stops, context, conversionMap, defaultString );

  return QgsProperty::fromExpression( scaleExpression );
}

QString QgsMapBoxGlStyleConverter::parsePointStops( double base, const QVariantList &stops, QgsMapBoxGlStyleConversionContext &context, double multiplier )
{
  QString caseString = u"CASE "_s;

  for ( int i = 0; i < stops.length() - 1; ++i )
  {
    // bottom zoom and value
    const QVariant bz = stops.value( i ).toList().value( 0 );
    const QVariant bv = stops.value( i ).toList().value( 1 );
    if ( bv.userType() != QMetaType::Type::QVariantList && bv.userType() != QMetaType::Type::QStringList )
    {
      context.pushWarning( QObject::tr( "%1: Skipping unsupported offset interpolation type (%2)." ).arg( context.layerId(), QMetaType::typeName( static_cast<QMetaType::Type>( bz.userType() ) ) ) );
      return QString();
    }

    // top zoom and value
    const QVariant tz = stops.value( i + 1 ).toList().value( 0 );
    const QVariant tv = stops.value( i + 1 ).toList().value( 1 );
    if ( tv.userType() != QMetaType::Type::QVariantList && tv.userType() != QMetaType::Type::QStringList )
    {
      context.pushWarning( QObject::tr( "%1: Skipping unsupported offset interpolation type (%2)." ).arg( context.layerId(), QMetaType::typeName( static_cast<QMetaType::Type>( tz.userType() ) ) ) );
      return QString();
    }

    caseString += QStringLiteral( "WHEN @vector_tile_zoom > %1 AND @vector_tile_zoom <= %2 "
                                  "THEN array(%3,%4)" ).arg( bz.toString(),
                                      tz.toString(),
                                      interpolateExpression( bz.toDouble(), tz.toDouble(), bv.toList().value( 0 ), tv.toList().value( 0 ), base, multiplier, &context ),
                                      interpolateExpression( bz.toDouble(), tz.toDouble(), bv.toList().value( 1 ), tv.toList().value( 1 ), base, multiplier, &context ) );
  }
  caseString += "END"_L1;
  return caseString;
}

QString QgsMapBoxGlStyleConverter::parseArrayStops( const QVariantList &stops, QgsMapBoxGlStyleConversionContext &, double multiplier )
{
  if ( stops.length() < 2 )
    return QString();

  QString caseString = u"CASE"_s;

  for ( int i = 0; i < stops.length(); ++i )
  {
    caseString += " WHEN "_L1;
    QStringList conditions;
    if ( i > 0 )
    {
      const QVariant bottomZoom = stops.value( i ).toList().value( 0 );
      conditions << u"@vector_tile_zoom > %1"_s.arg( bottomZoom.toString() );
    }
    if ( i < stops.length() - 1 )
    {
      const QVariant topZoom = stops.value( i + 1 ).toList().value( 0 );
      conditions << u"@vector_tile_zoom <= %1"_s.arg( topZoom.toString() );
    }

    const QVariantList values = stops.value( i ).toList().value( 1 ).toList();
    QStringList valuesFixed;
    bool ok = false;
    for ( const QVariant &value : values )
    {
      const double number = value.toDouble( &ok );
      if ( ok )
        valuesFixed << QString::number( number * multiplier );
    }

    // top zoom and value
    caseString += u"%1 THEN array(%3)"_s.arg(
                    conditions.join( " AND "_L1 ),
                    valuesFixed.join( ',' )
                  );
  }
  caseString += " END"_L1;
  return caseString;
}

QString QgsMapBoxGlStyleConverter::parseStops( double base, const QVariantList &stops, double multiplier, QgsMapBoxGlStyleConversionContext &context )
{
  QString caseString = u"CASE "_s;

  for ( int i = 0; i < stops.length() - 1; ++i )
  {
    // bottom zoom and value
    const QVariant bz = stops.value( i ).toList().value( 0 );
    const QVariant bv = stops.value( i ).toList().value( 1 );
    if ( bz.userType() == QMetaType::Type::QVariantList || bz.userType() == QMetaType::Type::QStringList )
    {
      context.pushWarning( QObject::tr( "%1: Expressions in interpolation function are not supported, skipping." ).arg( context.layerId() ) );
      return QString();
    }

    // top zoom and value
    const QVariant tz = stops.value( i + 1 ).toList().value( 0 );
    const QVariant tv = stops.value( i + 1 ).toList().value( 1 );
    if ( tz.userType() == QMetaType::Type::QVariantList || tz.userType() == QMetaType::Type::QStringList )
    {
      context.pushWarning( QObject::tr( "%1: Expressions in interpolation function are not supported, skipping." ).arg( context.layerId() ) );
      return QString();
    }

    const QString lowerComparator = i == 0 ? u">="_s : u">"_s;

    caseString += QStringLiteral( "WHEN @vector_tile_zoom %1 %2 AND @vector_tile_zoom <= %3 "
                                  "THEN %4 " ).arg( lowerComparator,
                                      bz.toString(),
                                      tz.toString(),
                                      interpolateExpression( bz.toDouble(), tz.toDouble(), bv, tv, base, multiplier, &context ) );
  }

  const QVariant z = stops.last().toList().value( 0 );
  const QVariant v = stops.last().toList().value( 1 );
  QString vStr = v.toString();
  if ( ( QMetaType::Type )v.userType() == QMetaType::QVariantList )
  {
    vStr = parseExpression( v.toList(), context );
    caseString += QStringLiteral( "WHEN @vector_tile_zoom > %1 "
                                  "THEN ( ( %2 ) * %3 ) END" ).arg( z.toString() ).arg( vStr ).arg( multiplier );
  }
  else
  {
    caseString += QStringLiteral( "WHEN @vector_tile_zoom > %1 "
                                  "THEN %2 END" ).arg( z.toString() ).arg( v.toDouble() * multiplier );
  }

  return caseString;
}

QString QgsMapBoxGlStyleConverter::parseStringStops( const QVariantList &stops, QgsMapBoxGlStyleConversionContext &context, const QVariantMap &conversionMap, QString *defaultString )
{
  QString caseString = u"CASE "_s;

  for ( int i = 0; i < stops.length() - 1; ++i )
  {
    // bottom zoom and value
    const QVariant bz = stops.value( i ).toList().value( 0 );
    const QString bv = stops.value( i ).toList().value( 1 ).toString();
    if ( bz.userType() == QMetaType::Type::QVariantList || bz.userType() == QMetaType::Type::QStringList )
    {
      context.pushWarning( QObject::tr( "%1: Expressions in interpolation function are not supported, skipping." ).arg( context.layerId() ) );
      return QString();
    }

    // top zoom
    const QVariant tz = stops.value( i + 1 ).toList().value( 0 );
    if ( tz.userType() == QMetaType::Type::QVariantList || tz.userType() == QMetaType::Type::QStringList )
    {
      context.pushWarning( QObject::tr( "%1: Expressions in interpolation function are not supported, skipping." ).arg( context.layerId() ) );
      return QString();
    }

    caseString += QStringLiteral( "WHEN @vector_tile_zoom > %1 AND @vector_tile_zoom <= %2 "
                                  "THEN %3 " ).arg( bz.toString(),
                                      tz.toString(),
                                      QgsExpression::quotedValue( conversionMap.value( bv, bv ) ) );
  }
  caseString += u"ELSE %1 END"_s.arg( QgsExpression::quotedValue( conversionMap.value( stops.constLast().toList().value( 1 ).toString(),
                                      stops.constLast().toList().value( 1 ) ) ) );
  if ( defaultString )
    *defaultString = stops.constLast().toList().value( 1 ).toString();
  return caseString;
}

QString QgsMapBoxGlStyleConverter::parseLabelStops( const QVariantList &stops, QgsMapBoxGlStyleConversionContext &context )
{
  QString caseString = u"CASE "_s;

  bool isExpression = false;
  for ( int i = 0; i < stops.length() - 1; ++i )
  {
    // bottom zoom and value
    const QVariant bz = stops.value( i ).toList().value( 0 );
    if ( bz.userType() == QMetaType::Type::QVariantList || bz.userType() == QMetaType::Type::QStringList )
    {
      context.pushWarning( QObject::tr( "%1: Lists in label interpolation function are not supported, skipping." ).arg( context.layerId() ) );
      return QString();
    }

    // top zoom
    const QVariant tz = stops.value( i + 1 ).toList().value( 0 );
    if ( tz.userType() == QMetaType::Type::QVariantList || tz.userType() == QMetaType::Type::QStringList )
    {
      context.pushWarning( QObject::tr( "%1: Lists in label interpolation function are not supported, skipping." ).arg( context.layerId() ) );
      return QString();
    }

    QString fieldPart = processLabelField( stops.constLast().toList().value( 1 ).toString(), isExpression );
    if ( fieldPart.isEmpty() )
      fieldPart = u"''"_s;
    else if ( !isExpression )
      fieldPart = QgsExpression::quotedColumnRef( fieldPart );

    caseString += QStringLiteral( "WHEN @vector_tile_zoom > %1 AND @vector_tile_zoom < %2 "
                                  "THEN %3 " ).arg( bz.toString(),
                                      tz.toString(),
                                      fieldPart ) ;
  }

  {
    const QVariant bz = stops.constLast().toList().value( 0 );
    if ( bz.userType() == QMetaType::Type::QVariantList || bz.userType() == QMetaType::Type::QStringList )
    {
      context.pushWarning( QObject::tr( "%1: Lists in label interpolation function are not supported, skipping." ).arg( context.layerId() ) );
      return QString();
    }

    QString fieldPart = processLabelField( stops.constLast().toList().value( 1 ).toString(), isExpression );
    if ( fieldPart.isEmpty() )
      fieldPart = u"''"_s;
    else if ( !isExpression )
      fieldPart = QgsExpression::quotedColumnRef( fieldPart );

    caseString += QStringLiteral( "WHEN @vector_tile_zoom >= %1 "
                                  "THEN %3 " ).arg( bz.toString(),
                                      fieldPart ) ;
  }

  QString defaultPart = processLabelField( stops.constFirst().toList().value( 1 ).toString(), isExpression );
  if ( defaultPart.isEmpty() )
    defaultPart = u"''"_s;
  else if ( !isExpression )
    defaultPart = QgsExpression::quotedColumnRef( defaultPart );
  caseString += u"ELSE %1 END"_s.arg( defaultPart );

  return caseString;
}

QgsProperty QgsMapBoxGlStyleConverter::parseValueList( const QVariantList &json, QgsMapBoxGlStyleConverter::PropertyType type, QgsMapBoxGlStyleConversionContext &context, double multiplier, int maxOpacity, QColor *defaultColor, double *defaultNumber )
{
  const QString method = json.value( 0 ).toString();
  if ( method == "interpolate"_L1 )
  {
    return parseInterpolateListByZoom( json, type, context, multiplier, maxOpacity, defaultColor, defaultNumber );
  }
  else if ( method == "match"_L1 )
  {
    return parseMatchList( json, type, context, multiplier, maxOpacity, defaultColor, defaultNumber );
  }
  else if ( method == "step"_L1 )
  {
    return parseStepList( json, type, context, multiplier, maxOpacity, defaultColor, defaultNumber );
  }
  else
  {
    return QgsProperty::fromExpression( parseExpression( json, context ) );
  }
}

QgsProperty QgsMapBoxGlStyleConverter::parseMatchList( const QVariantList &json, QgsMapBoxGlStyleConverter::PropertyType type, QgsMapBoxGlStyleConversionContext &context, double multiplier, int maxOpacity, QColor *defaultColor, double *defaultNumber )
{
  const QString attribute = parseExpression( json.value( 1 ).toList(), context );
  if ( attribute.isEmpty() )
  {
    context.pushWarning( QObject::tr( "%1: Could not interpret match list" ).arg( context.layerId() ) );
    return QgsProperty();
  }

  QString caseString = u"CASE "_s;

  for ( int i = 2; i < json.length() - 1; i += 2 )
  {
    QVariantList keys;
    QVariant variantKeys = json.value( i );
    if ( variantKeys.userType() == QMetaType::Type::QVariantList || variantKeys.userType() == QMetaType::Type::QStringList )
      keys = variantKeys.toList();
    else
      keys = {variantKeys};

    QStringList matchString;
    for ( const QVariant &key : keys )
    {
      matchString << QgsExpression::quotedValue( key );
    }

    const QVariant value = json.value( i + 1 );

    QString valueString;
    switch ( type )
    {
      case PropertyType::Color:
      {
        if ( value.userType() == QMetaType::Type::QVariantList || value.userType() == QMetaType::Type::QStringList )
        {
          valueString = parseMatchList( value.toList(), PropertyType::Color, context, multiplier, maxOpacity, defaultColor, defaultNumber ).asExpression();
        }
        else
        {
          const QColor color = parseColor( value, context );
          valueString = QgsExpression::quotedString( color.name() );
        }
        break;
      }

      case PropertyType::Numeric:
      {
        const double v = value.toDouble() * multiplier;
        valueString = QString::number( v );
        break;
      }

      case PropertyType::Opacity:
      {
        const double v = value.toDouble() * maxOpacity;
        valueString = QString::number( v );
        break;
      }

      case PropertyType::Point:
      {
        valueString = u"array(%1,%2)"_s.arg( value.toList().value( 0 ).toDouble() * multiplier,
                                             value.toList().value( 0 ).toDouble() * multiplier );
        break;
      }

      case PropertyType::NumericArray:
      {
        if ( value.toList().count() == 2 && value.toList().first().toString() == "literal"_L1 )
        {
          valueString = u"array(%1)"_s.arg( value.toList().at( 1 ).toStringList().join( ',' ) );
        }
        else
        {
          valueString = u"array(%1)"_s.arg( value.toStringList().join( ',' ) );
        }
        break;
      }
    }

    if ( matchString.count() == 1 )
    {
      caseString += u"WHEN %1 IS %2 THEN %3 "_s.arg( attribute, matchString.at( 0 ), valueString );
    }
    else
    {
      caseString += u"WHEN %1 IN (%2) THEN %3 "_s.arg( attribute, matchString.join( ',' ), valueString );
    }
  }

  QVariant lastValue = json.constLast();
  QString elseValue;

  switch ( lastValue.userType() )
  {
    case QMetaType::Type::QVariantList:
    case QMetaType::Type::QStringList:
      elseValue = parseValueList( lastValue.toList(), type, context, multiplier, maxOpacity, defaultColor, defaultNumber ).asExpression();
      break;

    default:
    {
      switch ( type )
      {
        case PropertyType::Color:
        {
          const QColor color = parseColor( lastValue, context );
          if ( defaultColor )
            *defaultColor = color;

          elseValue = QgsExpression::quotedString( color.name() );
          break;
        }

        case PropertyType::Numeric:
        {
          const double v = json.constLast().toDouble() * multiplier;
          if ( defaultNumber )
            *defaultNumber = v;
          elseValue = QString::number( v );
          break;
        }

        case PropertyType::Opacity:
        {
          const double v = json.constLast().toDouble() * maxOpacity;
          if ( defaultNumber )
            *defaultNumber = v;
          elseValue = QString::number( v );
          break;
        }

        case PropertyType::Point:
        {
          elseValue = u"array(%1,%2)"_s
                      .arg( json.constLast().toList().value( 0 ).toDouble() * multiplier )
                      .arg( json.constLast().toList().value( 0 ).toDouble() * multiplier );
          break;
        }

        case PropertyType::NumericArray:
        {
          if ( json.constLast().toList().count() == 2 && json.constLast().toList().first().toString() == "literal"_L1 )
          {
            elseValue = u"array(%1)"_s.arg( json.constLast().toList().at( 1 ).toStringList().join( ',' ) );
          }
          else
          {
            elseValue = u"array(%1)"_s.arg( json.constLast().toStringList().join( ',' ) );
          }
          break;
        }

      }
      break;
    }
  }

  caseString += u"ELSE %1 END"_s.arg( elseValue );
  return QgsProperty::fromExpression( caseString );
}

QgsProperty QgsMapBoxGlStyleConverter::parseStepList( const QVariantList &json, PropertyType type, QgsMapBoxGlStyleConversionContext &context, double multiplier, int maxOpacity, QColor *defaultColor, double *defaultNumber )
{
  const QString expression = parseExpression( json.value( 1 ).toList(), context );
  if ( expression.isEmpty() )
  {
    context.pushWarning( QObject::tr( "%1: Could not interpret step list" ).arg( context.layerId() ) );
    return QgsProperty();
  }

  QString caseString = u"CASE "_s;


  for ( int i = json.length() - 2; i > 0; i -= 2 )
  {
    const QVariant stepValue = json.value( i + 1 );

    QString valueString;
    if ( stepValue.canConvert<QVariantList>()
         && ( stepValue.toList().count() != 2 || type != PropertyType::Point )
         && type != PropertyType::NumericArray )
    {
      valueString = parseValueList( stepValue.toList(), type, context, multiplier, maxOpacity, defaultColor, defaultNumber ).expressionString();
    }
    else
    {
      switch ( type )
      {
        case PropertyType::Color:
        {
          const QColor color = parseColor( stepValue, context );
          valueString = QgsExpression::quotedString( color.name() );
          break;
        }

        case PropertyType::Numeric:
        {
          const double v = stepValue.toDouble() * multiplier;
          valueString = QString::number( v );
          break;
        }

        case PropertyType::Opacity:
        {
          const double v = stepValue.toDouble() * maxOpacity;
          valueString = QString::number( v );
          break;
        }

        case PropertyType::Point:
        {
          valueString = u"array(%1,%2)"_s.arg(
                          stepValue.toList().value( 0 ).toDouble() * multiplier ).arg(
                          stepValue.toList().value( 0 ).toDouble() * multiplier
                        );
          break;
        }

        case PropertyType::NumericArray:
        {
          if ( stepValue.toList().count() == 2 && stepValue.toList().first().toString() == "literal"_L1 )
          {
            valueString = u"array(%1)"_s.arg( stepValue.toList().at( 1 ).toStringList().join( ',' ) );
          }
          else
          {
            valueString = u"array(%1)"_s.arg( stepValue.toStringList().join( ',' ) );
          }
          break;
        }
      }
    }

    if ( i > 1 )
    {
      const QString stepKey = QgsExpression::quotedValue( json.value( i ) );
      caseString += u" WHEN %1 >= %2 THEN (%3) "_s.arg( expression, stepKey, valueString );
    }
    else
    {
      caseString += u"ELSE (%1) END"_s.arg( valueString );
    }
  }
  return QgsProperty::fromExpression( caseString );
}

QgsProperty QgsMapBoxGlStyleConverter::parseInterpolateListByZoom( const QVariantList &json, PropertyType type, QgsMapBoxGlStyleConversionContext &context, double multiplier, int maxOpacity, QColor *defaultColor, double *defaultNumber )
{
  if ( json.value( 0 ).toString() != "interpolate"_L1 )
  {
    context.pushWarning( QObject::tr( "%1: Could not interpret value list" ).arg( context.layerId() ) );
    return QgsProperty();
  }

  double base = 1;
  const QString technique = json.value( 1 ).toList().value( 0 ).toString();
  if ( technique == "linear"_L1 )
    base = 1;
  else if ( technique == "exponential"_L1 )
    base = json.value( 1 ).toList(). value( 1 ).toDouble();
  else if ( technique == "cubic-bezier"_L1 )
  {
    context.pushWarning( QObject::tr( "%1: Cubic-bezier interpolation is not supported, linear used instead." ).arg( context.layerId() ) );
    base = 1;
  }
  else
  {
    context.pushWarning( QObject::tr( "%1: Skipping not implemented interpolation method %2" ).arg( context.layerId(), technique ) );
    return QgsProperty();
  }

  if ( json.value( 2 ).toList().value( 0 ).toString() != "zoom"_L1 )
  {
    context.pushWarning( QObject::tr( "%1: Skipping not implemented interpolation input %2" ).arg( context.layerId(), json.value( 2 ).toString() ) );
    return QgsProperty();
  }

  //  Convert stops into list of lists
  QVariantList stops;
  for ( int i = 3; i < json.length(); i += 2 )
  {
    stops.push_back( QVariantList() << json.value( i ).toString() << json.value( i + 1 ) );
  }

  QVariantMap props;
  props.insert( u"stops"_s, stops );
  props.insert( u"base"_s, base );
  switch ( type )
  {
    case PropertyType::Color:
      return parseInterpolateColorByZoom( props, context, defaultColor );

    case PropertyType::Numeric:
      return parseInterpolateByZoom( props, context, multiplier, defaultNumber );

    case PropertyType::Opacity:
      return parseInterpolateOpacityByZoom( props, maxOpacity, &context );

    case PropertyType::Point:
      return parseInterpolatePointByZoom( props, context, multiplier );

    case PropertyType::NumericArray:
      context.pushWarning( QObject::tr( "%1: Skipping unsupported numeric array in interpolate" ).arg( context.layerId() ) );
      return QgsProperty();

  }
  return QgsProperty();
}

QString QgsMapBoxGlStyleConverter::parseColorExpression( const QVariant &colorExpression, QgsMapBoxGlStyleConversionContext &context )
{
  if ( ( QMetaType::Type )colorExpression.userType() == QMetaType::QVariantList )
  {
    return parseExpression( colorExpression.toList(), context, true );
  }
  return parseValue( colorExpression, context, true );
}

QColor QgsMapBoxGlStyleConverter::parseColor( const QVariant &color, QgsMapBoxGlStyleConversionContext &context )
{
  if ( color.userType() != QMetaType::Type::QString )
  {
    context.pushWarning( QObject::tr( "%1: Could not parse non-string color %2, skipping" ).arg( context.layerId(), color.toString() ) );
    return QColor();
  }

  return QgsSymbolLayerUtils::parseColor( color.toString() );
}

void QgsMapBoxGlStyleConverter::colorAsHslaComponents( const QColor &color, int &hue, int &saturation, int &lightness, int &alpha )
{
  hue = std::max( 0, color.hslHue() );
  saturation = color.hslSaturation() / 255.0 * 100;
  lightness = color.lightness() / 255.0 * 100;
  alpha = color.alpha();
}

QString QgsMapBoxGlStyleConverter::interpolateExpression( double zoomMin, double zoomMax, QVariant valueMin, QVariant valueMax, double base, double multiplier, QgsMapBoxGlStyleConversionContext *contextPtr )
{
  QgsMapBoxGlStyleConversionContext context;
  if ( contextPtr )
  {
    context = *contextPtr;
  }

  // special case where min = max !
  if ( valueMin.canConvert( QMetaType::Double ) && valueMax.canConvert( QMetaType::Double ) )
  {
    bool minDoubleOk = true;
    const double min = valueMin.toDouble( &minDoubleOk );
    bool maxDoubleOk = true;
    const double max = valueMax.toDouble( &maxDoubleOk );
    if ( minDoubleOk && maxDoubleOk && qgsDoubleNear( min, max ) )
    {
      return QString::number( min * multiplier );
    }
  }

  QString minValueExpr = valueMin.toString();
  QString maxValueExpr = valueMax.toString();
  if ( valueMin.userType() == QMetaType::Type::QVariantList )
  {
    minValueExpr = parseExpression( valueMin.toList(), context );
  }
  if ( valueMax.userType() == QMetaType::Type::QVariantList )
  {
    maxValueExpr = parseExpression( valueMax.toList(), context );
  }

  QString expression;
  if ( minValueExpr == maxValueExpr )
  {
    expression = minValueExpr;
  }
  else
  {
    if ( base == 1 )
    {
      expression = u"scale_linear(@vector_tile_zoom,%1,%2,%3,%4)"_s.arg( zoomMin ).arg( zoomMax ).arg( minValueExpr ).arg( maxValueExpr );
    }
    else
    {
      expression = u"scale_exponential(@vector_tile_zoom,%1,%2,%3,%4,%5)"_s.arg( zoomMin ).arg( zoomMax ).arg( minValueExpr ).arg( maxValueExpr ).arg( base );
    }
  }

  if ( multiplier != 1 )
    return u"(%1) * %2"_s.arg( expression ).arg( multiplier );
  else
    return expression;
}

Qt::PenCapStyle QgsMapBoxGlStyleConverter::parseCapStyle( const QString &style )
{
  if ( style == "round"_L1 )
    return Qt::RoundCap;
  else if ( style == "square"_L1 )
    return Qt::SquareCap;
  else
    return Qt::FlatCap; // "butt" is default
}

Qt::PenJoinStyle QgsMapBoxGlStyleConverter::parseJoinStyle( const QString &style )
{
  if ( style == "bevel"_L1 )
    return Qt::BevelJoin;
  else if ( style == "round"_L1 )
    return Qt::RoundJoin;
  else
    return Qt::MiterJoin; // "miter" is default
}

QString QgsMapBoxGlStyleConverter::parseExpression( const QVariantList &expression, QgsMapBoxGlStyleConversionContext &context, bool colorExpected )
{
  QString op = expression.value( 0 ).toString();
  if ( op == "%"_L1 && expression.size() >= 3 )
  {
    return u"%1 %2 %3"_s.arg( parseValue( expression.value( 1 ), context ),
                              op,
                              parseValue( expression.value( 2 ), context ) );
  }
  else if ( op == "to-number"_L1 )
  {
    return u"to_real(%1)"_s.arg( parseValue( expression.value( 1 ), context ) );
  }
  if ( op == "literal"_L1 )
  {
    return expression.value( 1 ).toString();
  }
  else if ( op == "all"_L1
            || op == "any"_L1
            || op == "none"_L1 )
  {
    QStringList parts;
    for ( int i = 1; i < expression.size(); ++i )
    {
      const QString part = parseValue( expression.at( i ), context );
      if ( part.isEmpty() )
      {
        context.pushWarning( QObject::tr( "%1: Skipping unsupported expression" ).arg( context.layerId() ) );
        return QString();
      }
      parts << part;
    }

    if ( op == "none"_L1 )
      return u"NOT (%1)"_s.arg( parts.join( ") AND NOT ("_L1 ) );

    QString operatorString;
    if ( op == "all"_L1 )
      operatorString = u") AND ("_s;
    else if ( op == "any"_L1 )
      operatorString = u") OR ("_s;

    return u"(%1)"_s.arg( parts.join( operatorString ) );
  }
  else if ( op == '!' )
  {
    // ! inverts next expression's meaning
    QVariantList contraJsonExpr = expression.value( 1 ).toList();
    contraJsonExpr[0] = QString( op + contraJsonExpr[0].toString() );
    // ['!', ['has', 'level']] -> ['!has', 'level']
    return parseKey( contraJsonExpr, context );
  }
  else if ( op == "=="_L1
            || op == "!="_L1
            || op == ">="_L1
            || op == '>'
            || op == "<="_L1
            || op == '<' )
  {
    // use IS and NOT IS instead of = and != because they can deal with NULL values
    if ( op == "=="_L1 )
      op = u"IS"_s;
    else if ( op == "!="_L1 )
      op = u"IS NOT"_s;
    return u"%1 %2 %3"_s.arg( parseKey( expression.value( 1 ), context ),
                              op, parseValue( expression.value( 2 ), context ) );
  }
  else if ( op == "has"_L1 )
  {
    return parseKey( expression.value( 1 ), context ) + u" IS NOT NULL"_s;
  }
  else if ( op == "!has"_L1 )
  {
    return parseKey( expression.value( 1 ), context ) + u" IS NULL"_s;
  }
  else if ( op == "in"_L1 || op == "!in"_L1 )
  {
    const QString key = parseKey( expression.value( 1 ), context );
    QStringList parts;

    QVariantList values = expression.mid( 2 );
    if ( expression.size() == 3
         && expression.at( 2 ).userType() == QMetaType::Type::QVariantList && expression.at( 2 ).toList().count() > 1
         && expression.at( 2 ).toList().at( 0 ).toString() == "literal"_L1 )
    {
      values = expression.at( 2 ).toList().at( 1 ).toList();
    }

    for ( const QVariant &value : std::as_const( values ) )
    {
      const QString part = parseValue( value, context );
      if ( part.isEmpty() )
      {
        context.pushWarning( QObject::tr( "%1: Skipping unsupported expression" ).arg( context.layerId() ) );
        return QString();
      }
      parts << part;
    }

    if ( parts.size() == 1 )
    {
      if ( op == "in"_L1 )
        return u"%1 IS %2"_s.arg( key, parts.at( 0 ) );
      else
        return u"(%1 IS NULL OR %1 IS NOT %2)"_s.arg( key, parts.at( 0 ) );
    }
    else
    {
      if ( op == "in"_L1 )
        return u"%1 IN (%2)"_s.arg( key, parts.join( ", "_L1 ) );
      else
        return u"(%1 IS NULL OR %1 NOT IN (%2))"_s.arg( key, parts.join( ", "_L1 ) );
    }
  }
  else if ( op == "get"_L1 )
  {
    return parseKey( expression.value( 1 ), context );
  }
  else if ( op == "match"_L1 )
  {
    const QString attribute = expression.value( 1 ).toList().value( 1 ).toString();

    if ( expression.size() == 5
         && expression.at( 3 ).userType() == QMetaType::Type::Bool && expression.at( 3 ).toBool() == true
         && expression.at( 4 ).userType() == QMetaType::Type::Bool && expression.at( 4 ).toBool() == false )
    {
      // simple case, make a nice simple expression instead of a CASE statement
      if ( expression.at( 2 ).userType() == QMetaType::Type::QVariantList || expression.at( 2 ).userType() == QMetaType::Type::QStringList )
      {
        QStringList parts;
        for ( const QVariant &p : expression.at( 2 ).toList() )
        {
          parts << parseValue( p, context );
        }

        if ( parts.size() > 1 )
          return u"%1 IN (%2)"_s.arg( QgsExpression::quotedColumnRef( attribute ), parts.join( ", " ) );
        else
          return QgsExpression::createFieldEqualityExpression( attribute, expression.at( 2 ).toList().value( 0 ) );
      }
      else if ( expression.at( 2 ).userType() == QMetaType::Type::QString || expression.at( 2 ).userType() == QMetaType::Type::Int
                || expression.at( 2 ).userType() == QMetaType::Type::Double || expression.at( 2 ).userType() == QMetaType::Type::LongLong )
      {
        return QgsExpression::createFieldEqualityExpression( attribute, expression.at( 2 ) );
      }
      else
      {
        context.pushWarning( QObject::tr( "%1: Skipping unsupported expression" ).arg( context.layerId() ) );
        return QString();
      }
    }
    else
    {
      QString caseString = u"CASE "_s;
      for ( int i = 2; i < expression.size() - 2; i += 2 )
      {
        if ( expression.at( i ).userType() == QMetaType::Type::QVariantList || expression.at( i ).userType() == QMetaType::Type::QStringList )
        {
          QStringList parts;
          for ( const QVariant &p : expression.at( i ).toList() )
          {
            parts << QgsExpression::quotedValue( p );
          }

          if ( parts.size() > 1 )
            caseString += u"WHEN %1 IN (%2) "_s.arg( QgsExpression::quotedColumnRef( attribute ), parts.join( ", " ) );
          else
            caseString += u"WHEN %1 "_s.arg( QgsExpression::createFieldEqualityExpression( attribute, expression.at( i ).toList().value( 0 ) ) );
        }
        else if ( expression.at( i ).userType() == QMetaType::Type::QString || expression.at( i ).userType() == QMetaType::Type::Int
                  || expression.at( i ).userType() == QMetaType::Type::Double || expression.at( i ).userType() == QMetaType::Type::LongLong )
        {
          caseString += u"WHEN (%1) "_s.arg( QgsExpression::createFieldEqualityExpression( attribute, expression.at( i ) ) );
        }

        caseString += u"THEN %1 "_s.arg( parseValue( expression.at( i + 1 ), context, colorExpected ) );
      }
      caseString += u"ELSE %1 END"_s.arg( parseValue( expression.last(), context, colorExpected ) );
      return caseString;
    }
  }
  else if ( op == "to-string"_L1 )
  {
    return u"to_string(%1)"_s.arg( parseExpression( expression.value( 1 ).toList(), context ) );
  }
  else if ( op == "to-boolean"_L1 )
  {
    return u"to_bool(%1)"_s.arg( parseExpression( expression.value( 1 ).toList(), context ) );
  }
  else if ( op == "case"_L1 )
  {
    QString caseString = u"CASE"_s;
    for ( int i = 1; i < expression.size() - 2; i += 2 )
    {
      const QString condition = parseExpression( expression.value( i ).toList(), context );
      const QString value = parseValue( expression.value( i + 1 ), context );
      caseString += u" WHEN (%1) THEN %2"_s.arg( condition, value );
    }
    const QString value = parseValue( expression.constLast(), context );
    caseString += u" ELSE %1 END"_s.arg( value );
    return caseString;
  }
  else if ( op == "zoom"_L1 && expression.count() == 1 )
  {
    return u"@vector_tile_zoom"_s;
  }
  else if ( op == "concat"_L1 )
  {
    QString concatString = u"concat("_s;
    for ( int i = 1; i < expression.size(); i++ )
    {
      if ( i > 1 )
        concatString += ", "_L1;
      concatString += parseValue( expression.value( i ), context );
    }
    concatString += ')'_L1;
    return concatString;
  }
  else if ( op == "length"_L1 )
  {
    return u"length(%1)"_s.arg( parseExpression( expression.value( 1 ).toList(), context ) );
  }
  else if ( op == "step"_L1 )
  {
    const QString stepExpression = parseExpression( expression.value( 1 ).toList(), context );
    if ( stepExpression.isEmpty() )
    {
      context.pushWarning( QObject::tr( "%1: Could not interpret step list" ).arg( context.layerId() ) );
      return QString();
    }

    QString caseString = u"CASE "_s;

    for ( int i = expression.length() - 2; i > 0; i -= 2 )
    {
      const QString stepValue = parseValue( expression.value( i + 1 ), context, colorExpected );
      if ( i > 1 )
      {
        const QString stepKey = QgsExpression::quotedValue( expression.value( i ) );
        caseString += u" WHEN %1 >= %2 THEN (%3) "_s.arg( stepExpression, stepKey, stepValue );
      }
      else
      {
        caseString += u"ELSE (%1) END"_s.arg( stepValue );
      }
    }
    return caseString;
  }
  else
  {
    context.pushWarning( QObject::tr( "%1: Skipping unsupported expression \"%2\"" ).arg( context.layerId(), op ) );
    return QString();
  }
}

QImage QgsMapBoxGlStyleConverter::retrieveSprite( const QString &name, QgsMapBoxGlStyleConversionContext &context, QSize &spriteSize )
{
  QImage spriteImage;
  QString category;
  QString actualName = name;
  const int categorySeparator = name.indexOf( ':' );
  if ( categorySeparator > 0 )
  {
    category = name.left( categorySeparator );
    if ( context.spriteCategories().contains( category ) )
    {
      actualName = name.mid( categorySeparator + 1 );
      spriteImage = context.spriteImage( category );
    }
    else
    {
      category.clear();
    }
  }

  if ( category.isEmpty() )
  {
    spriteImage = context.spriteImage();
  }

  if ( spriteImage.isNull() )
  {
    context.pushWarning( QObject::tr( "%1: Could not retrieve sprite '%2'" ).arg( context.layerId(), name ) );
    return QImage();
  }

  const QVariantMap spriteDefinition = context.spriteDefinitions( category ).value( actualName ).toMap();
  if ( spriteDefinition.size() == 0 )
  {
    context.pushWarning( QObject::tr( "%1: Could not retrieve sprite '%2'" ).arg( context.layerId(), name ) );
    return QImage();
  }

  const QImage sprite = spriteImage.copy( spriteDefinition.value( u"x"_s ).toInt(),
                                          spriteDefinition.value( u"y"_s ).toInt(),
                                          spriteDefinition.value( u"width"_s ).toInt(),
                                          spriteDefinition.value( u"height"_s ).toInt() );
  if ( sprite.isNull() )
  {
    context.pushWarning( QObject::tr( "%1: Could not retrieve sprite '%2'" ).arg( context.layerId(), name ) );
    return QImage();
  }

  spriteSize = sprite.size() / spriteDefinition.value( u"pixelRatio"_s ).toDouble() * context.pixelSizeConversionFactor();
  return sprite;
}

QString QgsMapBoxGlStyleConverter::retrieveSpriteAsBase64WithProperties( const QVariant &value, QgsMapBoxGlStyleConversionContext &context, QSize &spriteSize, QString &spriteProperty, QString &spriteSizeProperty )
{
  QString spritePath;

  auto prepareBase64 = []( const QImage & sprite )
  {
    QString path;
    if ( !sprite.isNull() )
    {
      QByteArray blob;
      QBuffer buffer( &blob );
      buffer.open( QIODevice::WriteOnly );
      sprite.save( &buffer, "PNG" );
      buffer.close();
      const QByteArray encoded = blob.toBase64();
      path = QString( encoded );
      path.prepend( "base64:"_L1 );
    }
    return path;
  };

  switch ( value.userType() )
  {
    case QMetaType::Type::QString:
    {
      QString spriteName = value.toString();
      const thread_local QRegularExpression fieldNameMatch( u"{([^}]+)}"_s );
      QRegularExpressionMatch match = fieldNameMatch.match( spriteName );
      if ( match.hasMatch() )
      {
        const QString fieldName = match.captured( 1 );
        spriteProperty = u"CASE"_s;
        spriteSizeProperty = u"CASE"_s;

        spriteName.replace( "(", "\\("_L1 );
        spriteName.replace( ")", "\\)"_L1 );
        spriteName.replace( fieldNameMatch, u"([^\\/\\\\]+)"_s );
        const QRegularExpression fieldValueMatch( spriteName );
        const QStringList spriteNames = context.spriteDefinitions().keys();
        for ( const QString &name : spriteNames )
        {
          match = fieldValueMatch.match( name );
          if ( match.hasMatch() )
          {
            QSize size;
            QString path;
            const QString fieldValue = match.captured( 1 );
            const QImage sprite = retrieveSprite( name, context, size );
            path = prepareBase64( sprite );
            if ( spritePath.isEmpty() && !path.isEmpty() )
            {
              spritePath = path;
              spriteSize = size;
            }

            spriteProperty += u" WHEN \"%1\" = '%2' THEN '%3'"_s
                              .arg( fieldName, fieldValue, path );
            spriteSizeProperty += u" WHEN \"%1\" = '%2' THEN %3"_s
                                  .arg( fieldName ).arg( fieldValue ).arg( size.width() );
          }
        }

        spriteProperty += " END"_L1;
        spriteSizeProperty += " END"_L1;
      }
      else
      {
        spriteProperty.clear();
        spriteSizeProperty.clear();
        const QImage sprite = retrieveSprite( spriteName, context, spriteSize );
        spritePath = prepareBase64( sprite );
      }
      break;
    }

    case QMetaType::Type::QVariantMap:
    {
      const QVariantList stops = value.toMap().value( u"stops"_s ).toList();
      if ( stops.size() == 0 )
        break;

      QString path;
      QSize size;
      QImage sprite;

      sprite = retrieveSprite( stops.value( 0 ).toList().value( 1 ).toString(), context, spriteSize );
      spritePath = prepareBase64( sprite );

      spriteProperty = u"CASE WHEN @vector_tile_zoom < %1 THEN '%2'"_s
                       .arg( stops.value( 0 ).toList().value( 0 ).toString() )
                       .arg( spritePath );
      spriteSizeProperty = u"CASE WHEN @vector_tile_zoom < %1 THEN %2"_s
                           .arg( stops.value( 0 ).toList().value( 0 ).toString() )
                           .arg( spriteSize.width() );

      for ( int i = 0; i < stops.size() - 1; ++i )
      {
        ;
        sprite = retrieveSprite( stops.value( 0 ).toList().value( 1 ).toString(), context, size );
        path = prepareBase64( sprite );

        spriteProperty += QStringLiteral( " WHEN @vector_tile_zoom >= %1 AND @vector_tile_zoom < %2 "
                                          "THEN '%3'" )
                          .arg( stops.value( i ).toList().value( 0 ).toString(),
                                stops.value( i + 1 ).toList().value( 0 ).toString(),
                                path );
        spriteSizeProperty += QStringLiteral( " WHEN @vector_tile_zoom >= %1 AND @vector_tile_zoom < %2 "
                                              "THEN %3" )
                              .arg( stops.value( i ).toList().value( 0 ).toString(),
                                    stops.value( i + 1 ).toList().value( 0 ).toString() )
                              .arg( size.width() );
      }
      sprite = retrieveSprite( stops.last().toList().value( 1 ).toString(), context, size );
      path = prepareBase64( sprite );

      spriteProperty += QStringLiteral( " WHEN @vector_tile_zoom >= %1 "
                                        "THEN '%2' END" )
                        .arg( stops.last().toList().value( 0 ).toString() )
                        .arg( path );
      spriteSizeProperty += QStringLiteral( " WHEN @vector_tile_zoom >= %1 "
                                            "THEN %2 END" )
                            .arg( stops.last().toList().value( 0 ).toString() )
                            .arg( size.width() );
      break;
    }

    case QMetaType::Type::QVariantList:
    {
      const QVariantList json = value.toList();
      const QString method = json.value( 0 ).toString();

      if ( method == "match"_L1 )
      {
        const QString attribute = parseExpression( json.value( 1 ).toList(), context );
        if ( attribute.isEmpty() )
        {
          context.pushWarning( QObject::tr( "%1: Could not interpret match list" ).arg( context.layerId() ) );
          break;
        }

        spriteProperty = u"CASE"_s;
        spriteSizeProperty = u"CASE"_s;

        for ( int i = 2; i < json.length() - 1; i += 2 )
        {
          const QVariant matchKey = json.value( i );
          const QVariant matchValue = json.value( i + 1 );
          QString matchString;
          switch ( matchKey.userType() )
          {
            case QMetaType::Type::QVariantList:
            case QMetaType::Type::QStringList:
            {
              const QVariantList keys = matchKey.toList();
              QStringList matchStringList;
              for ( const QVariant &key : keys )
              {
                matchStringList << QgsExpression::quotedValue( key );
              }
              matchString = matchStringList.join( ',' );
              break;
            }

            case QMetaType::Type::Bool:
            case QMetaType::Type::QString:
            case QMetaType::Type::Int:
            case QMetaType::Type::LongLong:
            case QMetaType::Type::Double:
            {
              matchString = QgsExpression::quotedValue( matchKey );
              break;
            }

            default:
              context.pushWarning( QObject::tr( "%1: Skipping unsupported sprite type (%2)." ).arg( context.layerId(), QMetaType::typeName( static_cast<QMetaType::Type>( value.userType() ) ) ) );
              break;

          }

          const QImage sprite = retrieveSprite( matchValue.toString(), context, spriteSize );
          spritePath = prepareBase64( sprite );

          spriteProperty += QStringLiteral( " WHEN %1 IN (%2) "
                                            "THEN '%3'" ).arg( attribute,
                                                matchString,
                                                spritePath );

          spriteSizeProperty += QStringLiteral( " WHEN %1 IN (%2) "
                                                "THEN %3" ).arg( attribute,
                                                    matchString ).arg( spriteSize.width() );
        }

        if ( !json.constLast().toString().isEmpty() )
        {
          const QImage sprite = retrieveSprite( json.constLast().toString(), context, spriteSize );
          spritePath = prepareBase64( sprite );
        }
        else
        {
          spritePath = QString();
        }

        spriteProperty += u" ELSE '%1' END"_s.arg( spritePath );
        spriteSizeProperty += u" ELSE %3 END"_s.arg( spriteSize.width() );
        break;
      }
      else if ( method == "step"_L1 )
      {
        const QString expression = parseExpression( json.value( 1 ).toList(), context );
        if ( expression.isEmpty() )
        {
          context.pushWarning( QObject::tr( "%1: Could not interpret step list" ).arg( context.layerId() ) );
          break;
        }

        spriteProperty = u"CASE"_s;
        spriteSizeProperty = u"CASE"_s;
        for ( int i = json.length() - 2; i > 2; i -= 2 )
        {
          const QString stepKey = QgsExpression::quotedValue( json.value( i ) );
          const QString stepValue = json.value( i + 1 ).toString();

          const QImage sprite = retrieveSprite( stepValue, context, spriteSize );
          spritePath = prepareBase64( sprite );

          spriteProperty += u" WHEN %1 >= %2 THEN '%3' "_s.arg( expression, stepKey, spritePath );
          spriteSizeProperty += u" WHEN %1 >= %2 THEN %3 "_s.arg( expression ).arg( stepKey ).arg( spriteSize.width() );
        }

        const QImage sprite = retrieveSprite( json.at( 2 ).toString(), context, spriteSize );
        spritePath = prepareBase64( sprite );

        spriteProperty += u"ELSE '%1' END"_s.arg( spritePath );
        spriteSizeProperty += u"ELSE %3 END"_s.arg( spriteSize.width() );
        break;
      }
      else if ( method == "case"_L1 )
      {
        spriteProperty = u"CASE"_s;
        spriteSizeProperty = u"CASE"_s;
        for ( int i = 1; i < json.length() - 2; i += 2 )
        {
          const QString caseExpression = parseExpression( json.value( i ).toList(), context );
          const QString caseValue = json.value( i + 1 ).toString();

          const QImage sprite = retrieveSprite( caseValue, context, spriteSize );
          spritePath = prepareBase64( sprite );

          spriteProperty += u" WHEN %1 THEN '%2' "_s.arg( caseExpression, spritePath );
          spriteSizeProperty += u" WHEN %1 THEN %2 "_s.arg( caseExpression ).arg( spriteSize.width() );
        }
        const QImage sprite = retrieveSprite( json.last().toString(), context, spriteSize );
        spritePath = prepareBase64( sprite );

        spriteProperty += u"ELSE '%1' END"_s.arg( spritePath );
        spriteSizeProperty += u"ELSE %3 END"_s.arg( spriteSize.width() );
        break;
      }
      else
      {
        context.pushWarning( QObject::tr( "%1: Could not interpret sprite value list with method %2" ).arg( context.layerId(), method ) );
        break;
      }
    }

    default:
      context.pushWarning( QObject::tr( "%1: Skipping unsupported sprite type (%2)." ).arg( context.layerId(), QMetaType::typeName( static_cast<QMetaType::Type>( value.userType() ) ) ) );
      break;
  }

  return spritePath;
}

QString QgsMapBoxGlStyleConverter::parseValue( const QVariant &value, QgsMapBoxGlStyleConversionContext &context, bool colorExpected )
{
  QColor c;
  switch ( value.userType() )
  {
    case QMetaType::Type::QVariantList:
    case QMetaType::Type::QStringList:
      return parseExpression( value.toList(), context, colorExpected );

    case QMetaType::Type::Bool:
    case QMetaType::Type::QString:
      if ( colorExpected )
      {
        QColor c = parseColor( value, context );
        if ( c.isValid() )
        {
          return parseValue( c, context );
        }
      }
      return QgsExpression::quotedValue( value );

    case QMetaType::Type::Int:
    case QMetaType::Type::LongLong:
    case QMetaType::Type::Double:
      return value.toString();

    case QMetaType::Type::QColor:
      c = value.value<QColor>();
      return QString( "color_rgba(%1,%2,%3,%4)" ).arg( c.red() ).arg( c.green() ).arg( c.blue() ).arg( c.alpha() );

    default:
      context.pushWarning( QObject::tr( "%1: Skipping unsupported expression part" ).arg( context.layerId() ) );
      break;
  }
  return QString();
}

QString QgsMapBoxGlStyleConverter::parseKey( const QVariant &value, QgsMapBoxGlStyleConversionContext &context )
{
  if ( value.toString() == "$type"_L1 )
  {
    return u"_geom_type"_s;
  }
  if ( value.toString() == "level"_L1 )
  {
    return u"level"_s;
  }
  else if ( ( value.userType() == QMetaType::Type::QVariantList && value.toList().size() == 1 ) || value.userType() == QMetaType::Type::QStringList )
  {
    if ( value.toList().size() > 1 )
      return value.toList().at( 1 ).toString();
    else
    {
      QString valueString = value.toList().value( 0 ).toString();
      if ( valueString == "geometry-type"_L1 )
      {
        return u"_geom_type"_s;
      }
      return valueString;
    }
  }
  else if ( value.userType() == QMetaType::Type::QVariantList && value.toList().size() > 1 )
  {
    return parseExpression( value.toList(), context );
  }
  return QgsExpression::quotedColumnRef( value.toString() );
}

QString QgsMapBoxGlStyleConverter::processLabelField( const QString &string, bool &isExpression )
{
  // {field_name} is permitted in string -- if multiple fields are present, convert them to an expression
  // but if single field is covered in {}, return it directly
  const thread_local QRegularExpression singleFieldRx( u"^{([^}]+)}$"_s );
  const QRegularExpressionMatch match = singleFieldRx.match( string );
  if ( match.hasMatch() )
  {
    isExpression = false;
    return match.captured( 1 );
  }

  const thread_local QRegularExpression multiFieldRx( u"(?={[^}]+})"_s );
  const QStringList parts = string.split( multiFieldRx );
  if ( parts.size() > 1 )
  {
    isExpression = true;

    QStringList res;
    for ( const QString &part : parts )
    {
      if ( part.isEmpty() )
        continue;

      if ( !part.contains( '{' ) )
      {
        res << QgsExpression::quotedValue( part );
        continue;
      }

      // part will start at a {field} reference
      const QStringList split = part.split( '}' );
      res << QgsExpression::quotedColumnRef( split.at( 0 ).mid( 1 ) );
      if ( !split.at( 1 ).isEmpty() )
        res << QgsExpression::quotedValue( split.at( 1 ) );
    }
    return u"concat(%1)"_s.arg( res.join( ',' ) );
  }
  else
  {
    isExpression = false;
    return string;
  }
}

QgsVectorTileRenderer *QgsMapBoxGlStyleConverter::renderer() const
{
  return mRenderer ? mRenderer->clone() : nullptr;
}

QgsVectorTileLabeling *QgsMapBoxGlStyleConverter::labeling() const
{
  return mLabeling ? mLabeling->clone() : nullptr;
}

QList<QgsMapBoxGlStyleAbstractSource *> QgsMapBoxGlStyleConverter::sources()
{
  return mSources;
}

QList<QgsMapBoxGlStyleRasterSubLayer> QgsMapBoxGlStyleConverter::rasterSubLayers() const
{
  return mRasterSubLayers;
}

QList<QgsMapLayer *> QgsMapBoxGlStyleConverter::createSubLayers() const
{
  QList<QgsMapLayer *> subLayers;
  for ( const QgsMapBoxGlStyleRasterSubLayer &subLayer : mRasterSubLayers )
  {
    const QString sourceName = subLayer.source();
    std::unique_ptr< QgsRasterLayer > rl;
    for ( const QgsMapBoxGlStyleAbstractSource *source : mSources )
    {
      if ( source->type() == Qgis::MapBoxGlStyleSourceType::Raster && source->name() == sourceName )
      {
        const QgsMapBoxGlStyleRasterSource *rasterSource = qgis::down_cast< const QgsMapBoxGlStyleRasterSource * >( source );
        rl.reset( rasterSource->toRasterLayer() );
        rl->pipe()->setDataDefinedProperties( subLayer.dataDefinedProperties() );
        break;
      }
    }

    if ( rl )
    {
      subLayers.append( rl.release() );
    }
  }
  return subLayers;
}


void QgsMapBoxGlStyleConverter::parseSources( const QVariantMap &sources, QgsMapBoxGlStyleConversionContext *context )
{
  std::unique_ptr< QgsMapBoxGlStyleConversionContext > tmpContext;
  if ( !context )
  {
    tmpContext = std::make_unique< QgsMapBoxGlStyleConversionContext >();
    context = tmpContext.get();
  }

  auto typeFromString = [context]( const QString & string, const QString & name )->Qgis::MapBoxGlStyleSourceType
  {
    if ( string.compare( "vector"_L1, Qt::CaseInsensitive ) == 0 )
      return Qgis::MapBoxGlStyleSourceType::Vector;
    else if ( string.compare( "raster"_L1, Qt::CaseInsensitive ) == 0 )
      return Qgis::MapBoxGlStyleSourceType::Raster;
    else if ( string.compare( "raster-dem"_L1, Qt::CaseInsensitive ) == 0 )
      return Qgis::MapBoxGlStyleSourceType::RasterDem;
    else if ( string.compare( "geojson"_L1, Qt::CaseInsensitive ) == 0 )
      return Qgis::MapBoxGlStyleSourceType::GeoJson;
    else if ( string.compare( "image"_L1, Qt::CaseInsensitive ) == 0 )
      return Qgis::MapBoxGlStyleSourceType::Image;
    else if ( string.compare( "video"_L1, Qt::CaseInsensitive ) == 0 )
      return Qgis::MapBoxGlStyleSourceType::Video;
    context->pushWarning( QObject::tr( "Invalid source type \"%1\" for source \"%2\"" ).arg( string, name ) );
    return Qgis::MapBoxGlStyleSourceType::Unknown;
  };

  for ( auto it = sources.begin(); it != sources.end(); ++it )
  {
    const QString name = it.key();
    const QVariantMap jsonSource = it.value().toMap();
    const QString typeString = jsonSource.value( u"type"_s ).toString();

    const Qgis::MapBoxGlStyleSourceType type = typeFromString( typeString, name );

    switch ( type )
    {
      case Qgis::MapBoxGlStyleSourceType::Raster:
        parseRasterSource( jsonSource, name, context );
        break;
      case Qgis::MapBoxGlStyleSourceType::Vector:
      case Qgis::MapBoxGlStyleSourceType::RasterDem:
      case Qgis::MapBoxGlStyleSourceType::GeoJson:
      case Qgis::MapBoxGlStyleSourceType::Image:
      case Qgis::MapBoxGlStyleSourceType::Video:
      case Qgis::MapBoxGlStyleSourceType::Unknown:
        QgsDebugError( u"Ignoring vector tile style source %1 (%2)"_s.arg( name, qgsEnumValueToKey( type ) ) );
        continue;
    }
  }
}

void QgsMapBoxGlStyleConverter::parseRasterSource( const QVariantMap &source, const QString &name, QgsMapBoxGlStyleConversionContext *context )
{
  std::unique_ptr< QgsMapBoxGlStyleConversionContext > tmpContext;
  if ( !context )
  {
    tmpContext = std::make_unique< QgsMapBoxGlStyleConversionContext >();
    context = tmpContext.get();
  }

  auto raster = std::make_unique< QgsMapBoxGlStyleRasterSource >( name );
  if ( raster->setFromJson( source, context ) )
    mSources.append( raster.release() );
}

bool QgsMapBoxGlStyleConverter::numericArgumentsOnly( const QVariant &bottomVariant, const QVariant &topVariant, double &bottom, double &top )
{
  if ( bottomVariant.canConvert( QMetaType::Double ) && topVariant.canConvert( QMetaType::Double ) )
  {
    bool bDoubleOk, tDoubleOk;
    bottom = bottomVariant.toDouble( &bDoubleOk );
    top = topVariant.toDouble( &tDoubleOk );
    return ( bDoubleOk && tDoubleOk );
  }
  return false;
}

//
// QgsMapBoxGlStyleConversionContext
//
void QgsMapBoxGlStyleConversionContext::pushWarning( const QString &warning )
{
  QgsDebugError( warning );
  mWarnings << warning;
}

Qgis::RenderUnit QgsMapBoxGlStyleConversionContext::targetUnit() const
{
  return mTargetUnit;
}

void QgsMapBoxGlStyleConversionContext::setTargetUnit( Qgis::RenderUnit targetUnit )
{
  mTargetUnit = targetUnit;
}

double QgsMapBoxGlStyleConversionContext::pixelSizeConversionFactor() const
{
  return mSizeConversionFactor;
}

void QgsMapBoxGlStyleConversionContext::setPixelSizeConversionFactor( double sizeConversionFactor )
{
  mSizeConversionFactor = sizeConversionFactor;
}

QStringList QgsMapBoxGlStyleConversionContext::spriteCategories() const
{
  return mSpriteImage.keys();
}

QImage QgsMapBoxGlStyleConversionContext::spriteImage( const QString &category ) const
{
  return mSpriteImage.contains( category ) ? mSpriteImage[category] : QImage();
}

QVariantMap QgsMapBoxGlStyleConversionContext::spriteDefinitions( const QString &category ) const
{
  return mSpriteDefinitions.contains( category ) ? mSpriteDefinitions[category] : QVariantMap();
}

void QgsMapBoxGlStyleConversionContext::setSprites( const QImage &image, const QVariantMap &definitions, const QString &category )
{
  mSpriteImage[category] = image;
  mSpriteDefinitions[category] = definitions;
}

void QgsMapBoxGlStyleConversionContext::setSprites( const QImage &image, const QString &definitions, const QString &category )
{
  setSprites( image, QgsJsonUtils::parseJson( definitions ).toMap(), category );
}

QString QgsMapBoxGlStyleConversionContext::layerId() const
{
  return mLayerId;
}

void QgsMapBoxGlStyleConversionContext::setLayerId( const QString &value )
{
  mLayerId = value;
}

//
// QgsMapBoxGlStyleAbstractSource
//
QgsMapBoxGlStyleAbstractSource::QgsMapBoxGlStyleAbstractSource( const QString &name )
  : mName( name )
{
}

QString QgsMapBoxGlStyleAbstractSource::name() const
{
  return mName;
}

QgsMapBoxGlStyleAbstractSource::~QgsMapBoxGlStyleAbstractSource() = default;

//
// QgsMapBoxGlStyleRasterSource
//

QgsMapBoxGlStyleRasterSource::QgsMapBoxGlStyleRasterSource( const QString &name )
  : QgsMapBoxGlStyleAbstractSource( name )
{

}

Qgis::MapBoxGlStyleSourceType QgsMapBoxGlStyleRasterSource::type() const
{
  return Qgis::MapBoxGlStyleSourceType::Raster;
}

bool QgsMapBoxGlStyleRasterSource::setFromJson( const QVariantMap &json, QgsMapBoxGlStyleConversionContext *context )
{
  mAttribution = json.value( u"attribution"_s ).toString();

  const QString scheme = json.value( u"scheme"_s, u"xyz"_s ).toString();
  if ( scheme.compare( "xyz"_L1 ) == 0 )
  {
    // xyz scheme is supported
  }
  else
  {
    context->pushWarning( QObject::tr( "%1 scheme is not supported for raster source %2" ).arg( scheme, name() ) );
    return false;
  }

  mMinZoom = json.value( u"minzoom"_s, u"0"_s ).toInt();
  mMaxZoom = json.value( u"maxzoom"_s, u"22"_s ).toInt();
  mTileSize = json.value( u"tileSize"_s, u"512"_s ).toInt();

  const QVariantList tiles = json.value( u"tiles"_s ).toList();
  for ( const QVariant &tile : tiles )
  {
    mTiles.append( tile.toString() );
  }

  return true;
}

QgsRasterLayer *QgsMapBoxGlStyleRasterSource::toRasterLayer() const
{
  QVariantMap parts;
  parts.insert( u"type"_s, u"xyz"_s );
  parts.insert( u"url"_s, mTiles.value( 0 ) );

  if ( mTileSize == 256 )
    parts.insert( u"tilePixelRation"_s, u"1"_s );
  else if ( mTileSize == 512 )
    parts.insert( u"tilePixelRation"_s, u"2"_s );

  parts.insert( u"zmax"_s, QString::number( mMaxZoom ) );
  parts.insert( u"zmin"_s, QString::number( mMinZoom ) );

  auto rl = std::make_unique< QgsRasterLayer >( QgsProviderRegistry::instance()->encodeUri( u"wms"_s, parts ), name(), u"wms"_s );
  return rl.release();
}

//
// QgsMapBoxGlStyleRasterSubLayer
//
QgsMapBoxGlStyleRasterSubLayer::QgsMapBoxGlStyleRasterSubLayer( const QString &id, const QString &source )
  : mId( id )
  , mSource( source )
{

}
