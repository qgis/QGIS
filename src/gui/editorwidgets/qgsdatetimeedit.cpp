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

#include <QCalendarWidget>
#include <QLineEdit>
#include <QMouseEvent>
#include <QSettings>
#include <QStyle>
#include <QStyleOptionSpinBox>
#include <QToolButton>

#include "qgsdatetimeedit.h"

#include "qgsapplication.h"
#include "qgslogger.h"

QgsDateTimeEdit::QgsDateTimeEdit( QWidget *parent )
    : QDateTimeEdit( parent )
    , mAllowNull( true )
    , mIsNull( true )
    , mIsEmpty( false )
{
  mClearButton = new QToolButton( this );
  mClearButton->setIcon( QgsApplication::getThemeIcon( "/mIconClear.svg" ) );
  mClearButton->setCursor( Qt::ArrowCursor );
  mClearButton->setStyleSheet( "position: absolute; border: none; padding: 0px;" );
  mClearButton->hide();
  connect( mClearButton, SIGNAL( clicked() ), this, SLOT( clear() ) );

  setStyleSheet( QString( ".QWidget, QLineEdit, QToolButton { padding-right: %1px; }" ).arg( mClearButton->sizeHint().width() + spinButtonWidth() + frameWidth() + 1 ) );

  QSize msz = minimumSizeHint();
  setMinimumSize( qMax( msz.width(), mClearButton->sizeHint().height() + frameWidth() * 2 + 2 ),
                  qMax( msz.height(), mClearButton->sizeHint().height() + frameWidth() * 2 + 2 ) );

  connect( this, SIGNAL( dateTimeChanged( QDateTime ) ), this, SLOT( changed( QDateTime ) ) );

  // set this by defaut to properly connect the calendar widget
  setCalendarPopup( true );
  // when clearing the widget, date of the QDateTimeEdit will be set to minimum date
  // hence when the calendar popups, on selection changed if it set to the minimum date,
  // the page of the current date will be shown
  connect( calendarWidget(), SIGNAL( selectionChanged() ), this, SLOT( calendarSelectionChanged() ) );

  // init with current time so mIsNull is properly initialized
  QDateTimeEdit::setDateTime( QDateTime::currentDateTime() );
}

void QgsDateTimeEdit::setAllowNull( bool allowNull )
{
  mAllowNull = allowNull;

  mClearButton->setVisible( mAllowNull && ( !mIsNull || mIsEmpty ) );
}


void QgsDateTimeEdit::clear()
{
  QDateTimeEdit::blockSignals( true );
  setSpecialValueText( QSettings().value( "qgis/nullValue", "NULL" ).toString() );
  QDateTimeEdit::setDateTime( minimumDateTime() );
  QDateTimeEdit::blockSignals( false );
  changed( QDateTime() );
  emit dateTimeChanged( QDateTime() );
}

void QgsDateTimeEdit::setEmpty()
{
  mClearButton->setVisible( mAllowNull );
  mIsEmpty = true;
}

void QgsDateTimeEdit::mousePressEvent( QMouseEvent* event )
{
  const QRect lerect = rect().adjusted( 0, 0, -spinButtonWidth(), 0 );
  if ( mAllowNull && mIsNull && lerect.contains( event->pos() ) )
    return;

  if ( mIsNull && !calendarPopup() )
  {
    QStyleOptionSpinBox opt;
    this->initStyleOption( &opt );
    const QRect buttonUpRect = style()->subControlRect( QStyle::CC_SpinBox, &opt, QStyle::SC_SpinBoxUp );
    const QRect buttonDownRect = style()->subControlRect( QStyle::CC_SpinBox, &opt, QStyle::SC_SpinBoxDown );
    if ( buttonUpRect.contains( event->pos() ) || buttonDownRect.contains( event->pos() ) )
    {
      blockSignals( true );
      QDateTimeEdit::setDateTime( QDateTime::currentDateTime() );
      blockSignals( false );
    }
  }

  QDateTimeEdit::mousePressEvent( event );
}

void QgsDateTimeEdit::changed( const QDateTime & dateTime )
{
  mIsEmpty = false;
  bool isNull = dateTime.isNull() || dateTime == minimumDateTime();
  if ( mIsNull != isNull )
  {
    mIsNull = isNull;
    if ( mIsNull )
    {
      if ( mOriginalStyleSheet.isNull() )
      {
        mOriginalStyleSheet = lineEdit()->styleSheet();
      }
      lineEdit()->setStyleSheet( "font-style: italic; color: grey; }" );
    }
    else
    {
      lineEdit()->setStyleSheet( mOriginalStyleSheet );
    }
  }
  mClearButton->setVisible( mAllowNull && !mIsNull );
}

void QgsDateTimeEdit::calendarSelectionChanged()
{
  if ( mAllowNull && calendarWidget() && calendarWidget()->selectedDate() == minimumDate() )
  {
    calendarWidget()->setCurrentPage( QDate::currentDate().year(), QDate::currentDate().month() );
  }
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
  mIsEmpty = false;

  // set an undefined date
  if ( !dateTime.isValid() || dateTime.isNull() )
  {
    clear();
  }
  else
  {
    QDateTimeEdit::setDateTime( dateTime );
    mIsNull = false;
    changed( dateTime );
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
}
