/***************************************************************************
  qgsdropfeedbackoverlay.cpp
  --------------------------------------
  Date                 : July 2026
  Copyright            : (C) 2026 by Denis Rouzaud
  Email                : denis@opengis.ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsdropfeedbackoverlay.h"

#include "qgsapplication.h"
#include "qgssvgcache.h"

#include <QPainter>
#include <QString>

#include "moc_qgsdropfeedbackoverlay.cpp"

using namespace Qt::StringLiterals;

QgsDropFeedbackOverlay::QgsDropFeedbackOverlay( QWidget *parent )
  : QWidget( parent )
{
  // never interfere with the drag and drop event delivery of the covered widget
  setAttribute( Qt::WA_TransparentForMouseEvents );
  setAttribute( Qt::WA_NoSystemBackground );
  setAttribute( Qt::WA_TranslucentBackground );
  setFocusPolicy( Qt::NoFocus );
}

void QgsDropFeedbackOverlay::setPayloadType( Qgis::LayerDropPayloadType type )
{
  if ( mPayloadType == type )
    return;

  mPayloadType = type;
  update();
}

void QgsDropFeedbackOverlay::paintEvent( QPaintEvent * )
{
  QPainter painter( this );
  paintFeedback( &painter, rect(), mPayloadType, this );
}

void QgsDropFeedbackOverlay::paintFeedback( QPainter *painter, const QRect &rect, Qgis::LayerDropPayloadType payloadType, const QWidget *widget )
{
  const int iconSize = widget->fontMetrics().height() * 3;

  QString message;
  QPixmap pixmap;
  switch ( payloadType )
  {
    case Qgis::LayerDropPayloadType::Layers:
    case Qgis::LayerDropPayloadType::CustomHandler:
      return;

    case Qgis::LayerDropPayloadType::Project:
      message = tr( "Load QGIS project\nThe current project will be replaced" );
      pixmap = QPixmap( QgsApplication::iconsPath() + u"qgis-icon-512x512.png"_s ).scaled( QSize( iconSize, iconSize ) * widget->devicePixelRatioF(), Qt::KeepAspectRatio, Qt::SmoothTransformation );
      pixmap.setDevicePixelRatio( widget->devicePixelRatioF() );
      break;

    case Qgis::LayerDropPayloadType::Invalid:
    {
      message = tr( "This file type cannot be loaded" );
      const double renderSize = iconSize * widget->devicePixelRatioF();
      bool fitsInCache = false;
      QImage image
        = QgsApplication::svgCache()->svgAsImage( QgsApplication::pkgDataPath() + u"/svg/backgrounds/background_forbidden.svg"_s, renderSize, QColor(), QColor( 180, 180, 180 ), renderSize / 16, 1.0, fitsInCache );
      image.setDevicePixelRatio( widget->devicePixelRatioF() );
      pixmap = QPixmap::fromImage( image );
      break;
    }
  }

  painter->setRenderHint( QPainter::Antialiasing );

  // partially hide the covered widget
  QColor washColor = widget->palette().color( QPalette::Window );
  washColor.setAlpha( 220 );
  painter->fillRect( rect, washColor );

  QFont messageFont = widget->font();
  messageFont.setBold( true );
  painter->setFont( messageFont );

  // center the icon and the message as one block, with a one line high gap in between
  const int spacing = widget->fontMetrics().height();
  const QRect availableRect = rect.adjusted( iconSize / 2, 0, -iconSize / 2, 0 );
  const QRect textRect = painter->boundingRect( availableRect, Qt::AlignHCenter | Qt::TextWordWrap, message );
  const int blockTop = rect.top() + ( rect.height() - ( iconSize + spacing + textRect.height() ) ) / 2;

  painter->drawPixmap( QPointF( rect.left() + ( rect.width() - iconSize ) / 2.0, blockTop ), pixmap );

  painter->setPen( widget->palette().color( QPalette::WindowText ) );
  painter->drawText( QRect( availableRect.left(), blockTop + iconSize + spacing, availableRect.width(), textRect.height() ), Qt::AlignHCenter | Qt::TextWordWrap, message );
}
