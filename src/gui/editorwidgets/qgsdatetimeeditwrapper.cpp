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
    , mQDateTimeEdit( NULL )
    , mQgsDateTimeEdit( NULL )
{
}

QWidget *QgsDateTimeEditWrapper::createWidget( QWidget *parent )
{
  QgsDateTimeEdit* widget = new QgsDateTimeEdit( parent );
  widget->setDateTime( QDateTime::currentDateTime() );
  return widget;
}

void QgsDateTimeEditWrapper::initWidget( QWidget *editor )
{
  QgsDateTimeEdit* qgsEditor = dynamic_cast<QgsDateTimeEdit*>( editor );
  if ( qgsEditor )
  {
    mQgsDateTimeEdit = qgsEditor;
  }
  // assign the Qt editor also if the QGIS editor has been previously assigned
  // this avoids testing each time which widget to use
  // the QGIS editor must be used for non-virtual methods (dateTime, setDateTime)
  QDateTimeEdit* qtEditor = dynamic_cast<QDateTimeEdit*>( editor );
  if ( qtEditor )
  {
    mQDateTimeEdit = qtEditor;
  }

  if ( !mQDateTimeEdit )
  {
    QgsDebugMsg( "Date/time edit widget could not be initialized because provided widget is not a QDateTimeEdit." );
    QgsMessageLog::logMessage( "Date/time edit widget could not be initialized because provided widget is not a QDateTimeEdit." , "UI forms", QgsMessageLog::WARNING );
    return;
  }

  const QString displayFormat = config( "display_format", QGSDATETIMEEDIT_DATEFORMAT ).toString();
  mQDateTimeEdit->setDisplayFormat( displayFormat );

  const bool calendar = config( "calendar_popup", false ).toBool();
  mQDateTimeEdit->setCalendarPopup( calendar );

  const bool allowNull = config( "allow_null", true ).toBool();
  if ( mQgsDateTimeEdit )
  {
    mQgsDateTimeEdit->setAllowNull( allowNull );
  }
  else
  {
    QgsMessageLog::instance()->logMessage( tr( "The usual date/time widget QDateTimeEdit cannot be configured to allow NULL values. "
                                           "For that the QGIS custom widget QgsDateTimeEdit is used." ),
                                           "field widgets" );
  }

  if ( mQgsDateTimeEdit )
  {
    connect( mQgsDateTimeEdit, SIGNAL( dateTimeChanged( QDateTime ) ), this, SLOT( dateTimeChanged( QDateTime ) ) );
  }
  else
  {
    connect( mQDateTimeEdit, SIGNAL( dateTimeChanged( QDateTime ) ), this,  SLOT( dateTimeChanged( QDateTime ) ) );
  }
}

void QgsDateTimeEditWrapper::dateTimeChanged( const QDateTime& dateTime )
{
  const QString fieldFormat = config( "field_format", QGSDATETIMEEDIT_DATEFORMAT ).toString();
  emit valueChanged( dateTime.toString( fieldFormat ) );
}

QVariant QgsDateTimeEditWrapper::value()
{
  if ( !mQDateTimeEdit )
    return QVariant( field().type() );

  const QString fieldFormat = config( "field_format", QGSDATETIMEEDIT_DATEFORMAT ).toString();

  if ( mQgsDateTimeEdit )
  {
    return mQgsDateTimeEdit->dateTime().toString( fieldFormat );
  }
  else
  {
    return mQDateTimeEdit->dateTime().toString( fieldFormat );
  }
}

void QgsDateTimeEditWrapper::setValue( const QVariant &value )
{
  if ( !mQDateTimeEdit )
    return;

  const QString fieldFormat = config( "field_format", QGSDATETIMEEDIT_DATEFORMAT ).toString();
  const QDateTime date = QDateTime::fromString( value.toString(), fieldFormat );

  if ( mQgsDateTimeEdit )
  {
    mQgsDateTimeEdit->setDateTime( date );
  }
  else
  {
    mQDateTimeEdit->setDateTime( date );
  }
}

void QgsDateTimeEditWrapper::setEnabled( bool enabled )
{
  if ( !mQDateTimeEdit )
    return;

  mQDateTimeEdit->setEnabled( enabled );
}
