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

#include "qgsapplication.h"
#include "qgsbarchartplot.h"
#include "qgscolorrampbutton.h"
#include "qgsexpressioncontextutils.h"
#include "qgsfillsymbol.h"
#include "qgslinechartplot.h"
#include "qgslinesymbol.h"
#include "qgsnumericformatselectorwidget.h"
#include "qgspiechartplot.h"
#include "qgsplotregistry.h"

#include "moc_qgsplotwidget.cpp"

void QgsPlotWidget::registerExpressionContextGenerator( QgsExpressionContextGenerator *generator )
{
  mExpressionContextGenerator = generator;
}

QgsExpressionContext QgsPlotWidget::createExpressionContext() const
{
  QgsExpressionContext context;
  if ( mExpressionContextGenerator )
  {
    context = mExpressionContextGenerator->createExpressionContext();
  }
  else
  {
    context.appendScope( QgsExpressionContextUtils::globalScope() );
  }

  auto plotScope = std::make_unique<QgsExpressionContextScope>( u"plot"_s );
  plotScope->addVariable( QgsExpressionContextScope::StaticVariable( u"plot_axis"_s, QString(), true ) );
  plotScope->addVariable( QgsExpressionContextScope::StaticVariable( u"plot_axis_value"_s, 0.0, true ) );
  context.appendScope( plotScope.release() );

  auto chartScope = std::make_unique<QgsExpressionContextScope>( u"chart"_s );
  chartScope->addVariable( QgsExpressionContextScope::StaticVariable( u"chart_category"_s, QString(), true ) );
  chartScope->addVariable( QgsExpressionContextScope::StaticVariable( u"chart_value"_s, 0.0, true ) );
  context.appendScope( chartScope.release() );

  context.setHighlightedVariables( { u"plot_axis"_s, u"plot_axis_value"_s, u"chart_category"_s, u"chart_value"_s } );

  return context;
}

void QgsPlotWidget::initializeDataDefinedButton( QgsPropertyOverrideButton *button, QgsPlot::DataDefinedProperty key )
{
  button->blockSignals( true );
  button->init( static_cast< int >( key ), mPropertyCollection, QgsPlot::propertyDefinitions(), nullptr );
  connect( button, &QgsPropertyOverrideButton::changed, this, &QgsPlotWidget::updateProperty );
  button->registerExpressionContextGenerator( this );
  button->blockSignals( false );
}

void QgsPlotWidget::updateDataDefinedButton( QgsPropertyOverrideButton *button )
{
  if ( !button )
  {
    return;
  }

  if ( button->propertyKey() < 0 )
  {
    return;
  }

  const QgsPlot::DataDefinedProperty key = static_cast<QgsPlot::DataDefinedProperty>( button->propertyKey() );
  whileBlocking( button )->setToProperty( mPropertyCollection.property( key ) );
}

void QgsPlotWidget::updateProperty()
{
  QgsPropertyOverrideButton *button = qobject_cast<QgsPropertyOverrideButton *>( sender() );
  const QgsPlot::DataDefinedProperty key = static_cast<QgsPlot::DataDefinedProperty>( button->propertyKey() );
  mPropertyCollection.setProperty( key, button->toProperty() );
  emit widgetChanged();
}


