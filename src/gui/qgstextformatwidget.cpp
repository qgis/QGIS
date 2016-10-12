/***************************************************************************
    qgstextformatwidget.h
    ---------------------
    begin                : June 2009
    copyright            : (C) Martin Dobias
    email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgstextformatwidget.h"
#include "qgsmapcanvas.h"
#include "qgscharacterselectdialog.h"
#include "qgslogger.h"
#include "qgsfontutils.h"
#include "qgssymbollayerutils.h"
#include "qgssvgcache.h"
#include "qgssvgselectorwidget.h"
#include "qgssubstitutionlistwidget.h"
#include "qgspallabeling.h" // for enum values
#include <QSettings>

QgsTextFormatWidget::QgsTextFormatWidget( const QgsTextFormat& format, QgsMapCanvas* mapCanvas, QWidget* parent )
    : QWidget( parent )
    , mQuadrantBtnGrp( nullptr )
    , mDirectSymbBtnGrp( nullptr )
    , mUpsidedownBtnGrp( nullptr )
    , mPlacePointBtnGrp( nullptr )
    , mPlaceLineBtnGrp( nullptr )
    , mPlacePolygonBtnGrp( nullptr )
    , mMinPixelLimit( 0 )
    , mWidgetMode( Text )
    , mMapCanvas( mapCanvas )
    , mCharDlg( nullptr )
    , mLoadSvgParams( false )
{
  initWidget();
  setWidgetMode( Text );
  updateWidgetForFormat( format );
}

QgsTextFormatWidget::QgsTextFormatWidget( QgsMapCanvas* mapCanvas, QWidget* parent, Mode mode )
    : QWidget( parent )
    , mQuadrantBtnGrp( nullptr )
    , mDirectSymbBtnGrp( nullptr )
    , mUpsidedownBtnGrp( nullptr )
    , mPlacePointBtnGrp( nullptr )
    , mPlaceLineBtnGrp( nullptr )
    , mPlacePolygonBtnGrp( nullptr )
    , mMinPixelLimit( 0 )
    , mWidgetMode( mode )
    , mMapCanvas( mapCanvas )
    , mCharDlg( nullptr )
    , mLoadSvgParams( false )
{
  initWidget();
  setWidgetMode( mode );
}

void QgsTextFormatWidget::initWidget()
{
  setupUi( this );

  mPreviewScaleComboBox->setMapCanvas( mMapCanvas );
  mPreviewScaleComboBox->setShowCurrentScaleButton( true );
  connect( mPreviewScaleComboBox, SIGNAL( scaleChanged( double ) ), this, SLOT( previewScaleChanged( double ) ) );

  Q_FOREACH ( QgsUnitSelectionWidget* unitWidget, findChildren<QgsUnitSelectionWidget*>() )
  {
    unitWidget->setMapCanvas( mMapCanvas );
  }
  mFontSizeUnitWidget->setUnits( QgsUnitTypes::RenderUnitList() << QgsUnitTypes::RenderPoints << QgsUnitTypes::RenderMapUnits
                                 << QgsUnitTypes::RenderMillimeters << QgsUnitTypes::RenderPixels );
  mBufferUnitWidget->setUnits( QgsUnitTypes::RenderUnitList() << QgsUnitTypes::RenderMillimeters << QgsUnitTypes::RenderMapUnits << QgsUnitTypes::RenderPixels );
  mShapeSizeUnitWidget->setUnits( QgsUnitTypes::RenderUnitList() << QgsUnitTypes::RenderMillimeters << QgsUnitTypes::RenderMapUnits << QgsUnitTypes::RenderPixels );
  mShapeOffsetUnitWidget->setUnits( QgsUnitTypes::RenderUnitList() << QgsUnitTypes::RenderMillimeters << QgsUnitTypes::RenderMapUnits << QgsUnitTypes::RenderPixels );
  mShapeRadiusUnitWidget->setUnits( QgsUnitTypes::RenderUnitList() << QgsUnitTypes::RenderMillimeters << QgsUnitTypes::RenderMapUnits
                                    << QgsUnitTypes::RenderPixels << QgsUnitTypes::RenderPercentage );
  mShapeBorderWidthUnitWidget->setUnits( QgsUnitTypes::RenderUnitList() << QgsUnitTypes::RenderMillimeters << QgsUnitTypes::RenderMapUnits << QgsUnitTypes::RenderPixels );
  mShadowOffsetUnitWidget->setUnits( QgsUnitTypes::RenderUnitList() << QgsUnitTypes::RenderMillimeters << QgsUnitTypes::RenderMapUnits << QgsUnitTypes::RenderPixels );
  mShadowRadiusUnitWidget->setUnits( QgsUnitTypes::RenderUnitList() << QgsUnitTypes::RenderMillimeters << QgsUnitTypes::RenderMapUnits << QgsUnitTypes::RenderPixels );
  mPointOffsetUnitWidget->setUnits( QgsUnitTypes::RenderUnitList() << QgsUnitTypes::RenderMillimeters << QgsUnitTypes::RenderMapUnits << QgsUnitTypes::RenderPixels );
  mLineDistanceUnitWidget->setUnits( QgsUnitTypes::RenderUnitList() << QgsUnitTypes::RenderMillimeters << QgsUnitTypes::RenderMapUnits << QgsUnitTypes::RenderPixels );
  mRepeatDistanceUnitWidget->setUnits( QgsUnitTypes::RenderUnitList() << QgsUnitTypes::RenderMillimeters << QgsUnitTypes::RenderMapUnits << QgsUnitTypes::RenderPixels );

  mFontLineHeightSpinBox->setClearValue( 1.0 );
  mShapeRotationDblSpnBx->setClearValue( 0.0 );
  mShapeOffsetXSpnBx->setClearValue( 0.0 );
  mShapeOffsetYSpnBx->setClearValue( 0.0 );
  mPointOffsetXSpinBox->setClearValue( 0.0 );
  mPointOffsetYSpinBox->setClearValue( 0.0 );
  mPointAngleSpinBox->setClearValue( 0.0 );
  mFontLetterSpacingSpinBox->setClearValue( 0.0 );
  mFontWordSpacingSpinBox->setClearValue( 0.0 );
  mZIndexSpinBox->setClearValue( 0.0 );

  mObstacleTypeComboBox->addItem( tr( "Over the feature's interior" ), QgsPalLayerSettings::PolygonInterior );
  mObstacleTypeComboBox->addItem( tr( "Over the feature's boundary" ), QgsPalLayerSettings::PolygonBoundary );

  mOffsetTypeComboBox->addItem( tr( "From point" ), QgsPalLayerSettings::FromPoint );
  mOffsetTypeComboBox->addItem( tr( "From symbol bounds" ), QgsPalLayerSettings::FromSymbolBounds );

  mCharDlg = new QgsCharacterSelectorDialog( this );

  mRefFont = lblFontPreview->font();

  // internal connections
  connect( mFontTranspSlider, SIGNAL( valueChanged( int ) ), mFontTranspSpinBox, SLOT( setValue( int ) ) );
  connect( mFontTranspSpinBox, SIGNAL( valueChanged( int ) ), mFontTranspSlider, SLOT( setValue( int ) ) );
  connect( mBufferTranspSlider, SIGNAL( valueChanged( int ) ), mBufferTranspSpinBox, SLOT( setValue( int ) ) );
  connect( mBufferTranspSpinBox, SIGNAL( valueChanged( int ) ), mBufferTranspSlider, SLOT( setValue( int ) ) );
  connect( mShapeTranspSlider, SIGNAL( valueChanged( int ) ), mShapeTranspSpinBox, SLOT( setValue( int ) ) );
  connect( mShapeTranspSpinBox, SIGNAL( valueChanged( int ) ), mShapeTranspSlider, SLOT( setValue( int ) ) );
  connect( mShadowOffsetAngleDial, SIGNAL( valueChanged( int ) ), mShadowOffsetAngleSpnBx, SLOT( setValue( int ) ) );
  connect( mShadowOffsetAngleSpnBx, SIGNAL( valueChanged( int ) ), mShadowOffsetAngleDial, SLOT( setValue( int ) ) );
  connect( mShadowTranspSlider, SIGNAL( valueChanged( int ) ), mShadowTranspSpnBx, SLOT( setValue( int ) ) );
  connect( mShadowTranspSpnBx, SIGNAL( valueChanged( int ) ), mShadowTranspSlider, SLOT( setValue( int ) ) );
  connect( mLimitLabelChkBox, SIGNAL( toggled( bool ) ), mLimitLabelSpinBox, SLOT( setEnabled( bool ) ) );
  connect( mCheckBoxSubstituteText, SIGNAL( toggled( bool ) ), mToolButtonConfigureSubstitutes, SLOT( setEnabled( bool ) ) );

  //connections to prevent users removing all line placement positions
  connect( chkLineAbove, SIGNAL( toggled( bool ) ), this, SLOT( updateLinePlacementOptions() ) );
  connect( chkLineBelow, SIGNAL( toggled( bool ) ), this, SLOT( updateLinePlacementOptions() ) );
  connect( chkLineOn, SIGNAL( toggled( bool ) ), this, SLOT( updateLinePlacementOptions() ) );

  populateFontCapitalsComboBox();

  // color buttons
  mPreviewBackgroundBtn->setColorDialogTitle( tr( "Select fill color" ) );
  mPreviewBackgroundBtn->setContext( "labelling" );
  mPreviewBackgroundBtn->setColor( QColor( 255, 255, 255 ) );
  btnTextColor->setColorDialogTitle( tr( "Select text color" ) );
  btnTextColor->setContext( "labelling" );
  btnTextColor->setDefaultColor( Qt::black );
  btnBufferColor->setColorDialogTitle( tr( "Select buffer color" ) );
  btnBufferColor->setContext( "labelling" );
  btnBufferColor->setDefaultColor( Qt::white );
  mShapeBorderColorBtn->setColorDialogTitle( tr( "Select border color" ) );
  mShapeBorderColorBtn->setContext( "labelling" );
  mShapeFillColorBtn->setColorDialogTitle( tr( "Select fill color" ) );
  mShapeFillColorBtn->setContext( "labelling" );
  mShadowColorBtn->setColorDialogTitle( tr( "Select shadow color" ) );
  mShadowColorBtn->setContext( "labelling" );
  mShadowColorBtn->setDefaultColor( Qt::black );

  // set up quadrant offset button group
  mQuadrantBtnGrp = new QButtonGroup( this );
  mQuadrantBtnGrp->addButton( mPointOffsetAboveLeft, ( int )QgsPalLayerSettings::QuadrantAboveLeft );
  mQuadrantBtnGrp->addButton( mPointOffsetAbove, ( int )QgsPalLayerSettings::QuadrantAbove );
  mQuadrantBtnGrp->addButton( mPointOffsetAboveRight, ( int )QgsPalLayerSettings::QuadrantAboveRight );
  mQuadrantBtnGrp->addButton( mPointOffsetLeft, ( int )QgsPalLayerSettings::QuadrantLeft );
  mQuadrantBtnGrp->addButton( mPointOffsetOver, ( int )QgsPalLayerSettings::QuadrantOver );
  mQuadrantBtnGrp->addButton( mPointOffsetRight, ( int )QgsPalLayerSettings::QuadrantRight );
  mQuadrantBtnGrp->addButton( mPointOffsetBelowLeft, ( int )QgsPalLayerSettings::QuadrantBelowLeft );
  mQuadrantBtnGrp->addButton( mPointOffsetBelow, ( int )QgsPalLayerSettings::QuadrantBelow );
  mQuadrantBtnGrp->addButton( mPointOffsetBelowRight, ( int )QgsPalLayerSettings::QuadrantBelowRight );
  mQuadrantBtnGrp->setExclusive( true );

  // setup direction symbol(s) button group
  mDirectSymbBtnGrp = new QButtonGroup( this );
  mDirectSymbBtnGrp->addButton( mDirectSymbRadioBtnLR, ( int )QgsPalLayerSettings::SymbolLeftRight );
  mDirectSymbBtnGrp->addButton( mDirectSymbRadioBtnAbove, ( int )QgsPalLayerSettings::SymbolAbove );
  mDirectSymbBtnGrp->addButton( mDirectSymbRadioBtnBelow, ( int )QgsPalLayerSettings::SymbolBelow );
  mDirectSymbBtnGrp->setExclusive( true );

  // upside-down labels button group
  mUpsidedownBtnGrp = new QButtonGroup( this );
  mUpsidedownBtnGrp->addButton( mUpsidedownRadioOff, ( int )QgsPalLayerSettings::Upright );
  mUpsidedownBtnGrp->addButton( mUpsidedownRadioDefined, ( int )QgsPalLayerSettings::ShowDefined );
  mUpsidedownBtnGrp->addButton( mUpsidedownRadioAll, ( int )QgsPalLayerSettings::ShowAll );
  mUpsidedownBtnGrp->setExclusive( true );

  //mShapeCollisionsChkBx->setVisible( false ); // until implemented

  // post updatePlacementWidgets() connections
  connect( chkLineAbove, SIGNAL( toggled( bool ) ), this, SLOT( updatePlacementWidgets() ) );
  connect( chkLineBelow, SIGNAL( toggled( bool ) ), this, SLOT( updatePlacementWidgets() ) );

  // setup point placement button group
  mPlacePointBtnGrp = new QButtonGroup( this );
  mPlacePointBtnGrp->addButton( radPredefinedOrder, ( int )QgsPalLayerSettings::OrderedPositionsAroundPoint );
  mPlacePointBtnGrp->addButton( radAroundPoint, ( int )QgsPalLayerSettings::AroundPoint );
  mPlacePointBtnGrp->addButton( radOverPoint, ( int )QgsPalLayerSettings::OverPoint );
  mPlacePointBtnGrp->setExclusive( true );
  connect( mPlacePointBtnGrp, SIGNAL( buttonClicked( int ) ), this, SLOT( updatePlacementWidgets() ) );

  // setup line placement button group (assigned enum id currently unused)
  mPlaceLineBtnGrp = new QButtonGroup( this );
  mPlaceLineBtnGrp->addButton( radLineParallel, ( int )QgsPalLayerSettings::Line );
  mPlaceLineBtnGrp->addButton( radLineCurved, ( int )QgsPalLayerSettings::Curved );
  mPlaceLineBtnGrp->addButton( radLineHorizontal, ( int )QgsPalLayerSettings::Horizontal );
  mPlaceLineBtnGrp->setExclusive( true );
  connect( mPlaceLineBtnGrp, SIGNAL( buttonClicked( int ) ), this, SLOT( updatePlacementWidgets() ) );

  // setup polygon placement button group (assigned enum id currently unused)
  mPlacePolygonBtnGrp = new QButtonGroup( this );
  mPlacePolygonBtnGrp->addButton( radOverCentroid, ( int )QgsPalLayerSettings::OverPoint );
  mPlacePolygonBtnGrp->addButton( radAroundCentroid, ( int )QgsPalLayerSettings::AroundPoint );
  mPlacePolygonBtnGrp->addButton( radPolygonHorizontal, ( int )QgsPalLayerSettings::Horizontal );
  mPlacePolygonBtnGrp->addButton( radPolygonFree, ( int )QgsPalLayerSettings::Free );
  mPlacePolygonBtnGrp->addButton( radPolygonPerimeter, ( int )QgsPalLayerSettings::Line );
  mPlacePolygonBtnGrp->addButton( radPolygonPerimeterCurved, ( int )QgsPalLayerSettings::PerimeterCurved );
  mPlacePolygonBtnGrp->setExclusive( true );
  connect( mPlacePolygonBtnGrp, SIGNAL( buttonClicked( int ) ), this, SLOT( updatePlacementWidgets() ) );

  // TODO: is this necessary? maybe just use the data defined-only rotation?
  mPointAngleDDBtn->setVisible( false );

  // Global settings group for groupboxes' saved/retored collapsed state
  // maintains state across different dialogs
  Q_FOREACH ( QgsCollapsibleGroupBox *grpbox, findChildren<QgsCollapsibleGroupBox*>() )
  {
    grpbox->setSettingGroup( QString( "mAdvLabelingDlg" ) );
  }

  connect( groupBox_mPreview,
           SIGNAL( collapsedStateChanged( bool ) ),
           this,
           SLOT( collapseSample( bool ) ) );

  // get rid of annoying outer focus rect on Mac
  mLabelingOptionsListWidget->setAttribute( Qt::WA_MacShowFocusRect, false );

  QSettings settings;

  // reset horiz strech of left side of options splitter (set to 1 for previewing in Qt Designer)
  QSizePolicy policy( mLabelingOptionsListFrame->sizePolicy() );
  policy.setHorizontalStretch( 0 );
  mLabelingOptionsListFrame->setSizePolicy( policy );
  if ( !settings.contains( QString( "/Windows/Labeling/OptionsSplitState" ) ) )
  {
    // set left list widget width on intial showing
    QList<int> splitsizes;
    splitsizes << 115;
    mLabelingOptionsSplitter->setSizes( splitsizes );
  }

  // set up reverse connection from stack to list
  connect( mLabelStackedWidget, SIGNAL( currentChanged( int ) ), this, SLOT( optionsStackedWidget_CurrentChanged( int ) ) );

  // restore dialog, splitters and current tab
  mFontPreviewSplitter->restoreState( settings.value( QString( "/Windows/Labeling/FontPreviewSplitState" ) ).toByteArray() );
  mLabelingOptionsSplitter->restoreState( settings.value( QString( "/Windows/Labeling/OptionsSplitState" ) ).toByteArray() );

  mLabelingOptionsListWidget->setCurrentRow( settings.value( QString( "/Windows/Labeling/Tab" ), 0 ).toInt() );

  setDockMode( false );


  QList<QWidget*> widgets;
  widgets << btnBufferColor
  << btnTextColor
  << chkLabelPerFeaturePart
  << chkLineAbove
  << chkLineBelow
  << chkLineOn
  << chkLineOrientationDependent
  << chkMergeLines
  << chkPreserveRotation
  << comboBlendMode
  << comboBufferBlendMode
  << mAlwaysShowDDBtn
  << mBufferBlendModeDDBtn
  << mBufferColorDDBtn
  << mBufferDrawChkBx
  << mBufferDrawDDBtn
  << mBufferJoinStyleComboBox
  << mBufferJoinStyleDDBtn
  << mBufferSizeDDBtn
  << mBufferTranspDDBtn
  << mBufferTranspFillChbx
  << mBufferTranspSpinBox
  << mBufferUnitsDDBtn
  << mCentroidDDBtn
  << mCentroidInsideCheckBox
  << mChkNoObstacle
  << mCoordAlignmentHDDBtn
  << mCoordAlignmentVDDBtn
  << mCoordRotationDDBtn
  << mCoordXDDBtn
  << mCoordYDDBtn
  << mDirectSymbChkBx
  << mDirectSymbDDBtn
  << mDirectSymbLeftDDBtn
  << mDirectSymbLeftLineEdit
  << mDirectSymbPlacementDDBtn
  << mDirectSymbRevChkBx
  << mDirectSymbRevDDBtn
  << mDirectSymbRightDDBtn
  << mDirectSymbRightLineEdit
  << mFitInsidePolygonCheckBox
  << mFontBlendModeDDBtn
  << mFontBoldDDBtn
  << mFontCapitalsComboBox
  << mFontCaseDDBtn
  << mFontColorDDBtn
  << mFontDDBtn
  << mFontItalicDDBtn
  << mFontLetterSpacingDDBtn
  << mFontLetterSpacingSpinBox
  << mFontLimitPixelChkBox
  << mFontLimitPixelDDBtn
  << mFontLineHeightDDBtn
  << mFontLineHeightSpinBox
  << mFontMaxPixelDDBtn
  << mFontMaxPixelSpinBox
  << mFontMinPixelDDBtn
  << mFontMinPixelSpinBox
  << mFontMultiLineAlignComboBox
  << mFontMultiLineAlignDDBtn
  << mFontSizeDDBtn
  << mFontSizeSpinBox
  << mFontStrikeoutDDBtn
  << mFontStyleComboBox
  << mFontStyleDDBtn
  << mFontTranspDDBtn
  << mFontTranspSpinBox
  << mFontUnderlineDDBtn
  << mFontUnitsDDBtn
  << mFontWordSpacingDDBtn
  << mFontWordSpacingSpinBox
  << mFormatNumChkBx
  << mFormatNumDDBtn
  << mFormatNumDecimalsDDBtn
  << mFormatNumDecimalsSpnBx
  << mFormatNumPlusSignChkBx
  << mFormatNumPlusSignDDBtn
  << mIsObstacleDDBtn
  << mLimitLabelChkBox
  << mLimitLabelSpinBox
  << mLineDistanceDDBtn
  << mLineDistanceSpnBx
  << mLineDistanceUnitDDBtn
  << mLineDistanceUnitWidget
  << mMaxCharAngleDDBtn
  << mMaxCharAngleInDSpinBox
  << mMaxCharAngleOutDSpinBox
  << mMinSizeSpinBox
  << mObstacleFactorDDBtn
  << mObstacleFactorSlider
  << mObstacleTypeComboBox
  << mOffsetTypeComboBox
  << mPalShowAllLabelsForLayerChkBx
  << mPointAngleDDBtn
  << mPointAngleSpinBox
  << mPointOffsetDDBtn
  << mPointOffsetUnitsDDBtn
  << mPointOffsetUnitWidget
  << mPointOffsetXSpinBox
  << mPointOffsetYSpinBox
  << mPointPositionOrderDDBtn
  << mPointQuadOffsetDDBtn
  << mPreviewBackgroundBtn
  << mPreviewTextEdit
  << mPriorityDDBtn
  << mPrioritySlider
  << mRepeatDistanceDDBtn
  << mRepeatDistanceSpinBox
  << mRepeatDistanceUnitDDBtn
  << mRepeatDistanceUnitWidget
  << mScaleBasedVisibilityChkBx
  << mScaleBasedVisibilityDDBtn
  << mScaleBasedVisibilityMaxDDBtn
  << mScaleBasedVisibilityMaxSpnBx
  << mScaleBasedVisibilityMinDDBtn
  << mScaleBasedVisibilityMinSpnBx
  << mShadowBlendCmbBx
  << mShadowBlendDDBtn
  << mShadowColorBtn
  << mShadowColorDDBtn
  << mShadowDrawChkBx
  << mShadowDrawDDBtn
  << mShadowOffsetAngleDDBtn
  << mShadowOffsetAngleSpnBx
  << mShadowOffsetDDBtn
  << mShadowOffsetGlobalChkBx
  << mShadowOffsetSpnBx
  << mShadowOffsetUnitsDDBtn
  << mShadowOffsetUnitWidget
  << mShadowRadiusAlphaChkBx
  << mShadowRadiusDDBtn
  << mShadowRadiusDblSpnBx
  << mShadowRadiusUnitsDDBtn
  << mShadowRadiusUnitWidget
  << mShadowScaleDDBtn
  << mShadowScaleSpnBx
  << mShadowTranspDDBtn
  << mShadowTranspSpnBx
  << mShadowUnderCmbBx
  << mShadowUnderDDBtn
  << mShapeBlendCmbBx
  << mShapeBlendModeDDBtn
  << mShapeBorderColorBtn
  << mShapeBorderColorDDBtn
  << mShapeBorderUnitsDDBtn
  << mShapeBorderWidthDDBtn
  << mShapeBorderWidthSpnBx
  << mShapeBorderWidthUnitWidget
  << mShapeDrawChkBx
  << mShapeDrawDDBtn
  << mShapeFillColorBtn
  << mShapeFillColorDDBtn
  << mShapeOffsetDDBtn
  << mShapeOffsetUnitsDDBtn
  << mShapeOffsetXSpnBx
  << mShapeOffsetYSpnBx
  << mShapeOffsetUnitWidget
  << mShapePenStyleCmbBx
  << mShapePenStyleDDBtn
  << mShapeRadiusDDBtn
  << mShapeRadiusUnitsDDBtn
  << mShapeRadiusXDbSpnBx
  << mShapeRadiusYDbSpnBx
  << mShapeRotationCmbBx
  << mShapeRotationDDBtn
  << mShapeRotationDblSpnBx
  << mShapeRotationTypeDDBtn
  << mShapeRadiusUnitWidget
  << mShapeSVGPathDDBtn
  << mShapeSVGPathLineEdit
  << mShapeSizeCmbBx
  << mShapeSizeTypeDDBtn
  << mShapeSizeUnitsDDBtn
  << mShapeSizeUnitWidget
  << mShapeSizeXDDBtn
  << mShapeSizeXSpnBx
  << mShapeSizeYDDBtn
  << mShapeSizeYSpnBx
  << mShapeTranspDDBtn
  << mShapeTranspSpinBox
  << mShapeTypeCmbBx
  << mShapeTypeDDBtn
  << mShowLabelDDBtn
  << mWrapCharDDBtn
  << mZIndexDDBtn
  << mZIndexSpinBox
  << spinBufferSize
  << wrapCharacterEdit
  << mCentroidRadioVisible
  << mCentroidRadioWhole
  << mDirectSymbRadioBtnAbove
  << mDirectSymbRadioBtnBelow
  << mDirectSymbRadioBtnLR
  << mUpsidedownRadioAll
  << mUpsidedownRadioDefined
  << mUpsidedownRadioOff
  << radAroundCentroid
  << radAroundPoint
  << radLineCurved
  << radLineHorizontal
  << radLineParallel
  << radOverCentroid
  << radOverPoint
  << radPolygonFree
  << radPolygonHorizontal
  << radPolygonPerimeter
  << radPolygonPerimeterCurved
  << radPredefinedOrder
  << mFieldExpressionWidget
  << mCheckBoxSubstituteText;
  connectValueChanged( widgets, SLOT( updatePreview() ) );

  connect( mQuadrantBtnGrp, SIGNAL( buttonClicked( int ) ), this, SLOT( updatePreview() ) );

  // set correct initial tab to match displayed setting page
  whileBlocking( mOptionsTab )->setCurrentIndex( mLabelStackedWidget->currentIndex() );

  if ( mMapCanvas )
  {
    lblFontPreview->setMapUnits( mMapCanvas->mapSettings().mapUnits() );
    mPreviewScaleComboBox->setScale( 1.0 / mMapCanvas->mapSettings().scale() );
  }
}

void QgsTextFormatWidget::setWidgetMode( QgsTextFormatWidget::Mode mode )
{
  mWidgetMode = mode;
  switch ( mode )
  {
    case Labeling:
      toggleDDButtons( true );
      break;

    case Text:
      toggleDDButtons( false );
      delete mLabelingOptionsListWidget->takeItem( 6 );
      delete mLabelingOptionsListWidget->takeItem( 5 );
      mOptionsTab->removeTab( 6 );
      mOptionsTab->removeTab( 5 );

      frameLabelWith->hide();
      mDirectSymbolsFrame->hide();
      mFormatNumFrame->hide();
      mFormatNumChkBx->hide();
      mFormatNumDDBtn->hide();
      mSubstitutionsFrame->hide();
      mFontBoldBtn->hide();
      mFontItalicBtn->hide();

      break;
  }
}

void QgsTextFormatWidget::toggleDDButtons( bool visible )
{
  Q_FOREACH ( QgsDataDefinedButton* button, findChildren< QgsDataDefinedButton* >() )
  {
    button->setVisible( visible );
  }
}

void QgsTextFormatWidget::setDockMode( bool enabled )
{
  mOptionsTab->setVisible( enabled );
  mOptionsTab->setTabToolTip( 0, tr( "Text" ) );
  mOptionsTab->setTabToolTip( 1, tr( "Formatting" ) );
  mOptionsTab->setTabToolTip( 2, tr( "Buffer" ) );
  mOptionsTab->setTabToolTip( 3, tr( "Background" ) );
  mOptionsTab->setTabToolTip( 4, tr( "Shadow" ) );
  mOptionsTab->setTabToolTip( 5, tr( "Placement" ) );
  mOptionsTab->setTabToolTip( 6, tr( "Rendering" ) );

  mLabelingOptionsListFrame->setVisible( !enabled );
  groupBox_mPreview->setVisible( !enabled );
  mDockMode = enabled;
}

void QgsTextFormatWidget::connectValueChanged( QList<QWidget *> widgets, const char *slot )
{
  Q_FOREACH ( QWidget* widget, widgets )
  {
    if ( QgsDataDefinedButton* w = qobject_cast<QgsDataDefinedButton*>( widget ) )
    {
      connect( w, SIGNAL( dataDefinedActivated( bool ) ), this, slot );
      connect( w, SIGNAL( dataDefinedChanged( QString ) ), this, slot );
    }
    else if ( QgsFieldExpressionWidget* w = qobject_cast< QgsFieldExpressionWidget*>( widget ) )
    {
      connect( w, SIGNAL( fieldChanged( QString ) ), this,  slot );
    }
    else if ( QgsUnitSelectionWidget* w = qobject_cast<QgsUnitSelectionWidget*>( widget ) )
    {
      connect( w, SIGNAL( changed() ), this,  slot );
    }
    else if ( QComboBox* w = qobject_cast<QComboBox*>( widget ) )
    {
      connect( w, SIGNAL( currentIndexChanged( int ) ), this, slot );
    }
    else if ( QSpinBox* w = qobject_cast<QSpinBox*>( widget ) )
    {
      connect( w, SIGNAL( valueChanged( int ) ), this, slot );
    }
    else if ( QDoubleSpinBox* w = qobject_cast<QDoubleSpinBox*>( widget ) )
    {
      connect( w , SIGNAL( valueChanged( double ) ), this, slot );
    }
    else if ( QgsColorButton* w = qobject_cast<QgsColorButton*>( widget ) )
    {
      connect( w, SIGNAL( colorChanged( QColor ) ), this, slot );
    }
    else if ( QCheckBox* w = qobject_cast<QCheckBox*>( widget ) )
    {
      connect( w, SIGNAL( toggled( bool ) ), this, slot );
    }
    else if ( QRadioButton* w = qobject_cast<QRadioButton*>( widget ) )
    {
      connect( w, SIGNAL( toggled( bool ) ), this, slot );
    }
    else if ( QLineEdit* w = qobject_cast<QLineEdit*>( widget ) )
    {
      connect( w, SIGNAL( textEdited( QString ) ), this, slot );
    }
    else if ( QSlider* w = qobject_cast<QSlider*>( widget ) )
    {
      connect( w, SIGNAL( valueChanged( int ) ), this, slot );
    }
    else
    {
      QgsLogger::warning( QString( "Could not create connection for widget %1" ).arg( widget->objectName() ) );
    }
  }
}

void QgsTextFormatWidget::updateWidgetForFormat( const QgsTextFormat& format )
{
  QgsTextBufferSettings buffer = format.buffer();
  QgsTextBackgroundSettings background = format.background();
  QgsTextShadowSettings shadow = format.shadow();

  // buffer
  mBufferDrawChkBx->setChecked( buffer.enabled() );
  spinBufferSize->setValue( buffer.size() );
  mBufferUnitWidget->setUnit( buffer.sizeUnit() );
  mBufferUnitWidget->setMapUnitScale( buffer.sizeMapUnitScale() );
  btnBufferColor->setColor( buffer.color() );
  mBufferTranspSpinBox->setValue( 100 - 100 * buffer.opacity() );
  mBufferJoinStyleComboBox->setPenJoinStyle( buffer.joinStyle() );
  mBufferTranspFillChbx->setChecked( buffer.fillBufferInterior() );
  comboBufferBlendMode->setBlendMode( buffer.blendMode() );


  mFontSizeUnitWidget->setUnit( format.sizeUnit() );
  mFontSizeUnitWidget->setMapUnitScale( format.sizeMapUnitScale() );
  mRefFont = format.font();
  mFontSizeSpinBox->setValue( format.size() );
  btnTextColor->setColor( format.color() );
  mFontTranspSpinBox->setValue( 100 - 100 * format.opacity() );
  comboBlendMode->setBlendMode( format.blendMode() );

  mFontWordSpacingSpinBox->setValue( format.font().wordSpacing() );
  mFontLetterSpacingSpinBox->setValue( format.font().letterSpacing() );

  QgsFontUtils::updateFontViaStyle( mRefFont, format.namedStyle() );
  updateFont( mRefFont );

  // show 'font not found' if substitution has occurred (should come after updateFont())
  mFontMissingLabel->setVisible( !format.fontFound() );
  if ( !format.fontFound() )
  {
    QString missingTxt = tr( "%1 not found. Default substituted." );
    QString txtPrepend = tr( "Chosen font" );
    if ( !format.resolvedFontFamily().isEmpty() )
    {
      txtPrepend = QString( "'%1'" ).arg( format.resolvedFontFamily() );
    }
    mFontMissingLabel->setText( missingTxt.arg( txtPrepend ) );

    // ensure user is sent to 'Text style' section to see notice
    mLabelingOptionsListWidget->setCurrentRow( 0 );
  }
  mFontLineHeightSpinBox->setValue( format.lineHeight() );

  // shape background
  mShapeDrawChkBx->setChecked( background.enabled() );
  mShapeTypeCmbBx->blockSignals( true );
  mShapeTypeCmbBx->setCurrentIndex( background.type() );
  mShapeTypeCmbBx->blockSignals( false );
  mShapeSVGPathLineEdit->setText( background.svgFile() );

  mShapeSizeCmbBx->setCurrentIndex( background.sizeType() );
  mShapeSizeXSpnBx->setValue( background.size().width() );
  mShapeSizeYSpnBx->setValue( background.size().height() );
  mShapeSizeUnitWidget->setUnit( background.sizeUnit() );
  mShapeSizeUnitWidget->setMapUnitScale( background.sizeMapUnitScale() );
  mShapeRotationCmbBx->setCurrentIndex( background.rotationType() );
  mShapeRotationDblSpnBx->setEnabled( background.rotationType() != QgsTextBackgroundSettings::RotationSync );
  mShapeRotationDDBtn->setEnabled( background.rotationType() != QgsTextBackgroundSettings::RotationSync );
  mShapeRotationDblSpnBx->setValue( background.rotation() );
  mShapeOffsetXSpnBx->setValue( background.offset().x() );
  mShapeOffsetYSpnBx->setValue( background.offset().y() );
  mShapeOffsetUnitWidget->setUnit( background.offsetUnit() );
  mShapeOffsetUnitWidget->setMapUnitScale( background.offsetMapUnitScale() );
  mShapeRadiusXDbSpnBx->setValue( background.radii().width() );
  mShapeRadiusYDbSpnBx->setValue( background.radii().height() );
  mShapeRadiusUnitWidget->setUnit( background.radiiUnit() );
  mShapeRadiusUnitWidget->setMapUnitScale( background.radiiMapUnitScale() );

  mShapeFillColorBtn->setColor( background.fillColor() );
  mShapeBorderColorBtn->setColor( background.borderColor() );
  mShapeBorderWidthSpnBx->setValue( background.borderWidth() );
  mShapeBorderWidthUnitWidget->setUnit( background.borderWidthUnit() );
  mShapeBorderWidthUnitWidget->setMapUnitScale( background.borderWidthMapUnitScale() );
  mShapePenStyleCmbBx->setPenJoinStyle( background.joinStyle() );

  mShapeTranspSpinBox->setValue( 100 - background.opacity() * 100.0 );
  mShapeBlendCmbBx->setBlendMode( background.blendMode() );

  mLoadSvgParams = false;
  on_mShapeTypeCmbBx_currentIndexChanged( background.type() ); // force update of shape background gui

  // drop shadow
  mShadowDrawChkBx->setChecked( shadow.enabled() );
  mShadowUnderCmbBx->setCurrentIndex( shadow.shadowPlacement() );
  mShadowOffsetAngleSpnBx->setValue( shadow.offsetAngle() );
  mShadowOffsetSpnBx->setValue( shadow.offsetDistance() );
  mShadowOffsetUnitWidget->setUnit( shadow.offsetUnit() );
  mShadowOffsetUnitWidget->setMapUnitScale( shadow.offsetMapUnitScale() );
  mShadowOffsetGlobalChkBx->setChecked( shadow.offsetGlobal() );

  mShadowRadiusDblSpnBx->setValue( shadow.blurRadius() );
  mShadowRadiusUnitWidget->setUnit( shadow.blurRadiusUnit() );
  mShadowRadiusUnitWidget->setMapUnitScale( shadow.blurRadiusMapUnitScale() );
  mShadowRadiusAlphaChkBx->setChecked( shadow.blurAlphaOnly() );
  mShadowTranspSpnBx->setValue( 100 - shadow.opacity() * 100.0 );
  mShadowScaleSpnBx->setValue( shadow.scale() );

  mShadowColorBtn->setColor( shadow.color() );
  mShadowBlendCmbBx->setBlendMode( shadow.blendMode() );

}

QgsTextFormatWidget::~QgsTextFormatWidget()
{
  QSettings settings;
  settings.setValue( QString( "/Windows/Labeling/FontPreviewSplitState" ), mFontPreviewSplitter->saveState() );
  settings.setValue( QString( "/Windows/Labeling/OptionsSplitState" ), mLabelingOptionsSplitter->saveState() );
  settings.setValue( QString( "/Windows/Labeling/Tab" ), mLabelingOptionsListWidget->currentRow() );
}

QgsTextFormat QgsTextFormatWidget::format() const
{
  QgsTextFormat format;
  format.setColor( btnTextColor->color() );
  format.setFont( mRefFont );
  format.setSize( mFontSizeSpinBox->value() );
  format.setNamedStyle( mFontStyleComboBox->currentText() );
  format.setOpacity( 1.0 - mFontTranspSpinBox->value() / 100.0 );
  format.setBlendMode( comboBlendMode->blendMode() );
  format.setSizeUnit( mFontSizeUnitWidget->unit() );
  format.setSizeMapUnitScale( mFontSizeUnitWidget->getMapUnitScale() );
  format.setLineHeight( mFontLineHeightSpinBox->value() );

  // buffer
  QgsTextBufferSettings buffer;
  buffer.setEnabled( mBufferDrawChkBx->isChecked() );
  buffer.setSize( spinBufferSize->value() );
  buffer.setColor( btnBufferColor->color() );
  buffer.setOpacity( 1.0 - mBufferTranspSpinBox->value() / 100.0 );
  buffer.setSizeUnit( mBufferUnitWidget->unit() );
  buffer.setSizeMapUnitScale( mBufferUnitWidget->getMapUnitScale() );
  buffer.setJoinStyle( mBufferJoinStyleComboBox->penJoinStyle() );
  buffer.setFillBufferInterior( mBufferTranspFillChbx->isChecked() );
  buffer.setBlendMode( comboBufferBlendMode->blendMode() );
  format.setBuffer( buffer );

  // shape background
  QgsTextBackgroundSettings background;
  background.setEnabled( mShapeDrawChkBx->isChecked() );
  background.setType(( QgsTextBackgroundSettings::ShapeType )mShapeTypeCmbBx->currentIndex() );
  background.setSvgFile( mShapeSVGPathLineEdit->text() );
  background.setSizeType(( QgsTextBackgroundSettings::SizeType )mShapeSizeCmbBx->currentIndex() );
  background.setSize( QSizeF( mShapeSizeXSpnBx->value(), mShapeSizeYSpnBx->value() ) );
  background.setSizeUnit( mShapeSizeUnitWidget->unit() );
  background.setSizeMapUnitScale( mShapeSizeUnitWidget->getMapUnitScale() );
  background.setRotationType(( QgsTextBackgroundSettings::RotationType )( mShapeRotationCmbBx->currentIndex() ) );
  background.setRotation( mShapeRotationDblSpnBx->value() );
  background.setOffset( QPointF( mShapeOffsetXSpnBx->value(), mShapeOffsetYSpnBx->value() ) );
  background.setOffsetUnit( mShapeOffsetUnitWidget->unit() );
  background.setOffsetMapUnitScale( mShapeOffsetUnitWidget->getMapUnitScale() );
  background.setRadii( QSizeF( mShapeRadiusXDbSpnBx->value(), mShapeRadiusYDbSpnBx->value() ) );
  background.setRadiiUnit( mShapeRadiusUnitWidget->unit() );
  background.setRadiiMapUnitScale( mShapeRadiusUnitWidget->getMapUnitScale() );

  background.setFillColor( mShapeFillColorBtn->color() );
  background.setBorderColor( mShapeBorderColorBtn->color() );
  background.setBorderWidth( mShapeBorderWidthSpnBx->value() );
  background.setBorderWidthUnit( mShapeBorderWidthUnitWidget->unit() );
  background.setBorderWidthMapUnitScale( mShapeBorderWidthUnitWidget->getMapUnitScale() );
  background.setJoinStyle( mShapePenStyleCmbBx->penJoinStyle() );
  background.setOpacity( 1.0 - mShapeTranspSpinBox->value() / 100.0 );
  background.setBlendMode( mShapeBlendCmbBx->blendMode() );
  format.setBackground( background );

  // drop shadow
  QgsTextShadowSettings shadow;
  shadow.setEnabled( mShadowDrawChkBx->isChecked() );
  shadow.setShadowPlacement(( QgsTextShadowSettings::ShadowPlacement )mShadowUnderCmbBx->currentIndex() );
  shadow.setOffsetAngle( mShadowOffsetAngleSpnBx->value() );
  shadow.setOffsetDistance( mShadowOffsetSpnBx->value() );
  shadow.setOffsetUnit( mShadowOffsetUnitWidget->unit() );
  shadow.setOffsetMapUnitScale( mShadowOffsetUnitWidget->getMapUnitScale() );
  shadow.setOffsetGlobal( mShadowOffsetGlobalChkBx->isChecked() );
  shadow.setBlurRadius( mShadowRadiusDblSpnBx->value() );
  shadow.setBlurRadiusUnit( mShadowRadiusUnitWidget->unit() );
  shadow.setBlurRadiusMapUnitScale( mShadowRadiusUnitWidget->getMapUnitScale() );
  shadow.setBlurAlphaOnly( mShadowRadiusAlphaChkBx->isChecked() );
  shadow.setOpacity( 1.0 - mShadowTranspSpnBx->value() / 100.0 );
  shadow.setScale( mShadowScaleSpnBx->value() );
  shadow.setColor( mShadowColorBtn->color() );
  shadow.setBlendMode( mShadowBlendCmbBx->blendMode() );
  format.setShadow( shadow );

  return format;
}

void QgsTextFormatWidget::optionsStackedWidget_CurrentChanged( int indx )
{
  mLabelingOptionsListWidget->blockSignals( true );
  mLabelingOptionsListWidget->setCurrentRow( indx );
  mLabelingOptionsListWidget->blockSignals( false );
}

void QgsTextFormatWidget::collapseSample( bool collapse )
{
  if ( collapse )
  {
    QList<int> splitSizes = mFontPreviewSplitter->sizes();
    if ( splitSizes[0] > groupBox_mPreview->height() )
    {
      int delta = splitSizes[0] - groupBox_mPreview->height();
      splitSizes[0] -= delta;
      splitSizes[1] += delta;
      mFontPreviewSplitter->setSizes( splitSizes );
    }
  }
}

void QgsTextFormatWidget::changeTextColor( const QColor &color )
{
  Q_UNUSED( color )
  updatePreview();
}

void QgsTextFormatWidget::updateFont( const QFont& font )
{
  // update background reference font
  if ( font != mRefFont )
  {
    mRefFont = font;
  }

  // test if font is actually available
  // NOTE: QgsFontUtils::fontMatchOnSystem may fail here, just crosscheck family
  mFontMissingLabel->setVisible( !QgsFontUtils::fontFamilyMatchOnSystem( mRefFont.family() ) );

  mDirectSymbLeftLineEdit->setFont( mRefFont );
  mDirectSymbRightLineEdit->setFont( mRefFont );

  blockFontChangeSignals( true );
  mFontFamilyCmbBx->setCurrentFont( mRefFont );
  populateFontStyleComboBox();
  int idx = mFontCapitalsComboBox->findData( QVariant(( unsigned int ) mRefFont.capitalization() ) );
  mFontCapitalsComboBox->setCurrentIndex( idx == -1 ? 0 : idx );
  mFontUnderlineBtn->setChecked( mRefFont.underline() );
  mFontStrikethroughBtn->setChecked( mRefFont.strikeOut() );
  blockFontChangeSignals( false );

  // update font name with font face
//  font.setPixelSize( 24 );

  updatePreview();
}

void QgsTextFormatWidget::blockFontChangeSignals( bool blk )
{
  mFontFamilyCmbBx->blockSignals( blk );
  mFontStyleComboBox->blockSignals( blk );
  mFontCapitalsComboBox->blockSignals( blk );
  mFontUnderlineBtn->blockSignals( blk );
  mFontStrikethroughBtn->blockSignals( blk );
  mFontWordSpacingSpinBox->blockSignals( blk );
  mFontLetterSpacingSpinBox->blockSignals( blk );
}

void QgsTextFormatWidget::updatePreview()
{
  // In dock mode we don't have a preview we
  // just let stuff know we have changed because
  // there might be live updates connected.
  if ( mDockMode )
  {
    emit widgetChanged();
    return;
  }

  scrollPreview();
  lblFontPreview->setFormat( format() );
}

void QgsTextFormatWidget::scrollPreview()
{
  scrollArea_mPreview->ensureVisible( 0, 0, 0, 0 );
}

void QgsTextFormatWidget::setPreviewBackground( const QColor& color )
{
  scrollArea_mPreview->widget()->setStyleSheet( QString( "background: rgb(%1, %2, %3);" ).arg( QString::number( color.red() ),
      QString::number( color.green() ),
      QString::number( color.blue() ) ) );
}

void QgsTextFormatWidget::changeBufferColor( const QColor &color )
{
  Q_UNUSED( color )
  updatePreview();
}

void QgsTextFormatWidget::updatePlacementWidgets()
{
  QWidget* curWdgt = stackedPlacement->currentWidget();

  bool showLineFrame = false;
  bool showCentroidFrame = false;
  bool showQuadrantFrame = false;
  bool showFixedQuadrantFrame = false;
  bool showPlacementPriorityFrame = false;
  bool showOffsetTypeFrame = false;
  bool showOffsetFrame = false;
  bool showDistanceFrame = false;
  bool showRotationFrame = false;
  bool showMaxCharAngleFrame = false;

  bool enableMultiLinesFrame = true;

  if (( curWdgt == pagePoint && radAroundPoint->isChecked() )
      || ( curWdgt == pagePolygon && radAroundCentroid->isChecked() ) )
  {
    showCentroidFrame = ( curWdgt == pagePolygon && radAroundCentroid->isChecked() );
    showDistanceFrame = true;
    //showRotationFrame = true; // TODO: uncomment when supported
    if ( curWdgt == pagePoint )
    {
      showQuadrantFrame = true;
    }
  }
  else if (( curWdgt == pagePoint && radOverPoint->isChecked() )
           || ( curWdgt == pagePolygon && radOverCentroid->isChecked() ) )
  {
    showCentroidFrame = ( curWdgt == pagePolygon && radOverCentroid->isChecked() );
    showQuadrantFrame = true;
    showFixedQuadrantFrame = true;
    showOffsetFrame = true;
    showRotationFrame = true;
  }
  else if ( curWdgt == pagePoint && radPredefinedOrder->isChecked() )
  {
    showDistanceFrame = true;
    showPlacementPriorityFrame = true;
    showOffsetTypeFrame  = true;
  }
  else if (( curWdgt == pageLine && radLineParallel->isChecked() )
           || ( curWdgt == pagePolygon && radPolygonPerimeter->isChecked() )
           || ( curWdgt == pageLine && radLineCurved->isChecked() )
           || ( curWdgt == pagePolygon && radPolygonPerimeterCurved->isChecked() ) )
  {
    showLineFrame = true;
    showDistanceFrame = true;
    //showRotationFrame = true; // TODO: uncomment when supported

    bool offline = chkLineAbove->isChecked() || chkLineBelow->isChecked();
    chkLineOrientationDependent->setEnabled( offline );
    mPlacementDistanceFrame->setEnabled( offline );

    bool isCurved = ( curWdgt == pageLine && radLineCurved->isChecked() )
                    || ( curWdgt == pagePolygon && radPolygonPerimeterCurved->isChecked() );
    showMaxCharAngleFrame = isCurved;
    // TODO: enable mMultiLinesFrame when supported for curved labels
    enableMultiLinesFrame = !isCurved;
  }

  mPlacementLineFrame->setVisible( showLineFrame );
  mPlacementCentroidFrame->setVisible( showCentroidFrame );
  mPlacementQuadrantFrame->setVisible( showQuadrantFrame );
  mPlacementFixedQuadrantFrame->setVisible( showFixedQuadrantFrame );
  mPlacementCartographicFrame->setVisible( showPlacementPriorityFrame );
  mPlacementOffsetFrame->setVisible( showOffsetFrame );
  mPlacementDistanceFrame->setVisible( showDistanceFrame );
  mPlacementOffsetTypeFrame->setVisible( showOffsetTypeFrame );
  mPlacementRotationFrame->setVisible( showRotationFrame );
  mPlacementRepeatDistanceFrame->setVisible( curWdgt == pageLine || ( curWdgt == pagePolygon &&
      ( radPolygonPerimeter->isChecked() || radPolygonPerimeterCurved->isChecked() ) ) );
  mPlacementMaxCharAngleFrame->setVisible( showMaxCharAngleFrame );

  mMultiLinesFrame->setEnabled( enableMultiLinesFrame );
}

void QgsTextFormatWidget::populateFontCapitalsComboBox()
{
  mFontCapitalsComboBox->addItem( tr( "No change" ), QVariant( 0 ) );
  mFontCapitalsComboBox->addItem( tr( "All uppercase" ), QVariant( 1 ) );
  mFontCapitalsComboBox->addItem( tr( "All lowercase" ), QVariant( 2 ) );
  // Small caps doesn't work right with QPainterPath::addText()
  // https://bugreports.qt-project.org/browse/QTBUG-13965
//  mFontCapitalsComboBox->addItem( tr( "Small caps" ), QVariant( 3 ) );
  mFontCapitalsComboBox->addItem( tr( "Capitalize first letter" ), QVariant( 4 ) );
}

void QgsTextFormatWidget::populateFontStyleComboBox()
{
  mFontStyleComboBox->clear();
  Q_FOREACH ( const QString &style, mFontDB.styles( mRefFont.family() ) )
  {
    mFontStyleComboBox->addItem( style );
  }

  int curIndx = 0;
  int stylIndx = mFontStyleComboBox->findText( mFontDB.styleString( mRefFont ) );
  if ( stylIndx > -1 )
  {
    curIndx = stylIndx;
  }

  mFontStyleComboBox->setCurrentIndex( curIndx );
}

void QgsTextFormatWidget::on_mFontSizeSpinBox_valueChanged( double d )
{
  mRefFont.setPointSizeF( d );
  updateFont( mRefFont );
}

void QgsTextFormatWidget::on_mFontCapitalsComboBox_currentIndexChanged( int index )
{
  int capitalsindex = mFontCapitalsComboBox->itemData( index ).toUInt();
  mRefFont.setCapitalization(( QFont::Capitalization ) capitalsindex );
  updateFont( mRefFont );
}

void QgsTextFormatWidget::on_mFontFamilyCmbBx_currentFontChanged( const QFont& f )
{
  mRefFont.setFamily( f.family() );
  updateFont( mRefFont );
}

void QgsTextFormatWidget::on_mFontStyleComboBox_currentIndexChanged( const QString & text )
{
  QgsFontUtils::updateFontViaStyle( mRefFont, text );
  updateFont( mRefFont );
}

void QgsTextFormatWidget::on_mFontUnderlineBtn_toggled( bool ckd )
{
  mRefFont.setUnderline( ckd );
  updateFont( mRefFont );
}

void QgsTextFormatWidget::on_mFontStrikethroughBtn_toggled( bool ckd )
{
  mRefFont.setStrikeOut( ckd );
  updateFont( mRefFont );
}

void QgsTextFormatWidget::on_mFontWordSpacingSpinBox_valueChanged( double spacing )
{
  mRefFont.setWordSpacing( spacing );
  updateFont( mRefFont );
}

void QgsTextFormatWidget::on_mFontLetterSpacingSpinBox_valueChanged( double spacing )
{
  mRefFont.setLetterSpacing( QFont::AbsoluteSpacing, spacing );
  updateFont( mRefFont );
}

void QgsTextFormatWidget::on_mFontSizeUnitWidget_changed()
{
  // disable pixel size limiting for labels defined in points
  if ( mFontSizeUnitWidget->unit() != QgsUnitTypes::RenderMapUnits )
  {
    mFontLimitPixelChkBox->setChecked( false );
  }
  else if ( mMinPixelLimit == 0 )
  {
    // initial minimum trigger value set, turn on pixel size limiting by default
    // for labels defined in map units (ignored after first settings save)
    mFontLimitPixelChkBox->setChecked( true );
  }
  updateFont( mRefFont );
}

void QgsTextFormatWidget::on_mFontMinPixelSpinBox_valueChanged( int px )
{
  // ensure max font pixel size for map unit labels can't be lower than min
  mFontMaxPixelSpinBox->setMinimum( px );
  mFontMaxPixelSpinBox->update();
}

void QgsTextFormatWidget::on_mFontMaxPixelSpinBox_valueChanged( int px )
{
  // ensure max font pixel size for map unit labels can't be lower than min
  if ( px < mFontMinPixelSpinBox->value() )
  {
    mFontMaxPixelSpinBox->blockSignals( true );
    mFontMaxPixelSpinBox->setValue( mFontMinPixelSpinBox->value() );
    mFontMaxPixelSpinBox->blockSignals( false );
  }
  mFontMaxPixelSpinBox->setMinimum( mFontMinPixelSpinBox->value() );
}

void QgsTextFormatWidget::on_mBufferUnitWidget_changed()
{
  updateFont( mRefFont );
}

void QgsTextFormatWidget::on_mCoordXDDBtn_dataDefinedActivated( bool active )
{
  if ( !active ) //no data defined alignment without data defined position
  {
    enableDataDefinedAlignment( false );
  }
  else if ( mCoordYDDBtn->isActive() )
  {
    enableDataDefinedAlignment( true );
  }
}

void QgsTextFormatWidget::on_mCoordYDDBtn_dataDefinedActivated( bool active )
{
  if ( !active ) //no data defined alignment without data defined position
  {
    enableDataDefinedAlignment( false );
  }
  else if ( mCoordXDDBtn->isActive() )
  {
    enableDataDefinedAlignment( true );
  }
}

void QgsTextFormatWidget::on_mShapeTypeCmbBx_currentIndexChanged( int index )
{
  // shape background
  bool isRect = (( QgsTextBackgroundSettings::ShapeType )index == QgsTextBackgroundSettings::ShapeRectangle
                 || ( QgsTextBackgroundSettings::ShapeType )index == QgsTextBackgroundSettings::ShapeSquare );
  bool isSVG = (( QgsTextBackgroundSettings::ShapeType )index == QgsTextBackgroundSettings::ShapeSVG );

  showBackgroundPenStyle( isRect );
  showBackgroundRadius( isRect );

  mShapeSVGPathFrame->setVisible( isSVG );
  // symbology SVG renderer only supports size^2 scaling, so we only use the x size spinbox
  mShapeSizeYLabel->setVisible( !isSVG );
  mShapeSizeYSpnBx->setVisible( !isSVG );
  mShapeSizeYDDBtn->setVisible( !isSVG );
  mShapeSizeXLabel->setText( tr( "Size%1" ).arg( !isSVG ? tr( " X" ) : "" ) );

  // SVG parameter setting doesn't support color's alpha component yet
  mShapeFillColorBtn->setAllowAlpha( !isSVG );
  mShapeFillColorBtn->setButtonBackground();
  mShapeBorderColorBtn->setAllowAlpha( !isSVG );
  mShapeBorderColorBtn->setButtonBackground();

  // configure SVG parameter widgets
  mShapeSVGParamsBtn->setVisible( isSVG );
  if ( isSVG )
  {
    updateSvgWidgets( mShapeSVGPathLineEdit->text() );
  }
  else
  {
    mShapeFillColorLabel->setEnabled( true );
    mShapeFillColorBtn->setEnabled( true );
    mShapeFillColorDDBtn->setEnabled( true );
    mShapeBorderColorLabel->setEnabled( true );
    mShapeBorderColorBtn->setEnabled( true );
    mShapeBorderColorDDBtn->setEnabled( true );
    mShapeBorderWidthLabel->setEnabled( true );
    mShapeBorderWidthSpnBx->setEnabled( true );
    mShapeBorderWidthDDBtn->setEnabled( true );
  }
  // TODO: fix overriding SVG symbol's border width units in QgsSvgCache
  // currently broken, fall back to symbol units only
  mShapeBorderWidthUnitWidget->setVisible( !isSVG );
  mShapeSVGUnitsLabel->setVisible( isSVG );
  mShapeBorderUnitsDDBtn->setEnabled( !isSVG );
}

void QgsTextFormatWidget::on_mShapeSVGPathLineEdit_textChanged( const QString& text )
{
  updateSvgWidgets( text );
}

void QgsTextFormatWidget::updateLinePlacementOptions()
{
  int numOptionsChecked = ( chkLineAbove->isChecked() ? 1 : 0 ) +
                          ( chkLineBelow->isChecked() ? 1 : 0 ) +
                          ( chkLineOn->isChecked() ? 1 : 0 );

  if ( numOptionsChecked == 1 )
  {
    //prevent unchecking last option
    chkLineAbove->setEnabled( !chkLineAbove->isChecked() );
    chkLineBelow->setEnabled( !chkLineBelow->isChecked() );
    chkLineOn->setEnabled( !chkLineOn->isChecked() );
  }
  else
  {
    chkLineAbove->setEnabled( true );
    chkLineBelow->setEnabled( true );
    chkLineOn->setEnabled( true );
  }
}

void QgsTextFormatWidget::onSubstitutionsChanged( const QgsStringReplacementCollection& substitutions )
{
  mSubstitutions = substitutions;
  emit widgetChanged();
}

void QgsTextFormatWidget::previewScaleChanged( double scale )
{
  lblFontPreview->setScale( scale );
}

void QgsTextFormatWidget::updateSvgWidgets( const QString& svgPath )
{
  if ( mShapeSVGPathLineEdit->text() != svgPath )
  {
    mShapeSVGPathLineEdit->setText( svgPath );
  }

  QString resolvedPath = QgsSymbolLayerUtils::symbolNameToPath( svgPath );
  bool validSVG = !resolvedPath.isNull();

  // draw red text for path field if invalid (path can't be resolved)
  mShapeSVGPathLineEdit->setStyleSheet( QString( !validSVG ? "QLineEdit{ color: rgb(225, 0, 0); }" : "" ) );
  mShapeSVGPathLineEdit->setToolTip( !validSVG ? tr( "File not found" ) : resolvedPath );

  QColor fill, outline;
  double outlineWidth = 0.0;
  bool fillParam = false, outlineParam = false, outlineWidthParam = false;
  if ( validSVG )
  {
    QgsSvgCache::instance()->containsParams( resolvedPath, fillParam, fill, outlineParam, outline, outlineWidthParam, outlineWidth );
  }

  mShapeSVGParamsBtn->setEnabled( validSVG && ( fillParam || outlineParam || outlineWidthParam ) );

  mShapeFillColorLabel->setEnabled( validSVG && fillParam );
  mShapeFillColorBtn->setEnabled( validSVG && fillParam );
  mShapeFillColorDDBtn->setEnabled( validSVG && fillParam );
  if ( mLoadSvgParams && validSVG && fillParam )
    mShapeFillColorBtn->setColor( fill );

  mShapeBorderColorLabel->setEnabled( validSVG && outlineParam );
  mShapeBorderColorBtn->setEnabled( validSVG && outlineParam );
  mShapeBorderColorDDBtn->setEnabled( validSVG && outlineParam );
  if ( mLoadSvgParams && validSVG && outlineParam )
    mShapeBorderColorBtn->setColor( outline );

  mShapeBorderWidthLabel->setEnabled( validSVG && outlineWidthParam );
  mShapeBorderWidthSpnBx->setEnabled( validSVG && outlineWidthParam );
  mShapeBorderWidthDDBtn->setEnabled( validSVG && outlineWidthParam );
  if ( mLoadSvgParams && validSVG && outlineWidthParam )
    mShapeBorderWidthSpnBx->setValue( outlineWidth );

  // TODO: fix overriding SVG symbol's border width units in QgsSvgCache
  // currently broken, fall back to symbol's
  //mShapeBorderWidthUnitWidget->setEnabled( validSVG && outlineWidthParam );
  //mShapeBorderUnitsDDBtn->setEnabled( validSVG && outlineWidthParam );
  mShapeSVGUnitsLabel->setEnabled( validSVG && outlineWidthParam );
}

void QgsTextFormatWidget::on_mShapeSVGSelectorBtn_clicked()
{
  QgsSvgSelectorDialog svgDlg( this );
  svgDlg.setWindowTitle( tr( "Select SVG file" ) );
  svgDlg.svgSelector()->setSvgPath( mShapeSVGPathLineEdit->text().trimmed() );

  if ( svgDlg.exec() == QDialog::Accepted )
  {
    QString svgPath = svgDlg.svgSelector()->currentSvgPath();
    if ( !svgPath.isEmpty() )
    {
      mShapeSVGPathLineEdit->setText( svgPath );
    }
  }
}

void QgsTextFormatWidget::on_mShapeSVGParamsBtn_clicked()
{
  QString svgPath = mShapeSVGPathLineEdit->text();
  mLoadSvgParams = true;
  updateSvgWidgets( svgPath );
  mLoadSvgParams = false;
}

void QgsTextFormatWidget::on_mShapeRotationCmbBx_currentIndexChanged( int index )
{
  mShapeRotationDblSpnBx->setEnabled(( QgsTextBackgroundSettings::RotationType )index != QgsTextBackgroundSettings::RotationSync );
  mShapeRotationDDBtn->setEnabled(( QgsTextBackgroundSettings::RotationType )index != QgsTextBackgroundSettings::RotationSync );
}

void QgsTextFormatWidget::on_mPreviewTextEdit_textChanged( const QString & text )
{
  lblFontPreview->setText( text );
  updatePreview();
}

void QgsTextFormatWidget::on_mPreviewTextBtn_clicked()
{
  mPreviewTextEdit->setText( QString( "Lorem Ipsum" ) );
  updatePreview();
}

void QgsTextFormatWidget::on_mPreviewBackgroundBtn_colorChanged( const QColor &color )
{
  setPreviewBackground( color );
}

void QgsTextFormatWidget::on_mDirectSymbLeftToolBtn_clicked()
{
  bool gotChar = false;
  QChar dirSymb = mCharDlg->selectCharacter( &gotChar, mRefFont, mFontDB.styleString( mRefFont ) );

  if ( !gotChar )
    return;

  if ( !dirSymb.isNull() )
    mDirectSymbLeftLineEdit->setText( QString( dirSymb ) );
}

void QgsTextFormatWidget::on_mDirectSymbRightToolBtn_clicked()
{
  bool gotChar = false;
  QChar dirSymb = mCharDlg->selectCharacter( &gotChar, mRefFont, mFontDB.styleString( mRefFont ) );

  if ( !gotChar )
    return;

  if ( !dirSymb.isNull() )
    mDirectSymbRightLineEdit->setText( QString( dirSymb ) );
}

void QgsTextFormatWidget::on_mChkNoObstacle_toggled( bool active )
{
  mPolygonObstacleTypeFrame->setEnabled( active );
  mObstaclePriorityFrame->setEnabled( active );
}

void QgsTextFormatWidget::on_chkLineOrientationDependent_toggled( bool active )
{
  if ( active )
  {
    chkLineAbove->setText( tr( "Left of line" ) );
    chkLineBelow->setText( tr( "Right of line" ) );
  }
  else
  {
    chkLineAbove->setText( tr( "Above line" ) );
    chkLineBelow->setText( tr( "Below line" ) );
  }
}


void QgsTextFormatWidget::on_mToolButtonConfigureSubstitutes_clicked()
{
  QgsPanelWidget* panel = QgsPanelWidget::findParentPanel( this );
  if ( panel && panel->dockMode() )
  {
    QgsSubstitutionListWidget* widget = new QgsSubstitutionListWidget( panel );
    widget->setPanelTitle( tr( "Substitutions" ) );
    widget->setSubstitutions( mSubstitutions );
    connect( widget, SIGNAL( substitutionsChanged( QgsStringReplacementCollection ) ), this, SLOT( onSubstitutionsChanged( QgsStringReplacementCollection ) ) );
    panel->openPanel( widget );
    return;
  }

  QgsSubstitutionListDialog dlg( this );
  dlg.setSubstitutions( mSubstitutions );
  if ( dlg.exec() == QDialog::Accepted )
  {
    mSubstitutions = dlg.substitutions();
    emit widgetChanged();
  }
}

void QgsTextFormatWidget::showBackgroundRadius( bool show )
{
  mShapeRadiusLabel->setVisible( show );
  mShapeRadiusXDbSpnBx->setVisible( show );

  mShapeRadiusYDbSpnBx->setVisible( show );

  mShapeRadiusUnitWidget->setVisible( show );

  mShapeRadiusDDBtn->setVisible( show );
  mShapeRadiusUnitsDDBtn->setVisible( show );
}

void QgsTextFormatWidget::showBackgroundPenStyle( bool show )
{
  mShapePenStyleLabel->setVisible( show );
  mShapePenStyleCmbBx->setVisible( show );

  mShapePenStyleDDBtn->setVisible( show );
}

void QgsTextFormatWidget::enableDataDefinedAlignment( bool enable )
{
  mCoordAlignmentFrame->setEnabled( enable );
}


//
// QgsTextFormatDialog
//

QgsTextFormatDialog::QgsTextFormatDialog( const QgsTextFormat& format, QgsMapCanvas* mapCanvas, QWidget* parent, Qt::WindowFlags fl )
    : QDialog( parent, fl )
{
  setWindowTitle( tr( "Text settings" ) );

  mFormatWidget = new QgsTextFormatWidget( format, mapCanvas, this );
  mFormatWidget->layout()->setContentsMargins( 0, 0, 0, 0 );

  QVBoxLayout *layout = new QVBoxLayout( this );
  layout->addWidget( mFormatWidget );

  QDialogButtonBox *buttonBox = new QDialogButtonBox( QDialogButtonBox::Ok | QDialogButtonBox::Cancel, Qt::Horizontal, this );
  layout->addWidget( buttonBox );

  setLayout( layout );

  QSettings settings;
  restoreGeometry( settings.value( "/Windows/TextFormatDialog/geometry" ).toByteArray() );

  connect( buttonBox->button( QDialogButtonBox::Ok ), SIGNAL( clicked() ), this, SLOT( accept() ) );
  connect( buttonBox->button( QDialogButtonBox::Cancel ), SIGNAL( clicked() ), this, SLOT( reject() ) );
}

QgsTextFormatDialog::~QgsTextFormatDialog()
{
  QSettings settings;
  settings.setValue( "/Windows/TextFormatDialog/geometry", saveGeometry() );
}

QgsTextFormat QgsTextFormatDialog::format() const
{
  return mFormatWidget->format();
}

QgsTextFormatPanelWidget::QgsTextFormatPanelWidget( const QgsTextFormat& format, QgsMapCanvas* mapCanvas, QWidget* parent )
    : QgsPanelWidgetWrapper( new QgsTextFormatWidget( format, mapCanvas ), parent )
{
  mFormatWidget = qobject_cast< QgsTextFormatWidget* >( widget() );
  connect( mFormatWidget, SIGNAL( widgetChanged() ), this, SIGNAL( widgetChanged() ) );
}

QgsTextFormat QgsTextFormatPanelWidget::format() const
{
  return mFormatWidget->format();
}

void QgsTextFormatPanelWidget::setDockMode( bool dockMode )
{
  mFormatWidget->setDockMode( dockMode );
  QgsPanelWidgetWrapper::setDockMode( dockMode );
}
