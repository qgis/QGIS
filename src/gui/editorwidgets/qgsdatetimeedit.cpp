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

  // enable calendar widget by default so it's already created
  setCalendarPopup( true );

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
  if ( mAllowNull )
  {
    displayNull();

    changed( QDateTime() );

    // avoid slot double activation
    disconnect( this, SIGNAL( dateTimeChanged() ), this, SLOT( changed() ) );
    emit dateTimeChanged( QDateTime() );
    connect( this, SIGNAL( dateTimeChanged() ), this, SLOT( changed() ) );
  }
}

void QgsDateTimeEdit::setEmpty()
{
  mClearButton->setVisible( mAllowNull );
  mIsEmpty = true;
}

void QgsDateTimeEdit::mousePressEvent( QMouseEvent *event )
{
  // catch mouse press on the button
  // in non-calendar mode: modifiy the date  so it leads to showing current date (don't bother about time)
  // in calendar mode: be sure NULL is displayed when needed and show page of current date in calendar widget

  bool updateCalendar = false;

  if ( mIsNull )
  {
    QStyleOptionSpinBox opt;
    this->initStyleOption( &opt );
    const QRect buttonUpRect = style()->subControlRect( QStyle::CC_SpinBox, &opt, QStyle::SC_SpinBoxUp );
    const QRect buttonDownRect = style()->subControlRect( QStyle::CC_SpinBox, &opt, QStyle::SC_SpinBoxDown );
    if ( buttonUpRect.contains( event->pos() ) || buttonDownRect.contains( event->pos() ) )
    {
      if ( calendarPopup() && calendarWidget() )
      {
        // ensure the line edit still displays NULL
        displayNull( true );
        updateCalendar = true;
      }
      else
      {
        blockSignals( true );
        resetBeforeChange( buttonUpRect.contains( event->pos() ) ? -1 : 1 );
        blockSignals( false );
      }
    }
  }

  QDateTimeEdit::mousePressEvent( event );

  if ( updateCalendar )
  {
    // set calendar page to current date to avoid going to minimal date page when value is null
    calendarWidget()->setCurrentPage( QDate::currentDate().year(), QDate::currentDate().month() );
  }
}

void QgsDateTimeEdit::focusOutEvent( QFocusEvent *event )
{
  if ( mAllowNull && mIsNull )
  {
    if ( lineEdit()->text() != QSettings().value( "qgis/nullValue", "NULL" ).toString() )
    {
      displayNull();
    }
    QWidget::focusOutEvent( event );
    emit editingFinished();
  }
  else
  {
    QDateTimeEdit::focusOutEvent( event );
  }
}

void QgsDateTimeEdit::wheelEvent( QWheelEvent *event )
{
  // dateTime might have been set to minimum in calendar mode
  if ( mAllowNull && mIsNull )
  {
    resetBeforeChange( -event->delta() );
  }
  QDateTimeEdit::wheelEvent( event );
}

void QgsDateTimeEdit::showEvent( QShowEvent *event )
{
  QDateTimeEdit::showEvent( event );
  if ( mAllowNull && mIsNull &&
       lineEdit()->text() != QSettings().value( "qgis/nullValue", "NULL" ).toString() )
  {
    displayNull();
  }
}

void QgsDateTimeEdit::changed( const QDateTime &dateTime )
{
  mIsEmpty = false;
  bool isNull = dateTime.isNull();
  if ( isNull != mIsNull )
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

void QgsDateTimeEdit::displayNull( bool updateCalendar )
{
  blockSignals( true );
  if ( updateCalendar )
  {
    // set current time to minimum date time to avoid having
    // a date selected in calendar widget
    QDateTimeEdit::setDateTime( minimumDateTime() );
  }
  lineEdit()->setText( QSettings().value( "qgis/nullValue", "NULL" ).toString() );
  blockSignals( false );
}

void QgsDateTimeEdit::resetBeforeChange( int delta )
{
  QDateTime dt = QDateTime::currentDateTime();
  switch ( currentSection() )
  {
    case QDateTimeEdit::DaySection:
      dt = dt.addDays( delta );
      break;
    case QDateTimeEdit::MonthSection:
      dt = dt.addMonths( delta );
      break;
    case QDateTimeEdit::YearSection:
      dt = dt.addYears( delta );
      break;
    default:
      break;
  }
  if ( dt < minimumDateTime() )
  {
    dt = minimumDateTime();
  }
  else if ( dt > maximumDateTime() )
  {
    dt = maximumDateTime();
  }
  QDateTimeEdit::setDateTime( dt );
}

int QgsDateTimeEdit::spinButtonWidth() const
{
  return calendarPopup() ? 25 : 18;
}

int QgsDateTimeEdit::frameWidth() const
{
  return style()->pixelMetric( QStyle::PM_DefaultFrameWidth );
}



void QgsDateTimeEdit::setDateTime( const QDateTime &dateTime )
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
