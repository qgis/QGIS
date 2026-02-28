/***************************************************************************
    qgsrastercontourlabelsettingswidget.cpp
    ---------------------------
    begin                : February 2026
    copyright            : (C) 2026 by the QGIS project
    email                : info at qgis dot org
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsrastercontourlabelsettingswidget.h"

#include "qgsbasicnumericformat.h"
#include "qgsnumericformatselectorwidget.h"
#include "qgsrastercontourlabeling.h"
#include "qgsrasterlayer.h"

#include <QCheckBox>
#include <QLabel>
#include <QPushButton>

#include "moc_qgsrastercontourlabelsettingswidget.cpp"

QgsRasterContourLabelSettingsWidget::QgsRasterContourLabelSettingsWidget( QgsRasterLayer *layer, QgsMapCanvas *mapCanvas, QWidget *parent )
  : QgsLabelingGui( mapCanvas, parent, layer )
  , mNumberFormat( std::make_unique<QgsBasicNumericFormat>() )
{
  mGeomType = Qgis::GeometryType::Line;
  mMode = Labels;

  init();

  QWidget *labelWithWidget = new QWidget();
  QGridLayout *gLayout = new QGridLayout();
  gLayout->setContentsMargins( 0, 0, 0, 0 );

  // Row 0: Number format
  gLayout->addWidget( new QLabel( tr( "Number format" ) ), 0, 0 );
  QPushButton *numberFormatButton = new QPushButton( tr( "Customize" ) );
  connect( numberFormatButton, &QPushButton::clicked, this, &QgsRasterContourLabelSettingsWidget::changeNumberFormat );
  gLayout->addWidget( numberFormatButton, 0, 1 );

  // Row 1: Label index contours only
  mLabelIndexOnlyCheck = new QCheckBox( tr( "Label index contours only" ) );
  connect( mLabelIndexOnlyCheck, &QCheckBox::toggled, this, &QgsRasterContourLabelSettingsWidget::widgetChanged );
  gLayout->addWidget( mLabelIndexOnlyCheck, 1, 0, 1, 2 );

  gLayout->setColumnStretch( 0, 1 );
  gLayout->setColumnStretch( 1, 2 );

  labelWithWidget->setLayout( gLayout );

  mStackedWidgetLabelWith->addWidget( labelWithWidget );
  mStackedWidgetLabelWith->setCurrentWidget( labelWithWidget );

  setLayer( layer );

  const int prevIndex = mOptionsTab->currentIndex();

  setPropertyOverrideButtonsVisible( true );
  mTextFormatsListWidget->setEntityTypes( QList<QgsStyle::StyleEntity>() << QgsStyle::TextFormatEntity );

  delete mCalloutItem;
  mCalloutItem = nullptr;
  delete mMaskItem;
  mMaskItem = nullptr;

  mOptionsTab->removeTab( mOptionsTab->indexOf( calloutsTab ) );
  mOptionsTab->removeTab( mOptionsTab->indexOf( maskTab ) );

  mLabelStackedWidget->removeWidget( mLabelPage_Callouts );
  mLabelStackedWidget->removeWidget( mLabelPage_Mask );

  switch ( prevIndex )
  {
    case 0:
    case 1:
    case 2:
      break;

    case 4: // background - account for removed mask tab
    case 5: // shadow
      mLabelStackedWidget->setCurrentIndex( prevIndex - 1 );
      mOptionsTab->setCurrentIndex( prevIndex - 1 );
      break;

    case 7: // background - account for removed mask & callouts tab
    case 8: // shadow- account for removed mask & callouts tab
      mLabelStackedWidget->setCurrentIndex( prevIndex - 2 );
      mOptionsTab->setCurrentIndex( prevIndex - 2 );
      break;

    case 3: // mask
    case 6: // callouts
      mLabelStackedWidget->setCurrentIndex( 0 );
      mOptionsTab->setCurrentIndex( 0 );
      break;

    default:
      break;
  }

  // hide settings which have no relevance to raster contour labeling
  mDirectSymbolsFrame->hide();
  mFormatNumFrame->hide();
  mFormatNumChkBx->hide();
  mFormatNumDDBtn->hide();
  mCheckBoxSubstituteText->hide();
  mToolButtonConfigureSubstitutes->hide();
  mLabelWrapOnCharacter->hide();
  wrapCharacterEdit->hide();
  mWrapCharDDBtn->hide();
  mLabelWrapLinesTo->hide();
  mAutoWrapLengthSpinBox->hide();
  mAutoWrapLengthDDBtn->hide();
  mAutoWrapTypeComboBox->hide();
  mFontMultiLineLabel->hide();
  mFontMultiLineAlignComboBox->hide();
  mFontMultiLineAlignDDBtn->hide();
  mGeometryGeneratorGroupBox->hide();
  mObstaclesGroupBox->hide();
  mPlacementDDGroupBox->hide();
  mPlacementGroupBox->hide();
  mInferiorPlacementWidget->hide();
  mLabelRenderingDDFrame->hide();
  mUpsidedownFrame->hide();
  mMultipartBehaviorWidget->hide();
  mFramePixelSizeVisibility->hide();
  line->hide();

  mMinSizeFrame->show();
  mMinSizeLabel->setText( tr( "Suppress labeling of contours shorter than" ) );

  mLimitLabelChkBox->setText( tr( "Limit number of contours to be labeled to" ) );

  // fix precision for priority slider
  mPrioritySlider->setRange( 0, 100 );
  mPrioritySlider->setTickInterval( 10 );

}

QgsRasterContourLabelSettingsWidget::~QgsRasterContourLabelSettingsWidget() = default;

void QgsRasterContourLabelSettingsWidget::setLabeling( QgsAbstractRasterLayerLabeling *labeling )
{
  if ( QgsRasterLayerContourLabeling *contourLabeling = dynamic_cast<QgsRasterLayerContourLabeling *>( labeling ) )
  {
    setFormat( contourLabeling->textFormat() );
    mLabelIndexOnlyCheck->setChecked( contourLabeling->labelIndexOnly() );
    mPrioritySlider->setValue( static_cast<int>( 100 - contourLabeling->priority() * 100 ) );

    mComboOverlapHandling->setCurrentIndex( mComboOverlapHandling->findData( static_cast<int>( contourLabeling->placementSettings().overlapHandling() ) ) );
    mZIndexSpinBox->setValue( contourLabeling->zIndex() );
    if ( const QgsNumericFormat *format = contourLabeling->numericFormat() )
      mNumberFormat.reset( format->clone() );

    mLimitLabelChkBox->setChecked( contourLabeling->thinningSettings().limitNumberOfLabelsEnabled() );
    mLimitLabelSpinBox->setValue( contourLabeling->thinningSettings().maximumNumberLabels() );
    mMinSizeSpinBox->setValue( contourLabeling->thinningSettings().minimumFeatureSize() );

    mScaleBasedVisibilityChkBx->setChecked( contourLabeling->hasScaleBasedVisibility() );
    mMinScaleWidget->setScale( contourLabeling->minimumScale() );
    mMaxScaleWidget->setScale( contourLabeling->maximumScale() );

    updateUi();
  }
}

void QgsRasterContourLabelSettingsWidget::updateLabeling( QgsAbstractRasterLayerLabeling *labeling )
{
  if ( QgsRasterLayerContourLabeling *contourLabeling = dynamic_cast<QgsRasterLayerContourLabeling *>( labeling ) )
  {
    contourLabeling->setTextFormat( format() );
    contourLabeling->setLabelIndexOnly( mLabelIndexOnlyCheck->isChecked() );
    contourLabeling->setPriority( 1.0 - mPrioritySlider->value() / 100.0 );
    contourLabeling->placementSettings().setOverlapHandling( static_cast<Qgis::LabelOverlapHandling>( mComboOverlapHandling->currentData().toInt() ) );
    contourLabeling->setZIndex( mZIndexSpinBox->value() );
    contourLabeling->setNumericFormat( mNumberFormat->clone() );

    contourLabeling->thinningSettings().setLimitNumberLabelsEnabled( mLimitLabelChkBox->isChecked() );
    contourLabeling->thinningSettings().setMaximumNumberLabels( mLimitLabelSpinBox->value() );
    contourLabeling->thinningSettings().setMinimumFeatureSize( mMinSizeSpinBox->value() );

    contourLabeling->setScaleBasedVisibility( mScaleBasedVisibilityChkBx->isChecked() );
    contourLabeling->setMinimumScale( mMinScaleWidget->scale() );
    contourLabeling->setMaximumScale( mMaxScaleWidget->scale() );
  }
}

void QgsRasterContourLabelSettingsWidget::setLayer( QgsMapLayer *layer )
{
  QgsLabelingGui::setLayer( layer );
  mMinSizeFrame->show();
}

void QgsRasterContourLabelSettingsWidget::changeNumberFormat()
{
  QgsPanelWidget *panel = QgsPanelWidget::findParentPanel( this );
  if ( panel && panel->dockMode() )
  {
    QgsNumericFormatSelectorWidget *widget = new QgsNumericFormatSelectorWidget( this );
    widget->setPanelTitle( tr( "Number Format" ) );
    widget->setFormat( mNumberFormat.get() );
    widget->registerExpressionContextGenerator( this );
    connect( widget, &QgsNumericFormatSelectorWidget::changed, this, [this, widget] {
      if ( !mBlockChangesSignal )
      {
        mNumberFormat.reset( widget->format() );
        emit widgetChanged();
      }
    } );
    panel->openPanel( widget );
  }
  else
  {
    QgsNumericFormatSelectorDialog dialog( this );
    dialog.setFormat( mNumberFormat.get() );
    dialog.registerExpressionContextGenerator( this );
    if ( dialog.exec() )
    {
      mNumberFormat.reset( dialog.format() );
      emit widgetChanged();
    }
  }
}
