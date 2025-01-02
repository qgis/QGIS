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
#include "moc_qgslabelinggui.cpp"
#include "qgsvectorlayer.h"
#include "qgsmapcanvas.h"
#include "qgsproject.h"
#include "qgsexpressioncontextutils.h"
#include "qgsexpressionbuilderdialog.h"
#include "qgshelp.h"
#include "qgsstylesavedialog.h"
#include "qgscallout.h"
#include "qgsapplication.h"
#include "qgscalloutsregistry.h"
#include "callouts/qgscalloutwidget.h"
#include "qgslabelobstaclesettingswidget.h"
#include "qgslabellineanchorwidget.h"
#include "qgsprojectstylesettings.h"
#include "qgsgui.h"
#include "qgsmeshlayer.h"
#include "qgsvectortilelayer.h"

#include <QButtonGroup>
#include <QMessageBox>

///@cond PRIVATE

QgsExpressionContext QgsLabelingGui::createExpressionContext() const
{
  QgsExpressionContext expContext;
  if ( mMapCanvas )
  {
    expContext = mMapCanvas->createExpressionContext();
  }
  else
  {
    expContext << QgsExpressionContextUtils::globalScope()
               << QgsExpressionContextUtils::projectScope( QgsProject::instance() )
               << QgsExpressionContextUtils::atlasScope( nullptr );
  }

  if ( mLayer )
    expContext << QgsExpressionContextUtils::layerScope( mLayer );

  if ( mLayer && mLayer->type() == Qgis::LayerType::Mesh )
  {
    if ( mGeomType == Qgis::GeometryType::Point )
      expContext << QgsExpressionContextUtils::meshExpressionScope( QgsMesh::ElementType::Vertex );
    else if ( mGeomType == Qgis::GeometryType::Polygon )
      expContext << QgsExpressionContextUtils::meshExpressionScope( QgsMesh::ElementType::Face );
  }

  expContext << QgsExpressionContextUtils::updateSymbolScope( nullptr, new QgsExpressionContextScope() );

  //TODO - show actual value
  expContext.setOriginalValueVariable( QVariant() );
  expContext.setHighlightedVariables( QStringList() << QgsExpressionContext::EXPR_ORIGINAL_VALUE << QgsExpressionContext::EXPR_SYMBOL_COLOR );

  return expContext;
}

void QgsLabelingGui::updateCalloutWidget( QgsCallout *callout )
{
  if ( !callout )
  {
    mCalloutStackedWidget->setCurrentWidget( pageDummy );
    return;
  }

  QgsVectorLayer *vLayer = qobject_cast<QgsVectorLayer *>( mLayer );
  if ( !vLayer )
  {
    mCalloutStackedWidget->setCurrentWidget( pageDummy );
    return;
  }

  if ( mCalloutStackedWidget->currentWidget() != pageDummy )
  {
    // stop updating from the original widget
    if ( QgsCalloutWidget *pew = qobject_cast<QgsCalloutWidget *>( mCalloutStackedWidget->currentWidget() ) )
      disconnect( pew, &QgsCalloutWidget::changed, this, &QgsLabelingGui::updatePreview );
  }

  QgsCalloutRegistry *registry = QgsApplication::calloutRegistry();
  if ( QgsCalloutAbstractMetadata *am = registry->calloutMetadata( callout->type() ) )
  {
    if ( QgsCalloutWidget *w = am->createCalloutWidget( vLayer ) )
    {
      Qgis::GeometryType geometryType = mGeomType;
      if ( mGeometryGeneratorGroupBox->isChecked() )
        geometryType = mGeometryGeneratorType->currentData().value<Qgis::GeometryType>();
      else if ( vLayer )
        geometryType = vLayer->geometryType();
      w->setGeometryType( geometryType );
      w->setCallout( callout );

      w->setContext( context() );
      mCalloutStackedWidget->addWidget( w );
      mCalloutStackedWidget->setCurrentWidget( w );
      // start receiving updates from widget
      connect( w, &QgsCalloutWidget::changed, this, &QgsLabelingGui::updatePreview );
      return;
    }
  }
  // When anything is not right
  mCalloutStackedWidget->setCurrentWidget( pageDummy );
}

void QgsLabelingGui::showObstacleSettings()
{
  QgsVectorLayer *vLayer = qobject_cast<QgsVectorLayer *>( mLayer );
  if ( !vLayer )
  {
    return;
  }

  QgsExpressionContext context = createExpressionContext();

  QgsSymbolWidgetContext symbolContext;
  symbolContext.setExpressionContext( &context );
  symbolContext.setMapCanvas( mMapCanvas );

  QgsLabelObstacleSettingsWidget *widget = new QgsLabelObstacleSettingsWidget( nullptr, vLayer );
  widget->setDataDefinedProperties( mDataDefinedProperties );
  widget->setSettings( mObstacleSettings );
  widget->setGeometryType( vLayer ? vLayer->geometryType() : Qgis::GeometryType::Unknown );
  widget->setContext( symbolContext );

  auto applySettings = [=] {
    mObstacleSettings = widget->settings();
    const QgsPropertyCollection obstacleDataDefinedProperties = widget->dataDefinedProperties();
    widget->updateDataDefinedProperties( mDataDefinedProperties );
    emit widgetChanged();
  };

  QgsPanelWidget *panel = QgsPanelWidget::findParentPanel( this );
  if ( panel && panel->dockMode() )
  {
    connect( widget, &QgsLabelSettingsWidgetBase::changed, this, [=] {
      applySettings();
    } );
    panel->openPanel( widget );
  }
  else
  {
    QgsLabelSettingsWidgetDialog dialog( widget, this );

    dialog.buttonBox()->addButton( QDialogButtonBox::Help );
    connect( dialog.buttonBox(), &QDialogButtonBox::helpRequested, this, [=] {
      QgsHelp::openHelp( QStringLiteral( "style_library/label_settings.html#obstacles" ) );
    } );

    if ( dialog.exec() )
    {
      applySettings();
    }
    // reactivate button's window
    activateWindow();
  }
}

