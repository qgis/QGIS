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
#include <QList>
#include <QPainter>

void QgsNumericScaleBarRenderer::draw( QgsRenderContext &context, const QgsScaleBarSettings &settings, const ScaleBarContext &scaleContext ) const
{
  if ( !context.painter() )
  {
    return;
  }

  QPainter *painter = context.painter();

  painter->save();
  if ( context.flags() & QgsRenderContext::Antialiasing )
    painter->setRenderHint( QPainter::Antialiasing, true );

  double margin = context.convertToPainterUnits( settings.boxContentSpace(), QgsUnitTypes::RenderMillimeters );
  //map scalebar alignment to Qt::AlignmentFlag type
  QgsTextRenderer::HAlignment hAlign = QgsTextRenderer::AlignLeft;
  switch ( settings.alignment() )
  {
    case QgsScaleBarSettings::AlignLeft:
      hAlign = QgsTextRenderer::AlignLeft;
      break;
    case QgsScaleBarSettings::AlignMiddle:
      hAlign = QgsTextRenderer::AlignCenter;
      break;
    case QgsScaleBarSettings::AlignRight:
      hAlign = QgsTextRenderer::AlignRight;
      break;
  }

  //text destination is item's rect, excluding the margin
  QRectF painterRect( margin, margin, context.convertToPainterUnits( scaleContext.size.width(), QgsUnitTypes::RenderMillimeters ) - 2 * margin,
                      context.convertToPainterUnits( scaleContext.size.height(), QgsUnitTypes::RenderMillimeters ) - 2 * margin );
  QgsTextRenderer::drawText( painterRect, 0, hAlign, QStringList() << scaleText( scaleContext.scale ), context, settings.textFormat() );

  painter->restore();
}

QSizeF QgsNumericScaleBarRenderer::calculateBoxSize( const QgsScaleBarSettings &settings,
    const QgsScaleBarRenderer::ScaleBarContext &scaleContext ) const
{
  QFont font = settings.textFormat().toQFont();

  double textWidth = QgsLayoutUtils::textWidthMM( font, scaleText( scaleContext.scale ) );
  double textHeight = QgsLayoutUtils::fontAscentMM( font );

  return QSizeF( 2 * settings.boxContentSpace() + 2 * settings.pen().width() + textWidth,
                 textHeight + 2 * settings.boxContentSpace() );
}

QString QgsNumericScaleBarRenderer::scaleText( double scale ) const
{
  return "1:" + QStringLiteral( "%L1" ).arg( scale, 0, 'f', 0 );
}
