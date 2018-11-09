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

#include <QPainter>

QgsLegendSettings::QgsLegendSettings()
  : mFontColor( QColor( 0, 0, 0 ) )
  , mSymbolSize( 7, 4 )
  , mWmsLegendSize( 50, 25 )
  , mRasterStrokeColor( Qt::black )
{
  rstyle( QgsLegendStyle::Title ).setMargin( QgsLegendStyle::Bottom, 3.5 );
  rstyle( QgsLegendStyle::Group ).setMargin( QgsLegendStyle::Top, 3 );
  rstyle( QgsLegendStyle::Subgroup ).setMargin( QgsLegendStyle::Top, 3 );
  rstyle( QgsLegendStyle::Symbol ).setMargin( QgsLegendStyle::Top, 2.5 );
  rstyle( QgsLegendStyle::SymbolLabel ).setMargin( QgsLegendStyle::Top, 2 );
  rstyle( QgsLegendStyle::SymbolLabel ).setMargin( QgsLegendStyle::Left, 2 );
  rstyle( QgsLegendStyle::Title ).rfont().setPointSizeF( 16.0 );
  rstyle( QgsLegendStyle::Group ).rfont().setPointSizeF( 14.0 );
  rstyle( QgsLegendStyle::Subgroup ).rfont().setPointSizeF( 12.0 );
  rstyle( QgsLegendStyle::SymbolLabel ).rfont().setPointSizeF( 12.0 );
}

QStringList QgsLegendSettings::splitStringForWrapping( const QString &stringToSplt ) const
{
  QStringList list;
  // If the string contains nothing then just return the string without splitting.
  if ( wrapChar().count() == 0 )
    list << stringToSplt;
  else
    list = stringToSplt.split( wrapChar() );
  return list;
}

#define FONT_WORKAROUND_SCALE 10 //scale factor for upscaling fontsize and downscaling painter


void QgsLegendSettings::drawText( QPainter *p, double x, double y, const QString &text, const QFont &font ) const
{
  QFont textFont = scaledFontPixelSize( font );

  p->save();
  p->setFont( textFont );
  double scaleFactor = 1.0 / FONT_WORKAROUND_SCALE;
  p->scale( scaleFactor, scaleFactor );
  p->drawText( QPointF( x * FONT_WORKAROUND_SCALE, y * FONT_WORKAROUND_SCALE ), text );
  p->restore();
}


void QgsLegendSettings::drawText( QPainter *p, const QRectF &rect, const QString &text, const QFont &font, Qt::AlignmentFlag halignment, Qt::AlignmentFlag valignment, int flags ) const
{
  QFont textFont = scaledFontPixelSize( font );

  QRectF scaledRect( rect.x() * FONT_WORKAROUND_SCALE, rect.y() * FONT_WORKAROUND_SCALE,
                     rect.width() * FONT_WORKAROUND_SCALE, rect.height() * FONT_WORKAROUND_SCALE );

  p->save();
  p->setFont( textFont );
  double scaleFactor = 1.0 / FONT_WORKAROUND_SCALE;
  p->scale( scaleFactor, scaleFactor );
  p->drawText( scaledRect, halignment | valignment | flags, text );
  p->restore();
}


QFont QgsLegendSettings::scaledFontPixelSize( const QFont &font ) const
{
  QFont scaledFont = font;
  double pixelSize = pixelFontSize( font.pointSizeF() ) * FONT_WORKAROUND_SCALE + 0.5;
  scaledFont.setPixelSize( pixelSize );
  return scaledFont;
}

double QgsLegendSettings::pixelFontSize( double pointSize ) const
{
  return ( pointSize * 0.3527 );
}

double QgsLegendSettings::textWidthMillimeters( const QFont &font, const QString &text ) const
{
  QFont metricsFont = scaledFontPixelSize( font );
  QFontMetricsF fontMetrics( metricsFont );
  return ( fontMetrics.width( text ) / FONT_WORKAROUND_SCALE );
}

double QgsLegendSettings::fontHeightCharacterMM( const QFont &font, QChar c ) const
{
  QFont metricsFont = scaledFontPixelSize( font );
  QFontMetricsF fontMetrics( metricsFont );
  return ( fontMetrics.boundingRect( c ).height() / FONT_WORKAROUND_SCALE );
}

double QgsLegendSettings::fontAscentMillimeters( const QFont &font ) const
{
  QFont metricsFont = scaledFontPixelSize( font );
  QFontMetricsF fontMetrics( metricsFont );
  return ( fontMetrics.ascent() / FONT_WORKAROUND_SCALE );
}

double QgsLegendSettings::fontDescentMillimeters( const QFont &font ) const
{
  QFont metricsFont = scaledFontPixelSize( font );
  QFontMetricsF fontMetrics( metricsFont );
  return ( fontMetrics.descent() / FONT_WORKAROUND_SCALE );
}

