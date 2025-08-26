/***************************************************************************
    qgsplotwidget.cpp
    -----------------
    begin                : August 2025
    copyright            : (C) 2025 by Mathieu Pellerin
    email                : mathieu dot opengis dot ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsplotwidget.h"
#include "moc_qgsplotwidget.cpp"
#include "qgsapplication.h"
#include "qgsfillsymbol.h"
#include "qgslinechartplot.h"
#include "qgslinesymbol.h"
#include "qgsplotregistry.h"
#include "qgsbarchartplot.h"
#include "qgsnumericformatselectorwidget.h"


QgsBarChartPlotWidget::QgsBarChartPlotWidget( QWidget *parent )
  : QgsPlotWidget( parent )
{
  setupUi( this );

  setPanelTitle( tr( "Bar Chart Plot Properties" ) );

  mSpinMinXAxis->setClearValue( 0 );
  connect( mSpinMinXAxis, qOverload<double>( &QDoubleSpinBox::valueChanged ), this, [this]( double ) {
    if ( mBlockChanges )
      return;
    emit widgetChanged();
  } );

  connect( mSpinMaxXAxis, qOverload<double>( &QDoubleSpinBox::valueChanged ), this, [this]( double ) {
    if ( mBlockChanges )
      return;
    emit widgetChanged();
  } );

  mSpinMinYAxis->setClearValue( 0 );
  connect( mSpinMinYAxis, qOverload<double>( &QDoubleSpinBox::valueChanged ), this, [this]( double ) {
    if ( mBlockChanges )
      return;
    emit widgetChanged();
  } );

  mSpinMaxYAxis->setClearValue( 0 );
  connect( mSpinMaxYAxis, qOverload<double>( &QDoubleSpinBox::valueChanged ), this, [this]( double ) {
    if ( mBlockChanges )
      return;
    emit widgetChanged();
  } );

  mXAxisMajorLinesSymbolButton->setSymbolType( Qgis::SymbolType::Line );
  connect( mXAxisMajorLinesSymbolButton, &QgsSymbolButton::changed, this, [this] {
    if ( mBlockChanges )
      return;
    emit widgetChanged();
  } );
  mXAxisMajorLinesSymbolButton->setDefaultSymbol( QgsPlotDefaultSettings::axisGridMajorSymbol() );

  mXAxisMinorLinesSymbolButton->setSymbolType( Qgis::SymbolType::Line );
  connect( mXAxisMinorLinesSymbolButton, &QgsSymbolButton::changed, this, [this] {
    if ( mBlockChanges )
      return;
    emit widgetChanged();
  } );
  mXAxisMinorLinesSymbolButton->setDefaultSymbol( QgsPlotDefaultSettings::axisGridMinorSymbol() );

  connect( mXAxisMajorIntervalSpin, qOverload<double>( &QDoubleSpinBox::valueChanged ), this, [this]( double ) {
    if ( mBlockChanges )
      return;
    emit widgetChanged();
  } );

  connect( mXAxisMinorIntervalSpin, qOverload<double>( &QDoubleSpinBox::valueChanged ), this, [this]( double ) {
    if ( mBlockChanges )
      return;
    emit widgetChanged();
  } );

  connect( mXAxisLabelIntervalSpin, qOverload<double>( &QDoubleSpinBox::valueChanged ), this, [this]( double ) {
    if ( mBlockChanges )
      return;
    emit widgetChanged();
  } );

  mYAxisMajorLinesSymbolButton->setSymbolType( Qgis::SymbolType::Line );
  connect( mYAxisMajorLinesSymbolButton, &QgsSymbolButton::changed, this, [this] {
    if ( mBlockChanges )
      return;
    emit widgetChanged();
  } );
  mYAxisMajorLinesSymbolButton->setDefaultSymbol( QgsPlotDefaultSettings::axisGridMajorSymbol() );

  mYAxisMinorLinesSymbolButton->setSymbolType( Qgis::SymbolType::Line );
  connect( mYAxisMinorLinesSymbolButton, &QgsSymbolButton::changed, this, [this] {
    if ( mBlockChanges )
      return;
    emit widgetChanged();
  } );
  mYAxisMinorLinesSymbolButton->setDefaultSymbol( QgsPlotDefaultSettings::axisGridMinorSymbol() );

  connect( mYAxisLabelIntervalSpin, qOverload<double>( &QDoubleSpinBox::valueChanged ), this, [this]( double ) {
    if ( mBlockChanges )
      return;
    emit widgetChanged();
  } );

  connect( mYAxisMajorIntervalSpin, qOverload<double>( &QDoubleSpinBox::valueChanged ), this, [this]( double ) {
    if ( mBlockChanges )
      return;
    emit widgetChanged();
  } );

  connect( mYAxisMinorIntervalSpin, qOverload<double>( &QDoubleSpinBox::valueChanged ), this, [this]( double ) {
    if ( mBlockChanges )
      return;
    emit widgetChanged();
  } );

  mChartBackgroundSymbolButton->setSymbolType( Qgis::SymbolType::Fill );
  connect( mChartBackgroundSymbolButton, &QgsSymbolButton::changed, this, [this] {
    if ( mBlockChanges )
      return;
    emit widgetChanged();
  } );
  mChartBackgroundSymbolButton->setDefaultSymbol( QgsPlotDefaultSettings::chartBackgroundSymbol() );

  mChartBorderSymbolButton->setSymbolType( Qgis::SymbolType::Fill );
  connect( mChartBorderSymbolButton, &QgsSymbolButton::changed, this, [this] {
    if ( mBlockChanges )
      return;
    emit widgetChanged();
  } );
  mChartBorderSymbolButton->setDefaultSymbol( QgsPlotDefaultSettings::chartBorderSymbol() );

  connect( mXAxisLabelFormatButton, &QPushButton::clicked, this, [this] {
    QgsNumericFormatSelectorWidget *widget = new QgsNumericFormatSelectorWidget( this );
    widget->setPanelTitle( tr( "X Axis Number Format" ) );
    widget->setFormat( mXAxisNumericFormat.get() );
    connect( widget, &QgsNumericFormatSelectorWidget::changed, this, [this, widget] {
      mXAxisNumericFormat.reset( widget->format() );
      emit widgetChanged();
    } );
    openPanel( widget );
  } );

  connect( mYAxisLabelFormatButton, &QPushButton::clicked, this, [this] {
    QgsNumericFormatSelectorWidget *widget = new QgsNumericFormatSelectorWidget( this );
    widget->setPanelTitle( tr( "Y Axis Number Format" ) );
    widget->setFormat( mYAxisNumericFormat.get() );
    connect( widget, &QgsNumericFormatSelectorWidget::changed, this, [this, widget] {
      mYAxisNumericFormat.reset( widget->format() );
      emit widgetChanged();
    } );
    openPanel( widget );
  } );

  mXAxisLabelFontButton->setDialogTitle( tr( "X Axis Label Font" ) );
  mYAxisLabelFontButton->setDialogTitle( tr( "Y Axis Label Font" ) );
  mXAxisLabelFontButton->setMode( QgsFontButton::ModeTextRenderer );
  mYAxisLabelFontButton->setMode( QgsFontButton::ModeTextRenderer );

  connect( mXAxisLabelFontButton, &QgsFontButton::changed, this, [this] {
    if ( mBlockChanges )
      return;
    emit widgetChanged();
  } );

  connect( mYAxisLabelFontButton, &QgsFontButton::changed, this, [this] {
    if ( mBlockChanges )
      return;
    emit widgetChanged();
  } );

  mSpinLeftMargin->setClearValue( 0 );
  connect( mSpinLeftMargin, qOverload<double>( &QDoubleSpinBox::valueChanged ), this, [this]( double ) {
    if ( mBlockChanges )
      return;
    emit widgetChanged();
  } );

  mSpinRightMargin->setClearValue( 0 );
  connect( mSpinRightMargin, qOverload<double>( &QDoubleSpinBox::valueChanged ), this, [this]( double ) {
    if ( mBlockChanges )
      return;
    emit widgetChanged();
  } );

  mSpinTopMargin->setClearValue( 0 );
  connect( mSpinTopMargin, qOverload<double>( &QDoubleSpinBox::valueChanged ), this, [this]( double ) {
    if ( mBlockChanges )
      return;
    emit widgetChanged();
  } );

  mSpinBottomMargin->setClearValue( 0 );
  connect( mSpinBottomMargin, qOverload<double>( &QDoubleSpinBox::valueChanged ), this, [this]( double ) {
    if ( mBlockChanges )
      return;
    emit widgetChanged();
  } );

  mXAxisTypeCombo->addItem( tr( "Interval" ), QVariant::fromValue( Qgis::PlotAxisType::Interval ) );
  mXAxisTypeCombo->addItem( tr( "Categorical" ), QVariant::fromValue( Qgis::PlotAxisType::Categorical ) );
  connect( mXAxisTypeCombo, qOverload<int>( &QComboBox::currentIndexChanged ), this, [this]( int ) {
    if ( mBlockChanges )
      return;
    emit widgetChanged();
  } );

  mXAxisLabelsCombo->addItem( tr( "None" ), QVariant::fromValue( Qgis::PlotAxisSuffixPlacement::NoLabels ) );
  mXAxisLabelsCombo->addItem( tr( "Every Value" ), QVariant::fromValue( Qgis::PlotAxisSuffixPlacement::EveryLabel ) );
  mXAxisLabelsCombo->addItem( tr( "First Value" ), QVariant::fromValue( Qgis::PlotAxisSuffixPlacement::FirstLabel ) );
  mXAxisLabelsCombo->addItem( tr( "Last Value" ), QVariant::fromValue( Qgis::PlotAxisSuffixPlacement::LastLabel ) );
  mXAxisLabelsCombo->addItem( tr( "First and Last Values" ), QVariant::fromValue( Qgis::PlotAxisSuffixPlacement::FirstAndLastLabels ) );
  connect( mXAxisLabelsCombo, qOverload<int>( &QComboBox::currentIndexChanged ), this, [this]( int ) {
    if ( mBlockChanges )
      return;
    emit widgetChanged();
  } );
}

void QgsBarChartPlotWidget::setPlot( QgsPlot *plot )
{
  QgsBarChartPlot *chartPlot = dynamic_cast<QgsBarChartPlot *>( plot );
  if ( !chartPlot )
  {
    return;
  }

  mBlockChanges++;

  mSpinMinXAxis->setValue( chartPlot->xMinimum() );
  mSpinMaxXAxis->setValue( chartPlot->xMaximum() );
  mSpinMinYAxis->setValue( chartPlot->yMinimum() );
  mSpinMaxYAxis->setValue( chartPlot->yMaximum() );

  if ( chartPlot->xAxis().gridMajorSymbol() )
    mXAxisMajorLinesSymbolButton->setSymbol( chartPlot->xAxis().gridMajorSymbol()->clone() );
  if ( chartPlot->xAxis().gridMinorSymbol() )
    mXAxisMinorLinesSymbolButton->setSymbol( chartPlot->xAxis().gridMinorSymbol()->clone() );
  if ( chartPlot->yAxis().gridMajorSymbol() )
    mYAxisMajorLinesSymbolButton->setSymbol( chartPlot->yAxis().gridMajorSymbol()->clone() );
  if ( chartPlot->yAxis().gridMinorSymbol() )
    mYAxisMinorLinesSymbolButton->setSymbol( chartPlot->yAxis().gridMinorSymbol()->clone() );

  mXAxisLabelFontButton->setTextFormat( chartPlot->xAxis().textFormat() );
  mYAxisLabelFontButton->setTextFormat( chartPlot->yAxis().textFormat() );

  mXAxisTypeCombo->setCurrentIndex( mXAxisTypeCombo->findData( QVariant::fromValue( chartPlot->xAxis().type() ) ) );
  mXAxisLabelsCombo->setCurrentIndex( mXAxisLabelsCombo->findData( QVariant::fromValue( chartPlot->xAxis().labelSuffixPlacement() ) ) );

  mXAxisMajorIntervalSpin->setValue( chartPlot->xAxis().gridIntervalMajor() );
  mXAxisMinorIntervalSpin->setValue( chartPlot->xAxis().gridIntervalMinor() );
  mXAxisLabelIntervalSpin->setValue( chartPlot->xAxis().labelInterval() );

  mYAxisMajorIntervalSpin->setValue( chartPlot->yAxis().gridIntervalMajor() );
  mYAxisMinorIntervalSpin->setValue( chartPlot->yAxis().gridIntervalMinor() );
  mYAxisLabelIntervalSpin->setValue( chartPlot->yAxis().labelInterval() );

  if ( chartPlot->chartBackgroundSymbol() )
    mChartBackgroundSymbolButton->setSymbol( chartPlot->chartBackgroundSymbol()->clone() );
  if ( chartPlot->chartBorderSymbol() )
    mChartBorderSymbolButton->setSymbol( chartPlot->chartBorderSymbol()->clone() );

  mSpinLeftMargin->setValue( chartPlot->margins().left() );
  mSpinRightMargin->setValue( chartPlot->margins().right() );
  mSpinTopMargin->setValue( chartPlot->margins().top() );
  mSpinBottomMargin->setValue( chartPlot->margins().bottom() );

  mBlockChanges--;
}


QgsPlot *QgsBarChartPlotWidget::plot()
{
  QgsPlot *plot = QgsApplication::plotRegistry()->createPlot( QStringLiteral( "bar" ) );
  QgsBarChartPlot *chartPlot = dynamic_cast<QgsBarChartPlot *>( plot );
  if ( !chartPlot )
  {
    return nullptr;
  }

  chartPlot->setXMinimum( mSpinMinXAxis->value() );
  chartPlot->setXMaximum( mSpinMaxXAxis->value() );
  chartPlot->setYMinimum( mSpinMinYAxis->value() );
  chartPlot->setYMaximum( mSpinMaxYAxis->value() );

  chartPlot->xAxis().setGridMajorSymbol( mXAxisMajorLinesSymbolButton->clonedSymbol<QgsLineSymbol>() );
  chartPlot->xAxis().setGridMajorSymbol( mXAxisMinorLinesSymbolButton->clonedSymbol<QgsLineSymbol>() );
  chartPlot->yAxis().setGridMajorSymbol( mYAxisMajorLinesSymbolButton->clonedSymbol<QgsLineSymbol>() );
  chartPlot->yAxis().setGridMajorSymbol( mYAxisMinorLinesSymbolButton->clonedSymbol<QgsLineSymbol>() );

  chartPlot->xAxis().setTextFormat( mXAxisLabelFontButton->textFormat() );
  chartPlot->yAxis().setTextFormat( mYAxisLabelFontButton->textFormat() );

  chartPlot->xAxis().setType( mXAxisTypeCombo->currentData().value<Qgis::PlotAxisType>() );
  chartPlot->xAxis().setLabelSuffixPlacement( mXAxisLabelsCombo->currentData().value<Qgis::PlotAxisSuffixPlacement>() );

  chartPlot->xAxis().setGridIntervalMajor( mXAxisMajorIntervalSpin->value() );
  chartPlot->xAxis().setGridIntervalMinor( mXAxisMinorIntervalSpin->value() );
  chartPlot->xAxis().setLabelInterval( mXAxisLabelIntervalSpin->value() );

  chartPlot->yAxis().setGridIntervalMajor( mYAxisMajorIntervalSpin->value() );
  chartPlot->yAxis().setGridIntervalMinor( mYAxisMinorIntervalSpin->value() );
  chartPlot->yAxis().setLabelInterval( mYAxisLabelIntervalSpin->value() );

  chartPlot->setChartBackgroundSymbol( mChartBackgroundSymbolButton->clonedSymbol<QgsFillSymbol>() );
  chartPlot->setChartBorderSymbol( mChartBorderSymbolButton->clonedSymbol<QgsFillSymbol>() );

  QgsMargins margins;

  margins.setLeft( mSpinLeftMargin->value() );
  margins.setRight( mSpinRightMargin->value() );
  margins.setTop( mSpinTopMargin->value() );
  margins.setBottom( mSpinBottomMargin->value() );
  chartPlot->setMargins( margins );

  return plot;
}


QgsLineChartPlotWidget::QgsLineChartPlotWidget( QWidget *parent )
  : QgsPlotWidget( parent )
{
  setupUi( this );

  setPanelTitle( tr( "Line Chart Plot Properties" ) );

  mSpinMinXAxis->setClearValue( 0 );
  connect( mSpinMinXAxis, qOverload<double>( &QDoubleSpinBox::valueChanged ), this, [this]( double ) {
    if ( mBlockChanges )
      return;
    emit widgetChanged();
  } );

  connect( mSpinMaxXAxis, qOverload<double>( &QDoubleSpinBox::valueChanged ), this, [this]( double ) {
    if ( mBlockChanges )
      return;
    emit widgetChanged();
  } );

  mSpinMinYAxis->setClearValue( 0 );
  connect( mSpinMinYAxis, qOverload<double>( &QDoubleSpinBox::valueChanged ), this, [this]( double ) {
    if ( mBlockChanges )
      return;
    emit widgetChanged();
  } );

  mSpinMaxYAxis->setClearValue( 0 );
  connect( mSpinMaxYAxis, qOverload<double>( &QDoubleSpinBox::valueChanged ), this, [this]( double ) {
    if ( mBlockChanges )
      return;
    emit widgetChanged();
  } );

  mXAxisMajorLinesSymbolButton->setSymbolType( Qgis::SymbolType::Line );
  connect( mXAxisMajorLinesSymbolButton, &QgsSymbolButton::changed, this, [this] {
    if ( mBlockChanges )
      return;
    emit widgetChanged();
  } );
  mXAxisMajorLinesSymbolButton->setDefaultSymbol( QgsPlotDefaultSettings::axisGridMajorSymbol() );

  mXAxisMinorLinesSymbolButton->setSymbolType( Qgis::SymbolType::Line );
  connect( mXAxisMinorLinesSymbolButton, &QgsSymbolButton::changed, this, [this] {
    if ( mBlockChanges )
      return;
    emit widgetChanged();
  } );
  mXAxisMinorLinesSymbolButton->setDefaultSymbol( QgsPlotDefaultSettings::axisGridMinorSymbol() );

  connect( mXAxisMajorIntervalSpin, qOverload<double>( &QDoubleSpinBox::valueChanged ), this, [this]( double ) {
    if ( mBlockChanges )
      return;
    emit widgetChanged();
  } );

  connect( mXAxisMinorIntervalSpin, qOverload<double>( &QDoubleSpinBox::valueChanged ), this, [this]( double ) {
    if ( mBlockChanges )
      return;
    emit widgetChanged();
  } );

  connect( mXAxisLabelIntervalSpin, qOverload<double>( &QDoubleSpinBox::valueChanged ), this, [this]( double ) {
    if ( mBlockChanges )
      return;
    emit widgetChanged();
  } );

  mYAxisMajorLinesSymbolButton->setSymbolType( Qgis::SymbolType::Line );
  connect( mYAxisMajorLinesSymbolButton, &QgsSymbolButton::changed, this, [this] {
    if ( mBlockChanges )
      return;
    emit widgetChanged();
  } );
  mYAxisMajorLinesSymbolButton->setDefaultSymbol( QgsPlotDefaultSettings::axisGridMajorSymbol() );

  mYAxisMinorLinesSymbolButton->setSymbolType( Qgis::SymbolType::Line );
  connect( mYAxisMinorLinesSymbolButton, &QgsSymbolButton::changed, this, [this] {
    if ( mBlockChanges )
      return;
    emit widgetChanged();
  } );
  mYAxisMinorLinesSymbolButton->setDefaultSymbol( QgsPlotDefaultSettings::axisGridMinorSymbol() );

  connect( mYAxisLabelIntervalSpin, qOverload<double>( &QDoubleSpinBox::valueChanged ), this, [this]( double ) {
    if ( mBlockChanges )
      return;
    emit widgetChanged();
  } );

  connect( mYAxisMajorIntervalSpin, qOverload<double>( &QDoubleSpinBox::valueChanged ), this, [this]( double ) {
    if ( mBlockChanges )
      return;
    emit widgetChanged();
  } );

  connect( mYAxisMinorIntervalSpin, qOverload<double>( &QDoubleSpinBox::valueChanged ), this, [this]( double ) {
    if ( mBlockChanges )
      return;
    emit widgetChanged();
  } );

  mChartBackgroundSymbolButton->setSymbolType( Qgis::SymbolType::Fill );
  connect( mChartBackgroundSymbolButton, &QgsSymbolButton::changed, this, [this] {
    if ( mBlockChanges )
      return;
    emit widgetChanged();
  } );
  mChartBackgroundSymbolButton->setDefaultSymbol( QgsPlotDefaultSettings::chartBackgroundSymbol() );

  mChartBorderSymbolButton->setSymbolType( Qgis::SymbolType::Fill );
  connect( mChartBorderSymbolButton, &QgsSymbolButton::changed, this, [this] {
    if ( mBlockChanges )
      return;
    emit widgetChanged();
  } );
  mChartBorderSymbolButton->setDefaultSymbol( QgsPlotDefaultSettings::chartBorderSymbol() );

  connect( mXAxisLabelFormatButton, &QPushButton::clicked, this, [this] {
    QgsNumericFormatSelectorWidget *widget = new QgsNumericFormatSelectorWidget( this );
    widget->setPanelTitle( tr( "X Axis Number Format" ) );
    widget->setFormat( mXAxisNumericFormat.get() );
    connect( widget, &QgsNumericFormatSelectorWidget::changed, this, [this, widget] {
      mXAxisNumericFormat.reset( widget->format() );
      emit widgetChanged();
    } );
    openPanel( widget );
  } );

  connect( mYAxisLabelFormatButton, &QPushButton::clicked, this, [this] {
    QgsNumericFormatSelectorWidget *widget = new QgsNumericFormatSelectorWidget( this );
    widget->setPanelTitle( tr( "Y Axis Number Format" ) );
    widget->setFormat( mYAxisNumericFormat.get() );
    connect( widget, &QgsNumericFormatSelectorWidget::changed, this, [this, widget] {
      mYAxisNumericFormat.reset( widget->format() );
      emit widgetChanged();
    } );
    openPanel( widget );
  } );

  mXAxisLabelFontButton->setDialogTitle( tr( "X Axis Label Font" ) );
  mYAxisLabelFontButton->setDialogTitle( tr( "Y Axis Label Font" ) );
  mXAxisLabelFontButton->setMode( QgsFontButton::ModeTextRenderer );
  mYAxisLabelFontButton->setMode( QgsFontButton::ModeTextRenderer );

  connect( mXAxisLabelFontButton, &QgsFontButton::changed, this, [this] {
    if ( mBlockChanges )
      return;
    emit widgetChanged();
  } );

  connect( mYAxisLabelFontButton, &QgsFontButton::changed, this, [this] {
    if ( mBlockChanges )
      return;
    emit widgetChanged();
  } );

  mSpinLeftMargin->setClearValue( 0 );
  connect( mSpinLeftMargin, qOverload<double>( &QDoubleSpinBox::valueChanged ), this, [this]( double ) {
    if ( mBlockChanges )
      return;
    emit widgetChanged();
  } );

  mSpinRightMargin->setClearValue( 0 );
  connect( mSpinRightMargin, qOverload<double>( &QDoubleSpinBox::valueChanged ), this, [this]( double ) {
    if ( mBlockChanges )
      return;
    emit widgetChanged();
  } );

  mSpinTopMargin->setClearValue( 0 );
  connect( mSpinTopMargin, qOverload<double>( &QDoubleSpinBox::valueChanged ), this, [this]( double ) {
    if ( mBlockChanges )
      return;
    emit widgetChanged();
  } );

  mSpinBottomMargin->setClearValue( 0 );
  connect( mSpinBottomMargin, qOverload<double>( &QDoubleSpinBox::valueChanged ), this, [this]( double ) {
    if ( mBlockChanges )
      return;
    emit widgetChanged();
  } );

  mXAxisTypeCombo->addItem( tr( "Interval" ), QVariant::fromValue( Qgis::PlotAxisType::Interval ) );
  mXAxisTypeCombo->addItem( tr( "Categorical" ), QVariant::fromValue( Qgis::PlotAxisType::Categorical ) );
  connect( mXAxisTypeCombo, qOverload<int>( &QComboBox::currentIndexChanged ), this, [this]( int ) {
    if ( mBlockChanges )
      return;
    emit widgetChanged();
  } );

  mXAxisLabelsCombo->addItem( tr( "None" ), QVariant::fromValue( Qgis::PlotAxisSuffixPlacement::NoLabels ) );
  mXAxisLabelsCombo->addItem( tr( "Every Value" ), QVariant::fromValue( Qgis::PlotAxisSuffixPlacement::EveryLabel ) );
  mXAxisLabelsCombo->addItem( tr( "First Value" ), QVariant::fromValue( Qgis::PlotAxisSuffixPlacement::FirstLabel ) );
  mXAxisLabelsCombo->addItem( tr( "Last Value" ), QVariant::fromValue( Qgis::PlotAxisSuffixPlacement::LastLabel ) );
  mXAxisLabelsCombo->addItem( tr( "First and Last Values" ), QVariant::fromValue( Qgis::PlotAxisSuffixPlacement::FirstAndLastLabels ) );
  connect( mXAxisLabelsCombo, qOverload<int>( &QComboBox::currentIndexChanged ), this, [this]( int ) {
    if ( mBlockChanges )
      return;
    emit widgetChanged();
  } );
}

void QgsLineChartPlotWidget::setPlot( QgsPlot *plot )
{
  QgsLineChartPlot *chartPlot = dynamic_cast<QgsLineChartPlot *>( plot );
  if ( !chartPlot )
  {
    return;
  }

  mBlockChanges++;

  mSpinMinXAxis->setValue( chartPlot->xMinimum() );
  mSpinMaxXAxis->setValue( chartPlot->xMaximum() );
  mSpinMinYAxis->setValue( chartPlot->yMinimum() );
  mSpinMaxYAxis->setValue( chartPlot->yMaximum() );

  if ( chartPlot->xAxis().gridMajorSymbol() )
    mXAxisMajorLinesSymbolButton->setSymbol( chartPlot->xAxis().gridMajorSymbol()->clone() );
  if ( chartPlot->xAxis().gridMinorSymbol() )
    mXAxisMinorLinesSymbolButton->setSymbol( chartPlot->xAxis().gridMinorSymbol()->clone() );
  if ( chartPlot->yAxis().gridMajorSymbol() )
    mYAxisMajorLinesSymbolButton->setSymbol( chartPlot->yAxis().gridMajorSymbol()->clone() );
  if ( chartPlot->yAxis().gridMinorSymbol() )
    mYAxisMinorLinesSymbolButton->setSymbol( chartPlot->yAxis().gridMinorSymbol()->clone() );

  mXAxisLabelFontButton->setTextFormat( chartPlot->xAxis().textFormat() );
  mYAxisLabelFontButton->setTextFormat( chartPlot->yAxis().textFormat() );

  mXAxisTypeCombo->setCurrentIndex( mXAxisTypeCombo->findData( QVariant::fromValue( chartPlot->xAxis().type() ) ) );
  mXAxisLabelsCombo->setCurrentIndex( mXAxisLabelsCombo->findData( QVariant::fromValue( chartPlot->xAxis().labelSuffixPlacement() ) ) );

  mXAxisMajorIntervalSpin->setValue( chartPlot->xAxis().gridIntervalMajor() );
  mXAxisMinorIntervalSpin->setValue( chartPlot->xAxis().gridIntervalMinor() );
  mXAxisLabelIntervalSpin->setValue( chartPlot->xAxis().labelInterval() );

  mYAxisMajorIntervalSpin->setValue( chartPlot->yAxis().gridIntervalMajor() );
  mYAxisMinorIntervalSpin->setValue( chartPlot->yAxis().gridIntervalMinor() );
  mYAxisLabelIntervalSpin->setValue( chartPlot->yAxis().labelInterval() );

  if ( chartPlot->chartBackgroundSymbol() )
    mChartBackgroundSymbolButton->setSymbol( chartPlot->chartBackgroundSymbol()->clone() );
  if ( chartPlot->chartBorderSymbol() )
    mChartBorderSymbolButton->setSymbol( chartPlot->chartBorderSymbol()->clone() );

  mSpinLeftMargin->setValue( chartPlot->margins().left() );
  mSpinRightMargin->setValue( chartPlot->margins().right() );
  mSpinTopMargin->setValue( chartPlot->margins().top() );
  mSpinBottomMargin->setValue( chartPlot->margins().bottom() );

  mBlockChanges--;
}


QgsPlot *QgsLineChartPlotWidget::plot()
{
  QgsPlot *plot = QgsApplication::plotRegistry()->createPlot( QStringLiteral( "line" ) );
  QgsLineChartPlot *chartPlot = dynamic_cast<QgsLineChartPlot *>( plot );
  if ( !chartPlot )
  {
    return nullptr;
  }

  chartPlot->setXMinimum( mSpinMinXAxis->value() );
  chartPlot->setXMaximum( mSpinMaxXAxis->value() );
  chartPlot->setYMinimum( mSpinMinYAxis->value() );
  chartPlot->setYMaximum( mSpinMaxYAxis->value() );

  chartPlot->xAxis().setGridMajorSymbol( mXAxisMajorLinesSymbolButton->clonedSymbol<QgsLineSymbol>() );
  chartPlot->xAxis().setGridMajorSymbol( mXAxisMinorLinesSymbolButton->clonedSymbol<QgsLineSymbol>() );
  chartPlot->yAxis().setGridMajorSymbol( mYAxisMajorLinesSymbolButton->clonedSymbol<QgsLineSymbol>() );
  chartPlot->yAxis().setGridMajorSymbol( mYAxisMinorLinesSymbolButton->clonedSymbol<QgsLineSymbol>() );

  chartPlot->xAxis().setTextFormat( mXAxisLabelFontButton->textFormat() );
  chartPlot->yAxis().setTextFormat( mYAxisLabelFontButton->textFormat() );

  chartPlot->xAxis().setType( mXAxisTypeCombo->currentData().value<Qgis::PlotAxisType>() );
  chartPlot->xAxis().setLabelSuffixPlacement( mXAxisLabelsCombo->currentData().value<Qgis::PlotAxisSuffixPlacement>() );

  chartPlot->xAxis().setGridIntervalMajor( mXAxisMajorIntervalSpin->value() );
  chartPlot->xAxis().setGridIntervalMinor( mXAxisMinorIntervalSpin->value() );
  chartPlot->xAxis().setLabelInterval( mXAxisLabelIntervalSpin->value() );

  chartPlot->yAxis().setGridIntervalMajor( mYAxisMajorIntervalSpin->value() );
  chartPlot->yAxis().setGridIntervalMinor( mYAxisMinorIntervalSpin->value() );
  chartPlot->yAxis().setLabelInterval( mYAxisLabelIntervalSpin->value() );

  chartPlot->setChartBackgroundSymbol( mChartBackgroundSymbolButton->clonedSymbol<QgsFillSymbol>() );
  chartPlot->setChartBorderSymbol( mChartBorderSymbolButton->clonedSymbol<QgsFillSymbol>() );

  QgsMargins margins;

  margins.setLeft( mSpinLeftMargin->value() );
  margins.setRight( mSpinRightMargin->value() );
  margins.setTop( mSpinTopMargin->value() );
  margins.setBottom( mSpinBottomMargin->value() );
  chartPlot->setMargins( margins );

  return plot;
}