QgsBarChartPlotWidget::QgsBarChartPlotWidget( QWidget *parent )
  : QgsPlotWidget( parent )
{
  setupUi( this );

  setPanelTitle( tr( "Bar Chart Plot Properties" ) );

  mSymbolsList->setColumnCount( 2 );
  mSymbolsList->setSelectionBehavior( QAbstractItemView::SelectRows );
  mSymbolsList->setSelectionMode( QAbstractItemView::ExtendedSelection );
  mSymbolsList->setSortingEnabled( false );
  mSymbolsList->horizontalHeader()->setSectionResizeMode( 0, QHeaderView::Stretch );
  mSymbolsList->horizontalHeader()->setSectionResizeMode( 1, QHeaderView::Stretch );
  mSymbolsList->horizontalHeader()->hide();
  mSymbolsList->verticalHeader()->hide();

  connect( mAddSymbolPushButton, &QPushButton::clicked, this, &QgsBarChartPlotWidget::mAddSymbolPushButton_clicked );
  connect( mRemoveSymbolPushButton, &QPushButton::clicked, this, &QgsBarChartPlotWidget::mRemoveSymbolPushButton_clicked );

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

  mXAxisMajorLinesSymbolButton->registerExpressionContextGenerator( this );
  mXAxisMinorLinesSymbolButton->registerExpressionContextGenerator( this );
  mXAxisLabelFontButton->registerExpressionContextGenerator( this );
  mYAxisMajorLinesSymbolButton->registerExpressionContextGenerator( this );
  mYAxisMinorLinesSymbolButton->registerExpressionContextGenerator( this );
  mYAxisLabelFontButton->registerExpressionContextGenerator( this );
  mChartBackgroundSymbolButton->registerExpressionContextGenerator( this );
  mChartBorderSymbolButton->registerExpressionContextGenerator( this );

  initializeDataDefinedButton( mDDBtnMinXAxis, QgsPlot::DataDefinedProperty::XAxisMinimum );
  initializeDataDefinedButton( mDDBtnMaxXAxis, QgsPlot::DataDefinedProperty::XAxisMaximum );
  initializeDataDefinedButton( mDDBtnMinYAxis, QgsPlot::DataDefinedProperty::YAxisMinimum );
  initializeDataDefinedButton( mDDBtnMaxYAxis, QgsPlot::DataDefinedProperty::YAxisMaximum );

  initializeDataDefinedButton( mDDBtnXAxisMajorInterval, QgsPlot::DataDefinedProperty::XAxisMajorInterval );
  initializeDataDefinedButton( mDDBtnXAxisMinorInterval, QgsPlot::DataDefinedProperty::XAxisMinorInterval );
  initializeDataDefinedButton( mDDBtnXAxisLabelInterval, QgsPlot::DataDefinedProperty::XAxisLabelInterval );
  initializeDataDefinedButton( mDDBtnYAxisMajorInterval, QgsPlot::DataDefinedProperty::YAxisMajorInterval );
  initializeDataDefinedButton( mDDBtnYAxisMinorInterval, QgsPlot::DataDefinedProperty::YAxisMinorInterval );
  initializeDataDefinedButton( mDDBtnYAxisLabelInterval, QgsPlot::DataDefinedProperty::YAxisLabelInterval );

  initializeDataDefinedButton( mDDBtnLeftMargin, QgsPlot::DataDefinedProperty::MarginLeft );
  initializeDataDefinedButton( mDDBtnRightMargin, QgsPlot::DataDefinedProperty::MarginRight );
  initializeDataDefinedButton( mDDBtnTopMargin, QgsPlot::DataDefinedProperty::MarginTop );
  initializeDataDefinedButton( mDDBtnBottomMargin, QgsPlot::DataDefinedProperty::MarginBottom );
}

void QgsBarChartPlotWidget::mAddSymbolPushButton_clicked()
{
  const int row = mSymbolsList->rowCount();
  mSymbolsList->insertRow( row );

  QTableWidgetItem *item = new QTableWidgetItem();
  item->setData( Qt::DisplayRole, tr( "Symbol" ) );
  mSymbolsList->setItem( row, 0, item );

  QgsSymbolButton *symbolButton = new QgsSymbolButton( this );
  symbolButton->setSymbolType( Qgis::SymbolType::Fill );
  symbolButton->setShowNull( true );
  symbolButton->setSymbol( QgsPlotDefaultSettings::barChartFillSymbol() );
  symbolButton->registerExpressionContextGenerator( this );
  connect( symbolButton, &QgsSymbolButton::changed, this, [this] {
    if ( mBlockChanges )
      return;
    emit widgetChanged();
  } );
  mSymbolsList->setCellWidget( row, 1, symbolButton );

  emit widgetChanged();
}

void QgsBarChartPlotWidget::mRemoveSymbolPushButton_clicked()
{
  QTableWidgetItem *item = mSymbolsList->currentItem();
  if ( !item )
  {
    return;
  }

  mSymbolsList->removeRow( mSymbolsList->row( item ) );

  emit widgetChanged();
}

