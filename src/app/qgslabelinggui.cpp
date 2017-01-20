/***************************************************************************
  qgslabelinggui.cpp
  Smart labeling for vector layers
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

#include "qgslabelinggui.h"
#include "qgisapp.h"
#include "qgsvectorlayer.h"
#include "qgsmapcanvas.h"
#include "qgsvectorlayerlabeling.h"
#include "qgsproject.h"

QgsExpressionContext QgsLabelingGui::createExpressionContext() const
{
  QgsExpressionContext expContext;
  expContext << QgsExpressionContextUtils::globalScope()
  << QgsExpressionContextUtils::projectScope( QgsProject::instance() )
  << QgsExpressionContextUtils::atlasScope( nullptr )
  << QgsExpressionContextUtils::mapSettingsScope( QgisApp::instance()->mapCanvas()->mapSettings() );

  if ( mLayer )
    expContext << QgsExpressionContextUtils::layerScope( mLayer );

  expContext << QgsExpressionContextUtils::updateSymbolScope( nullptr, new QgsExpressionContextScope() );

  //TODO - show actual value
  expContext.setOriginalValueVariable( QVariant() );
  expContext.setHighlightedVariables( QStringList() << QgsExpressionContext::EXPR_ORIGINAL_VALUE << QgsExpressionContext::EXPR_SYMBOL_COLOR );

  return expContext;
}

QgsLabelingGui::QgsLabelingGui( QgsVectorLayer* layer, QgsMapCanvas* mapCanvas, const QgsPalLayerSettings* layerSettings, QWidget* parent )
    : QgsTextFormatWidget( mapCanvas, parent, QgsTextFormatWidget::Labeling )
    , mLayer( layer )
    , mSettings( layerSettings )
    , mMode( NoLabels )
{
  // connections for groupboxes with separate activation checkboxes (that need to honor data defined setting)
  connect( mBufferDrawChkBx, SIGNAL( toggled( bool ) ), this, SLOT( updateUi() ) );
  connect( mShapeDrawChkBx, SIGNAL( toggled( bool ) ), this, SLOT( updateUi() ) );
  connect( mShadowDrawChkBx, SIGNAL( toggled( bool ) ), this, SLOT( updateUi() ) );
  connect( mDirectSymbChkBx, SIGNAL( toggled( bool ) ), this, SLOT( updateUi() ) );
  connect( mFormatNumChkBx, SIGNAL( toggled( bool ) ), this, SLOT( updateUi() ) );
  connect( mScaleBasedVisibilityChkBx, SIGNAL( toggled( bool ) ), this, SLOT( updateUi() ) );
  connect( mFontLimitPixelChkBox, SIGNAL( toggled( bool ) ), this, SLOT( updateUi() ) );

  mFieldExpressionWidget->registerExpressionContextGenerator( this );

  setLayer( layer );
}

void QgsLabelingGui::setLayer( QgsMapLayer* mapLayer )
{
  if ( !mapLayer || mapLayer->type() != QgsMapLayer::VectorLayer )
  {
    setEnabled( false );
    return;
  }
  else
  {
    setEnabled( true );
  }

  QgsVectorLayer *layer = qobject_cast<QgsVectorLayer*>( mapLayer );
  mLayer = layer;

  // load labeling settings from layer
  QgsPalLayerSettings lyr;
  if ( mSettings )
    lyr = *mSettings;
  else
    lyr.readFromLayer( mLayer );

  // show/hide options based upon geometry type
  chkMergeLines->setVisible( mLayer->geometryType() == QgsWkbTypes::LineGeometry );
  mDirectSymbolsFrame->setVisible( mLayer->geometryType() == QgsWkbTypes::LineGeometry );
  mMinSizeFrame->setVisible( mLayer->geometryType() != QgsWkbTypes::PointGeometry );
  mPolygonObstacleTypeFrame->setVisible( mLayer->geometryType() == QgsWkbTypes::PolygonGeometry );
  mPolygonFeatureOptionsFrame->setVisible( mLayer->geometryType() == QgsWkbTypes::PolygonGeometry );

  mFieldExpressionWidget->setLayer( mLayer );
  QgsDistanceArea da;
  da.setSourceCrs( mLayer->crs().srsid() );
  da.setEllipsoidalMode( true );
  da.setEllipsoid( QgsProject::instance()->ellipsoid() );
  mFieldExpressionWidget->setGeomCalculator( da );

  // set placement methods page based on geometry type
  switch ( mLayer->geometryType() )
  {
    case QgsWkbTypes::PointGeometry:
      stackedPlacement->setCurrentWidget( pagePoint );
      break;
    case QgsWkbTypes::LineGeometry:
      stackedPlacement->setCurrentWidget( pageLine );
      break;
    case QgsWkbTypes::PolygonGeometry:
      stackedPlacement->setCurrentWidget( pagePolygon );
      break;
    case QgsWkbTypes::NullGeometry:
      break;
    case QgsWkbTypes::UnknownGeometry:
      qFatal( "unknown geometry type unexpected" );
  }

  if ( mLayer->geometryType() == QgsWkbTypes::PointGeometry )
  {
    // follow placement alignment is only valid for point layers
    if ( mFontMultiLineAlignComboBox->findText( tr( "Follow label placement" ) ) == -1 )
      mFontMultiLineAlignComboBox->addItem( tr( "Follow label placement" ) );
  }
  else
  {
    int idx = mFontMultiLineAlignComboBox->findText( tr( "Follow label placement" ) );
    if ( idx >= 0 )
      mFontMultiLineAlignComboBox->removeItem( idx );
  }

  mFieldExpressionWidget->setEnabled( mMode == Labels );
  mLabelingFrame->setEnabled( mMode == Labels );

  updateWidgetForFormat( lyr.format() );

  blockInitSignals( true );

  mFieldExpressionWidget->setRow( -1 );
  mFieldExpressionWidget->setField( lyr.fieldName );
  mCheckBoxSubstituteText->setChecked( lyr.useSubstitutions );
  mSubstitutions = lyr.substitutions;

  // populate placement options
  mCentroidRadioWhole->setChecked( lyr.centroidWhole );
  mCentroidInsideCheckBox->setChecked( lyr.centroidInside );
  mFitInsidePolygonCheckBox->setChecked( lyr.fitInPolygonOnly );
  mLineDistanceSpnBx->setValue( lyr.dist );
  mLineDistanceUnitWidget->setUnit( lyr.distInMapUnits ? QgsUnitTypes::RenderMapUnits : QgsUnitTypes::RenderMillimeters );
  mLineDistanceUnitWidget->setMapUnitScale( lyr.distMapUnitScale );
  mOffsetTypeComboBox->setCurrentIndex( mOffsetTypeComboBox->findData( lyr.offsetType ) );
  mQuadrantBtnGrp->button(( int )lyr.quadOffset )->setChecked( true );
  mPointOffsetXSpinBox->setValue( lyr.xOffset );
  mPointOffsetYSpinBox->setValue( lyr.yOffset );
  mPointOffsetUnitWidget->setUnit( lyr.labelOffsetInMapUnits ? QgsUnitTypes::RenderMapUnits : QgsUnitTypes::RenderMillimeters );
  mPointOffsetUnitWidget->setMapUnitScale( lyr.labelOffsetMapUnitScale );
  mPointAngleSpinBox->setValue( lyr.angleOffset );
  chkLineAbove->setChecked( lyr.placementFlags & QgsPalLayerSettings::AboveLine );
  chkLineBelow->setChecked( lyr.placementFlags & QgsPalLayerSettings::BelowLine );
  chkLineOn->setChecked( lyr.placementFlags & QgsPalLayerSettings::OnLine );
  chkLineOrientationDependent->setChecked( !( lyr.placementFlags & QgsPalLayerSettings::MapOrientation ) );

  switch ( lyr.placement )
  {
    case QgsPalLayerSettings::AroundPoint:
      radAroundPoint->setChecked( true );
      radAroundCentroid->setChecked( true );
      //spinAngle->setValue( lyr.angle ); // TODO: uncomment when supported
      break;
    case QgsPalLayerSettings::OverPoint:
      radOverPoint->setChecked( true );
      radOverCentroid->setChecked( true );
      break;
    case QgsPalLayerSettings::OrderedPositionsAroundPoint:
      radPredefinedOrder->setChecked( true );
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
    case QgsPalLayerSettings::PerimeterCurved:
      radPolygonPerimeterCurved->setChecked( true );
      break;
  }

  // Label repeat distance
  mRepeatDistanceSpinBox->setValue( lyr.repeatDistance );
  mRepeatDistanceUnitWidget->setUnit( lyr.repeatDistanceUnit == QgsPalLayerSettings::MapUnits ? QgsUnitTypes::RenderMapUnits : QgsUnitTypes::RenderMillimeters );
  mRepeatDistanceUnitWidget->setMapUnitScale( lyr.repeatDistanceMapUnitScale );

  mPrioritySlider->setValue( lyr.priority );
  mChkNoObstacle->setChecked( lyr.obstacle );
  mObstacleFactorSlider->setValue( lyr.obstacleFactor * 50 );
  mObstacleTypeComboBox->setCurrentIndex( mObstacleTypeComboBox->findData( lyr.obstacleType ) );
  mPolygonObstacleTypeFrame->setEnabled( lyr.obstacle );
  mObstaclePriorityFrame->setEnabled( lyr.obstacle );
  chkLabelPerFeaturePart->setChecked( lyr.labelPerPart );
  mPalShowAllLabelsForLayerChkBx->setChecked( lyr.displayAll );
  chkMergeLines->setChecked( lyr.mergeLines );
  mMinSizeSpinBox->setValue( lyr.minFeatureSize );
  mLimitLabelChkBox->setChecked( lyr.limitNumLabels );
  mLimitLabelSpinBox->setValue( lyr.maxNumLabels );

  // direction symbol(s)
  mDirectSymbChkBx->setChecked( lyr.addDirectionSymbol );
  mDirectSymbLeftLineEdit->setText( lyr.leftDirectionSymbol );
  mDirectSymbRightLineEdit->setText( lyr.rightDirectionSymbol );
  mDirectSymbRevChkBx->setChecked( lyr.reverseDirectionSymbol );

  mDirectSymbBtnGrp->button(( int )lyr.placeDirectionSymbol )->setChecked( true );
  mUpsidedownBtnGrp->button(( int )lyr.upsidedownLabels )->setChecked( true );

  // curved label max character angles
  mMaxCharAngleInDSpinBox->setValue( lyr.maxCurvedCharAngleIn );
  // lyr.maxCurvedCharAngleOut must be negative, but it is shown as positive spinbox in GUI
  mMaxCharAngleOutDSpinBox->setValue( qAbs( lyr.maxCurvedCharAngleOut ) );

  wrapCharacterEdit->setText( lyr.wrapChar );
  mFontMultiLineAlignComboBox->setCurrentIndex(( unsigned int ) lyr.multilineAlign );
  chkPreserveRotation->setChecked( lyr.preserveRotation );

  mPreviewBackgroundBtn->setColor( lyr.previewBkgrdColor );
  mPreviewBackgroundBtn->setDefaultColor( lyr.previewBkgrdColor );
  setPreviewBackground( lyr.previewBkgrdColor );

  mScaleBasedVisibilityChkBx->setChecked( lyr.scaleVisibility );
  mScaleBasedVisibilityMinSpnBx->setValue( lyr.scaleMin );
  mScaleBasedVisibilityMaxSpnBx->setValue( lyr.scaleMax );

  mFormatNumChkBx->setChecked( lyr.formatNumbers );
  mFormatNumDecimalsSpnBx->setValue( lyr.decimals );
  mFormatNumPlusSignChkBx->setChecked( lyr.plusSign );

  // set pixel size limiting checked state before unit choice so limiting can be
  // turned on as a default for map units, if minimum trigger value of 0 is used
  mFontLimitPixelChkBox->setChecked( lyr.fontLimitPixelSize );
  mMinPixelLimit = lyr.fontMinPixelSize; // ignored after first settings save
  mFontMinPixelSpinBox->setValue( lyr.fontMinPixelSize == 0 ? 3 : lyr.fontMinPixelSize );
  mFontMaxPixelSpinBox->setValue( lyr.fontMaxPixelSize );

  mZIndexSpinBox->setValue( lyr.zIndex );

  updatePlacementWidgets();
  updateLinePlacementOptions();

  // needs to come before data defined setup, so connections work
  blockInitSignals( false );

  // set up data defined toolbuttons
  // do this after other widgets are configured, so they can be enabled/disabled
  populateDataDefinedButtons( lyr );

  enableDataDefinedAlignment( mCoordXDDBtn->isActive() && mCoordYDDBtn->isActive() );
  updateUi(); // should come after data defined button setup
}

void QgsLabelingGui::blockInitSignals( bool block )
{
  chkLineAbove->blockSignals( block );
  chkLineBelow->blockSignals( block );
  mPlacePointBtnGrp->blockSignals( block );
  mPlaceLineBtnGrp->blockSignals( block );
  mPlacePolygonBtnGrp->blockSignals( block );
}

void QgsLabelingGui::apply()
{
  writeSettingsToLayer();
  mFontMissingLabel->setVisible( false );
  QgisApp::instance()->markDirty();
  // trigger refresh
  mLayer->triggerRepaint();
}

void QgsLabelingGui::writeSettingsToLayer()
{
  mLayer->setLabeling( new QgsVectorLayerSimpleLabeling );

  // all configuration is still in layer's custom properties
  QgsPalLayerSettings settings = layerSettings();
  settings.writeToLayer( mLayer );
}

void QgsLabelingGui::setLabelMode( LabelMode mode )
{
  mMode = mode;
  mFieldExpressionWidget->setEnabled( mMode == Labels );
  mLabelingFrame->setEnabled( mMode == Labels );
}

QgsPalLayerSettings QgsLabelingGui::layerSettings()
{
  QgsPalLayerSettings lyr;

  lyr.enabled = ( mMode == Labels || mMode == ObstaclesOnly );
  lyr.drawLabels = ( mMode == Labels );

  bool isExpression;
  lyr.fieldName = mFieldExpressionWidget->currentField( &isExpression );
  lyr.isExpression = isExpression;

  lyr.dist = 0;
  lyr.placementFlags = 0;

  QWidget* curPlacementWdgt = stackedPlacement->currentWidget();
  lyr.centroidWhole = mCentroidRadioWhole->isChecked();
  lyr.centroidInside = mCentroidInsideCheckBox->isChecked();
  lyr.fitInPolygonOnly = mFitInsidePolygonCheckBox->isChecked();
  lyr.dist = mLineDistanceSpnBx->value();
  lyr.distInMapUnits = ( mLineDistanceUnitWidget->unit() == QgsUnitTypes::RenderMapUnits );
  lyr.distMapUnitScale = mLineDistanceUnitWidget->getMapUnitScale();
  lyr.offsetType = static_cast< QgsPalLayerSettings::OffsetType >( mOffsetTypeComboBox->currentData().toInt() );
  if ( mQuadrantBtnGrp )
  {
    lyr.quadOffset = ( QgsPalLayerSettings::QuadrantPosition )mQuadrantBtnGrp->checkedId();
  }
  lyr.xOffset = mPointOffsetXSpinBox->value();
  lyr.yOffset = mPointOffsetYSpinBox->value();
  lyr.labelOffsetInMapUnits = ( mPointOffsetUnitWidget->unit() == QgsUnitTypes::RenderMapUnits );
  lyr.labelOffsetMapUnitScale = mPointOffsetUnitWidget->getMapUnitScale();
  lyr.angleOffset = mPointAngleSpinBox->value();
  if ( chkLineAbove->isChecked() )
    lyr.placementFlags |= QgsPalLayerSettings::AboveLine;
  if ( chkLineBelow->isChecked() )
    lyr.placementFlags |= QgsPalLayerSettings::BelowLine;
  if ( chkLineOn->isChecked() )
    lyr.placementFlags |= QgsPalLayerSettings::OnLine;
  if ( ! chkLineOrientationDependent->isChecked() )
    lyr.placementFlags |= QgsPalLayerSettings::MapOrientation;
  if (( curPlacementWdgt == pagePoint && radAroundPoint->isChecked() )
      || ( curPlacementWdgt == pagePolygon && radAroundCentroid->isChecked() ) )
  {
    lyr.placement = QgsPalLayerSettings::AroundPoint;
  }
  else if (( curPlacementWdgt == pagePoint && radOverPoint->isChecked() )
           || ( curPlacementWdgt == pagePolygon && radOverCentroid->isChecked() ) )
  {
    lyr.placement = QgsPalLayerSettings::OverPoint;
  }
  else if ( curPlacementWdgt == pagePoint && radPredefinedOrder->isChecked() )
  {
    lyr.placement = QgsPalLayerSettings::OrderedPositionsAroundPoint;
  }
  else if (( curPlacementWdgt == pageLine && radLineParallel->isChecked() )
           || ( curPlacementWdgt == pagePolygon && radPolygonPerimeter->isChecked() ) )
  {
    lyr.placement = QgsPalLayerSettings::Line;
  }
  else if ( curPlacementWdgt == pageLine && radLineCurved->isChecked() )
  {
    lyr.placement = QgsPalLayerSettings::Curved;
  }
  else if ( curPlacementWdgt == pagePolygon && radPolygonPerimeterCurved->isChecked() )
  {
    lyr.placement = QgsPalLayerSettings::PerimeterCurved;
  }
  else if (( curPlacementWdgt == pageLine && radLineHorizontal->isChecked() )
           || ( curPlacementWdgt == pagePolygon && radPolygonHorizontal->isChecked() ) )
  {
    lyr.placement = QgsPalLayerSettings::Horizontal;
  }
  else if ( radPolygonFree->isChecked() )
  {
    lyr.placement = QgsPalLayerSettings::Free;
  }
  else
  {
    qFatal( "Invalid settings" );
  }

  lyr.repeatDistance = mRepeatDistanceSpinBox->value();
  lyr.repeatDistanceUnit = mRepeatDistanceUnitWidget->unit() == QgsUnitTypes::RenderMapUnits ? QgsPalLayerSettings::MapUnits : QgsPalLayerSettings::MM;
  lyr.repeatDistanceMapUnitScale = mRepeatDistanceUnitWidget->getMapUnitScale();

  lyr.previewBkgrdColor = mPreviewBackgroundBtn->color();

  lyr.priority = mPrioritySlider->value();
  lyr.obstacle = mChkNoObstacle->isChecked() || mMode == ObstaclesOnly;
  lyr.obstacleFactor = mObstacleFactorSlider->value() / 50.0;
  lyr.obstacleType = ( QgsPalLayerSettings::ObstacleType )mObstacleTypeComboBox->currentData().toInt();
  lyr.labelPerPart = chkLabelPerFeaturePart->isChecked();
  lyr.displayAll = mPalShowAllLabelsForLayerChkBx->isChecked();
  lyr.mergeLines = chkMergeLines->isChecked();

  lyr.scaleVisibility = mScaleBasedVisibilityChkBx->isChecked();
  lyr.scaleMin = mScaleBasedVisibilityMinSpnBx->value();
  lyr.scaleMax = mScaleBasedVisibilityMaxSpnBx->value();
  lyr.useSubstitutions = mCheckBoxSubstituteText->isChecked();
  lyr.substitutions = mSubstitutions;

  lyr.setFormat( format() );

  // format numbers
  lyr.formatNumbers = mFormatNumChkBx->isChecked();
  lyr.decimals = mFormatNumDecimalsSpnBx->value();
  lyr.plusSign = mFormatNumPlusSignChkBx->isChecked();

  // direction symbol(s)
  lyr.addDirectionSymbol = mDirectSymbChkBx->isChecked();
  lyr.leftDirectionSymbol = mDirectSymbLeftLineEdit->text();
  lyr.rightDirectionSymbol = mDirectSymbRightLineEdit->text();
  lyr.reverseDirectionSymbol = mDirectSymbRevChkBx->isChecked();
  if ( mDirectSymbBtnGrp )
  {
    lyr.placeDirectionSymbol = ( QgsPalLayerSettings::DirectionSymbols )mDirectSymbBtnGrp->checkedId();
  }
  if ( mUpsidedownBtnGrp )
  {
    lyr.upsidedownLabels = ( QgsPalLayerSettings::UpsideDownLabels )mUpsidedownBtnGrp->checkedId();
  }

  lyr.maxCurvedCharAngleIn = mMaxCharAngleInDSpinBox->value();
  // lyr.maxCurvedCharAngleOut must be negative, but it is shown as positive spinbox in GUI
  lyr.maxCurvedCharAngleOut = -mMaxCharAngleOutDSpinBox->value();


  lyr.minFeatureSize = mMinSizeSpinBox->value();
  lyr.limitNumLabels = mLimitLabelChkBox->isChecked();
  lyr.maxNumLabels = mLimitLabelSpinBox->value();
  lyr.fontLimitPixelSize = mFontLimitPixelChkBox->isChecked();
  lyr.fontMinPixelSize = mFontMinPixelSpinBox->value();
  lyr.fontMaxPixelSize = mFontMaxPixelSpinBox->value();
  lyr.wrapChar = wrapCharacterEdit->text();
  lyr.multilineAlign = ( QgsPalLayerSettings::MultiLineAlign ) mFontMultiLineAlignComboBox->currentIndex();
  lyr.preserveRotation = chkPreserveRotation->isChecked();

  lyr.zIndex = mZIndexSpinBox->value();

  // data defined labeling
  // text style
  setDataDefinedProperty( mFontDDBtn, QgsPalLayerSettings::Family, lyr );
  setDataDefinedProperty( mFontStyleDDBtn, QgsPalLayerSettings::FontStyle, lyr );
  setDataDefinedProperty( mFontUnderlineDDBtn, QgsPalLayerSettings::Underline, lyr );
  setDataDefinedProperty( mFontStrikeoutDDBtn, QgsPalLayerSettings::Strikeout, lyr );
  setDataDefinedProperty( mFontBoldDDBtn, QgsPalLayerSettings::Bold, lyr );
  setDataDefinedProperty( mFontItalicDDBtn, QgsPalLayerSettings::Italic, lyr );
  setDataDefinedProperty( mFontSizeDDBtn, QgsPalLayerSettings::Size, lyr );
  setDataDefinedProperty( mFontUnitsDDBtn, QgsPalLayerSettings::FontSizeUnit, lyr );
  setDataDefinedProperty( mFontColorDDBtn, QgsPalLayerSettings::Color, lyr );
  setDataDefinedProperty( mFontTranspDDBtn, QgsPalLayerSettings::FontTransp, lyr );
  setDataDefinedProperty( mFontCaseDDBtn, QgsPalLayerSettings::FontCase, lyr );
  setDataDefinedProperty( mFontLetterSpacingDDBtn, QgsPalLayerSettings::FontLetterSpacing, lyr );
  setDataDefinedProperty( mFontWordSpacingDDBtn, QgsPalLayerSettings::FontWordSpacing, lyr );
  setDataDefinedProperty( mFontBlendModeDDBtn, QgsPalLayerSettings::FontBlendMode, lyr );

  // text formatting
  setDataDefinedProperty( mWrapCharDDBtn, QgsPalLayerSettings::MultiLineWrapChar, lyr );
  setDataDefinedProperty( mFontLineHeightDDBtn, QgsPalLayerSettings::MultiLineHeight, lyr );
  setDataDefinedProperty( mFontMultiLineAlignDDBtn, QgsPalLayerSettings::MultiLineAlignment, lyr );
  setDataDefinedProperty( mDirectSymbDDBtn, QgsPalLayerSettings::DirSymbDraw, lyr );
  setDataDefinedProperty( mDirectSymbLeftDDBtn, QgsPalLayerSettings::DirSymbLeft, lyr );
  setDataDefinedProperty( mDirectSymbRightDDBtn, QgsPalLayerSettings::DirSymbRight, lyr );
  setDataDefinedProperty( mDirectSymbPlacementDDBtn, QgsPalLayerSettings::DirSymbPlacement, lyr );
  setDataDefinedProperty( mDirectSymbRevDDBtn, QgsPalLayerSettings::DirSymbReverse, lyr );
  setDataDefinedProperty( mFormatNumDDBtn, QgsPalLayerSettings::NumFormat, lyr );
  setDataDefinedProperty( mFormatNumDecimalsDDBtn, QgsPalLayerSettings::NumDecimals, lyr );
  setDataDefinedProperty( mFormatNumPlusSignDDBtn, QgsPalLayerSettings::NumPlusSign, lyr );

  // text buffer
  setDataDefinedProperty( mBufferDrawDDBtn, QgsPalLayerSettings::BufferDraw, lyr );
  setDataDefinedProperty( mBufferSizeDDBtn, QgsPalLayerSettings::BufferSize, lyr );
  setDataDefinedProperty( mBufferUnitsDDBtn, QgsPalLayerSettings::BufferUnit, lyr );
  setDataDefinedProperty( mBufferColorDDBtn, QgsPalLayerSettings::BufferColor, lyr );
  setDataDefinedProperty( mBufferTranspDDBtn, QgsPalLayerSettings::BufferTransp, lyr );
  setDataDefinedProperty( mBufferJoinStyleDDBtn, QgsPalLayerSettings::BufferJoinStyle, lyr );
  setDataDefinedProperty( mBufferBlendModeDDBtn, QgsPalLayerSettings::BufferBlendMode, lyr );

  // background
  setDataDefinedProperty( mShapeDrawDDBtn, QgsPalLayerSettings::ShapeDraw, lyr );
  setDataDefinedProperty( mShapeTypeDDBtn, QgsPalLayerSettings::ShapeKind, lyr );
  setDataDefinedProperty( mShapeSVGPathDDBtn, QgsPalLayerSettings::ShapeSVGFile, lyr );
  setDataDefinedProperty( mShapeSizeTypeDDBtn, QgsPalLayerSettings::ShapeSizeType, lyr );
  setDataDefinedProperty( mShapeSizeXDDBtn, QgsPalLayerSettings::ShapeSizeX, lyr );
  setDataDefinedProperty( mShapeSizeYDDBtn, QgsPalLayerSettings::ShapeSizeY, lyr );
  setDataDefinedProperty( mShapeSizeUnitsDDBtn, QgsPalLayerSettings::ShapeSizeUnits, lyr );
  setDataDefinedProperty( mShapeRotationTypeDDBtn, QgsPalLayerSettings::ShapeRotationType, lyr );
  setDataDefinedProperty( mShapeRotationDDBtn, QgsPalLayerSettings::ShapeRotation, lyr );
  setDataDefinedProperty( mShapeOffsetDDBtn, QgsPalLayerSettings::ShapeOffset, lyr );
  setDataDefinedProperty( mShapeOffsetUnitsDDBtn, QgsPalLayerSettings::ShapeOffsetUnits, lyr );
  setDataDefinedProperty( mShapeRadiusDDBtn, QgsPalLayerSettings::ShapeRadii, lyr );
  setDataDefinedProperty( mShapeRadiusUnitsDDBtn, QgsPalLayerSettings::ShapeRadiiUnits, lyr );
  setDataDefinedProperty( mShapeTranspDDBtn, QgsPalLayerSettings::ShapeTransparency, lyr );
  setDataDefinedProperty( mShapeBlendModeDDBtn, QgsPalLayerSettings::ShapeBlendMode, lyr );
  setDataDefinedProperty( mShapeFillColorDDBtn, QgsPalLayerSettings::ShapeFillColor, lyr );
  setDataDefinedProperty( mShapeBorderColorDDBtn, QgsPalLayerSettings::ShapeBorderColor, lyr );
  setDataDefinedProperty( mShapeBorderWidthDDBtn, QgsPalLayerSettings::ShapeBorderWidth, lyr );
  setDataDefinedProperty( mShapeBorderUnitsDDBtn, QgsPalLayerSettings::ShapeBorderWidthUnits, lyr );
  setDataDefinedProperty( mShapePenStyleDDBtn, QgsPalLayerSettings::ShapeJoinStyle, lyr );

  // drop shadow
  setDataDefinedProperty( mShadowDrawDDBtn, QgsPalLayerSettings::ShadowDraw, lyr );
  setDataDefinedProperty( mShadowUnderDDBtn, QgsPalLayerSettings::ShadowUnder, lyr );
  setDataDefinedProperty( mShadowOffsetAngleDDBtn, QgsPalLayerSettings::ShadowOffsetAngle, lyr );
  setDataDefinedProperty( mShadowOffsetDDBtn, QgsPalLayerSettings::ShadowOffsetDist, lyr );
  setDataDefinedProperty( mShadowOffsetUnitsDDBtn, QgsPalLayerSettings::ShadowOffsetUnits, lyr );
  setDataDefinedProperty( mShadowRadiusDDBtn, QgsPalLayerSettings::ShadowRadius, lyr );
  setDataDefinedProperty( mShadowRadiusUnitsDDBtn, QgsPalLayerSettings::ShadowRadiusUnits, lyr );
  setDataDefinedProperty( mShadowTranspDDBtn, QgsPalLayerSettings::ShadowTransparency, lyr );
  setDataDefinedProperty( mShadowScaleDDBtn, QgsPalLayerSettings::ShadowScale, lyr );
  setDataDefinedProperty( mShadowColorDDBtn, QgsPalLayerSettings::ShadowColor, lyr );
  setDataDefinedProperty( mShadowBlendDDBtn, QgsPalLayerSettings::ShadowBlendMode, lyr );

  // placement
  setDataDefinedProperty( mCentroidDDBtn, QgsPalLayerSettings::CentroidWhole, lyr );
  setDataDefinedProperty( mPointQuadOffsetDDBtn, QgsPalLayerSettings::OffsetQuad, lyr );
  setDataDefinedProperty( mPointPositionOrderDDBtn, QgsPalLayerSettings::PredefinedPositionOrder, lyr );
  setDataDefinedProperty( mPointOffsetDDBtn, QgsPalLayerSettings::OffsetXY, lyr );
  setDataDefinedProperty( mPointOffsetUnitsDDBtn, QgsPalLayerSettings::OffsetUnits, lyr );
  setDataDefinedProperty( mLineDistanceDDBtn, QgsPalLayerSettings::LabelDistance, lyr );
  setDataDefinedProperty( mLineDistanceUnitDDBtn, QgsPalLayerSettings::DistanceUnits, lyr );
  // TODO: is this necessary? maybe just use the data defined-only rotation?
  //setDataDefinedProperty( mPointAngleDDBtn, QgsPalLayerSettings::OffsetRotation, lyr );
  setDataDefinedProperty( mMaxCharAngleDDBtn, QgsPalLayerSettings::CurvedCharAngleInOut, lyr );
  setDataDefinedProperty( mRepeatDistanceDDBtn, QgsPalLayerSettings::RepeatDistance, lyr );
  setDataDefinedProperty( mRepeatDistanceUnitDDBtn, QgsPalLayerSettings::RepeatDistanceUnit, lyr );
  setDataDefinedProperty( mPriorityDDBtn, QgsPalLayerSettings::Priority, lyr );

  // data defined-only
  setDataDefinedProperty( mCoordXDDBtn, QgsPalLayerSettings::PositionX, lyr );
  setDataDefinedProperty( mCoordYDDBtn, QgsPalLayerSettings::PositionY, lyr );
  setDataDefinedProperty( mCoordAlignmentHDDBtn, QgsPalLayerSettings::Hali, lyr );
  setDataDefinedProperty( mCoordAlignmentVDDBtn, QgsPalLayerSettings::Vali, lyr );
  setDataDefinedProperty( mCoordRotationDDBtn, QgsPalLayerSettings::Rotation, lyr );

  // rendering
  setDataDefinedProperty( mScaleBasedVisibilityDDBtn, QgsPalLayerSettings::ScaleVisibility, lyr );
  setDataDefinedProperty( mScaleBasedVisibilityMinDDBtn, QgsPalLayerSettings::MinScale, lyr );
  setDataDefinedProperty( mScaleBasedVisibilityMaxDDBtn, QgsPalLayerSettings::MaxScale, lyr );
  setDataDefinedProperty( mFontLimitPixelDDBtn, QgsPalLayerSettings::FontLimitPixel, lyr );
  setDataDefinedProperty( mFontMinPixelDDBtn, QgsPalLayerSettings::FontMinPixel, lyr );
  setDataDefinedProperty( mFontMaxPixelDDBtn, QgsPalLayerSettings::FontMaxPixel, lyr );
  setDataDefinedProperty( mShowLabelDDBtn, QgsPalLayerSettings::Show, lyr );
  setDataDefinedProperty( mAlwaysShowDDBtn, QgsPalLayerSettings::AlwaysShow, lyr );
  setDataDefinedProperty( mIsObstacleDDBtn, QgsPalLayerSettings::IsObstacle, lyr );
  setDataDefinedProperty( mObstacleFactorDDBtn, QgsPalLayerSettings::ObstacleFactor, lyr );
  setDataDefinedProperty( mZIndexDDBtn, QgsPalLayerSettings::ZIndex, lyr );

  return lyr;
}

void QgsLabelingGui::setDataDefinedProperty( const QgsDataDefinedButton* ddBtn, QgsPalLayerSettings::DataDefinedProperties p, QgsPalLayerSettings& lyr )
{
  const QMap< QString, QString >& map = ddBtn->definedProperty();
  lyr.setDataDefinedProperty( p, map.value( QStringLiteral( "active" ) ).toInt(), map.value( QStringLiteral( "useexpr" ) ).toInt(), map.value( QStringLiteral( "expression" ) ), map.value( QStringLiteral( "field" ) ) );
}

void QgsLabelingGui::populateDataDefinedButtons( QgsPalLayerSettings& s )
{
  Q_FOREACH ( QgsDataDefinedButton* button, findChildren< QgsDataDefinedButton* >() )
  {
    button->registerExpressionContextGenerator( this );
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
                        trString + QStringLiteral( "[<b>NoChange</b>|<b>Upper</b>|<br>"
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
                                  QgsDataDefinedButton::String, trString + "[<b>Left</b>|<b>Center</b>|<b>Right</b>|<b>Follow</b>]" );

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
                         trString + QStringLiteral( "[<b>Rectangle</b>|<b>Square</b>|<br>"
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
                           trString + QStringLiteral( "[<b>Lowest</b>|<b>Text</b>|<br>"
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
                               tr( "int<br>" ) + QStringLiteral( "[<b>0</b>=Above Left|<b>1</b>=Above|<b>2</b>=Above Right|<br>"
                                                                 "<b>3</b>=Left|<b>4</b>=Over|<b>5</b>=Right|<br>"
                                                                 "<b>6</b>=Below Left|<b>7</b>=Below|<b>8</b>=Below Right]" ) );
  mPointPositionOrderDDBtn->init( mLayer, s.dataDefinedProperty( QgsPalLayerSettings::PredefinedPositionOrder ),
                                  QgsDataDefinedButton::String,
                                  tr( "Comma separated list of placements in order of priority<br>" )
                                  + QStringLiteral( "[<b>TL</b>=Top left|<b>TSL</b>=Top, slightly left|<b>T</b>=Top middle|<br>"
                                                    "<b>TSR</b>=Top, slightly right|<b>TR</b>=Top right|<br>"
                                                    "<b>L</b>=Left|<b>R</b>=Right|<br>"
                                                    "<b>BL</b>=Bottom left|<b>BSL</b>=Bottom, slightly left|<b>B</b>=Bottom middle|<br>"
                                                    "<b>BSR</b>=Bottom, slightly right|<b>BR</b>=Bottom right]" ) );
  mPointOffsetDDBtn->init( mLayer, s.dataDefinedProperty( QgsPalLayerSettings::OffsetXY ),
                           QgsDataDefinedButton::AnyType, QgsDataDefinedButton::doubleXYDesc() );
  mPointOffsetUnitsDDBtn->init( mLayer, s.dataDefinedProperty( QgsPalLayerSettings::OffsetUnits ),
                                QgsDataDefinedButton::String, QgsDataDefinedButton::unitsMmMuDesc() );
  mLineDistanceDDBtn->init( mLayer, s.dataDefinedProperty( QgsPalLayerSettings::LabelDistance ),
                            QgsDataDefinedButton::AnyType, QgsDataDefinedButton::doublePosDesc() );
  mLineDistanceUnitDDBtn->init( mLayer, s.dataDefinedProperty( QgsPalLayerSettings::DistanceUnits ),
                                QgsDataDefinedButton::String, QgsDataDefinedButton::unitsMmMuDesc() );
  mPriorityDDBtn->init( mLayer, s.dataDefinedProperty( QgsPalLayerSettings::Priority ),
                        QgsDataDefinedButton::AnyType, tr( "double [0.0-10.0]" ) );

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
                               trString + QStringLiteral( "[<b>Bottom</b>|<b>Base</b>|<br>"
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

  mIsObstacleDDBtn->init( mLayer, s.dataDefinedProperty( QgsPalLayerSettings::IsObstacle ),
                          QgsDataDefinedButton::AnyType, QgsDataDefinedButton::boolDesc() );
  mObstacleFactorDDBtn->init( mLayer, s.dataDefinedProperty( QgsPalLayerSettings::ObstacleFactor ),
                              QgsDataDefinedButton::AnyType, tr( "double [0.0-10.0]" ) );
  mZIndexDDBtn->init( mLayer, s.dataDefinedProperty( QgsPalLayerSettings::ZIndex ),
                      QgsDataDefinedButton::AnyType, QgsDataDefinedButton::doubleDesc() );
}

void QgsLabelingGui::syncDefinedCheckboxFrame( QgsDataDefinedButton* ddBtn, QCheckBox* chkBx, QFrame* f )
{
  if ( ddBtn->isActive() && !chkBx->isChecked() )
  {
    chkBx->setChecked( true );
  }
  f->setEnabled( chkBx->isChecked() );
}

void QgsLabelingGui::updateUi()
{
  // enable/disable inline groupbox-like setups (that need to honor data defined setting)

  syncDefinedCheckboxFrame( mBufferDrawDDBtn, mBufferDrawChkBx, mBufferFrame );
  syncDefinedCheckboxFrame( mShapeDrawDDBtn, mShapeDrawChkBx, mShapeFrame );
  syncDefinedCheckboxFrame( mShadowDrawDDBtn, mShadowDrawChkBx, mShadowFrame );

  syncDefinedCheckboxFrame( mDirectSymbDDBtn, mDirectSymbChkBx, mDirectSymbFrame );
  syncDefinedCheckboxFrame( mFormatNumDDBtn, mFormatNumChkBx, mFormatNumFrame );
  syncDefinedCheckboxFrame( mScaleBasedVisibilityDDBtn, mScaleBasedVisibilityChkBx, mScaleBasedVisibilityFrame );
  syncDefinedCheckboxFrame( mFontLimitPixelDDBtn, mFontLimitPixelChkBox, mFontLimitPixelFrame );

  chkMergeLines->setEnabled( !mDirectSymbChkBx->isChecked() );
  if ( mDirectSymbChkBx->isChecked() )
  {
    chkMergeLines->setToolTip( tr( "This option is not compatible with line direction symbols." ) );
  }
  else
  {
    chkMergeLines->setToolTip( QString() );
  }
}




