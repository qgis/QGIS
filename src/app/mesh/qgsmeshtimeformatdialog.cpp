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
#include "qgsmeshlayerutils.h"

QgsMeshTimeFormatDialog::QgsMeshTimeFormatDialog( QgsMeshLayer *meshLayer, QWidget *parent, Qt::WindowFlags f )
  : QDialog( parent, f ),
    mLayer( meshLayer )
{
  setupUi( this );
  QgsGui::enableAutoGeometryRestore( this );

  if ( !meshLayer )
    return;

  loadSettings();

  mReloadReferenceTimeButton->setEnabled( layerHasReferenceTime() );

  connect( mUseTimeComboBox, qgis::overload<int>::of( &QComboBox::currentIndexChanged ), this, &QgsMeshTimeFormatDialog::saveSettings );
  connect( mReferenceDateTimeEdit, &QDateTimeEdit::dateTimeChanged, this, &QgsMeshTimeFormatDialog::saveSettings );
  connect( mAbsoluteTimeFormatComboBox, qgis::overload<int>::of( &QComboBox::currentIndexChanged ), this, &QgsMeshTimeFormatDialog::saveSettings );
  connect( mUseTimeComboBox, qgis::overload<int>::of( &QComboBox::currentIndexChanged ), this, &QgsMeshTimeFormatDialog::saveSettings );
  connect( mRelativeTimeFormatComboBox, qgis::overload<int>::of( &QComboBox::currentIndexChanged ), this, &QgsMeshTimeFormatDialog::saveSettings );
  connect( mOffsetHoursSpinBox, qgis::overload<double>::of( &QDoubleSpinBox::valueChanged ), this, &QgsMeshTimeFormatDialog::saveSettings );
  connect( mPlaybackIntervalSpinBox, qgis::overload<double>::of( &QDoubleSpinBox::valueChanged ), this, &QgsMeshTimeFormatDialog::saveSettings );
  connect( mProviderTimeUnitComboBox, qgis::overload<int>::of( &QComboBox::currentIndexChanged ), this, &QgsMeshTimeFormatDialog::saveSettings );

  connect( mReloadReferenceTimeButton, &QPushButton::clicked, this, &QgsMeshTimeFormatDialog::loadProviderReferenceTime );

}

void QgsMeshTimeFormatDialog::loadProviderReferenceTime()
{
  mReferenceDateTimeEdit->setDateTime( QgsMeshLayerUtils::firstReferenceTime( mLayer ) );
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

  // Sets the reference time, if not valid, sets the current date + time 00:00:00
  if ( settings.absoluteTimeReferenceTime().isValid() )
    mReferenceDateTimeEdit->setDateTime( settings.absoluteTimeReferenceTime() );
  else
    mReferenceDateTimeEdit->setDateTime( QDateTime( QDate::currentDate(), QTime( 00, 00, 00 ) ) );

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

  mProviderTimeUnitComboBox->setCurrentIndex( settings.providerTimeUnit() );
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
  settings.setProviderTimeUnit( static_cast<QgsMeshTimeSettings::TimeUnit>( mProviderTimeUnitComboBox->currentIndex() ) );
  enableGroups( settings.useAbsoluteTime() ) ;
  mLayer->setTimeSettings( settings );
}

void QgsMeshTimeFormatDialog::enableGroups( bool useAbsoluteTime )
{
  mAbsoluteTimeGroupBox->setEnabled( useAbsoluteTime );
  mRelativeTimeGroupBox->setEnabled( ! useAbsoluteTime );
}

bool QgsMeshTimeFormatDialog::layerHasReferenceTime() const
{
  return QgsMeshLayerUtils::firstReferenceTime( mLayer ).isValid();
}

QgsMeshTimeFormatDialog::~QgsMeshTimeFormatDialog() = default;
