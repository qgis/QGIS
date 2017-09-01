/***************************************************************************
                            qgsticksscalebarrenderer.cpp
                            ----------------------------
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

#include "qgsticksscalebarrenderer.h"
#include "qgsscalebarsettings.h"
#include "qgscomposerutils.h"
#include <QPainter>

QString QgsTicksScaleBarRenderer::name() const
{
  switch ( mTickPosition )
  {
    case TicksUp:
      return QStringLiteral( "Line Ticks Up" );
    case TicksDown:
      return QStringLiteral( "Line Ticks Down" );
    case TicksMiddle:
      return QStringLiteral( "Line Ticks Middle" );
  }
  return QString();  // to make gcc happy
}

void QgsTicksScaleBarRenderer::draw( QgsRenderContext &context, const QgsScaleBarSettings &settings, const ScaleBarContext &scaleContext ) const
{
  if ( !context.painter() )
    return;

  QPainter *painter = context.painter();

  double barTopPosition = QgsComposerUtils::fontAscentMM( settings.font() ) + settings.labelBarSpace() + settings.boxContentSpace();
  double middlePosition = barTopPosition + settings.height() / 2.0;
  double bottomPosition = barTopPosition + settings.height();

  double xOffset = firstLabelXOffset( settings );

  painter->save();
  if ( context.flags() & QgsRenderContext::Antialiasing )
    painter->setRenderHint( QPainter::Antialiasing, true );

  painter->setPen( settings.pen() );

  QList<double> positions = segmentPositions( scaleContext, settings );

  for ( int i = 0; i < positions.size(); ++i )
  {
    painter->drawLine( QLineF( positions.at( i ) + xOffset, barTopPosition,
                               positions.at( i ) + xOffset, barTopPosition + settings.height() ) );
  }

  //draw last tick and horizontal line
  if ( !positions.isEmpty() )
  {
    double lastTickPositionX = positions.at( positions.size() - 1 ) + scaleContext.segmentWidth + xOffset;
    double verticalPos = 0.0;
    switch ( mTickPosition )
    {
      case TicksDown:
        verticalPos = barTopPosition;
        break;
      case TicksMiddle:
        verticalPos = middlePosition;
        break;
      case TicksUp:
        verticalPos = bottomPosition;
        break;
    }
    //horizontal line
    painter->drawLine( QLineF( xOffset + positions.at( 0 ), verticalPos, lastTickPositionX, verticalPos ) );
    //last vertical line
    painter->drawLine( QLineF( lastTickPositionX, barTopPosition, lastTickPositionX, barTopPosition + settings.height() ) );
  }

  painter->restore();

  //draw labels using the default method
  drawDefaultLabels( context, settings, scaleContext );
}


