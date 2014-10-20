/***************************************************************************
    qgsdoublespinbox.cpp
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

#include "qgsdoublespinbox.h"

#include "qgsapplication.h"
#include "qgslogger.h"

QgsDoubleSpinBox::QgsDoubleSpinBox( QWidget *parent )
    : QDoubleSpinBox( parent )
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

  connect( this, SIGNAL( valueChanged( double ) ), this, SLOT( changed( double ) ) );
}

void QgsDoubleSpinBox::setShowClearButton( const bool showClearButton )
{
  mShowClearButton = showClearButton;
  mClearButton->setVisible( mShowClearButton && isEnabled() && value() != minimum() );
}

void QgsDoubleSpinBox::changeEvent( QEvent *event )
{
  QDoubleSpinBox::changeEvent( event );
  mClearButton->setVisible( mShowClearButton && isEnabled() && value() != minimum() );
}

void QgsDoubleSpinBox::changed( const double& value )
{
  mClearButton->setVisible( mShowClearButton && isEnabled() && value != minimum() );
}

void QgsDoubleSpinBox::clear()
{
  setValue( minimum() );
}

int QgsDoubleSpinBox::frameWidth() const
{
  return style()->pixelMetric( QStyle::PM_DefaultFrameWidth );
}

void QgsDoubleSpinBox::resizeEvent( QResizeEvent * event )
{
  QDoubleSpinBox::resizeEvent( event );

  QSize sz = mClearButton->sizeHint();

  mClearButton->move( rect().right() - frameWidth() - 18 - sz.width(),
                      ( rect().bottom() + 1 - sz.height() ) / 2 );

}
