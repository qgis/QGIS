/***************************************************************************
   qgssymbolconverteresrirest.cpp
   ----------------------
    begin                : February 2026
    copyright            : (C) 2026 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

#include "qgssymbolconverteresrirest.h"

#include "qgsfillsymbol.h"
#include "qgsfillsymbollayer.h"
#include "qgsjsonutils.h"
#include "qgslinesymbol.h"
#include "qgslinesymbollayer.h"
#include "qgsmarkersymbol.h"
#include "qgsmarkersymbollayer.h"

#include <QString>

using namespace Qt::StringLiterals;

Qgis::SymbolConverterCapabilities QgsSymbolConverterEsriRest::capabilities() const
{
  return Qgis::SymbolConverterCapability::ReadSymbol;
}

QString QgsSymbolConverterEsriRest::name() const
{
  return u"esri_rest"_s;
}

QString QgsSymbolConverterEsriRest::formatName() const
{
  return QObject::tr( "ESRI REST JSON" );
}

QVariant QgsSymbolConverterEsriRest::toVariant( const QgsSymbol *, QgsSymbolConverterContext & ) const
{
  throw QgsNotSupportedException( u"This symbol converter does not support serialization of symbols"_s );
}

std::unique_ptr< QgsSymbol > QgsSymbolConverterEsriRest::createSymbol( const QVariant &variant, QgsSymbolConverterContext & ) const
{
  QVariantMap symbolData;
  if ( variant.type() == QVariant::Map )
  {
    symbolData = variant.toMap();
  }
  else if ( variant.type() == QVariant::String )
  {
    const QVariant v = QgsJsonUtils::parseJson( variant.toString() );
    if ( v.type() == QVariant::Map )
    {
      symbolData = v.toMap();
    }
  }
  if ( symbolData.isEmpty() )
    return nullptr;

  const QString type = symbolData.value( u"type"_s ).toString();
  if ( type == "esriSMS"_L1 )
  {
    // marker symbol
    return parseEsriMarkerSymbolJson( symbolData );
  }
  else if ( type == "esriSLS"_L1 )
  {
    // line symbol
    return parseEsriLineSymbolJson( symbolData );
  }
  else if ( type == "esriSFS"_L1 )
  {
    // fill symbol
    return parseEsriFillSymbolJson( symbolData );
  }
  else if ( type == "esriPFS"_L1 )
  {
    return parseEsriPictureFillSymbolJson( symbolData );
  }
  else if ( type == "esriPMS"_L1 )
  {
    // picture marker
    return parseEsriPictureMarkerSymbolJson( symbolData );
  }
  else if ( type == "esriTS"_L1 )
  {
    return parseEsriTextMarkerSymbolJson( symbolData );
  }
  return nullptr;
}

std::unique_ptr<QgsLineSymbol> QgsSymbolConverterEsriRest::parseEsriLineSymbolJson( const QVariantMap &symbolData )
{
  const QColor lineColor = convertColor( symbolData.value( u"color"_s ) );
  if ( !lineColor.isValid() )
    return nullptr;

  bool ok = false;
  const double widthInPoints = symbolData.value( u"width"_s ).toDouble( &ok );
  if ( !ok )
    return nullptr;

  QgsSymbolLayerList layers;
  const Qt::PenStyle penStyle = convertLineStyle( symbolData.value( u"style"_s ).toString() );
  auto lineLayer = std::make_unique< QgsSimpleLineSymbolLayer >( lineColor, widthInPoints, penStyle );
  lineLayer->setWidthUnit( Qgis::RenderUnit::Points );
  layers.append( lineLayer.release() );

  auto symbol = std::make_unique< QgsLineSymbol >( layers );
  return symbol;
}

std::unique_ptr<QgsFillSymbol> QgsSymbolConverterEsriRest::parseEsriFillSymbolJson( const QVariantMap &symbolData )
{
  const QColor fillColor = convertColor( symbolData.value( u"color"_s ) );
  const Qt::BrushStyle brushStyle = convertFillStyle( symbolData.value( u"style"_s ).toString() );

  const QVariantMap outlineData = symbolData.value( u"outline"_s ).toMap();
  const QColor lineColor = convertColor( outlineData.value( u"color"_s ) );
  const Qt::PenStyle penStyle = convertLineStyle( outlineData.value( u"style"_s ).toString() );
  bool ok = false;
  const double penWidthInPoints = outlineData.value( u"width"_s ).toDouble( &ok );

  QgsSymbolLayerList layers;
  auto fillLayer = std::make_unique< QgsSimpleFillSymbolLayer >( fillColor, brushStyle, lineColor, penStyle, penWidthInPoints );
  fillLayer->setStrokeWidthUnit( Qgis::RenderUnit::Points );
  layers.append( fillLayer.release() );

  auto symbol = std::make_unique< QgsFillSymbol >( layers );
  return symbol;
}

std::unique_ptr<QgsFillSymbol> QgsSymbolConverterEsriRest::parseEsriPictureFillSymbolJson( const QVariantMap &symbolData )
{
  bool ok = false;

  double widthInPixels = symbolData.value( u"width"_s ).toInt( &ok );
  if ( !ok )
    return nullptr;

  const double xScale = symbolData.value( u"xscale"_s ).toDouble( &ok );
  if ( !qgsDoubleNear( xScale, 0.0 ) )
    widthInPixels *= xScale;

  const double angleCCW = symbolData.value( u"angle"_s ).toDouble( &ok );
  double angleCW = 0;
  if ( ok )
    angleCW = -angleCCW;

  const double xOffset = symbolData.value( u"xoffset"_s ).toDouble();
  const double yOffset = symbolData.value( u"yoffset"_s ).toDouble();

  QString symbolPath( symbolData.value( u"imageData"_s ).toString() );
  symbolPath.prepend( "base64:"_L1 );

  QgsSymbolLayerList layers;
  auto fillLayer = std::make_unique< QgsRasterFillSymbolLayer >( symbolPath );
  fillLayer->setWidth( widthInPixels );
  fillLayer->setAngle( angleCW );
  fillLayer->setSizeUnit( Qgis::RenderUnit::Points );
  fillLayer->setOffset( QPointF( xOffset, yOffset ) );
  fillLayer->setOffsetUnit( Qgis::RenderUnit::Points );
  layers.append( fillLayer.release() );

  const QVariantMap outlineData = symbolData.value( u"outline"_s ).toMap();
  const QColor lineColor = convertColor( outlineData.value( u"color"_s ) );
  const Qt::PenStyle penStyle = convertLineStyle( outlineData.value( u"style"_s ).toString() );
  const double penWidthInPoints = outlineData.value( u"width"_s ).toDouble( &ok );

  auto lineLayer = std::make_unique< QgsSimpleLineSymbolLayer >( lineColor, penWidthInPoints, penStyle );
  lineLayer->setWidthUnit( Qgis::RenderUnit::Points );
  layers.append( lineLayer.release() );

  auto symbol = std::make_unique< QgsFillSymbol >( layers );
  return symbol;
}

Qgis::MarkerShape QgsSymbolConverterEsriRest::parseEsriMarkerShape( const QString &style )
{
  if ( style == "esriSMSCircle"_L1 )
    return Qgis::MarkerShape::Circle;
  else if ( style == "esriSMSCross"_L1 )
    return Qgis::MarkerShape::Cross;
  else if ( style == "esriSMSDiamond"_L1 )
    return Qgis::MarkerShape::Diamond;
  else if ( style == "esriSMSSquare"_L1 )
    return Qgis::MarkerShape::Square;
  else if ( style == "esriSMSX"_L1 )
    return Qgis::MarkerShape::Cross2;
  else if ( style == "esriSMSTriangle"_L1 )
    return Qgis::MarkerShape::Triangle;
  else
    return Qgis::MarkerShape::Circle;
}

std::unique_ptr<QgsMarkerSymbol> QgsSymbolConverterEsriRest::parseEsriMarkerSymbolJson( const QVariantMap &symbolData )
{
  QColor fillColor = convertColor( symbolData.value( u"color"_s ) );
  bool ok = false;
  const double sizeInPoints = symbolData.value( u"size"_s ).toDouble( &ok );
  if ( !ok )
    return nullptr;
  const double angleCCW = symbolData.value( u"angle"_s ).toDouble( &ok );
  double angleCW = 0;
  if ( ok )
    angleCW = -angleCCW;

  const Qgis::MarkerShape shape = parseEsriMarkerShape( symbolData.value( u"style"_s ).toString() );

  const double xOffset = symbolData.value( u"xoffset"_s ).toDouble();
  const double yOffset = symbolData.value( u"yoffset"_s ).toDouble();

  const QVariantMap outlineData = symbolData.value( u"outline"_s ).toMap();
  const QColor lineColor = convertColor( outlineData.value( u"color"_s ) );
  const Qt::PenStyle penStyle = convertLineStyle( outlineData.value( u"style"_s ).toString() );
  double penWidthInPoints = outlineData.value( u"width"_s ).toDouble( &ok );

  QgsSymbolLayerList layers;
  auto markerLayer = std::make_unique< QgsSimpleMarkerSymbolLayer >( shape, sizeInPoints, angleCW, Qgis::ScaleMethod::ScaleArea, fillColor, lineColor );
  markerLayer->setSizeUnit( Qgis::RenderUnit::Points );
  markerLayer->setStrokeWidthUnit( Qgis::RenderUnit::Points );
  markerLayer->setStrokeStyle( penStyle );
  markerLayer->setStrokeWidth( penWidthInPoints );
  markerLayer->setOffset( QPointF( xOffset, yOffset ) );
  markerLayer->setOffsetUnit( Qgis::RenderUnit::Points );
  layers.append( markerLayer.release() );

  auto symbol = std::make_unique< QgsMarkerSymbol >( layers );
  return symbol;
}

std::unique_ptr<QgsMarkerSymbol> QgsSymbolConverterEsriRest::parseEsriPictureMarkerSymbolJson( const QVariantMap &symbolData )
{
  bool ok = false;
  const double widthInPixels = symbolData.value( u"width"_s ).toInt( &ok );
  if ( !ok )
    return nullptr;
  const double heightInPixels = symbolData.value( u"height"_s ).toInt( &ok );
  if ( !ok )
    return nullptr;

  const double angleCCW = symbolData.value( u"angle"_s ).toDouble( &ok );
  double angleCW = 0;
  if ( ok )
    angleCW = -angleCCW;

  const double xOffset = symbolData.value( u"xoffset"_s ).toDouble();
  const double yOffset = symbolData.value( u"yoffset"_s ).toDouble();

  QString symbolPath( symbolData.value( u"imageData"_s ).toString() );
  symbolPath.prepend( "base64:"_L1 );

  QgsSymbolLayerList layers;
  auto markerLayer = std::make_unique< QgsRasterMarkerSymbolLayer >( symbolPath, widthInPixels, angleCW, Qgis::ScaleMethod::ScaleArea );
  markerLayer->setSizeUnit( Qgis::RenderUnit::Points );

  // only change the default aspect ratio if the server height setting requires this
  if ( !qgsDoubleNear( static_cast< double >( heightInPixels ) / widthInPixels, markerLayer->defaultAspectRatio() ) )
    markerLayer->setFixedAspectRatio( static_cast< double >( heightInPixels ) / widthInPixels );

  markerLayer->setOffset( QPointF( xOffset, yOffset ) );
  markerLayer->setOffsetUnit( Qgis::RenderUnit::Points );
  layers.append( markerLayer.release() );

  auto symbol = std::make_unique< QgsMarkerSymbol >( layers );
  return symbol;
}

std::unique_ptr<QgsMarkerSymbol> QgsSymbolConverterEsriRest::parseEsriTextMarkerSymbolJson( const QVariantMap &symbolData )
{
  QgsSymbolLayerList layers;

  const QString fontFamily = symbolData.value( u"font"_s ).toMap().value( u"family"_s ).toString();
  const QString chr = symbolData.value( u"text"_s ).toString();
  const double pointSize = symbolData.value( u"font"_s ).toMap().value( u"size"_s ).toDouble();
  const QColor color = convertColor( symbolData.value( u"color"_s ) );
  const double esriAngle = symbolData.value( u"angle"_s ).toDouble();
  const double angle = 90.0 - esriAngle;

  auto markerLayer = std::make_unique< QgsFontMarkerSymbolLayer >( fontFamily, chr, pointSize, color, angle );

  QColor strokeColor = convertColor( symbolData.value( u"borderLineColor"_s ) );
  markerLayer->setStrokeColor( strokeColor );

  double borderLineSize = symbolData.value( u"borderLineSize"_s ).toDouble();
  markerLayer->setStrokeWidth( borderLineSize );

  const QString fontStyle = symbolData.value( u"font"_s ).toMap().value( u"style"_s ).toString();
  markerLayer->setFontStyle( fontStyle );

  double xOffset = symbolData.value( u"xoffset"_s ).toDouble();
  double yOffset = symbolData.value( u"yoffset"_s ).toDouble();

  markerLayer->setOffset( QPointF( xOffset, yOffset ) );
  markerLayer->setOffsetUnit( Qgis::RenderUnit::Points );

  markerLayer->setSizeUnit( Qgis::RenderUnit::Points );
  markerLayer->setStrokeWidthUnit( Qgis::RenderUnit::Points );

  Qgis::HorizontalAnchorPoint hAlign = Qgis::HorizontalAnchorPoint::Center;
  Qgis::VerticalAnchorPoint vAlign = Qgis::VerticalAnchorPoint::Center;

  QString horizontalAnchorPoint = symbolData.value( u"horizontalAlignment"_s ).toString();
  QString verticalAnchorPoint = symbolData.value( u"verticalAlignment"_s ).toString();

  if ( horizontalAnchorPoint == QString( "center" ) )
  {
    hAlign = Qgis::HorizontalAnchorPoint::Center;
  }
  else if ( horizontalAnchorPoint == QString( "left" ) )
  {
    hAlign = Qgis::HorizontalAnchorPoint::Left;
  }
  else if ( horizontalAnchorPoint == QString( "right" ) )
  {
    hAlign = Qgis::HorizontalAnchorPoint::Right;
  }

  if ( verticalAnchorPoint == QString( "center" ) )
  {
    vAlign = Qgis::VerticalAnchorPoint::Center;
  }
  else if ( verticalAnchorPoint == QString( "top" ) )
  {
    vAlign = Qgis::VerticalAnchorPoint::Top;
  }
  else if ( verticalAnchorPoint == QString( "bottom" ) )
  {
    vAlign = Qgis::VerticalAnchorPoint::Bottom;
  }

  markerLayer->setHorizontalAnchorPoint( hAlign );
  markerLayer->setVerticalAnchorPoint( vAlign );

  layers.append( markerLayer.release() );

  auto symbol = std::make_unique< QgsMarkerSymbol >( layers );
  return symbol;
}

QColor QgsSymbolConverterEsriRest::convertColor( const QVariant &colorData )
{
  const QVariantList colorParts = colorData.toList();
  if ( colorParts.count() < 4 )
    return QColor();

  const int red = colorParts.at( 0 ).toInt();
  const int green = colorParts.at( 1 ).toInt();
  const int blue = colorParts.at( 2 ).toInt();
  const int alpha = colorParts.at( 3 ).toInt();
  return QColor( red, green, blue, alpha );
}

Qt::PenStyle QgsSymbolConverterEsriRest::convertLineStyle( const QString &style )
{
  if ( style == "esriSLSSolid"_L1 )
    return Qt::SolidLine;
  else if ( style == "esriSLSDash"_L1 )
    return Qt::DashLine;
  else if ( style == "esriSLSDashDot"_L1 )
    return Qt::DashDotLine;
  else if ( style == "esriSLSDashDotDot"_L1 )
    return Qt::DashDotDotLine;
  else if ( style == "esriSLSDot"_L1 )
    return Qt::DotLine;
  else if ( style == "esriSLSNull"_L1 )
    return Qt::NoPen;
  else
    return Qt::SolidLine;
}

Qt::BrushStyle QgsSymbolConverterEsriRest::convertFillStyle( const QString &style )
{
  if ( style == "esriSFSBackwardDiagonal"_L1 )
    return Qt::BDiagPattern;
  else if ( style == "esriSFSCross"_L1 )
    return Qt::CrossPattern;
  else if ( style == "esriSFSDiagonalCross"_L1 )
    return Qt::DiagCrossPattern;
  else if ( style == "esriSFSForwardDiagonal"_L1 )
    return Qt::FDiagPattern;
  else if ( style == "esriSFSHorizontal"_L1 )
    return Qt::HorPattern;
  else if ( style == "esriSFSNull"_L1 )
    return Qt::NoBrush;
  else if ( style == "esriSFSSolid"_L1 )
    return Qt::SolidPattern;
  else if ( style == "esriSFSVertical"_L1 )
    return Qt::VerPattern;
  else
    return Qt::SolidPattern;
}
