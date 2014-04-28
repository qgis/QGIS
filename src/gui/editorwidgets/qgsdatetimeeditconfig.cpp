/***************************************************************************
    qgsdatetimeeditconfig.cpp
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

#include "qgsdatetimeeditconfig.h"
#include "qgsdatetimeeditfactory.h"

QgsDateTimeEditConfig::QgsDateTimeEditConfig( QgsVectorLayer* vl, int fieldIdx, QWidget* parent )
    : QgsEditorConfigWidget( vl, fieldIdx, parent )
{
  setupUi( this );

  mDemoDateTimeEdit->setDateTime( QDateTime::currentDateTime() );

  connect( mDateTimeRadio, SIGNAL( toggled( bool ) ), this, SLOT( updateDemoWidget() ) );
  connect( mDateRadio, SIGNAL( toggled( bool ) ), this, SLOT( updateDemoWidget() ) );
  connect( mTimeRadio, SIGNAL( toggled( bool ) ), this, SLOT( updateDemoWidget() ) );
  connect( mCustomRadio, SIGNAL( toggled( bool ) ), this, SLOT( updateDemoWidget() ) );
  connect( mFormatEdit, SIGNAL( textChanged( QString ) ), this, SLOT( updateDemoWidget() ) );
  connect( mCalendarPopupCheckBox, SIGNAL( toggled( bool ) ), this, SLOT( updateDemoWidget() ) );
}

void QgsDateTimeEditConfig::updateDemoWidget()
{
  if ( mDateTimeRadio->isChecked() )
  {
    mDemoDateTimeEdit->setDisplayFormat( "yyyy-MM-dd HH:mm:ss" );
  }
  else if ( mTimeRadio->isChecked() )
  {
    mDemoDateTimeEdit->setDisplayFormat( "HH:mm:ss" );
  }
  else if ( mDateRadio->isChecked() )
  {
    mDemoDateTimeEdit->setDisplayFormat( "yyyy-MM-dd" );
  }
  else if ( mCustomRadio->isChecked() )
  {
    mDemoDateTimeEdit->setDisplayFormat( mFormatEdit->text() );
  }

  mDemoDateTimeEdit->setCalendarPopup( mCalendarPopupCheckBox->isChecked() );
}


QgsEditorWidgetConfig QgsDateTimeEditConfig::config()
{
  QgsEditorWidgetConfig myConfig;

  QString type = "datetime";
  if ( mDateRadio->isChecked() )
    type = "date";
  else if ( mTimeRadio->isChecked() )
    type = "time";
  myConfig.insert( "type", type );

  myConfig.insert( "format", mFormatEdit->text() );
  myConfig.insert( "calendar_popup", mCalendarPopupCheckBox->isChecked() );

  return myConfig;
}

void QgsDateTimeEditConfig::setConfig( const QgsEditorWidgetConfig &config )
{
  if ( config.contains( "type" ) )
  {
    QString type = config[ "type" ].toString();
    if ( type == "datetime" )
      mDateTimeRadio->setChecked( true );
    else if ( type == "time" )
      mTimeRadio->setChecked( true );
    else if ( type == "date" )
      mDateRadio->setChecked( true );
  }

  if ( config.contains( "format" ) )
  {
    mFormatEdit->setText( config[ "format" ].toString() );
  }

  if ( config.contains( "calendar_popup" ) )
  {
    mCalendarPopupCheckBox->setChecked( config[ "calendar_popup" ].toBool() );
  }

}