void QgsLabelingGui::showLineAnchorSettings()
{
  QgsVectorLayer *vLayer = qobject_cast<QgsVectorLayer *>( mLayer );
  if ( !vLayer )
  {
    return;
  }

  QgsExpressionContext context = createExpressionContext();

  QgsSymbolWidgetContext symbolContext;
  symbolContext.setExpressionContext( &context );
  symbolContext.setMapCanvas( mMapCanvas );

  QgsLabelLineAnchorWidget *widget = new QgsLabelLineAnchorWidget( nullptr, vLayer );
  widget->setDataDefinedProperties( mDataDefinedProperties );
  widget->setSettings( mLineSettings );
  widget->setGeometryType( vLayer ? vLayer->geometryType() : Qgis::GeometryType::Unknown );
  widget->setContext( symbolContext );

  auto applySettings = [=] {
    const QgsLabelLineSettings widgetSettings = widget->settings();
    mLineSettings.setLineAnchorPercent( widgetSettings.lineAnchorPercent() );
    mLineSettings.setAnchorType( widgetSettings.anchorType() );
    mLineSettings.setAnchorClipping( widgetSettings.anchorClipping() );
    mLineSettings.setAnchorTextPoint( widgetSettings.anchorTextPoint() );
    const QgsPropertyCollection obstacleDataDefinedProperties = widget->dataDefinedProperties();
    widget->updateDataDefinedProperties( mDataDefinedProperties );
    emit widgetChanged();
  };

  QgsPanelWidget *panel = QgsPanelWidget::findParentPanel( this );
  if ( panel && panel->dockMode() )
  {
    connect( widget, &QgsLabelSettingsWidgetBase::changed, this, [=] {
      applySettings();
    } );
    panel->openPanel( widget );
  }
  else
  {
    QgsLabelSettingsWidgetDialog dialog( widget, this );

    dialog.buttonBox()->addButton( QDialogButtonBox::Help );
    connect( dialog.buttonBox(), &QDialogButtonBox::helpRequested, this, [=] {
      QgsHelp::openHelp( QStringLiteral( "style_library/label_settings.html#placement-for-line-layers" ) );
    } );

    if ( dialog.exec() )
    {
      applySettings();
    }
    // reactivate button's window
    activateWindow();
  }
}

QgsLabelingGui::QgsLabelingGui( QgsVectorLayer *layer, QgsMapCanvas *mapCanvas, const QgsPalLayerSettings &layerSettings, QWidget *parent, Qgis::GeometryType geomType )
  : QgsTextFormatWidget( mapCanvas, parent, QgsTextFormatWidget::Labeling, layer )
  , mMode( NoLabels )
  , mSettings( layerSettings )
{
  mGeomType = geomType;

  init();

  setLayer( layer );
}

QgsLabelingGui::QgsLabelingGui( QgsMeshLayer *layer, QgsMapCanvas *mapCanvas, const QgsPalLayerSettings &settings, QWidget *parent, Qgis::GeometryType geomType )
  : QgsTextFormatWidget( mapCanvas, parent, QgsTextFormatWidget::Labeling, layer )
  , mMode( NoLabels )
  , mSettings( settings )
{
  mGeomType = geomType;

  init();

  setLayer( layer );
}

QgsLabelingGui::QgsLabelingGui( QgsVectorTileLayer *layer, QgsMapCanvas *mapCanvas, const QgsPalLayerSettings &settings, QWidget *parent, Qgis::GeometryType geomType )
  : QgsTextFormatWidget( mapCanvas, parent, QgsTextFormatWidget::Labeling, layer )
  , mMode( NoLabels )
  , mSettings( settings )
{
  mGeomType = geomType;

  init();

  setLayer( layer );
}


QgsLabelingGui::QgsLabelingGui( QgsMapCanvas *mapCanvas, QWidget *parent, QgsMapLayer *layer )
  : QgsTextFormatWidget( mapCanvas, parent, QgsTextFormatWidget::Labeling, layer )
  , mMode( NoLabels )
{
}


QgsLabelingGui::QgsLabelingGui( QgsMapCanvas *mapCanvas, const QgsPalLayerSettings &settings, QWidget *parent )
  : QgsTextFormatWidget( mapCanvas, parent, QgsTextFormatWidget::Labeling, nullptr )
  , mMode( NoLabels )
  , mSettings( settings )
{
  init();

  setLayer( nullptr );
}

