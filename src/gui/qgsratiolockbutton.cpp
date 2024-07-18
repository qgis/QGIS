/***************************************************************************
    qgsratiolockbutton.cpp - Lock button
     --------------------------------------
    Date                 : July, 2017
    Copyright            : (C) 2017 by Mathieu Pellerin
    Email                : nirvn dot asia at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsratiolockbutton.h"
#include "qgsapplication.h"
#include "qgssvgcache.h"
#include "qgis.h"

#include <QApplication>
#include <QMouseEvent>
#include <QPainter>
#include <QPushButton>
#include <QWidget>
#include <QDoubleSpinBox>

QgsRatioLockButton::QgsRatioLockButton( QWidget *parent )
  : QToolButton( parent )
{
  setMinimumSize( QSize( 24, 24 ) );
  setMaximumWidth( fontMetrics().horizontalAdvance( '0' ) * 3 );
  setCheckable( true );
  setAutoRaise( true );
  connect( this, &QPushButton::clicked, this, &QgsRatioLockButton::buttonClicked );
}

void QgsRatioLockButton::setLocked( const bool locked )
{
  if ( mLocked != locked )
    buttonClicked();
}

void QgsRatioLockButton::buttonClicked()
{
  mLocked = !mLocked;
  setChecked( mLocked );

  emit lockChanged( mLocked );

  drawButton();
}

void QgsRatioLockButton::widthSpinBoxChanged( double value )
{
  if ( mUpdatingRatio || qgsDoubleNear( value, 0.0 ) || qgsDoubleNear( mPrevWidth, 0.0 )
       || qgsDoubleNear( mPrevHeight, 0.0 ) || !mHeightSpinBox || !mLocked )
  {
    mPrevWidth = value;
    return;
  }

  const double oldRatio = mPrevHeight / mPrevWidth;
  mUpdatingRatio = true;
  mHeightSpinBox->setValue( oldRatio * value );
  mUpdatingRatio = false;
  mPrevWidth = value;
}

void QgsRatioLockButton::heightSpinBoxChanged( double value )
{
  if ( mUpdatingRatio || qgsDoubleNear( value, 0.0 ) || qgsDoubleNear( mPrevWidth, 0.0 )
       || qgsDoubleNear( mPrevHeight, 0.0 ) || !mWidthSpinBox || !mLocked )
  {
    mPrevHeight = value;
    return;
  }

  const double oldRatio = mPrevWidth / mPrevHeight;
  mUpdatingRatio = true;
  mWidthSpinBox->setValue( oldRatio * value );
  mUpdatingRatio = false;
  mPrevHeight = value;
}

void QgsRatioLockButton::changeEvent( QEvent *e )
{
  if ( e->type() == QEvent::EnabledChange )
  {
    drawButton();
  }
  QToolButton::changeEvent( e );
}

void QgsRatioLockButton::showEvent( QShowEvent *e )
{
  drawButton();
  QToolButton::showEvent( e );
}

void QgsRatioLockButton::resizeEvent( QResizeEvent *event )
{
  QToolButton::resizeEvent( event );
  drawButton();
}

void QgsRatioLockButton::drawButton()
{
  QSize currentIconSize;

#ifdef Q_OS_WIN
  currentIconSize = QSize( width() - 10, height() - 6 );
#else
  currentIconSize = QSize( width() - 10, height() - 12 );
#endif

  if ( !currentIconSize.isValid() || currentIconSize.width() <= 0 || currentIconSize.height() <= 0 )
  {
    return;
  }

  const double pixelRatio = devicePixelRatioF();
  QPixmap pm( currentIconSize * pixelRatio );
  pm.setDevicePixelRatio( pixelRatio );
  pm.fill( Qt::transparent );

  QPainter painter;
  QPen pen = QPen( QColor( 136, 136, 136 ) );
  pen.setWidth( 2 );

  painter.begin( &pm );
  painter.setRenderHint( QPainter::Antialiasing, true );
  painter.setPen( pen );

  painter.drawLine( QPointF( 1, 1 ), QPointF( currentIconSize.width() / 2, 1 ) );
  painter.drawLine( QPointF( currentIconSize.width() / 2, 1 ), QPointF( currentIconSize.width() / 2, currentIconSize.height() / 2 - 13 ) );
  painter.drawLine( QPointF( currentIconSize.width() / 2, currentIconSize.height() / 2 + 13 ), QPointF( currentIconSize.width() / 2, currentIconSize.height() - 2 ) );
  painter.drawLine( QPointF( currentIconSize.width() / 2, currentIconSize.height() - 2 ), QPointF( 1, currentIconSize.height() - 2 ) );

  const QString imageSource = mLocked ? QStringLiteral( ":/images/themes/default/lockedGray.svg" ) : QStringLiteral( ":/images/themes/default/unlockedGray.svg" );
  bool fitsInCache = false;
  QImage image = QgsApplication::svgCache()->svgAsImage(
                   imageSource, 16 * pixelRatio, QColor(), QColor(), 0, 1, fitsInCache
                 );
  image.setDevicePixelRatio( pixelRatio );
  painter.drawImage( QRectF(
                       currentIconSize.width() / 2 - 8,
                       currentIconSize.height() / 2 - 8,
                       16,
                       16 ),
                     image );

  painter.end();

  setIconSize( currentIconSize );
  setIcon( pm );
}

void QgsRatioLockButton::setWidthSpinBox( QDoubleSpinBox *widget )
{
  mWidthSpinBox = widget;
  mPrevWidth = widget->value();
  connect( mWidthSpinBox, static_cast<void ( QDoubleSpinBox::* )( double )>( &QDoubleSpinBox::valueChanged ), this, &QgsRatioLockButton::widthSpinBoxChanged );
}

void QgsRatioLockButton::setHeightSpinBox( QDoubleSpinBox *widget )
{
  mHeightSpinBox = widget;
  mPrevHeight = widget->value();
  connect( mHeightSpinBox, static_cast<void ( QDoubleSpinBox::* )( double )>( &QDoubleSpinBox::valueChanged ), this, &QgsRatioLockButton::heightSpinBoxChanged );
}

void QgsRatioLockButton::resetRatio()
{
  mPrevWidth = mWidthSpinBox ? mWidthSpinBox->value() : 0.0;
  mPrevHeight = mHeightSpinBox ? mHeightSpinBox->value() : 0.0;
}
