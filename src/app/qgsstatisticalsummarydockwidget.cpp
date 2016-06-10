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
#include "qgisapp.h"
#include "qgsmapcanvas.h"
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

QList< QgsStringStatisticalSummary::Statistic > QgsStatisticalSummaryDockWidget::mDisplayStringStats =
  QList< QgsStringStatisticalSummary::Statistic > () << QgsStringStatisticalSummary::Count
  << QgsStringStatisticalSummary::CountDistinct
  << QgsStringStatisticalSummary::CountMissing
  << QgsStringStatisticalSummary::Min
  << QgsStringStatisticalSummary::Max
  << QgsStringStatisticalSummary::MinimumLength
  << QgsStringStatisticalSummary::MaximumLength;

QList< QgsDateTimeStatisticalSummary::Statistic > QgsStatisticalSummaryDockWidget::mDisplayDateTimeStats =
  QList< QgsDateTimeStatisticalSummary::Statistic > () << QgsDateTimeStatisticalSummary::Count
  << QgsDateTimeStatisticalSummary::CountDistinct
  << QgsDateTimeStatisticalSummary::CountMissing
  << QgsDateTimeStatisticalSummary::Min
  << QgsDateTimeStatisticalSummary::Max
  << QgsDateTimeStatisticalSummary::Range;

#define MISSING_VALUES -1

static QgsExpressionContext _getExpressionContext( const void* context )
{
  QgsExpressionContext expContext;
  expContext << QgsExpressionContextUtils::globalScope()
  << QgsExpressionContextUtils::projectScope()
  << QgsExpressionContextUtils::mapSettingsScope( QgisApp::instance()->mapCanvas()->mapSettings() );

  const QgsStatisticalSummaryDockWidget* widget = ( const QgsStatisticalSummaryDockWidget* ) context;
  if ( widget )
  {
    expContext << QgsExpressionContextUtils::layerScope( widget->layer() );
  }

  return expContext;
}

QgsStatisticalSummaryDockWidget::QgsStatisticalSummaryDockWidget( QWidget *parent )
    : QgsDockWidget( parent )
    , mLayer( nullptr )
{
  setupUi( this );

  mFieldExpressionWidget->registerGetExpressionContextCallback( &_getExpressionContext, this );

  mLayerComboBox->setFilters( QgsMapLayerProxyModel::VectorLayer );
  mFieldExpressionWidget->setFilters( QgsFieldProxyModel::Numeric |
                                      QgsFieldProxyModel::String |
                                      QgsFieldProxyModel::Date );

  mLayerComboBox->setLayer( mLayerComboBox->layer( 0 ) );
  mFieldExpressionWidget->setLayer( mLayerComboBox->layer( 0 ) );

  connect( mLayerComboBox, SIGNAL( layerChanged( QgsMapLayer* ) ), this, SLOT( layerChanged( QgsMapLayer* ) ) );
  connect( mFieldExpressionWidget, SIGNAL( fieldChanged( QString ) ), this, SLOT( refreshStatistics() ) );
  connect( mSelectedOnlyCheckBox, SIGNAL( toggled( bool ) ), this, SLOT( refreshStatistics() ) );
  connect( mButtonRefresh, SIGNAL( clicked( bool ) ), this, SLOT( refreshStatistics() ) );
  connect( QgsMapLayerRegistry::instance(), SIGNAL( layersWillBeRemoved( QStringList ) ), this, SLOT( layersRemoved( QStringList ) ) );

  QSettings settings;
  Q_FOREACH ( QgsStatisticalSummary::Statistic stat, mDisplayStats )
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
  if ( !mLayer || ( mFieldExpressionWidget->isExpression() && !mFieldExpressionWidget->isValidExpression() ) )
  {
    mStatisticsTable->setRowCount( 0 );
    return;
  }

  // non numeric field?
  bool isNumeric = true;
  QVariant::Type fieldType = QVariant::Double;
  if ( !mFieldExpressionWidget->isExpression() )
  {
    QString field = mFieldExpressionWidget->currentField();
    fieldType = mLayer->fields().field( mLayer->fields().fieldNameIndex( field ) ).type();
    if ( fieldType == QVariant::String || fieldType == QVariant::Date || fieldType == QVariant::DateTime )
    {
      isNumeric = false;
    }
  }

  bool selectedOnly = mSelectedOnlyCheckBox->isChecked();

  if ( isNumeric )
  {
    updateNumericStatistics( selectedOnly );
  }
  else
  {
    switch ( fieldType )
    {
      case QVariant::String:
        updateStringStatistics( selectedOnly );
        break;
      case QVariant::Date:
      case QVariant::DateTime:
        updateDateTimeStatistics( selectedOnly );
        break;
      default:
        break;
    }
  }
}