void QgsBarChartPlotWidget::setPlot( QgsPlot *plot )
{
  QgsBarChartPlot *chartPlot = dynamic_cast<QgsBarChartPlot *>( plot );
  if ( !chartPlot )
  {
    return;
  }

  mBlockChanges++;

  mSymbolsList->clear();
  const int symbolCount = chartPlot->fillSymbolCount();
  for ( int i = 0; i < symbolCount; i++ )
  {
    const int row = mSymbolsList->rowCount();
    mSymbolsList->insertRow( row );

    QTableWidgetItem *item = new QTableWidgetItem();
    item->setData( Qt::DisplayRole, tr( "Symbol" ) );
    mSymbolsList->setItem( row, 0, item );

    QgsSymbolButton *symbolButton = new QgsSymbolButton( this );
    symbolButton->setSymbolType( Qgis::SymbolType::Fill );
    symbolButton->setShowNull( true );
    symbolButton->setSymbol( chartPlot->fillSymbolAt( i )->clone() );
    symbolButton->registerExpressionContextGenerator( this );
    connect( symbolButton, &QgsSymbolButton::changed, this, [this] {
      if ( mBlockChanges )
        return;
      emit widgetChanged();
    } );
    mSymbolsList->setCellWidget( row, 1, symbolButton );
  }

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
  mXAxisNumericFormat.reset( chartPlot->xAxis().numericFormat()->clone() );
  mYAxisLabelFontButton->setTextFormat( chartPlot->yAxis().textFormat() );
  mYAxisNumericFormat.reset( chartPlot->yAxis().numericFormat()->clone() );

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

  mPropertyCollection = chartPlot->dataDefinedProperties();

  updateDataDefinedButton( mDDBtnMinXAxis );
  updateDataDefinedButton( mDDBtnMaxXAxis );
  updateDataDefinedButton( mDDBtnMinYAxis );
  updateDataDefinedButton( mDDBtnMaxYAxis );

  updateDataDefinedButton( mDDBtnXAxisMajorInterval );
  updateDataDefinedButton( mDDBtnXAxisMinorInterval );
  updateDataDefinedButton( mDDBtnXAxisLabelInterval );
  updateDataDefinedButton( mDDBtnYAxisMajorInterval );
  updateDataDefinedButton( mDDBtnYAxisMinorInterval );
  updateDataDefinedButton( mDDBtnYAxisLabelInterval );

  updateDataDefinedButton( mDDBtnLeftMargin );
  updateDataDefinedButton( mDDBtnRightMargin );
  updateDataDefinedButton( mDDBtnTopMargin );
  updateDataDefinedButton( mDDBtnBottomMargin );

  mBlockChanges--;
}


QgsPlot *QgsBarChartPlotWidget::createPlot()
{
  QgsPlot *plot = QgsApplication::plotRegistry()->createPlot( u"bar"_s );
  QgsBarChartPlot *chartPlot = dynamic_cast<QgsBarChartPlot *>( plot );
  if ( !chartPlot )
  {
    return nullptr;
  }

  const int rowCount = mSymbolsList->rowCount();
  for ( int i = 0; i < rowCount; i++ )
  {
    QgsSymbolButton *symbolButton = dynamic_cast<QgsSymbolButton *>( mSymbolsList->cellWidget( i, 1 ) );
    if ( symbolButton )
    {
      chartPlot->setFillSymbolAt( i, symbolButton->clonedSymbol<QgsFillSymbol>() );
    }
  }

  chartPlot->setXMinimum( mSpinMinXAxis->value() );
  chartPlot->setXMaximum( mSpinMaxXAxis->value() );
  chartPlot->setYMinimum( mSpinMinYAxis->value() );
  chartPlot->setYMaximum( mSpinMaxYAxis->value() );

  chartPlot->xAxis().setGridMajorSymbol( mXAxisMajorLinesSymbolButton->clonedSymbol<QgsLineSymbol>() );
  chartPlot->xAxis().setGridMinorSymbol( mXAxisMinorLinesSymbolButton->clonedSymbol<QgsLineSymbol>() );
  chartPlot->yAxis().setGridMajorSymbol( mYAxisMajorLinesSymbolButton->clonedSymbol<QgsLineSymbol>() );
  chartPlot->yAxis().setGridMinorSymbol( mYAxisMinorLinesSymbolButton->clonedSymbol<QgsLineSymbol>() );

  chartPlot->xAxis().setTextFormat( mXAxisLabelFontButton->textFormat() );
  chartPlot->xAxis().setNumericFormat( mXAxisNumericFormat.get()->clone() );
  chartPlot->yAxis().setTextFormat( mYAxisLabelFontButton->textFormat() );
  chartPlot->yAxis().setNumericFormat( mYAxisNumericFormat.get()->clone() );

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

  chartPlot->setDataDefinedProperties( mPropertyCollection );

  return plot;
}


