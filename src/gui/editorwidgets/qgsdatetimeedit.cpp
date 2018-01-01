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

  // set this by default to properly connect the calendar widget
  setCalendarPopup( true );
  // when clearing the widget, date of the QDateTimeEdit will be set to minimum date
  // hence when the calendar popups, on selection changed if it set to the minimum date,
  // the page of the current date will be shown
  connect( calendarWidget(), &QCalendarWidget::selectionChanged, this, &QgsDateTimeEdit::calendarSelectionChanged );

  // init with current time so mIsNull is properly initialized
  QDateTimeEdit::setDateTime( QDateTime::currentDateTime() );

  setMinimumEditDateTime();
}

void QgsDateTimeEdit::setAllowNull( bool allowNull )
{
  mAllowNull = allowNull;
  mClearAction->setVisible( mAllowNull && ( !mIsNull || mIsEmpty ) );
}


void QgsDateTimeEdit::clear()
{
  QDateTimeEdit::blockSignals( true );
  setSpecialValueText( QgsApplication::nullRepresentation() );
  QDateTimeEdit::setDateTime( minimumDateTime() );
  QDateTimeEdit::blockSignals( false );
  changed( QDateTime() );
  emit dateTimeChanged( QDateTime() );
}

void QgsDateTimeEdit::setEmpty()
{
  mClearAction->setVisible( mAllowNull );
  mIsEmpty = true;
}

void QgsDateTimeEdit::mousePressEvent( QMouseEvent *event )
{
  // catch mouse press on the button when in non-calendar mode to modifiy the date
  // so it leads to showing current date (don't bother about time)

  if ( mIsNull && !calendarPopup() )
  {
    QStyleOptionSpinBox opt;
    this->initStyleOption( &opt );
    const QRect buttonUpRect = style()->subControlRect( QStyle::CC_SpinBox, &opt, QStyle::SC_SpinBoxUp );
    const QRect buttonDownRect = style()->subControlRect( QStyle::CC_SpinBox, &opt, QStyle::SC_SpinBoxDown );
    if ( buttonUpRect.contains( event->pos() ) || buttonDownRect.contains( event->pos() ) )
    {
      int before = 1;
      if ( buttonUpRect.contains( event->pos() ) )
      {
        before = -1;
      }

      blockSignals( true );
      switch ( currentSection() )
      {
        case QDateTimeEdit::DaySection:
          QDateTimeEdit::setDateTime( QDateTime::currentDateTime().addDays( before ) );
          break;
        case QDateTimeEdit::MonthSection:
          QDateTimeEdit::setDateTime( QDateTime::currentDateTime().addMonths( before ) );
          break;
        case QDateTimeEdit::YearSection:
          QDateTimeEdit::setDateTime( QDateTime::currentDateTime().addYears( before ) );
          break;
        default:
          QDateTimeEdit::setDateTime( QDateTime::currentDateTime() );
          break;
      }
      blockSignals( false );
    }
  }

  QDateTimeEdit::mousePressEvent( event );
}

void QgsDateTimeEdit::changed( const QDateTime &dateTime )
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
      lineEdit()->setStyleSheet( QStringLiteral( "font-style: italic; color: grey; }" ) );
    }
    else
    {
      lineEdit()->setStyleSheet( mOriginalStyleSheet );
    }
  }
  mClearAction->setVisible( mAllowNull && !mIsNull );
}

void QgsDateTimeEdit::calendarSelectionChanged()
{
  // set calendar page to current date to avoid going to minimal date page when value is null
  if ( mAllowNull && calendarWidget() && calendarWidget()->selectedDate() == minimumDate() )
  {
    calendarWidget()->setCurrentPage( QDate::currentDate().year(), QDate::currentDate().month() );
  }
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