void QgsLabelingGui::init()
{
  QgsGui::initCalloutWidgets();

  mStackedWidgetLabelWith->setSizeMode( QgsStackedWidget::SizeMode::CurrentPageOnly );

  mFontMultiLineAlignComboBox->addItem( tr( "Left" ), static_cast<int>( Qgis::LabelMultiLineAlignment::Left ) );
  mFontMultiLineAlignComboBox->addItem( tr( "Center" ), static_cast<int>( Qgis::LabelMultiLineAlignment::Center ) );
  mFontMultiLineAlignComboBox->addItem( tr( "Right" ), static_cast<int>( Qgis::LabelMultiLineAlignment::Right ) );
  mFontMultiLineAlignComboBox->addItem( tr( "Justify" ), static_cast<int>( Qgis::LabelMultiLineAlignment::Justify ) );

  mCoordRotationUnitComboBox->addItem( QgsUnitTypes::toString( Qgis::AngleUnit::Degrees ), static_cast<int>( Qgis::AngleUnit::Degrees ) );
  mCoordRotationUnitComboBox->addItem( QgsUnitTypes::toString( Qgis::AngleUnit::Radians ), static_cast<int>( Qgis::AngleUnit::Radians ) );
  mCoordRotationUnitComboBox->addItem( QgsUnitTypes::toString( Qgis::AngleUnit::Gon ), static_cast<int>( Qgis::AngleUnit::Gon ) );
  mCoordRotationUnitComboBox->addItem( QgsUnitTypes::toString( Qgis::AngleUnit::MinutesOfArc ), static_cast<int>( Qgis::AngleUnit::MinutesOfArc ) );
  mCoordRotationUnitComboBox->addItem( QgsUnitTypes::toString( Qgis::AngleUnit::SecondsOfArc ), static_cast<int>( Qgis::AngleUnit::SecondsOfArc ) );
  mCoordRotationUnitComboBox->addItem( QgsUnitTypes::toString( Qgis::AngleUnit::Turn ), static_cast<int>( Qgis::AngleUnit::Turn ) );
  mCoordRotationUnitComboBox->addItem( QgsUnitTypes::toString( Qgis::AngleUnit::MilliradiansSI ), static_cast<int>( Qgis::AngleUnit::MilliradiansSI ) );
  mCoordRotationUnitComboBox->addItem( QgsUnitTypes::toString( Qgis::AngleUnit::MilNATO ), static_cast<int>( Qgis::AngleUnit::MilNATO ) );

  // connections for groupboxes with separate activation checkboxes (that need to honor data defined setting)
  connect( mBufferDrawChkBx, &QAbstractButton::toggled, this, &QgsLabelingGui::updateUi );
  connect( mBufferDrawDDBtn, &QgsPropertyOverrideButton::changed, this, &QgsLabelingGui::updateUi );
  connect( mEnableMaskChkBx, &QAbstractButton::toggled, this, &QgsLabelingGui::updateUi );
  connect( mShapeDrawChkBx, &QAbstractButton::toggled, this, &QgsLabelingGui::updateUi );
  connect( mCalloutsDrawCheckBox, &QAbstractButton::toggled, this, &QgsLabelingGui::updateUi );
  connect( mShadowDrawChkBx, &QAbstractButton::toggled, this, &QgsLabelingGui::updateUi );
  connect( mDirectSymbChkBx, &QAbstractButton::toggled, this, &QgsLabelingGui::updateUi );
  connect( mFormatNumChkBx, &QAbstractButton::toggled, this, &QgsLabelingGui::updateUi );
  connect( mScaleBasedVisibilityChkBx, &QAbstractButton::toggled, this, &QgsLabelingGui::updateUi );
  connect( mFontLimitPixelChkBox, &QAbstractButton::toggled, this, &QgsLabelingGui::updateUi );
  connect( mGeometryGeneratorGroupBox, &QGroupBox::toggled, this, &QgsLabelingGui::updateGeometryTypeBasedWidgets );
  connect( mGeometryGeneratorType, qOverload<int>( &QComboBox::currentIndexChanged ), this, &QgsLabelingGui::updateGeometryTypeBasedWidgets );
  connect( mGeometryGeneratorExpressionButton, &QToolButton::clicked, this, &QgsLabelingGui::showGeometryGeneratorExpressionBuilder );
  connect( mGeometryGeneratorGroupBox, &QGroupBox::toggled, this, &QgsLabelingGui::validateGeometryGeneratorExpression );
  connect( mGeometryGenerator, &QgsCodeEditorExpression::textChanged, this, &QgsLabelingGui::validateGeometryGeneratorExpression );
  connect( mGeometryGeneratorType, qOverload<int>( &QComboBox::currentIndexChanged ), this, &QgsLabelingGui::validateGeometryGeneratorExpression );
  connect( mObstacleSettingsButton, &QAbstractButton::clicked, this, &QgsLabelingGui::showObstacleSettings );
  connect( mLineAnchorSettingsButton, &QAbstractButton::clicked, this, &QgsLabelingGui::showLineAnchorSettings );

  mFieldExpressionWidget->registerExpressionContextGenerator( this );

  mMinScaleWidget->setMapCanvas( mMapCanvas );
  mMinScaleWidget->setShowCurrentScaleButton( true );
  mMaxScaleWidget->setMapCanvas( mMapCanvas );
  mMaxScaleWidget->setShowCurrentScaleButton( true );

  mGeometryGeneratorExpressionButton->setSizePolicy( QSizePolicy::Minimum, QSizePolicy::Minimum );
  mGeometryGeneratorExpressionButton->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "/mIconExpression.svg" ) ) );

  const QStringList calloutTypes = QgsApplication::calloutRegistry()->calloutTypes();
  for ( const QString &type : calloutTypes )
  {
    mCalloutStyleComboBox->addItem( QgsApplication::calloutRegistry()->calloutMetadata( type )->icon(), QgsApplication::calloutRegistry()->calloutMetadata( type )->visibleName(), type );
  }

  mGeometryGeneratorWarningLabel->setStyleSheet( QStringLiteral( "color: #FFC107;" ) );
  mGeometryGeneratorWarningLabel->setTextInteractionFlags( Qt::TextBrowserInteraction );
  connect( mGeometryGeneratorWarningLabel, &QLabel::linkActivated, this, [this]( const QString &link ) {
    if ( link == QLatin1String( "#determineGeometryGeneratorType" ) )
      determineGeometryGeneratorType();
  } );

  connect( mCalloutStyleComboBox, static_cast<void ( QComboBox::* )( int )>( &QComboBox::currentIndexChanged ), this, &QgsLabelingGui::calloutTypeChanged );

  mLblNoObstacle1->installEventFilter( this );
}

