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
#include "moc_qgslayoutpagepropertieswidget.cpp"
#include "qgsapplication.h"
#include "qgspagesizeregistry.h"
#include "qgslayoutitempage.h"
#include "qgslayout.h"
#include "qgslayoutpagecollection.h"
#include "qgslayoutundostack.h"
#include "qgsvectorlayer.h"
#include "qgsfillsymbol.h"
#include "qgslayoutrendercontext.h"
#include "qgslayoutreportcontext.h"

QgsLayoutPagePropertiesWidget::QgsLayoutPagePropertiesWidget( QWidget *parent, QgsLayoutItem *layoutItem )
  : QgsLayoutItemBaseWidget( parent, layoutItem )
  , mPage( static_cast<QgsLayoutItemPage *>( layoutItem ) )
{
  setupUi( this );

  mPageOrientationComboBox->addItem( tr( "Portrait" ), QgsLayoutItemPage::Portrait );
  mPageOrientationComboBox->addItem( tr( "Landscape" ), QgsLayoutItemPage::Landscape );

  const auto constEntries = QgsApplication::pageSizeRegistry()->entries();
  for ( const QgsPageSize &size : constEntries )
  {
    mPageSizeComboBox->addItem( size.displayName, size.name );
  }
  mPageSizeComboBox->addItem( tr( "Custom" ) );

  mWidthSpin->setValue( mPage->pageSize().width() );
  mHeightSpin->setValue( mPage->pageSize().height() );
  mSizeUnitsComboBox->setUnit( mPage->pageSize().units() );
  mExcludePageCheckBox->setChecked( mPage->excludeFromExports() );

  mPageOrientationComboBox->setCurrentIndex( mPageOrientationComboBox->findData( mPage->orientation() ) );

  mSizeUnitsComboBox->linkToWidget( mWidthSpin );
  mSizeUnitsComboBox->linkToWidget( mHeightSpin );
  mSizeUnitsComboBox->setConverter( &mPage->layout()->renderContext().measurementConverter() );

  mLockAspectRatio->setWidthSpinBox( mWidthSpin );
  mLockAspectRatio->setHeightSpinBox( mHeightSpin );

  mSymbolButton->setSymbolType( Qgis::SymbolType::Fill );
  mSymbolButton->setSymbol( mPage->pageStyleSymbol()->clone() );

  connect( mPageSizeComboBox, static_cast<void ( QComboBox::* )( int )>( &QComboBox::currentIndexChanged ), this, &QgsLayoutPagePropertiesWidget::pageSizeChanged );
  connect( mPageOrientationComboBox, static_cast<void ( QComboBox::* )( int )>( &QComboBox::currentIndexChanged ), this, &QgsLayoutPagePropertiesWidget::orientationChanged );

  connect( mWidthSpin, static_cast<void ( QDoubleSpinBox::* )( double )>( &QDoubleSpinBox::valueChanged ), this, &QgsLayoutPagePropertiesWidget::updatePageSize );
  connect( mHeightSpin, static_cast<void ( QDoubleSpinBox::* )( double )>( &QDoubleSpinBox::valueChanged ), this, &QgsLayoutPagePropertiesWidget::updatePageSize );
  connect( mWidthSpin, static_cast<void ( QDoubleSpinBox::* )( double )>( &QDoubleSpinBox::valueChanged ), this, &QgsLayoutPagePropertiesWidget::setToCustomSize );
  connect( mHeightSpin, static_cast<void ( QDoubleSpinBox::* )( double )>( &QDoubleSpinBox::valueChanged ), this, &QgsLayoutPagePropertiesWidget::setToCustomSize );
  connect( mExcludePageCheckBox, &QCheckBox::toggled, this, &QgsLayoutPagePropertiesWidget::excludeExportsToggled );

  connect( mSymbolButton, &QgsSymbolButton::changed, this, &QgsLayoutPagePropertiesWidget::symbolChanged );
  registerDataDefinedButton( mPaperSizeDDBtn, QgsLayoutObject::DataDefinedProperty::PresetPaperSize );
  registerDataDefinedButton( mWidthDDBtn, QgsLayoutObject::DataDefinedProperty::ItemWidth );
  registerDataDefinedButton( mHeightDDBtn, QgsLayoutObject::DataDefinedProperty::ItemHeight );
  registerDataDefinedButton( mOrientationDDBtn, QgsLayoutObject::DataDefinedProperty::PaperOrientation );
  registerDataDefinedButton( mExcludePageDDBtn, QgsLayoutObject::DataDefinedProperty::ExcludeFromExports );

  connect( mPaperSizeDDBtn, &QgsPropertyOverrideButton::changed, this, &QgsLayoutPagePropertiesWidget::refreshLayout );
  connect( mWidthDDBtn, &QgsPropertyOverrideButton::changed, this, &QgsLayoutPagePropertiesWidget::refreshLayout );
  connect( mHeightDDBtn, &QgsPropertyOverrideButton::changed, this, &QgsLayoutPagePropertiesWidget::refreshLayout );
  connect( mOrientationDDBtn, &QgsPropertyOverrideButton::changed, this, &QgsLayoutPagePropertiesWidget::refreshLayout );

  mExcludePageDDBtn->registerEnabledWidget( mExcludePageCheckBox, false );

  mSymbolButton->registerExpressionContextGenerator( mPage );
  mSymbolButton->setLayer( coverageLayer() );

  connect( mApplyToAllButton, &QPushButton::clicked, this, &QgsLayoutPagePropertiesWidget::applyToAll );

  if ( mPage->layout() )
  {
    connect( &mPage->layout()->reportContext(), &QgsLayoutReportContext::layerChanged, mSymbolButton, &QgsSymbolButton::setLayer );

    QgsLayoutPageCollection *pages = mPage->layout()->pageCollection();
    const bool multiPage = pages->pageCount() > 1;
    if ( multiPage )
    {
      const int pageNumber = mPage->layout()->pageCollection()->pageNumber( mPage );
      mTitleLabel->setText( tr( "Page (%1/%2)" ).arg( pageNumber + 1 ).arg( pages->pageCount() ) );
    }
    else
    {
      mTitleLabel->setText( tr( "Page" ) );
    }
    mApplyToAllButton->setVisible( multiPage );
  }

  showCurrentPageSize();
}

