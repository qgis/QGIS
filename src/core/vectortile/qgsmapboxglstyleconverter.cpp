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

#include "qgsmapboxglstyleconverter.h"
#include "qgsvectortilebasicrenderer.h"
#include "qgsvectortilebasiclabeling.h"
#include "qgssymbollayer.h"
#include "qgssymbollayerutils.h"
#include "qgslogger.h"
#include "qgsfillsymbollayer.h"

QgsMapBoxGlStyleConverter::QgsMapBoxGlStyleConverter( const QVariantMap &style, const QString &styleName )
  : mStyle( style )
{
  if ( mStyle.contains( QStringLiteral( "layers" ) ) )
  {
    parseLayers( mStyle.value( QStringLiteral( "layers" ) ).toList(), styleName );
  }
  else
  {
    mError = QObject::tr( "Could not find layers list in JSON" );
  }
}

QgsMapBoxGlStyleConverter::~QgsMapBoxGlStyleConverter() = default;

void QgsMapBoxGlStyleConverter::parseLayers( const QVariantList &layers, const QString &styleName )
{
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
      // filterExpression = parseExpression( jsonLayer.value( QStringLiteral( "filter" ) ) );
    }

    QgsVectorTileBasicRendererStyle rendererStyle;
    QgsVectorTileBasicLabelingStyle labelingStyle;

    bool hasRendererStyle = false;
    bool hasLabelingStyle = false;
    if ( layerType == QLatin1String( "fill" ) )
    {
      hasRendererStyle = parseFillLayer( jsonLayer, styleName, rendererStyle );
    }
    else if ( layerType == QLatin1String( "line" ) )
    {
      // hasRendererStyle = parseLineLayer( jsonLayer, styleName );
    }
    else if ( layerType == QLatin1String( "symbol" ) )
    {
      // hasLabelingStyle = parseSymbolLayer( jsonLayer, styleName );
    }
    else
    {
      QgsDebugMsg( QStringLiteral( "Skipping unknown layer type: %1" ).arg( layerType ) );
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

  }

  mRenderer = qgis::make_unique< QgsVectorTileBasicRenderer >();
  QgsVectorTileBasicRenderer *renderer = dynamic_cast< QgsVectorTileBasicRenderer *>( mRenderer.get() );
  renderer->setStyles( rendererStyles );

  mLabeling = qgis::make_unique< QgsVectorTileBasicLabeling >();
  QgsVectorTileBasicLabeling *labeling = dynamic_cast< QgsVectorTileBasicLabeling * >( mLabeling.get() );
  labeling->setStyles( labelingStyles );
}

bool QgsMapBoxGlStyleConverter::parseFillLayer( const QVariantMap &jsonLayer, const QString &, QgsVectorTileBasicRendererStyle &style )
{
  if ( !jsonLayer.contains( QStringLiteral( "paint" ) ) )
  {
    QgsDebugMsg( QStringLiteral( "Style layer %1 has no paint property, skipping" ).arg( jsonLayer.value( QStringLiteral( "id" ) ).toString() ) );
    return false;
  }

  const QVariantMap jsonPaint = jsonLayer.value( QStringLiteral( "paint" ) ).toMap();

  QgsPropertyCollection ddProperties;

  // fill color
  QColor fillColor;
  if ( jsonLayer.contains( QStringLiteral( "fill-color" ) ) )
  {
    const QVariant jsonFillColor = jsonPaint.value( QStringLiteral( "fill-color" ) );
    switch ( jsonFillColor.type() )
    {
      case QVariant::Map:
        ddProperties.setProperty( QgsSymbolLayer::PropertyFillColor, parseInterpolateColorByZoom( jsonFillColor.toMap() ) );
        break;

      case QVariant::List:
      case QVariant::StringList:
        ddProperties.setProperty( QgsSymbolLayer::PropertyFillColor, parseInterpolateListByZoom( jsonFillColor.toList(), PropertyType::Color ) );
        break;

      case QVariant::String:
        fillColor = parseColor( jsonFillColor.toString() );
        break;

      default:
        QgsDebugMsg( "Skipping non-implemented color expression" );
        break;
    }
  }

  QColor fillOutlineColor;
  if ( !jsonLayer.contains( QStringLiteral( "fill-outline-color" ) ) )
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
        ddProperties.setProperty( QgsSymbolLayer::PropertyStrokeColor, parseInterpolateColorByZoom( jsonFillOutlineColor.toMap() ) );
        break;

      case QVariant::List:
      case QVariant::StringList:
        ddProperties.setProperty( QgsSymbolLayer::PropertyStrokeColor, parseInterpolateListByZoom( jsonFillOutlineColor.toList(), PropertyType::Color ) );
        break;

      case QVariant::String:
        fillOutlineColor = parseColor( jsonFillOutlineColor.toString() );
        break;

      default:
        QgsDebugMsg( "Skipping non-implemented color expression" );
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
          QgsDebugMsg( QStringLiteral( "Could not set opacity of layer %1, opacity already defined in fill color" ).arg( jsonLayer.value( QStringLiteral( "id" ) ).toString() ) );
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
          QgsDebugMsg( QStringLiteral( "Could not set opacity of layer %1, opacity already defined in fill color" ).arg( jsonLayer.value( QStringLiteral( "id" ) ).toString() ) );
        }
        else
        {
          ddProperties.setProperty( QgsSymbolLayer::PropertyFillColor, parseInterpolateListByZoom( jsonFillOpacity.toList(), PropertyType::Opacity ) );
          ddProperties.setProperty( QgsSymbolLayer::PropertyStrokeColor, parseInterpolateListByZoom( jsonFillOpacity.toList(), PropertyType::Opacity ) );
        }
        break;

      default:
        QgsDebugMsg( "Skipping non-implemented opacity expression" );
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

