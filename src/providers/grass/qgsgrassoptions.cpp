/***************************************************************************
                              qgsgrassoptions.cpp
                             -------------------
    begin                : May, 2015
    copyright            : (C) 2015 Radim Blazek
    email                : radim.blazek@gmail.com
 ***************************************************************************/
/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "qgsrasterprojector.h"

#include "qgsgrassoptions.h"
#include "ui_qgsgrassoptionsbase.h"

QgsGrassOptions::QgsGrassOptions( QWidget *parent )
    : QDialog( parent )
    , QgsGrassOptionsBase()
    , mImportSettingsPath( "/GRASS/browser/import" )
{
  setupUi( this );

  connect( this, SIGNAL( accepted() ), this, SLOT( saveOptions() ) );

  QSettings settings;

  QgsRasterProjector::Precision crsTransform = ( QgsRasterProjector::Precision ) settings.value( mImportSettingsPath + "/crsTransform", QgsRasterProjector::Approximate ).toInt();
  mCrsTransformationComboBox->addItem( QgsRasterProjector::precisionLabel( QgsRasterProjector::Approximate ), QgsRasterProjector::Approximate );
  mCrsTransformationComboBox->addItem( QgsRasterProjector::precisionLabel( QgsRasterProjector::Exact ), QgsRasterProjector::Exact );
  mCrsTransformationComboBox->setCurrentIndex( mCrsTransformationComboBox->findData( crsTransform ) );

  mImportExternalCheckBox->setChecked( settings.value( mImportSettingsPath + "/external", true ).toBool() );
}

QgsGrassOptions::~QgsGrassOptions()
{
}

void QgsGrassOptions::saveOptions()
{
  QSettings settings;

  settings.setValue( mImportSettingsPath + "/crsTransform",
                     mCrsTransformationComboBox->itemData( mCrsTransformationComboBox->currentIndex() ).toInt() );

  settings.setValue( mImportSettingsPath + "/external", mImportExternalCheckBox->isChecked() );
}
