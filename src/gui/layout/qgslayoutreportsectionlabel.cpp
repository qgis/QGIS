/***************************************************************************
                             qgslayoutreportsectionlabel.cpp
                             ------------------------
    begin                : January 2018
    copyright            : (C) 2018 by Nyall Dawson
    email                : nyall.dawson@gmail.com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgslayoutreportsectionlabel.h"
#include "qgslayout.h"
#include "qgslayoutview.h"
#include <QGraphicsView>
#include <QPainter>
#include <QWidget>
#include <QBrush>

///@cond PRIVATE

QgsLayoutReportSectionLabel::QgsLayoutReportSectionLabel( QgsLayout *layout, QgsLayoutView *view )
  : QGraphicsRectItem( nullptr )
  , mLayout( layout )
  , mView( view )
{
  setCacheMode( QGraphicsItem::DeviceCoordinateCache );
}

void QgsLayoutReportSectionLabel::paint( QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget )
{
  Q_UNUSED( option )
  Q_UNUSED( widget )

  if ( !mLayout || !mLayout->renderContext().isPreviewRender() )
  {
    //don't draw label in outputs
    return;
  }

  if ( mLabel.isEmpty() )
    return;

  QFont f;
  f.setPointSizeF( 8 );
  const QFontMetrics fm( f );
  const QSize s = fm.size( 0, mLabel );
  const double margin = fm.height() / 5.0;

  const double scaleValue = scale() / painter->transform().m11();
  const QgsScopedQPainterState painterState( painter );
  painter->setRenderHint( QPainter::Antialiasing, true );
  painter->scale( scaleValue, scaleValue );
  const QRectF r = rect();
  const QRectF scaledRect( r.left() / scaleValue, r.top() / scaleValue, r.width() / scaleValue, r.height() / scaleValue );

  if ( scaledRect.width() < s.width() + 2 * margin || scaledRect.height() < s.height() + 2 * margin )
  {
    // zoomed out too far to fully draw label inside item rect
    return;
  }

  const QRectF textRect = QRectF( scaledRect.left() + margin, scaledRect.top() + margin, scaledRect.width() - 2 * margin, scaledRect.height() - 2 * margin );
  const QRectF boxRect = QRectF( scaledRect.left(), scaledRect.bottom() - ( s.height() + 2 * margin ), s.width() + 2 * margin, s.height() + 2 * margin );

  QPainterPath p;
  p.moveTo( boxRect.bottomRight() );
  p.lineTo( boxRect.right(), boxRect.top() + margin );
  p.arcTo( boxRect.right() - 2 * margin, boxRect.top(), 2 * margin, 2 * margin, 0, 90 );
  p.lineTo( boxRect.left() + margin, boxRect.top() );
  p.arcTo( boxRect.left(), boxRect.top(), 2 * margin, 2 * margin, 90, 90 );
  p.lineTo( boxRect.bottomLeft() );
  p.lineTo( boxRect.bottomRight() );

  painter->setPen( QColor( 150, 150, 150, 150 ) );
  QLinearGradient g( 0, boxRect.top(), 0, boxRect.bottom() );
  g.setColorAt( 0, QColor( 200, 200, 200, 150 ) );
  g.setColorAt( 1, QColor( 150, 150, 150, 150 ) );

  painter->setBrush( QBrush( g ) );
  painter->drawPath( p );

  painter->setPen( QPen( QColor( 0, 0, 0, 100 ) ) );
  painter->setFont( f );
  painter->drawText( textRect, Qt::AlignBottom, mLabel );
}

void QgsLayoutReportSectionLabel::setLabel( const QString &label )
{
  mLabel = label;
  update();
}

///@endcond PRIVATE
