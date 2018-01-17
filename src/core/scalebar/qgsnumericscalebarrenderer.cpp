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

  QFont scaledFont = settings.font();
  scaledFont.setPointSizeF( scaledFont.pointSizeF() * context.scaleFactor() );

  double margin = context.convertToPainterUnits( settings.boxContentSpace(), QgsUnitTypes::RenderMillimeters );
  //map scalebar alignment to Qt::AlignmentFlag type
  Qt::AlignmentFlag hAlign = Qt::AlignLeft;
  switch ( settings.alignment() )
  {
    case QgsScaleBarSettings::AlignLeft:
      hAlign = Qt::AlignLeft;
      break;
    case QgsScaleBarSettings::AlignMiddle:
      hAlign = Qt::AlignHCenter;
      break;
    case QgsScaleBarSettings::AlignRight:
      hAlign = Qt::AlignRight;
      break;
  }

  //text destination is item's rect, excluding the margin
  QRectF painterRect( margin, margin, context.convertToPainterUnits( scaleContext.size.width(), QgsUnitTypes::RenderMillimeters ) - 2 * margin,
                      context.convertToPainterUnits( scaleContext.size.height(), QgsUnitTypes::RenderMillimeters ) - 2 * margin );
  QgsLayoutUtils::drawText( painter, painterRect, scaleText( scaleContext.scale ), scaledFont, settings.fontColor(), hAlign, Qt::AlignTop );

  painter->restore();
}

QSizeF QgsNumericScaleBarRenderer::calculateBoxSize( const QgsScaleBarSettings &settings,
    const QgsScaleBarRenderer::ScaleBarContext &scaleContext ) const
{
  double textWidth = QgsLayoutUtils::textWidthMM( settings.font(), scaleText( scaleContext.scale ) );
  double textHeight = QgsLayoutUtils::fontAscentMM( settings.font() );

  return QSizeF( 2 * settings.boxContentSpace() + 2 * settings.pen().width() + textWidth,
                 textHeight + 2 * settings.boxContentSpace() );
}

QString QgsNumericScaleBarRenderer::scaleText( double scale ) const
{
  return "1:" + QStringLiteral( "%L1" ).arg( scale, 0, 'f', 0 );
}
