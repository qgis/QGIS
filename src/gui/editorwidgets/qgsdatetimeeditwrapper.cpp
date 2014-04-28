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
    , mType( "date" )
{
}

QWidget *QgsDateTimeEditWrapper::createWidget( QWidget *parent )
{
  return new QDateTimeEdit( parent );
}

void QgsDateTimeEditWrapper::initWidget( QWidget *editor )
{
  mType = config( "type", "datetime" ).toString();

  QDateTimeEdit* w = dynamic_cast<QDateTimeEdit*>( editor );
  if ( !w )
  {
    QgsDebugMsg( "Date/time edit widget could not be initialized because provided widget is not a QDateTimeEdit." );
    QgsMessageLog::logMessage( "Date/time edit widget could not be initialized because provided widget is not a QDateTimeEdit." , "UI forms", QgsMessageLog::WARNING );
    return;
  }
  mDateTimeWidget = w;

  if ( mType ==  "datetime" )
  {
    mDateTimeWidget->setDisplayFormat( "yyyy-MM-dd HH:mm:ss" );
  }
  else if ( mType ==  "time" )
  {
    mDateTimeWidget->setDisplayFormat( "HH:mm:ss" );
  }
  else if ( mType ==  "date" )
  {
    mDateTimeWidget->setDisplayFormat( "yyyy-MM-dd" );
  }
  else if ( mType == "custom" )
  {
    QString format = config( "format", "yyyy-MM-dd HH:mm" ).toString();
    mDateTimeWidget->setDisplayFormat( format );
  }

  if ( mType != "time" )
  {
    mDateTimeWidget->setCalendarPopup( config( "calendar_popup", "0" ) == 1 );
  }
}

QVariant QgsDateTimeEditWrapper::value()
{
  if ( !mDateTimeWidget )
    return QVariant();

  if ( mType ==  "datetime" )
  {
    return mDateTimeWidget->dateTime().toString( "yyyy-MM-dd HH:mm:ss" );
  }
  else if ( mType ==  "time" )
  {
    return mDateTimeWidget->time().toString( "HH:mm:ss" );
  }
  else if ( mType ==  "date" )
  {

    return mDateTimeWidget->date().toString( "yyyy-MM-dd" );
  }
  else if ( mType == "custom" )
  {
    QString format = config( "format", "yyyy-MM-dd HH:mm:ss" ).toString();
    return mDateTimeWidget->date().toString( format );
  }
}


void QgsDateTimeEditWrapper::setValue( const QVariant &value )
{
  if ( !mDateTimeWidget )
    return;

  if ( mType == "datetime" )
  {
    mDateTimeWidget->setDateTime( QDateTime::fromString( value.toString(), "yyyy-MM-dd HH:mm:ss" ) );
  }
  else if ( mType ==  "time" )
  {
    mDateTimeWidget->setTime( value.toTime() );
  }
  else if ( mType == "date" )
  {
    mDateTimeWidget->setDate( value.toDate() );
  }
  else if ( mType == "custom" )
  {
    QString format = config( "format", "yyyy-MM-dd HH:mm" ).toString();
    mDateTimeWidget->setDateTime( QDateTime::fromString( value.toString(), format ) );
  }
  else
    return;

}

void QgsDateTimeEditWrapper::setEnabled( bool enabled )
{
  if ( !mDateTimeWidget )
    return;

  mDateTimeWidget->setEnabled( enabled );
}
