/***************************************************************************
    qgsrasterlabelsettingswidget.cpp
    ---------------------------
    begin                : December 2024
    copyright            : (C) 2024 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsrasterlabelsettingswidget.h"
#include "moc_qgsrasterlabelsettingswidget.cpp"
#include "qgsrasterbandcombobox.h"
#include "qgsrasterlabeling.h"
#include "qgsnumericformatselectorwidget.h"
#include "qgsbasicnumericformat.h"

#include <QPushButton>

QgsRasterLabelSettingsWidget::QgsRasterLabelSettingsWidget( QgsRasterLayer *layer, QgsMapCanvas *mapCanvas, QWidget *parent )
  : QgsLabelingGui( mapCanvas, parent, layer )
  , mNumberFormat( std::make_unique<QgsBasicNumericFormat>() )
{
  mGeomType = Qgis::GeometryType::Point;
  mMode = Labels;

  init();

  QWidget *labelWithWidget = new QWidget();
  QGridLayout *gLayout = new QGridLayout();
  gLayout->setContentsMargins( 0, 0, 0, 0 );
  gLayout->addWidget( new QLabel( tr( "Value" ) ), 0, 0 );
  mBandCombo = new QgsRasterBandComboBox();
  mBandCombo->setLayer( layer );
  gLayout->addWidget( mBandCombo, 0, 1 );
  gLayout->setColumnStretch( 0, 1 );
  gLayout->setColumnStretch( 1, 2 );

  gLayout->addWidget( new QLabel( tr( "Number format" ) ), 1, 0 );

  QPushButton *numberFormatButton = new QPushButton( tr( "Customize" ) );
  connect( numberFormatButton, &QPushButton::clicked, this, &QgsRasterLabelSettingsWidget::changeNumberFormat );

  gLayout->addWidget( numberFormatButton, 1, 1 );

  gLayout->addWidget( new QLabel( tr( "Resample over" ) ), 2, 0 );
  mResampleOverSpin = new QgsSpinBox();
  mResampleOverSpin->setMinimum( 1 );
  mResampleOverSpin->setMaximum( 128 );
  mResampleOverSpin->setClearValue( 1 );
  mResampleOverSpin->setSuffix( tr( " pixels" ) );
  connect( mResampleOverSpin, qOverload<int>( &QgsSpinBox::valueChanged ), this, &QgsRasterLabelSettingsWidget::widgetChanged );
  gLayout->addWidget( mResampleOverSpin, 2, 1 );

  gLayout->addWidget( new QLabel( tr( "Resample using" ) ), 3, 0 );
  mResampleMethodComboBox = new QComboBox();
  mResampleMethodComboBox->addItem( QObject::tr( "Nearest Neighbour" ), QVariant::fromValue( Qgis::RasterResamplingMethod::Nearest ) );
  mResampleMethodComboBox->addItem( QObject::tr( "Bilinear (2x2 Kernel)" ), QVariant::fromValue( Qgis::RasterResamplingMethod::Bilinear ) );
  mResampleMethodComboBox->addItem( QObject::tr( "Cubic (4x4 Kernel)" ), QVariant::fromValue( Qgis::RasterResamplingMethod::Cubic ) );
  mResampleMethodComboBox->addItem( QObject::tr( "Cubic B-Spline (4x4 Kernel)" ), QVariant::fromValue( Qgis::RasterResamplingMethod::CubicSpline ) );
  mResampleMethodComboBox->addItem( QObject::tr( "Lanczos (6x6 Kernel)" ), QVariant::fromValue( Qgis::RasterResamplingMethod::Lanczos ) );
  mResampleMethodComboBox->addItem( QObject::tr( "Average" ), QVariant::fromValue( Qgis::RasterResamplingMethod::Average ) );
  mResampleMethodComboBox->addItem( QObject::tr( "Mode" ), QVariant::fromValue( Qgis::RasterResamplingMethod::Mode ) );
  mResampleMethodComboBox->addItem( QObject::tr( "Gauss" ), QVariant::fromValue( Qgis::RasterResamplingMethod::Gauss ) );
  connect( mResampleMethodComboBox, qOverload<int>( &QComboBox::currentIndexChanged ), this, &QgsRasterLabelSettingsWidget::widgetChanged );
  gLayout->addWidget( mResampleMethodComboBox, 3, 1 );

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

  // hide settings which have no relevance to raster labeling
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
  mLabelEveryPartWidget->hide();
  mFramePixelSizeVisibility->hide();
  line->hide();

  mMinSizeFrame->show();
  mMinSizeLabel->setText( tr( "Suppress labeling of pixels smaller than" ) );

  mLimitLabelChkBox->setText( tr( "Limit number of pixels to be labeled to" ) );

  // fix precision for priority slider
  mPrioritySlider->setRange( 0, 100 );
  mPrioritySlider->setTickInterval( 10 );

  connect( mBandCombo, &QgsRasterBandComboBox::bandChanged, this, &QgsRasterLabelSettingsWidget::widgetChanged );
}

QgsRasterLabelSettingsWidget::~QgsRasterLabelSettingsWidget() = default;

void QgsRasterLabelSettingsWidget::setLabeling( QgsAbstractRasterLayerLabeling *labeling )
{
  if ( QgsRasterLayerSimpleLabeling *simpleLabeling = dynamic_cast<QgsRasterLayerSimpleLabeling *>( labeling ) )
  {
    setFormat( simpleLabeling->textFormat() );
    mBandCombo->setBand( simpleLabeling->band() );
    mPrioritySlider->setValue( static_cast<int>( 100 - simpleLabeling->priority() * 100 ) );

    mComboOverlapHandling->setCurrentIndex( mComboOverlapHandling->findData( static_cast<int>( simpleLabeling->placementSettings().overlapHandling() ) ) );
    mZIndexSpinBox->setValue( simpleLabeling->zIndex() );
    if ( const QgsNumericFormat *format = simpleLabeling->numericFormat() )
      mNumberFormat.reset( format->clone() );

    mLimitLabelChkBox->setChecked( simpleLabeling->thinningSettings().limitNumberOfLabelsEnabled() );
    mLimitLabelSpinBox->setValue( simpleLabeling->thinningSettings().maximumNumberLabels() );
    mMinSizeSpinBox->setValue( simpleLabeling->thinningSettings().minimumFeatureSize() );

    mScaleBasedVisibilityChkBx->setChecked( simpleLabeling->hasScaleBasedVisibility() );
    mMinScaleWidget->setScale( simpleLabeling->minimumScale() );
    mMaxScaleWidget->setScale( simpleLabeling->maximumScale() );

    mResampleOverSpin->setValue( simpleLabeling->resampleOver() );
    mResampleMethodComboBox->setCurrentIndex( mResampleMethodComboBox->findData( QVariant::fromValue( simpleLabeling->resampleMethod() ) ) );

    updateUi();
  }
}

void QgsRasterLabelSettingsWidget::updateLabeling( QgsAbstractRasterLayerLabeling *labeling )
{
  if ( QgsRasterLayerSimpleLabeling *simpleLabeling = dynamic_cast<QgsRasterLayerSimpleLabeling *>( labeling ) )
  {
    simpleLabeling->setTextFormat( format() );
    simpleLabeling->setBand( mBandCombo->currentBand() );
    simpleLabeling->setPriority( 1.0 - mPrioritySlider->value() / 100.0 );
    simpleLabeling->placementSettings().setOverlapHandling( static_cast<Qgis::LabelOverlapHandling>( mComboOverlapHandling->currentData().toInt() ) );
    simpleLabeling->setZIndex( mZIndexSpinBox->value() );
    simpleLabeling->setNumericFormat( mNumberFormat->clone() );

    simpleLabeling->thinningSettings().setLimitNumberLabelsEnabled( mLimitLabelChkBox->isChecked() );
    simpleLabeling->thinningSettings().setMaximumNumberLabels( mLimitLabelSpinBox->value() );
    simpleLabeling->thinningSettings().setMinimumFeatureSize( mMinSizeSpinBox->value() );

    simpleLabeling->setScaleBasedVisibility( mScaleBasedVisibilityChkBx->isChecked() );
    simpleLabeling->setMinimumScale( mMinScaleWidget->scale() );
    simpleLabeling->setMaximumScale( mMaxScaleWidget->scale() );

    simpleLabeling->setResampleOver( mResampleOverSpin->value() );
    simpleLabeling->setResampleMethod( mResampleMethodComboBox->currentData().value<Qgis::RasterResamplingMethod>() );
  }
}

void QgsRasterLabelSettingsWidget::setLayer( QgsMapLayer *layer )
{
  if ( mBandCombo )
    mBandCombo->setLayer( layer );

  QgsLabelingGui::setLayer( layer );
  mMinSizeFrame->show();
}

void QgsRasterLabelSettingsWidget::changeNumberFormat()
{
  QgsPanelWidget *panel = QgsPanelWidget::findParentPanel( this );
  if ( panel && panel->dockMode() )
  {
    QgsNumericFormatSelectorWidget *widget = new QgsNumericFormatSelectorWidget( this );
    widget->setPanelTitle( tr( "Number Format" ) );
    widget->setFormat( mNumberFormat.get() );
    widget->registerExpressionContextGenerator( this );
    connect( widget, &QgsNumericFormatSelectorWidget::changed, this, [=] {
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