void QgsStatisticalSummaryDockWidget::updateNumericStatistics( bool selectedOnly )
{
  QString sourceFieldExp = mFieldExpressionWidget->currentField();

  bool ok;
  int missingValues = 0;
  QList< double > values = mLayer->getDoubleValues( sourceFieldExp, ok, selectedOnly, &missingValues );

  if ( ! ok )
  {
    return;
  }

  QList< QgsStatisticalSummary::Statistic > statsToDisplay;
  QgsStatisticalSummary::Statistics statsToCalc = nullptr;
  Q_FOREACH ( QgsStatisticalSummary::Statistic stat, mDisplayStats )
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
  Q_FOREACH ( QgsStatisticalSummary::Statistic stat, statsToDisplay )
  {
    addRow( row, QgsStatisticalSummary::displayName( stat ),
            QString::number( stats.statistic( stat ) ),
            stats.count() != 0 );
    row++;
  }

  if ( mStatsActions.value( MISSING_VALUES )->isChecked() )
  {
    addRow( row, tr( "Missing (null) values" ),
            QString::number( missingValues ),
            stats.count() != 0 || missingValues != 0 );
    row++;
  }
}

void QgsStatisticalSummaryDockWidget::updateStringStatistics( bool selectedOnly )
{
  QString field = mFieldExpressionWidget->currentField();

  bool ok;
  QVariantList values = mLayer->getValues( field, ok, selectedOnly );

  if ( ! ok )
  {
    return;
  }

  QgsStringStatisticalSummary stats;
  stats.setStatistics( QgsStringStatisticalSummary::All );
  stats.calculateFromVariants( values );

  mStatisticsTable->setRowCount( mDisplayStringStats.count() );
  mStatisticsTable->setColumnCount( 2 );

  int row = 0;
  Q_FOREACH ( QgsStringStatisticalSummary::Statistic stat, mDisplayStringStats )
  {
    addRow( row, QgsStringStatisticalSummary::displayName( stat ),
            stats.statistic( stat ).toString(),
            stats.count() != 0 );
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

void QgsStatisticalSummaryDockWidget::layersRemoved( const QStringList& layers )
{
  if ( mLayer && layers.contains( mLayer->id() ) )
  {
    disconnect( mLayer, SIGNAL( selectionChanged() ), this, SLOT( layerSelectionChanged() ) );
    mLayer = nullptr;
  }
}

void QgsStatisticalSummaryDockWidget::layerSelectionChanged()
{
  if ( mSelectedOnlyCheckBox->isChecked() )
    refreshStatistics();
}

void QgsStatisticalSummaryDockWidget::updateDateTimeStatistics( bool selectedOnly )
{
  QString field = mFieldExpressionWidget->currentField();

  bool ok;
  QVariantList values = mLayer->getValues( field, ok, selectedOnly );

  if ( ! ok )
  {
    return;
  }

  QgsDateTimeStatisticalSummary stats;
  stats.setStatistics( QgsDateTimeStatisticalSummary::All );
  stats.calculate( values );

  mStatisticsTable->setRowCount( mDisplayDateTimeStats.count() );
  mStatisticsTable->setColumnCount( 2 );

  int row = 0;
  Q_FOREACH ( QgsDateTimeStatisticalSummary::Statistic stat, mDisplayDateTimeStats )
  {
    QString value = ( stat == QgsDateTimeStatisticalSummary::Range
                      ? tr( "%1 seconds" ).arg( stats.range().seconds() )
                      : stats.statistic( stat ).toString() );

    addRow( row, QgsDateTimeStatisticalSummary::displayName( stat ),
            value,
            stats.count() != 0 );
    row++;
  }
}

void QgsStatisticalSummaryDockWidget::addRow( int row, const QString& name, const QString& value,
    bool showValue )
{
  QTableWidgetItem* nameItem = new QTableWidgetItem( name );
  nameItem->setToolTip( name );
  nameItem->setFlags( Qt::ItemIsSelectable | Qt::ItemIsEnabled );
  mStatisticsTable->setItem( row, 0, nameItem );

  QTableWidgetItem* valueItem = new QTableWidgetItem();
  if ( showValue )
  {
    valueItem->setText( value );
  }
  valueItem->setToolTip( value );
  valueItem->setFlags( Qt::ItemIsSelectable | Qt::ItemIsEnabled );
  mStatisticsTable->setItem( row, 1, valueItem );
}

