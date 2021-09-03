/***************************************************************************
                             qgslayoutaddpagesdialog.cpp
                             ---------------------------
    Date                 : July 2017
    Copyright            : (C) 2017 Nyall Dawson
    Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgslayoutaddpagesdialog.h"
#include "qgspagesizeregistry.h"
#include "qgssettings.h"
#include "qgslayout.h"
#include "qgslayoutmeasurementconverter.h"
#include "qgslayoutpagecollection.h"
#include "qgshelp.h"

QgsLayoutAddPagesDialog::QgsLayoutAddPagesDialog( QWidget *parent, Qt::WindowFlags flags )
  : QDialog( parent, flags )
{
  setupUi( this );

  mPageOrientationComboBox->addItem( tr( "Portrait" ), QgsLayoutItemPage::Portrait );
  mPageOrientationComboBox->addItem( tr( "Landscape" ), QgsLayoutItemPage::Landscape );
  mPageOrientationComboBox->setCurrentIndex( 1 );

  const auto constEntries = QgsApplication::pageSizeRegistry()->entries();
  for ( const QgsPageSize &size : constEntries )
  {
    mPageSizeComboBox->addItem( size.displayName, size.name );
  }
  mPageSizeComboBox->addItem( tr( "Custom" ) );
  mPageSizeComboBox->setCurrentIndex( mPageSizeComboBox->findData( QStringLiteral( "A4" ) ) );
  pageSizeChanged( mPageSizeComboBox->currentIndex() );
  orientationChanged( 1 );

  mSizeUnitsComboBox->linkToWidget( mWidthSpin );
  mSizeUnitsComboBox->linkToWidget( mHeightSpin );

  mLockAspectRatio->setWidthSpinBox( mWidthSpin );
  mLockAspectRatio->setHeightSpinBox( mHeightSpin );

  connect( mPositionComboBox, static_cast<void ( QComboBox::* )( int )>( &QComboBox::currentIndexChanged ), this, &QgsLayoutAddPagesDialog::positionChanged );
  mExistingPageSpinBox->setEnabled( false );

  connect( mPageSizeComboBox, static_cast<void ( QComboBox::* )( int )>( &QComboBox::currentIndexChanged ), this, &QgsLayoutAddPagesDialog::pageSizeChanged );
  connect( mPageOrientationComboBox, static_cast<void ( QComboBox::* )( int )>( &QComboBox::currentIndexChanged ), this, &QgsLayoutAddPagesDialog::orientationChanged );

  connect( mWidthSpin, static_cast< void ( QDoubleSpinBox::* )( double )>( &QDoubleSpinBox::valueChanged ), this, &QgsLayoutAddPagesDialog::setToCustomSize );
  connect( mHeightSpin, static_cast< void ( QDoubleSpinBox::* )( double )>( &QDoubleSpinBox::valueChanged ), this, &QgsLayoutAddPagesDialog::setToCustomSize );

  connect( buttonBox, &QDialogButtonBox::helpRequested, this, &QgsLayoutAddPagesDialog::showHelp );
}

void QgsLayoutAddPagesDialog::setLayout( QgsLayout *layout )
{
  mConverter = layout->renderContext().measurementConverter();
  mSizeUnitsComboBox->setConverter( &mConverter );
  mExistingPageSpinBox->setMaximum( layout->pageCollection()->pageCount() );
  mSizeUnitsComboBox->setUnit( layout->units() );
}

int QgsLayoutAddPagesDialog::numberPages() const
{
  return mPagesSpinBox->value();
}

QgsLayoutAddPagesDialog::PagePosition QgsLayoutAddPagesDialog::pagePosition() const
{
  return static_cast< PagePosition >( mPositionComboBox->currentIndex() );
}

int QgsLayoutAddPagesDialog::beforePage() const
{
  return mExistingPageSpinBox->value();
}

QgsLayoutSize QgsLayoutAddPagesDialog::pageSize() const
{
  return QgsLayoutSize( mWidthSpin->value(), mHeightSpin->value(), mSizeUnitsComboBox->unit() );
}

void QgsLayoutAddPagesDialog::positionChanged( int index )
{
  mExistingPageSpinBox->setEnabled( index != 2 );
}

void QgsLayoutAddPagesDialog::pageSizeChanged( int )
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

void QgsLayoutAddPagesDialog::orientationChanged( int )
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

void QgsLayoutAddPagesDialog::setToCustomSize()
{
  if ( mSettingPresetSize )
    return;
  whileBlocking( mPageSizeComboBox )->setCurrentIndex( mPageSizeComboBox->count() - 1 );
  mPageOrientationComboBox->setEnabled( false );
  mLockAspectRatio->setEnabled( true );
  mSizeUnitsComboBox->setEnabled( true );
}

void QgsLayoutAddPagesDialog::showHelp()
{
  QgsHelp::openHelp( QStringLiteral( "print_composer/overview_composer.html#working-with-the-page-properties" ) );
}