QgsLineChartPlotWidget::QgsLineChartPlotWidget( QWidget *parent )
  : QgsPlotWidget( parent )
{
  setupUi( this );

  setPanelTitle( tr( "Line Chart Plot Properties" ) );

  mSymbolsList->setColumnCount( 3 );
  mSymbolsList->setSelectionBehavior( QAbstractItemView::SelectRows );
  mSymbolsList->setSelectionMode( QAbstractItemView::ExtendedSelection );
  mSymbolsList->setSortingEnabled( false );
  mSymbolsList->horizontalHeader()->setSectionResizeMode( 0, QHeaderView::Stretch );
  mSymbolsList->horizontalHeader()->setSectionResizeMode( 1, QHeaderView::Stretch );
  mSymbolsList->horizontalHeader()->setSectionResizeMode( 2, QHeaderView::Stretch );
  mSymbolsList->horizontalHeader()->hide();
  mSymbolsList->verticalHeader()->hide();

  connect( mAddSymbolPushButton, &QPushButton::clicked, this, &QgsLineChartPlotWidget::mAddSymbolPushButton_clicked );
  connect( mRemoveSymbolPushButton, &QPushButton::clicked, this, &QgsLineChartPlotWidget::mRemoveSymbolPushButton_clicked );

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

  mXAxisMajorLinesSymbolButton->registerExpressionContextGenerator( this );
  mXAxisMinorLinesSymbolButton->registerExpressionContextGenerator( this );
  mXAxisLabelFontButton->registerExpressionContextGenerator( this );
  mYAxisMajorLinesSymbolButton->registerExpressionContextGenerator( this );
  mYAxisMinorLinesSymbolButton->registerExpressionContextGenerator( this );
  mYAxisLabelFontButton->registerExpressionContextGenerator( this );
  mChartBackgroundSymbolButton->registerExpressionContextGenerator( this );
  mChartBorderSymbolButton->registerExpressionContextGenerator( this );

  initializeDataDefinedButton( mDDBtnMinXAxis, QgsPlot::DataDefinedProperty::XAxisMinimum );
  initializeDataDefinedButton( mDDBtnMaxXAxis, QgsPlot::DataDefinedProperty::XAxisMaximum );
  initializeDataDefinedButton( mDDBtnMinYAxis, QgsPlot::DataDefinedProperty::YAxisMinimum );
  initializeDataDefinedButton( mDDBtnMaxYAxis, QgsPlot::DataDefinedProperty::YAxisMaximum );

  initializeDataDefinedButton( mDDBtnXAxisMajorInterval, QgsPlot::DataDefinedProperty::XAxisMajorInterval );
  initializeDataDefinedButton( mDDBtnXAxisMinorInterval, QgsPlot::DataDefinedProperty::XAxisMinorInterval );
  initializeDataDefinedButton( mDDBtnXAxisLabelInterval, QgsPlot::DataDefinedProperty::XAxisLabelInterval );
  initializeDataDefinedButton( mDDBtnYAxisMajorInterval, QgsPlot::DataDefinedProperty::YAxisMajorInterval );
  initializeDataDefinedButton( mDDBtnYAxisMinorInterval, QgsPlot::DataDefinedProperty::YAxisMinorInterval );
  initializeDataDefinedButton( mDDBtnYAxisLabelInterval, QgsPlot::DataDefinedProperty::YAxisLabelInterval );

  initializeDataDefinedButton( mDDBtnLeftMargin, QgsPlot::DataDefinedProperty::MarginLeft );
  initializeDataDefinedButton( mDDBtnRightMargin, QgsPlot::DataDefinedProperty::MarginRight );
  initializeDataDefinedButton( mDDBtnTopMargin, QgsPlot::DataDefinedProperty::MarginTop );
  initializeDataDefinedButton( mDDBtnBottomMargin, QgsPlot::DataDefinedProperty::MarginBottom );
}

