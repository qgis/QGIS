/***************************************************************************
  qgstextsettingswidget.cpp
  -------------------
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

#include "qgstextsettingswidget.h"

#include "qgsvectorlayer.h"
#include "qgsdatadefinedbutton.h"
#include "qgsfontutils.h"
#include "qgssvgcache.h"
#include "qgssymbollayerv2utils.h"
#include "qgscharacterselectdialog.h"
#include "qgssvgselectorwidget.h"
#include <QSettings>

QgsTextSettingsWidget::QgsTextSettingsWidget( QWidget* parent )
    : QWidget( parent )
    , mLayer( 0 )
    , mDirectSymbBtnGrp( 0 )
    , mUpsidedownBtnGrp( 0 )
    , mLoadingSettings( false )
{
  setupUi( this );
  mFontSizeUnitWidget->setUnits( QStringList() << tr( "points" ) << tr( "map units" ), 1 );
  mBufferUnitWidget->setUnits( QStringList() << tr( "mm" ) << tr( "map units" ), 1 );
  mShapeSizeUnitWidget->setUnits( QStringList() << tr( "mm" ) << tr( "map units" ), 1 );
  mShapeOffsetUnitWidget->setUnits( QStringList() << tr( "mm" ) << tr( "map units" ), 1 );
  mShapeRadiusUnitWidget->setUnits( QStringList() << tr( "mm" ) << tr( "map units" ) << tr( "% of length" ), 1 );
  mShapeBorderWidthUnitWidget->setUnits( QStringList() << tr( "mm" ) << tr( "map units" ), 1 );
  mShadowOffsetUnitWidget->setUnits( QStringList() << tr( "mm" ) << tr( "map units" ), 1 );
  mShadowRadiusUnitWidget->setUnits( QStringList() << tr( "mm" ) << tr( "map units" ), 1 );
  mPointOffsetUnitWidget->setUnits( QStringList() << tr( "mm" ) << tr( "map units" ), 1 );
  mLineDistanceUnitWidget->setUnits( QStringList() << tr( "mm" ) << tr( "map units" ), 1 );
  mRepeatDistanceUnitWidget->setUnits( QStringList() << tr( "mm" ) << tr( "map units" ), 1 );

  mCharDlg = new QgsCharacterSelectorDialog( this );

  mRefFont = lblFontPreview->font();
  mPreviewSize = 10;

  // connections for groupboxes with separate activation checkboxes (that need to honor data defined setting)
  connect( mBufferDrawChkBx, SIGNAL( toggled( bool ) ), this, SLOT( updateUi() ) );
  connect( mShapeDrawChkBx, SIGNAL( toggled( bool ) ), this, SLOT( updateUi() ) );
  connect( mShadowDrawChkBx, SIGNAL( toggled( bool ) ), this, SLOT( updateUi() ) );

  connect( mDirectSymbChkBx, SIGNAL( toggled( bool ) ), this, SLOT( updateUi() ) );
  connect( mFormatNumChkBx, SIGNAL( toggled( bool ) ), this, SLOT( updateUi() ) );
  connect( mScaleBasedVisibilityChkBx, SIGNAL( toggled( bool ) ), this, SLOT( updateUi() ) );
  connect( mFontLimitPixelChkBox, SIGNAL( toggled( bool ) ), this, SLOT( updateUi() ) );

  // preview and basic option connections
  connect( btnTextColor, SIGNAL( colorChanged( const QColor& ) ), this, SLOT( updatePreview() ) );
  connect( mFontTranspSpinBox, SIGNAL( valueChanged( int ) ), this, SLOT( updatePreview() ) );
  connect( comboBlendMode, SIGNAL( currentIndexChanged( int ) ), this, SLOT( updatePreview() ) );
  connect( wrapCharacterEdit, SIGNAL( textEdited( const QString& ) ), this, SLOT( updatePreview() ) );
  connect( mFontLineHeightSpinBox, SIGNAL( valueChanged( double ) ), this, SLOT( updatePreview() ) );
  connect( mFontMultiLineAlignComboBox, SIGNAL( currentIndexChanged( int ) ), this, SLOT( updatePreview() ) );
  connect( mBufferTranspSpinBox, SIGNAL( valueChanged( int ) ), this, SLOT( updatePreview() ) );
  connect( mBufferJoinStyleComboBox, SIGNAL( currentIndexChanged( int ) ), this, SLOT( updatePreview() ) );
  connect( mBufferTranspFillChbx, SIGNAL( toggled( bool ) ), this, SLOT( updatePreview() ) );
  connect( mBufferDrawChkBx, SIGNAL( toggled( bool ) ), this, SLOT( updatePreview() ) );
  connect( btnBufferColor, SIGNAL( colorChanged( const QColor& ) ), this, SLOT( updatePreview() ) );
  connect( spinBufferSize, SIGNAL( valueChanged( double ) ), this, SLOT( updatePreview() ) );
  connect( comboBufferBlendMode, SIGNAL( currentIndexChanged( int ) ), this, SLOT( updatePreview() ) );
  connect( mShapeDrawChkBx, SIGNAL( toggled( bool ) ), this, SLOT( updatePreview() ) );
  connect( mShapeTypeCmbBx, SIGNAL( currentIndexChanged( int ) ), this, SLOT( updatePreview() ) );
  connect( mShapeSVGPathLineEdit, SIGNAL( editingFinished() ), this, SLOT( updatePreview() ) );
  connect( mShapeSizeCmbBx, SIGNAL( currentIndexChanged( int ) ), this, SLOT( updatePreview() ) );
  connect( mShapeSizeXSpnBx, SIGNAL( valueChanged( double ) ), this, SLOT( updatePreview() ) );
  connect( mShapeSizeYSpnBx, SIGNAL( valueChanged( double ) ), this, SLOT( updatePreview() ) );
  connect( mShapeRotationDblSpnBx, SIGNAL( valueChanged( double ) ), this, SLOT( updatePreview() ) );
  connect( mShapeRotationCmbBx, SIGNAL( currentIndexChanged( int ) ), this, SLOT( updatePreview() ) );
  connect( mShapeOffsetXSpnBx, SIGNAL( valueChanged( double ) ), this, SLOT( updatePreview() ) );
  connect( mShapeOffsetYSpnBx, SIGNAL( valueChanged( double ) ), this, SLOT( updatePreview() ) );
  connect( mShapeRadiusXDbSpnBx, SIGNAL( valueChanged( double ) ), this, SLOT( updatePreview() ) );
  connect( mShapeRadiusYDbSpnBx, SIGNAL( valueChanged( double ) ), this, SLOT( updatePreview() ) );
  connect( mShapeTranspSpinBox, SIGNAL( valueChanged( int ) ), this, SLOT( updatePreview() ) );
  connect( mShapeBlendCmbBx, SIGNAL( currentIndexChanged( int ) ), this, SLOT( updatePreview() ) );
  connect( mShapeFillColorBtn, SIGNAL( colorChanged( const QColor& ) ), this, SLOT( updatePreview() ) );
  connect( mShapeBorderColorBtn, SIGNAL( colorChanged( const QColor& ) ), this, SLOT( updatePreview() ) );
  connect( mShapeBorderWidthSpnBx, SIGNAL( valueChanged( double ) ), this, SLOT( updatePreview() ) );
  connect( mShapePenStyleCmbBx, SIGNAL( currentIndexChanged( int ) ), this, SLOT( updatePreview() ) );
  connect( mShadowDrawChkBx, SIGNAL( toggled( bool ) ), this, SLOT( updatePreview() ) );
  connect( mShadowUnderCmbBx, SIGNAL( currentIndexChanged( int ) ), this, SLOT( updatePreview() ) );
  connect( mShadowOffsetAngleSpnBx, SIGNAL( valueChanged( int ) ), this, SLOT( updatePreview() ) );
  connect( mShadowOffsetSpnBx, SIGNAL( valueChanged( double ) ), this, SLOT( updatePreview() ) );
  connect( mShadowOffsetGlobalChkBx, SIGNAL( toggled( bool ) ), this, SLOT( updatePreview() ) );
  connect( mShadowRadiusDblSpnBx, SIGNAL( valueChanged( double ) ), this, SLOT( updatePreview() ) );
  connect( mShadowRadiusAlphaChkBx, SIGNAL( toggled( bool ) ), this, SLOT( updatePreview() ) );
  connect( mShadowTranspSpnBx, SIGNAL( valueChanged( int ) ), this, SLOT( updatePreview() ) );
  connect( mShadowScaleSpnBx, SIGNAL( valueChanged( int ) ), this, SLOT( updatePreview() ) );
  connect( mShadowColorBtn, SIGNAL( colorChanged( const QColor& ) ), this, SLOT( updatePreview() ) );
  connect( mShadowBlendCmbBx, SIGNAL( currentIndexChanged( int ) ), this, SLOT( updatePreview() ) );

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

  populateFontCapitalsComboBox();

  // color buttons
  mPreviewBackgroundBtn->setColorDialogTitle( tr( "Select fill color" ) );
  mPreviewBackgroundBtn->setContext( "labelling" );
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

  // setup point placement button group (assigned enum id currently unused)
  mPlacePointBtnGrp = new QButtonGroup( this );
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
  mPlacePolygonBtnGrp->setExclusive( true );
  connect( mPlacePolygonBtnGrp, SIGNAL( buttonClicked( int ) ), this, SLOT( updatePlacementWidgets() ) );

  // TODO: is this necessary? maybe just use the data defined-only rotation?
  mPointAngleDDBtn->setVisible( false );

  // Global settings group for groupboxes' saved/retored collapsed state
  // maintains state across different dialogs
  foreach ( QgsCollapsibleGroupBox *grpbox, findChildren<QgsCollapsibleGroupBox*>() )
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
}

void QgsTextSettingsWidget::toggleDDButtons( const bool visible )
{
  mFontDDBtn->setVisible( visible );
  mFontStyleDDBtn->setVisible( visible );
  mFontUnderlineDDBtn->setVisible( visible );
  mFontStrikeoutDDBtn->setVisible( visible );
  mFontBoldDDBtn->setVisible( visible );
  mFontItalicDDBtn->setVisible( visible );
  mFontBoldBtn->setVisible( visible );
  mFontItalicBtn->setVisible( visible );
  mFontSizeDDBtn->setVisible( visible );
  mFontUnitsDDBtn->setVisible( visible );
  mFontColorDDBtn->setVisible( visible );
  mFontTranspDDBtn->setVisible( visible );
  mFontCaseDDBtn->setVisible( visible );
  mFontLetterSpacingDDBtn->setVisible( visible );
  mFontWordSpacingDDBtn->setVisible( visible );
  mFontBlendModeDDBtn->setVisible( visible );
  mWrapCharDDBtn->setVisible( visible );
  mFontLineHeightDDBtn->setVisible( visible );
  mFontMultiLineAlignDDBtn->setVisible( visible );
  mDirectSymbDDBtn->setVisible( visible );
  mDirectSymbLeftDDBtn->setVisible( visible );
  mDirectSymbRightDDBtn->setVisible( visible );
  mDirectSymbPlacementDDBtn->setVisible( visible );
  mDirectSymbRevDDBtn->setVisible( visible );
  mFormatNumDDBtn->setVisible( visible );
  mFormatNumDecimalsDDBtn->setVisible( visible );
  mFormatNumPlusSignDDBtn->setVisible( visible );
  mBufferDrawDDBtn->setVisible( visible );
  mBufferSizeDDBtn->setVisible( visible );
  mBufferUnitsDDBtn->setVisible( visible );
  mBufferColorDDBtn->setVisible( visible );
  mBufferTranspDDBtn->setVisible( visible );
  mBufferJoinStyleDDBtn->setVisible( visible );
  mBufferBlendModeDDBtn->setVisible( visible );
  mShapeDrawDDBtn->setVisible( visible );
  mShapeTypeDDBtn->setVisible( visible );
  mShapeSVGPathDDBtn->setVisible( visible );
  mShapeSizeTypeDDBtn->setVisible( visible );
  mShapeSizeXDDBtn->setVisible( visible );
  mShapeSizeYDDBtn->setVisible( visible );
  mShapeSizeUnitsDDBtn->setVisible( visible );
  mShapeRotationTypeDDBtn->setVisible( visible );
  mShapeRotationDDBtn->setVisible( visible );
  mShapeOffsetDDBtn->setVisible( visible );
  mShapeOffsetUnitsDDBtn->setVisible( visible );
  mShapeRadiusDDBtn->setVisible( visible );
  mShapeRadiusUnitsDDBtn->setVisible( visible );
  mShapeTranspDDBtn->setVisible( visible );
  mShapeBlendModeDDBtn->setVisible( visible );
  mShapeFillColorDDBtn->setVisible( visible );
  mShapeBorderColorDDBtn->setVisible( visible );
  mShapeBorderWidthDDBtn->setVisible( visible );
  mShapeBorderUnitsDDBtn->setVisible( visible );
  mShapePenStyleDDBtn->setVisible( visible );
  mShadowDrawDDBtn->setVisible( visible );
  mShadowUnderDDBtn->setVisible( visible );
  mShadowOffsetAngleDDBtn->setVisible( visible );
  mShadowOffsetDDBtn->setVisible( visible );
  mShadowOffsetUnitsDDBtn->setVisible( visible );
  mShadowRadiusDDBtn->setVisible( visible );
  mShadowRadiusUnitsDDBtn->setVisible( visible );
  mShadowTranspDDBtn->setVisible( visible );
  mShadowScaleDDBtn->setVisible( visible );
  mShadowColorDDBtn->setVisible( visible );
  mShadowBlendDDBtn->setVisible( visible );
  mCentroidDDBtn->setVisible( visible );
  mPointQuadOffsetDDBtn->setVisible( visible );
  mPointOffsetDDBtn->setVisible( visible );
  mPointOffsetUnitsDDBtn->setVisible( visible );
  mLineDistanceDDBtn->setVisible( visible );
  mLineDistanceUnitDDBtn->setVisible( visible );
  mMaxCharAngleDDBtn->setVisible( visible );
  mRepeatDistanceDDBtn->setVisible( visible );
  mRepeatDistanceUnitDDBtn->setVisible( visible );
  mCoordXDDBtn->setVisible( visible );
  mCoordYDDBtn->setVisible( visible );
  mCoordAlignmentHDDBtn->setVisible( visible );
  mCoordAlignmentVDDBtn->setVisible( visible );
  mCoordRotationDDBtn->setVisible( visible );
  mScaleBasedVisibilityDDBtn->setVisible( visible );
  mScaleBasedVisibilityMinDDBtn->setVisible( visible );
  mScaleBasedVisibilityMaxDDBtn->setVisible( visible );
  mFontLimitPixelDDBtn->setVisible( visible );
  mFontMinPixelDDBtn->setVisible( visible );
  mFontMaxPixelDDBtn->setVisible( visible );
  mShowLabelDDBtn->setVisible( visible );
  mAlwaysShowDDBtn->setVisible( visible );
}

void QgsTextSettingsWidget::setLayer( QgsVectorLayer *layer )
{
  mLayer = layer;

  if ( !mLayer )
  {
    return;
  }
  // set placement methods page based on geometry type
  switch ( mLayer->geometryType() )
  {
    case QGis::Point:
      stackedPlacement->setCurrentWidget( pagePoint );
      break;
    case QGis::Line:
      stackedPlacement->setCurrentWidget( pageLine );
      break;
    case QGis::Polygon:
      stackedPlacement->setCurrentWidget( pagePolygon );
      break;
    case QGis::NoGeometry:
      break;
    case QGis::UnknownGeometry:
      qFatal( "unknown geometry type unexpected" );
  }
  // show/hide options based upon geometry type
  chkMergeLines->setVisible( mLayer->geometryType() == QGis::Line );
  mDirectSymbolsFrame->setVisible( mLayer->geometryType() == QGis::Line );
  mMinSizeFrame->setVisible( mLayer->geometryType() != QGis::Point );

  // load labeling settings from layer
  QgsPalLayerSettings layerSettings;
  layerSettings.readFromLayer( mLayer );
  loadSettings( &layerSettings );
  populateDataDefinedButtons( layerSettings );
}

void QgsTextSettingsWidget::loadSettings( QgsTextRendererSettings* settings )
{
  if ( !settings )
  {
    return;
  }

  blockInitSignals( true );
  mLoadingSettings = true;

  //try casting to pal settings for pal specific options
  QgsPalLayerSettings* palSettings = dynamic_cast<QgsPalLayerSettings*>( settings );

  //hide pal specific pages if not using pal settings
  if ( !palSettings )
  {
    delete mLabelingOptionsListWidget->takeItem( 6 );
    delete mLabelingOptionsListWidget->takeItem( 5 );

    mDirectSymbolsFrame->hide();
    mFormatNumFrame->hide();
    mFormatNumChkBx->hide();
    mFormatNumDDBtn->hide();
    //hide data defined buttons
    toggleDDButtons( false );
  }

  // populate placement options
  if ( palSettings )
  {
    mCentroidRadioWhole->setChecked( palSettings->centroidWhole );
    mCentroidInsideCheckBox->setChecked( palSettings->centroidInside );
    switch ( palSettings->placement )
    {
      case QgsPalLayerSettings::AroundPoint:
        radAroundPoint->setChecked( true );
        radAroundCentroid->setChecked( true );

        mLineDistanceSpnBx->setValue( palSettings->dist );
        mLineDistanceUnitWidget->setUnit( palSettings->distInMapUnits ? 1 : 0 );
        mLineDistanceUnitWidget->setMapUnitScale( palSettings->distMapUnitScale );
        //spinAngle->setValue( palSettings->angle ); // TODO: uncomment when supported
        break;
      case QgsPalLayerSettings::OverPoint:
        radOverPoint->setChecked( true );
        radOverCentroid->setChecked( true );

        mQuadrantBtnGrp->button(( int )palSettings->quadOffset )->setChecked( true );
        mPointOffsetXSpinBox->setValue( palSettings->xOffset );
        mPointOffsetYSpinBox->setValue( palSettings->yOffset );
        mPointOffsetUnitWidget->setUnit( palSettings->labelOffsetInMapUnits ? 1 : 0 );
        mPointOffsetUnitWidget->setMapUnitScale( palSettings->labelOffsetMapUnitScale );
        mPointAngleSpinBox->setValue( palSettings->angleOffset );
        break;
      case QgsPalLayerSettings::Line:
        radLineParallel->setChecked( true );
        radPolygonPerimeter->setChecked( true );
        break;
      case QgsPalLayerSettings::Curved:
        radLineCurved->setChecked( true );
        break;
      case QgsPalLayerSettings::Horizontal:
        radPolygonHorizontal->setChecked( true );
        radLineHorizontal->setChecked( true );
        break;
      case QgsPalLayerSettings::Free:
        radPolygonFree->setChecked( true );
        break;
    }


    if ( palSettings->placement == QgsPalLayerSettings::Line || palSettings->placement == QgsPalLayerSettings::Curved )
    {
      mLineDistanceSpnBx->setValue( palSettings->dist );
      mLineDistanceUnitWidget->setUnit( palSettings->distInMapUnits ? 1 : 0 );
      mLineDistanceUnitWidget->setMapUnitScale( palSettings->distMapUnitScale );
      chkLineAbove->setChecked( palSettings->placementFlags & QgsPalLayerSettings::AboveLine );
      chkLineBelow->setChecked( palSettings->placementFlags & QgsPalLayerSettings::BelowLine );
      chkLineOn->setChecked( palSettings->placementFlags & QgsPalLayerSettings::OnLine );
      if ( !( palSettings->placementFlags & QgsPalLayerSettings::MapOrientation ) )
        chkLineOrientationDependent->setChecked( true );
    }


    // Label repeat distance
    mRepeatDistanceSpinBox->setValue( palSettings->repeatDistance );
    mRepeatDistanceUnitWidget->setUnit( palSettings->repeatDistanceUnit - 1 );
    mRepeatDistanceUnitWidget->setMapUnitScale( palSettings->repeatDistanceMapUnitScale );

    mPrioritySlider->setValue( palSettings->priority );
    chkNoObstacle->setChecked( palSettings->obstacle );
    chkLabelPerFeaturePart->setChecked( palSettings->labelPerPart );
    mPalShowAllLabelsForLayerChkBx->setChecked( palSettings->displayAll );
    chkMergeLines->setChecked( palSettings->mergeLines );
    mMinSizeSpinBox->setValue( palSettings->minFeatureSize );
    mLimitLabelChkBox->setChecked( palSettings->limitNumLabels );
    mLimitLabelSpinBox->setValue( palSettings->maxNumLabels );

    // direction symbol(s)
    mDirectSymbChkBx->setChecked( palSettings->addDirectionSymbol );
    mDirectSymbLeftLineEdit->setText( palSettings->leftDirectionSymbol );
    mDirectSymbRightLineEdit->setText( palSettings->rightDirectionSymbol );
    mDirectSymbRevChkBx->setChecked( palSettings->reverseDirectionSymbol );

    mDirectSymbBtnGrp->button(( int )palSettings->placeDirectionSymbol )->setChecked( true );
    mUpsidedownBtnGrp->button(( int )palSettings->upsidedownLabels )->setChecked( true );

    // curved label max character angles
    mMaxCharAngleInDSpinBox->setValue( palSettings->maxCurvedCharAngleIn );
    // settings->maxCurvedCharAngleOut must be negative, but it is shown as positive spinbox in GUI
    mMaxCharAngleOutDSpinBox->setValue( qAbs( palSettings->maxCurvedCharAngleOut ) );

    chkPreserveRotation->setChecked( palSettings->preserveRotation );
    mScaleBasedVisibilityChkBx->setChecked( palSettings->scaleVisibility );
    mScaleBasedVisibilityMinSpnBx->setValue( palSettings->scaleMin );
    mScaleBasedVisibilityMaxSpnBx->setValue( palSettings->scaleMax );

    mFormatNumChkBx->setChecked( palSettings->formatNumbers );
    mFormatNumDecimalsSpnBx->setValue( palSettings->decimals );
    mFormatNumPlusSignChkBx->setChecked( palSettings->plusSign );

    // set pixel size limiting checked state before unit choice so limiting can be
    // turned on as a default for map units, if minimum trigger value of 0 is used
    mFontLimitPixelChkBox->setChecked( palSettings->fontLimitPixelSize );
    mMinPixelLimit = palSettings->fontMinPixelSize; // ignored after first settings save
    mFontMinPixelSpinBox->setValue( palSettings->fontMinPixelSize == 0 ? 3 : palSettings->fontMinPixelSize );
    mFontMaxPixelSpinBox->setValue( palSettings->fontMaxPixelSize );
  }

  wrapCharacterEdit->setText( settings->wrapChar );
  mFontLineHeightSpinBox->setValue( settings->multilineHeight );
  mFontMultiLineAlignComboBox->setCurrentIndex(( unsigned int ) settings->multilineAlign );

  mPreviewBackgroundBtn->setColor( settings->previewBkgrdColor );
  mPreviewBackgroundBtn->setDefaultColor( settings->previewBkgrdColor );
  setPreviewBackground( settings->previewBkgrdColor );

  // buffer
  mBufferDrawChkBx->setChecked( settings->bufferDraw );
  spinBufferSize->setValue( settings->bufferSize );
  mBufferUnitWidget->setUnit( settings->bufferSizeInMapUnits ? 1 : 0 );
  mBufferUnitWidget->setMapUnitScale( settings->bufferSizeMapUnitScale );
  btnBufferColor->setColor( settings->bufferColor );
  mBufferTranspSpinBox->setValue( settings->bufferTransp );
  mBufferJoinStyleComboBox->setPenJoinStyle( settings->bufferJoinStyle );
  mBufferTranspFillChbx->setChecked( !settings->bufferNoFill );
  comboBufferBlendMode->setBlendMode( settings->bufferBlendMode );

  mFontSizeUnitWidget->setUnit( settings->fontSizeInMapUnits ? 1 : 0 );
  mFontSizeUnitWidget->setMapUnitScale( settings->fontSizeMapUnitScale );

  mRefFont = settings->textFont;
  mFontSizeSpinBox->setValue( settings->textFont.pointSizeF() );
  btnTextColor->setColor( settings->textColor );
  mFontTranspSpinBox->setValue( settings->textTransp );
  comboBlendMode->setBlendMode( settings->blendMode );

  mFontWordSpacingSpinBox->setValue( settings->textFont.wordSpacing() );
  mFontLetterSpacingSpinBox->setValue( settings->textFont.letterSpacing() );

  QgsFontUtils::updateFontViaStyle( mRefFont, settings->textNamedStyle );
  updateFont( mRefFont );

  // show 'font not found' if substitution has occurred (should come after updateFont())
  mFontMissingLabel->setVisible( !settings->mTextFontFound );
  if ( !settings->mTextFontFound )
  {
    QString missingTxt = tr( "%1 not found. Default substituted." );
    QString txtPrepend = tr( "Chosen font" );
    if ( !settings->mTextFontFamily.isEmpty() )
    {
      txtPrepend = QString( "'%1'" ).arg( settings->mTextFontFamily );
    }
    mFontMissingLabel->setText( missingTxt.arg( txtPrepend ) );

    // ensure user is sent to 'Text style' section to see notice
    mLabelingOptionsListWidget->setCurrentRow( 0 );
  }

  // shape background
  mShapeDrawChkBx->setChecked( settings->shapeDraw );
  mShapeTypeCmbBx->blockSignals( true );
  mShapeTypeCmbBx->setCurrentIndex( settings->shapeType );
  mShapeTypeCmbBx->blockSignals( false );
  mShapeSVGPathLineEdit->setText( settings->shapeSVGFile );

  mShapeSizeCmbBx->setCurrentIndex( settings->shapeSizeType );
  mShapeSizeXSpnBx->setValue( settings->shapeSize.x() );
  mShapeSizeYSpnBx->setValue( settings->shapeSize.y() );
  mShapeSizeUnitWidget->setUnit( settings->shapeSizeUnits - 1 );
  mShapeSizeUnitWidget->setMapUnitScale( settings->shapeSizeMapUnitScale );
  mShapeRotationCmbBx->setCurrentIndex( settings->shapeRotationType );
  mShapeRotationDblSpnBx->setEnabled( settings->shapeRotationType != QgsPalLayerSettings::RotationSync );
  mShapeRotationDDBtn->setEnabled( settings->shapeRotationType != QgsPalLayerSettings::RotationSync );
  mShapeRotationDblSpnBx->setValue( settings->shapeRotation );
  mShapeOffsetXSpnBx->setValue( settings->shapeOffset.x() );
  mShapeOffsetYSpnBx->setValue( settings->shapeOffset.y() );
  mShapeOffsetUnitWidget->setUnit( settings->shapeOffsetUnits - 1 );
  mShapeOffsetUnitWidget->setMapUnitScale( settings->shapeOffsetMapUnitScale );
  mShapeRadiusXDbSpnBx->setValue( settings->shapeRadii.x() );
  mShapeRadiusYDbSpnBx->setValue( settings->shapeRadii.y() );
  mShapeRadiusUnitWidget->setUnit( settings->shapeRadiiUnits - 1 );
  mShapeRadiusUnitWidget->setMapUnitScale( settings->shapeRadiiMapUnitScale );

  mShapeFillColorBtn->setColor( settings->shapeFillColor );
  mShapeBorderColorBtn->setColor( settings->shapeBorderColor );
  mShapeBorderWidthSpnBx->setValue( settings->shapeBorderWidth );
  mShapeBorderWidthUnitWidget->setUnit( settings->shapeBorderWidthUnits - 1 );
  mShapeBorderWidthUnitWidget->setMapUnitScale( settings->shapeBorderWidthMapUnitScale );
  mShapePenStyleCmbBx->setPenJoinStyle( settings->shapeJoinStyle );

  mShapeTranspSpinBox->setValue( settings->shapeTransparency );
  mShapeBlendCmbBx->setBlendMode( settings->shapeBlendMode );

  mLoadSvgParams = false;
  on_mShapeTypeCmbBx_currentIndexChanged( settings->shapeType ); // force update of shape background gui

  // drop shadow
  mShadowDrawChkBx->setChecked( settings->shadowDraw );
  mShadowUnderCmbBx->setCurrentIndex( settings->shadowUnder );
  mShadowOffsetAngleSpnBx->setValue( settings->shadowOffsetAngle );
  mShadowOffsetSpnBx->setValue( settings->shadowOffsetDist );
  mShadowOffsetUnitWidget->setUnit( settings->shadowOffsetUnits - 1 );
  mShadowOffsetUnitWidget->setMapUnitScale( settings->shadowOffsetMapUnitScale );
  mShadowOffsetGlobalChkBx->setChecked( settings->shadowOffsetGlobal );

  mShadowRadiusDblSpnBx->setValue( settings->shadowRadius );
  mShadowRadiusUnitWidget->setUnit( settings->shadowRadiusUnits - 1 );
  mShadowRadiusUnitWidget->setMapUnitScale( settings->shadowRadiusMapUnitScale );
  mShadowRadiusAlphaChkBx->setChecked( settings->shadowRadiusAlphaOnly );
  mShadowTranspSpnBx->setValue( settings->shadowTransparency );
  mShadowScaleSpnBx->setValue( settings->shadowScale );

  mShadowColorBtn->setColor( settings->shadowColor );
  mShadowBlendCmbBx->setBlendMode( settings->shadowBlendMode );

  updatePlacementWidgets();

  // needs to come before data defined setup, so connections work
  blockInitSignals( false );

  // set up data defined toolbuttons
  // do this after other widgets are configured, so they can be enabled/disabled
  if ( palSettings )
  {
    populateDataDefinedButtons( *palSettings );
  }

  enableDataDefinedAlignment( mCoordXDDBtn->isActive() && mCoordYDDBtn->isActive() );

  mLoadingSettings = false;

  updateUi(); // should come after data defined button setup
  updatePreview();
}

void QgsTextSettingsWidget::saveToSettings( QgsTextRendererSettings *settings )
{
  if ( !settings )
  {
    return;
  }

  //try casting to pal settings for pal specific options
  QgsPalLayerSettings* palSettings = dynamic_cast<QgsPalLayerSettings*>( settings );

  //save pal specific settings
  if ( palSettings )
  {

    palSettings->dist = 0;
    palSettings->placementFlags = 0;

    QWidget* curPlacementWdgt = stackedPlacement->currentWidget();
    palSettings->centroidWhole = mCentroidRadioWhole->isChecked();
    palSettings->centroidInside = mCentroidInsideCheckBox->isChecked();
    if (( curPlacementWdgt == pagePoint && radAroundPoint->isChecked() )
        || ( curPlacementWdgt == pagePolygon && radAroundCentroid->isChecked() ) )
    {
      palSettings->placement = QgsPalLayerSettings::AroundPoint;
      palSettings->dist = mLineDistanceSpnBx->value();
      palSettings->distInMapUnits = ( mLineDistanceUnitWidget->getUnit() == 1 );
      palSettings->distMapUnitScale = mLineDistanceUnitWidget->getMapUnitScale();
    }
    else if (( curPlacementWdgt == pagePoint && radOverPoint->isChecked() )
             || ( curPlacementWdgt == pagePolygon && radOverCentroid->isChecked() ) )
    {
      palSettings->placement = QgsPalLayerSettings::OverPoint;
      palSettings->quadOffset = ( QgsPalLayerSettings::QuadrantPosition )mQuadrantBtnGrp->checkedId();
      palSettings->xOffset = mPointOffsetXSpinBox->value();
      palSettings->yOffset = mPointOffsetYSpinBox->value();
      palSettings->labelOffsetInMapUnits = ( mPointOffsetUnitWidget->getUnit() == 1 );
      palSettings->labelOffsetMapUnitScale = mPointOffsetUnitWidget->getMapUnitScale();
      palSettings->angleOffset = mPointAngleSpinBox->value();
    }
    else if (( curPlacementWdgt == pageLine && radLineParallel->isChecked() )
             || ( curPlacementWdgt == pagePolygon && radPolygonPerimeter->isChecked() )
             || ( curPlacementWdgt == pageLine && radLineCurved->isChecked() ) )
    {
      bool curved = ( curPlacementWdgt == pageLine && radLineCurved->isChecked() );
      palSettings->placement = ( curved ? QgsPalLayerSettings::Curved : QgsPalLayerSettings::Line );
      palSettings->dist = mLineDistanceSpnBx->value();
      palSettings->distInMapUnits = ( mLineDistanceUnitWidget->getUnit() == 1 );
      palSettings->distMapUnitScale = mLineDistanceUnitWidget->getMapUnitScale();
      if ( chkLineAbove->isChecked() )
        palSettings->placementFlags |= QgsPalLayerSettings::AboveLine;
      if ( chkLineBelow->isChecked() )
        palSettings->placementFlags |= QgsPalLayerSettings::BelowLine;
      if ( chkLineOn->isChecked() )
        palSettings->placementFlags |= QgsPalLayerSettings::OnLine;

      if ( ! chkLineOrientationDependent->isChecked() )
        palSettings->placementFlags |= QgsPalLayerSettings::MapOrientation;
    }
    else if (( curPlacementWdgt == pageLine && radLineHorizontal->isChecked() )
             || ( curPlacementWdgt == pagePolygon && radPolygonHorizontal->isChecked() ) )
    {
      palSettings->placement = QgsPalLayerSettings::Horizontal;
    }
    else if ( radPolygonFree->isChecked() )
    {
      palSettings->placement = QgsPalLayerSettings::Free;
    }
    else
    {
      qFatal( "Invalid settings" );
    }

    palSettings->repeatDistance = mRepeatDistanceSpinBox->value();
    palSettings->repeatDistanceUnit = static_cast<QgsPalLayerSettings::SizeUnit>( 1 + mRepeatDistanceUnitWidget->getUnit() );
    palSettings->repeatDistanceMapUnitScale = mRepeatDistanceUnitWidget->getMapUnitScale();

    // format numbers
    palSettings->formatNumbers = mFormatNumChkBx->isChecked();
    palSettings->decimals = mFormatNumDecimalsSpnBx->value();
    palSettings->plusSign = mFormatNumPlusSignChkBx->isChecked();

    // direction symbol(s)
    palSettings->addDirectionSymbol = mDirectSymbChkBx->isChecked();
    palSettings->leftDirectionSymbol = mDirectSymbLeftLineEdit->text();
    palSettings->rightDirectionSymbol = mDirectSymbRightLineEdit->text();
    palSettings->reverseDirectionSymbol = mDirectSymbRevChkBx->isChecked();

    palSettings->priority = mPrioritySlider->value();
    palSettings->obstacle = chkNoObstacle->isChecked();
    palSettings->labelPerPart = chkLabelPerFeaturePart->isChecked();
    palSettings->displayAll = mPalShowAllLabelsForLayerChkBx->isChecked();
    palSettings->mergeLines = chkMergeLines->isChecked();

    palSettings->scaleVisibility = mScaleBasedVisibilityChkBx->isChecked();
    palSettings->scaleMin = mScaleBasedVisibilityMinSpnBx->value();
    palSettings->scaleMax = mScaleBasedVisibilityMaxSpnBx->value();

    if ( mDirectSymbBtnGrp )
    {
      palSettings->placeDirectionSymbol = ( QgsPalLayerSettings::DirectionSymbols )mDirectSymbBtnGrp->checkedId();
    }
    if ( mDirectSymbBtnGrp )
    {
      palSettings->upsidedownLabels = ( QgsPalLayerSettings::UpsideDownLabels )mUpsidedownBtnGrp->checkedId();
    }

    palSettings->maxCurvedCharAngleIn = mMaxCharAngleInDSpinBox->value();
    // settings->maxCurvedCharAngleOut must be negative, but it is shown as positive spinbox in GUI
    palSettings->maxCurvedCharAngleOut = -mMaxCharAngleOutDSpinBox->value();

    palSettings->minFeatureSize = mMinSizeSpinBox->value();
    palSettings->limitNumLabels = mLimitLabelChkBox->isChecked();
    palSettings->maxNumLabels = mLimitLabelSpinBox->value();

    palSettings->fontLimitPixelSize = mFontLimitPixelChkBox->isChecked();
    palSettings->fontMinPixelSize = mFontMinPixelSpinBox->value();
    palSettings->fontMaxPixelSize = mFontMaxPixelSpinBox->value();

    palSettings->preserveRotation = chkPreserveRotation->isChecked();

    // data defined labeling
    // text style
    setDataDefinedProperty( mFontDDBtn, QgsPalLayerSettings::Family, *palSettings );
    setDataDefinedProperty( mFontStyleDDBtn, QgsPalLayerSettings::FontStyle, *palSettings );
    setDataDefinedProperty( mFontUnderlineDDBtn, QgsPalLayerSettings::Underline, *palSettings );
    setDataDefinedProperty( mFontStrikeoutDDBtn, QgsPalLayerSettings::Strikeout, *palSettings );
    setDataDefinedProperty( mFontBoldDDBtn, QgsPalLayerSettings::Bold, *palSettings );
    setDataDefinedProperty( mFontItalicDDBtn, QgsPalLayerSettings::Italic, *palSettings );
    setDataDefinedProperty( mFontSizeDDBtn, QgsPalLayerSettings::Size, *palSettings );
    setDataDefinedProperty( mFontUnitsDDBtn, QgsPalLayerSettings::FontSizeUnit, *palSettings );
    setDataDefinedProperty( mFontColorDDBtn, QgsPalLayerSettings::Color, *palSettings );
    setDataDefinedProperty( mFontTranspDDBtn, QgsPalLayerSettings::FontTransp, *palSettings );
    setDataDefinedProperty( mFontCaseDDBtn, QgsPalLayerSettings::FontCase, *palSettings );
    setDataDefinedProperty( mFontLetterSpacingDDBtn, QgsPalLayerSettings::FontLetterSpacing, *palSettings );
    setDataDefinedProperty( mFontWordSpacingDDBtn, QgsPalLayerSettings::FontWordSpacing, *palSettings );
    setDataDefinedProperty( mFontBlendModeDDBtn, QgsPalLayerSettings::FontBlendMode, *palSettings );

    // text formatting
    setDataDefinedProperty( mWrapCharDDBtn, QgsPalLayerSettings::MultiLineWrapChar, *palSettings );
    setDataDefinedProperty( mFontLineHeightDDBtn, QgsPalLayerSettings::MultiLineHeight, *palSettings );
    setDataDefinedProperty( mFontMultiLineAlignDDBtn, QgsPalLayerSettings::MultiLineAlignment, *palSettings );
    setDataDefinedProperty( mDirectSymbDDBtn, QgsPalLayerSettings::DirSymbDraw, *palSettings );
    setDataDefinedProperty( mDirectSymbLeftDDBtn, QgsPalLayerSettings::DirSymbLeft, *palSettings );
    setDataDefinedProperty( mDirectSymbRightDDBtn, QgsPalLayerSettings::DirSymbRight, *palSettings );
    setDataDefinedProperty( mDirectSymbPlacementDDBtn, QgsPalLayerSettings::DirSymbPlacement, *palSettings );
    setDataDefinedProperty( mDirectSymbRevDDBtn, QgsPalLayerSettings::DirSymbReverse, *palSettings );
    setDataDefinedProperty( mFormatNumDDBtn, QgsPalLayerSettings::NumFormat, *palSettings );
    setDataDefinedProperty( mFormatNumDecimalsDDBtn, QgsPalLayerSettings::NumDecimals, *palSettings );
    setDataDefinedProperty( mFormatNumPlusSignDDBtn, QgsPalLayerSettings::NumPlusSign, *palSettings );

    // text buffer
    setDataDefinedProperty( mBufferDrawDDBtn, QgsPalLayerSettings::BufferDraw, *palSettings );
    setDataDefinedProperty( mBufferSizeDDBtn, QgsPalLayerSettings::BufferSize, *palSettings );
    setDataDefinedProperty( mBufferUnitsDDBtn, QgsPalLayerSettings::BufferUnit, *palSettings );
    setDataDefinedProperty( mBufferColorDDBtn, QgsPalLayerSettings::BufferColor, *palSettings );
    setDataDefinedProperty( mBufferTranspDDBtn, QgsPalLayerSettings::BufferTransp, *palSettings );
    setDataDefinedProperty( mBufferJoinStyleDDBtn, QgsPalLayerSettings::BufferJoinStyle, *palSettings );
    setDataDefinedProperty( mBufferBlendModeDDBtn, QgsPalLayerSettings::BufferBlendMode, *palSettings );

    // background
    setDataDefinedProperty( mShapeDrawDDBtn, QgsPalLayerSettings::ShapeDraw, *palSettings );
    setDataDefinedProperty( mShapeTypeDDBtn, QgsPalLayerSettings::ShapeKind, *palSettings );
    setDataDefinedProperty( mShapeSVGPathDDBtn, QgsPalLayerSettings::ShapeSVGFile, *palSettings );
    setDataDefinedProperty( mShapeSizeTypeDDBtn, QgsPalLayerSettings::ShapeSizeType, *palSettings );
    setDataDefinedProperty( mShapeSizeXDDBtn, QgsPalLayerSettings::ShapeSizeX, *palSettings );
    setDataDefinedProperty( mShapeSizeYDDBtn, QgsPalLayerSettings::ShapeSizeY, *palSettings );
    setDataDefinedProperty( mShapeSizeUnitsDDBtn, QgsPalLayerSettings::ShapeSizeUnits, *palSettings );
    setDataDefinedProperty( mShapeRotationTypeDDBtn, QgsPalLayerSettings::ShapeRotationType, *palSettings );
    setDataDefinedProperty( mShapeRotationDDBtn, QgsPalLayerSettings::ShapeRotation, *palSettings );
    setDataDefinedProperty( mShapeOffsetDDBtn, QgsPalLayerSettings::ShapeOffset, *palSettings );
    setDataDefinedProperty( mShapeOffsetUnitsDDBtn, QgsPalLayerSettings::ShapeOffsetUnits, *palSettings );
    setDataDefinedProperty( mShapeRadiusDDBtn, QgsPalLayerSettings::ShapeRadii, *palSettings );
    setDataDefinedProperty( mShapeRadiusUnitsDDBtn, QgsPalLayerSettings::ShapeRadiiUnits, *palSettings );
    setDataDefinedProperty( mShapeTranspDDBtn, QgsPalLayerSettings::ShapeTransparency, *palSettings );
    setDataDefinedProperty( mShapeBlendModeDDBtn, QgsPalLayerSettings::ShapeBlendMode, *palSettings );
    setDataDefinedProperty( mShapeFillColorDDBtn, QgsPalLayerSettings::ShapeFillColor, *palSettings );
    setDataDefinedProperty( mShapeBorderColorDDBtn, QgsPalLayerSettings::ShapeBorderColor, *palSettings );
    setDataDefinedProperty( mShapeBorderWidthDDBtn, QgsPalLayerSettings::ShapeBorderWidth, *palSettings );
    setDataDefinedProperty( mShapeBorderUnitsDDBtn, QgsPalLayerSettings::ShapeBorderWidthUnits, *palSettings );
    setDataDefinedProperty( mShapePenStyleDDBtn, QgsPalLayerSettings::ShapeJoinStyle, *palSettings );

    // drop shadow
    setDataDefinedProperty( mShadowDrawDDBtn, QgsPalLayerSettings::ShadowDraw, *palSettings );
    setDataDefinedProperty( mShadowUnderDDBtn, QgsPalLayerSettings::ShadowUnder, *palSettings );
    setDataDefinedProperty( mShadowOffsetAngleDDBtn, QgsPalLayerSettings::ShadowOffsetAngle, *palSettings );
    setDataDefinedProperty( mShadowOffsetDDBtn, QgsPalLayerSettings::ShadowOffsetDist, *palSettings );
    setDataDefinedProperty( mShadowOffsetUnitsDDBtn, QgsPalLayerSettings::ShadowOffsetUnits, *palSettings );
    setDataDefinedProperty( mShadowRadiusDDBtn, QgsPalLayerSettings::ShadowRadius, *palSettings );
    setDataDefinedProperty( mShadowRadiusUnitsDDBtn, QgsPalLayerSettings::ShadowRadiusUnits, *palSettings );
    setDataDefinedProperty( mShadowTranspDDBtn, QgsPalLayerSettings::ShadowTransparency, *palSettings );
    setDataDefinedProperty( mShadowScaleDDBtn, QgsPalLayerSettings::ShadowScale, *palSettings );
    setDataDefinedProperty( mShadowColorDDBtn, QgsPalLayerSettings::ShadowColor, *palSettings );
    setDataDefinedProperty( mShadowBlendDDBtn, QgsPalLayerSettings::ShadowBlendMode, *palSettings );

    // placement
    setDataDefinedProperty( mCentroidDDBtn, QgsPalLayerSettings::CentroidWhole, *palSettings );
    setDataDefinedProperty( mPointQuadOffsetDDBtn, QgsPalLayerSettings::OffsetQuad, *palSettings );
    setDataDefinedProperty( mPointOffsetDDBtn, QgsPalLayerSettings::OffsetXY, *palSettings );
    setDataDefinedProperty( mPointOffsetUnitsDDBtn, QgsPalLayerSettings::OffsetUnits, *palSettings );
    setDataDefinedProperty( mLineDistanceDDBtn, QgsPalLayerSettings::LabelDistance, *palSettings );
    setDataDefinedProperty( mLineDistanceUnitDDBtn, QgsPalLayerSettings::DistanceUnits, *palSettings );
    // TODO: is this necessary? maybe just use the data defined-only rotation?
    //setDataDefinedProperty( mPointAngleDDBtn, QgsPalLayerSettings::OffsetRotation, *palSettings );
    setDataDefinedProperty( mMaxCharAngleDDBtn, QgsPalLayerSettings::CurvedCharAngleInOut, *palSettings );
    setDataDefinedProperty( mRepeatDistanceDDBtn, QgsPalLayerSettings::RepeatDistance, *palSettings );
    setDataDefinedProperty( mRepeatDistanceUnitDDBtn, QgsPalLayerSettings::RepeatDistanceUnit, *palSettings );

    // data defined-only
    setDataDefinedProperty( mCoordXDDBtn, QgsPalLayerSettings::PositionX, *palSettings );
    setDataDefinedProperty( mCoordYDDBtn, QgsPalLayerSettings::PositionY, *palSettings );
    setDataDefinedProperty( mCoordAlignmentHDDBtn, QgsPalLayerSettings::Hali, *palSettings );
    setDataDefinedProperty( mCoordAlignmentVDDBtn, QgsPalLayerSettings::Vali, *palSettings );
    setDataDefinedProperty( mCoordRotationDDBtn, QgsPalLayerSettings::Rotation, *palSettings );

    // rendering
    setDataDefinedProperty( mScaleBasedVisibilityDDBtn, QgsPalLayerSettings::ScaleVisibility, *palSettings );
    setDataDefinedProperty( mScaleBasedVisibilityMinDDBtn, QgsPalLayerSettings::MinScale, *palSettings );
    setDataDefinedProperty( mScaleBasedVisibilityMaxDDBtn, QgsPalLayerSettings::MaxScale, *palSettings );
    setDataDefinedProperty( mFontLimitPixelDDBtn, QgsPalLayerSettings::FontLimitPixel, *palSettings );
    setDataDefinedProperty( mFontMinPixelDDBtn, QgsPalLayerSettings::FontMinPixel, *palSettings );
    setDataDefinedProperty( mFontMaxPixelDDBtn, QgsPalLayerSettings::FontMaxPixel, *palSettings );
    setDataDefinedProperty( mShowLabelDDBtn, QgsPalLayerSettings::Show, *palSettings );
    setDataDefinedProperty( mAlwaysShowDDBtn, QgsPalLayerSettings::AlwaysShow, *palSettings );
  }

  settings->textColor = btnTextColor->color();
  settings->textFont = mRefFont;
  settings->textNamedStyle = mFontStyleComboBox->currentText();
  settings->textTransp = mFontTranspSpinBox->value();
  settings->blendMode = comboBlendMode->blendMode();
  settings->previewBkgrdColor = mPreviewBackgroundBtn->color();

  // buffer
  settings->bufferDraw = mBufferDrawChkBx->isChecked();
  settings->bufferSize = spinBufferSize->value();
  settings->bufferColor = btnBufferColor->color();
  settings->bufferTransp = mBufferTranspSpinBox->value();
  settings->bufferSizeInMapUnits = ( mBufferUnitWidget->getUnit() == 1 );
  settings->bufferSizeMapUnitScale = mBufferUnitWidget->getMapUnitScale();
  settings->bufferJoinStyle = mBufferJoinStyleComboBox->penJoinStyle();
  settings->bufferNoFill = !mBufferTranspFillChbx->isChecked();
  settings->bufferBlendMode = comboBufferBlendMode->blendMode();

  // shape background
  settings->shapeDraw = mShapeDrawChkBx->isChecked();
  settings->shapeType = ( QgsPalLayerSettings::ShapeType )mShapeTypeCmbBx->currentIndex();
  settings->shapeSVGFile = mShapeSVGPathLineEdit->text();

  settings->shapeSizeType = ( QgsPalLayerSettings::SizeType )mShapeSizeCmbBx->currentIndex();
  settings->shapeSize = QPointF( mShapeSizeXSpnBx->value(), mShapeSizeYSpnBx->value() );
  settings->shapeSizeUnits = ( QgsPalLayerSettings::SizeUnit )( mShapeSizeUnitWidget->getUnit() + 1 );
  settings->shapeSizeMapUnitScale = mShapeSizeUnitWidget->getMapUnitScale();
  settings->shapeRotationType = ( QgsPalLayerSettings::RotationType )( mShapeRotationCmbBx->currentIndex() );
  settings->shapeRotation = mShapeRotationDblSpnBx->value();
  settings->shapeOffset = QPointF( mShapeOffsetXSpnBx->value(), mShapeOffsetYSpnBx->value() );
  settings->shapeOffsetUnits = ( QgsPalLayerSettings::SizeUnit )( mShapeOffsetUnitWidget->getUnit() + 1 );
  settings->shapeOffsetMapUnitScale = mShapeOffsetUnitWidget->getMapUnitScale();
  settings->shapeRadii = QPointF( mShapeRadiusXDbSpnBx->value(), mShapeRadiusYDbSpnBx->value() );
  settings->shapeRadiiUnits = ( QgsPalLayerSettings::SizeUnit )( mShapeRadiusUnitWidget->getUnit() + 1 );
  settings->shapeRadiiMapUnitScale = mShapeRadiusUnitWidget->getMapUnitScale();

  settings->shapeFillColor = mShapeFillColorBtn->color();
  settings->shapeBorderColor = mShapeBorderColorBtn->color();
  settings->shapeBorderWidth = mShapeBorderWidthSpnBx->value();
  settings->shapeBorderWidthUnits = ( QgsPalLayerSettings::SizeUnit )( mShapeBorderWidthUnitWidget->getUnit() + 1 );
  settings->shapeBorderWidthMapUnitScale = mShapeBorderWidthUnitWidget->getMapUnitScale();
  settings->shapeJoinStyle = mShapePenStyleCmbBx->penJoinStyle();
  settings->shapeTransparency = mShapeTranspSpinBox->value();
  settings->shapeBlendMode = mShapeBlendCmbBx->blendMode();

  // drop shadow
  settings->shadowDraw = mShadowDrawChkBx->isChecked();
  settings->shadowUnder = ( QgsPalLayerSettings::ShadowType )mShadowUnderCmbBx->currentIndex();
  settings->shadowOffsetAngle = mShadowOffsetAngleSpnBx->value();
  settings->shadowOffsetDist = mShadowOffsetSpnBx->value();
  settings->shadowOffsetUnits = ( QgsPalLayerSettings::SizeUnit )( mShadowOffsetUnitWidget->getUnit() + 1 );
  settings->shadowOffsetMapUnitScale = mShadowOffsetUnitWidget->getMapUnitScale();
  settings->shadowOffsetGlobal = mShadowOffsetGlobalChkBx->isChecked();
  settings->shadowRadius = mShadowRadiusDblSpnBx->value();
  settings->shadowRadiusUnits = ( QgsPalLayerSettings::SizeUnit )( mShadowRadiusUnitWidget->getUnit() + 1 );
  settings->shadowRadiusMapUnitScale = mShadowRadiusUnitWidget->getMapUnitScale();
  settings->shadowRadiusAlphaOnly = mShadowRadiusAlphaChkBx->isChecked();
  settings->shadowTransparency = mShadowTranspSpnBx->value();
  settings->shadowScale = mShadowScaleSpnBx->value();
  settings->shadowColor = mShadowColorBtn->color();
  settings->shadowBlendMode = mShadowBlendCmbBx->blendMode();

  settings->fontSizeInMapUnits = ( mFontSizeUnitWidget->getUnit() == 1 );
  settings->fontSizeMapUnitScale = mFontSizeUnitWidget->getMapUnitScale();
  settings->wrapChar = wrapCharacterEdit->text();
  settings->multilineHeight = mFontLineHeightSpinBox->value();
  settings->multilineAlign = ( QgsPalLayerSettings::MultiLineAlign ) mFontMultiLineAlignComboBox->currentIndex();
}


QgsTextSettingsWidget::~QgsTextSettingsWidget()
{
  QSettings settings;
  settings.setValue( QString( "/Windows/Labeling/FontPreviewSplitState" ), mFontPreviewSplitter->saveState() );
  settings.setValue( QString( "/Windows/Labeling/OptionsSplitState" ), mLabelingOptionsSplitter->saveState() );
  settings.setValue( QString( "/Windows/Labeling/Tab" ), mLabelingOptionsListWidget->currentRow() );
}

void QgsTextSettingsWidget::blockInitSignals( bool block )
{
  chkLineAbove->blockSignals( block );
  chkLineBelow->blockSignals( block );
  mPlacePointBtnGrp->blockSignals( block );
  mPlaceLineBtnGrp->blockSignals( block );
  mPlacePolygonBtnGrp->blockSignals( block );
}

void QgsTextSettingsWidget::optionsStackedWidget_CurrentChanged( int indx )
{
  mLabelingOptionsListWidget->blockSignals( true );
  mLabelingOptionsListWidget->setCurrentRow( indx );
  mLabelingOptionsListWidget->blockSignals( false );
}

void QgsTextSettingsWidget::collapseSample( bool collapse )
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

void QgsTextSettingsWidget::setDataDefinedProperty( const QgsDataDefinedButton* ddBtn, QgsPalLayerSettings::DataDefinedProperties p, QgsPalLayerSettings& lyr )
{
  const QMap< QString, QString >& map = ddBtn->definedProperty();
  lyr.setDataDefinedProperty( p, map.value( "active" ).toInt(), map.value( "useexpr" ).toInt(), map.value( "expression" ), map.value( "field" ) );
}

void QgsTextSettingsWidget::populateDataDefinedButtons( QgsPalLayerSettings& s )
{
  if ( !mLayer )
  {
    return;
  }

  // don't register enable/disable siblings, since visual feedback from data defined buttons should be enough,
  // and ability to edit layer-level setting should remain enabled regardless

  QString trString = tr( "string " );

  // text style
  mFontDDBtn->init( mLayer, s.dataDefinedProperty( QgsPalLayerSettings::Family ),
                    QgsDataDefinedButton::String,
                    trString + tr( "[<b>family</b>|<b>family[foundry]</b>],<br>"
                                   "e.g. Helvetica or Helvetica [Cronyx]" ) );

  mFontStyleDDBtn->init( mLayer, s.dataDefinedProperty( QgsPalLayerSettings::FontStyle ),
                         QgsDataDefinedButton::String,
                         trString + tr( "[<b>font style name</b>|<b>Ignore</b>],<br>"
                                        "e.g. Bold Condensed or Light Italic" ) );

  mFontUnderlineDDBtn->init( mLayer, s.dataDefinedProperty( QgsPalLayerSettings::Underline ),
                             QgsDataDefinedButton::AnyType, QgsDataDefinedButton::boolDesc() );

  mFontStrikeoutDDBtn->init( mLayer, s.dataDefinedProperty( QgsPalLayerSettings::Strikeout ),
                             QgsDataDefinedButton::AnyType, QgsDataDefinedButton::boolDesc() );

  mFontBoldDDBtn->init( mLayer, s.dataDefinedProperty( QgsPalLayerSettings::Bold ),
                        QgsDataDefinedButton::AnyType, QgsDataDefinedButton::boolDesc() );

  mFontItalicDDBtn->init( mLayer, s.dataDefinedProperty( QgsPalLayerSettings::Italic ),
                          QgsDataDefinedButton::AnyType, QgsDataDefinedButton::boolDesc() );

  mFontSizeDDBtn->init( mLayer, s.dataDefinedProperty( QgsPalLayerSettings::Size ),
                        QgsDataDefinedButton::AnyType, QgsDataDefinedButton::doublePosDesc() );

  mFontUnitsDDBtn->init( mLayer, s.dataDefinedProperty( QgsPalLayerSettings::FontSizeUnit ),
                         QgsDataDefinedButton::String, trString + "[<b>Points</b>|<b>MapUnit</b>]" );

  mFontColorDDBtn->init( mLayer, s.dataDefinedProperty( QgsPalLayerSettings::Color ),
                         QgsDataDefinedButton::String, QgsDataDefinedButton::colorNoAlphaDesc() );

  mFontTranspDDBtn->init( mLayer, s.dataDefinedProperty( QgsPalLayerSettings::FontTransp ),
                          QgsDataDefinedButton::AnyType, QgsDataDefinedButton::intTranspDesc() );

  mFontCaseDDBtn->init( mLayer, s.dataDefinedProperty( QgsPalLayerSettings::FontCase ),
                        QgsDataDefinedButton::String,
                        trString + QString( "[<b>NoChange</b>|<b>Upper</b>|<br>"
                                            "<b>Lower</b>|<b>Capitalize</b>]" ) );

  mFontLetterSpacingDDBtn->init( mLayer, s.dataDefinedProperty( QgsPalLayerSettings::FontLetterSpacing ),
                                 QgsDataDefinedButton::AnyType, QgsDataDefinedButton::doubleDesc() );
  mFontWordSpacingDDBtn->init( mLayer, s.dataDefinedProperty( QgsPalLayerSettings::FontWordSpacing ),
                               QgsDataDefinedButton::AnyType, QgsDataDefinedButton::doubleDesc() );

  mFontBlendModeDDBtn->init( mLayer, s.dataDefinedProperty( QgsPalLayerSettings::FontBlendMode ),
                             QgsDataDefinedButton::String, QgsDataDefinedButton::blendModesDesc() );

  // text formatting
  mWrapCharDDBtn->init( mLayer, s.dataDefinedProperty( QgsPalLayerSettings::MultiLineWrapChar ),
                        QgsDataDefinedButton::String, QgsDataDefinedButton::anyStringDesc() );
  mFontLineHeightDDBtn->init( mLayer, s.dataDefinedProperty( QgsPalLayerSettings::MultiLineHeight ),
                              QgsDataDefinedButton::AnyType, tr( "double [0.0-10.0]" ) );
  mFontMultiLineAlignDDBtn->init( mLayer, s.dataDefinedProperty( QgsPalLayerSettings::MultiLineAlignment ),
                                  QgsDataDefinedButton::String, QgsDataDefinedButton::textHorzAlignDesc() );

  mDirectSymbDDBtn->init( mLayer, s.dataDefinedProperty( QgsPalLayerSettings::DirSymbDraw ),
                          QgsDataDefinedButton::AnyType, QgsDataDefinedButton::boolDesc() );
  mDirectSymbDDBtn->registerCheckedWidget( mDirectSymbChkBx );
  mDirectSymbLeftDDBtn->init( mLayer, s.dataDefinedProperty( QgsPalLayerSettings::DirSymbLeft ),
                              QgsDataDefinedButton::String, QgsDataDefinedButton::anyStringDesc() );
  mDirectSymbRightDDBtn->init( mLayer, s.dataDefinedProperty( QgsPalLayerSettings::DirSymbRight ),
                               QgsDataDefinedButton::String, QgsDataDefinedButton::anyStringDesc() );

  mDirectSymbPlacementDDBtn->init( mLayer, s.dataDefinedProperty( QgsPalLayerSettings::DirSymbPlacement ),
                                   QgsDataDefinedButton::String,
                                   trString + "[<b>LeftRight</b>|<b>Above</b>|<b>Below</b>]" );
  mDirectSymbRevDDBtn->init( mLayer, s.dataDefinedProperty( QgsPalLayerSettings::DirSymbReverse ),
                             QgsDataDefinedButton::AnyType, QgsDataDefinedButton::boolDesc() );

  mFormatNumDDBtn->init( mLayer, s.dataDefinedProperty( QgsPalLayerSettings::NumFormat ),
                         QgsDataDefinedButton::AnyType, QgsDataDefinedButton::boolDesc() );
  mFormatNumDDBtn->registerCheckedWidget( mFormatNumChkBx );
  mFormatNumDecimalsDDBtn->init( mLayer, s.dataDefinedProperty( QgsPalLayerSettings::NumDecimals ),
                                 QgsDataDefinedButton::AnyType, tr( "int [0-20]" ) );
  mFormatNumPlusSignDDBtn->init( mLayer, s.dataDefinedProperty( QgsPalLayerSettings::NumPlusSign ),
                                 QgsDataDefinedButton::AnyType, QgsDataDefinedButton::boolDesc() );

  // text buffer
  mBufferDrawDDBtn->init( mLayer, s.dataDefinedProperty( QgsPalLayerSettings::BufferDraw ),
                          QgsDataDefinedButton::AnyType, QgsDataDefinedButton::boolDesc() );
  mBufferDrawDDBtn->registerCheckedWidget( mBufferDrawChkBx );
  mBufferSizeDDBtn->init( mLayer, s.dataDefinedProperty( QgsPalLayerSettings::BufferSize ),
                          QgsDataDefinedButton::AnyType, QgsDataDefinedButton::doublePosDesc() );
  mBufferUnitsDDBtn->init( mLayer, s.dataDefinedProperty( QgsPalLayerSettings::BufferUnit ),
                           QgsDataDefinedButton::String, QgsDataDefinedButton::unitsMmMuDesc() );
  mBufferColorDDBtn->init( mLayer, s.dataDefinedProperty( QgsPalLayerSettings::BufferColor ),
                           QgsDataDefinedButton::String, QgsDataDefinedButton::colorNoAlphaDesc() );
  mBufferTranspDDBtn->init( mLayer, s.dataDefinedProperty( QgsPalLayerSettings::BufferTransp ),
                            QgsDataDefinedButton::AnyType, QgsDataDefinedButton::intTranspDesc() );
  mBufferJoinStyleDDBtn->init( mLayer, s.dataDefinedProperty( QgsPalLayerSettings::BufferJoinStyle ),
                               QgsDataDefinedButton::String, QgsDataDefinedButton::penJoinStyleDesc() );
  mBufferBlendModeDDBtn->init( mLayer, s.dataDefinedProperty( QgsPalLayerSettings::BufferBlendMode ),
                               QgsDataDefinedButton::String, QgsDataDefinedButton::blendModesDesc() );

  // background
  mShapeDrawDDBtn->init( mLayer, s.dataDefinedProperty( QgsPalLayerSettings::ShapeDraw ),
                         QgsDataDefinedButton::AnyType, QgsDataDefinedButton::boolDesc() );
  mShapeDrawDDBtn->registerCheckedWidget( mShapeDrawChkBx );
  mShapeTypeDDBtn->init( mLayer, s.dataDefinedProperty( QgsPalLayerSettings::ShapeKind ),
                         QgsDataDefinedButton::String,
                         trString + QString( "[<b>Rectangle</b>|<b>Square</b>|<br>"
                                             "<b>Ellipse</b>|<b>Circle</b>|<b>SVG</b>]" ) );
  mShapeSVGPathDDBtn->init( mLayer, s.dataDefinedProperty( QgsPalLayerSettings::ShapeSVGFile ),
                            QgsDataDefinedButton::String, QgsDataDefinedButton::svgPathDesc() );
  mShapeSizeTypeDDBtn->init( mLayer, s.dataDefinedProperty( QgsPalLayerSettings::ShapeSizeType ),
                             QgsDataDefinedButton::String,
                             trString + "[<b>Buffer</b>|<b>Fixed</b>]" );
  mShapeSizeXDDBtn->init( mLayer, s.dataDefinedProperty( QgsPalLayerSettings::ShapeSizeX ),
                          QgsDataDefinedButton::AnyType, QgsDataDefinedButton::doubleDesc() );
  mShapeSizeYDDBtn->init( mLayer, s.dataDefinedProperty( QgsPalLayerSettings::ShapeSizeY ),
                          QgsDataDefinedButton::AnyType, QgsDataDefinedButton::doubleDesc() );
  mShapeSizeUnitsDDBtn->init( mLayer, s.dataDefinedProperty( QgsPalLayerSettings::ShapeSizeUnits ),
                              QgsDataDefinedButton::String, QgsDataDefinedButton::unitsMmMuDesc() );
  mShapeRotationTypeDDBtn->init( mLayer, s.dataDefinedProperty( QgsPalLayerSettings::ShapeRotationType ),
                                 QgsDataDefinedButton::String,
                                 trString + "[<b>Sync</b>|<b>Offset</b>|<b>Fixed</b>]" );
  mShapeRotationDDBtn->init( mLayer, s.dataDefinedProperty( QgsPalLayerSettings::ShapeRotation ),
                             QgsDataDefinedButton::AnyType, QgsDataDefinedButton::double180RotDesc() );
  mShapeOffsetDDBtn->init( mLayer, s.dataDefinedProperty( QgsPalLayerSettings::ShapeOffset ),
                           QgsDataDefinedButton::AnyType, QgsDataDefinedButton::doubleXYDesc() );
  mShapeOffsetUnitsDDBtn->init( mLayer, s.dataDefinedProperty( QgsPalLayerSettings::ShapeOffsetUnits ),
                                QgsDataDefinedButton::String, QgsDataDefinedButton::unitsMmMuDesc() );
  mShapeRadiusDDBtn->init( mLayer, s.dataDefinedProperty( QgsPalLayerSettings::ShapeRadii ),
                           QgsDataDefinedButton::AnyType, QgsDataDefinedButton::doubleXYDesc() );
  mShapeRadiusUnitsDDBtn->init( mLayer, s.dataDefinedProperty( QgsPalLayerSettings::ShapeRadiiUnits ),
                                QgsDataDefinedButton::String, QgsDataDefinedButton::unitsMmMuPercentDesc() );
  mShapeTranspDDBtn->init( mLayer, s.dataDefinedProperty( QgsPalLayerSettings::ShapeTransparency ),
                           QgsDataDefinedButton::AnyType, QgsDataDefinedButton::intTranspDesc() );
  mShapeBlendModeDDBtn->init( mLayer, s.dataDefinedProperty( QgsPalLayerSettings::ShapeBlendMode ),
                              QgsDataDefinedButton::String, QgsDataDefinedButton::blendModesDesc() );
  mShapeFillColorDDBtn->init( mLayer, s.dataDefinedProperty( QgsPalLayerSettings::ShapeFillColor ),
                              QgsDataDefinedButton::String, QgsDataDefinedButton::colorAlphaDesc() );
  mShapeBorderColorDDBtn->init( mLayer, s.dataDefinedProperty( QgsPalLayerSettings::ShapeBorderColor ),
                                QgsDataDefinedButton::String, QgsDataDefinedButton::colorAlphaDesc() );
  mShapeBorderWidthDDBtn->init( mLayer, s.dataDefinedProperty( QgsPalLayerSettings::ShapeBorderWidth ),
                                QgsDataDefinedButton::AnyType, QgsDataDefinedButton::doublePosDesc() );
  mShapeBorderUnitsDDBtn->init( mLayer, s.dataDefinedProperty( QgsPalLayerSettings::ShapeBorderWidthUnits ),
                                QgsDataDefinedButton::String, QgsDataDefinedButton::unitsMmMuDesc() );
  mShapePenStyleDDBtn->init( mLayer, s.dataDefinedProperty( QgsPalLayerSettings::ShapeJoinStyle ),
                             QgsDataDefinedButton::String, QgsDataDefinedButton::penJoinStyleDesc() );

  // drop shadows
  mShadowDrawDDBtn->init( mLayer, s.dataDefinedProperty( QgsPalLayerSettings::ShadowDraw ),
                          QgsDataDefinedButton::AnyType, QgsDataDefinedButton::boolDesc() );
  mShadowDrawDDBtn->registerCheckedWidget( mShadowDrawChkBx );
  mShadowUnderDDBtn->init( mLayer, s.dataDefinedProperty( QgsPalLayerSettings::ShadowUnder ),
                           QgsDataDefinedButton::String,
                           trString + QString( "[<b>Lowest</b>|<b>Text</b>|<br>"
                                               "<b>Buffer</b>|<b>Background</b>]" ) );
  mShadowOffsetAngleDDBtn->init( mLayer, s.dataDefinedProperty( QgsPalLayerSettings::ShadowOffsetAngle ),
                                 QgsDataDefinedButton::AnyType, QgsDataDefinedButton::double180RotDesc() );
  mShadowOffsetDDBtn->init( mLayer, s.dataDefinedProperty( QgsPalLayerSettings::ShadowOffsetDist ),
                            QgsDataDefinedButton::AnyType, QgsDataDefinedButton::doublePosDesc() );
  mShadowOffsetUnitsDDBtn->init( mLayer, s.dataDefinedProperty( QgsPalLayerSettings::ShadowOffsetUnits ),
                                 QgsDataDefinedButton::String, QgsDataDefinedButton::unitsMmMuDesc() );
  mShadowRadiusDDBtn->init( mLayer, s.dataDefinedProperty( QgsPalLayerSettings::ShadowRadius ),
                            QgsDataDefinedButton::AnyType, QgsDataDefinedButton::doublePosDesc() );
  mShadowRadiusUnitsDDBtn->init( mLayer, s.dataDefinedProperty( QgsPalLayerSettings::ShadowRadiusUnits ),
                                 QgsDataDefinedButton::String, QgsDataDefinedButton::unitsMmMuDesc() );
  mShadowTranspDDBtn->init( mLayer, s.dataDefinedProperty( QgsPalLayerSettings::ShadowTransparency ),
                            QgsDataDefinedButton::AnyType, QgsDataDefinedButton::intTranspDesc() );
  mShadowScaleDDBtn->init( mLayer, s.dataDefinedProperty( QgsPalLayerSettings::ShadowScale ),
                           QgsDataDefinedButton::AnyType, tr( "int [0-2000]" ) );
  mShadowColorDDBtn->init( mLayer, s.dataDefinedProperty( QgsPalLayerSettings::ShadowColor ),
                           QgsDataDefinedButton::String, QgsDataDefinedButton::colorNoAlphaDesc() );
  mShadowBlendDDBtn->init( mLayer, s.dataDefinedProperty( QgsPalLayerSettings::ShadowBlendMode ),
                           QgsDataDefinedButton::String, QgsDataDefinedButton::blendModesDesc() );

  // placement
  mCentroidDDBtn->init( mLayer, s.dataDefinedProperty( QgsPalLayerSettings::CentroidWhole ),
                        QgsDataDefinedButton::String,
                        trString + "[<b>Visible</b>|<b>Whole</b>]" );
  mPointQuadOffsetDDBtn->init( mLayer, s.dataDefinedProperty( QgsPalLayerSettings::OffsetQuad ),
                               QgsDataDefinedButton::AnyType,
                               tr( "int<br>" ) + QString( "[<b>0</b>=Above Left|<b>1</b>=Above|<b>2</b>=Above Right|<br>"
                                                          "<b>3</b>=Left|<b>4</b>=Over|<b>5</b>=Right|<br>"
                                                          "<b>6</b>=Below Left|<b>7</b>=Below|<b>8</b>=Below Right]" ) );
  mPointOffsetDDBtn->init( mLayer, s.dataDefinedProperty( QgsPalLayerSettings::OffsetXY ),
                           QgsDataDefinedButton::AnyType, QgsDataDefinedButton::doubleXYDesc() );
  mPointOffsetUnitsDDBtn->init( mLayer, s.dataDefinedProperty( QgsPalLayerSettings::OffsetUnits ),
                                QgsDataDefinedButton::String, QgsDataDefinedButton::unitsMmMuDesc() );
  mLineDistanceDDBtn->init( mLayer, s.dataDefinedProperty( QgsPalLayerSettings::LabelDistance ),
                            QgsDataDefinedButton::AnyType, QgsDataDefinedButton::doublePosDesc() );
  mLineDistanceUnitDDBtn->init( mLayer, s.dataDefinedProperty( QgsPalLayerSettings::DistanceUnits ),
                                QgsDataDefinedButton::String, QgsDataDefinedButton::unitsMmMuDesc() );
  // TODO: is this necessary? maybe just use the data defined-only rotation?
  //mPointAngleDDBtn->init( mLayer, s.dataDefinedProperty( QgsPalLayerSettings::OffsetRotation ),
  //                        QgsDataDefinedButton::AnyType, QgsDataDefinedButton::double180RotDesc() );
  mMaxCharAngleDDBtn->init( mLayer, s.dataDefinedProperty( QgsPalLayerSettings::CurvedCharAngleInOut ),
                            QgsDataDefinedButton::AnyType, tr( "double coord [<b>in,out</b> as 20.0-60.0,20.0-95.0]" ) );
  mRepeatDistanceDDBtn->init( mLayer, s.dataDefinedProperty( QgsPalLayerSettings::RepeatDistance ),
                              QgsDataDefinedButton::AnyType, QgsDataDefinedButton::doublePosDesc() );
  mRepeatDistanceUnitDDBtn->init( mLayer, s.dataDefinedProperty( QgsPalLayerSettings::RepeatDistanceUnit ),
                                  QgsDataDefinedButton::String, QgsDataDefinedButton::unitsMmMuDesc() );

  // data defined-only
  QString ddPlaceInfo = tr( "In edit mode, layer's relevant labeling map tool is:<br>"
                            "&nbsp;&nbsp;Defined attribute field -&gt; <i>enabled</i><br>"
                            "&nbsp;&nbsp;Defined expression -&gt; <i>disabled</i>" );
  mCoordXDDBtn->init( mLayer, s.dataDefinedProperty( QgsPalLayerSettings::PositionX ),
                      QgsDataDefinedButton::AnyType, QgsDataDefinedButton::doubleDesc() );
  mCoordXDDBtn->setUsageInfo( ddPlaceInfo );
  mCoordYDDBtn->init( mLayer, s.dataDefinedProperty( QgsPalLayerSettings::PositionY ),
                      QgsDataDefinedButton::AnyType, QgsDataDefinedButton::doubleDesc() );
  mCoordYDDBtn->setUsageInfo( ddPlaceInfo );
  mCoordAlignmentHDDBtn->init( mLayer, s.dataDefinedProperty( QgsPalLayerSettings::Hali ),
                               QgsDataDefinedButton::String,
                               trString + "[<b>Left</b>|<b>Center</b>|<b>Right</b>]" );
  mCoordAlignmentHDDBtn->setUsageInfo( ddPlaceInfo );
  mCoordAlignmentVDDBtn->init( mLayer, s.dataDefinedProperty( QgsPalLayerSettings::Vali ),
                               QgsDataDefinedButton::String,
                               trString + QString( "[<b>Bottom</b>|<b>Base</b>|<br>"
                                                   "<b>Half</b>|<b>Cap</b>|<b>Top</b>]" ) );
  mCoordAlignmentVDDBtn->setUsageInfo( ddPlaceInfo );
  mCoordRotationDDBtn->init( mLayer, s.dataDefinedProperty( QgsPalLayerSettings::Rotation ),
                             QgsDataDefinedButton::AnyType, QgsDataDefinedButton::double180RotDesc() );
  mCoordRotationDDBtn->setUsageInfo( ddPlaceInfo );

  // rendering
  QString ddScaleVisInfo = tr( "Value &lt; 0 represents a scale closer than 1:1, e.g. -10 = 10:1<br>"
                               "Value of 0 disables the specific limit." );
  mScaleBasedVisibilityDDBtn->init( mLayer, s.dataDefinedProperty( QgsPalLayerSettings::ScaleVisibility ),
                                    QgsDataDefinedButton::AnyType, QgsDataDefinedButton::boolDesc() );
  mScaleBasedVisibilityDDBtn->registerCheckedWidget( mScaleBasedVisibilityChkBx );
  mScaleBasedVisibilityMinDDBtn->init( mLayer, s.dataDefinedProperty( QgsPalLayerSettings::MinScale ),
                                       QgsDataDefinedButton::AnyType, QgsDataDefinedButton::intDesc() );
  mScaleBasedVisibilityMinDDBtn->setUsageInfo( ddScaleVisInfo );
  mScaleBasedVisibilityMaxDDBtn->init( mLayer, s.dataDefinedProperty( QgsPalLayerSettings::MaxScale ),
                                       QgsDataDefinedButton::AnyType, QgsDataDefinedButton::intDesc() );
  mScaleBasedVisibilityMaxDDBtn->setUsageInfo( ddScaleVisInfo );

  mFontLimitPixelDDBtn->init( mLayer, s.dataDefinedProperty( QgsPalLayerSettings::FontLimitPixel ),
                              QgsDataDefinedButton::AnyType, QgsDataDefinedButton::boolDesc() );
  mFontLimitPixelDDBtn->registerCheckedWidget( mFontLimitPixelChkBox );
  mFontMinPixelDDBtn->init( mLayer, s.dataDefinedProperty( QgsPalLayerSettings::FontMinPixel ),
                            QgsDataDefinedButton::AnyType, tr( "int [1-1000]" ) );
  mFontMaxPixelDDBtn->init( mLayer, s.dataDefinedProperty( QgsPalLayerSettings::FontMaxPixel ),
                            QgsDataDefinedButton::AnyType, tr( "int [1-10000]" ) );

  mShowLabelDDBtn->init( mLayer, s.dataDefinedProperty( QgsPalLayerSettings::Show ),
                         QgsDataDefinedButton::AnyType, QgsDataDefinedButton::boolDesc() );

  mAlwaysShowDDBtn->init( mLayer, s.dataDefinedProperty( QgsPalLayerSettings::AlwaysShow ),
                          QgsDataDefinedButton::AnyType, QgsDataDefinedButton::boolDesc() );
}

void QgsTextSettingsWidget::updateFont( QFont font )
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

void QgsTextSettingsWidget::blockFontChangeSignals( bool blk )
{
  mFontFamilyCmbBx->blockSignals( blk );
  mFontStyleComboBox->blockSignals( blk );
  mFontCapitalsComboBox->blockSignals( blk );
  mFontUnderlineBtn->blockSignals( blk );
  mFontStrikethroughBtn->blockSignals( blk );
  mFontWordSpacingSpinBox->blockSignals( blk );
  mFontLetterSpacingSpinBox->blockSignals( blk );
}

void QgsTextSettingsWidget::updatePreview()
{
  if ( mLoadingSettings )
  {
    //don't update the preview while loading settings
    return;
  }

  scrollPreview();

  QgsTextRendererSettings previewSettings;
  saveToSettings( &previewSettings );

  QFont previewFont = previewSettings.textFont;
  double fontSize = previewFont.pointSizeF();
  double previewRatio = mPreviewSize / fontSize;
  QString grpboxtitle;
  QString sampleTxt = tr( "Text/Buffer sample" );

  if ( mFontSizeUnitWidget->getUnit() == 1 ) // map units
  {
    // TODO: maybe match current map zoom level instead?
    previewFont.setPointSize( mPreviewSize );
    mPreviewSizeSlider->setEnabled( true );
    grpboxtitle = sampleTxt + tr( " @ %1 pts (using map units)" ).arg( mPreviewSize );

    previewFont.setWordSpacing( previewRatio * previewFont.wordSpacing() );
    previewFont.setLetterSpacing( QFont::AbsoluteSpacing, previewRatio * previewFont.letterSpacing() );
    previewSettings.textFont = previewFont;

    if ( previewSettings.bufferDraw )
    {
      if ( previewSettings.bufferSizeInMapUnits ) // map units
      {
        previewSettings.bufferSize = previewRatio * previewSettings.bufferSize / 3.527;
        previewSettings.bufferSizeInMapUnits = false;
      }
      else // millimeters
      {
        grpboxtitle = sampleTxt + tr( " @ %1 pts (using map units, BUFFER IN MILLIMETERS)" ).arg( mPreviewSize );
      }
    }
  }
  else // in points
  {
    mPreviewSizeSlider->setEnabled( false );
    grpboxtitle = sampleTxt;

    if ( previewSettings.bufferDraw )
    {
      if ( previewSettings.bufferSizeInMapUnits ) // map units
      {
        grpboxtitle = sampleTxt + tr( " (BUFFER NOT SHOWN, in map units)" );
        previewSettings.bufferDraw = false;
      }
    }
  }

  //lblFontPreview->setMapUnitScale( 10.0 / mPreviewSize );
  lblFontPreview->setTextRendererSettings( previewSettings );
  groupBox_mPreview->setTitle( grpboxtitle );
}

void QgsTextSettingsWidget::scrollPreview()
{
  scrollArea_mPreview->ensureVisible( 0, 0, 0, 0 );
}

void QgsTextSettingsWidget::setPreviewBackground( QColor color )
{
  scrollArea_mPreview->widget()->setStyleSheet( QString( "background: rgb(%1, %2, %3);" ).arg( QString::number( color.red() ),
      QString::number( color.green() ),
      QString::number( color.blue() ) ) );
}

void QgsTextSettingsWidget::syncDefinedCheckboxFrame( QgsDataDefinedButton* ddBtn, QCheckBox* chkBx, QFrame* f )
{
  if ( ddBtn->isActive() && !chkBx->isChecked() )
  {
    chkBx->setChecked( true );
  }
  f->setEnabled( chkBx->isChecked() );
}

void QgsTextSettingsWidget::updateUi()
{
  // enable/disable inline groupbox-like setups (that need to honor data defined setting)

  syncDefinedCheckboxFrame( mBufferDrawDDBtn, mBufferDrawChkBx, mBufferFrame );
  syncDefinedCheckboxFrame( mShapeDrawDDBtn, mShapeDrawChkBx, mShapeFrame );
  syncDefinedCheckboxFrame( mShadowDrawDDBtn, mShadowDrawChkBx, mShadowFrame );

  syncDefinedCheckboxFrame( mDirectSymbDDBtn, mDirectSymbChkBx, mDirectSymbFrame );
  syncDefinedCheckboxFrame( mFormatNumDDBtn, mFormatNumChkBx, mFormatNumFrame );
  syncDefinedCheckboxFrame( mScaleBasedVisibilityDDBtn, mScaleBasedVisibilityChkBx, mScaleBasedVisibilityFrame );
  syncDefinedCheckboxFrame( mFontLimitPixelDDBtn, mFontLimitPixelChkBox, mFontLimitPixelFrame );
}

void QgsTextSettingsWidget::updatePlacementWidgets()
{
  QWidget* curWdgt = stackedPlacement->currentWidget();

  bool showLineFrame = false;
  bool showCentroidFrame = false;
  bool showQuadrantFrame = false;
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
  }
  else if (( curWdgt == pagePoint && radOverPoint->isChecked() )
           || ( curWdgt == pagePolygon && radOverCentroid->isChecked() ) )
  {
    showCentroidFrame = ( curWdgt == pagePolygon && radOverCentroid->isChecked() );
    showQuadrantFrame = true;
    showOffsetFrame = true;
    showRotationFrame = true;
  }
  else if (( curWdgt == pageLine && radLineParallel->isChecked() )
           || ( curWdgt == pagePolygon && radPolygonPerimeter->isChecked() )
           || ( curWdgt == pageLine && radLineCurved->isChecked() ) )
  {
    showLineFrame = true;
    showDistanceFrame = true;
    //showRotationFrame = true; // TODO: uncomment when supported

    bool offline = chkLineAbove->isChecked() || chkLineBelow->isChecked();
    chkLineOrientationDependent->setEnabled( offline );
    mPlacementDistanceFrame->setEnabled( offline );

    showMaxCharAngleFrame = ( curWdgt == pageLine && radLineCurved->isChecked() );
    // TODO: enable mMultiLinesFrame when supported for curved labels
    enableMultiLinesFrame = !( curWdgt == pageLine && radLineCurved->isChecked() );
  }

  mPlacementLineFrame->setVisible( showLineFrame );
  mPlacementCentroidFrame->setVisible( showCentroidFrame );
  mPlacementQuadrantFrame->setVisible( showQuadrantFrame );
  mPlacementOffsetFrame->setVisible( showOffsetFrame );
  mPlacementDistanceFrame->setVisible( showDistanceFrame );
  mPlacementRotationFrame->setVisible( showRotationFrame );
  mPlacementRepeatDistanceFrame->setVisible( curWdgt == pageLine || ( curWdgt == pagePolygon && radPolygonPerimeter->isChecked() ) );
  mPlacementMaxCharAngleFrame->setVisible( showMaxCharAngleFrame );

  mMultiLinesFrame->setEnabled( enableMultiLinesFrame );
}

void QgsTextSettingsWidget::populateFontCapitalsComboBox()
{
  mFontCapitalsComboBox->addItem( tr( "No change" ), QVariant( 0 ) );
  mFontCapitalsComboBox->addItem( tr( "All uppercase" ), QVariant( 1 ) );
  mFontCapitalsComboBox->addItem( tr( "All lowercase" ), QVariant( 2 ) );
  // Small caps doesn't work right with QPainterPath::addText()
  // https://bugreports.qt-project.org/browse/QTBUG-13965
//  mFontCapitalsComboBox->addItem( tr( "Small caps" ), QVariant( 3 ) );
  mFontCapitalsComboBox->addItem( tr( "Capitalize first letter" ), QVariant( 4 ) );
}

void QgsTextSettingsWidget::populateFontStyleComboBox()
{
  mFontStyleComboBox->clear();
  foreach ( const QString &style, mFontDB.styles( mRefFont.family() ) )
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

void QgsTextSettingsWidget::on_mPreviewSizeSlider_valueChanged( int i )
{
  mPreviewSize = i;
  updatePreview();
}

void QgsTextSettingsWidget::on_mFontSizeSpinBox_valueChanged( double d )
{
  mRefFont.setPointSizeF( d );
  updateFont( mRefFont );
}

void QgsTextSettingsWidget::on_mFontCapitalsComboBox_currentIndexChanged( int index )
{
  int capitalsindex = mFontCapitalsComboBox->itemData( index ).toUInt();
  mRefFont.setCapitalization(( QFont::Capitalization ) capitalsindex );
  updateFont( mRefFont );
}

void QgsTextSettingsWidget::on_mFontFamilyCmbBx_currentFontChanged( const QFont& f )
{
  mRefFont.setFamily( f.family() );
  updateFont( mRefFont );
}

void QgsTextSettingsWidget::on_mFontStyleComboBox_currentIndexChanged( const QString & text )
{
  QgsFontUtils::updateFontViaStyle( mRefFont, text );
  updateFont( mRefFont );
}

void QgsTextSettingsWidget::on_mFontUnderlineBtn_toggled( bool ckd )
{
  mRefFont.setUnderline( ckd );
  updateFont( mRefFont );
}

void QgsTextSettingsWidget::on_mFontStrikethroughBtn_toggled( bool ckd )
{
  mRefFont.setStrikeOut( ckd );
  updateFont( mRefFont );
}

void QgsTextSettingsWidget::on_mFontWordSpacingSpinBox_valueChanged( double spacing )
{
  mRefFont.setWordSpacing( spacing );
  updateFont( mRefFont );
}

void QgsTextSettingsWidget::on_mFontLetterSpacingSpinBox_valueChanged( double spacing )
{
  mRefFont.setLetterSpacing( QFont::AbsoluteSpacing, spacing );
  updateFont( mRefFont );
}

void QgsTextSettingsWidget::on_mFontSizeUnitWidget_changed()
{
  int index = mFontSizeUnitWidget->getUnit();
  // disable pixel size limiting for labels defined in points
  if ( index == 0 )
  {
    mFontLimitPixelChkBox->setChecked( false );
  }
  else if ( index == 1 && mMinPixelLimit == 0 )
  {
    // initial minimum trigger value set, turn on pixel size limiting by default
    // for labels defined in map units (ignored after first settings save)
    mFontLimitPixelChkBox->setChecked( true );
  }
  updateFont( mRefFont );
}

void QgsTextSettingsWidget::on_mFontMinPixelSpinBox_valueChanged( int px )
{
  // ensure max font pixel size for map unit labels can't be lower than min
  mFontMaxPixelSpinBox->setMinimum( px );
  mFontMaxPixelSpinBox->update();
}

void QgsTextSettingsWidget::on_mFontMaxPixelSpinBox_valueChanged( int px )
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

void QgsTextSettingsWidget::on_mBufferUnitWidget_changed()
{
  updateFont( mRefFont );
}

void QgsTextSettingsWidget::on_mCoordXDDBtn_dataDefinedActivated( bool active )
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

void QgsTextSettingsWidget::on_mCoordYDDBtn_dataDefinedActivated( bool active )
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

void QgsTextSettingsWidget::on_mShapeTypeCmbBx_currentIndexChanged( int index )
{
  // shape background
  bool isRect = (( QgsPalLayerSettings::ShapeType )index == QgsPalLayerSettings::ShapeRectangle
                 || ( QgsPalLayerSettings::ShapeType )index == QgsPalLayerSettings::ShapeSquare );
  bool isSVG = (( QgsPalLayerSettings::ShapeType )index == QgsPalLayerSettings::ShapeSVG );

  showBackgroundPenStyle( isRect );
  showBackgroundRadius( isRect );

  mShapeSVGPathFrame->setVisible( isSVG );
  // symbology SVG renderer only supports size^2 scaling, so we only use the x size spinbox
  mShapeSizeYLabel->setVisible( !isSVG );
  mShapeSizeYSpnBx->setVisible( !isSVG );
  mShapeSizeYDDBtn->setVisible( !isSVG );
  mShapeSizeXLabel->setText( tr( "Size%1" ).arg( !isSVG ? tr( " X" ) : "" ) );

  // SVG parameter setting doesn't support color's alpha component yet
  mShapeFillColorBtn->setColorDialogOptions( isSVG ? QColorDialog::ColorDialogOptions( 0 ) : QColorDialog::ShowAlphaChannel );
  mShapeFillColorBtn->setButtonBackground();
  mShapeBorderColorBtn->setColorDialogOptions( isSVG ? QColorDialog::ColorDialogOptions( 0 ) : QColorDialog::ShowAlphaChannel );
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

void QgsTextSettingsWidget::on_mShapeSVGPathLineEdit_textChanged( const QString& text )
{
  updateSvgWidgets( text );
}

void QgsTextSettingsWidget::updateSvgWidgets( const QString& svgPath )
{
  if ( mShapeSVGPathLineEdit->text() != svgPath )
  {
    mShapeSVGPathLineEdit->setText( svgPath );
  }

  QString resolvedPath = QgsSymbolLayerV2Utils::symbolNameToPath( svgPath );
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

void QgsTextSettingsWidget::on_mShapeSVGSelectorBtn_clicked()
{
  QgsSvgSelectorDialog svgDlg( this );
  svgDlg.svgSelector()->setSvgPath( mShapeSVGPathLineEdit->text().trimmed() );

  if ( svgDlg.exec() == QDialog::Accepted )
  {
    QString svgPath = svgDlg.svgSelector()->currentSvgPath();
    if ( !svgPath.isEmpty() )
    {
      mShapeSVGPathLineEdit->setText( svgPath );
    }
  }
  updatePreview();
}

void QgsTextSettingsWidget::on_mShapeSVGParamsBtn_clicked()
{
  QString svgPath = mShapeSVGPathLineEdit->text();
  mLoadSvgParams = true;
  updateSvgWidgets( svgPath );
  mLoadSvgParams = false;
}

void QgsTextSettingsWidget::on_mShapeRotationCmbBx_currentIndexChanged( int index )
{
  mShapeRotationDblSpnBx->setEnabled(( QgsPalLayerSettings::RotationType )index != QgsPalLayerSettings::RotationSync );
  mShapeRotationDDBtn->setEnabled(( QgsPalLayerSettings::RotationType )index != QgsPalLayerSettings::RotationSync );
}

void QgsTextSettingsWidget::on_mPreviewTextEdit_textChanged( const QString & text )
{
  lblFontPreview->setText( text );
  updatePreview();
}

void QgsTextSettingsWidget::on_mPreviewTextBtn_clicked()
{
  mPreviewTextEdit->setText( QString( "Lorem Ipsum" ) );
  updatePreview();
}

void QgsTextSettingsWidget::on_mPreviewBackgroundBtn_colorChanged( const QColor &color )
{
  setPreviewBackground( color );
}

void QgsTextSettingsWidget::on_mDirectSymbLeftToolBtn_clicked()
{
  bool gotChar = false;
  QChar dirSymb = mCharDlg->selectCharacter( &gotChar, mRefFont, mFontDB.styleString( mRefFont ) );

  if ( !gotChar )
    return;

  if ( !dirSymb.isNull() )
    mDirectSymbLeftLineEdit->setText( QString( dirSymb ) );
}

void QgsTextSettingsWidget::on_mDirectSymbRightToolBtn_clicked()
{
  bool gotChar = false;
  QChar dirSymb = mCharDlg->selectCharacter( &gotChar, mRefFont, mFontDB.styleString( mRefFont ) );

  if ( !gotChar )
    return;

  if ( !dirSymb.isNull() )
    mDirectSymbRightLineEdit->setText( QString( dirSymb ) );
}

void QgsTextSettingsWidget::showBackgroundRadius( bool show )
{
  mShapeRadiusLabel->setVisible( show );
  mShapeRadiusXDbSpnBx->setVisible( show );

  mShapeRadiusYDbSpnBx->setVisible( show );

  mShapeRadiusUnitWidget->setVisible( show );

  mShapeRadiusDDBtn->setVisible( show );
  mShapeRadiusUnitsDDBtn->setVisible( show );
}

void QgsTextSettingsWidget::showBackgroundPenStyle( bool show )
{
  mShapePenStyleLabel->setVisible( show );
  mShapePenStyleCmbBx->setVisible( show );

  mShapePenStyleDDBtn->setVisible( show );
}

void QgsTextSettingsWidget::enableDataDefinedAlignment( bool enable )
{
  mCoordAlignmentFrame->setEnabled( enable );
}

//
// QgsTextSettingsDialog
//

QgsTextSettingsDialog::QgsTextSettingsDialog( QWidget *parent, Qt::WindowFlags fl )
    : QDialog( parent, fl )
{
  setWindowTitle( tr( "Text settings" ) );

  mSettingsWidget = new QgsTextSettingsWidget( this );
  mSettingsWidget->layout()->setContentsMargins( 0, 0, 0, 0 );

  QVBoxLayout *layout = new QVBoxLayout( this );
  layout->addWidget( mSettingsWidget );

  QDialogButtonBox *buttonBox = new QDialogButtonBox( QDialogButtonBox::Ok | QDialogButtonBox::Cancel, Qt::Horizontal, this );
  layout->addWidget( buttonBox );

  setLayout( layout );

  QSettings settings;
  restoreGeometry( settings.value( "/Windows/TextSettings/geometry" ).toByteArray() );

  connect( buttonBox->button( QDialogButtonBox::Ok ), SIGNAL( clicked() ), this, SLOT( accept() ) );
  connect( buttonBox->button( QDialogButtonBox::Cancel ), SIGNAL( clicked() ), this, SLOT( reject() ) );
}


QgsTextSettingsDialog::~QgsTextSettingsDialog()
{

}

bool QgsTextSettingsDialog::execForSettings( QgsTextRendererSettings *settings )
{
  if ( settings )
  {
    //initialise with settings
    mSettingsWidget->loadSettings( settings );
  }

  QApplication::setOverrideCursor( Qt::ArrowCursor );
  int res = exec();
  QApplication::restoreOverrideCursor();

  if ( res == QDialog::Accepted )
  {
    if ( settings )
    {
      mSettingsWidget->saveToSettings( settings );
    }
    return true;
  }
  else
  {
    return false;
  }
}