void QgsLabelingGui::setLayer( QgsMapLayer *mapLayer )
{
  mPreviewFeature = QgsFeature();

  if ( ( !mapLayer || mapLayer->type() != Qgis::LayerType::Vector ) && mGeomType == Qgis::GeometryType::Unknown )
  {
    setEnabled( false );
    return;
  }

  setEnabled( true );

  mLayer = mapLayer;
  QgsVectorLayer *vLayer = qobject_cast<QgsVectorLayer *>( mapLayer );

  mTextFormatsListWidget->setLayerType( vLayer ? vLayer->geometryType() : mGeomType );
  mBackgroundMarkerSymbolButton->setLayer( vLayer );
  mBackgroundFillSymbolButton->setLayer( vLayer );

  // load labeling settings from layer
  updateGeometryTypeBasedWidgets();

  mFieldExpressionWidget->setLayer( mapLayer );
  QgsDistanceArea da;
  if ( mLayer )
    da.setSourceCrs( mLayer->crs(), QgsProject::instance()->transformContext() );
  da.setEllipsoid( QgsProject::instance()->ellipsoid() );
  mFieldExpressionWidget->setGeomCalculator( da );

  mFieldExpressionWidget->setEnabled( mMode == Labels || !mLayer );
  mLabelingFrame->setEnabled( mMode == Labels || !mLayer );

  blockInitSignals( true );

  mGeometryGenerator->setText( mSettings.geometryGenerator );
  mGeometryGeneratorGroupBox->setChecked( mSettings.geometryGeneratorEnabled );
  if ( !mSettings.geometryGeneratorEnabled )
    mGeometryGeneratorGroupBox->setCollapsed( true );
  mGeometryGeneratorType->setCurrentIndex( mGeometryGeneratorType->findData( QVariant::fromValue( mSettings.geometryGeneratorType ) ) );

  updateWidgetForFormat( mSettings.format().isValid() ? mSettings.format() : QgsStyle::defaultTextFormatForProject( QgsProject::instance(), QgsStyle::TextFormatContext::Labeling ) );

  mFieldExpressionWidget->setRow( -1 );
  mFieldExpressionWidget->setField( mSettings.fieldName );
  mCheckBoxSubstituteText->setChecked( mSettings.useSubstitutions );
  mSubstitutions = mSettings.substitutions;

  // populate placement options
  mCentroidRadioWhole->setChecked( mSettings.centroidWhole );
  mCentroidInsideCheckBox->setChecked( mSettings.centroidInside );
  mFitInsidePolygonCheckBox->setChecked( mSettings.fitInPolygonOnly );
  mLineDistanceSpnBx->setValue( mSettings.dist );
  mLineDistanceUnitWidget->setUnit( mSettings.distUnits );
  mLineDistanceUnitWidget->setMapUnitScale( mSettings.distMapUnitScale );

  mMaximumDistanceSpnBx->setValue( mSettings.pointSettings().maximumDistance() );
  mMaximumDistanceUnitWidget->setUnit( mSettings.pointSettings().maximumDistanceUnit() );
  mMaximumDistanceUnitWidget->setMapUnitScale( mSettings.pointSettings().maximumDistanceMapUnitScale() );

  mOffsetTypeComboBox->setCurrentIndex( mOffsetTypeComboBox->findData( static_cast<int>( mSettings.offsetType ) ) );
  mQuadrantBtnGrp->button( static_cast<int>( mSettings.pointSettings().quadrant() ) )->setChecked( true );
  mPointOffsetXSpinBox->setValue( mSettings.xOffset );
  mPointOffsetYSpinBox->setValue( mSettings.yOffset );
  mPointOffsetUnitWidget->setUnit( mSettings.offsetUnits );
  mPointOffsetUnitWidget->setMapUnitScale( mSettings.labelOffsetMapUnitScale );
  mPointAngleSpinBox->setValue( mSettings.angleOffset );
  chkLineAbove->setChecked( mSettings.lineSettings().placementFlags() & Qgis::LabelLinePlacementFlag::AboveLine );
  chkLineBelow->setChecked( mSettings.lineSettings().placementFlags() & Qgis::LabelLinePlacementFlag::BelowLine );
  chkLineOn->setChecked( mSettings.lineSettings().placementFlags() & Qgis::LabelLinePlacementFlag::OnLine );
  chkLineOrientationDependent->setChecked( !( mSettings.lineSettings().placementFlags() & Qgis::LabelLinePlacementFlag::MapOrientation ) );

  mCheckAllowLabelsOutsidePolygons->setChecked( mSettings.polygonPlacementFlags() & Qgis::LabelPolygonPlacementFlag::AllowPlacementOutsideOfPolygon );

  const int placementIndex = mPlacementModeComboBox->findData( static_cast<int>( mSettings.placement ) );
  if ( placementIndex >= 0 )
  {
    mPlacementModeComboBox->setCurrentIndex( placementIndex );
  }
  else
  {
    // use default placement for layer type
    mPlacementModeComboBox->setCurrentIndex( 0 );
  }

  // Label repeat distance
  mRepeatDistanceSpinBox->setValue( mSettings.repeatDistance );
  mRepeatDistanceUnitWidget->setUnit( mSettings.repeatDistanceUnit );
  mRepeatDistanceUnitWidget->setMapUnitScale( mSettings.repeatDistanceMapUnitScale );

  mOverrunDistanceSpinBox->setValue( mSettings.lineSettings().overrunDistance() );
  mOverrunDistanceUnitWidget->setUnit( mSettings.lineSettings().overrunDistanceUnit() );
  mOverrunDistanceUnitWidget->setMapUnitScale( mSettings.lineSettings().overrunDistanceMapUnitScale() );

  mPrioritySlider->setValue( mSettings.priority );
  mChkNoObstacle->setChecked( mSettings.obstacleSettings().isObstacle() );

  mObstacleSettings = mSettings.obstacleSettings();
  mLineSettings = mSettings.lineSettings();

  chkLabelPerFeaturePart->setChecked( mSettings.labelPerPart );

  mComboOverlapHandling->setCurrentIndex( mComboOverlapHandling->findData( static_cast<int>( mSettings.placementSettings().overlapHandling() ) ) );
  mCheckAllowDegradedPlacement->setChecked( mSettings.placementSettings().allowDegradedPlacement() );
  mPrioritizationComboBox->setCurrentIndex( mPrioritizationComboBox->findData( QVariant::fromValue( mSettings.placementSettings().prioritization() ) ) );

  chkMergeLines->setChecked( mSettings.lineSettings().mergeLines() );
  mMinSizeSpinBox->setValue( mSettings.thinningSettings().minimumFeatureSize() );
  mLimitLabelChkBox->setChecked( mSettings.thinningSettings().limitNumberOfLabelsEnabled() );
  mLimitLabelSpinBox->setValue( mSettings.thinningSettings().maximumNumberLabels() );

  // direction symbol(s)
  mDirectSymbChkBx->setChecked( mSettings.lineSettings().addDirectionSymbol() );
  mDirectSymbLeftLineEdit->setText( mSettings.lineSettings().leftDirectionSymbol() );
  mDirectSymbRightLineEdit->setText( mSettings.lineSettings().rightDirectionSymbol() );
  mDirectSymbRevChkBx->setChecked( mSettings.lineSettings().reverseDirectionSymbol() );

  mDirectSymbBtnGrp->button( static_cast<int>( mSettings.lineSettings().directionSymbolPlacement() ) )->setChecked( true );
  mUpsidedownBtnGrp->button( static_cast<int>( mSettings.upsidedownLabels ) )->setChecked( true );

  // curved label max character angles
  mMaxCharAngleInDSpinBox->setValue( mSettings.maxCurvedCharAngleIn );
  // lyr.maxCurvedCharAngleOut must be negative, but it is shown as positive spinbox in GUI
  mMaxCharAngleOutDSpinBox->setValue( std::fabs( mSettings.maxCurvedCharAngleOut ) );

  wrapCharacterEdit->setText( mSettings.wrapChar );
  mAutoWrapLengthSpinBox->setValue( mSettings.autoWrapLength );
  mAutoWrapTypeComboBox->setCurrentIndex( mSettings.useMaxLineLengthForAutoWrap ? 0 : 1 );

  if ( mFontMultiLineAlignComboBox->findData( static_cast<int>( mSettings.multilineAlign ) ) != -1 )
  {
    mFontMultiLineAlignComboBox->setCurrentIndex( mFontMultiLineAlignComboBox->findData( static_cast<int>( mSettings.multilineAlign ) ) );
  }
  else
  {
    // the default pal layer settings for multiline alignment is to follow label placement, which isn't always available
    // revert to left alignment in such case
    mFontMultiLineAlignComboBox->setCurrentIndex( 0 );
  }

  chkPreserveRotation->setChecked( mSettings.preserveRotation );

  mCoordRotationUnitComboBox->setCurrentIndex( 0 );
  if ( mCoordRotationUnitComboBox->findData( static_cast<unsigned int>( mSettings.rotationUnit() ) ) >= 0 )
    mCoordRotationUnitComboBox->setCurrentIndex( mCoordRotationUnitComboBox->findData( static_cast<unsigned int>( mSettings.rotationUnit() ) ) );

  mScaleBasedVisibilityChkBx->setChecked( mSettings.scaleVisibility );
  mMinScaleWidget->setScale( mSettings.minimumScale );
  mMaxScaleWidget->setScale( mSettings.maximumScale );

  mFormatNumChkBx->setChecked( mSettings.formatNumbers );
  mFormatNumDecimalsSpnBx->setValue( mSettings.decimals );
  mFormatNumPlusSignChkBx->setChecked( mSettings.plusSign );

  // set pixel size limiting checked state before unit choice so limiting can be
  // turned on as a default for map units, if minimum trigger value of 0 is used
  mFontLimitPixelChkBox->setChecked( mSettings.fontLimitPixelSize );
  mMinPixelLimit = mSettings.fontMinPixelSize; // ignored after first settings save
  mFontMinPixelSpinBox->setValue( mSettings.fontMinPixelSize == 0 ? 3 : mSettings.fontMinPixelSize );
  mFontMaxPixelSpinBox->setValue( mSettings.fontMaxPixelSize );

  mZIndexSpinBox->setValue( mSettings.zIndex );

  mDataDefinedProperties = mSettings.dataDefinedProperties();

  // callout settings, to move to custom widget when multiple styles exist
  if ( auto *lCallout = mSettings.callout() )
  {
    whileBlocking( mCalloutsDrawCheckBox )->setChecked( lCallout->enabled() );
    whileBlocking( mCalloutStyleComboBox )->setCurrentIndex( mCalloutStyleComboBox->findData( lCallout->type() ) );
    updateCalloutWidget( lCallout );
  }
  else
  {
    std::unique_ptr<QgsCallout> defaultCallout( QgsCalloutRegistry::defaultCallout() );
    whileBlocking( mCalloutStyleComboBox )->setCurrentIndex( mCalloutStyleComboBox->findData( defaultCallout->type() ) );
    whileBlocking( mCalloutsDrawCheckBox )->setChecked( false );
    updateCalloutWidget( defaultCallout.get() );
  }

  updatePlacementWidgets();
  updateLinePlacementOptions();

  // needs to come before data defined setup, so connections work
  blockInitSignals( false );

  // set up data defined toolbuttons
  // do this after other widgets are configured, so they can be enabled/disabled
  populateDataDefinedButtons();

  updateUi(); // should come after data defined button setup
}

