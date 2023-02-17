/***************************************************************************
    qgsdatetimeeditwrapper.cpp
     --------------------------------------
    Date                 : 03.2014
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

#include "qgsdatetimeeditwrapper.h"
#include "qgsdatetimeeditfactory.h"
#include "qgsmessagelog.h"
#include "qgslogger.h"
#include "qgsdatetimeedit.h"
#include "qgsdatetimeeditconfig.h"
#include "qgsdatetimefieldformatter.h"
#include "qgsapplication.h"

#include <QDateTimeEdit>
#include <QDateEdit>
#include <QTimeEdit>
#include <QTextCharFormat>
#include <QCalendarWidget>

QgsDateTimeEditWrapper::QgsDateTimeEditWrapper( QgsVectorLayer *layer, int fieldIdx, QWidget *editor, QWidget *parent )
  : QgsEditorWidgetWrapper( layer, fieldIdx, editor, parent )

{
}

QWidget *QgsDateTimeEditWrapper::createWidget( QWidget *parent )
{
  QgsDateTimeEdit *widget = new QgsDateTimeEdit( parent );
  widget->setDateTime( QDateTime::currentDateTime() );
  return widget;
}

void QgsDateTimeEditWrapper::initWidget( QWidget *editor )
{
  QgsDateTimeEdit *qgsEditor = dynamic_cast<QgsDateTimeEdit *>( editor );
  if ( qgsEditor )
  {
    mQgsDateTimeEdit = qgsEditor;
  }
  // assign the Qt editor also if the QGIS editor has been previously assigned
  // this avoids testing each time which widget to use
  // the QGIS editor must be used for non-virtual methods (dateTime, setDateTime)
  QDateTimeEdit *qtEditor = dynamic_cast<QDateTimeEdit *>( editor );
  if ( qtEditor )
  {
    mQDateTimeEdit = qtEditor;
  }

  if ( !mQDateTimeEdit )
  {
    QgsDebugMsg( QStringLiteral( "Date/time edit widget could not be initialized because provided widget is not a QDateTimeEdit." ) );
    QgsMessageLog::logMessage( tr( "Date/time edit widget could not be initialized because provided widget is not a QDateTimeEdit." ), tr( "UI forms" ), Qgis::MessageLevel::Warning );
    return;
  }

  const QString displayFormat = config( QStringLiteral( "display_format" ), QgsDateTimeFieldFormatter::defaultFormat( field().type() ) ).toString();
  mQDateTimeEdit->setDisplayFormat( displayFormat );

  const bool calendar = config( QStringLiteral( "calendar_popup" ), true ).toBool();
  if ( calendar != mQDateTimeEdit->calendarPopup() )
  {
    mQDateTimeEdit->setCalendarPopup( calendar );
  }
  if ( calendar && mQDateTimeEdit->calendarWidget() )
  {
    // highlight today's date
    QTextCharFormat todayFormat;
    todayFormat.setBackground( QColor( 160, 180, 200 ) );
    mQDateTimeEdit->calendarWidget()->setDateTextFormat( QDate::currentDate(), todayFormat );
  }

  const bool allowNull = config( QStringLiteral( "allow_null" ), true ).toBool();
  if ( mQgsDateTimeEdit )
  {
    mQgsDateTimeEdit->setAllowNull( allowNull );
  }
  else
  {
    QgsMessageLog::logMessage( tr( "The usual date/time widget QDateTimeEdit cannot be configured to allow NULL values. "
                                   "For that the QGIS custom widget QgsDateTimeEdit needs to be used." ),
                               tr( "field widgets" ) );
  }

  if ( mQgsDateTimeEdit )
  {
    connect( mQgsDateTimeEdit, &QgsDateTimeEdit::valueChanged, this, &QgsDateTimeEditWrapper::dateTimeChanged );
  }
  else
  {
    connect( mQDateTimeEdit, &QDateTimeEdit::dateTimeChanged, this,  &QgsDateTimeEditWrapper::dateTimeChanged );
  }
}

bool QgsDateTimeEditWrapper::valid() const
{
  return mQgsDateTimeEdit || mQDateTimeEdit;
}

void QgsDateTimeEditWrapper::showIndeterminateState()
{
  if ( mQgsDateTimeEdit )
    mQgsDateTimeEdit->setEmpty();
}

void QgsDateTimeEditWrapper::dateTimeChanged( const QDateTime &dateTime )
{
  switch ( field().type() )
  {
    case QVariant::DateTime:
      Q_NOWARN_DEPRECATED_PUSH
      emit valueChanged( dateTime );
      Q_NOWARN_DEPRECATED_POP
      emit valuesChanged( dateTime );
      break;
    case QVariant::Date:
      Q_NOWARN_DEPRECATED_PUSH
      emit valueChanged( dateTime.date() );
      Q_NOWARN_DEPRECATED_POP
      emit valuesChanged( dateTime.date() );
      break;
    case QVariant::Time:
      Q_NOWARN_DEPRECATED_PUSH
      emit valueChanged( dateTime.time() );
      Q_NOWARN_DEPRECATED_POP
      emit valuesChanged( dateTime.time() );
      break;
    default:
      if ( !dateTime.isValid() || dateTime.isNull() )
      {
        Q_NOWARN_DEPRECATED_PUSH
        emit valueChanged( QVariant( field().type() ) );
        Q_NOWARN_DEPRECATED_POP
        emit valuesChanged( QVariant( field().type() ) );
      }
      else
      {
        const bool fieldIsoFormat = config( QStringLiteral( "field_iso_format" ), false ).toBool();
        const QString fieldFormat = config( QStringLiteral( "field_format" ), QgsDateTimeFieldFormatter::defaultFormat( field().type() ) ).toString();
        if ( fieldIsoFormat )
        {
          Q_NOWARN_DEPRECATED_PUSH
          emit valueChanged( dateTime.toString( Qt::ISODate ) );
          Q_NOWARN_DEPRECATED_POP
          emit valuesChanged( dateTime.toString( Qt::ISODate ) );
        }
        else
        {
          Q_NOWARN_DEPRECATED_PUSH
          emit valueChanged( dateTime.toString( fieldFormat ) );
          Q_NOWARN_DEPRECATED_POP
          emit valuesChanged( dateTime.toString( fieldFormat ) );
        }
      }
      break;
  }
}

QVariant QgsDateTimeEditWrapper::value() const
{
  if ( !mQDateTimeEdit )
    return QVariant( field().type() );

  QDateTime dateTime;
  if ( mQgsDateTimeEdit )
  {
    dateTime = mQgsDateTimeEdit->dateTime();
  }
  else
  {
    dateTime = mQDateTimeEdit->dateTime();
  }

  if ( dateTime.isNull() )
    return QVariant( field().type() );

  switch ( field().type() )
  {
    case QVariant::DateTime:
      return dateTime;
    case QVariant::Date:
      return dateTime.date();
    case QVariant::Time:
      return dateTime.time();
    default:
      const bool fieldIsoFormat = config( QStringLiteral( "field_iso_format" ), false ).toBool();
      const QString fieldFormat = config( QStringLiteral( "field_format" ), QgsDateTimeFieldFormatter::defaultFormat( field().type() ) ).toString();
      if ( fieldIsoFormat )
      {
        return dateTime.toString( Qt::ISODate );
      }
      else
      {
        return dateTime.toString( fieldFormat );
      }
  }
#ifndef _MSC_VER // avoid warnings
  return QVariant(); // avoid warnings
#endif
}

void QgsDateTimeEditWrapper::updateValues( const QVariant &value, const QVariantList & )
{
  if ( !mQDateTimeEdit )
    return;

  QDateTime dateTime;

  switch ( field().type() )
  {
    case QVariant::DateTime:
      dateTime = value.toDateTime();
      break;
    case QVariant::Date:
      dateTime.setDate( value.toDate() );
      dateTime.setTime( QTime( 0, 0, 0 ) );
      break;
    case QVariant::Time:
      dateTime.setDate( QDate::currentDate() );
      dateTime.setTime( value.toTime() );
      break;
    default:
      // Field type is not a date/time but we might already have a date/time variant
      // value coming from a default: no need for string parsing in that case
      switch ( value.type() )
      {
        case QVariant::DateTime:
        {
          dateTime = value.toDateTime();
          break;
        }
        case QVariant::Date:
        {
          dateTime.setDate( value.toDate() );
          dateTime.setTime( QTime( 0, 0, 0 ) );
          break;
        }
        case QVariant::Time:
        {
          dateTime.setDate( QDate::currentDate() );
          dateTime.setTime( value.toTime() );
          break;
        }
        default:
        {
          const bool fieldIsoFormat = config( QStringLiteral( "field_iso_format" ), false ).toBool();
          const QString fieldFormat = config( QStringLiteral( "field_format" ), QgsDateTimeFieldFormatter::defaultFormat( field().type() ) ).toString();
          if ( fieldIsoFormat )
          {
            dateTime = QDateTime::fromString( value.toString(), Qt::ISODate );
          }
          else
          {
            dateTime = QDateTime::fromString( value.toString(), fieldFormat );
          }
          break;
        }
      }
      break;
  }

  if ( mQgsDateTimeEdit )
  {
    mQgsDateTimeEdit->setDateTime( dateTime );
  }
  else
  {
    mQDateTimeEdit->setDateTime( dateTime );
  }
}

void QgsDateTimeEditWrapper::setEnabled( bool enabled )
{
  if ( !mQDateTimeEdit )
    return;

  mQDateTimeEdit->setEnabled( enabled );
}
