/***************************************************************************
                          qgsmeshtimeformatdialog.cpp
                          ---------------------------
    begin                : March 2019
    copyright            : (C) 2019 by Peter Petrik
    email                : zilolv at gmail dot com
 ***************************************************************************/
/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsmeshtimeformatdialog.h"
#include "qgsgui.h"
#include "qgsmeshtimesettings.h"

QgsMeshTimeFormatDialog::QgsMeshTimeFormatDialog( QgsMeshLayer *meshLayer, QWidget *parent, Qt::WindowFlags f )
  : QDialog( parent, f ),
    mLayer( meshLayer )
{
  setupUi( this );
  QgsGui::enableAutoGeometryRestore( this );

  if ( !meshLayer )
    return;

  loadSettings();

  connect( mUseTimeComboBox, qgis::overload<int>::of( &QComboBox::currentIndexChanged ), this, &QgsMeshTimeFormatDialog::saveSettings );
  connect( mReferenceDateTimeEdit, &QDateTimeEdit::timeChanged, this, &QgsMeshTimeFormatDialog::saveSettings );
  connect( mAbsoluteTimeFormatComboBox, qgis::overload<int>::of( &QComboBox::currentIndexChanged ), this, &QgsMeshTimeFormatDialog::saveSettings );
  connect( mUseTimeComboBox, qgis::overload<int>::of( &QComboBox::currentIndexChanged ), this, &QgsMeshTimeFormatDialog::saveSettings );
  connect( mRelativeTimeFormatComboBox, qgis::overload<int>::of( &QComboBox::currentIndexChanged ), this, &QgsMeshTimeFormatDialog::saveSettings );
  connect( mOffsetHoursSpinBox, qgis::overload<double>::of( &QDoubleSpinBox::valueChanged ), this, &QgsMeshTimeFormatDialog::saveSettings );
  connect( mPlaybackIntervalSpinBox, qgis::overload<double>::of( &QDoubleSpinBox::valueChanged ), this, &QgsMeshTimeFormatDialog::saveSettings );

}

void QgsMeshTimeFormatDialog::loadSettings()
{
  const QgsMeshTimeSettings settings = mLayer->timeSettings();

  enableGroups( settings.useAbsoluteTime() ) ;
  if ( settings.useAbsoluteTime() )
  {
    mUseTimeComboBox->setCurrentIndex( 0 );
  }
  else
  {
    mUseTimeComboBox->setCurrentIndex( 1 );
  }

  mReferenceDateTimeEdit->setDateTime( settings.absoluteTimeReferenceTime() );
  mReferenceDateTimeEdit->setDisplayFormat( settings.absoluteTimeFormat() );

  int index = mAbsoluteTimeFormatComboBox->findText( settings.absoluteTimeFormat() );
  if ( index < 0 )
  {
    index = mAbsoluteTimeFormatComboBox->count();
    mAbsoluteTimeFormatComboBox->addItem( settings.absoluteTimeFormat() );
  }
  mAbsoluteTimeFormatComboBox->setCurrentIndex( index );

  index = mRelativeTimeFormatComboBox->findText( settings.relativeTimeFormat() );
  if ( index < 0 )
  {
    index = mRelativeTimeFormatComboBox->count();
    mRelativeTimeFormatComboBox->addItem( settings.relativeTimeFormat() );
  }
  mRelativeTimeFormatComboBox->setCurrentIndex( index );

  mOffsetHoursSpinBox->setValue( settings.relativeTimeOffsetHours() );
  mPlaybackIntervalSpinBox->setValue( settings.datasetPlaybackInterval() );
}

void QgsMeshTimeFormatDialog::saveSettings()
{
  QgsMeshTimeSettings settings;
  settings.setUseAbsoluteTime( mUseTimeComboBox->currentIndex() == 0 );
  settings.setAbsoluteTimeReferenceTime( mReferenceDateTimeEdit->dateTime() );
  settings.setAbsoluteTimeFormat( mAbsoluteTimeFormatComboBox->currentText() );
  settings.setRelativeTimeOffsetHours( mOffsetHoursSpinBox->value() );
  settings.setRelativeTimeFormat( mRelativeTimeFormatComboBox->currentText() );
  settings.setDatasetPlaybackInterval( mPlaybackIntervalSpinBox->value() );
  enableGroups( settings.useAbsoluteTime() ) ;
  mLayer->setTimeSettings( settings );
}

void QgsMeshTimeFormatDialog::enableGroups( bool useAbsoluteTime )
{
  mAbsoluteTimeGroupBox->setEnabled( useAbsoluteTime );
  mRelativeTimeGroupBox->setEnabled( ! useAbsoluteTime );
}

QgsMeshTimeFormatDialog::~QgsMeshTimeFormatDialog() = default;
