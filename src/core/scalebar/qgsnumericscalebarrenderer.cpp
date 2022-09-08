/***************************************************************************
                            qgsnumericscalebarrenderer.cpp
                            ------------------------------
    begin                : June 2008
    copyright            : (C) 2008 by Marco Hugentobler
    email                : marco.hugentobler@karto.baug.ethz.ch
 ***************************************************************************/
/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsnumericscalebarrenderer.h"
#include "qgsscalebarsettings.h"
#include "qgslayoututils.h"
#include "qgsnumericformat.h"
#include "qgstextrenderer.h"
#include <QList>
#include <QPainter>

QString QgsNumericScaleBarRenderer::id() const
{
  return QStringLiteral( "Numeric" );
}

QString QgsNumericScaleBarRenderer::visibleName() const
{
  return QObject::tr( "Numeric" );
}

int QgsNumericScaleBarRenderer::sortKey() const
{
  return 100;
}

QgsScaleBarRenderer::Flags QgsNumericScaleBarRenderer::flags() const
{
  return Flag::FlagUsesAlignment;
}

QgsNumericScaleBarRenderer *QgsNumericScaleBarRenderer::clone() const
{
  return new QgsNumericScaleBarRenderer( *this );
}

void QgsNumericScaleBarRenderer::draw( QgsRenderContext &context, const QgsScaleBarSettings &settings, const ScaleBarContext &scaleContext ) const
{
  if ( !context.painter() )
  {
    return;
  }

  QPainter *painter = context.painter();

  const QgsScopedQPainterState painterState( painter );
  context.setPainterFlagsUsingContext( painter );

  const double margin = context.convertToPainterUnits( settings.boxContentSpace(), QgsUnitTypes::RenderMillimeters );
  //map scalebar alignment to Qt::AlignmentFlag type
  Qgis::TextHorizontalAlignment hAlign = Qgis::TextHorizontalAlignment::Left;
  switch ( settings.alignment() )
  {
    case QgsScaleBarSettings::AlignLeft:
      hAlign = Qgis::TextHorizontalAlignment::Left;
      break;
    case QgsScaleBarSettings::AlignMiddle:
      hAlign = Qgis::TextHorizontalAlignment::Center;
      break;
    case QgsScaleBarSettings::AlignRight:
      hAlign = Qgis::TextHorizontalAlignment::Right;
      break;
  }

  //text destination is item's rect, excluding the margin
  const QRectF painterRect( margin, margin, context.convertToPainterUnits( scaleContext.size.width(), QgsUnitTypes::RenderMillimeters ) - 2 * margin,
                            context.convertToPainterUnits( scaleContext.size.height(), QgsUnitTypes::RenderMillimeters ) - 2 * margin );
  QgsTextRenderer::drawText( painterRect, 0, hAlign, QStringList() << scaleText( scaleContext.scale, settings ), context, settings.textFormat() );
}

QSizeF QgsNumericScaleBarRenderer::calculateBoxSize( QgsRenderContext &context, const QgsScaleBarSettings &settings,
    const QgsScaleBarRenderer::ScaleBarContext &scaleContext ) const
{
  const double painterToMm = 1.0 / context.convertToPainterUnits( 1, QgsUnitTypes::RenderMillimeters );

  const double textWidth = QgsTextRenderer::textWidth( context, settings.textFormat(), QStringList() << scaleText( scaleContext.scale, settings ) ) * painterToMm;
  const double textHeight = QgsTextRenderer::textHeight( context, settings.textFormat(), QStringList() << scaleText( scaleContext.scale, settings ) ) * painterToMm;

  return QSizeF( 2 * settings.boxContentSpace() + textWidth,
                 textHeight + 2 * settings.boxContentSpace() );
}

QSizeF QgsNumericScaleBarRenderer::calculateBoxSize( const QgsScaleBarSettings &settings, const QgsScaleBarRenderer::ScaleBarContext &scaleContext ) const
{
  const QFont font = settings.textFormat().toQFont();

  const double textWidth = QgsLayoutUtils::textWidthMM( font, scaleText( scaleContext.scale, settings ) );
  const double textHeight = QgsLayoutUtils::fontAscentMM( font );

  return QSizeF( 2 * settings.boxContentSpace() + textWidth,
                 textHeight + 2 * settings.boxContentSpace() );
}

QString QgsNumericScaleBarRenderer::scaleText( double scale, const QgsScaleBarSettings &settings ) const
{
  return "1:" + settings.numericFormat()->formatDouble( scale, QgsNumericFormatContext() );
}
