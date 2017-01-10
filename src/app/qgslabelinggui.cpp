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

void QgsLabelingGui::registerDataDefinedButton( QgsDataDefinedButtonV2* button, QgsPalLayerSettings::Property key,
    QgsDataDefinedButtonV2::DataType type, const QString& description )
{
  button->init( mLayer, mProperties.property( key ), type, description );
  button->setProperty( "propertyKey", key );
  connect( button, &QgsDataDefinedButtonV2::changed, this, &QgsLabelingGui::updateProperty );
  button->registerExpressionContextGenerator( this );
}

void QgsLabelingGui::updateProperty()
{
  QgsDataDefinedButtonV2* button = qobject_cast<QgsDataDefinedButtonV2*>( sender() );
  QgsPalLayerSettings::Property key = static_cast< QgsPalLayerSettings::Property >( button->property( "propertyKey" ).toInt() );
  mProperties.setProperty( key, button->toProperty() );
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

  mProperties = lyr.properties();

  updatePlacementWidgets();
  updateLinePlacementOptions();

  // needs to come before data defined setup, so connections work
  blockInitSignals( false );

  // set up data defined toolbuttons
  // do this after other widgets are configured, so they can be enabled/disabled
  populateDataDefinedButtons();

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

  lyr.setProperties( mProperties );

  return lyr;
}

