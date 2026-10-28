/***************************************************************************
    qgsdatetimesearchwidgetwrapper.cpp
     ---------------------------------
    Date                 : 2016-05-23
    Copyright            : (C) 2016 Nyall Dawson
    Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsdatetimesearchwidgetwrapper.h"

#include "qgsdatetimeedit.h"
#include "qgsdatetimeeditconfig.h"
#include "qgsdatetimeeditfactory.h"
#include "qgsdatetimefieldformatter.h"
#include "qgsfields.h"
#include "qgsvectorlayer.h"

#include <QSettings>
#include <QString>
#include <qcalendarwidget.h>

#include "moc_qgsdatetimesearchwidgetwrapper.cpp"

using namespace Qt::StringLiterals;

QgsDateTimeSearchWidgetWrapper::QgsDateTimeSearchWidgetWrapper( QgsVectorLayer *vl, int fieldIdx, QWidget *parent )
  : QgsSearchWidgetWrapper( vl, fieldIdx, parent )

{}

bool QgsDateTimeSearchWidgetWrapper::applyDirectly()
{
  return true;
}

QString QgsDateTimeSearchWidgetWrapper::expression() const
{
  return mExpression;
}

QVariant QgsDateTimeSearchWidgetWrapper::value() const
{
  if ( !mDateTimeEdit )
    return QDateTime();

  QMetaType::Type fieldType = QMetaType::QString;
  if ( QgsVectorLayer *vl = layer() )
  {
    fieldType = vl->fields().at( mFieldIdx ).type();
  }

  switch ( fieldType )
  {
    case QMetaType::Type::QDateTime:
      return mDateTimeEdit->dateTime();
    case QMetaType::Type::QDate:
      return mDateTimeEdit->date();
    case QMetaType::Type::QTime:
      return mDateTimeEdit->time();
    default:
      break;
  }

  // if not an explicit date/time field, treat as string values
  const bool fieldIsoFormat = config( u"field_iso_format"_s, false ).toBool();
  const QString fieldFormat = config( u"field_format"_s, QgsDateTimeFieldFormatter::defaultFormat( layer()->fields().at( mFieldIdx ).type() ) ).toString();
  if ( fieldIsoFormat )
  {
    return mDateTimeEdit->dateTime().toString( Qt::ISODate );
  }
  else
  {
    return mDateTimeEdit->dateTime().toString( fieldFormat );
  }
}

QgsSearchWidgetWrapper::FilterFlags QgsDateTimeSearchWidgetWrapper::supportedFlags() const
{
  return EqualTo | NotEqualTo | GreaterThan | LessThan | GreaterThanOrEqualTo | LessThanOrEqualTo | IsNull | Between | IsNotNull | IsNotBetween;
}

QgsSearchWidgetWrapper::FilterFlags QgsDateTimeSearchWidgetWrapper::defaultFlags() const
{
  return EqualTo;
}

QString QgsDateTimeSearchWidgetWrapper::createExpression( QgsSearchWidgetWrapper::FilterFlags flags ) const
{
  const QString fieldName = createFieldIdentifier();
  const QMetaType::Type fieldType = layer()->fields().at( mFieldIdx ).type();

  //clear any unsupported flags
  flags &= supportedFlags();
  if ( flags & IsNull )
    return fieldName + " IS NULL";
  if ( flags & IsNotNull )
    return fieldName + " IS NOT NULL";

  const QVariant v = value();
  if ( !v.isValid() )
    return QString();

  QString valuePart;
  switch ( fieldType )
  {
    case QMetaType::Type::QDateTime:
      valuePart = u"to_datetime('%1')"_s.arg( v.toDateTime().toString( Qt::ISODate ) );
      break;
    case QMetaType::Type::QDate:
      valuePart = u"to_date('%1')"_s.arg( v.toDate().toString( Qt::ISODate ) );
      break;
    case QMetaType::Type::QTime:
      valuePart = u"to_time('%1')"_s.arg( v.toTime().toString( Qt::ISODate ) );
      break;
    default:
      valuePart = QgsExpression::quotedString( v.toString() );
      break;
  }

  if ( flags & EqualTo )
    return fieldName + "=" + valuePart;
  else if ( flags & NotEqualTo )
    return fieldName + "<>" + valuePart;
  else if ( flags & GreaterThan )
    return fieldName + ">" + valuePart;
  else if ( flags & LessThan )
    return fieldName + "<" + valuePart;
  else if ( flags & GreaterThanOrEqualTo )
    return fieldName + ">=" + valuePart;
  else if ( flags & LessThanOrEqualTo )
    return fieldName + "<=" + valuePart;

  return QString();
}

void QgsDateTimeSearchWidgetWrapper::clearWidget()
{
  if ( mDateTimeEdit )
  {
    mDateTimeEdit->setEmpty();
  }
}

void QgsDateTimeSearchWidgetWrapper::setEnabled( bool enabled )
{
  if ( mDateTimeEdit )
  {
    mDateTimeEdit->setEnabled( enabled );
  }
}

bool QgsDateTimeSearchWidgetWrapper::valid() const
{
  return true;
}

void QgsDateTimeSearchWidgetWrapper::setExpression( const QString &expression )
{
  QString exp = expression;

  const QgsField field = layer()->fields().at( mFieldIdx );
  const QMetaType::Type fieldType = field.type();
  const QString fieldName = field.name();
  switch ( fieldType )
  {
    case QMetaType::Type::QDateTime:
      mExpression = u"%1 = to_datetime('%2')"_s.arg( QgsExpression::quotedColumnRef( fieldName ), expression );
      break;
    case QMetaType::Type::QDate:
      mExpression = u"%1 = to_date('%2')"_s.arg( QgsExpression::quotedColumnRef( fieldName ), expression );
      break;
    case QMetaType::Type::QTime:
      mExpression = u"%1 = to_time('%2')"_s.arg( QgsExpression::quotedColumnRef( fieldName ), expression );
      break;
    default:
      mExpression = QgsExpression::createFieldEqualityExpression( fieldName, expression );
      break;
  }
}

void QgsDateTimeSearchWidgetWrapper::dateTimeChanged( const QDateTime &dt )
{
  if ( mDateTimeEdit )
  {
    const QVariant v = value();
    QString valueAsString;
    switch ( v.userType() )
    {
      case QMetaType::Type::QDateTime:
        valueAsString = v.toDateTime().toString( Qt::ISODate );
        break;
      case QMetaType::Type::QDate:
        valueAsString = v.toDate().toString( Qt::ISODate );
        break;
      case QMetaType::Type::QTime:
        valueAsString = v.toTime().toString( Qt::ISODate );
        break;
      default:
        valueAsString = v.toString();
        break;
    }

    setExpression( valueAsString );
    if ( dt.isValid() && !dt.isNull() )
      emit valueChanged();
    else
      emit valueCleared();
    emit expressionChanged( mExpression );
  }
}

QWidget *QgsDateTimeSearchWidgetWrapper::createWidget( QWidget *parent )
{
  QgsDateTimeEdit *widget = new QgsDateTimeEdit( parent );
  widget->setEmpty();
  return widget;
}

void QgsDateTimeSearchWidgetWrapper::initWidget( QWidget *editor )
{
  mDateTimeEdit = qobject_cast<QgsDateTimeEdit *>( editor );

  if ( mDateTimeEdit )
  {
    mDateTimeEdit->setAllowNull( false );

    const QString displayFormat = config( u"display_format"_s, QgsDateTimeFieldFormatter::defaultFormat( layer()->fields().at( mFieldIdx ).type() ) ).toString();
    mDateTimeEdit->setDisplayFormat( displayFormat );

    const bool calendar = config( u"calendar_popup"_s, false ).toBool();
    mDateTimeEdit->setCalendarPopup( calendar );
    if ( calendar && mDateTimeEdit->calendarWidget() )
    {
      // highlight today's date
      QTextCharFormat todayFormat;
      todayFormat.setBackground( QColor( 160, 180, 200 ) );
      mDateTimeEdit->calendarWidget()->setDateTextFormat( QDate::currentDate(), todayFormat );
    }

    mDateTimeEdit->setEmpty();

    connect( mDateTimeEdit, &QDateTimeEdit::dateTimeChanged, this, &QgsDateTimeSearchWidgetWrapper::dateTimeChanged );
  }
}
