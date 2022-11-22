/***************************************************************************
  qgslegendsettings.cpp
  --------------------------------------
  Date                 : July 2014
  Copyright            : (C) 2014 by Martin Dobias
  Email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgslegendsettings.h"
#include "qgsexpressioncontext.h"
#include "qgsexpression.h"
#include "qgsrendercontext.h"

#include <QPainter>

QgsLegendSettings::QgsLegendSettings()
  : mSymbolSize( 7, 4 )
  , mWmsLegendSize( 50, 25 )
  , mRasterStrokeColor( Qt::black )
{
  rstyle( QgsLegendStyle::Title ).setMargin( QgsLegendStyle::Bottom, 3.5 );
  rstyle( QgsLegendStyle::Group ).setMargin( QgsLegendStyle::Top, 3 );
  rstyle( QgsLegendStyle::Subgroup ).setMargin( QgsLegendStyle::Top, 3 );
  rstyle( QgsLegendStyle::Symbol ).setMargin( QgsLegendStyle::Top, 2.5 );
  rstyle( QgsLegendStyle::SymbolLabel ).setMargin( QgsLegendStyle::Top, 2 );
  rstyle( QgsLegendStyle::SymbolLabel ).setMargin( QgsLegendStyle::Left, 2 );
  rstyle( QgsLegendStyle::Group ).setIndent( 0.0 );
  rstyle( QgsLegendStyle::Subgroup ).setIndent( 0.0 );

  QgsTextFormat f = rstyle( QgsLegendStyle::Title ).textFormat();
  f.setSize( 16.0 );
  f.setSizeUnit( QgsUnitTypes::RenderPoints );
  // these default line heights are not ideal, but needed to maintain api
  f.setLineHeight( 1.1 );
  f.setLineHeightUnit( QgsUnitTypes::RenderPercentage );
  rstyle( QgsLegendStyle::Title ).setTextFormat( f );

  f = rstyle( QgsLegendStyle::Group ).textFormat();
  f.setSize( 14.0 );
  f.setSizeUnit( QgsUnitTypes::RenderPoints );
  f.setLineHeight( 1.1 );
  f.setLineHeightUnit( QgsUnitTypes::RenderPercentage );
  rstyle( QgsLegendStyle::Group ).setTextFormat( f );

  f = rstyle( QgsLegendStyle::Subgroup ).textFormat();
  f.setSize( 12.0 );
  f.setSizeUnit( QgsUnitTypes::RenderPoints );
  f.setLineHeight( 1.1 );
  f.setLineHeightUnit( QgsUnitTypes::RenderPercentage );
  rstyle( QgsLegendStyle::Subgroup ).setTextFormat( f );

  f = rstyle( QgsLegendStyle::SymbolLabel ).textFormat();
  f.setSize( 12.0 );
  f.setSizeUnit( QgsUnitTypes::RenderPoints );
  f.setLineHeight( 1.1 );
  f.setLineHeightUnit( QgsUnitTypes::RenderPercentage );
  rstyle( QgsLegendStyle::SymbolLabel ).setTextFormat( f );
}

QColor QgsLegendSettings::fontColor() const
{
  return style( QgsLegendStyle::SymbolLabel ).textFormat().color();
}

void QgsLegendSettings::setFontColor( const QColor &c )
{
  rstyle( QgsLegendStyle::Title ).textFormat().setColor( c );
  rstyle( QgsLegendStyle::Group ).textFormat().setColor( c );
  rstyle( QgsLegendStyle::Subgroup ).textFormat().setColor( c );
  rstyle( QgsLegendStyle::SymbolLabel ).textFormat().setColor( c );
}

QColor QgsLegendSettings::layerFontColor() const
{
  return style( QgsLegendStyle::Subgroup ).textFormat().color();
}

void QgsLegendSettings::setLayerFontColor( const QColor &fontColor )
{
  rstyle( QgsLegendStyle::Group ).textFormat().setColor( fontColor );
  rstyle( QgsLegendStyle::Subgroup ).textFormat().setColor( fontColor );
}

void QgsLegendSettings::setLineSpacing( double s ) SIP_DEPRECATED
{
  // line spacing *was* a fixed amount (in mm) added between each line of text.
  mLineSpacing = s;

  QgsTextFormat f = rstyle( QgsLegendStyle::Title ).textFormat();
  // assume font sizes in points, since that was what we always had from before this method was deprecated
  f.setLineHeight( f.size() * 0.352778 + s );
  f.setLineHeightUnit( QgsUnitTypes::RenderMillimeters );
  rstyle( QgsLegendStyle::Title ).setTextFormat( f );

  f = rstyle( QgsLegendStyle::Group ).textFormat();
  f.setLineHeight( f.size() * 0.352778 + s );
  f.setLineHeightUnit( QgsUnitTypes::RenderMillimeters );
  rstyle( QgsLegendStyle::Group ).setTextFormat( f );

  f = rstyle( QgsLegendStyle::Subgroup ).textFormat();
  f.setLineHeight( f.size() * 0.352778 + s );
  f.setLineHeightUnit( QgsUnitTypes::RenderMillimeters );
  rstyle( QgsLegendStyle::Subgroup ).setTextFormat( f );

  f = rstyle( QgsLegendStyle::SymbolLabel ).textFormat();
  f.setLineHeight( f.size() * 0.352778 + s );
  f.setLineHeightUnit( QgsUnitTypes::RenderMillimeters );
  rstyle( QgsLegendStyle::SymbolLabel ).setTextFormat( f );
}

double QgsLegendSettings::mmPerMapUnit() const
{
  return mMmPerMapUnit;
}

void QgsLegendSettings::setMmPerMapUnit( double mmPerMapUnit )
{
  mMmPerMapUnit = mmPerMapUnit;
}

bool QgsLegendSettings::useAdvancedEffects() const
{
  return mUseAdvancedEffects;
}

void QgsLegendSettings::setUseAdvancedEffects( bool use )
{
  mUseAdvancedEffects = use;
}

double QgsLegendSettings::mapScale() const
{
  return mMapScale;
}

void QgsLegendSettings::setMapScale( double scale )
{
  mMapScale = scale;
}

double QgsLegendSettings::mapUnitsPerPixel() const
{
  return 1 / ( mMmPerMapUnit * ( mDpi / 25.4 ) );
}

void QgsLegendSettings::setMapUnitsPerPixel( double mapUnitsPerPixel )
{
  mMmPerMapUnit = 1 / mapUnitsPerPixel / ( mDpi / 25.4 );
}

int QgsLegendSettings::dpi() const
{
  return mDpi;
}

void QgsLegendSettings::setDpi( int dpi )
{
  mDpi = dpi;
}

QStringList QgsLegendSettings::evaluateItemText( const QString &text, const QgsExpressionContext &context ) const
{
  const QString textToRender = QgsExpression::replaceExpressionText( text, &context );
  return splitStringForWrapping( textToRender );
}

QStringList QgsLegendSettings::splitStringForWrapping( const QString &stringToSplit ) const
{
  const QStringList lines = stringToSplit.split( '\n' );

  // If the string contains nothing then just return the string without splitting.
  if ( wrapChar().isEmpty() )
    return lines;

  QStringList res;
  for ( const QString &line : lines )
  {
    res.append( line.split( wrapChar() ) );
  }
  return res;
}

#define FONT_WORKAROUND_SCALE 10 //scale factor for upscaling fontsize and downscaling painter


void QgsLegendSettings::drawText( QPainter *p, double x, double y, const QString &text, const QFont &font ) const
{
  const QFont textFont = scaledFontPixelSize( font );

  const QgsScopedQPainterState painterState( p );
  p->setFont( textFont );
  const double scaleFactor = 1.0 / FONT_WORKAROUND_SCALE;
  p->scale( scaleFactor, scaleFactor );
  p->drawText( QPointF( x * FONT_WORKAROUND_SCALE, y * FONT_WORKAROUND_SCALE ), text );
}


void QgsLegendSettings::drawText( QPainter *p, const QRectF &rect, const QString &text, const QFont &font, Qt::AlignmentFlag halignment, Qt::AlignmentFlag valignment, int flags ) const
{
  const QFont textFont = scaledFontPixelSize( font );

  const QRectF scaledRect( rect.x() * FONT_WORKAROUND_SCALE, rect.y() * FONT_WORKAROUND_SCALE,
                           rect.width() * FONT_WORKAROUND_SCALE, rect.height() * FONT_WORKAROUND_SCALE );

  const QgsScopedQPainterState painterState( p );
  p->setFont( textFont );
  const double scaleFactor = 1.0 / FONT_WORKAROUND_SCALE;
  p->scale( scaleFactor, scaleFactor );
  p->drawText( scaledRect, halignment | valignment | flags, text );
}


QFont QgsLegendSettings::scaledFontPixelSize( const QFont &font ) const
{
  QFont scaledFont = font;
  const double pixelSize = pixelFontSize( font.pointSizeF() ) * FONT_WORKAROUND_SCALE + 0.5;
  scaledFont.setPixelSize( pixelSize );
  return scaledFont;
}

double QgsLegendSettings::pixelFontSize( double pointSize ) const
{
  return ( pointSize * 0.3527 );
}

double QgsLegendSettings::textWidthMillimeters( const QFont &font, const QString &text ) const
{
  const QFont metricsFont = scaledFontPixelSize( font );
  const QFontMetricsF fontMetrics( metricsFont );
  return ( fontMetrics.horizontalAdvance( text ) / FONT_WORKAROUND_SCALE );
}

double QgsLegendSettings::fontHeightCharacterMM( const QFont &font, QChar c ) const
{
  const QFont metricsFont = scaledFontPixelSize( font );
  const QFontMetricsF fontMetrics( metricsFont );
  return ( fontMetrics.boundingRect( c ).height() / FONT_WORKAROUND_SCALE );
}

double QgsLegendSettings::fontAscentMillimeters( const QFont &font ) const
{
  const QFont metricsFont = scaledFontPixelSize( font );
  const QFontMetricsF fontMetrics( metricsFont );
  return ( fontMetrics.ascent() / FONT_WORKAROUND_SCALE );
}

double QgsLegendSettings::fontDescentMillimeters( const QFont &font ) const
{
  const QFont metricsFont = scaledFontPixelSize( font );
  const QFontMetricsF fontMetrics( metricsFont );
  return ( fontMetrics.descent() / FONT_WORKAROUND_SCALE );
}

