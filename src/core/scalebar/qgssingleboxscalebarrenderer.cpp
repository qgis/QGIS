/***************************************************************************
                            qgssingleboxscalebarrenderer.cpp
                            --------------------------------
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

#include "qgssingleboxscalebarrenderer.h"
#include "qgsscalebarsettings.h"
#include "qgscomposerutils.h"
#include <QList>
#include <QPainter>

void QgsSingleBoxScaleBarRenderer::draw( QgsRenderContext &context, const QgsScaleBarSettings &settings, const ScaleBarContext &scaleContext ) const
{
  if ( !context.painter() )
  {
    return;
  }
  QPainter *painter = context.painter();

  double barTopPosition = QgsComposerUtils::fontAscentMM( settings.font() ) + settings.labelBarSpace() + settings.boxContentSpace();

  painter->save();
  if ( context.flags() & QgsRenderContext::Antialiasing )
    painter->setRenderHint( QPainter::Antialiasing, true );
  painter->setPen( settings.pen() );

  bool useColor = true; //alternate brush color/white
  double xOffset = firstLabelXOffset( settings );

  QList<double> positions = segmentPositions( scaleContext, settings );
  QList<double> widths = segmentWidths( scaleContext, settings );

  for ( int i = 0; i < positions.size(); ++i )
  {
    if ( useColor ) //alternating colors
    {
      painter->setBrush( settings.brush() );
    }
    else //secondary color
    {
      painter->setBrush( settings.brush2() );
    }

    QRectF segmentRect( positions.at( i ) + xOffset, barTopPosition, widths.at( i ), settings.height() );
    painter->drawRect( segmentRect );
    useColor = !useColor;
  }

  painter->restore();

  //draw labels using the default method
  drawDefaultLabels( context, settings, scaleContext );
}