void QgsLineChartPlotWidget::mAddSymbolPushButton_clicked()
{
  const int row = mSymbolsList->rowCount();
  mSymbolsList->insertRow( row );

  QTableWidgetItem *item = new QTableWidgetItem();
  item->setData( Qt::DisplayRole, tr( "Symbol" ) );
  mSymbolsList->setItem( row, 0, item );

  // Line
  QgsSymbolButton *symbolButton = new QgsSymbolButton( this );
  symbolButton->setSymbolType( Qgis::SymbolType::Line );
  symbolButton->setShowNull( true );
  symbolButton->setSymbol( QgsPlotDefaultSettings::lineChartLineSymbol() );
  symbolButton->registerExpressionContextGenerator( this );
  connect( symbolButton, &QgsSymbolButton::changed, this, [this] {
    if ( mBlockChanges )
      return;
    emit widgetChanged();
  } );
  mSymbolsList->setCellWidget( row, 1, symbolButton );

  // Marker
  symbolButton = new QgsSymbolButton( this );
  symbolButton->setFixedSizeConstraints( false );
  symbolButton->setSymbolType( Qgis::SymbolType::Marker );
  symbolButton->setShowNull( true );
  symbolButton->setSymbol( QgsPlotDefaultSettings::lineChartMarkerSymbol() );
  symbolButton->registerExpressionContextGenerator( this );
  connect( symbolButton, &QgsSymbolButton::changed, this, [this] {
    if ( mBlockChanges )
      return;
    emit widgetChanged();
  } );
  mSymbolsList->setCellWidget( row, 2, symbolButton );

  emit widgetChanged();
}

void QgsLineChartPlotWidget::mRemoveSymbolPushButton_clicked()
{
  QTableWidgetItem *item = mSymbolsList->currentItem();
  if ( !item )
  {
    return;
  }

  mSymbolsList->removeRow( mSymbolsList->row( item ) );

  emit widgetChanged();
}

void QgsLineChartPlotWidget::setPlot( QgsPlot *plot )
{
  QgsLineChartPlot *chartPlot = dynamic_cast<QgsLineChartPlot *>( plot );
  if ( !chartPlot )
  {
    return;
  }

  mBlockChanges++;

  mSymbolsList->clear();
  const int symbolCount = std::max( chartPlot->markerSymbolCount(), chartPlot->lineSymbolCount() );
  for ( int i = 0; i < symbolCount; i++ )
  {
    const int row = mSymbolsList->rowCount();
    mSymbolsList->insertRow( row );

    QTableWidgetItem *item = new QTableWidgetItem();
    item->setData( Qt::DisplayRole, tr( "Symbol" ) );
    mSymbolsList->setItem( row, 0, item );

    // Line
    QgsSymbolButton *symbolButton = new QgsSymbolButton( this );
    symbolButton->setSymbolType( Qgis::SymbolType::Line );
    symbolButton->setShowNull( true );
    symbolButton->setSymbol( i < chartPlot->lineSymbolCount() ? chartPlot->lineSymbolAt( i )->clone() : nullptr );
    symbolButton->registerExpressionContextGenerator( this );
    connect( symbolButton, &QgsSymbolButton::changed, this, [this] {
      if ( mBlockChanges )
        return;
      emit widgetChanged();
    } );
    mSymbolsList->setCellWidget( row, 1, symbolButton );

    // Marker
    symbolButton = new QgsSymbolButton( this );
    symbolButton->setFixedSizeConstraints( false );
    symbolButton->setSymbolType( Qgis::SymbolType::Marker );
    symbolButton->setShowNull( true );
    symbolButton->setSymbol( i < chartPlot->markerSymbolCount() ? chartPlot->markerSymbolAt( i )->clone() : nullptr );
    symbolButton->registerExpressionContextGenerator( this );
    connect( symbolButton, &QgsSymbolButton::changed, this, [this] {
      if ( mBlockChanges )
        return;
      emit widgetChanged();
    } );
    mSymbolsList->setCellWidget( row, 2, symbolButton );
  }

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

  mPropertyCollection = chartPlot->dataDefinedProperties();

  updateDataDefinedButton( mDDBtnMinXAxis );
  updateDataDefinedButton( mDDBtnMaxXAxis );
  updateDataDefinedButton( mDDBtnMinYAxis );
  updateDataDefinedButton( mDDBtnMaxYAxis );

  updateDataDefinedButton( mDDBtnXAxisMajorInterval );
  updateDataDefinedButton( mDDBtnXAxisMinorInterval );
  updateDataDefinedButton( mDDBtnXAxisLabelInterval );
  updateDataDefinedButton( mDDBtnYAxisMajorInterval );
  updateDataDefinedButton( mDDBtnYAxisMinorInterval );
  updateDataDefinedButton( mDDBtnYAxisLabelInterval );

  updateDataDefinedButton( mDDBtnLeftMargin );
  updateDataDefinedButton( mDDBtnRightMargin );
  updateDataDefinedButton( mDDBtnTopMargin );
  updateDataDefinedButton( mDDBtnBottomMargin );

  mBlockChanges--;
}


