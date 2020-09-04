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

bool QgsMapBoxGlStyleConverter::parseFillLayer( const QVariantMap &jsonLayer, const QString &styleName, QgsVectorTileBasicRendererStyle &style )
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

        break;

      case QVariant::String:

        break;

      default:
        break;
    }

  }


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