void QgsLayoutPagePropertiesWidget::pageSizeChanged( int )
{
  mBlockPageUpdate = true;
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
  mBlockPageUpdate = false;
  updatePageSize();
}

void QgsLayoutPagePropertiesWidget::orientationChanged( int )
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

  updatePageSize();
}

void QgsLayoutPagePropertiesWidget::updatePageSize()
{
  if ( mBlockPageUpdate )
    return;

  mPage->layout()->undoStack()->beginMacro( tr( "Change Page Size" ) );
  mPage->layout()->pageCollection()->beginPageSizeChange();
  mPage->layout()->undoStack()->beginCommand( mPage, tr( "Change Page Size" ), 1 + mPage->layout()->pageCollection()->pageNumber( mPage ) );
  mPage->setPageSize( QgsLayoutSize( mWidthSpin->value(), mHeightSpin->value(), mSizeUnitsComboBox->unit() ) );
  mPage->layout()->undoStack()->endCommand();
  mPage->layout()->pageCollection()->reflow();
  mPage->layout()->pageCollection()->endPageSizeChange();
  mPage->layout()->undoStack()->endMacro();

  refreshLayout();
  emit pageOrientationChanged();
}

void QgsLayoutPagePropertiesWidget::setToCustomSize()
{
  if ( mSettingPresetSize )
    return;
  whileBlocking( mPageSizeComboBox )->setCurrentIndex( mPageSizeComboBox->count() - 1 );
  mPageOrientationComboBox->setEnabled( false );
  pageSizeChanged( mPageSizeComboBox->currentIndex() );
}

void QgsLayoutPagePropertiesWidget::symbolChanged()
{
  mPage->layout()->undoStack()->beginCommand( mPage->layout()->pageCollection(), tr( "Change Page Background" ), QgsLayoutItemPage::UndoPageSymbol );
  mPage->setPageStyleSymbol( static_cast<QgsFillSymbol *>( mSymbolButton->symbol() )->clone() );
  mPage->layout()->undoStack()->endCommand();
}

void QgsLayoutPagePropertiesWidget::excludeExportsToggled( bool checked )
{
  mPage->beginCommand( !checked ? tr( "Include Page in Exports" ) : tr( "Exclude Page from Exports" ) );
  mPage->setExcludeFromExports( checked );
  mPage->endCommand();
}

void QgsLayoutPagePropertiesWidget::refreshLayout()
{
  mPage->layout()->refresh();
}

void QgsLayoutPagePropertiesWidget::showCurrentPageSize()
{
  const QgsLayoutSize paperSize = mPage->pageSize();
  const QString pageSize = QgsApplication::pageSizeRegistry()->find( paperSize );
  if ( !pageSize.isEmpty() )
  {
    whileBlocking( mPageSizeComboBox )->setCurrentIndex( mPageSizeComboBox->findData( pageSize ) );
    mLockAspectRatio->setEnabled( false );
    mLockAspectRatio->setLocked( false );
    mSizeUnitsComboBox->setEnabled( false );
    mPageOrientationComboBox->setEnabled( true );
  }
  else
  {
    // custom
    whileBlocking( mPageSizeComboBox )->setCurrentIndex( mPageSizeComboBox->count() - 1 );
    mLockAspectRatio->setEnabled( true );
    mSizeUnitsComboBox->setEnabled( true );
    mPageOrientationComboBox->setEnabled( false );
  }
}

void QgsLayoutPagePropertiesWidget::applyToAll()
{
  QgsLayoutPageCollection *pages = mPage->layout()->pageCollection();
  pages->applyPropertiesToAllOtherPages( pages->pageNumber( mPage ) );
}
