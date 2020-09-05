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
#include "qgsvectortilebasicrenderer.h"
#include "qgsvectortilebasiclabeling.h"
#include "qgssymbollayer.h"
#include "qgssymbollayerutils.h"
#include "qgslogger.h"
#include "qgsfillsymbollayer.h"
#include "qgslinesymbollayer.h"
#include "qgsfontutils.h"
#include "qgsjsonutils.h"



constexpr double PIXEL_RATIO = 1;

QgsMapBoxGlStyleConverter::QgsMapBoxGlStyleConverter()
{
}

QgsMapBoxGlStyleConverter::Result QgsMapBoxGlStyleConverter::convert( const QVariantMap &style )
{
  mError.clear();
  mWarnings.clear();
  if ( style.contains( QStringLiteral( "layers" ) ) )
  {
    parseLayers( style.value( QStringLiteral( "layers" ) ).toList() );
  }
  else
  {
    mError = QObject::tr( "Could not find layers list in JSON" );
    return NoLayerList;
  }
  return Success;
}

QgsMapBoxGlStyleConverter::Result QgsMapBoxGlStyleConverter::convert( const QString &style )
{
  return convert( QgsJsonUtils::parseJson( style ).toMap() );
}

QgsMapBoxGlStyleConverter::~QgsMapBoxGlStyleConverter() = default;

