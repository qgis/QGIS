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
#include "qgscomposerutils.h"
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
  painter->setFont( settings.font() );

  double margin = settings.boxContentSpace();
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
  QRectF painterRect( margin, margin, scaleContext.size.width() - 2 * margin, scaleContext.size.height() - 2 * margin );
  QgsComposerUtils::drawText( painter, painterRect, scaleText( scaleContext.scale ), settings.font(), settings.fontColor(), hAlign, Qt::AlignTop );

  painter->restore();
}

QSizeF QgsNumericScaleBarRenderer::calculateBoxSize( const QgsScaleBarSettings &settings,
    const QgsScaleBarRenderer::ScaleBarContext &scaleContext ) const
{
  double textWidth = QgsComposerUtils::textWidthMM( settings.font(), scaleText( scaleContext.scale ) );
  double textHeight = QgsComposerUtils::fontAscentMM( settings.font() );

  return QSizeF( 2 * settings.boxContentSpace() + 2 * settings.pen().width() + textWidth,
                 textHeight + 2 * settings.boxContentSpace() );
}

QString QgsNumericScaleBarRenderer::scaleText( double scale ) const
{
  return "1:" + QStringLiteral( "%L1" ).arg( scale, 0, 'f', 0 );
}
