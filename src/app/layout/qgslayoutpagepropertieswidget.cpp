/***************************************************************************
                             qgslayoutpagepropertieswidget.cpp
                             ---------------------------------
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

#include "qgslayoutpagepropertieswidget.h"
#include "qgsapplication.h"
#include "qgspagesizeregistry.h"
#include "qgslayoutitempage.h"
#include "qgslayout.h"

QgsLayoutPagePropertiesWidget::QgsLayoutPagePropertiesWidget( QWidget *parent, QgsLayoutItem *layoutItem )
  : QgsLayoutItemBaseWidget( parent, layoutItem )
  , mPage( static_cast< QgsLayoutItemPage *>( layoutItem ) )
{
  setupUi( this );

  mPageOrientationComboBox->addItem( tr( "Portrait" ), QgsLayoutItemPage::Portrait );
  mPageOrientationComboBox->addItem( tr( "Landscape" ), QgsLayoutItemPage::Landscape );

  Q_FOREACH ( const QgsPageSize &size, QgsApplication::pageSizeRegistry()->entries() )
  {
    mPageSizeComboBox->addItem( size.displayName, size.name );
  }
  mPageSizeComboBox->addItem( tr( "Custom" ) );

  mWidthSpin->setValue( mPage->pageSize().width() );
  mHeightSpin->setValue( mPage->pageSize().height() );
  mSizeUnitsComboBox->setUnit( mPage->pageSize().units() );

  mPageOrientationComboBox->setCurrentIndex( mPageOrientationComboBox->findData( mPage->orientation() ) );

  mSizeUnitsComboBox->linkToWidget( mWidthSpin );
  mSizeUnitsComboBox->linkToWidget( mHeightSpin );
  mSizeUnitsComboBox->setConverter( &mPage->layout()->context().measurementConverter() );

  mLockAspectRatio->setWidthSpinBox( mWidthSpin );
  mLockAspectRatio->setHeightSpinBox( mHeightSpin );

  mSymbolButton->setSymbolType( QgsSymbol::Fill );
  mSymbolButton->setSymbol( mPage->layout()->pageCollection()->pageStyleSymbol()->clone() );

  connect( mPageSizeComboBox, static_cast<void ( QComboBox::* )( int )>( &QComboBox::currentIndexChanged ), this, &QgsLayoutPagePropertiesWidget::pageSizeChanged );
  connect( mPageOrientationComboBox, static_cast<void ( QComboBox::* )( int )>( &QComboBox::currentIndexChanged ), this, &QgsLayoutPagePropertiesWidget::orientationChanged );

  connect( mWidthSpin, static_cast< void ( QDoubleSpinBox::* )( double )>( &QDoubleSpinBox::valueChanged ), this, &QgsLayoutPagePropertiesWidget::updatePageSize );
  connect( mHeightSpin, static_cast< void ( QDoubleSpinBox::* )( double )>( &QDoubleSpinBox::valueChanged ), this, &QgsLayoutPagePropertiesWidget::updatePageSize );
  connect( mWidthSpin, static_cast< void ( QDoubleSpinBox::* )( double )>( &QDoubleSpinBox::valueChanged ), this, &QgsLayoutPagePropertiesWidget::setToCustomSize );
  connect( mHeightSpin, static_cast< void ( QDoubleSpinBox::* )( double )>( &QDoubleSpinBox::valueChanged ), this, &QgsLayoutPagePropertiesWidget::setToCustomSize );

  connect( mSymbolButton, &QgsSymbolButton::changed, this, &QgsLayoutPagePropertiesWidget::symbolChanged );
  registerDataDefinedButton( mPaperSizeDDBtn, QgsLayoutObject::PresetPaperSize );
  registerDataDefinedButton( mWidthDDBtn, QgsLayoutObject::ItemWidth );
  registerDataDefinedButton( mHeightDDBtn, QgsLayoutObject::ItemHeight );

  showCurrentPageSize();
}

void QgsLayoutPagePropertiesWidget::pageSizeChanged( int )
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
    QgsPageSize size = QgsApplication::pageSizeRegistry()->find( mPageSizeComboBox->currentData().toString() ).value( 0 );
    QgsLayoutSize convertedSize = mConverter.convert( size.size, mSizeUnitsComboBox->unit() );
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
  updatePageSize();
}

void QgsLayoutPagePropertiesWidget::orientationChanged( int )
{
  if ( mPageSizeComboBox->currentData().toString().isEmpty() )
    return;

  double width = mWidthSpin->value();
  double height = mHeightSpin->value();
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

  updatePageSize();
}

void QgsLayoutPagePropertiesWidget::updatePageSize()
{
  mPage->layout()->undoStack()->beginCommand( mPage, tr( "Change Page Size" ), 1 + mPage->layout()->pageCollection()->pageNumber( mPage ) );
  mPage->setPageSize( QgsLayoutSize( mWidthSpin->value(), mHeightSpin->value(), mSizeUnitsComboBox->unit() ) );
  mPage->layout()->undoStack()->endCommand();
  mPage->layout()->pageCollection()->reflow();
}

void QgsLayoutPagePropertiesWidget::setToCustomSize()
{
  if ( mSettingPresetSize )
    return;
  whileBlocking( mPageSizeComboBox )->setCurrentIndex( mPageSizeComboBox->count() - 1 );
  mPageOrientationComboBox->setEnabled( false );
}

void QgsLayoutPagePropertiesWidget::symbolChanged()
{
  mPage->layout()->undoStack()->beginCommand( mPage->layout()->pageCollection(), tr( "Change Page Background" ), QgsLayoutItemPage::UndoPageSymbol );
  mPage->layout()->pageCollection()->setPageStyleSymbol( static_cast< QgsFillSymbol * >( mSymbolButton->symbol() )->clone() );
  mPage->layout()->undoStack()->endCommand();
}

void QgsLayoutPagePropertiesWidget::showCurrentPageSize()
{
  QgsLayoutSize paperSize = mPage->pageSize();
  QString pageSize = QgsApplication::pageSizeRegistry()->find( paperSize );
  if ( !pageSize.isEmpty() )
  {
    mPageSizeComboBox->setCurrentIndex( mPageSizeComboBox->findData( pageSize ) );
    mLockAspectRatio->setEnabled( false );
    mLockAspectRatio->setLocked( false );
    mSizeUnitsComboBox->setEnabled( false );
    mPageOrientationComboBox->setEnabled( true );
  }
  else
  {
    // custom
    mPageSizeComboBox->setCurrentIndex( mPageSizeComboBox->count() - 1 );
    mLockAspectRatio->setEnabled( true );
    mSizeUnitsComboBox->setEnabled( true );
    mPageOrientationComboBox->setEnabled( false );
  }
}