void QgsLabelingGui::setSettings( const QgsPalLayerSettings &settings )
{
  mSettings = settings;
  setLayer( mLayer );
}

void QgsLabelingGui::blockInitSignals( bool block )
{
  chkLineAbove->blockSignals( block );
  chkLineBelow->blockSignals( block );
  mPlacementModeComboBox->blockSignals( block );
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

  // restore properties which aren't exposed in GUI
  lyr.setUnplacedVisibility( mSettings.unplacedVisibility() );

  lyr.drawLabels = ( mMode == Labels ) || !mLayer;

  bool isExpression;
  lyr.fieldName = mFieldExpressionWidget->currentField( &isExpression );
  lyr.isExpression = isExpression;

  lyr.dist = 0;

  Qgis::LabelPolygonPlacementFlags polygonPlacementFlags = Qgis::LabelPolygonPlacementFlag::AllowPlacementInsideOfPolygon;
  if ( mCheckAllowLabelsOutsidePolygons->isChecked() )
    polygonPlacementFlags |= Qgis::LabelPolygonPlacementFlag::AllowPlacementOutsideOfPolygon;
  lyr.setPolygonPlacementFlags( polygonPlacementFlags );

  lyr.centroidWhole = mCentroidRadioWhole->isChecked();
  lyr.centroidInside = mCentroidInsideCheckBox->isChecked();
  lyr.fitInPolygonOnly = mFitInsidePolygonCheckBox->isChecked();
  lyr.dist = mLineDistanceSpnBx->value();
  lyr.distUnits = mLineDistanceUnitWidget->unit();
  lyr.distMapUnitScale = mLineDistanceUnitWidget->getMapUnitScale();

  lyr.pointSettings().setMaximumDistance( mMaximumDistanceSpnBx->value() );
  lyr.pointSettings().setMaximumDistanceUnit( mMaximumDistanceUnitWidget->unit() );
  lyr.pointSettings().setMaximumDistanceMapUnitScale( mMaximumDistanceUnitWidget->getMapUnitScale() );

  lyr.offsetType = static_cast<Qgis::LabelOffsetType>( mOffsetTypeComboBox->currentData().toInt() );
  if ( mQuadrantBtnGrp )
  {
    lyr.pointSettings().setQuadrant( static_cast<Qgis::LabelQuadrantPosition>( mQuadrantBtnGrp->checkedId() ) );
  }
  lyr.xOffset = mPointOffsetXSpinBox->value();
  lyr.yOffset = mPointOffsetYSpinBox->value();
  lyr.offsetUnits = mPointOffsetUnitWidget->unit();
  lyr.labelOffsetMapUnitScale = mPointOffsetUnitWidget->getMapUnitScale();
  lyr.angleOffset = mPointAngleSpinBox->value();

  Qgis::LabelLinePlacementFlags linePlacementFlags = Qgis::LabelLinePlacementFlags();
  if ( chkLineAbove->isChecked() )
    linePlacementFlags |= Qgis::LabelLinePlacementFlag::AboveLine;
  if ( chkLineBelow->isChecked() )
    linePlacementFlags |= Qgis::LabelLinePlacementFlag::BelowLine;
  if ( chkLineOn->isChecked() )
    linePlacementFlags |= Qgis::LabelLinePlacementFlag::OnLine;
  if ( !chkLineOrientationDependent->isChecked() )
    linePlacementFlags |= Qgis::LabelLinePlacementFlag::MapOrientation;
  lyr.lineSettings().setPlacementFlags( linePlacementFlags );

  lyr.placement = static_cast<Qgis::LabelPlacement>( mPlacementModeComboBox->currentData().toInt() );

  lyr.repeatDistance = mRepeatDistanceSpinBox->value();
  lyr.repeatDistanceUnit = mRepeatDistanceUnitWidget->unit();
  lyr.repeatDistanceMapUnitScale = mRepeatDistanceUnitWidget->getMapUnitScale();

  lyr.lineSettings().setOverrunDistance( mOverrunDistanceSpinBox->value() );
  lyr.lineSettings().setOverrunDistanceUnit( mOverrunDistanceUnitWidget->unit() );
  lyr.lineSettings().setOverrunDistanceMapUnitScale( mOverrunDistanceUnitWidget->getMapUnitScale() );

  lyr.priority = mPrioritySlider->value();

  mObstacleSettings.setIsObstacle( mChkNoObstacle->isChecked() || mMode == ObstaclesOnly );
  lyr.setObstacleSettings( mObstacleSettings );

  lyr.lineSettings().setLineAnchorPercent( mLineSettings.lineAnchorPercent() );
  lyr.lineSettings().setAnchorType( mLineSettings.anchorType() );
  lyr.lineSettings().setAnchorClipping( mLineSettings.anchorClipping() );
  lyr.lineSettings().setAnchorTextPoint( mLineSettings.anchorTextPoint() );

  lyr.labelPerPart = chkLabelPerFeaturePart->isChecked();
  lyr.placementSettings().setOverlapHandling( static_cast<Qgis::LabelOverlapHandling>( mComboOverlapHandling->currentData().toInt() ) );
  lyr.placementSettings().setAllowDegradedPlacement( mCheckAllowDegradedPlacement->isChecked() );
  lyr.placementSettings().setPrioritization( mPrioritizationComboBox->currentData().value<Qgis::LabelPrioritization>() );

  lyr.lineSettings().setMergeLines( chkMergeLines->isChecked() );

  lyr.scaleVisibility = mScaleBasedVisibilityChkBx->isChecked();
  lyr.minimumScale = mMinScaleWidget->scale();
  lyr.maximumScale = mMaxScaleWidget->scale();
  lyr.useSubstitutions = mCheckBoxSubstituteText->isChecked();
  lyr.substitutions = mSubstitutions;

  lyr.setFormat( format( false ) );

  // format numbers
  lyr.formatNumbers = mFormatNumChkBx->isChecked();
  lyr.decimals = mFormatNumDecimalsSpnBx->value();
  lyr.plusSign = mFormatNumPlusSignChkBx->isChecked();

  // direction symbol(s)
  lyr.lineSettings().setAddDirectionSymbol( mDirectSymbChkBx->isChecked() );
  lyr.lineSettings().setLeftDirectionSymbol( mDirectSymbLeftLineEdit->text() );
  lyr.lineSettings().setRightDirectionSymbol( mDirectSymbRightLineEdit->text() );
  lyr.lineSettings().setReverseDirectionSymbol( mDirectSymbRevChkBx->isChecked() );
  if ( mDirectSymbBtnGrp )
  {
    lyr.lineSettings().setDirectionSymbolPlacement( static_cast<QgsLabelLineSettings::DirectionSymbolPlacement>( mDirectSymbBtnGrp->checkedId() ) );
  }
  if ( mUpsidedownBtnGrp )
  {
    lyr.upsidedownLabels = static_cast<Qgis::UpsideDownLabelHandling>( mUpsidedownBtnGrp->checkedId() );
  }

  lyr.maxCurvedCharAngleIn = mMaxCharAngleInDSpinBox->value();
  // lyr.maxCurvedCharAngleOut must be negative, but it is shown as positive spinbox in GUI
  lyr.maxCurvedCharAngleOut = -mMaxCharAngleOutDSpinBox->value();


  lyr.thinningSettings().setMinimumFeatureSize( mMinSizeSpinBox->value() );
  lyr.thinningSettings().setLimitNumberLabelsEnabled( mLimitLabelChkBox->isChecked() );
  lyr.thinningSettings().setMaximumNumberLabels( mLimitLabelSpinBox->value() );
  lyr.fontLimitPixelSize = mFontLimitPixelChkBox->isChecked();
  lyr.fontMinPixelSize = mFontMinPixelSpinBox->value();
  lyr.fontMaxPixelSize = mFontMaxPixelSpinBox->value();
  lyr.wrapChar = wrapCharacterEdit->text();
  lyr.autoWrapLength = mAutoWrapLengthSpinBox->value();
  lyr.useMaxLineLengthForAutoWrap = mAutoWrapTypeComboBox->currentIndex() == 0;
  lyr.multilineAlign = static_cast<Qgis::LabelMultiLineAlignment>( mFontMultiLineAlignComboBox->currentData().toInt() );
  lyr.preserveRotation = chkPreserveRotation->isChecked();
  lyr.setRotationUnit( static_cast<Qgis::AngleUnit>( mCoordRotationUnitComboBox->currentData().toInt() ) );
  lyr.geometryGenerator = mGeometryGenerator->text();
  lyr.geometryGeneratorType = mGeometryGeneratorType->currentData().value<Qgis::GeometryType>();
  lyr.geometryGeneratorEnabled = mGeometryGeneratorGroupBox->isChecked();

  QgsVectorLayer *vLayer = qobject_cast<QgsVectorLayer *>( mLayer );
  lyr.layerType = vLayer ? vLayer->geometryType() : mGeomType;

  lyr.zIndex = mZIndexSpinBox->value();

  lyr.setDataDefinedProperties( mDataDefinedProperties );

  // callout settings
  const QString calloutType = mCalloutStyleComboBox->currentData().toString();
  std::unique_ptr<QgsCallout> callout;
  if ( QgsCalloutWidget *pew = qobject_cast<QgsCalloutWidget *>( mCalloutStackedWidget->currentWidget() ) )
  {
    callout.reset( pew->callout()->clone() );
  }
  if ( !callout )
    callout.reset( QgsApplication::calloutRegistry()->createCallout( calloutType ) );

  callout->setEnabled( mCalloutsDrawCheckBox->isChecked() );
  lyr.setCallout( callout.release() );

  return lyr;
}

