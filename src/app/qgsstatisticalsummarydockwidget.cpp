/***************************************************************************
    qgsstatisticalsummarydockwidget.cpp
    -----------------------------------
    begin                : May 2015
    copyright            : (C) 2015 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "qgsstatisticalsummarydockwidget.h"
#include "qgsstatisticalsummary.h"
#include "qgsmaplayerregistry.h"
#include <QTableWidget>
#include <QAction>
#include <QSettings>

QList< QgsStatisticalSummary::Statistic > QgsStatisticalSummaryDockWidget::mDisplayStats =
  QList< QgsStatisticalSummary::Statistic > () << QgsStatisticalSummary::Count
  << QgsStatisticalSummary::Sum
  << QgsStatisticalSummary::Mean
  << QgsStatisticalSummary::Median
  << QgsStatisticalSummary::StDev
  << QgsStatisticalSummary::StDevSample
  << QgsStatisticalSummary::Min
  << QgsStatisticalSummary::Max
  << QgsStatisticalSummary::Range
  << QgsStatisticalSummary::Minority
  << QgsStatisticalSummary::Majority
  << QgsStatisticalSummary::Variety
  << QgsStatisticalSummary::FirstQuartile
  << QgsStatisticalSummary::ThirdQuartile
  << QgsStatisticalSummary::InterQuartileRange;

#define MISSING_VALUES -1

QgsStatisticalSummaryDockWidget::QgsStatisticalSummaryDockWidget( QWidget *parent )
    : QDockWidget( parent )
    , mLayer( 0 )
{
  setupUi( this );

  mLayerComboBox->setFilters( QgsMapLayerProxyModel::VectorLayer );
  mFieldExpressionWidget->setFilters( QgsFieldProxyModel::Numeric );

  mLayerComboBox->setLayer( mLayerComboBox->layer( 0 ) );
  mFieldExpressionWidget->setLayer( mLayerComboBox->layer( 0 ) );

  connect( mLayerComboBox, SIGNAL( layerChanged( QgsMapLayer* ) ), this, SLOT( layerChanged( QgsMapLayer* ) ) );
  connect( mFieldExpressionWidget, SIGNAL( fieldChanged( QString ) ), this, SLOT( refreshStatistics() ) );
  connect( mSelectedOnlyCheckBox, SIGNAL( toggled( bool ) ), this, SLOT( refreshStatistics() ) );
  connect( mButtonRefresh, SIGNAL( clicked( bool ) ), this, SLOT( refreshStatistics() ) );
  connect( QgsMapLayerRegistry::instance(), SIGNAL( layersWillBeRemoved( QStringList ) ), this, SLOT( layersRemoved( QStringList ) ) );

  QSettings settings;
  foreach ( QgsStatisticalSummary::Statistic stat, mDisplayStats )
  {
    QAction* action = new QAction( QgsStatisticalSummary::displayName( stat ), mOptionsToolButton );
    action->setCheckable( true );
    bool checked = settings.value( QString( "/StatisticalSummaryDock/checked_%1" ).arg( stat ), true ).toBool();
    action->setChecked( checked );
    action->setData( stat );
    mStatsActions.insert( stat, action );
    connect( action, SIGNAL( triggered( bool ) ), this, SLOT( statActionTriggered( bool ) ) );
    mOptionsToolButton->addAction( action );
  }

  //count of null values statistic:
  QAction* nullCountAction = new QAction( tr( "Missing (null) values" ), mOptionsToolButton );
  nullCountAction->setCheckable( true );
  bool checked = settings.value( QString( "/StatisticalSummaryDock/checked_missing_values" ), true ).toBool();
  nullCountAction->setChecked( checked );
  nullCountAction->setData( MISSING_VALUES );
  mStatsActions.insert( MISSING_VALUES, nullCountAction );
  connect( nullCountAction, SIGNAL( triggered( bool ) ), this, SLOT( statActionTriggered( bool ) ) );
  mOptionsToolButton->addAction( nullCountAction );
}

QgsStatisticalSummaryDockWidget::~QgsStatisticalSummaryDockWidget()
{

}

void QgsStatisticalSummaryDockWidget::refreshStatistics()
{
  if ( !mLayer || !mFieldExpressionWidget->isValidExpression() )
  {
    mStatisticsTable->setRowCount( 0 );
    return;
  }

  QString sourceFieldExp = mFieldExpressionWidget->currentField();

  bool ok;
  bool selectedOnly = mSelectedOnlyCheckBox->isChecked();
  int missingValues = 0;
  QList< double > values = mLayer->getDoubleValues( sourceFieldExp, ok, selectedOnly, &missingValues );

  if ( ! ok )
  {
    return;
  }

  QList< QgsStatisticalSummary::Statistic > statsToDisplay;
  QgsStatisticalSummary::Statistics statsToCalc = 0;
  foreach ( QgsStatisticalSummary::Statistic stat, mDisplayStats )
  {
    if ( mStatsActions.value( stat )->isChecked() )
    {
      statsToDisplay << stat;
      statsToCalc |= stat;
    }
  }

  int extraRows = 0;
  if ( mStatsActions.value( MISSING_VALUES )->isChecked() )
    extraRows++;

  QgsStatisticalSummary stats;
  stats.setStatistics( statsToCalc );
  stats.calculate( values );

  mStatisticsTable->setRowCount( statsToDisplay.count() + extraRows );
  mStatisticsTable->setColumnCount( 2 );

  int row = 0;
  foreach ( QgsStatisticalSummary::Statistic stat, statsToDisplay )
  {
    QTableWidgetItem* nameItem = new QTableWidgetItem( QgsStatisticalSummary::displayName( stat ) );
    nameItem->setToolTip( nameItem->text() );
    nameItem->setFlags( Qt::ItemIsSelectable | Qt::ItemIsEnabled );
    mStatisticsTable->setItem( row, 0, nameItem );

    QTableWidgetItem* valueItem = new QTableWidgetItem();
    if ( stats.count() != 0 )
    {
      valueItem->setText( QString::number( stats.statistic( stat ) ) );
    }
    valueItem->setToolTip( valueItem->text() );
    valueItem->setFlags( Qt::ItemIsSelectable | Qt::ItemIsEnabled );
    mStatisticsTable->setItem( row, 1, valueItem );

    row++;
  }

  if ( mStatsActions.value( MISSING_VALUES )->isChecked() )
  {
    QTableWidgetItem* nameItem = new QTableWidgetItem( tr( "Missing (null) values" ) );
    nameItem->setToolTip( nameItem->text() );
    nameItem->setFlags( Qt::ItemIsSelectable | Qt::ItemIsEnabled );
    mStatisticsTable->setItem( row, 0, nameItem );

    QTableWidgetItem* valueItem = new QTableWidgetItem();
    if ( stats.count() != 0 || missingValues != 0 )
    {
      valueItem->setText( QString::number( missingValues ) );
    }
    valueItem->setToolTip( valueItem->text() );
    valueItem->setFlags( Qt::ItemIsSelectable | Qt::ItemIsEnabled );
    mStatisticsTable->setItem( row, 1, valueItem );
    row++;
  }
}

void QgsStatisticalSummaryDockWidget::layerChanged( QgsMapLayer *layer )
{
  QgsVectorLayer* newLayer = dynamic_cast< QgsVectorLayer* >( layer );
  if ( mLayer && mLayer != newLayer )
  {
    disconnect( mLayer, SIGNAL( selectionChanged() ), this, SLOT( layerSelectionChanged() ) );
  }

  mLayer = newLayer;

  if ( mLayer )
  {
    connect( mLayer, SIGNAL( selectionChanged() ), this, SLOT( layerSelectionChanged() ) );
  }

  mFieldExpressionWidget->setLayer( mLayer );

  if ( mFieldExpressionWidget->currentField().isEmpty() )
  {
    mStatisticsTable->setRowCount( 0 );
  }
  else
  {
    refreshStatistics();
  }
}

void QgsStatisticalSummaryDockWidget::statActionTriggered( bool checked )
{
  refreshStatistics();
  QAction* action = dynamic_cast<QAction*>( sender() );
  int stat = action->data().toInt();

  QSettings settings;
  if ( stat >= 0 )
  {
    settings.setValue( QString( "/StatisticalSummaryDock/checked_%1" ).arg( stat ), checked );
  }
  else if ( stat == MISSING_VALUES )
  {
    settings.setValue( QString( "/StatisticalSummaryDock/checked_missing_values" ).arg( stat ), checked );
  }
}

void QgsStatisticalSummaryDockWidget::layersRemoved( QStringList layers )
{
  if ( mLayer && layers.contains( mLayer->id() ) )
  {
    disconnect( mLayer, SIGNAL( selectionChanged() ), this, SLOT( layerSelectionChanged() ) );
    mLayer = 0;
  }
}

void QgsStatisticalSummaryDockWidget::layerSelectionChanged()
{
  if ( mSelectedOnlyCheckBox->isChecked() )
    refreshStatistics();
}
