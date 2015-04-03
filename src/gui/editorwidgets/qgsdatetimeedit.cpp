/***************************************************************************
    qgsdatetimeedit.cpp
     --------------------------------------
    Date                 : 08.2014
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

#include "qgsdatetimeedit.h"

#include "qgsapplication.h"
#include "qgslogger.h"

QgsDateTimeEdit::QgsDateTimeEdit( QWidget *parent )
    : QDateTimeEdit( parent )
    , mAllowNull( true )
    , mIsNull( true )
{
  mClearButton = new QToolButton( this );
  mClearButton->setIcon( QgsApplication::getThemeIcon( "/mIconClear.svg" ) );
  mClearButton->setCursor( Qt::ArrowCursor );
  mClearButton->setStyleSheet( "position: absolute; border: none; padding: 0px;" );
  mClearButton->hide();
  connect( mClearButton, SIGNAL( clicked() ), this, SLOT( clear() ) );

  mNullLabel = new QLineEdit( QSettings().value( "qgis/nullValue", "NULL" ).toString(), this );
  mNullLabel->setReadOnly( true );
  mNullLabel->setStyleSheet( "position: absolute; border: none; font-style: italic; color: grey;" );
  mNullLabel->hide();

  setStyleSheet( QString( "padding-right: %1px;" ).arg( mClearButton->sizeHint().width() + spinButtonWidth() + frameWidth() + 1 ) );

  QSize msz = minimumSizeHint();
  setMinimumSize( qMax( msz.width(), mClearButton->sizeHint().height() + frameWidth() * 2 + 2 ),
                  qMax( msz.height(), mClearButton->sizeHint().height() + frameWidth() * 2 + 2 ) );

  connect( this, SIGNAL( dateTimeChanged( QDateTime ) ), this, SLOT( changed( QDateTime ) ) );

  // init with current time so mIsNull is properly initialized
  QDateTimeEdit::setDateTime( QDateTime::currentDateTime() );
}

void QgsDateTimeEdit::setAllowNull( bool allowNull )
{
  mAllowNull = allowNull;

  mNullLabel->setVisible( mAllowNull && mIsNull );
  mClearButton->setVisible( mAllowNull && !mIsNull );
  lineEdit()->setVisible( !mAllowNull || !mIsNull );
}


void QgsDateTimeEdit::clear()
{
  changed( QDateTime() );
  emit dateTimeChanged( QDateTime() );
}

void QgsDateTimeEdit::mousePressEvent( QMouseEvent* event )
{
  QRect lerect = rect().adjusted( 0, 0, -spinButtonWidth(), 0 );
  if ( mAllowNull && mIsNull && lerect.contains( event->pos() ) )
    return;

  QDateTimeEdit::mousePressEvent( event );
}

void QgsDateTimeEdit::changed( const QDateTime & dateTime )
{
  mIsNull = dateTime.isNull();
  mNullLabel->setVisible( mAllowNull && mIsNull );
  mClearButton->setVisible( mAllowNull && !mIsNull );
  lineEdit()->setVisible( !mAllowNull || !mIsNull );
}

int QgsDateTimeEdit::spinButtonWidth() const
{
  return calendarPopup() ? 25 : 18;
}

int QgsDateTimeEdit::frameWidth() const
{
  return style()->pixelMetric( QStyle::PM_DefaultFrameWidth );
}

void QgsDateTimeEdit::setDateTime( const QDateTime& dateTime )
{
  // set an undefined date
  if ( !dateTime.isValid() || dateTime.isNull() )
  {
    clear();
  }
  else
  {
    QDateTimeEdit::setDateTime( dateTime );
  }
}

QDateTime QgsDateTimeEdit::dateTime() const
{
  if ( mAllowNull && mIsNull )
  {
    return QDateTime();
  }
  else
  {
    return QDateTimeEdit::dateTime();
  }
}

void QgsDateTimeEdit::resizeEvent( QResizeEvent * event )
{
  QDateTimeEdit::resizeEvent( event );

  QSize sz = mClearButton->sizeHint();


  mClearButton->move( rect().right() - frameWidth() - spinButtonWidth() - sz.width(),
                      ( rect().bottom() + 1 - sz.height() ) / 2 );

  mNullLabel->move( 0, 0 );
  mNullLabel->setMinimumSize( rect().adjusted( 0, 0, -spinButtonWidth(), 0 ).size() );
  mNullLabel->setMaximumSize( rect().adjusted( 0, 0, -spinButtonWidth(), 0 ).size() );
}