QgsPlot *QgsLineChartPlotWidget::createPlot()
{
  QgsPlot *plot = QgsApplication::plotRegistry()->createPlot( u"line"_s );
  QgsLineChartPlot *chartPlot = dynamic_cast<QgsLineChartPlot *>( plot );
  if ( !chartPlot )
  {
    return nullptr;
  }

  const int rowCount = mSymbolsList->rowCount();
  for ( int i = 0; i < rowCount; i++ )
  {
    QgsSymbolButton *symbolButton = dynamic_cast<QgsSymbolButton *>( mSymbolsList->cellWidget( i, 1 ) );
    if ( symbolButton )
    {
      chartPlot->setLineSymbolAt( i, symbolButton->clonedSymbol<QgsLineSymbol>() );
    }

    symbolButton = dynamic_cast<QgsSymbolButton *>( mSymbolsList->cellWidget( i, 2 ) );
    if ( symbolButton )
    {
      chartPlot->setMarkerSymbolAt( i, symbolButton->clonedSymbol<QgsMarkerSymbol>() );
    }
  }

  chartPlot->setXMinimum( mSpinMinXAxis->value() );
  chartPlot->setXMaximum( mSpinMaxXAxis->value() );
  chartPlot->setYMinimum( mSpinMinYAxis->value() );
  chartPlot->setYMaximum( mSpinMaxYAxis->value() );

  chartPlot->xAxis().setGridMajorSymbol( mXAxisMajorLinesSymbolButton->clonedSymbol<QgsLineSymbol>() );
  chartPlot->xAxis().setGridMinorSymbol( mXAxisMinorLinesSymbolButton->clonedSymbol<QgsLineSymbol>() );
  chartPlot->yAxis().setGridMajorSymbol( mYAxisMajorLinesSymbolButton->clonedSymbol<QgsLineSymbol>() );
  chartPlot->yAxis().setGridMinorSymbol( mYAxisMinorLinesSymbolButton->clonedSymbol<QgsLineSymbol>() );

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

  chartPlot->setDataDefinedProperties( mPropertyCollection );

  return plot;
}


