/***************************************************************************
  qgslayertreemodellegendfullwidthnode.cpp
  --------------------------------------
  Date                 : March 2017
  Copyright            : (C) 2017 by Stéphane Brunner
  Email                : stephane dot brunner at camptocamp dot com

 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgslayertreemodellegendfullwidthnode.h"

#include "qgslegendsettings.h"
#include "qgsrenderer.h"
#include "qgssymbollayerutils.h"
#include "qgsimageoperation.h"

#include <QPainter>
#include <QSize>
#include <QFont>


QgsFullWidthLabelLegendNode::QgsFullWidthLabelLegendNode( QgsLayerTreeLayer *nodeLayer, const QString &label )
  : QgsSimpleLegendNode( nodeLayer, label )
{
}

QgsFullWidthSymbolLegendNode::QgsFullWidthSymbolLegendNode( QgsLayerTreeLayer *nodeLayer, const QgsLegendSymbolItem &item, QObject *parent )
  : QgsSymbolLegendNode( nodeLayer, item, parent )
{
}

QgsSymbolLegendNode::SymbolMetrics QgsFullWidthSymbolLegendNode::symbolMetrics( const QgsLegendSettings &settings, const QgsRenderContext context, QgsSymbol *symbol ) const
{
  QFont font( settings.style( QgsLegendStyle::SymbolLabel ).font() );
  QScopedPointer<QgsRenderContext> temp_context( new QgsRenderContext );
  QScopedPointer<QPainter> temp_painter( new QPainter );
  QSize size( 4 * settings.dpi(), 4 * settings.dpi() );
  QPixmap pixmap( size );
  pixmap.fill( Qt::transparent );
  temp_painter->begin( &pixmap );
  QFont temp_font( font );
  temp_font.setPointSizeF( font.pointSizeF() * settings.dpi() / 96 );
  temp_painter->setFont( temp_font );
  temp_context->setScaleFactor( context.scaleFactor() );
  temp_context->setRendererScale( context.rendererScale() );
  temp_context->setMapToPixel( context.mapToPixel() );
  temp_context->setForceVectorOutput( true );
  temp_context->setPainter( temp_painter.data() );

  QgsSymbolLayerUtils::symbolPreviewPixmap( symbol, size, 0, temp_context.data() );

  const QSize legend_size = QgsImageOperation::nonTransparentImageRect(
                              pixmap.toImage(), QSize(), true ).size();

  temp_painter->end();

  SymbolMetrics result;
  result.size = QSizeF( legend_size.width() / context.scaleFactor(), legend_size.height() / context.scaleFactor() );
  result.offset = QSizeF( 0, 0 );
  return result;
}
