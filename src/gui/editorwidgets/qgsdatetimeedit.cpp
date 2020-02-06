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

#include <QAction>
#include <QCalendarWidget>
#include <QLineEdit>
#include <QMouseEvent>
#include <QStyle>
#include <QStyleOptionSpinBox>


#include "qgsdatetimeedit.h"

#include "qgsapplication.h"
#include "qgslogger.h"



QgsDateTimeEdit::QgsDateTimeEdit( QWidget *parent )
  : QDateTimeEdit( parent )
{
  QIcon clearIcon = QgsApplication::getThemeIcon( "/mIconClearText.svg" );
  mClearAction = new QAction( clearIcon, tr( "clear" ), this );
  mClearAction->setCheckable( false );
  lineEdit()->addAction( mClearAction, QLineEdit::TrailingPosition );
  mClearAction->setVisible( mAllowNull );
  connect( mClearAction, &QAction::triggered, this, &QgsDateTimeEdit::clear );

  connect( this, &QDateTimeEdit::dateTimeChanged, this, &QgsDateTimeEdit::changed );

  // enable calendar widget by default so it's already created
  setCalendarPopup( true );

  setMinimumEditDateTime();

  // init with current time so mIsNull is properly initialized
  QDateTimeEdit::setDateTime( QDateTime::currentDateTime() );
}

void QgsDateTimeEdit::setAllowNull( bool allowNull )
{
  mAllowNull = allowNull;
  mClearAction->setVisible( mAllowNull && ( !mIsNull || mIsEmpty ) );
}


void QgsDateTimeEdit::clear()
{
  if ( mAllowNull )
  {
    displayCurrentDate();

    // Check if it's really changed or crash, see GH #29937
    if ( ! dateTime().isNull() )
    {
      changed( QDateTime() );
    }

    // emit signal of QDateTime::dateTimeChanged with an invalid date
    // anyway, using parent's signal should be avoided
    // If you consequently connect parent's dateTimeChanged signal
    // and call dateTime() afterwards there is no warranty to
    // have a proper NULL value handling
    disconnect( this, &QDateTimeEdit::dateTimeChanged, this, &QgsDateTimeEdit::changed );
    emit dateTimeChanged( QDateTime() );
    connect( this, &QDateTimeEdit::dateTimeChanged, this, &QgsDateTimeEdit::changed );
  }
}

void QgsDateTimeEdit::setEmpty()
{
  mClearAction->setVisible( mAllowNull );
  mIsEmpty = true;
}

void QgsDateTimeEdit::mousePressEvent( QMouseEvent *event )
{
  // catch mouse press on the button (when the current value is null)
  // in non-calendar mode: modify the date  so it leads to showing current date (don't bother about time)
  // in calendar mode: be sure NULL is displayed when needed and show page of current date in calendar widget

  bool updateCalendar = false;

  if ( mIsNull )
  {
    QStyle::SubControl control;
    if ( calendarPopup() )
    {
      QStyleOptionComboBox optCombo;
      optCombo.init( this );
      optCombo.editable = true;
      optCombo.subControls = QStyle::SC_All;
      control = style()->hitTestComplexControl( QStyle::CC_ComboBox, &optCombo, event->pos(), this );

      if ( control == QStyle::SC_ComboBoxArrow && calendarWidget() )
      {
        mCurrentPressEvent = true;
        // ensure the line edit still displays NULL
        updateCalendar = true;
        displayNull( updateCalendar );
        mCurrentPressEvent = false;
      }
    }
    else
    {
      QStyleOptionSpinBox opt;
      this->initStyleOption( &opt );
      control  = style()->hitTestComplexControl( QStyle::CC_SpinBox, &opt, event->pos(), this );

      if ( control == QStyle::SC_SpinBoxDown || control == QStyle::SC_SpinBoxUp )
      {
        mCurrentPressEvent = true;
        disconnect( this, &QDateTimeEdit::dateTimeChanged, this, &QgsDateTimeEdit::changed );
        resetBeforeChange( control == QStyle::SC_SpinBoxDown ? -1 : 1 );
        connect( this, &QDateTimeEdit::dateTimeChanged, this, &QgsDateTimeEdit::changed );
        mCurrentPressEvent = false;
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
  if ( mAllowNull && mIsNull && !mCurrentPressEvent )
  {
    QAbstractSpinBox::focusOutEvent( event );
    if ( lineEdit()->text() != QgsApplication::nullRepresentation() )
    {
      displayNull();
    }
    emit editingFinished();
  }
  else
  {
    QDateTimeEdit::focusOutEvent( event );
  }
}

void QgsDateTimeEdit::focusInEvent( QFocusEvent *event )
{
  if ( mAllowNull && mIsNull && !mCurrentPressEvent )
  {
    QAbstractSpinBox::focusInEvent( event );

    displayCurrentDate();
  }
  else
  {
    QDateTimeEdit::focusInEvent( event );
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
       lineEdit()->text() != QgsApplication::nullRepresentation() )
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
      lineEdit()->setStyleSheet( QStringLiteral( "QLineEdit { font-style: italic; color: grey; }" ) );
    }
    else
    {
      lineEdit()->setStyleSheet( mOriginalStyleSheet );
    }
  }

  mClearAction->setVisible( mAllowNull && !mIsNull );

  emit QgsDateTimeEdit::valueChanged( dateTime );
}

void QgsDateTimeEdit::displayNull( bool updateCalendar )
{
  disconnect( this, &QDateTimeEdit::dateTimeChanged, this, &QgsDateTimeEdit::changed );
  if ( updateCalendar )
  {
    // set current time to minimum date time to avoid having
    // a date selected in calendar widget
    QDateTimeEdit::setDateTime( minimumDateTime() );
  }
  lineEdit()->setCursorPosition( lineEdit()->text().length() );
  lineEdit()->setText( QgsApplication::nullRepresentation() );
  connect( this, &QDateTimeEdit::dateTimeChanged, this, &QgsDateTimeEdit::changed );
}

void QgsDateTimeEdit::displayCurrentDate()
{
  disconnect( this, &QDateTimeEdit::dateTimeChanged, this, &QgsDateTimeEdit::changed );
  QDateTimeEdit::setDateTime( QDateTime::currentDateTime() );
  connect( this, &QDateTimeEdit::dateTimeChanged, this, &QgsDateTimeEdit::changed );
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

void QgsDateTimeEdit::setDateTime( const QDateTime &dateTime )
{
  mIsEmpty = false;

  // set an undefined date
  if ( !dateTime.isValid() || dateTime.isNull() )
  {
    clear();
    displayNull();
  }
  // Check if it's really changed or crash, see GH #29937
  else if ( dateTime != QgsDateTimeEdit::dateTime() )
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

QTime QgsDateTimeEdit::time() const
{
  if ( mAllowNull && mIsNull )
  {
    return QTime();
  }
  else
  {
    return QDateTimeEdit::time();
  }
}

QDate QgsDateTimeEdit::date() const
{
  if ( mAllowNull && mIsNull )
  {
    return QDate();
  }
  else
  {
    return QDateTimeEdit::date();
  }
}
