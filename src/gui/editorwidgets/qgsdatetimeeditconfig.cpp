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

  connect( displayButtonGroup, SIGNAL( buttonClicked( int ) ), this, SLOT( updateDemoWidget() ) );
  connect( mDisplayFormatEdit, SIGNAL( textChanged( QString ) ), this, SLOT( updateDemoWidget() ) );
  connect( mCalendarPopupCheckBox, SIGNAL( toggled( bool ) ), this, SLOT( updateDemoWidget() ) );

  connect( fieldButtonGroup, SIGNAL( buttonClicked( int ) ), this, SLOT( updateFieldFormatText() ) );
  connect( displayButtonGroup, SIGNAL( buttonClicked( int ) ), this, SLOT( updateDisplayFormatText() ) );
  connect( mFieldFormatEdit, SIGNAL( textChanged( QString ) ), this, SLOT( updateDisplayFormatText() ) );

  updateFieldFormatText();
  updateDisplayFormatText();
}


void QgsDateTimeEditConfig::updateDemoWidget()
{
  mDemoDateTimeEdit->setDisplayFormat( mDisplayFormatEdit->text() );
  mDemoDateTimeEdit->setCalendarPopup( mCalendarPopupCheckBox->isChecked() );
}


void QgsDateTimeEditConfig::updateFieldFormatText()
{
  // if auto => use the default field format
  if ( !mCustomFieldFormatRadio->isChecked() )
  {
    if ( mDateTimeRadio->isChecked() )
    {
      mFieldFormatEdit->setText( QgsDateTimeEditFactory::DateTimeFormat );
    }
    else if ( mTimeRadio->isChecked() )
    {
      mFieldFormatEdit->setText( QgsDateTimeEditFactory::TimeFormat );
    }
    else if ( mDateRadio->isChecked() )
    {
      mFieldFormatEdit->setText( QgsDateTimeEditFactory::DateFormat );
    }
  }
}


void QgsDateTimeEditConfig::updateDisplayFormatText()
{
  if ( !mCustomDisplayFormatRadio->isChecked() )
  {
    mDisplayFormatEdit->setText( mFieldFormatEdit->text() );
  }
}


QgsEditorWidgetConfig QgsDateTimeEditConfig::config()
{
  QgsEditorWidgetConfig myConfig;

  myConfig.insert( "field_format", mFieldFormatEdit->text() );
  myConfig.insert( "display_format", mDisplayFormatEdit->text() );
  myConfig.insert( "calendar_popup", mCalendarPopupCheckBox->isChecked() );

  return myConfig;
}


void QgsDateTimeEditConfig::setConfig( const QgsEditorWidgetConfig &config )
{
  if ( config.contains( "field_format" ) )
  {
    const QString fieldFormat = config[ "field_format" ].toString();
    mFieldFormatEdit->setText( fieldFormat );

    if ( fieldFormat == QgsDateTimeEditFactory::DateFormat )
      mDateRadio->setChecked( true );
    else if ( fieldFormat == QgsDateTimeEditFactory::TimeFormat )
      mTimeRadio->setChecked( true );
    else if ( fieldFormat == QgsDateTimeEditFactory::DateTimeFormat )
      mDateTimeRadio->setChecked( true );
    else
      mCustomFieldFormatRadio->setChecked( true );
  }

  if ( config.contains( "display_format" ) )
  {
    const QString displayFormat = config[ "display_format" ].toString();
    mDisplayFormatEdit->setText( displayFormat );
    mAutoDisplayFormatRadio->setChecked( displayFormat == mFieldFormatEdit->text() );
    mCustomDisplayFormatRadio->setChecked( displayFormat != mFieldFormatEdit->text() );
  }

  if ( config.contains( "calendar_popup" ) )
  {
    mCalendarPopupCheckBox->setChecked( config[ "calendar_popup" ].toBool() );
  }

}