QgsProperty QgsMapBoxGlStyleConverter::parseInterpolateColorByZoom( const QVariantMap &json )
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

      const QColor bottomColor = parseColor( stops.at( i ).toList().value( 1 ) );
      const QColor topColor = parseColor( stops.at( i + 1 ).toList().value( 1 ) );

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

      const QColor bottomColor = parseColor( stops.at( i ).toList().value( 1 ) );
      const QColor topColor = parseColor( stops.at( i + 1 ).toList().value( 1 ) );

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
  const QColor topColor = parseColor( stops.last().toList().value( 1 ) );
  int tcHue;
  int tcSat;
  int tcLight;
  int tcAlpha;
  colorAsHslaComponents( topColor, tcHue, tcSat, tcLight, tcAlpha );

  caseString += QStringLiteral( "WHEN @zoom_level >= %1 THEN color_hsla(%2, %3, %4, %5) "
                                "ELSE color_hsla(%2, %3, %4, %5) END" ).arg( tz )
                .arg( tcHue ).arg( tcSat ).arg( tcLight ).arg( tcAlpha );

  return QgsProperty::fromExpression( caseString );
}

QgsProperty QgsMapBoxGlStyleConverter::parseInterpolateByZoom( const QVariantMap &json, double multiplier )
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
    scaleExpression = parseStops( base, stops, multiplier );
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

QString QgsMapBoxGlStyleConverter::parseStops( double base, const QVariantList &stops, double multiplier )
{
  QString caseString = QStringLiteral( "CASE " );

  for ( int i = 0; i < stops.length() - 1; ++i )
  {
    // bottom zoom and value
    const QVariant bz = stops.value( i ).toList().value( 0 );
    const QVariant bv = stops.value( i ).toList().value( 1 );
    if ( bz.type() == QVariant::List || bz.type() == QVariant::StringList )
    {
      QgsDebugMsg( "QGIS does not support expressions in interpolation function, skipping." );
      return QString();
    }

    // top zoom and value
    const QVariant tz = stops.value( i + 1 ).toList().value( 0 );
    const QVariant tv = stops.value( i + 1 ).toList().value( 1 );
    if ( tz.type() == QVariant::List || tz.type() == QVariant::StringList )
    {
      QgsDebugMsg( "QGIS does not support expressions in interpolation function, skipping." );
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

QgsProperty QgsMapBoxGlStyleConverter::parseInterpolateListByZoom( const QVariantList &json, PropertyType type, double multiplier )
{
  if ( json.value( 0 ).toString() != QLatin1String( "interpolate" ) )
  {
    QgsDebugMsg( QStringLiteral( "Could not interpret value list" ) );
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
    QgsDebugMsg( QStringLiteral( "QGIS does not support cubic-bezier interpolation, linear used instead." ) );
    base = 1;
  }
  else
  {
    QgsDebugMsg( QStringLiteral( "Skipping not implemented interpolation method %1" ).arg( technique ) );
    return QgsProperty();
  }

  if ( json.value( 2 ).toList().value( 0 ).toString() != QLatin1String( "zoom" ) )
  {
    QgsDebugMsg( QStringLiteral( "Skipping not implemented interpolation input %1" ).arg( json.value( 2 ).toString() ) );
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
      return parseInterpolateColorByZoom( props );

    case PropertyType::Line:
    case PropertyType::Text:
      return parseInterpolateByZoom( props, multiplier );

    case PropertyType::Opacity:
      return parseInterpolateOpacityByZoom( props );
  }
  return QgsProperty();
}

QColor QgsMapBoxGlStyleConverter::parseColor( const QVariant &color )
{
  if ( color.type() != QVariant::String )
  {
    QgsDebugMsg( QStringLiteral( "Could not parse non-string color %1, skipping" ).arg( color.toString() ) );
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

QgsVectorTileRenderer *QgsMapBoxGlStyleConverter::renderer() const
{
  return mRenderer ? mRenderer->clone() : nullptr;
}

QgsVectorTileLabeling *QgsMapBoxGlStyleConverter::labeling() const
{
  return mLabeling ? mLabeling->clone() : nullptr;
}
