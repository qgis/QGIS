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

  mFieldFormatComboBox->clear();
  mFieldFormatComboBox->addItem( tr( "Date" ), QGSDATETIMEEDIT_DATEFORMAT );
  mFieldFormatComboBox->addItem( tr( "Time" ), QGSDATETIMEEDIT_TIMEFORMAT );
  mFieldFormatComboBox->addItem( tr( "Date time" ), QGSDATETIMEEDIT_DATETIMEFORMAT );
  mFieldFormatComboBox->addItem( tr( "ISO date time" ), QGSDATETIMEEDIT_ISODATETIMEFORMAT );
  mFieldFormatComboBox->addItem( tr( "Custom" ), QString() );

  mDemoDateTimeEdit->setDateTime( QDateTime::currentDateTime() );

  connect( mDisplayFormatEdit, SIGNAL( textChanged( QString ) ), this, SLOT( updateDemoWidget() ) );
  connect( mCalendarPopupCheckBox, SIGNAL( toggled( bool ) ), this, SLOT( updateDemoWidget() ) );

  connect( mFieldFormatComboBox, SIGNAL( currentIndexChanged( int ) ), this, SLOT( updateFieldFormat( int ) ) );
  connect( mFieldFormatEdit, SIGNAL( textChanged( QString ) ), this, SLOT( updateDisplayFormat( QString ) ) );
  connect( mDisplayFormatComboBox, SIGNAL( currentIndexChanged( int ) ), this, SLOT( displayFormatChanged( int ) ) );

  connect( mFieldHelpToolButton, SIGNAL( clicked( bool ) ), this, SLOT( showHelp( bool ) ) );
  connect( mDisplayHelpToolButton, SIGNAL( clicked( bool ) ), this, SLOT( showHelp( bool ) ) );

  connect( mFieldFormatEdit, SIGNAL( textChanged( QString ) ), this, SIGNAL( changed() ) );
  connect( mDisplayFormatEdit, SIGNAL( textChanged( QString ) ), this, SIGNAL( changed() ) );
  connect( mCalendarPopupCheckBox, SIGNAL( toggled( bool ) ), this, SIGNAL( changed() ) );
  connect( mAllowNullCheckBox, SIGNAL( toggled( bool ) ), this, SIGNAL( changed() ) );

  // initialize
  updateFieldFormat( mFieldFormatComboBox->currentIndex() );
  displayFormatChanged( mDisplayFormatComboBox->currentIndex() );
}


void QgsDateTimeEditConfig::updateDemoWidget()
{
  mDemoDateTimeEdit->setDisplayFormat( mDisplayFormatEdit->text() );
  mDemoDateTimeEdit->setCalendarPopup( mCalendarPopupCheckBox->isChecked() );
}


void QgsDateTimeEditConfig::updateFieldFormat( int idx )
{
  const QString format = mFieldFormatComboBox->itemData( idx ).toString();
  bool custom = format.isEmpty();
  if ( !custom )
  {
    mFieldFormatEdit->setText( format );
  }

  mFieldFormatEdit->setEnabled( custom );
  mFieldHelpToolButton->setVisible( custom );
  if ( mFieldHelpToolButton->isHidden() && mDisplayHelpToolButton->isHidden() )
  {
    mHelpScrollArea->setVisible( false );
  }
}



void QgsDateTimeEditConfig::updateDisplayFormat( const QString& fieldFormat )
{
  if ( mDisplayFormatComboBox->currentIndex() == 0 )
  {
    // i.e. display format is default
    if ( mFieldFormatComboBox->itemData( mFieldFormatComboBox->currentIndex() ) == QGSDATETIMEEDIT_ISODATETIMEFORMAT )
    {
      mDisplayFormatEdit->setText( QGSDATETIMEEDIT_ISODISPLAYFORMAT );
    }
    else
    {
      mDisplayFormatEdit->setText( fieldFormat );
    }
  }
}


void QgsDateTimeEditConfig::displayFormatChanged( int idx )
{
  const bool custom = idx == 1;
  mDisplayFormatEdit->setEnabled( custom );
  mDisplayHelpToolButton->setVisible( custom );
  if ( mFieldHelpToolButton->isHidden() && mDisplayHelpToolButton->isHidden() )
  {
    mHelpScrollArea->setVisible( false );
  }
  if ( !custom )
  {
    mDisplayFormatEdit->setText( mFieldFormatEdit->text() );
  }
}

void QgsDateTimeEditConfig::showHelp( bool buttonChecked )
{
  mFieldHelpToolButton->setChecked( buttonChecked );
  mDisplayHelpToolButton->setChecked( buttonChecked );
  mHelpScrollArea->setVisible( buttonChecked );
}


QgsEditorWidgetConfig QgsDateTimeEditConfig::config()
{
  QgsEditorWidgetConfig myConfig;

  myConfig.insert( "field_iso_format", mFieldFormatEdit->text() == QGSDATETIMEEDIT_ISODATETIMEFORMAT );
  myConfig.insert( "field_format", mFieldFormatEdit->text() );
  myConfig.insert( "display_format", mDisplayFormatEdit->text() );
  myConfig.insert( "calendar_popup", mCalendarPopupCheckBox->isChecked() );
  myConfig.insert( "allow_null", mAllowNullCheckBox->isChecked() );

  return myConfig;
}


void QgsDateTimeEditConfig::setConfig( const QgsEditorWidgetConfig &config )
{
  if ( config.contains( "field_format" ) )
  {
    const QString fieldFormat = config[ "field_format" ].toString();
    mFieldFormatEdit->setText( fieldFormat );

    const int idx = mFieldFormatComboBox->findData( fieldFormat );
    if ( idx >= 0 )
    {
      mFieldFormatComboBox->setCurrentIndex( idx );
    }
    else
    {
      mFieldFormatComboBox->setCurrentIndex( 4 );
    }
  }

  if ( config.contains( "display_format" ) )
  {
    const QString displayFormat = config[ "display_format" ].toString();
    mDisplayFormatEdit->setText( displayFormat );
    if ( displayFormat == mFieldFormatEdit->text() )
    {
      mDisplayFormatComboBox->setCurrentIndex( 0 );
    }
    else
    {
      mDisplayFormatComboBox->setCurrentIndex( 1 );
    }
  }

  if ( config.contains( "calendar_popup" ) )
  {
    mCalendarPopupCheckBox->setChecked( config[ "calendar_popup" ].toBool() );
  }

  if ( config.contains( "allow_null" ) )
  {
    mAllowNullCheckBox->setChecked( config[ "allow_null" ].toBool() );
  }

}