QgsPieChartPlotWidget::QgsPieChartPlotWidget( QWidget *parent )
  : QgsPlotWidget( parent )
{
  setupUi( this );

  setPanelTitle( tr( "Pie Chart Plot Properties" ) );

  mSymbolsList->setColumnCount( 3 );
  mSymbolsList->setSelectionBehavior( QAbstractItemView::SelectRows );
  mSymbolsList->setSelectionMode( QAbstractItemView::ExtendedSelection );
  mSymbolsList->setSortingEnabled( false );
  mSymbolsList->horizontalHeader()->setSectionResizeMode( 0, QHeaderView::Stretch );
  mSymbolsList->horizontalHeader()->setSectionResizeMode( 1, QHeaderView::Stretch );
  mSymbolsList->horizontalHeader()->setSectionResizeMode( 2, QHeaderView::Stretch );
  mSymbolsList->horizontalHeader()->hide();
  mSymbolsList->verticalHeader()->hide();

  mLabelCombo->addItem( tr( "None" ), QVariant::fromValue( Qgis::PieChartLabelType::NoLabels ) );
  mLabelCombo->addItem( tr( "Category Labels" ), QVariant::fromValue( Qgis::PieChartLabelType::Categories ) );
  mLabelCombo->addItem( tr( "Value Labels" ), QVariant::fromValue( Qgis::PieChartLabelType::Values ) );
  connect( mLabelCombo, qOverload<int>( &QComboBox::currentIndexChanged ), this, [this]( int ) {
    if ( mBlockChanges )
      return;
    emit widgetChanged();
  } );

  mLabelFontButton->setDialogTitle( tr( "Chart Label Font" ) );
  mLabelFontButton->setMode( QgsFontButton::ModeTextRenderer );
  connect( mLabelFontButton, &QgsFontButton::changed, this, [this] {
    if ( mBlockChanges )
      return;
    emit widgetChanged();
  } );

  connect( mLabelFormatButton, &QPushButton::clicked, this, [this] {
    QgsNumericFormatSelectorWidget *widget = new QgsNumericFormatSelectorWidget( this );
    widget->setPanelTitle( tr( "Chart Number Format" ) );
    widget->setFormat( mNumericFormat.get() );
    connect( widget, &QgsNumericFormatSelectorWidget::changed, this, [this, widget] {
      mNumericFormat.reset( widget->format() );
      emit widgetChanged();
    } );
    openPanel( widget );
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

  connect( mAddSymbolPushButton, &QPushButton::clicked, this, &QgsPieChartPlotWidget::mAddSymbolPushButton_clicked );
  connect( mRemoveSymbolPushButton, &QPushButton::clicked, this, &QgsPieChartPlotWidget::mRemoveSymbolPushButton_clicked );

  mLabelFontButton->registerExpressionContextGenerator( this );

  initializeDataDefinedButton( mDDBtnLeftMargin, QgsPlot::DataDefinedProperty::MarginLeft );
  initializeDataDefinedButton( mDDBtnRightMargin, QgsPlot::DataDefinedProperty::MarginRight );
  initializeDataDefinedButton( mDDBtnTopMargin, QgsPlot::DataDefinedProperty::MarginTop );
  initializeDataDefinedButton( mDDBtnBottomMargin, QgsPlot::DataDefinedProperty::MarginBottom );
}

void QgsPieChartPlotWidget::mAddSymbolPushButton_clicked()
{
  const int row = mSymbolsList->rowCount();
  mSymbolsList->insertRow( row );

  QTableWidgetItem *item = new QTableWidgetItem();
  item->setData( Qt::DisplayRole, tr( "Symbol" ) );
  mSymbolsList->setItem( row, 0, item );

  // Fill
  QgsSymbolButton *symbolButton = new QgsSymbolButton( this );
  symbolButton->setSymbolType( Qgis::SymbolType::Fill );
  symbolButton->setShowNull( true );
  symbolButton->setSymbol( QgsPlotDefaultSettings::pieChartFillSymbol() );
  symbolButton->registerExpressionContextGenerator( this );
  connect( symbolButton, &QgsSymbolButton::changed, this, [this] {
    if ( mBlockChanges )
      return;
    emit widgetChanged();
  } );
  mSymbolsList->setCellWidget( row, 1, symbolButton );

  // Color ramp
  QgsColorRampButton *colorRampButton = new QgsColorRampButton( this );
  colorRampButton->setShowNull( true );
  colorRampButton->setColorRamp( QgsPlotDefaultSettings::pieChartColorRamp() );
  connect( colorRampButton, &QgsColorRampButton::colorRampChanged, this, [this] {
    if ( mBlockChanges )
      return;
    emit widgetChanged();
  } );
  mSymbolsList->setCellWidget( row, 2, colorRampButton );

  emit widgetChanged();
}

void QgsPieChartPlotWidget::mRemoveSymbolPushButton_clicked()
{
  QTableWidgetItem *item = mSymbolsList->currentItem();
  if ( !item )
  {
    return;
  }

  mSymbolsList->removeRow( mSymbolsList->row( item ) );

  emit widgetChanged();
}

void QgsPieChartPlotWidget::setPlot( QgsPlot *plot )
{
  QgsPieChartPlot *chartPlot = dynamic_cast<QgsPieChartPlot *>( plot );
  if ( !chartPlot )
  {
    return;
  }

  mBlockChanges++;

  mSymbolsList->clear();
  const int symbolCount = std::max( chartPlot->fillSymbolCount(), chartPlot->colorRampCount() );
  for ( int i = 0; i < symbolCount; i++ )
  {
    const int row = mSymbolsList->rowCount();
    mSymbolsList->insertRow( row );

    QTableWidgetItem *item = new QTableWidgetItem();
    item->setData( Qt::DisplayRole, tr( "Symbol" ) );
    mSymbolsList->setItem( row, 0, item );

    // Fill
    QgsSymbolButton *symbolButton = new QgsSymbolButton( this );
    symbolButton->setSymbolType( Qgis::SymbolType::Fill );
    symbolButton->setShowNull( true );
    symbolButton->setSymbol( i < chartPlot->fillSymbolCount() ? chartPlot->fillSymbolAt( i )->clone() : nullptr );
    symbolButton->registerExpressionContextGenerator( this );
    connect( symbolButton, &QgsSymbolButton::changed, this, [this] {
      if ( mBlockChanges )
        return;
      emit widgetChanged();
    } );
    mSymbolsList->setCellWidget( row, 1, symbolButton );

    // Color ramp
    QgsColorRampButton *colorRampButton = new QgsColorRampButton( this );
    colorRampButton->setShowNull( true );
    colorRampButton->setColorRamp( i < chartPlot->colorRampCount() ? chartPlot->colorRampAt( i )->clone() : nullptr );
    connect( colorRampButton, &QgsColorRampButton::colorRampChanged, this, [this] {
      if ( mBlockChanges )
        return;
      emit widgetChanged();
    } );
    mSymbolsList->setCellWidget( row, 2, colorRampButton );
  }

  mNumericFormat.reset( chartPlot->numericFormat()->clone() );
  mLabelFontButton->setTextFormat( chartPlot->textFormat() );

  mLabelCombo->setCurrentIndex( mLabelCombo->findData( QVariant::fromValue( chartPlot->labelType() ) ) );

  mPropertyCollection = chartPlot->dataDefinedProperties();

  updateDataDefinedButton( mDDBtnLeftMargin );
  updateDataDefinedButton( mDDBtnRightMargin );
  updateDataDefinedButton( mDDBtnTopMargin );
  updateDataDefinedButton( mDDBtnBottomMargin );

  mBlockChanges--;
}

QgsPlot *QgsPieChartPlotWidget::createPlot()
{
  QgsPlot *plot = QgsApplication::plotRegistry()->createPlot( u"pie"_s );
  QgsPieChartPlot *chartPlot = dynamic_cast<QgsPieChartPlot *>( plot );
  if ( !chartPlot )
  {
    return nullptr;
  }

  const int rowCount = mSymbolsList->rowCount();
  for ( int i = 0; i < rowCount; i++ )
  {
    QgsSymbolButton *symbolButton = dynamic_cast<QgsSymbolButton *>( mSymbolsList->cellWidget( i, 1 ) );
    if ( symbolButton )
    {
      chartPlot->setFillSymbolAt( i, symbolButton->clonedSymbol<QgsFillSymbol>() );
    }

    QgsColorRampButton *colorRampButton = dynamic_cast<QgsColorRampButton *>( mSymbolsList->cellWidget( i, 2 ) );
    if ( colorRampButton )
    {
      chartPlot->setColorRampAt( i, colorRampButton->colorRamp()->clone() );
    }
  }

  chartPlot->setNumericFormat( mNumericFormat.get()->clone() );
  chartPlot->setTextFormat( mLabelFontButton->textFormat() );
  chartPlot->setLabelType( mLabelCombo->currentData().value<Qgis::PieChartLabelType>() );

  QgsMargins margins;
  margins.setLeft( mSpinLeftMargin->value() );
  margins.setRight( mSpinRightMargin->value() );
  margins.setTop( mSpinTopMargin->value() );
  margins.setBottom( mSpinBottomMargin->value() );
  chartPlot->setMargins( margins );

  chartPlot->setDataDefinedProperties( mPropertyCollection );

  return plot;
}
