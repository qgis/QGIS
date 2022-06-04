/***************************************************************************
                          qgselevationprofilepdfexportdialog.h
                          ---------------
    begin                : March 2022
    copyright            : (C) 2022 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgselevationprofilepdfexportdialog.h"
#include "qgsplot.h"
#include "qgselevationprofileexportsettingswidget.h"
#include "qgsgui.h"
#include "qgslayoutitempage.h"
#include "qgspagesizeregistry.h"

QgsElevationProfilePdfExportDialog::QgsElevationProfilePdfExportDialog( QWidget *parent )
  : QDialog( parent )
{
  setupUi( this );

  mProfileSettingsWidget = new QgsElevationProfileExportSettingsWidget();
  scrollAreaLayout->addWidget( mProfileSettingsWidget );
  scrollAreaLayout->addStretch( 1 );

  QgsGui::enableAutoGeometryRestore( this );

  mPageOrientationComboBox->addItem( tr( "Portrait" ), QgsLayoutItemPage::Portrait );
  mPageOrientationComboBox->addItem( tr( "Landscape" ), QgsLayoutItemPage::Landscape );

  const QList< QgsPageSize> sizes = QgsApplication::pageSizeRegistry()->entries();
  for ( const QgsPageSize &size : sizes )
  {
    mPageSizeComboBox->addItem( size.displayName, size.name );
  }
  mPageSizeComboBox->addItem( tr( "Custom" ) );

  const QgsPageSize a4Size = QgsApplication::pageSizeRegistry()->find( QStringLiteral( "A4" ) ).at( 0 );
  mWidthSpin->setValue( a4Size.size.width() );
  mHeightSpin->setValue( a4Size.size.height() );
  mSizeUnitsComboBox->setUnit( a4Size.size.units() );

  mSizeUnitsComboBox->linkToWidget( mWidthSpin );
  mSizeUnitsComboBox->linkToWidget( mHeightSpin );
  mSizeUnitsComboBox->setConverter( &mConverter );

  mLockAspectRatio->setWidthSpinBox( mWidthSpin );
  mLockAspectRatio->setHeightSpinBox( mHeightSpin );

  connect( mPageSizeComboBox, static_cast<void ( QComboBox::* )( int )>( &QComboBox::currentIndexChanged ), this, &QgsElevationProfilePdfExportDialog::pageSizeChanged );
  connect( mPageOrientationComboBox, static_cast<void ( QComboBox::* )( int )>( &QComboBox::currentIndexChanged ), this, &QgsElevationProfilePdfExportDialog::orientationChanged );

  connect( mWidthSpin, static_cast< void ( QDoubleSpinBox::* )( double )>( &QDoubleSpinBox::valueChanged ), this, &QgsElevationProfilePdfExportDialog::setToCustomSize );
  connect( mHeightSpin, static_cast< void ( QDoubleSpinBox::* )( double )>( &QDoubleSpinBox::valueChanged ), this, &QgsElevationProfilePdfExportDialog::setToCustomSize );

  whileBlocking( mPageSizeComboBox )->setCurrentIndex( mPageSizeComboBox->findData( QStringLiteral( "A4" ) ) );
  mLockAspectRatio->setEnabled( false );
  mLockAspectRatio->setLocked( false );
  mSizeUnitsComboBox->setEnabled( false );
  mPageOrientationComboBox->setEnabled( true );

  mPageOrientationComboBox->setCurrentIndex( mPageOrientationComboBox->findData( QgsLayoutItemPage::Landscape ) );
}

void QgsElevationProfilePdfExportDialog::setPlotSettings( const Qgs2DPlot &plot )
{
  mProfileSettingsWidget->setPlotSettings( plot );
}

void QgsElevationProfilePdfExportDialog::updatePlotSettings( Qgs2DPlot &plot )
{
  mProfileSettingsWidget->updatePlotSettings( plot );
}

QgsLayoutSize QgsElevationProfilePdfExportDialog::pageSizeMM() const
{
  return mConverter.convert( QgsLayoutSize( mWidthSpin->value(), mHeightSpin->value(), mSizeUnitsComboBox->unit() ), QgsUnitTypes::LayoutMillimeters );
}

void QgsElevationProfilePdfExportDialog::pageSizeChanged( int )
{
  if ( mPageSizeComboBox->currentData().toString().isEmpty() )
  {
    //custom size
    mLockAspectRatio->setEnabled( true );
    mSizeUnitsComboBox->setEnabled( true );
    mPageOrientationComboBox->setEnabled( false );
  }
  else
  {
    mLockAspectRatio->setEnabled( false );
    mLockAspectRatio->setLocked( false );
    mSizeUnitsComboBox->setEnabled( false );
    mPageOrientationComboBox->setEnabled( true );
    const QgsPageSize size = QgsApplication::pageSizeRegistry()->find( mPageSizeComboBox->currentData().toString() ).value( 0 );
    const QgsLayoutSize convertedSize = mConverter.convert( size.size, mSizeUnitsComboBox->unit() );
    mSettingPresetSize = true;
    switch ( mPageOrientationComboBox->currentData().toInt() )
    {
      case QgsLayoutItemPage::Landscape:
        mWidthSpin->setValue( convertedSize.height() );
        mHeightSpin->setValue( convertedSize.width() );
        break;

      case QgsLayoutItemPage::Portrait:
        mWidthSpin->setValue( convertedSize.width() );
        mHeightSpin->setValue( convertedSize.height() );
        break;
    }
    mSettingPresetSize = false;
  }
}

void QgsElevationProfilePdfExportDialog::orientationChanged( int )
{
  if ( mPageSizeComboBox->currentData().toString().isEmpty() )
    return;

  const double width = mWidthSpin->value();
  const double height = mHeightSpin->value();
  switch ( mPageOrientationComboBox->currentData().toInt() )
  {
    case QgsLayoutItemPage::Landscape:
      if ( width < height )
      {
        whileBlocking( mWidthSpin )->setValue( height );
        whileBlocking( mHeightSpin )->setValue( width );
      }
      break;

    case QgsLayoutItemPage::Portrait:
      if ( width > height )
      {
        whileBlocking( mWidthSpin )->setValue( height );
        whileBlocking( mHeightSpin )->setValue( width );
      }
      break;
  }
}

void QgsElevationProfilePdfExportDialog::setToCustomSize()
{
  if ( mSettingPresetSize )
    return;
  whileBlocking( mPageSizeComboBox )->setCurrentIndex( mPageSizeComboBox->count() - 1 );
  mPageOrientationComboBox->setEnabled( false );
  pageSizeChanged( mPageSizeComboBox->currentIndex() );
}