void QgsLabelingGui::populateDataDefinedButtons()
{
  QString trString = tr( "string " );

  // text style
  registerDataDefinedButton( mFontDDBtn, QgsPalLayerSettings::Family,
                             QgsDataDefinedButtonV2::String,
                             trString + tr( "[<b>family</b>|<b>family[foundry]</b>],<br>"
                                            "e.g. Helvetica or Helvetica [Cronyx]" ) );

  registerDataDefinedButton( mFontStyleDDBtn, QgsPalLayerSettings::FontStyle,
                             QgsDataDefinedButtonV2::String,
                             trString + tr( "[<b>font style name</b>|<b>Ignore</b>],<br>"
                                            "e.g. Bold Condensed or Light Italic" ) );

  registerDataDefinedButton( mFontUnderlineDDBtn, QgsPalLayerSettings::Underline,
                             QgsDataDefinedButtonV2::AnyType, QgsDataDefinedButtonV2::boolDesc() );

  registerDataDefinedButton( mFontStrikeoutDDBtn, QgsPalLayerSettings::Strikeout,
                             QgsDataDefinedButtonV2::AnyType, QgsDataDefinedButtonV2::boolDesc() );

  registerDataDefinedButton( mFontBoldDDBtn, QgsPalLayerSettings::Bold,
                             QgsDataDefinedButtonV2::AnyType, QgsDataDefinedButtonV2::boolDesc() );

  registerDataDefinedButton( mFontItalicDDBtn, QgsPalLayerSettings::Italic,
                             QgsDataDefinedButtonV2::AnyType, QgsDataDefinedButtonV2::boolDesc() );

  registerDataDefinedButton( mFontSizeDDBtn, QgsPalLayerSettings::Size,
                             QgsDataDefinedButtonV2::AnyType, QgsDataDefinedButtonV2::doublePosDesc() );

  registerDataDefinedButton( mFontUnitsDDBtn, QgsPalLayerSettings::FontSizeUnit,
                             QgsDataDefinedButtonV2::String, trString + "[<b>Points</b>|<b>MapUnit</b>]" );

  registerDataDefinedButton( mFontColorDDBtn, QgsPalLayerSettings::Color,
                             QgsDataDefinedButtonV2::String, QgsDataDefinedButtonV2::colorNoAlphaDesc() );

  registerDataDefinedButton( mFontTranspDDBtn, QgsPalLayerSettings::FontTransp,
                             QgsDataDefinedButtonV2::AnyType, QgsDataDefinedButtonV2::intTranspDesc() );

  registerDataDefinedButton( mFontCaseDDBtn, QgsPalLayerSettings::FontCase,
                             QgsDataDefinedButtonV2::String,
                             trString + QStringLiteral( "[<b>NoChange</b>|<b>Upper</b>|<br>"
                                                        "<b>Lower</b>|<b>Capitalize</b>]" ) );

  registerDataDefinedButton( mFontLetterSpacingDDBtn, QgsPalLayerSettings::FontLetterSpacing,
                             QgsDataDefinedButtonV2::AnyType, QgsDataDefinedButtonV2::doubleDesc() );
  registerDataDefinedButton( mFontWordSpacingDDBtn, QgsPalLayerSettings::FontWordSpacing,
                             QgsDataDefinedButtonV2::AnyType, QgsDataDefinedButtonV2::doubleDesc() );

  registerDataDefinedButton( mFontBlendModeDDBtn, QgsPalLayerSettings::FontBlendMode,
                             QgsDataDefinedButtonV2::String, QgsDataDefinedButtonV2::blendModesDesc() );

  // text formatting
  registerDataDefinedButton( mWrapCharDDBtn, QgsPalLayerSettings::MultiLineWrapChar,
                             QgsDataDefinedButtonV2::String, QgsDataDefinedButtonV2::anyStringDesc() );
  registerDataDefinedButton( mFontLineHeightDDBtn, QgsPalLayerSettings::MultiLineHeight,
                             QgsDataDefinedButtonV2::AnyType, tr( "double [0.0-10.0]" ) );
  registerDataDefinedButton( mFontMultiLineAlignDDBtn, QgsPalLayerSettings::MultiLineAlignment,
                             QgsDataDefinedButtonV2::String, trString + "[<b>Left</b>|<b>Center</b>|<b>Right</b>|<b>Follow</b>]" );

  registerDataDefinedButton( mDirectSymbDDBtn, QgsPalLayerSettings::DirSymbDraw,
                             QgsDataDefinedButtonV2::AnyType, QgsDataDefinedButtonV2::boolDesc() );
  mDirectSymbDDBtn->registerCheckedWidget( mDirectSymbChkBx );
  registerDataDefinedButton( mDirectSymbLeftDDBtn, QgsPalLayerSettings::DirSymbLeft,
                             QgsDataDefinedButtonV2::String, QgsDataDefinedButtonV2::anyStringDesc() );
  registerDataDefinedButton( mDirectSymbRightDDBtn, QgsPalLayerSettings::DirSymbRight,
                             QgsDataDefinedButtonV2::String, QgsDataDefinedButtonV2::anyStringDesc() );

  registerDataDefinedButton( mDirectSymbPlacementDDBtn, QgsPalLayerSettings::DirSymbPlacement,
                             QgsDataDefinedButtonV2::String,
                             trString + "[<b>LeftRight</b>|<b>Above</b>|<b>Below</b>]" );
  registerDataDefinedButton( mDirectSymbRevDDBtn, QgsPalLayerSettings::DirSymbReverse,
                             QgsDataDefinedButtonV2::AnyType, QgsDataDefinedButtonV2::boolDesc() );

  registerDataDefinedButton( mFormatNumDDBtn, QgsPalLayerSettings::NumFormat,
                             QgsDataDefinedButtonV2::AnyType, QgsDataDefinedButtonV2::boolDesc() );
  mFormatNumDDBtn->registerCheckedWidget( mFormatNumChkBx );
  registerDataDefinedButton( mFormatNumDecimalsDDBtn, QgsPalLayerSettings::NumDecimals,
                             QgsDataDefinedButtonV2::AnyType, tr( "int [0-20]" ) );
  registerDataDefinedButton( mFormatNumPlusSignDDBtn, QgsPalLayerSettings::NumPlusSign,
                             QgsDataDefinedButtonV2::AnyType, QgsDataDefinedButtonV2::boolDesc() );

  // text buffer
  registerDataDefinedButton( mBufferDrawDDBtn, QgsPalLayerSettings::BufferDraw,
                             QgsDataDefinedButtonV2::AnyType, QgsDataDefinedButtonV2::boolDesc() );
  mBufferDrawDDBtn->registerCheckedWidget( mBufferDrawChkBx );
  registerDataDefinedButton( mBufferSizeDDBtn, QgsPalLayerSettings::BufferSize,
                             QgsDataDefinedButtonV2::AnyType, QgsDataDefinedButtonV2::doublePosDesc() );
  registerDataDefinedButton( mBufferUnitsDDBtn, QgsPalLayerSettings::BufferUnit,
                             QgsDataDefinedButtonV2::String, QgsDataDefinedButtonV2::unitsMmMuDesc() );
  registerDataDefinedButton( mBufferColorDDBtn, QgsPalLayerSettings::BufferColor,
                             QgsDataDefinedButtonV2::String, QgsDataDefinedButtonV2::colorNoAlphaDesc() );
  registerDataDefinedButton( mBufferTranspDDBtn, QgsPalLayerSettings::BufferTransp,
                             QgsDataDefinedButtonV2::AnyType, QgsDataDefinedButtonV2::intTranspDesc() );
  registerDataDefinedButton( mBufferJoinStyleDDBtn, QgsPalLayerSettings::BufferJoinStyle,
                             QgsDataDefinedButtonV2::String, QgsDataDefinedButtonV2::penJoinStyleDesc() );
  registerDataDefinedButton( mBufferBlendModeDDBtn, QgsPalLayerSettings::BufferBlendMode,
                             QgsDataDefinedButtonV2::String, QgsDataDefinedButtonV2::blendModesDesc() );

  // background
  registerDataDefinedButton( mShapeDrawDDBtn, QgsPalLayerSettings::ShapeDraw,
                             QgsDataDefinedButtonV2::AnyType, QgsDataDefinedButtonV2::boolDesc() );
  mShapeDrawDDBtn->registerCheckedWidget( mShapeDrawChkBx );
  registerDataDefinedButton( mShapeTypeDDBtn, QgsPalLayerSettings::ShapeKind,
                             QgsDataDefinedButtonV2::String,
                             trString + QStringLiteral( "[<b>Rectangle</b>|<b>Square</b>|<br>"
                                                        "<b>Ellipse</b>|<b>Circle</b>|<b>SVG</b>]" ) );
  registerDataDefinedButton( mShapeSVGPathDDBtn, QgsPalLayerSettings::ShapeSVGFile,
                             QgsDataDefinedButtonV2::String, QgsDataDefinedButtonV2::svgPathDesc() );
  registerDataDefinedButton( mShapeSizeTypeDDBtn, QgsPalLayerSettings::ShapeSizeType,
                             QgsDataDefinedButtonV2::String,
                             trString + "[<b>Buffer</b>|<b>Fixed</b>]" );
  registerDataDefinedButton( mShapeSizeXDDBtn, QgsPalLayerSettings::ShapeSizeX,
                             QgsDataDefinedButtonV2::AnyType, QgsDataDefinedButtonV2::doubleDesc() );
  registerDataDefinedButton( mShapeSizeYDDBtn, QgsPalLayerSettings::ShapeSizeY,
                             QgsDataDefinedButtonV2::AnyType, QgsDataDefinedButtonV2::doubleDesc() );
  registerDataDefinedButton( mShapeSizeUnitsDDBtn, QgsPalLayerSettings::ShapeSizeUnits,
                             QgsDataDefinedButtonV2::String, QgsDataDefinedButtonV2::unitsMmMuDesc() );
  registerDataDefinedButton( mShapeRotationTypeDDBtn, QgsPalLayerSettings::ShapeRotationType,
                             QgsDataDefinedButtonV2::String,
                             trString + "[<b>Sync</b>|<b>Offset</b>|<b>Fixed</b>]" );
  registerDataDefinedButton( mShapeRotationDDBtn, QgsPalLayerSettings::ShapeRotation,
                             QgsDataDefinedButtonV2::AnyType, QgsDataDefinedButtonV2::double180RotDesc() );
  registerDataDefinedButton( mShapeOffsetDDBtn, QgsPalLayerSettings::ShapeOffset,
                             QgsDataDefinedButtonV2::AnyType, QgsDataDefinedButtonV2::doubleXYDesc() );
  registerDataDefinedButton( mShapeOffsetUnitsDDBtn, QgsPalLayerSettings::ShapeOffsetUnits,
                             QgsDataDefinedButtonV2::String, QgsDataDefinedButtonV2::unitsMmMuDesc() );
  registerDataDefinedButton( mShapeRadiusDDBtn, QgsPalLayerSettings::ShapeRadii,
                             QgsDataDefinedButtonV2::AnyType, QgsDataDefinedButtonV2::doubleXYDesc() );
  registerDataDefinedButton( mShapeRadiusUnitsDDBtn, QgsPalLayerSettings::ShapeRadiiUnits,
                             QgsDataDefinedButtonV2::String, QgsDataDefinedButtonV2::unitsMmMuPercentDesc() );
  registerDataDefinedButton( mShapeTranspDDBtn, QgsPalLayerSettings::ShapeTransparency,
                             QgsDataDefinedButtonV2::AnyType, QgsDataDefinedButtonV2::intTranspDesc() );
  registerDataDefinedButton( mShapeBlendModeDDBtn, QgsPalLayerSettings::ShapeBlendMode,
                             QgsDataDefinedButtonV2::String, QgsDataDefinedButtonV2::blendModesDesc() );
  registerDataDefinedButton( mShapeFillColorDDBtn, QgsPalLayerSettings::ShapeFillColor,
                             QgsDataDefinedButtonV2::String, QgsDataDefinedButtonV2::colorAlphaDesc() );
  registerDataDefinedButton( mShapeBorderColorDDBtn, QgsPalLayerSettings::ShapeBorderColor,
                             QgsDataDefinedButtonV2::String, QgsDataDefinedButtonV2::colorAlphaDesc() );
  registerDataDefinedButton( mShapeBorderWidthDDBtn, QgsPalLayerSettings::ShapeBorderWidth,
                             QgsDataDefinedButtonV2::AnyType, QgsDataDefinedButtonV2::doublePosDesc() );
  registerDataDefinedButton( mShapeBorderUnitsDDBtn, QgsPalLayerSettings::ShapeBorderWidthUnits,
                             QgsDataDefinedButtonV2::String, QgsDataDefinedButtonV2::unitsMmMuDesc() );
  registerDataDefinedButton( mShapePenStyleDDBtn, QgsPalLayerSettings::ShapeJoinStyle,
                             QgsDataDefinedButtonV2::String, QgsDataDefinedButtonV2::penJoinStyleDesc() );

  // drop shadows
  registerDataDefinedButton( mShadowDrawDDBtn, QgsPalLayerSettings::ShadowDraw,
                             QgsDataDefinedButtonV2::AnyType, QgsDataDefinedButtonV2::boolDesc() );
  mShadowDrawDDBtn->registerCheckedWidget( mShadowDrawChkBx );
  registerDataDefinedButton( mShadowUnderDDBtn, QgsPalLayerSettings::ShadowUnder,
                             QgsDataDefinedButtonV2::String,
                             trString + QStringLiteral( "[<b>Lowest</b>|<b>Text</b>|<br>"
                                                        "<b>Buffer</b>|<b>Background</b>]" ) );
  registerDataDefinedButton( mShadowOffsetAngleDDBtn, QgsPalLayerSettings::ShadowOffsetAngle,
                             QgsDataDefinedButtonV2::AnyType, QgsDataDefinedButtonV2::double180RotDesc() );
  registerDataDefinedButton( mShadowOffsetDDBtn, QgsPalLayerSettings::ShadowOffsetDist,
                             QgsDataDefinedButtonV2::AnyType, QgsDataDefinedButtonV2::doublePosDesc() );
  registerDataDefinedButton( mShadowOffsetUnitsDDBtn, QgsPalLayerSettings::ShadowOffsetUnits,
                             QgsDataDefinedButtonV2::String, QgsDataDefinedButtonV2::unitsMmMuDesc() );
  registerDataDefinedButton( mShadowRadiusDDBtn, QgsPalLayerSettings::ShadowRadius,
                             QgsDataDefinedButtonV2::AnyType, QgsDataDefinedButtonV2::doublePosDesc() );
  registerDataDefinedButton( mShadowRadiusUnitsDDBtn, QgsPalLayerSettings::ShadowRadiusUnits,
                             QgsDataDefinedButtonV2::String, QgsDataDefinedButtonV2::unitsMmMuDesc() );
  registerDataDefinedButton( mShadowTranspDDBtn, QgsPalLayerSettings::ShadowTransparency,
                             QgsDataDefinedButtonV2::AnyType, QgsDataDefinedButtonV2::intTranspDesc() );
  registerDataDefinedButton( mShadowScaleDDBtn, QgsPalLayerSettings::ShadowScale,
                             QgsDataDefinedButtonV2::AnyType, tr( "int [0-2000]" ) );
  registerDataDefinedButton( mShadowColorDDBtn, QgsPalLayerSettings::ShadowColor,
                             QgsDataDefinedButtonV2::String, QgsDataDefinedButtonV2::colorNoAlphaDesc() );
  registerDataDefinedButton( mShadowBlendDDBtn, QgsPalLayerSettings::ShadowBlendMode,
                             QgsDataDefinedButtonV2::String, QgsDataDefinedButtonV2::blendModesDesc() );

  // placement
  registerDataDefinedButton( mCentroidDDBtn, QgsPalLayerSettings::CentroidWhole,
                             QgsDataDefinedButtonV2::String,
                             trString + "[<b>Visible</b>|<b>Whole</b>]" );
  registerDataDefinedButton( mPointQuadOffsetDDBtn, QgsPalLayerSettings::OffsetQuad,
                             QgsDataDefinedButtonV2::AnyType,
                             tr( "int<br>" ) + QStringLiteral( "[<b>0</b>=Above Left|<b>1</b>=Above|<b>2</b>=Above Right|<br>"
                                                               "<b>3</b>=Left|<b>4</b>=Over|<b>5</b>=Right|<br>"
                                                               "<b>6</b>=Below Left|<b>7</b>=Below|<b>8</b>=Below Right]" ) );
  registerDataDefinedButton( mPointPositionOrderDDBtn, QgsPalLayerSettings::PredefinedPositionOrder,
                             QgsDataDefinedButtonV2::String,
                             tr( "Comma separated list of placements in order of priority<br>" )
                             + QStringLiteral( "[<b>TL</b>=Top left|<b>TSL</b>=Top, slightly left|<b>T</b>=Top middle|<br>"
                                               "<b>TSR</b>=Top, slightly right|<b>TR</b>=Top right|<br>"
                                               "<b>L</b>=Left|<b>R</b>=Right|<br>"
                                               "<b>BL</b>=Bottom left|<b>BSL</b>=Bottom, slightly left|<b>B</b>=Bottom middle|<br>"
                                               "<b>BSR</b>=Bottom, slightly right|<b>BR</b>=Bottom right]" ) );
  registerDataDefinedButton( mPointOffsetDDBtn, QgsPalLayerSettings::OffsetXY,
                             QgsDataDefinedButtonV2::AnyType, QgsDataDefinedButtonV2::doubleXYDesc() );
  registerDataDefinedButton( mPointOffsetUnitsDDBtn, QgsPalLayerSettings::OffsetUnits,
                             QgsDataDefinedButtonV2::String, QgsDataDefinedButtonV2::unitsMmMuDesc() );
  registerDataDefinedButton( mLineDistanceDDBtn, QgsPalLayerSettings::LabelDistance,
                             QgsDataDefinedButtonV2::AnyType, QgsDataDefinedButtonV2::doublePosDesc() );
  registerDataDefinedButton( mLineDistanceUnitDDBtn, QgsPalLayerSettings::DistanceUnits,
                             QgsDataDefinedButtonV2::String, QgsDataDefinedButtonV2::unitsMmMuDesc() );
  registerDataDefinedButton( mPriorityDDBtn, QgsPalLayerSettings::Priority,
                             QgsDataDefinedButtonV2::AnyType, tr( "double [0.0-10.0]" ) );

  // TODO: is this necessary? maybe just use the data defined-only rotation?
  //mPointAngleDDBtn, QgsPalLayerSettings::OffsetRotation,
  //                        QgsDataDefinedButtonV2::AnyType, QgsDataDefinedButtonV2::double180RotDesc() );
  registerDataDefinedButton( mMaxCharAngleDDBtn, QgsPalLayerSettings::CurvedCharAngleInOut,
                             QgsDataDefinedButtonV2::AnyType, tr( "double coord [<b>in,out</b> as 20.0-60.0,20.0-95.0]" ) );
  registerDataDefinedButton( mRepeatDistanceDDBtn, QgsPalLayerSettings::RepeatDistance,
                             QgsDataDefinedButtonV2::AnyType, QgsDataDefinedButtonV2::doublePosDesc() );
  registerDataDefinedButton( mRepeatDistanceUnitDDBtn, QgsPalLayerSettings::RepeatDistanceUnit,
                             QgsDataDefinedButtonV2::String, QgsDataDefinedButtonV2::unitsMmMuDesc() );

  // data defined-only
  QString ddPlaceInfo = tr( "In edit mode, layer's relevant labeling map tool is:<br>"
                            "&nbsp;&nbsp;Defined attribute field -&gt; <i>enabled</i><br>"
                            "&nbsp;&nbsp;Defined expression -&gt; <i>disabled</i>" );
  registerDataDefinedButton( mCoordXDDBtn, QgsPalLayerSettings::PositionX,
                             QgsDataDefinedButtonV2::AnyType, QgsDataDefinedButtonV2::doubleDesc() );
  mCoordXDDBtn->setUsageInfo( ddPlaceInfo );
  registerDataDefinedButton( mCoordYDDBtn, QgsPalLayerSettings::PositionY,
                             QgsDataDefinedButtonV2::AnyType, QgsDataDefinedButtonV2::doubleDesc() );
  mCoordYDDBtn->setUsageInfo( ddPlaceInfo );
  registerDataDefinedButton( mCoordAlignmentHDDBtn, QgsPalLayerSettings::Hali,
                             QgsDataDefinedButtonV2::String,
                             trString + "[<b>Left</b>|<b>Center</b>|<b>Right</b>]" );
  mCoordAlignmentHDDBtn->setUsageInfo( ddPlaceInfo );
  registerDataDefinedButton( mCoordAlignmentVDDBtn, QgsPalLayerSettings::Vali,
                             QgsDataDefinedButtonV2::String,
                             trString + QStringLiteral( "[<b>Bottom</b>|<b>Base</b>|<br>"
                                                        "<b>Half</b>|<b>Cap</b>|<b>Top</b>]" ) );
  mCoordAlignmentVDDBtn->setUsageInfo( ddPlaceInfo );
  registerDataDefinedButton( mCoordRotationDDBtn, QgsPalLayerSettings::Rotation,
                             QgsDataDefinedButtonV2::AnyType, QgsDataDefinedButtonV2::double180RotDesc() );
  mCoordRotationDDBtn->setUsageInfo( ddPlaceInfo );

  // rendering
  QString ddScaleVisInfo = tr( "Value &lt; 0 represents a scale closer than 1:1, e.g. -10 = 10:1<br>"
                               "Value of 0 disables the specific limit." );
  registerDataDefinedButton( mScaleBasedVisibilityDDBtn, QgsPalLayerSettings::ScaleVisibility,
                             QgsDataDefinedButtonV2::AnyType, QgsDataDefinedButtonV2::boolDesc() );
  mScaleBasedVisibilityDDBtn->registerCheckedWidget( mScaleBasedVisibilityChkBx );
  registerDataDefinedButton( mScaleBasedVisibilityMinDDBtn, QgsPalLayerSettings::MinScale,
                             QgsDataDefinedButtonV2::AnyType, QgsDataDefinedButtonV2::intDesc() );
  mScaleBasedVisibilityMinDDBtn->setUsageInfo( ddScaleVisInfo );
  registerDataDefinedButton( mScaleBasedVisibilityMaxDDBtn, QgsPalLayerSettings::MaxScale,
                             QgsDataDefinedButtonV2::AnyType, QgsDataDefinedButtonV2::intDesc() );
  mScaleBasedVisibilityMaxDDBtn->setUsageInfo( ddScaleVisInfo );

  registerDataDefinedButton( mFontLimitPixelDDBtn, QgsPalLayerSettings::FontLimitPixel,
                             QgsDataDefinedButtonV2::AnyType, QgsDataDefinedButtonV2::boolDesc() );
  mFontLimitPixelDDBtn->registerCheckedWidget( mFontLimitPixelChkBox );
  registerDataDefinedButton( mFontMinPixelDDBtn, QgsPalLayerSettings::FontMinPixel,
                             QgsDataDefinedButtonV2::AnyType, tr( "int [1-1000]" ) );
  registerDataDefinedButton( mFontMaxPixelDDBtn, QgsPalLayerSettings::FontMaxPixel,
                             QgsDataDefinedButtonV2::AnyType, tr( "int [1-10000]" ) );

  registerDataDefinedButton( mShowLabelDDBtn, QgsPalLayerSettings::Show,
                             QgsDataDefinedButtonV2::AnyType, QgsDataDefinedButtonV2::boolDesc() );

  registerDataDefinedButton( mAlwaysShowDDBtn, QgsPalLayerSettings::AlwaysShow,
                             QgsDataDefinedButtonV2::AnyType, QgsDataDefinedButtonV2::boolDesc() );

  registerDataDefinedButton( mIsObstacleDDBtn, QgsPalLayerSettings::IsObstacle,
                             QgsDataDefinedButtonV2::AnyType, QgsDataDefinedButtonV2::boolDesc() );
  registerDataDefinedButton( mObstacleFactorDDBtn, QgsPalLayerSettings::ObstacleFactor,
                             QgsDataDefinedButtonV2::AnyType, tr( "double [0.0-10.0]" ) );
  registerDataDefinedButton( mZIndexDDBtn, QgsPalLayerSettings::ZIndex,
                             QgsDataDefinedButtonV2::AnyType, QgsDataDefinedButtonV2::doubleDesc() );
}

void QgsLabelingGui::syncDefinedCheckboxFrame( QgsDataDefinedButtonV2* ddBtn, QCheckBox* chkBx, QFrame* f )
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