void QgsMapBoxGlStyleConverter::parseLayers( const QVariantList &layers )
{
  QgsMapBoxGlStyleConversionContext context;

  QList<QgsVectorTileBasicRendererStyle> rendererStyles;
  QList<QgsVectorTileBasicLabelingStyle> labelingStyles;

  for ( const QVariant &layer : layers )
  {
    const QVariantMap jsonLayer = layer.toMap();

    const QString layerType = jsonLayer.value( QStringLiteral( "type" ) ).toString();
    if ( layerType == QLatin1String( "background" ) )
      continue;

    const QString styleId = jsonLayer.value( QStringLiteral( "id" ) ).toString();
    const QString layerName = jsonLayer.value( QStringLiteral( "source-layer" ) ).toString();

    const int minZoom = jsonLayer.value( QStringLiteral( "minzoom" ), QStringLiteral( "-1" ) ).toInt();
    const int maxZoom = jsonLayer.value( QStringLiteral( "maxzoom" ), QStringLiteral( "-1" ) ).toInt();

    const bool enabled = jsonLayer.value( QStringLiteral( "visibility" ) ).toString() != QLatin1String( "none" );

    QString filterExpression;
    if ( jsonLayer.contains( QStringLiteral( "filter" ) ) )
    {
      filterExpression = parseExpression( jsonLayer.value( QStringLiteral( "filter" ) ).toList(), context );
    }

    QgsVectorTileBasicRendererStyle rendererStyle;
    QgsVectorTileBasicLabelingStyle labelingStyle;

    bool hasRendererStyle = false;
    bool hasLabelingStyle = false;
    if ( layerType == QLatin1String( "fill" ) )
    {
      hasRendererStyle = parseFillLayer( jsonLayer, rendererStyle, context );
    }
    else if ( layerType == QLatin1String( "line" ) )
    {
      hasRendererStyle = parseLineLayer( jsonLayer, rendererStyle, context );
    }
    else if ( layerType == QLatin1String( "symbol" ) )
    {
      parseSymbolLayer( jsonLayer, rendererStyle, hasRendererStyle, labelingStyle, hasLabelingStyle, context );
    }
    else
    {
      mWarnings << QObject::tr( "Skipping unknown layer type: %1" ).arg( layerType );
      QgsDebugMsg( mWarnings.constLast() );
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

    mWarnings.append( context.warnings() );
    context.clearWarnings();
  }

  mRenderer = qgis::make_unique< QgsVectorTileBasicRenderer >();
  QgsVectorTileBasicRenderer *renderer = dynamic_cast< QgsVectorTileBasicRenderer *>( mRenderer.get() );
  renderer->setStyles( rendererStyles );

  mLabeling = qgis::make_unique< QgsVectorTileBasicLabeling >();
  QgsVectorTileBasicLabeling *labeling = dynamic_cast< QgsVectorTileBasicLabeling * >( mLabeling.get() );
  labeling->setStyles( labelingStyles );
}

bool QgsMapBoxGlStyleConverter::parseFillLayer( const QVariantMap &jsonLayer, QgsVectorTileBasicRendererStyle &style, QgsMapBoxGlStyleConversionContext &context )
{
  if ( !jsonLayer.contains( QStringLiteral( "paint" ) ) )
  {
    context.pushWarning( QObject::tr( "Style layer %1 has no paint property, skipping" ).arg( jsonLayer.value( QStringLiteral( "id" ) ).toString() ) );
    return false;
  }

  const QVariantMap jsonPaint = jsonLayer.value( QStringLiteral( "paint" ) ).toMap();

  QgsPropertyCollection ddProperties;

  // fill color
  QColor fillColor;
  if ( jsonPaint.contains( QStringLiteral( "fill-color" ) ) )
  {
    const QVariant jsonFillColor = jsonPaint.value( QStringLiteral( "fill-color" ) );
    switch ( jsonFillColor.type() )
    {
      case QVariant::Map:
        ddProperties.setProperty( QgsSymbolLayer::PropertyFillColor, parseInterpolateColorByZoom( jsonFillColor.toMap(), context, &fillColor ) );
        break;

      case QVariant::List:
      case QVariant::StringList:
        ddProperties.setProperty( QgsSymbolLayer::PropertyFillColor, parseInterpolateListByZoom( jsonFillColor.toList(), PropertyType::Color, context, 1, &fillColor ) );
        break;

      case QVariant::String:
        fillColor = parseColor( jsonFillColor.toString(), context );
        break;

      default:
      {
        context.pushWarning( QObject::tr( "Skipping non-implemented color expression" ) );
        break;
      }
    }
  }

  QColor fillOutlineColor;
  if ( !jsonPaint.contains( QStringLiteral( "fill-outline-color" ) ) )
  {
    // fill-outline-color
    if ( fillColor.isValid() )
      fillOutlineColor = fillColor;
    else
    {
      // use fill color data defined property
      if ( ddProperties.isActive( QgsSymbolLayer::PropertyFillColor ) )
        ddProperties.setProperty( QgsSymbolLayer::PropertyStrokeColor,  ddProperties.property( QgsSymbolLayer::PropertyFillColor ) );
    }
  }
  else
  {
    const QVariant jsonFillOutlineColor = jsonPaint.value( QStringLiteral( "fill-outline-color" ) );
    switch ( jsonFillOutlineColor.type() )
    {
      case QVariant::Map:
        ddProperties.setProperty( QgsSymbolLayer::PropertyStrokeColor, parseInterpolateColorByZoom( jsonFillOutlineColor.toMap(), context, &fillOutlineColor ) );
        break;

      case QVariant::List:
      case QVariant::StringList:
        ddProperties.setProperty( QgsSymbolLayer::PropertyStrokeColor, parseInterpolateListByZoom( jsonFillOutlineColor.toList(), PropertyType::Color, context, 1, &fillOutlineColor ) );
        break;

      case QVariant::String:
        fillOutlineColor = parseColor( jsonFillOutlineColor.toString(), context );
        break;

      default:
        context.pushWarning( QObject::tr( "Skipping non-implemented color expression" ) );
        break;
    }
  }

  double fillOpacity = -1.0;
  if ( jsonPaint.contains( QStringLiteral( "fill-opacity" ) ) )
  {
    const QVariant jsonFillOpacity = jsonPaint.value( QStringLiteral( "fill-opacity" ) );
    switch ( jsonFillOpacity.type() )
    {
      case QVariant::Int:
      case QVariant::Double:
        fillOpacity = jsonFillOpacity.toDouble();
        break;

      case QVariant::Map:
        if ( ddProperties.isActive( QgsSymbolLayer::PropertyFillColor ) )
        {
          context.pushWarning( QObject::tr( "Could not set opacity of layer %1, opacity already defined in fill color" ).arg( jsonLayer.value( QStringLiteral( "id" ) ).toString() ) );
        }
        else
        {
          ddProperties.setProperty( QgsSymbolLayer::PropertyFillColor, parseInterpolateOpacityByZoom( jsonFillOpacity.toMap() ) );
          ddProperties.setProperty( QgsSymbolLayer::PropertyStrokeColor, parseInterpolateOpacityByZoom( jsonFillOpacity.toMap() ) );
        }
        break;

      case QVariant::List:
      case QVariant::StringList:
        if ( ddProperties.isActive( QgsSymbolLayer::PropertyFillColor ) )
        {
          context.pushWarning( QObject::tr( "Could not set opacity of layer %1, opacity already defined in fill color" ).arg( jsonLayer.value( QStringLiteral( "id" ) ).toString() ) );
        }
        else
        {
          ddProperties.setProperty( QgsSymbolLayer::PropertyFillColor, parseInterpolateListByZoom( jsonFillOpacity.toList(), PropertyType::Opacity, context ) );
          ddProperties.setProperty( QgsSymbolLayer::PropertyStrokeColor, parseInterpolateListByZoom( jsonFillOpacity.toList(), PropertyType::Opacity, context ) );
        }
        break;

      default:
        context.pushWarning( QObject::tr( "Skipping non-implemented opacity expression" ) );
        break;
    }
  }

  // TODO: fill-translate

  std::unique_ptr< QgsSymbol > symbol( QgsSymbol::defaultSymbol( QgsWkbTypes::PolygonGeometry ) );
  QgsSimpleFillSymbolLayer *fillSymbol = dynamic_cast< QgsSimpleFillSymbolLayer * >( symbol->symbolLayer( 0 ) );

  // set render units
  symbol->setOutputUnit( QgsUnitTypes::RenderPixels );
  fillSymbol->setOutputUnit( QgsUnitTypes::RenderPixels );

#if 0 // todo
  // get fill-pattern to set sprite
  // sprite imgs will already have been downloaded in converter.py
  fill_pattern = json_paint.get( "fill-pattern" )

                 // fill-pattern can be String or Object
                 // String: {"fill-pattern": "dash-t"}
                 // Object: {"fill-pattern":{"stops":[[11,"wetland8"],[12,"wetland16"]]}}

                 // if Object, simpify into one sprite.
                 // TODO:
                 if isinstance( fill_pattern, dict )
  {
    pattern_stops = fill_pattern.get( "stops", [None] )
                    fill_pattern = pattern_stops[-1][-1]
  }

  // when fill-pattern exists, set and insert RasterFillSymbolLayer
  if fill_pattern
{
  SPRITES_PATH = os.path.join( os.path.dirname( os.path.realpath( __file__ ) ), "sprites" )
    raster_fill_symbol = QgsRasterFillSymbolLayer( os.path.join( SPRITES_PATH, fill_pattern + ".png" ) )
    sym.appendSymbolLayer( raster_fill_symbol )
  }
#endif

  fillSymbol->setDataDefinedProperties( ddProperties );

  if ( fillOpacity != -1 )
  {
    symbol->setOpacity( fillOpacity );
  }

  if ( fillOutlineColor.isValid() )
  {
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
  else
  {
    fillSymbol->setBrushStyle( Qt::NoBrush );
  }

  style.setGeometryType( QgsWkbTypes::PolygonGeometry );
  style.setSymbol( symbol.release() );
  return true;
}

bool QgsMapBoxGlStyleConverter::parseLineLayer( const QVariantMap &jsonLayer, QgsVectorTileBasicRendererStyle &style, QgsMapBoxGlStyleConversionContext &context )
{
  if ( !jsonLayer.contains( QStringLiteral( "paint" ) ) )
  {
    context.pushWarning( QObject::tr( "Style layer %1 has no paint property, skipping" ).arg( jsonLayer.value( QStringLiteral( "id" ) ).toString() ) );
    return false;
  }

  const QVariantMap jsonPaint = jsonLayer.value( QStringLiteral( "paint" ) ).toMap();

  QgsPropertyCollection ddProperties;

  // line color
  QColor lineColor;
  if ( jsonPaint.contains( QStringLiteral( "line-color" ) ) )
  {
    const QVariant jsonLineColor = jsonPaint.value( QStringLiteral( "line-color" ) );
    switch ( jsonLineColor.type() )
    {
      case QVariant::Map:
        ddProperties.setProperty( QgsSymbolLayer::PropertyFillColor, parseInterpolateColorByZoom( jsonLineColor.toMap(), context, &lineColor ) );
        ddProperties.setProperty( QgsSymbolLayer::PropertyStrokeColor, ddProperties.property( QgsSymbolLayer::PropertyFillColor ) );
        break;

      case QVariant::List:
      case QVariant::StringList:
        ddProperties.setProperty( QgsSymbolLayer::PropertyFillColor, parseInterpolateListByZoom( jsonLineColor.toList(), PropertyType::Color, context, 1, &lineColor ) );
        ddProperties.setProperty( QgsSymbolLayer::PropertyStrokeColor, ddProperties.property( QgsSymbolLayer::PropertyFillColor ) );
        break;

      case QVariant::String:
        lineColor = parseColor( jsonLineColor.toString(), context );
        break;

      default:
        context.pushWarning( QObject::tr( "Skipping non-implemented color expression" ) );
        break;
    }
  }


  double lineWidth = 1.0;
  if ( jsonPaint.contains( QStringLiteral( "line-width" ) ) )
  {
    const QVariant jsonLineWidth = jsonPaint.value( QStringLiteral( "line-width" ) );
    switch ( jsonLineWidth.type() )
    {
      case QVariant::Int:
      case QVariant::Double:
        lineWidth = jsonLineWidth.toDouble();
        break;

      case QVariant::Map:
        lineWidth = -1;
        ddProperties.setProperty( QgsSymbolLayer::PropertyStrokeWidth, parseInterpolateByZoom( jsonLineWidth.toMap(), context, PIXEL_RATIO ) );
        break;

      case QVariant::List:
      case QVariant::StringList:
        ddProperties.setProperty( QgsSymbolLayer::PropertyStrokeWidth, parseInterpolateListByZoom( jsonLineWidth.toList(), PropertyType::Numeric, context, PIXEL_RATIO ) );
        break;

      default:
        context.pushWarning( QObject::tr( "Skipping non-implemented line-width expression" ) );
        break;
    }
  }

  double lineOpacity = -1.0;
  if ( jsonPaint.contains( QStringLiteral( "line-opacity" ) ) )
  {
    const QVariant jsonLineOpacity = jsonPaint.value( QStringLiteral( "line-opacity" ) );
    switch ( jsonLineOpacity.type() )
    {
      case QVariant::Int:
      case QVariant::Double:
        lineOpacity = jsonLineOpacity.toDouble();
        break;

      case QVariant::Map:
        if ( ddProperties.isActive( QgsSymbolLayer::PropertyStrokeColor ) )
        {
          context.pushWarning( QObject::tr( "Could not set opacity of layer %1, opacity already defined in stroke color" ).arg( jsonLayer.value( QStringLiteral( "id" ) ).toString() ) );
        }
        else
        {
          ddProperties.setProperty( QgsSymbolLayer::PropertyFillColor, parseInterpolateOpacityByZoom( jsonLineOpacity.toMap() ) );
          ddProperties.setProperty( QgsSymbolLayer::PropertyStrokeColor, parseInterpolateOpacityByZoom( jsonLineOpacity.toMap() ) );
        }
        break;

      case QVariant::List:
      case QVariant::StringList:
        if ( ddProperties.isActive( QgsSymbolLayer::PropertyStrokeColor ) )
        {
          context.pushWarning( QObject::tr( "Could not set opacity of layer %1, opacity already defined in stroke color" ).arg( jsonLayer.value( QStringLiteral( "id" ) ).toString() ) );
        }
        else
        {
          ddProperties.setProperty( QgsSymbolLayer::PropertyFillColor, parseInterpolateListByZoom( jsonLineOpacity.toList(), PropertyType::Opacity, context ) );
          ddProperties.setProperty( QgsSymbolLayer::PropertyStrokeColor, parseInterpolateListByZoom( jsonLineOpacity.toList(), PropertyType::Opacity, context ) );
        }
        break;

      default:
        context.pushWarning( QObject::tr( "Skipping non-implemented opacity expression" ) );
        break;
    }
  }

  QVector< double > dashVector;
  if ( jsonPaint.contains( QStringLiteral( "line-dasharray" ) ) )
  {
    const QVariant jsonLineDashArray = jsonPaint.value( QStringLiteral( "line-dasharray" ) );
    switch ( jsonLineDashArray.type() )
    {
      case QVariant::Map:
      {
        //TODO improve parsing (use PropertyCustomDash?)
        const QVariantList dashSource = jsonLineDashArray.toMap().value( QStringLiteral( "stops" ) ).toList().last().toList().value( 1 ).toList();
        for ( const QVariant &v : dashSource )
        {
          dashVector << v.toDouble() * PIXEL_RATIO;
        }
        break;
      }

      case QVariant::List:
      case QVariant::StringList:
      {
        const QVariantList dashSource = jsonLineDashArray.toList();
        for ( const QVariant &v : dashSource )
        {
          dashVector << v.toDouble() * PIXEL_RATIO;
        }
        break;
      }

      default:
        context.pushWarning( QObject::tr( "Skipping non-implemented dash vector expression" ) );
        break;
    }
  }

  Qt::PenCapStyle penCapStyle = Qt::FlatCap;
  Qt::PenJoinStyle penJoinStyle = Qt::MiterJoin;
  if ( jsonLayer.contains( QStringLiteral( "layout" ) ) )
  {
    const QVariantMap jsonLayout = jsonPaint.value( QStringLiteral( "layout" ) ).toMap();
    if ( jsonLayout.contains( QStringLiteral( "line-cap" ) ) )
    {
      penCapStyle = parseCapStyle( jsonLayout.value( QStringLiteral( "line-cap" ) ).toString() );
    }
    if ( jsonLayout.contains( QStringLiteral( "line-join" ) ) )
    {
      penJoinStyle = parseJoinStyle( jsonLayout.value( QStringLiteral( "line-join" ) ).toString() );
    }
  }

  std::unique_ptr< QgsSymbol > symbol( QgsSymbol::defaultSymbol( QgsWkbTypes::LineGeometry ) );
  QgsSimpleLineSymbolLayer *lineSymbol = dynamic_cast< QgsSimpleLineSymbolLayer * >( symbol->symbolLayer( 0 ) );

  // set render units
  symbol->setOutputUnit( QgsUnitTypes::RenderPixels );
  lineSymbol->setOutputUnit( QgsUnitTypes::RenderPixels );
  lineSymbol->setPenCapStyle( penCapStyle );
  lineSymbol->setPenJoinStyle( penJoinStyle );
  lineSymbol->setDataDefinedProperties( ddProperties );

  if ( lineOpacity != -1 )
  {
    symbol->setOpacity( lineOpacity );
  }
  if ( lineColor.isValid() )
  {
    lineSymbol->setColor( lineColor );
  }
  if ( lineWidth != -1 )
  {
    lineSymbol->setWidth( lineWidth * PIXEL_RATIO );
  }
  if ( !dashVector.empty() )
  {
    lineSymbol->setUseCustomDashPattern( true );
    lineSymbol->setCustomDashVector( dashVector );
  }

  style.setGeometryType( QgsWkbTypes::LineGeometry );
  style.setSymbol( symbol.release() );
  return true;
}

void QgsMapBoxGlStyleConverter::parseSymbolLayer( const QVariantMap &jsonLayer, QgsVectorTileBasicRendererStyle &, bool &hasRenderer, QgsVectorTileBasicLabelingStyle &labelingStyle, bool &hasLabeling, QgsMapBoxGlStyleConversionContext &context )
{
  hasLabeling = false;
  hasRenderer = false;
  if ( !jsonLayer.contains( QStringLiteral( "paint" ) ) )
  {
    context.pushWarning( QObject::tr( "Style layer %1 has no paint property, skipping" ).arg( jsonLayer.value( QStringLiteral( "id" ) ).toString() ) );
    return;
  }

  const QVariantMap jsonPaint = jsonLayer.value( QStringLiteral( "paint" ) ).toMap();

  if ( !jsonLayer.contains( QStringLiteral( "layout" ) ) )
  {
    context.pushWarning( QObject::tr( "Style layer %1 has no layout property, skipping" ).arg( jsonLayer.value( QStringLiteral( "id" ) ).toString() ) );
    return;
  }

  const QVariantMap jsonLayout = jsonLayer.value( QStringLiteral( "layout" ) ).toMap();

  QgsPropertyCollection ddLabelProperties;

  double textSize = 16.0;
  if ( jsonLayout.contains( QStringLiteral( "text-size" ) ) )
  {
    const QVariant jsonTextSize = jsonLayout.value( QStringLiteral( "text-size" ) );
    switch ( jsonTextSize.type() )
    {
      case QVariant::Int:
      case QVariant::Double:
        textSize = jsonTextSize.toDouble();
        break;

      case QVariant::Map:
        textSize = -1;
        ddLabelProperties.setProperty( QgsPalLayerSettings::Size, parseInterpolateByZoom( jsonTextSize.toMap(), context ) );
        break;

      case QVariant::List:
      case QVariant::StringList:
        textSize = -1;
        ddLabelProperties.setProperty( QgsPalLayerSettings::Size, parseInterpolateListByZoom( jsonTextSize.toList(), PropertyType::Numeric, context ) );
        break;

      default:
        context.pushWarning( QObject::tr( "Skipping non-implemented text-size expression" ) );
        break;
    }
  }

  QFont textFont;
  bool foundFont = false;
  if ( jsonLayout.contains( QStringLiteral( "text-font" ) ) )
  {
    const QVariant jsonTextFont = jsonLayout.value( QStringLiteral( "text-font" ) );
    if ( jsonTextFont.type() != QVariant::List && jsonTextFont.type() != QVariant::StringList && jsonTextFont.type() != QVariant::String )
    {
      context.pushWarning( QObject::tr( "Skipping non-implemented text-font expression" ) );
    }
    else
    {
      QString fontName;
      switch ( jsonTextFont.type() )
      {
        case QVariant::List:
        case QVariant::StringList:
          fontName = jsonTextFont.toList().value( 0 ).toString();
          break;

        case QVariant::String:
          fontName = jsonTextFont.toString();
          break;

        default:
          break;
      }

      const QStringList textFontParts = fontName.split( ' ' );
      for ( int i = 1; i < textFontParts.size(); ++i )
      {
        const QString candidateFontName = textFontParts.mid( 0, i ).join( ' ' );
        const QString candidateFontStyle = textFontParts.mid( i ).join( ' ' );
        if ( QgsFontUtils::fontFamilyHasStyle( candidateFontName, candidateFontStyle ) )
        {
          textFont = QFont( candidateFontName );
          textFont.setStyleName( candidateFontStyle );
          foundFont = true;
          break;
        }
      }

      if ( !foundFont )
      {
        // probably won't work, but we'll try anyway... maybe the json isn't following the spec correctly!!
        textFont = QFont( fontName );
        foundFont = true;
      }
    }
  }

  // text color
  QColor textColor;
  if ( jsonPaint.contains( QStringLiteral( "text-color" ) ) )
  {
    const QVariant jsonTextColor = jsonPaint.value( QStringLiteral( "text-color" ) );
    switch ( jsonTextColor.type() )
    {
      case QVariant::Map:
        ddLabelProperties.setProperty( QgsPalLayerSettings::Color, parseInterpolateColorByZoom( jsonTextColor.toMap(), context, &textColor ) );
        break;

      case QVariant::List:
      case QVariant::StringList:
        ddLabelProperties.setProperty( QgsPalLayerSettings::Color, parseInterpolateListByZoom( jsonTextColor.toList(), PropertyType::Color, context, 1, &textColor ) );
        break;

      case QVariant::String:
        textColor = parseColor( jsonTextColor.toString(), context );
        break;

      default:
        context.pushWarning( QObject::tr( "Skipping non-implemented text-color expression" ) );
        break;
    }
  }

  // buffer color
  QColor bufferColor;
  if ( jsonPaint.contains( QStringLiteral( "text-halo-color" ) ) )
  {
    const QVariant jsonBufferColor = jsonPaint.value( QStringLiteral( "text-halo-color" ) );
    switch ( jsonBufferColor.type() )
    {
      case QVariant::Map:
        ddLabelProperties.setProperty( QgsPalLayerSettings::BufferColor, parseInterpolateColorByZoom( jsonBufferColor.toMap(), context, &bufferColor ) );
        break;

      case QVariant::List:
      case QVariant::StringList:
        ddLabelProperties.setProperty( QgsPalLayerSettings::BufferColor, parseInterpolateListByZoom( jsonBufferColor.toList(), PropertyType::Color, context, 1, &bufferColor ) );
        break;

      case QVariant::String:
        bufferColor = parseColor( jsonBufferColor.toString(), context );
        break;

      default:
        context.pushWarning( QObject::tr( "Skipping non-implemented text-halo-color expression" ) );
        break;
    }
  }

  double bufferSize = 0.0;
  if ( jsonPaint.contains( QStringLiteral( "text-halo-width" ) ) )
  {
    const QVariant jsonHaloWidth = jsonPaint.value( QStringLiteral( "text-halo-width" ) );
    switch ( jsonHaloWidth.type() )
    {
      case QVariant::Int:
      case QVariant::Double:
        bufferSize = jsonHaloWidth.toDouble();
        break;

      case QVariant::Map:
        bufferSize = 1;
        ddLabelProperties.setProperty( QgsPalLayerSettings::BufferSize, parseInterpolateByZoom( jsonHaloWidth.toMap(), context ) );
        break;

      case QVariant::List:
      case QVariant::StringList:
        bufferSize = 1;
        ddLabelProperties.setProperty( QgsPalLayerSettings::BufferSize, parseInterpolateListByZoom( jsonHaloWidth.toList(), PropertyType::Numeric, context ) );
        break;

      default:
        context.pushWarning( QObject::tr( "Skipping non-implemented text-halo-width expression" ) );
        break;
    }
  }

  // TODO implement halo blur
#if 0
  if ( 'text-halo-blur' in json_paint )
  {
    json_text_halo_blur = json_paint['text-halo-blur'];
    if ( isinstance( json_text_halo_blur, ( float, int ) ) )
      buffer_size = buffer_size - json_text_halo_blur;
    else
      print( "skipping non-float text-halo-blur", json_text_halo_blur );
  }
#endif

  QgsTextFormat format;
  format.setSizeUnit( QgsUnitTypes::RenderPixels );
  if ( textColor.isValid() )
    format.setColor( textColor );
  if ( textSize >= 0 )
    format.setSize( textSize );
  if ( foundFont )
    format.setFont( textFont );

  if ( bufferSize > 0 )
  {
    format.buffer().setEnabled( true );
    format.buffer().setSize( bufferSize * PIXEL_RATIO );
    format.buffer().setSizeUnit( QgsUnitTypes::RenderPixels );
    format.buffer().setColor( bufferColor );
  }

  QgsPalLayerSettings labelSettings;

  // TODO - likely improvements required here
  labelSettings.fieldName = QStringLiteral( "name:latin" );
  if ( jsonLayout.contains( QStringLiteral( "text-field" ) ) )
  {
    const QVariant jsonTextField = jsonLayout.value( QStringLiteral( "text-field" ) );
    switch ( jsonTextField.type() )
    {
      case QVariant::String:
        labelSettings.fieldName = jsonTextField.toString();
        break;

      case QVariant::List:
      case QVariant::StringList:
        labelSettings.fieldName = jsonTextField.toList().value( 1 ).toList().value( 1 ).toString();
        break;

      default:
        context.pushWarning( QObject::tr( "Skipping non-implemented text-field expression" ) );
        break;
    }

    // handle ESRI specific field name constants
    if ( labelSettings.fieldName == QLatin1String( "{_name_global}" ) )
      labelSettings.fieldName = QStringLiteral( "_name_global" );
    else if ( labelSettings.fieldName == QLatin1String( "{_name}" ) )
      labelSettings.fieldName = QStringLiteral( "_name" );
  }

  if ( jsonLayout.contains( QStringLiteral( "text-transform" ) ) )
  {
    labelSettings.isExpression = true;
    const QString textTransform = jsonLayout.value( QStringLiteral( "text-transform" ) ).toString();
    if ( textTransform == QLatin1String( "uppercase" ) )
    {
      labelSettings.fieldName = QStringLiteral( "upper(%1)" ).arg( labelSettings.fieldName );
    }
    else if ( textTransform == QLatin1String( "lowercase" ) )
    {
      labelSettings.fieldName = QStringLiteral( "lower(%1)" ).arg( labelSettings.fieldName );
    }
  }

  labelSettings.placement = QgsPalLayerSettings::OverPoint;
  QgsWkbTypes::GeometryType geometryType = QgsWkbTypes::PointGeometry;
  if ( jsonLayout.contains( QStringLiteral( "symbol-placement" ) ) )
  {
    const QString symbolPlacement = jsonLayout.value( QStringLiteral( "symbol-placement" ) ).toString();
    if ( symbolPlacement == QLatin1String( "line" ) )
    {
      labelSettings.placement = QgsPalLayerSettings::Curved;
      labelSettings.lineSettings().setPlacementFlags( QgsLabeling::OnLine );
      geometryType = QgsWkbTypes::LineGeometry;
    }
  }

  if ( textSize >= 0 )
  {
    // TODO -- this probably needs revisiting -- it was copied from the MapTiler code, but may be wrong...
    labelSettings.priority = std::min( textSize / 3, 10.0 );
  }

  labelSettings.setFormat( format );

  // use a low obstacle weight for layers by default -- we'd rather have more labels for these layers, even if placement isn't ideal
  labelSettings.obstacleSettings().setFactor( 0.1 );

  labelSettings.setDataDefinedProperties( ddLabelProperties );

  labelingStyle.setGeometryType( geometryType );
  labelingStyle.setLabelSettings( labelSettings );

  hasLabeling = true;
}

QgsProperty QgsMapBoxGlStyleConverter::parseInterpolateColorByZoom( const QVariantMap &json, QgsMapBoxGlStyleConversionContext &context, QColor *defaultColor )
{
  const double base = json.value( QStringLiteral( "base" ), QStringLiteral( "1" ) ).toDouble();
  const QVariantList stops = json.value( QStringLiteral( "stops" ) ).toList();
  if ( stops.empty() )
    return QgsProperty();

  QString caseString = QStringLiteral( "CASE " );

  if ( base == 1 )
  {
    // base = 1 -> scale linear
    for ( int i = 0; i < stops.length() - 1; ++i )
    {
      // step bottom zoom
      const QString bz = stops.at( i ).toList().value( 0 ).toString();
      // step top zoom
      const QString tz = stops.at( i + 1 ).toList().value( 0 ).toString();

      const QColor bottomColor = parseColor( stops.at( i ).toList().value( 1 ), context );
      const QColor topColor = parseColor( stops.at( i + 1 ).toList().value( 1 ), context );

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

      caseString += QStringLiteral( "WHEN @zoom_level >= %1 AND @zoom_level < %2 THEN color_hsla("
                                    "scale_linear(@zoom_level, %1, %2, %3, %4), "
                                    "scale_linear(@zoom_level, %1, %2, %5, %6), "
                                    "scale_linear(@zoom_level, %1, %2, %7, %8), "
                                    "scale_linear(@zoom_level, %1, %2, %9, %10)) "
                                  ).arg( bz, tz )
                    .arg( bcHue )
                    .arg( tcHue )
                    .arg( bcSat )
                    .arg( tcSat )
                    .arg( bcLight )
                    .arg( tcLight )
                    .arg( bcAlpha )
                    .arg( tcAlpha );
    }
  }
  else
  {
    // Base != 1 -> scale_exp()
    for ( int i = 0; i < stops.length() - 1; ++i )
    {
      // step bottom zoom
      const QString bz = stops.at( i ).toList().value( 0 ).toString();
      // step top zoom
      const QString tz = stops.at( i + 1 ).toList().value( 0 ).toString();

      const QColor bottomColor = parseColor( stops.at( i ).toList().value( 1 ), context );
      const QColor topColor = parseColor( stops.at( i + 1 ).toList().value( 1 ), context );

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

      caseString += QStringLiteral( "WHEN @zoom_level >= %1 AND @zoom_level < %2 THEN color_hsla("
                                    "%3, %4, %5, %6) " ).arg( bz, tz,
                                        interpolateExpression( bz.toInt(), tz.toInt(), bcHue, tcHue, base ),
                                        interpolateExpression( bz.toInt(), tz.toInt(), bcSat, tcSat, base ),
                                        interpolateExpression( bz.toInt(), tz.toInt(), bcLight, tcLight, base ),
                                        interpolateExpression( bz.toInt(), tz.toInt(), bcAlpha, tcAlpha, base ) );
    }
  }

  // top color
  const QString tz = stops.last().toList().value( 0 ).toString();
  const QColor topColor = parseColor( stops.last().toList().value( 1 ), context );
  int tcHue;
  int tcSat;
  int tcLight;
  int tcAlpha;
  colorAsHslaComponents( topColor, tcHue, tcSat, tcLight, tcAlpha );

  caseString += QStringLiteral( "WHEN @zoom_level >= %1 THEN color_hsla(%2, %3, %4, %5) "
                                "ELSE color_hsla(%2, %3, %4, %5) END" ).arg( tz )
                .arg( tcHue ).arg( tcSat ).arg( tcLight ).arg( tcAlpha );


  if ( !stops.empty() && defaultColor )
    *defaultColor = parseColor( stops.value( 0 ).toList().value( 1 ).toString(), context );

  return QgsProperty::fromExpression( caseString );
}

QgsProperty QgsMapBoxGlStyleConverter::parseInterpolateByZoom( const QVariantMap &json, QgsMapBoxGlStyleConversionContext &context, double multiplier )
{
  const double base = json.value( QStringLiteral( "base" ), QStringLiteral( "1" ) ).toDouble();
  const QVariantList stops = json.value( QStringLiteral( "stops" ) ).toList();
  if ( stops.empty() )
    return QgsProperty();

  QString scaleExpression;
  if ( stops.size() <= 2 )
  {
    if ( base == 1 )
    {
      scaleExpression = QStringLiteral( "scale_linear(@zoom_level, %1, %2, %3, %4) * %5" ).arg( stops.value( 0 ).toList().value( 0 ).toString(),
                        stops.last().toList().value( 0 ).toString(),
                        stops.value( 0 ).toList().value( 1 ).toString(),
                        stops.last().toList().value( 1 ).toString() ).arg( multiplier );
    }
    else
    {
      scaleExpression = interpolateExpression( stops.value( 0 ).toList().value( 0 ).toInt(),
                        stops.last().toList().value( 0 ).toInt(),
                        stops.value( 0 ).toList().value( 1 ).toInt(),
                        stops.last().toList().value( 1 ).toInt(), base ) + QStringLiteral( "* %1" ).arg( multiplier );
    }
  }
  else
  {
    scaleExpression = parseStops( base, stops, multiplier, context );
  }
  return QgsProperty::fromExpression( scaleExpression );
}

QgsProperty QgsMapBoxGlStyleConverter::parseInterpolateOpacityByZoom( const QVariantMap &json )
{
  const double base = json.value( QStringLiteral( "base" ), QStringLiteral( "1" ) ).toDouble();
  const QVariantList stops = json.value( QStringLiteral( "stops" ) ).toList();
  if ( stops.empty() )
    return QgsProperty();

  QString scaleExpression;
  if ( stops.length() <= 2 )
  {
    if ( base == 1 )
    {
      scaleExpression = QStringLiteral( "set_color_part(@symbol_color, 'alpha', scale_linear(@zoom_level, %1, %2, %3, %4))" )
                        .arg( stops.value( 0 ).toList().value( 0 ).toString(),
                              stops.last().toList().value( 0 ).toString() )
                        .arg( stops.value( 0 ).toList().value( 1 ).toDouble() * 255 )
                        .arg( stops.last().toList().value( 1 ).toDouble() * 255 );
    }
    else
    {
      scaleExpression = QStringLiteral( "set_color_part(@symbol_color, 'alpha', %1)" )
                        .arg( interpolateExpression( stops.value( 0 ).toList().value( 0 ).toInt(),
                              stops.last().toList().value( 0 ).toInt(),
                              stops.value( 0 ).toList().value( 1 ).toDouble() * 255,
                              stops.value( 0 ).toList().value( 1 ).toDouble() * 255, base ) );
    }
  }
  else
  {
    scaleExpression = parseOpacityStops( base, stops );
  }
  return QgsProperty::fromExpression( scaleExpression );
}

QString QgsMapBoxGlStyleConverter::parseOpacityStops( double base, const QVariantList &stops )
{
  QString caseString = QStringLiteral( "CASE WHEN @zoom_level < %1 THEN set_color_part(@symbol_color, 'alpha', %2 * 255)" )
                       .arg( stops.value( 0 ).toList().value( 0 ).toString(),
                             stops.value( 0 ).toList().value( 1 ).toString() );

  if ( base == 1 )
  {
    // base = 1 -> scale_linear
    for ( int i = 0; i < stops.size() - 1; ++i )
    {
      caseString += QStringLiteral( " WHEN @zoom_level >= %1 AND @zoom_level < %2 "
                                    "THEN set_color_part(@symbol_color, 'alpha', "
                                    "scale_linear(@zoom_level, %1, %2, "
                                    "%3 * 255, %4 * 255)) " )
                    .arg( stops.value( i ).toList().value( 0 ).toString(),
                          stops.value( i + 1 ).toList().value( 0 ).toString(),
                          stops.value( i ).toList().value( 1 ).toString(),
                          stops.value( i + 1 ).toList().value( 1 ).toString() );
    }
  }
  else
  {
    // base != 1 -> scale_expr
    for ( int i = 0; i < stops.size() - 1; ++i )
    {
      caseString += QStringLiteral( " WHEN @zoom_level >= %1 AND @zoom_level < %2 "
                                    "THEN set_color_part(@symbol_color, 'alpha', %3)" )
                    .arg( stops.value( i ).toList().value( 0 ).toString(),
                          stops.value( i + 1 ).toList().value( 0 ).toString(),
                          interpolateExpression( stops.value( i ).toList().value( 0 ).toInt(),
                              stops.value( i + 1 ).toList().value( 0 ).toInt(),
                              stops.value( i ).toList().value( 1 ).toDouble() * 255,
                              stops.value( i + 1 ).toList().value( 1 ).toDouble() * 255, base ) );
    }
  }

  caseString += QStringLiteral( "WHEN @zoom_level >= %1 "
                                "THEN set_color_part(@symbol_color, 'alpha', %2) END" )
                .arg( stops.last().toList().value( 0 ).toString(),
                      stops.last().toList().value( 1 ).toString() );
  return caseString;
}

QString QgsMapBoxGlStyleConverter::parseStops( double base, const QVariantList &stops, double multiplier, QgsMapBoxGlStyleConversionContext &context )
{
  QString caseString = QStringLiteral( "CASE " );

  for ( int i = 0; i < stops.length() - 1; ++i )
  {
    // bottom zoom and value
    const QVariant bz = stops.value( i ).toList().value( 0 );
    const QVariant bv = stops.value( i ).toList().value( 1 );
    if ( bz.type() == QVariant::List || bz.type() == QVariant::StringList )
    {
      context.pushWarning( QObject::tr( "QGIS does not support expressions in interpolation function, skipping." ) );
      return QString();
    }

    // top zoom and value
    const QVariant tz = stops.value( i + 1 ).toList().value( 0 );
    const QVariant tv = stops.value( i + 1 ).toList().value( 1 );
    if ( tz.type() == QVariant::List || tz.type() == QVariant::StringList )
    {
      context.pushWarning( QObject::tr( "QGIS does not support expressions in interpolation function, skipping." ) );
      return QString();
    }

    if ( base == 1 )
    {
      // base = 1 -> scale_linear
      caseString += QStringLiteral( "WHEN @zoom_level > %1 AND @zoom_level <= %2 "
                                    "THEN scale_linear(@zoom_level, %1, %2, %3, %4) "
                                    "* %5 " ).arg( bz.toString(),
                                        tz.toString(),
                                        bv.toString(),
                                        tv.toString() ).arg( multiplier );
    }
    else
    {
      // base != 1 -> scale_exp
      caseString += QStringLiteral( "WHEN @zoom_level > %1 AND @zoom_level <= %2 "
                                    "THEN %3 * %4 " ).arg( bz.toString(),
                                        tz.toString(),
                                        interpolateExpression( bz.toInt(), tz.toInt(), bv.toDouble(), tv.toDouble(), base ) ).arg( multiplier );
    }
  }
  caseString += QStringLiteral( "END" );
  return caseString;
}

QgsProperty QgsMapBoxGlStyleConverter::parseInterpolateListByZoom( const QVariantList &json, PropertyType type, QgsMapBoxGlStyleConversionContext &context, double multiplier, QColor *defaultColor )
{
  if ( json.value( 0 ).toString() != QLatin1String( "interpolate" ) )
  {
    context.pushWarning( QObject::tr( "Could not interpret value list" ) );
    return QgsProperty();
  }

  double base = 1;
  const QString technique = json.value( 1 ).toList().value( 0 ).toString();
  if ( technique == QLatin1String( "linear" ) )
    base = 1;
  else if ( technique == QLatin1String( "exponential" ) )
    base = json.value( 1 ).toList(). value( 1 ).toDouble();
  else if ( technique == QLatin1String( "cubic-bezier" ) )
  {
    context.pushWarning( QObject::tr( "QGIS does not support cubic-bezier interpolation, linear used instead." ) );
    base = 1;
  }
  else
  {
    context.pushWarning( QObject::tr( "Skipping not implemented interpolation method %1" ).arg( technique ) );
    return QgsProperty();
  }

  if ( json.value( 2 ).toList().value( 0 ).toString() != QLatin1String( "zoom" ) )
  {
    context.pushWarning( QObject::tr( "Skipping not implemented interpolation input %1" ).arg( json.value( 2 ).toString() ) );
    return QgsProperty();
  }

  //  Convert stops into list of lists
  QVariantList stops;
  for ( int i = 3; i < json.length(); i += 2 )
  {
    stops.push_back( QVariantList() << json.value( i ).toString() << json.value( i + 1 ).toString() );
  }

  QVariantMap props;
  props.insert( QStringLiteral( "stops" ), stops );
  props.insert( QStringLiteral( "base" ), base );
  switch ( type )
  {
    case PropertyType::Color:
      return parseInterpolateColorByZoom( props, context, defaultColor );

    case PropertyType::Numeric:
      return parseInterpolateByZoom( props, context, multiplier );

    case PropertyType::Opacity:
      return parseInterpolateOpacityByZoom( props );
  }
  return QgsProperty();
}

QColor QgsMapBoxGlStyleConverter::parseColor( const QVariant &color, QgsMapBoxGlStyleConversionContext &context )
{
  if ( color.type() != QVariant::String )
  {
    context.pushWarning( QObject::tr( "Could not parse non-string color %1, skipping" ).arg( color.toString() ) );
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

QString QgsMapBoxGlStyleConverter::interpolateExpression( int zoomMin, int zoomMax, double valueMin, double valueMax, double base )
{
  return QStringLiteral( "%1 + %2 * (%3^(@zoom_level-%4)-1)/(%3^(%5-%4)-1)" ).arg( valueMin )
         .arg( valueMax - valueMin )
         .arg( base )
         .arg( zoomMin )
         .arg( zoomMax );
}

Qt::PenCapStyle QgsMapBoxGlStyleConverter::parseCapStyle( const QString &style )
{
  if ( style == QLatin1String( "round" ) )
    return Qt::RoundCap;
  else if ( style == QLatin1String( "square" ) )
    return Qt::SquareCap;
  else
    return Qt::FlatCap; // "butt" is default
}

Qt::PenJoinStyle QgsMapBoxGlStyleConverter::parseJoinStyle( const QString &style )
{
  if ( style == QLatin1String( "bevel" ) )
    return Qt::BevelJoin;
  else if ( style == QLatin1String( "round" ) )
    return Qt::RoundJoin;
  else
    return Qt::MiterJoin; // "miter" is default
}

QString QgsMapBoxGlStyleConverter::parseExpression( const QVariantList &expression, QgsMapBoxGlStyleConversionContext &context )
{
  QString op = expression.value( 0 ).toString();
  if ( op == QLatin1String( "all" )
       || op == QLatin1String( "any" )
       || op == QLatin1String( "none" ) )
  {
    QStringList parts;
    for ( int i = 1; i < expression.size(); ++i )
    {
      QString part = parseValue( expression.at( i ), context );
      if ( part.isEmpty() )
      {
        context.pushWarning( QObject::tr( "Skipping unsupported expression" ) );
        return QString();
      }
      parts << part;
    }

    if ( op == QLatin1String( "none" ) )
      return QStringLiteral( "NOT (%1)" ).arg( parts.join( QStringLiteral( ") AND NOT (" ) ) );

    QString operatorString;
    if ( op == QLatin1String( "all" ) )
      operatorString = QStringLiteral( ") AND (" );
    else if ( op == QLatin1String( "any" ) )
      operatorString = QStringLiteral( ") OR (" );

    return QStringLiteral( "(%1)" ).arg( parts.join( operatorString ) );
  }
  else if ( op == '!' )
  {
    // ! inverts next expression's meaning
    QVariantList contraJsonExpr = expression.value( 1 ).toList();
    contraJsonExpr[0] = op + contraJsonExpr[0].toString();
    // ['!', ['has', 'level']] -> ['!has', 'level']
    return parseKey( contraJsonExpr );
  }
  else if ( op == QLatin1String( "==" )
            || op == QLatin1String( "!=" )
            || op == QLatin1String( ">=" )
            || op == '>'
            || op == QLatin1String( "<=" )
            || op == '<' )
  {
    // use IS and NOT IS instead of = and != because they can deal with NULL values
    if ( op == QLatin1String( "==" ) )
      op = QStringLiteral( "IS" );
    else if ( op == QLatin1String( "!=" ) )
      op = QStringLiteral( "IS NOT" );
    return QStringLiteral( "%1 %2 %3" ).arg( parseKey( expression.value( 1 ) ),
           op, parseValue( expression.value( 2 ), context ) );
  }
  else if ( op == QLatin1String( "has" ) )
  {
    return parseKey( expression.value( 1 ) ) + QStringLiteral( " IS NOT NULL" );
  }
  else if ( op == QLatin1String( "!has" ) )
  {
    return parseKey( expression.value( 1 ) ) + QStringLiteral( " IS NULL" );
  }
  else if ( op == QLatin1String( "in" ) || op == QLatin1String( "!in" ) )
  {
    const QString key = parseKey( expression.value( 1 ) );
    QStringList parts;
    for ( int i = 2; i < expression.size(); ++i )
    {
      QString part = parseValue( expression.at( i ), context );
      if ( part.isEmpty() )
      {
        context.pushWarning( QObject::tr( "Skipping unsupported expression" ) );
        return QString();
      }
      parts << part;
    }
    if ( op == QLatin1String( "in" ) )
      return QStringLiteral( "%1 IN (%2)" ).arg( key, parts.join( QStringLiteral( ", " ) ) );
    else
      return QStringLiteral( "(%1 IS NULL OR %1 NOT IN (%2))" ).arg( key, parts.join( QStringLiteral( ", " ) ) );
  }
  else if ( op == QLatin1String( "get" ) )
  {
    return parseKey( expression.value( 1 ).toList().value( 1 ) );
  }
  else if ( op == QLatin1String( "match" ) )
  {
    const QString attribute = expression.value( 1 ).toList().value( 1 ).toString();

    if ( expression.size() == 5
         && expression.at( 3 ).type() == QVariant::Bool && expression.at( 3 ).toBool() == true
         && expression.at( 4 ).type() == QVariant::Bool && expression.at( 4 ).toBool() == false )
    {
      // simple case, make a nice simple expression instead of a CASE statement
      if ( expression.at( 2 ).type() == QVariant::List || expression.at( 2 ).type() == QVariant::StringList )
      {
        QStringList parts;
        for ( const QVariant &p : expression.at( 2 ).toList() )
        {
          parts << QgsExpression::quotedValue( p );
        }

        if ( parts.size() > 1 )
          return QStringLiteral( "%1 IN (%2)" ).arg( QgsExpression::quotedColumnRef( attribute ), parts.join( ", " ) );
        else
          return QgsExpression::createFieldEqualityExpression( attribute, expression.at( 2 ).toList().value( 0 ) );
      }
      else if ( expression.at( 2 ).type() == QVariant::String || expression.at( 2 ).type() == QVariant::Int
                || expression.at( 2 ).type() == QVariant::Double )
      {
        return QgsExpression::createFieldEqualityExpression( attribute, expression.at( 2 ) );
      }
      else
      {
        context.pushWarning( QObject::tr( "Skipping non-supported expression" ) );
        return QString();
      }
    }
    else
    {
      QString caseString = QStringLiteral( "CASE " );
      for ( int i = 2; i < expression.size() - 2; i += 2 )
      {
        if ( expression.at( i ).type() == QVariant::List || expression.at( i ).type() == QVariant::StringList )
        {
          QStringList parts;
          for ( const QVariant &p : expression.at( i ).toList() )
          {
            parts << QgsExpression::quotedValue( p );
          }

          if ( parts.size() > 1 )
            caseString += QStringLiteral( "WHEN %1 IN (%2) " ).arg( QgsExpression::quotedColumnRef( attribute ), parts.join( ", " ) );
          else
            caseString += QStringLiteral( "WHEN %1 " ).arg( QgsExpression::createFieldEqualityExpression( attribute, expression.at( i ).toList().value( 0 ) ) );
        }
        else if ( expression.at( i ).type() == QVariant::String || expression.at( i ).type() == QVariant::Int
                  || expression.at( i ).type() == QVariant::Double )
        {
          caseString += QStringLiteral( "WHEN (%1) " ).arg( QgsExpression::createFieldEqualityExpression( attribute, expression.at( i ) ) );
        }

        caseString += QStringLiteral( "THEN %1 " ).arg( QgsExpression::quotedValue( expression.at( i + 1 ) ) );
      }
      caseString += QStringLiteral( "ELSE %1 END" ).arg( QgsExpression::quotedValue( expression.last() ) );
      return caseString;
    }
  }
  else
  {
    context.pushWarning( QObject::tr( "Skipping non-supported expression" ) );
    return QString();
  }
}

QString QgsMapBoxGlStyleConverter::parseValue( const QVariant &value, QgsMapBoxGlStyleConversionContext &context )
{
  switch ( value.type() )
  {
    case QVariant::List:
    case QVariant::StringList:
      return parseExpression( value.toList(), context );

    case QVariant::String:
      return QgsExpression::quotedValue( value.toString() );

    case QVariant::Int:
    case QVariant::Double:
      return value.toString();

    default:
      context.pushWarning( QObject::tr( "Skipping unsupported expression part" ) );
      break;
  }
  return QString();
}

QString QgsMapBoxGlStyleConverter::parseKey( const QVariant &value )
{
  if ( value.toString() == QLatin1String( "$type" ) )
    return QStringLiteral( "_geom_type" );
  else if ( value.type() == QVariant::List || value.type() == QVariant::StringList )
  {
    if ( value.toList().size() > 1 )
      return value.toList().at( 1 ).toString();
    else
      return value.toList().value( 0 ).toString();
  }
  return QgsExpression::quotedColumnRef( value.toString() );
}

QgsVectorTileRenderer *QgsMapBoxGlStyleConverter::renderer() const
{
  return mRenderer ? mRenderer->clone() : nullptr;
}

QgsVectorTileLabeling *QgsMapBoxGlStyleConverter::labeling() const
{
  return mLabeling ? mLabeling->clone() : nullptr;
}

//
// QgsMapBoxGlStyleConversionContext
//
void QgsMapBoxGlStyleConversionContext::pushWarning( const QString &warning )
{
  QgsDebugMsg( warning );
  mWarnings << warning;
}
