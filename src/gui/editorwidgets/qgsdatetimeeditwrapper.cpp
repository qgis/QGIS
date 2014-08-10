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


#include <QDateTimeEdit>
#include <QDateEdit>
#include <QTimeEdit>

#include "qgsdatetimeeditwrapper.h"
#include "qgsdatetimeeditfactory.h"
#include "qgsmessagelog.h"
#include "qgslogger.h"


QgsDateTimeEditWrapper::QgsDateTimeEditWrapper( QgsVectorLayer* vl, int fieldIdx, QWidget* editor, QWidget* parent )
    : QgsEditorWidgetWrapper( vl, fieldIdx, editor, parent )
    , mDateTimeWidget( NULL )
{
}

QWidget *QgsDateTimeEditWrapper::createWidget( QWidget *parent )
{
  QDateTimeEdit* widget = new QDateTimeEdit( parent );
  widget->setDateTime( QDateTime::currentDateTime() );
  return widget;
}

void QgsDateTimeEditWrapper::initWidget( QWidget *editor )
{
  QDateTimeEdit* w = dynamic_cast<QDateTimeEdit*>( editor );
  if ( !w )
  {
    QgsDebugMsg( "Date/time edit widget could not be initialized because provided widget is not a QDateTimeEdit." );
    QgsMessageLog::logMessage( "Date/time edit widget could not be initialized because provided widget is not a QDateTimeEdit." , "UI forms", QgsMessageLog::WARNING );
    return;
  }
  mDateTimeWidget = w;

  const QString displayFormat = config( "display_format", QGSDATETIMEEDIT_DATEFORMAT ).toString();
  mDateTimeWidget->setDisplayFormat( displayFormat );

  const bool calendar = config( "calendar_popup", false ).toBool();
  mDateTimeWidget->setCalendarPopup( calendar );
}

QVariant QgsDateTimeEditWrapper::value()
{
  if ( !mDateTimeWidget )
    return QVariant();

  const QString fieldFormat = config( "field_format", QGSDATETIMEEDIT_DATEFORMAT ).toString();
  return mDateTimeWidget->dateTime().toString( fieldFormat );
}

void QgsDateTimeEditWrapper::setValue( const QVariant &value )
{
  if ( !mDateTimeWidget )
    return;

  const QString fieldFormat = config( "field_format", QGSDATETIMEEDIT_DATEFORMAT ).toString();
  mDateTimeWidget->setDateTime( QDateTime::fromString( value.toString(), fieldFormat ) );
}

void QgsDateTimeEditWrapper::setEnabled( bool enabled )
{
  if ( !mDateTimeWidget )
    return;

  mDateTimeWidget->setEnabled( enabled );
}