void QgsLabelingGui::syncDefinedCheckboxFrame( QgsPropertyOverrideButton *ddBtn, QCheckBox *chkBx, QFrame *f )
{
  f->setEnabled( chkBx->isChecked() || ddBtn->isActive() );
}

bool QgsLabelingGui::eventFilter( QObject *object, QEvent *event )
{
  if ( object == mLblNoObstacle1 )
  {
    if ( event->type() == QEvent::MouseButtonPress && qgis::down_cast<QMouseEvent *>( event )->button() == Qt::LeftButton )
    {
      // clicking the obstacle label toggles the checkbox, just like a "normal" checkbox label...
      mChkNoObstacle->setChecked( !mChkNoObstacle->isChecked() );
      return true;
    }
    return false;
  }
  return QgsTextFormatWidget::eventFilter( object, event );
}

void QgsLabelingGui::updateUi()
{
  // enable/disable inline groupbox-like setups (that need to honor data defined setting)

  syncDefinedCheckboxFrame( mBufferDrawDDBtn, mBufferDrawChkBx, mBufferFrame );
  syncDefinedCheckboxFrame( mEnableMaskDDBtn, mEnableMaskChkBx, mMaskFrame );
  syncDefinedCheckboxFrame( mShapeDrawDDBtn, mShapeDrawChkBx, mShapeFrame );
  syncDefinedCheckboxFrame( mShadowDrawDDBtn, mShadowDrawChkBx, mShadowFrame );
  syncDefinedCheckboxFrame( mCalloutDrawDDBtn, mCalloutsDrawCheckBox, mCalloutFrame );

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

void QgsLabelingGui::setFormatFromStyle( const QString &name, QgsStyle::StyleEntity type, const QString &stylePath )
{
  QgsStyle *style = QgsProject::instance()->styleSettings()->styleAtPath( stylePath );

  if ( !style )
    style = QgsStyle::defaultStyle();

  switch ( type )
  {
    case QgsStyle::SymbolEntity:
    case QgsStyle::ColorrampEntity:
    case QgsStyle::TagEntity:
    case QgsStyle::SmartgroupEntity:
    case QgsStyle::TextFormatEntity:
    case QgsStyle::LegendPatchShapeEntity:
    case QgsStyle::Symbol3DEntity:
    {
      QgsTextFormatWidget::setFormatFromStyle( name, type, stylePath );
      return;
    }

    case QgsStyle::LabelSettingsEntity:
    {
      if ( !style->labelSettingsNames().contains( name ) )
        return;

      QgsPalLayerSettings settings = style->labelSettings( name );
      if ( settings.fieldName.isEmpty() )
      {
        // if saved settings doesn't have a field name stored, retain the current one
        bool isExpression;
        settings.fieldName = mFieldExpressionWidget->currentField( &isExpression );
        settings.isExpression = isExpression;
      }
      setSettings( settings );
      break;
    }
  }
}

void QgsLabelingGui::setContext( const QgsSymbolWidgetContext &context )
{
  if ( QgsCalloutWidget *cw = qobject_cast<QgsCalloutWidget *>( mCalloutStackedWidget->currentWidget() ) )
  {
    cw->setContext( context );
  }
  QgsTextFormatWidget::setContext( context );
}

void QgsLabelingGui::saveFormat()
{
  QgsStyleSaveDialog saveDlg( this, QgsStyle::LabelSettingsEntity );
  saveDlg.setDefaultTags( mTextFormatsListWidget->currentTagFilter() );
  if ( !saveDlg.exec() )
    return;

  if ( saveDlg.name().isEmpty() )
    return;

  QgsStyle *style = saveDlg.destinationStyle();
  if ( !style )
    return;

  switch ( saveDlg.selectedType() )
  {
    case QgsStyle::TextFormatEntity:
    {
      // check if there is no format with same name
      if ( style->textFormatNames().contains( saveDlg.name() ) )
      {
        const int res = QMessageBox::warning( this, tr( "Save Text Format" ), tr( "Format with name '%1' already exists. Overwrite?" ).arg( saveDlg.name() ), QMessageBox::Yes | QMessageBox::No );
        if ( res != QMessageBox::Yes )
        {
          return;
        }
        style->removeTextFormat( saveDlg.name() );
      }
      const QStringList symbolTags = saveDlg.tags().split( ',' );

      const QgsTextFormat newFormat = format();
      style->addTextFormat( saveDlg.name(), newFormat );
      style->saveTextFormat( saveDlg.name(), newFormat, saveDlg.isFavorite(), symbolTags );
      break;
    }

    case QgsStyle::LabelSettingsEntity:
    {
      // check if there is no settings with same name
      if ( style->labelSettingsNames().contains( saveDlg.name() ) )
      {
        const int res = QMessageBox::warning( this, tr( "Save Label Settings" ), tr( "Label settings with the name '%1' already exist. Overwrite?" ).arg( saveDlg.name() ), QMessageBox::Yes | QMessageBox::No );
        if ( res != QMessageBox::Yes )
        {
          return;
        }
        style->removeLabelSettings( saveDlg.name() );
      }
      const QStringList symbolTags = saveDlg.tags().split( ',' );

      const QgsPalLayerSettings newSettings = layerSettings();
      style->addLabelSettings( saveDlg.name(), newSettings );
      style->saveLabelSettings( saveDlg.name(), newSettings, saveDlg.isFavorite(), symbolTags );
      break;
    }

    case QgsStyle::SymbolEntity:
    case QgsStyle::ColorrampEntity:
    case QgsStyle::TagEntity:
    case QgsStyle::SmartgroupEntity:
    case QgsStyle::LegendPatchShapeEntity:
    case QgsStyle::Symbol3DEntity:
      break;
  }
}

void QgsLabelingGui::updateGeometryTypeBasedWidgets()
{
  Qgis::GeometryType geometryType = mGeomType;

  QgsVectorLayer *vLayer = qobject_cast<QgsVectorLayer *>( mLayer );

  if ( mGeometryGeneratorGroupBox->isChecked() )
    geometryType = mGeometryGeneratorType->currentData().value<Qgis::GeometryType>();
  else if ( vLayer )
    geometryType = vLayer->geometryType();

  // show/hide options based upon geometry type
  chkMergeLines->setVisible( geometryType == Qgis::GeometryType::Line );
  mDirectSymbolsFrame->setVisible( geometryType == Qgis::GeometryType::Line );
  mMinSizeFrame->setVisible( geometryType != Qgis::GeometryType::Point );
  mPolygonFeatureOptionsFrame->setVisible( geometryType == Qgis::GeometryType::Polygon );


  const Qgis::LabelPlacement prevPlacement = static_cast<Qgis::LabelPlacement>( mPlacementModeComboBox->currentData().toInt() );
  mPlacementModeComboBox->clear();

  switch ( geometryType )
  {
    case Qgis::GeometryType::Point:
      mPlacementModeComboBox->addItem( tr( "Cartographic" ), static_cast<int>( Qgis::LabelPlacement::OrderedPositionsAroundPoint ) );
      mPlacementModeComboBox->addItem( tr( "Around Point" ), static_cast<int>( Qgis::LabelPlacement::AroundPoint ) );
      mPlacementModeComboBox->addItem( tr( "Offset from Point" ), static_cast<int>( Qgis::LabelPlacement::OverPoint ) );
      break;

    case Qgis::GeometryType::Line:
      mPlacementModeComboBox->addItem( tr( "Parallel" ), static_cast<int>( Qgis::LabelPlacement::Line ) );
      mPlacementModeComboBox->addItem( tr( "Curved" ), static_cast<int>( Qgis::LabelPlacement::Curved ) );
      mPlacementModeComboBox->addItem( tr( "Horizontal" ), static_cast<int>( Qgis::LabelPlacement::Horizontal ) );
      break;

    case Qgis::GeometryType::Polygon:
      mPlacementModeComboBox->addItem( tr( "Offset from Centroid" ), static_cast<int>( Qgis::LabelPlacement::OverPoint ) );
      mPlacementModeComboBox->addItem( tr( "Around Centroid" ), static_cast<int>( Qgis::LabelPlacement::AroundPoint ) );
      mPlacementModeComboBox->addItem( tr( "Horizontal" ), static_cast<int>( Qgis::LabelPlacement::Horizontal ) );
      mPlacementModeComboBox->addItem( tr( "Free (Angled)" ), static_cast<int>( Qgis::LabelPlacement::Free ) );
      mPlacementModeComboBox->addItem( tr( "Using Perimeter" ), static_cast<int>( Qgis::LabelPlacement::Line ) );
      mPlacementModeComboBox->addItem( tr( "Using Perimeter (Curved)" ), static_cast<int>( Qgis::LabelPlacement::PerimeterCurved ) );
      mPlacementModeComboBox->addItem( tr( "Outside Polygons" ), static_cast<int>( Qgis::LabelPlacement::OutsidePolygons ) );
      break;

    case Qgis::GeometryType::Null:
      break;
    case Qgis::GeometryType::Unknown:
      qFatal( "unknown geometry type unexpected" );
  }

  if ( mPlacementModeComboBox->findData( static_cast<int>( prevPlacement ) ) != -1 )
  {
    mPlacementModeComboBox->setCurrentIndex( mPlacementModeComboBox->findData( static_cast<int>( prevPlacement ) ) );
  }

  if ( geometryType == Qgis::GeometryType::Point || geometryType == Qgis::GeometryType::Polygon )
  {
    // follow placement alignment is only valid for point or polygon layers
    if ( mFontMultiLineAlignComboBox->findData( static_cast<int>( Qgis::LabelMultiLineAlignment::FollowPlacement ) ) == -1 )
      mFontMultiLineAlignComboBox->addItem( tr( "Follow Label Placement" ), static_cast<int>( Qgis::LabelMultiLineAlignment::FollowPlacement ) );
  }
  else
  {
    const int idx = mFontMultiLineAlignComboBox->findData( static_cast<int>( Qgis::LabelMultiLineAlignment::FollowPlacement ) );
    if ( idx >= 0 )
      mFontMultiLineAlignComboBox->removeItem( idx );
  }

  updatePlacementWidgets();
  updateLinePlacementOptions();
}

void QgsLabelingGui::showGeometryGeneratorExpressionBuilder()
{
  QgsVectorLayer *vLayer = qobject_cast<QgsVectorLayer *>( mLayer );
  QgsExpressionBuilderDialog expressionBuilder( vLayer );

  expressionBuilder.setExpressionText( mGeometryGenerator->text() );
  expressionBuilder.setExpressionContext( createExpressionContext() );

  if ( expressionBuilder.exec() )
  {
    mGeometryGenerator->setText( expressionBuilder.expressionText() );
  }
}

void QgsLabelingGui::validateGeometryGeneratorExpression()
{
  bool valid = true;

  QgsVectorLayer *vLayer = qobject_cast<QgsVectorLayer *>( mLayer );

  if ( mGeometryGeneratorGroupBox->isChecked() )
  {
    if ( !mPreviewFeature.isValid() && vLayer )
      vLayer->getFeatures( QgsFeatureRequest().setLimit( 1 ) ).nextFeature( mPreviewFeature );

    QgsExpression expression( mGeometryGenerator->text() );
    QgsExpressionContext context = createExpressionContext();
    context.setFeature( mPreviewFeature );

    expression.prepare( &context );

    if ( expression.hasParserError() )
    {
      mGeometryGeneratorWarningLabel->setText( expression.parserErrorString() );
      valid = false;
    }
    else
    {
      const QVariant result = expression.evaluate( &context );
      const QgsGeometry geometry = result.value<QgsGeometry>();
      const Qgis::GeometryType configuredGeometryType = mGeometryGeneratorType->currentData().value<Qgis::GeometryType>();
      if ( geometry.isNull() )
      {
        mGeometryGeneratorWarningLabel->setText( tr( "Result of the expression is not a geometry" ) );
        valid = false;
      }
      else if ( geometry.type() != configuredGeometryType )
      {
        mGeometryGeneratorWarningLabel->setText( QStringLiteral( "<p>%1</p><p><a href=\"#determineGeometryGeneratorType\">%2</a></p>" ).arg( tr( "Result of the expression does not match configured geometry type." ), tr( "Change to %1" ).arg( QgsWkbTypes::geometryDisplayString( geometry.type() ) ) ) );
        valid = false;
      }
    }
  }

  // The collapsible groupbox internally changes the visibility of this
  // Work around by setting the visibility deferred in the next event loop cycle.
  QTimer *timer = new QTimer();
  connect( timer, &QTimer::timeout, this, [this, valid]() {
    mGeometryGeneratorWarningLabel->setVisible( !valid );
  } );
  connect( timer, &QTimer::timeout, timer, &QTimer::deleteLater );
  timer->start( 0 );
}

void QgsLabelingGui::determineGeometryGeneratorType()
{
  QgsVectorLayer *vLayer = qobject_cast<QgsVectorLayer *>( mLayer );
  if ( !mPreviewFeature.isValid() && vLayer )
    vLayer->getFeatures( QgsFeatureRequest().setLimit( 1 ) ).nextFeature( mPreviewFeature );

  QgsExpression expression( mGeometryGenerator->text() );
  QgsExpressionContext context = createExpressionContext();
  context.setFeature( mPreviewFeature );

  expression.prepare( &context );
  const QgsGeometry geometry = expression.evaluate( &context ).value<QgsGeometry>();

  mGeometryGeneratorType->setCurrentIndex( mGeometryGeneratorType->findData( QVariant::fromValue( geometry.type() ) ) );
}

void QgsLabelingGui::calloutTypeChanged()
{
  const QString newCalloutType = mCalloutStyleComboBox->currentData().toString();
  QgsCalloutWidget *pew = qobject_cast<QgsCalloutWidget *>( mCalloutStackedWidget->currentWidget() );
  if ( pew )
  {
    if ( pew->callout() && pew->callout()->type() == newCalloutType )
      return;
  }

  // get creation function for new callout from registry
  QgsCalloutRegistry *registry = QgsApplication::calloutRegistry();
  QgsCalloutAbstractMetadata *am = registry->calloutMetadata( newCalloutType );
  if ( !am ) // check whether the metadata is assigned
    return;

  // change callout to a new one (with different type)
  // base new callout on existing callout's properties
  const std::unique_ptr<QgsCallout> newCallout( am->createCallout( pew && pew->callout() ? pew->callout()->properties( QgsReadWriteContext() ) : QVariantMap(), QgsReadWriteContext() ) );
  if ( !newCallout )
    return;

  updateCalloutWidget( newCallout.get() );
  updatePreview();
}


//
// QgsLabelSettingsDialog
//

QgsLabelSettingsDialog::QgsLabelSettingsDialog( const QgsPalLayerSettings &settings, QgsVectorLayer *layer, QgsMapCanvas *mapCanvas, QWidget *parent, Qgis::GeometryType geomType )
  : QDialog( parent )
{
  QVBoxLayout *vLayout = new QVBoxLayout();
  mWidget = new QgsLabelingGui( layer, mapCanvas, settings, nullptr, geomType );
  vLayout->addWidget( mWidget );
  mButtonBox = new QDialogButtonBox( QDialogButtonBox::Cancel | QDialogButtonBox::Help | QDialogButtonBox::Ok, Qt::Horizontal );
  connect( mButtonBox, &QDialogButtonBox::accepted, this, &QDialog::accept );
  connect( mButtonBox, &QDialogButtonBox::rejected, this, &QDialog::reject );
  connect( mButtonBox, &QDialogButtonBox::helpRequested, this, &QgsLabelSettingsDialog::showHelp );
  vLayout->addWidget( mButtonBox );
  setLayout( vLayout );
  setWindowTitle( tr( "Label Settings" ) );
}

QDialogButtonBox *QgsLabelSettingsDialog::buttonBox() const
{
  return mButtonBox;
}

void QgsLabelSettingsDialog::showHelp()
{
  QgsHelp::openHelp( QStringLiteral( "style_library/label_settings.html" ) );
}


///@endcond
