/***************************************************************************
                         qgslayoutelevationprofilewidget.cpp
                         ----------------------
    begin                : January 2023
    copyright            : (C) 2023 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgslayoutelevationprofilewidget.h"

#include "qgscurve.h"
#include "qgselevationprofilecanvas.h"
#include "qgselevationprofilelayertreeview.h"
#include "qgsfillsymbol.h"
#include "qgsgui.h"
#include "qgslayertree.h"
#include "qgslayertreeregistrybridge.h"
#include "qgslayout.h"
#include "qgslayoutatlas.h"
#include "qgslayoutitemelevationprofile.h"
#include "qgslayoutitemregistry.h"
#include "qgslayoutitemwidget.h"
#include "qgslayoutreportcontext.h"
#include "qgslinesymbol.h"
#include "qgsnumericformatselectorwidget.h"
#include "qgsplot.h"
#include "qgsprofilerenderer.h"
#include "qgsprofilesourceregistry.h"
#include "qgsvectorlayer.h"

#include <QMenu>

#include "moc_qgslayoutelevationprofilewidget.cpp"

std::function<void( QgsLayoutElevationProfileWidget *, QMenu * )> QgsLayoutElevationProfileWidget::sBuildCopyMenuFunction = []( QgsLayoutElevationProfileWidget *, QMenu * ) {};

QgsLayoutElevationProfileWidget::QgsLayoutElevationProfileWidget( QgsLayoutItemElevationProfile *profile )
  : QgsLayoutItemBaseWidget( nullptr, profile )
  , mProfile( profile )
  , mLayerTree( new QgsLayerTree() )
  , mLayerTreeBridge( new QgsLayerTreeRegistryBridge( mLayerTree.get(), mProfile->layout() ? mProfile->layout()->project() : QgsProject::instance(), this ) )
{
  Q_ASSERT( mProfile );

  setupUi( this );
  setPanelTitle( tr( "Elevation Profile Properties" ) );

  mCopyFromDockMenu = new QMenu( this );
  connect( mCopyFromDockMenu, &QMenu::aboutToShow, this, [this] {
    sBuildCopyMenuFunction( this, mCopyFromDockMenu );
  } );

  connect( mActionRefresh, &QAction::triggered, this, [this] {
    if ( !mProfile )
    {
      return;
    }
    mProfile->invalidateCache();
    mProfile->refresh();
  } );

  QToolButton *copyFromDockButton = new QToolButton();
  copyFromDockButton->setAutoRaise( true );
  copyFromDockButton->setToolTip( tr( "Copy From Profile" ) );
  copyFromDockButton->setMenu( mCopyFromDockMenu );
  copyFromDockButton->setPopupMode( QToolButton::InstantPopup );
  copyFromDockButton->setIcon( QgsApplication::getThemeIcon( u"/mActionCopyProfileSettings.svg"_s ) );

  mDockToolbar->addWidget( copyFromDockButton );

  //add widget for general composer item properties
  mItemPropertiesWidget = new QgsLayoutItemPropertiesWidget( this, profile );
  mainLayout->addWidget( mItemPropertiesWidget );

  connect( mLayerTree.get(), &QgsLayerTree::layerOrderChanged, this, &QgsLayoutElevationProfileWidget::updateItemSources );
  connect( mLayerTree.get(), &QgsLayerTreeGroup::visibilityChanged, this, &QgsLayoutElevationProfileWidget::updateItemSources );

  mSpinTolerance->setClearValue( 0 );
  connect( mSpinTolerance, qOverload<double>( &QDoubleSpinBox::valueChanged ), this, [this]( double value ) {
    if ( !mProfile || mBlockChanges )
      return;

    mProfile->beginCommand( tr( "Change Profile Tolerance Distance" ), QgsLayoutItem::UndoElevationProfileTolerance );
    mProfile->setTolerance( value );
    mProfile->invalidateCache();
    mProfile->update();
    mProfile->endCommand();
  } );

  connect( mCheckControlledByAtlas, &QCheckBox::toggled, this, [this] {
    if ( !mProfile || mBlockChanges )
      return;

    mProfile->beginCommand( tr( "Change Profile Atlas Control" ) );
    mProfile->setAtlasDriven( mCheckControlledByAtlas->isChecked() );
    mProfile->invalidateCache();
    mProfile->update();
    mProfile->endCommand();
  } );

  // subsections indicator
  mSubsectionsSymbolButton->setSymbolType( Qgis::SymbolType::Line );
  connect( mSubsectionsSymbolButton, &QgsSymbolButton::changed, this, [this] {
    if ( !mProfile || mBlockChanges )
      return;

    mProfile->beginCommand( tr( "Change Profile Subsection Indicator" ), QgsLayoutItem::UndoElevationProfileSubsectionLines );
    mProfile->setSubsectionsSymbol( mSubsectionsSymbolButton->clonedSymbol<QgsLineSymbol>() );
    mProfile->invalidateCache();
    mProfile->update();
    mProfile->endCommand();
  } );
  mSubsectionsSymbolButton->setDefaultSymbol( QgsProfilePlotRenderer::defaultSubSectionsSymbol().release() );

  connect( mSubsectionsActivateCheck, &QGroupBox::toggled, this, [this] {
    if ( !mProfile || mBlockChanges )
      return;

    const bool subsectionsActivated = mSubsectionsActivateCheck->isChecked();
    mProfile->beginCommand( tr( "Change Profile Subsection Indicator" ), QgsLayoutItem::UndoElevationProfileSubsectionLines );
    std::unique_ptr<QgsLineSymbol> subSectionsSymbol( subsectionsActivated ? mSubsectionsSymbolButton->clonedSymbol<QgsLineSymbol>() : nullptr );
    mProfile->setSubsectionsSymbol( subSectionsSymbol.release() );

    mProfile->invalidateCache();
    mProfile->update();
    mProfile->endCommand();
  } );

  mSpinMinDistance->setClearValue( 0 );
  connect( mSpinMinDistance, qOverload<double>( &QDoubleSpinBox::valueChanged ), this, [this]( double value ) {
    if ( !mProfile || mBlockChanges )
      return;

    mProfile->beginCommand( tr( "Change Profile Chart Minimum Distance" ), QgsLayoutItem::UndoElevationProfileMinimumDistance );
    mProfile->plot()->setXMinimum( value );
    mProfile->invalidateCache();
    mProfile->update();
    mProfile->endCommand();
  } );

  connect( mSpinMaxDistance, qOverload<double>( &QDoubleSpinBox::valueChanged ), this, [this]( double value ) {
    if ( !mProfile || mBlockChanges )
      return;

    mProfile->beginCommand( tr( "Change Profile Chart Maximum Distance" ), QgsLayoutItem::UndoElevationProfileMaximumDistance );
    mProfile->plot()->setXMaximum( value );
    mProfile->invalidateCache();
    mProfile->update();
    mProfile->endCommand();
  } );

  mSpinMinElevation->setClearValue( 0 );
  connect( mSpinMinElevation, qOverload<double>( &QDoubleSpinBox::valueChanged ), this, [this]( double value ) {
    if ( !mProfile || mBlockChanges )
      return;

    mProfile->beginCommand( tr( "Change Profile Chart Minimum Elevation" ), QgsLayoutItem::UndoElevationProfileMinimumElevation );
    mProfile->plot()->setYMinimum( value );
    mProfile->invalidateCache();
    mProfile->update();
    mProfile->endCommand();
  } );

  connect( mSpinMaxElevation, qOverload<double>( &QDoubleSpinBox::valueChanged ), this, [this]( double value ) {
    if ( !mProfile || mBlockChanges )
      return;

    mProfile->beginCommand( tr( "Change Profile Chart Maximum Elevation" ), QgsLayoutItem::UndoElevationProfileMaximumElevation );
    mProfile->plot()->setYMaximum( value );
    mProfile->invalidateCache();
    mProfile->update();
    mProfile->endCommand();
  } );

  mDistanceAxisMajorLinesSymbolButton->setSymbolType( Qgis::SymbolType::Line );
  connect( mDistanceAxisMajorLinesSymbolButton, &QgsSymbolButton::changed, this, [this] {
    if ( !mProfile || mBlockChanges )
      return;

    mProfile->beginCommand( tr( "Change Profile Chart Distance Major Gridlines" ), QgsLayoutItem::UndoElevationProfileDistanceMajorGridlines );
    mProfile->plot()->xAxis().setGridMajorSymbol( mDistanceAxisMajorLinesSymbolButton->clonedSymbol<QgsLineSymbol>() );
    mProfile->invalidateCache();
    mProfile->update();
    mProfile->endCommand();
  } );
  mDistanceAxisMajorLinesSymbolButton->setDefaultSymbol( QgsPlotDefaultSettings::axisGridMajorSymbol() );

  mDistanceAxisMinorLinesSymbolButton->setSymbolType( Qgis::SymbolType::Line );
  connect( mDistanceAxisMinorLinesSymbolButton, &QgsSymbolButton::changed, this, [this] {
    if ( !mProfile || mBlockChanges )
      return;

    mProfile->beginCommand( tr( "Change Profile Chart Distance Minor Gridlines" ), QgsLayoutItem::UndoElevationProfileDistanceMinorGridlines );
    mProfile->plot()->xAxis().setGridMinorSymbol( mDistanceAxisMinorLinesSymbolButton->clonedSymbol<QgsLineSymbol>() );
    mProfile->invalidateCache();
    mProfile->update();
    mProfile->endCommand();
  } );
  mDistanceAxisMinorLinesSymbolButton->setDefaultSymbol( QgsPlotDefaultSettings::axisGridMinorSymbol() );

  connect( mDistanceAxisMajorIntervalSpin, qOverload<double>( &QDoubleSpinBox::valueChanged ), this, [this]( double value ) {
    if ( !mProfile || mBlockChanges )
      return;

    mProfile->beginCommand( tr( "Change Profile Chart Distance Major Gridlines" ), QgsLayoutItem::UndoElevationProfileDistanceMajorGridlines );
    mProfile->plot()->xAxis().setGridIntervalMajor( value );
    mProfile->invalidateCache();
    mProfile->update();
    mProfile->endCommand();
  } );

  connect( mDistanceAxisMinorIntervalSpin, qOverload<double>( &QDoubleSpinBox::valueChanged ), this, [this]( double value ) {
    if ( !mProfile || mBlockChanges )
      return;

    mProfile->beginCommand( tr( "Change Profile Chart Distance Minor Gridlines" ), QgsLayoutItem::UndoElevationProfileDistanceMinorGridlines );
    mProfile->plot()->xAxis().setGridIntervalMinor( value );
    mProfile->invalidateCache();
    mProfile->update();
    mProfile->endCommand();
  } );

  connect( mDistanceAxisLabelIntervalSpin, qOverload<double>( &QDoubleSpinBox::valueChanged ), this, [this]( double value ) {
    if ( !mProfile || mBlockChanges )
      return;

    mProfile->beginCommand( tr( "Change Profile Chart Distance Label" ), QgsLayoutItem::UndoElevationProfileDistanceLabels );
    mProfile->plot()->xAxis().setLabelInterval( value );
    mProfile->invalidateCache();
    mProfile->update();
    mProfile->endCommand();
  } );

  mElevationAxisMajorLinesSymbolButton->setSymbolType( Qgis::SymbolType::Line );
  connect( mElevationAxisMajorLinesSymbolButton, &QgsSymbolButton::changed, this, [this] {
    if ( !mProfile || mBlockChanges )
      return;

    mProfile->beginCommand( tr( "Change Profile Chart Elevation Major Gridlines" ), QgsLayoutItem::UndoElevationProfileElevationMajorGridlines );
    mProfile->plot()->yAxis().setGridMajorSymbol( mElevationAxisMajorLinesSymbolButton->clonedSymbol<QgsLineSymbol>() );
    mProfile->invalidateCache();
    mProfile->update();
    mProfile->endCommand();
  } );
  mElevationAxisMajorLinesSymbolButton->setDefaultSymbol( QgsPlotDefaultSettings::axisGridMajorSymbol() );

  mElevationAxisMinorLinesSymbolButton->setSymbolType( Qgis::SymbolType::Line );
  connect( mElevationAxisMinorLinesSymbolButton, &QgsSymbolButton::changed, this, [this] {
    if ( !mProfile || mBlockChanges )
      return;

    mProfile->beginCommand( tr( "Change Profile Chart Elevation Minor Gridlines" ), QgsLayoutItem::UndoElevationProfileElevationMinorGridlines );
    mProfile->plot()->yAxis().setGridMinorSymbol( mElevationAxisMinorLinesSymbolButton->clonedSymbol<QgsLineSymbol>() );
    mProfile->invalidateCache();
    mProfile->update();
    mProfile->endCommand();
  } );
  mElevationAxisMinorLinesSymbolButton->setDefaultSymbol( QgsPlotDefaultSettings::axisGridMinorSymbol() );

  connect( mElevationAxisLabelIntervalSpin, qOverload<double>( &QDoubleSpinBox::valueChanged ), this, [this]( double value ) {
    if ( !mProfile || mBlockChanges )
      return;

    mProfile->beginCommand( tr( "Change Profile Chart Elevation Label" ), QgsLayoutItem::UndoElevationProfileElevationLabels );
    mProfile->plot()->yAxis().setLabelInterval( value );
    mProfile->invalidateCache();
    mProfile->update();
    mProfile->endCommand();
  } );

  connect( mElevationAxisMajorIntervalSpin, qOverload<double>( &QDoubleSpinBox::valueChanged ), this, [this]( double value ) {
    if ( !mProfile || mBlockChanges )
      return;

    mProfile->beginCommand( tr( "Change Profile Chart Elevation Major Gridlines" ), QgsLayoutItem::UndoElevationProfileElevationMajorGridlines );
    mProfile->plot()->yAxis().setGridIntervalMajor( value );
    mProfile->invalidateCache();
    mProfile->update();
    mProfile->endCommand();
  } );

  connect( mElevationAxisMinorIntervalSpin, qOverload<double>( &QDoubleSpinBox::valueChanged ), this, [this]( double value ) {
    if ( !mProfile || mBlockChanges )
      return;

    mProfile->beginCommand( tr( "Change Profile Chart Distance Minor Gridlines" ), QgsLayoutItem::UndoElevationProfileElevationMinorGridlines );
    mProfile->plot()->yAxis().setGridIntervalMinor( value );
    mProfile->invalidateCache();
    mProfile->update();
    mProfile->endCommand();
  } );

  mChartBackgroundSymbolButton->setSymbolType( Qgis::SymbolType::Fill );
  connect( mChartBackgroundSymbolButton, &QgsSymbolButton::changed, this, [this] {
    if ( !mProfile || mBlockChanges )
      return;

    mProfile->beginCommand( tr( "Change Profile Chart Background" ), QgsLayoutItem::UndoElevationProfileChartBackground );
    mProfile->plot()->setChartBackgroundSymbol( mChartBackgroundSymbolButton->clonedSymbol<QgsFillSymbol>() );
    mProfile->invalidateCache();
    mProfile->update();
    mProfile->endCommand();
  } );
  mChartBackgroundSymbolButton->setDefaultSymbol( QgsPlotDefaultSettings::chartBackgroundSymbol() );

  mChartBorderSymbolButton->setSymbolType( Qgis::SymbolType::Fill );
  connect( mChartBorderSymbolButton, &QgsSymbolButton::changed, this, [this] {
    if ( !mProfile || mBlockChanges )
      return;

    mProfile->beginCommand( tr( "Change Profile Chart Border" ), QgsLayoutItem::UndoElevationProfileChartBorder );
    mProfile->plot()->setChartBorderSymbol( mChartBorderSymbolButton->clonedSymbol<QgsFillSymbol>() );
    mProfile->invalidateCache();
    mProfile->update();
    mProfile->endCommand();
  } );
  mChartBorderSymbolButton->setDefaultSymbol( QgsPlotDefaultSettings::chartBorderSymbol() );

  connect( mDistanceAxisLabelFormatButton, &QPushButton::clicked, this, [this] {
    if ( !mProfile || mBlockChanges )
      return;

    QgsNumericFormatSelectorWidget *widget = new QgsNumericFormatSelectorWidget( this );
    widget->setPanelTitle( tr( "Distance Number Format" ) );
    widget->setFormat( mProfile->plot()->xAxis().numericFormat() );
    connect( widget, &QgsNumericFormatSelectorWidget::changed, this, [this, widget] {
      mProfile->beginCommand( tr( "Change Profile Chart Distance Format" ), QgsLayoutItem::UndoElevationProfileDistanceFormat );
      mProfile->plot()->xAxis().setNumericFormat( widget->format() );
      mProfile->invalidateCache();
      mProfile->endCommand();
      mProfile->update();
    } );
    openPanel( widget );
  } );

  connect( mElevationAxisLabelFormatButton, &QPushButton::clicked, this, [this] {
    if ( !mProfile || mBlockChanges )
      return;

    QgsNumericFormatSelectorWidget *widget = new QgsNumericFormatSelectorWidget( this );
    widget->setPanelTitle( tr( "Elevation Number Format" ) );
    widget->setFormat( mProfile->plot()->yAxis().numericFormat() );
    connect( widget, &QgsNumericFormatSelectorWidget::changed, this, [this, widget] {
      mProfile->beginCommand( tr( "Change Profile Chart Elevation Format" ), QgsLayoutItem::UndoElevationProfileElevationFormat );
      mProfile->plot()->yAxis().setNumericFormat( widget->format() );
      mProfile->invalidateCache();
      mProfile->endCommand();
      mProfile->update();
    } );
    openPanel( widget );
  } );

  mDistanceAxisLabelFontButton->setDialogTitle( tr( "Distance Label Font" ) );
  mElevationAxisLabelFontButton->setDialogTitle( tr( "Elevation Label Font" ) );
  mDistanceAxisLabelFontButton->setMode( QgsFontButton::ModeTextRenderer );
  mElevationAxisLabelFontButton->setMode( QgsFontButton::ModeTextRenderer );

  connect( mDistanceAxisLabelFontButton, &QgsFontButton::changed, this, [this] {
    if ( !mProfile || mBlockChanges )
      return;

    mProfile->beginCommand( tr( "Change Profile Chart Distance Font" ), QgsLayoutItem::UndoElevationProfileDistanceFont );
    mProfile->plot()->xAxis().setTextFormat( mDistanceAxisLabelFontButton->textFormat() );
    mProfile->invalidateCache();
    mProfile->endCommand();
    mProfile->update();
  } );

  connect( mElevationAxisLabelFontButton, &QgsFontButton::changed, this, [this] {
    if ( !mProfile || mBlockChanges )
      return;

    mProfile->beginCommand( tr( "Change Profile Chart Elevation Font" ), QgsLayoutItem::UndoElevationProfileElevationFont );
    mProfile->plot()->yAxis().setTextFormat( mElevationAxisLabelFontButton->textFormat() );
    mProfile->invalidateCache();
    mProfile->endCommand();
    mProfile->update();
  } );

  mSpinLeftMargin->setClearValue( 0 );
  connect( mSpinLeftMargin, qOverload<double>( &QDoubleSpinBox::valueChanged ), this, [this]( double value ) {
    if ( !mProfile || mBlockChanges )
      return;

    mProfile->beginCommand( tr( "Change Profile Chart Left Margin" ), QgsLayoutItem::UndoMarginLeft );
    QgsMargins margins = mProfile->plot()->margins();
    margins.setLeft( value );
    mProfile->plot()->setMargins( margins );
    mProfile->invalidateCache();
    mProfile->update();
    mProfile->endCommand();
  } );

  mSpinRightMargin->setClearValue( 0 );
  connect( mSpinRightMargin, qOverload<double>( &QDoubleSpinBox::valueChanged ), this, [this]( double value ) {
    if ( !mProfile || mBlockChanges )
      return;

    mProfile->beginCommand( tr( "Change Profile Chart Right Margin" ), QgsLayoutItem::UndoMarginRight );
    QgsMargins margins = mProfile->plot()->margins();
    margins.setRight( value );
    mProfile->plot()->setMargins( margins );
    mProfile->invalidateCache();
    mProfile->update();
    mProfile->endCommand();
  } );

  mSpinTopMargin->setClearValue( 0 );
  connect( mSpinTopMargin, qOverload<double>( &QDoubleSpinBox::valueChanged ), this, [this]( double value ) {
    if ( !mProfile || mBlockChanges )
      return;

    mProfile->beginCommand( tr( "Change Profile Chart Top Margin" ), QgsLayoutItem::UndoMarginTop );
    QgsMargins margins = mProfile->plot()->margins();
    margins.setTop( value );
    mProfile->plot()->setMargins( margins );
    mProfile->invalidateCache();
    mProfile->update();
    mProfile->endCommand();
  } );

  mSpinBottomMargin->setClearValue( 0 );
  connect( mSpinBottomMargin, qOverload<double>( &QDoubleSpinBox::valueChanged ), this, [this]( double value ) {
    if ( !mProfile || mBlockChanges )
      return;

    mProfile->beginCommand( tr( "Change Profile Chart Bottom Margin" ), QgsLayoutItem::UndoMarginBottom );
    QgsMargins margins = mProfile->plot()->margins();
    margins.setBottom( value );
    mProfile->plot()->setMargins( margins );
    mProfile->invalidateCache();
    mProfile->update();
    mProfile->endCommand();
  } );

  for ( Qgis::DistanceUnit unit :
        {
          Qgis::DistanceUnit::Kilometers,
          Qgis::DistanceUnit::Meters,
          Qgis::DistanceUnit::Centimeters,
          Qgis::DistanceUnit::Millimeters,
          Qgis::DistanceUnit::Miles,
          Qgis::DistanceUnit::NauticalMiles,
          Qgis::DistanceUnit::Yards,
          Qgis::DistanceUnit::Feet,
          Qgis::DistanceUnit::Inches,
          Qgis::DistanceUnit::Degrees,
        } )
  {
    QString title;
    if ( ( QgsGui::higFlags() & QgsGui::HigDialogTitleIsTitleCase ) )
    {
      title = QgsStringUtils::capitalize( QgsUnitTypes::toString( unit ), Qgis::Capitalization::TitleCase );
    }
    else
    {
      title = QgsUnitTypes::toString( unit );
    }
    mDistanceUnitCombo->addItem( title, QVariant::fromValue( unit ) );
  }

  connect( mDistanceUnitCombo, qOverload<int>( &QComboBox::currentIndexChanged ), this, [this]( int ) {
    if ( !mProfile || mBlockChanges )
      return;

    mProfile->beginCommand( tr( "Change Profile Chart Units" ) );
    mProfile->setDistanceUnit( mDistanceUnitCombo->currentData().value<Qgis::DistanceUnit>() );
    mProfile->invalidateCache();
    mProfile->update();
    mProfile->endCommand();
  } );

  mDistanceLabelsCombo->addItem( tr( "None" ), QVariant::fromValue( Qgis::PlotAxisSuffixPlacement::NoLabels ) );
  mDistanceLabelsCombo->addItem( tr( "Every Value" ), QVariant::fromValue( Qgis::PlotAxisSuffixPlacement::EveryLabel ) );
  mDistanceLabelsCombo->addItem( tr( "First Value" ), QVariant::fromValue( Qgis::PlotAxisSuffixPlacement::FirstLabel ) );
  mDistanceLabelsCombo->addItem( tr( "Last Value" ), QVariant::fromValue( Qgis::PlotAxisSuffixPlacement::LastLabel ) );
  mDistanceLabelsCombo->addItem( tr( "First and Last Values" ), QVariant::fromValue( Qgis::PlotAxisSuffixPlacement::FirstAndLastLabels ) );
  connect( mDistanceLabelsCombo, qOverload<int>( &QComboBox::currentIndexChanged ), this, [this]( int ) {
    if ( !mProfile || mBlockChanges )
      return;

    mProfile->beginCommand( tr( "Change Profile Chart Label Placement" ) );
    mProfile->plot()->xAxis().setLabelSuffixPlacement( mDistanceLabelsCombo->currentData().value<Qgis::PlotAxisSuffixPlacement>() );
    mProfile->invalidateCache();
    mProfile->update();
    mProfile->endCommand();
  } );


  registerDataDefinedButton( mDDBtnTolerance, QgsLayoutObject::DataDefinedProperty::ElevationProfileTolerance );
  registerDataDefinedButton( mDDBtnMinDistance, QgsLayoutObject::DataDefinedProperty::ElevationProfileMinimumDistance );
  registerDataDefinedButton( mDDBtnMaxDistance, QgsLayoutObject::DataDefinedProperty::ElevationProfileMaximumDistance );
  registerDataDefinedButton( mDDBtnMinElevation, QgsLayoutObject::DataDefinedProperty::ElevationProfileMinimumElevation );
  registerDataDefinedButton( mDDBtnMaxElevation, QgsLayoutObject::DataDefinedProperty::ElevationProfileMaximumElevation );
  registerDataDefinedButton( mDDBtnDistanceMajorInterval, QgsLayoutObject::DataDefinedProperty::ElevationProfileDistanceMajorInterval );
  registerDataDefinedButton( mDDBtnDistanceMinorInterval, QgsLayoutObject::DataDefinedProperty::ElevationProfileDistanceMinorInterval );
  registerDataDefinedButton( mDDBtnDistanceLabelInterval, QgsLayoutObject::DataDefinedProperty::ElevationProfileDistanceLabelInterval );
  registerDataDefinedButton( mDDBtnElevationMajorInterval, QgsLayoutObject::DataDefinedProperty::ElevationProfileElevationMajorInterval );
  registerDataDefinedButton( mDDBtnElevationMinorInterval, QgsLayoutObject::DataDefinedProperty::ElevationProfileElevationMinorInterval );
  registerDataDefinedButton( mDDBtnElevationLabelInterval, QgsLayoutObject::DataDefinedProperty::ElevationProfileElevationLabelInterval );
  registerDataDefinedButton( mDDBtnLeftMargin, QgsLayoutObject::DataDefinedProperty::MarginLeft );
  registerDataDefinedButton( mDDBtnRightMargin, QgsLayoutObject::DataDefinedProperty::MarginRight );
  registerDataDefinedButton( mDDBtnTopMargin, QgsLayoutObject::DataDefinedProperty::MarginTop );
  registerDataDefinedButton( mDDBtnBottomMargin, QgsLayoutObject::DataDefinedProperty::MarginBottom );

  mLayerTreeView = new QgsElevationProfileLayerTreeView( mLayerTree.get() );

  QVBoxLayout *vl = new QVBoxLayout();
  vl->setContentsMargins( 0, 0, 0, 0 );
  vl->addWidget( mLayerTreeView );
  mTreeViewContainer->setLayout( vl );

  mBlockChanges++;
  mLayerTreeView->populateInitialSources( mProfile->layout() && mProfile->layout()->project() ? mProfile->layout()->project() : QgsProject::instance() );
  mBlockChanges--;

  setGuiElementValues();

  mSubsectionsSymbolButton->registerExpressionContextGenerator( mProfile );
  mDistanceAxisMajorLinesSymbolButton->registerExpressionContextGenerator( mProfile );
  mDistanceAxisMinorLinesSymbolButton->registerExpressionContextGenerator( mProfile );
  mElevationAxisMajorLinesSymbolButton->registerExpressionContextGenerator( mProfile );
  mElevationAxisMinorLinesSymbolButton->registerExpressionContextGenerator( mProfile );
  mChartBackgroundSymbolButton->registerExpressionContextGenerator( mProfile );
  mChartBorderSymbolButton->registerExpressionContextGenerator( mProfile );
  mDistanceAxisLabelFontButton->registerExpressionContextGenerator( mProfile );
  mElevationAxisLabelFontButton->registerExpressionContextGenerator( mProfile );

  mSubsectionsSymbolButton->setLayer( coverageLayer() );
  mDistanceAxisMajorLinesSymbolButton->setLayer( coverageLayer() );
  mDistanceAxisMinorLinesSymbolButton->setLayer( coverageLayer() );
  mElevationAxisMajorLinesSymbolButton->setLayer( coverageLayer() );
  mElevationAxisMinorLinesSymbolButton->setLayer( coverageLayer() );
  mDistanceAxisLabelFontButton->setLayer( coverageLayer() );
  mElevationAxisLabelFontButton->setLayer( coverageLayer() );
  mChartBackgroundSymbolButton->setLayer( coverageLayer() );
  mChartBorderSymbolButton->setLayer( coverageLayer() );

  if ( mProfile->layout() )
  {
    connect( &mProfile->layout()->reportContext(), &QgsLayoutReportContext::layerChanged, this, [this]( QgsVectorLayer *layer ) {
      mSubsectionsSymbolButton->setLayer( layer );
      mDistanceAxisMajorLinesSymbolButton->setLayer( layer );
      mDistanceAxisMinorLinesSymbolButton->setLayer( layer );
      mElevationAxisMajorLinesSymbolButton->setLayer( layer );
      mElevationAxisMinorLinesSymbolButton->setLayer( layer );
      mDistanceAxisLabelFontButton->setLayer( layer );
      mElevationAxisLabelFontButton->setLayer( layer );
      mChartBackgroundSymbolButton->setLayer( layer );
      mChartBorderSymbolButton->setLayer( layer );
    } );

    connect( &mProfile->layout()->reportContext(), &QgsLayoutReportContext::layerChanged, this, &QgsLayoutElevationProfileWidget::atlasLayerChanged );
  }

  if ( QgsLayoutAtlas *atlas = layoutAtlas() )
  {
    connect( atlas, &QgsLayoutAtlas::toggled, this, &QgsLayoutElevationProfileWidget::layoutAtlasToggled );
    layoutAtlasToggled( atlas->enabled() );
  }
}

QgsLayoutElevationProfileWidget::~QgsLayoutElevationProfileWidget() = default;

void QgsLayoutElevationProfileWidget::setMasterLayout( QgsMasterLayoutInterface *masterLayout )
{
  if ( mItemPropertiesWidget )
    mItemPropertiesWidget->setMasterLayout( masterLayout );
}

QgsExpressionContext QgsLayoutElevationProfileWidget::createExpressionContext() const
{
  QgsExpressionContext context = mProfile->createExpressionContext();

  auto plotScope = std::make_unique<QgsExpressionContextScope>( u"plot"_s );
  plotScope->addVariable( QgsExpressionContextScope::StaticVariable( u"plot_axis"_s, QString(), true ) );
  plotScope->addVariable( QgsExpressionContextScope::StaticVariable( u"plot_axis_value"_s, 0.0, true ) );
  context.appendScope( plotScope.release() );

  return context;
}

void QgsLayoutElevationProfileWidget::setDesignerInterface( QgsLayoutDesignerInterface *iface )
{
  mInterface = iface;
  QgsLayoutItemBaseWidget::setDesignerInterface( iface );
}

void QgsLayoutElevationProfileWidget::setReportTypeString( const QString &string )
{
  mCheckControlledByAtlas->setText( tr( "Controlled by %1" ).arg( string == tr( "atlas" ) ? tr( "Atlas" ) : tr( "Report" ) ) );
}

void QgsLayoutElevationProfileWidget::copySettingsFromProfileCanvas( QgsElevationProfileCanvas *canvas )
{
  mBlockChanges++;

  mProfile->setCrs( canvas->crs() );

  mSpinTolerance->setValue( canvas->tolerance() );
  mProfile->setTolerance( canvas->tolerance() );

  mProfile->setDistanceUnit( canvas->distanceUnit() );
  mDistanceUnitCombo->setCurrentIndex( mDistanceUnitCombo->findData( QVariant::fromValue( canvas->distanceUnit() ) ) );

  mProfile->plot()->xAxis().setLabelSuffixPlacement( canvas->plot().xAxis().labelSuffixPlacement() );
  mDistanceLabelsCombo->setCurrentIndex( mDistanceLabelsCombo->findData( QVariant::fromValue( canvas->plot().xAxis().labelSuffixPlacement() ) ) );

  if ( const QgsCurve *curve = canvas->profileCurve() )
    mProfile->setProfileCurve( curve->clone() );

  mSpinMinDistance->setValue( canvas->plot().xMinimum() );
  mSpinMinDistance->setClearValue( canvas->plot().xMinimum() );
  mProfile->plot()->setXMinimum( canvas->plot().xMinimum() );

  mSpinMaxDistance->setValue( canvas->plot().xMaximum() );
  mSpinMaxDistance->setClearValue( canvas->plot().xMaximum() );
  mProfile->plot()->setXMaximum( canvas->plot().xMaximum() );

  mDistanceAxisMajorIntervalSpin->setValue( canvas->plot().xAxis().gridIntervalMajor() );
  mDistanceAxisMajorIntervalSpin->setClearValue( canvas->plot().xAxis().gridIntervalMajor() );
  mProfile->plot()->xAxis().setGridIntervalMajor( canvas->plot().xAxis().gridIntervalMajor() );

  mDistanceAxisMinorIntervalSpin->setValue( canvas->plot().xAxis().gridIntervalMinor() );
  mDistanceAxisMinorIntervalSpin->setClearValue( canvas->plot().xAxis().gridIntervalMinor() );
  mProfile->plot()->xAxis().setGridIntervalMinor( canvas->plot().xAxis().gridIntervalMinor() );

  mDistanceAxisLabelIntervalSpin->setValue( canvas->plot().xAxis().labelInterval() );
  mDistanceAxisLabelIntervalSpin->setClearValue( canvas->plot().xAxis().labelInterval() );
  mProfile->plot()->xAxis().setLabelInterval( canvas->plot().xAxis().labelInterval() );

  mSpinMinElevation->setValue( canvas->plot().yMinimum() );
  mSpinMinElevation->setClearValue( canvas->plot().yMinimum() );
  mProfile->plot()->setYMinimum( canvas->plot().yMinimum() );

  mSpinMaxElevation->setValue( canvas->plot().yMaximum() );
  mSpinMaxElevation->setClearValue( canvas->plot().yMaximum() );
  mProfile->plot()->setYMaximum( canvas->plot().yMaximum() );

  mElevationAxisMajorIntervalSpin->setValue( canvas->plot().yAxis().gridIntervalMajor() );
  mElevationAxisMajorIntervalSpin->setClearValue( canvas->plot().yAxis().gridIntervalMajor() );
  mProfile->plot()->yAxis().setGridIntervalMajor( canvas->plot().yAxis().gridIntervalMajor() );

  mElevationAxisMinorIntervalSpin->setValue( canvas->plot().yAxis().gridIntervalMinor() );
  mElevationAxisMinorIntervalSpin->setClearValue( canvas->plot().yAxis().gridIntervalMinor() );
  mProfile->plot()->yAxis().setGridIntervalMinor( canvas->plot().yAxis().gridIntervalMinor() );

  mElevationAxisLabelIntervalSpin->setValue( canvas->plot().yAxis().labelInterval() );
  mElevationAxisLabelIntervalSpin->setClearValue( canvas->plot().yAxis().labelInterval() );
  mProfile->plot()->yAxis().setLabelInterval( canvas->plot().yAxis().labelInterval() );

  const QgsLineSymbol *subSectionsSymbol = canvas->subsectionsSymbol() ? canvas->subsectionsSymbol() : nullptr;
  const bool subSectionsEnabled = static_cast< bool >( subSectionsSymbol );
  mSubsectionsActivateCheck->setChecked( subSectionsEnabled );
  if ( subSectionsSymbol )
  {
    mSubsectionsSymbolButton->setSymbol( subSectionsSymbol->clone() );
    mProfile->setSubsectionsSymbol( subSectionsSymbol->clone() );
  }

  QList<QgsMapLayer *> canvasLayers = canvas->layers();
  mProfile->setLayers( canvasLayers );

  QList<QgsAbstractProfileSource *> canvasSources = canvas->sources();
  mProfile->setSources( canvasSources );

  syncLayerTreeAndProfileItemSources();

  mProfile->invalidateCache();
  mProfile->update();
  mBlockChanges--;
}

void QgsLayoutElevationProfileWidget::syncLayerTreeAndProfileItemSources()
{
  // Update layer tree node visibility, based on layout item profile
  const QList<QgsLayerTreeNode *> nodes = mLayerTree->findLayersAndCustomNodes();
  for ( QgsLayerTreeNode *node : nodes )
  {
    QgsAbstractProfileSource *source = nullptr;
    if ( QgsLayerTree::isLayer( node ) )
    {
      QgsLayerTreeLayer *layerNode = QgsLayerTree::toLayer( node );
      QgsMapLayer *layer = layerNode->layer();
      if ( !layer )
        continue;

      source = layer->profileSource();
    }
    else if ( QgsLayerTree::isCustomNode( node ) && node->customProperty( u"source"_s ) == QgsElevationProfileLayerTreeView::CUSTOM_NODE_ELEVATION_PROFILE_SOURCE )
    {
      QgsLayerTreeCustomNode *customNode = QgsLayerTree::toCustomNode( node );
      source = QgsApplication::profileSourceRegistry()->findSourceById( customNode->nodeId() );
    }

    if ( !source )
    {
      node->setItemVisibilityChecked( false );
    }
    else
    {
      node->setItemVisibilityChecked( mProfile->sources().contains( source ) );
    }
  }

  // Update layer tree node ordering, based on layout item profile
  QList< QgsLayerTreeNode * > orderedNodes;
  const QList<QgsAbstractProfileSource *> profileSources = mProfile->sources();
  for ( const QgsAbstractProfileSource *source : profileSources )
  {
    if ( QgsLayerTreeLayer *layerNode = mLayerTree->findLayer( source->profileSourceId() ) )
    {
      orderedNodes << layerNode;
    }
    else if ( QgsLayerTreeCustomNode *customNode = mLayerTree->findCustomNode( source->profileSourceId() ) )
    {
      orderedNodes << customNode;
    }
  }
  mLayerTree->reorderGroupLayersAndCustomNodes( orderedNodes );
}

bool QgsLayoutElevationProfileWidget::setNewItem( QgsLayoutItem *item )
{
  if ( item->type() != QgsLayoutItemRegistry::LayoutElevationProfile )
    return false;

  if ( mProfile )
  {
    disconnect( mProfile, &QgsLayoutObject::changed, this, &QgsLayoutElevationProfileWidget::setGuiElementValues );
  }

  mProfile = qobject_cast<QgsLayoutItemElevationProfile *>( item );
  mItemPropertiesWidget->setItem( mProfile );

  if ( mProfile )
  {
    connect( mProfile, &QgsLayoutObject::changed, this, &QgsLayoutElevationProfileWidget::setGuiElementValues );
    mSubsectionsSymbolButton->registerExpressionContextGenerator( mProfile );
    mDistanceAxisMajorLinesSymbolButton->registerExpressionContextGenerator( mProfile );
    mDistanceAxisMinorLinesSymbolButton->registerExpressionContextGenerator( mProfile );
    mElevationAxisMajorLinesSymbolButton->registerExpressionContextGenerator( mProfile );
    mElevationAxisMinorLinesSymbolButton->registerExpressionContextGenerator( mProfile );
    mDistanceAxisLabelFontButton->registerExpressionContextGenerator( mProfile );
    mElevationAxisLabelFontButton->registerExpressionContextGenerator( mProfile );
    mChartBackgroundSymbolButton->registerExpressionContextGenerator( mProfile );
    mChartBorderSymbolButton->registerExpressionContextGenerator( mProfile );
  }

  setGuiElementValues();

  return true;
}

void QgsLayoutElevationProfileWidget::setGuiElementValues()
{
  mBlockChanges++;

  mSpinTolerance->setValue( mProfile->tolerance() );
  mCheckControlledByAtlas->setChecked( mProfile->atlasDriven() );

  mSpinMinDistance->setValue( mProfile->plot()->xMinimum() );
  mSpinMaxDistance->setValue( mProfile->plot()->xMaximum() );
  mSpinMinElevation->setValue( mProfile->plot()->yMinimum() );
  mSpinMaxElevation->setValue( mProfile->plot()->yMaximum() );

  mSubsectionsActivateCheck->setChecked( mProfile->subsectionsSymbol() );
  if ( mProfile->subsectionsSymbol() )
  {
    mSubsectionsSymbolButton->setSymbol( mProfile->subsectionsSymbol()->clone() );
  }

  if ( mProfile->plot()->xAxis().gridMajorSymbol() )
    mDistanceAxisMajorLinesSymbolButton->setSymbol( mProfile->plot()->xAxis().gridMajorSymbol()->clone() );
  if ( mProfile->plot()->xAxis().gridMinorSymbol() )
    mDistanceAxisMinorLinesSymbolButton->setSymbol( mProfile->plot()->xAxis().gridMinorSymbol()->clone() );
  if ( mProfile->plot()->yAxis().gridMajorSymbol() )
    mElevationAxisMajorLinesSymbolButton->setSymbol( mProfile->plot()->yAxis().gridMajorSymbol()->clone() );
  if ( mProfile->plot()->yAxis().gridMinorSymbol() )
    mElevationAxisMinorLinesSymbolButton->setSymbol( mProfile->plot()->yAxis().gridMinorSymbol()->clone() );

  mDistanceAxisLabelFontButton->setTextFormat( mProfile->plot()->xAxis().textFormat() );
  mElevationAxisLabelFontButton->setTextFormat( mProfile->plot()->yAxis().textFormat() );

  mDistanceUnitCombo->setCurrentIndex( mDistanceUnitCombo->findData( QVariant::fromValue( mProfile->distanceUnit() ) ) );
  mDistanceLabelsCombo->setCurrentIndex( mDistanceLabelsCombo->findData( QVariant::fromValue( mProfile->plot()->xAxis().labelSuffixPlacement() ) ) );

  mDistanceAxisMajorIntervalSpin->setValue( mProfile->plot()->xAxis().gridIntervalMajor() );
  mDistanceAxisMinorIntervalSpin->setValue( mProfile->plot()->xAxis().gridIntervalMinor() );
  mDistanceAxisLabelIntervalSpin->setValue( mProfile->plot()->xAxis().labelInterval() );

  mElevationAxisMajorIntervalSpin->setValue( mProfile->plot()->yAxis().gridIntervalMajor() );
  mElevationAxisMinorIntervalSpin->setValue( mProfile->plot()->yAxis().gridIntervalMinor() );
  mElevationAxisLabelIntervalSpin->setValue( mProfile->plot()->yAxis().labelInterval() );

  if ( mProfile->plot()->chartBackgroundSymbol() )
    mChartBackgroundSymbolButton->setSymbol( mProfile->plot()->chartBackgroundSymbol()->clone() );
  if ( mProfile->plot()->chartBorderSymbol() )
    mChartBorderSymbolButton->setSymbol( mProfile->plot()->chartBorderSymbol()->clone() );

  mSpinLeftMargin->setValue( mProfile->plot()->margins().left() );
  mSpinRightMargin->setValue( mProfile->plot()->margins().right() );
  mSpinTopMargin->setValue( mProfile->plot()->margins().top() );
  mSpinBottomMargin->setValue( mProfile->plot()->margins().bottom() );

  syncLayerTreeAndProfileItemSources();

  updateDataDefinedButton( mDDBtnTolerance );
  updateDataDefinedButton( mDDBtnMinDistance );
  updateDataDefinedButton( mDDBtnMaxDistance );
  updateDataDefinedButton( mDDBtnMinElevation );
  updateDataDefinedButton( mDDBtnMaxElevation );
  updateDataDefinedButton( mDDBtnDistanceMajorInterval );
  updateDataDefinedButton( mDDBtnDistanceMinorInterval );
  updateDataDefinedButton( mDDBtnDistanceLabelInterval );
  updateDataDefinedButton( mDDBtnElevationMajorInterval );
  updateDataDefinedButton( mDDBtnElevationMinorInterval );
  updateDataDefinedButton( mDDBtnElevationLabelInterval );
  updateDataDefinedButton( mDDBtnLeftMargin );
  updateDataDefinedButton( mDDBtnRightMargin );
  updateDataDefinedButton( mDDBtnTopMargin );
  updateDataDefinedButton( mDDBtnBottomMargin );

  mBlockChanges--;
}

void QgsLayoutElevationProfileWidget::updateItemSources()
{
  if ( mBlockChanges )
    return;

  QList<QgsMapLayer *> layers;
  QList<QgsAbstractProfileSource *> sources;
  const QList<QgsLayerTreeNode *> layerAndCustomNodeOrder = mLayerTree->layerAndCustomNodeOrder();
  for ( QgsLayerTreeNode *node : layerAndCustomNodeOrder )
  {
    if ( QgsLayerTree::isLayer( node ) )
    {
      QgsMapLayer *layer = QgsLayerTree::toLayer( node )->layer();
      if ( mLayerTree->findLayer( layer )->isVisible() )
      {
        layers << layer;
        sources << layer->profileSource();
      }
    }
    else if ( QgsLayerTree::isCustomNode( node ) && node->customProperty( u"source"_s ) == QgsElevationProfileLayerTreeView::CUSTOM_NODE_ELEVATION_PROFILE_SOURCE )
    {
      QgsLayerTreeCustomNode *customNode = QgsLayerTree::toCustomNode( node );
      if ( mLayerTree->findCustomNode( customNode->nodeId() )->isVisible() )
      {
        if ( QgsAbstractProfileSource *customSource = QgsApplication::profileSourceRegistry()->findSourceById( customNode->nodeId() ) )
        {
          sources << customSource;
        }
      }
    }
  }

  // Legacy: layer tree layers are in opposite direction to what the elevation profile requires
  std::reverse( layers.begin(), layers.end() );
  mProfile->setLayers( layers );

  mProfile->setSources( sources );
  mProfile->update();
}

void QgsLayoutElevationProfileWidget::layoutAtlasToggled( bool atlasEnabled )
{
  if ( atlasEnabled && mProfile && mProfile->layout() && mProfile->layout()->reportContext().layer()
       && mProfile->layout()->reportContext().layer()->geometryType() == Qgis::GeometryType::Line )
  {
    mCheckControlledByAtlas->setEnabled( true );
  }
  else
  {
    mCheckControlledByAtlas->setEnabled( false );
    mCheckControlledByAtlas->setChecked( false );
  }
}

void QgsLayoutElevationProfileWidget::atlasLayerChanged( QgsVectorLayer *layer )
{
  if ( !layer || layer->geometryType() != Qgis::GeometryType::Line )
  {
    //non-line layer, disable atlas control
    mCheckControlledByAtlas->setChecked( false );
    mCheckControlledByAtlas->setEnabled( false );
    return;
  }
  else
  {
    mCheckControlledByAtlas->setEnabled( true );
  }
}
