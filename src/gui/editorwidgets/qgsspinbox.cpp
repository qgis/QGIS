/***************************************************************************
    qgsspinbox.cpp
     --------------------------------------
    Date                 : 09.2014
    Copyright            : (C) 2014 Denis Rouzaud
    Email                : denis.rouzaud@gmail.com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <QLineEdit>
#include <QMouseEvent>
#include <QSettings>
#include <QStyle>
#include <QToolButton>

#include "qgsspinbox.h"

#include "qgsapplication.h"
#include "qgslogger.h"

QgsSpinBox::QgsSpinBox( QWidget *parent )
    : QSpinBox( parent )
    , mShowClearButton( true )
{
  mClearButton = new QToolButton( this );
  mClearButton->setIcon( QgsApplication::getThemeIcon( "/mIconClear.svg" ) );
  mClearButton->setCursor( Qt::ArrowCursor );
  mClearButton->setStyleSheet( "position: absolute; border: none; padding: 0px;" );
  connect( mClearButton, SIGNAL( clicked() ), this, SLOT( clear() ) );

  setStyleSheet( QString( "padding-right: %1px;" ).arg( mClearButton->sizeHint().width() + 18 + frameWidth() + 1 ) );

  QSize msz = minimumSizeHint();
  setMinimumSize( qMax( msz.width(), mClearButton->sizeHint().height() + frameWidth() * 2 + 2 ),
                  qMax( msz.height(), mClearButton->sizeHint().height() + frameWidth() * 2 + 2 ) );

  connect( this, SIGNAL( valueChanged( int ) ), this, SLOT( changed( int ) ) );
}

void QgsSpinBox::setShowClearButton( const bool showClearButton )
{
  mShowClearButton = showClearButton;
  mClearButton->setVisible( mShowClearButton && isEnabled() && value() != minimum() );
}

void QgsSpinBox::changeEvent( QEvent *event )
{
  QSpinBox::changeEvent( event );
  mClearButton->setVisible( mShowClearButton && isEnabled() && value() != minimum() );
}

void QgsSpinBox::changed( const int& value )
{
  mClearButton->setVisible( mShowClearButton && isEnabled() && value != minimum() );
}

void QgsSpinBox::clear()
{
  setValue( minimum() );
}

int QgsSpinBox::frameWidth() const
{
  return style()->pixelMetric( QStyle::PM_DefaultFrameWidth );
}

void QgsSpinBox::resizeEvent( QResizeEvent * event )
{
  QSpinBox::resizeEvent( event );

  QSize sz = mClearButton->sizeHint();

  mClearButton->move( rect().right() - frameWidth() - 18 - sz.width(),
                      ( rect().bottom() + 1 - sz.height() ) / 2 );

}
