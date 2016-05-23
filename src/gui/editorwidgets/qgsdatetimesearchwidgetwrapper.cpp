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

#include "qgsfield.h"
#include "qgsdatetimeeditfactory.h"
#include "qgsvectorlayer.h"
#include "qgsdatetimeedit.h"
#include "qcalendarwidget.h"

#include <QSettings>

QgsDateTimeSearchWidgetWrapper::QgsDateTimeSearchWidgetWrapper( QgsVectorLayer* vl, int fieldIdx, QWidget* parent )
    : QgsSearchWidgetWrapper( vl, fieldIdx, parent )
    , mDateTimeEdit( nullptr )
    , mLayer( nullptr )
{
}

bool QgsDateTimeSearchWidgetWrapper::applyDirectly()
{
  return true;
}

QString QgsDateTimeSearchWidgetWrapper::expression()
{
  return mExpression;
}

QVariant QgsDateTimeSearchWidgetWrapper::value() const
{
  if ( ! mDateTimeEdit )
    return QDateTime();

  const QString fieldFormat = config( "field_format", QGSDATETIMEEDIT_DATEFORMAT ).toString();
  return mDateTimeEdit->dateTime().toString( fieldFormat );
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
  QString fieldName = QgsExpression::quotedColumnRef( layer()->fields().at( mFieldIdx ).name() );

  //clear any unsupported flags
  flags &= supportedFlags();
  if ( flags & IsNull )
    return fieldName + " IS NULL";
  if ( flags & IsNotNull )
    return fieldName + " IS NOT NULL";

  QVariant v = value();
  if ( !v.isValid() )
    return QString();

  if ( flags & EqualTo )
    return fieldName + "='" + v.toString() + '\'';
  else if ( flags & NotEqualTo )
    return fieldName + "<>'" + v.toString() + '\'';
  else if ( flags & GreaterThan )
    return fieldName + ">'" + v.toString() + '\'';
  else if ( flags & LessThan )
    return fieldName + "<'" + v.toString() + '\'';
  else if ( flags & GreaterThanOrEqualTo )
    return fieldName + ">='" + v.toString() + '\'';
  else if ( flags & LessThanOrEqualTo )
    return fieldName + "<='" + v.toString() + '\'';

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

void QgsDateTimeSearchWidgetWrapper::setExpression( QString exp )
{
  QString fieldName = layer()->fields().at( mFieldIdx ).name();

  QString str = QString( "%1 = '%3'" )
                .arg( QgsExpression::quotedColumnRef( fieldName ),
                      exp.replace( '\'', "''" )
                    );
  mExpression = str;
}

void QgsDateTimeSearchWidgetWrapper::dateTimeChanged( const QDateTime& dt )
{
  if ( mDateTimeEdit )
  {
    QString exp = value().toString();
    setExpression( exp );
    if ( dt.isValid() && !dt.isNull() )
      emit valueChanged();
    else
      emit valueCleared();
    emit expressionChanged( mExpression );
  }
}

QWidget* QgsDateTimeSearchWidgetWrapper::createWidget( QWidget* parent )
{
  QgsDateTimeEdit* widget = new QgsDateTimeEdit( parent );
  widget->setEmpty();
  return widget;
}

void QgsDateTimeSearchWidgetWrapper::initWidget( QWidget* editor )
{
  mDateTimeEdit = qobject_cast<QgsDateTimeEdit*>( editor );

  if ( mDateTimeEdit )
  {
    mDateTimeEdit->setAllowNull( false );

    const QString displayFormat = config( "display_format", QGSDATETIMEEDIT_DATEFORMAT ).toString();
    mDateTimeEdit->setDisplayFormat( displayFormat );

    const bool calendar = config( "calendar_popup", false ).toBool();
    mDateTimeEdit->setCalendarPopup( calendar );
    if ( calendar )
    {
      // highlight today's date
      QTextCharFormat todayFormat;
      todayFormat.setBackground( QColor( 160, 180, 200 ) );
      mDateTimeEdit->calendarWidget()->setDateTextFormat( QDate::currentDate(), todayFormat );
    }

    mDateTimeEdit->setEmpty();

    connect( mDateTimeEdit, SIGNAL( dateTimeChanged( QDateTime ) ), this, SLOT( dateTimeChanged( QDateTime ) ) );
  }
}


