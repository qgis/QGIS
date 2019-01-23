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

#include "qgisapp.h"
#include "qgsclipboard.h"
#include "qgsmapcanvas.h"
#include "qgsproject.h"
#include "qgssettings.h"
#include "qgsstatisticalsummarydockwidget.h"
#include "qgsstatisticalsummary.h"
#include "qgsvectorlayer.h"
#include "qgsfeedback.h"
#include "qgsvectorlayerutils.h"
#include "qgsapplication.h"

#include <QTableWidget>
#include <QAction>
#include <QMenu>

QList< QgsStatisticalSummary::Statistic > QgsStatisticalSummaryDockWidget::sDisplayStats =
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

QList< QgsStringStatisticalSummary::Statistic > QgsStatisticalSummaryDockWidget::sDisplayStringStats =
  QList< QgsStringStatisticalSummary::Statistic > () << QgsStringStatisticalSummary::Count
  << QgsStringStatisticalSummary::CountDistinct
  << QgsStringStatisticalSummary::CountMissing
  << QgsStringStatisticalSummary::Min
  << QgsStringStatisticalSummary::Max
  << QgsStringStatisticalSummary::MinimumLength
  << QgsStringStatisticalSummary::MaximumLength;

QList< QgsDateTimeStatisticalSummary::Statistic > QgsStatisticalSummaryDockWidget::sDisplayDateTimeStats =
  QList< QgsDateTimeStatisticalSummary::Statistic > () << QgsDateTimeStatisticalSummary::Count
  << QgsDateTimeStatisticalSummary::CountDistinct
  << QgsDateTimeStatisticalSummary::CountMissing
  << QgsDateTimeStatisticalSummary::Min
  << QgsDateTimeStatisticalSummary::Max
  << QgsDateTimeStatisticalSummary::Range;

#define MISSING_VALUES -1

QgsExpressionContext QgsStatisticalSummaryDockWidget::createExpressionContext() const
{
  QgsExpressionContext expContext;
  expContext << QgsExpressionContextUtils::globalScope()
             << QgsExpressionContextUtils::projectScope( QgsProject::instance() )
             << QgsExpressionContextUtils::mapSettingsScope( QgisApp::instance()->mapCanvas()->mapSettings() )
             << QgsExpressionContextUtils::layerScope( mLayer );

  return expContext;
}

QgsStatisticalSummaryDockWidget::QgsStatisticalSummaryDockWidget( QWidget *parent )
  : QgsDockWidget( parent )

{
  setupUi( this );

  mCancelButton->hide();
  mCalculatingProgressBar->hide();

  mFieldExpressionWidget->registerExpressionContextGenerator( this );

  mLayerComboBox->setFilters( QgsMapLayerProxyModel::VectorLayer );
  mFieldExpressionWidget->setFilters( QgsFieldProxyModel::Numeric |
                                      QgsFieldProxyModel::String |
                                      QgsFieldProxyModel::Date );

  mLayerComboBox->setLayer( mLayerComboBox->layer( 0 ) );
  mFieldExpressionWidget->setLayer( mLayerComboBox->layer( 0 ) );

  connect( mLayerComboBox, &QgsMapLayerComboBox::layerChanged, this, &QgsStatisticalSummaryDockWidget::layerChanged );
  connect( mFieldExpressionWidget, static_cast<void ( QgsFieldExpressionWidget::* )( const QString & )>( &QgsFieldExpressionWidget::fieldChanged ), this, &QgsStatisticalSummaryDockWidget::fieldChanged );
  connect( mSelectedOnlyCheckBox, &QAbstractButton::toggled, this, &QgsStatisticalSummaryDockWidget::refreshStatistics );
  connect( mButtonCopy, &QAbstractButton::clicked, this, &QgsStatisticalSummaryDockWidget::copyStatistics );
  connect( mButtonRefresh, &QAbstractButton::clicked, this, &QgsStatisticalSummaryDockWidget::refreshStatistics );
  connect( QgsProject::instance(), static_cast<void ( QgsProject::* )( const QStringList & )>( &QgsProject::layersWillBeRemoved ), this, &QgsStatisticalSummaryDockWidget::layersRemoved );

  mStatisticsMenu = new QMenu( mOptionsToolButton );
  mOptionsToolButton->setMenu( mStatisticsMenu );

  mFieldType = DataType::Numeric;
  mPreviousFieldType = DataType::Numeric;
  refreshStatisticsMenu();
}

QgsStatisticalSummaryDockWidget::~QgsStatisticalSummaryDockWidget()
{
  if ( mGatherer )
  {
    mGatherer->cancel();
  }
}

void QgsStatisticalSummaryDockWidget::fieldChanged()
{
  if ( mFieldExpressionWidget->expression() != mExpression )
  {
    mExpression = mFieldExpressionWidget->expression();
    refreshStatistics();
  }
}

void QgsStatisticalSummaryDockWidget::copyStatistics()
{
  QStringList rows;
  QStringList columns;
  for ( int i = 0; i < mStatisticsTable->rowCount(); i++ )
  {
    for ( int j = 0; j < mStatisticsTable->columnCount(); j++ )
    {
      QTableWidgetItem *item =  mStatisticsTable->item( i, j );
      columns += item->text();
    }
    rows += columns.join( QStringLiteral( "\t" ) );
    columns.clear();
  }

  if ( !rows.isEmpty() )
  {
    QString text = QStringLiteral( "%1\t%2\n%3" ).arg( mStatisticsTable->horizontalHeaderItem( 0 )->text(),
                   mStatisticsTable->horizontalHeaderItem( 1 )->text(),
                   rows.join( QStringLiteral( "\n" ) ) );
    QString html = QStringLiteral( "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.0 Transitional//EN\"><html><head><meta http-equiv=\"content-type\" content=\"text/html; charset=utf-8\"/></head><body><table border=\"1\"><tr><td>%1</td></tr></table></body></html>" ).arg( text );
    html.replace( QStringLiteral( "\t" ), QStringLiteral( "</td><td>" ) ).replace( QStringLiteral( "\n" ), QStringLiteral( "</td></tr><tr><td>" ) );

    QgsClipboard clipboard;
    clipboard.setData( QStringLiteral( "text/html" ), html.toUtf8(), text );
  }
}

void QgsStatisticalSummaryDockWidget::refreshStatistics()
{
  if ( !mLayer || mFieldExpressionWidget->currentField().isEmpty() || ( mFieldExpressionWidget->isExpression() && !mFieldExpressionWidget->isValidExpression() ) )
  {
    mStatisticsTable->setRowCount( 0 );
    return;
  }

  // determine field type
  mFieldType = DataType::Numeric;
  if ( !mFieldExpressionWidget->isExpression() )
  {
    mFieldType = fieldType( mFieldExpressionWidget->currentField() );
  }

  if ( mFieldType != mPreviousFieldType )
  {
    refreshStatisticsMenu();
    mPreviousFieldType = mFieldType;
  }

  QString sourceFieldExp = mFieldExpressionWidget->currentField();
  bool selectedOnly = mSelectedOnlyCheckBox->isChecked();

  if ( mGatherer )
  {
    mGatherer->cancel();
  }

  bool ok;
  QgsFeatureIterator fit = QgsVectorLayerUtils::getValuesIterator( mLayer, sourceFieldExp, ok, selectedOnly );
  if ( ok )
  {
    long featureCount = selectedOnly ? mLayer->selectedFeatureCount() : mLayer->featureCount();
    std::unique_ptr< QgsStatisticsValueGatherer > gatherer = qgis::make_unique< QgsStatisticsValueGatherer >( mLayer, fit, featureCount, sourceFieldExp );
    switch ( mFieldType )
    {
      case DataType::Numeric:
        connect( gatherer.get(), &QgsStatisticsValueGatherer::taskCompleted, this, &QgsStatisticalSummaryDockWidget::updateNumericStatistics );
        break;
      case DataType::String:
        connect( gatherer.get(), &QgsStatisticsValueGatherer::taskCompleted, this, &QgsStatisticalSummaryDockWidget::updateStringStatistics );
        break;
      case DataType::DateTime:
        connect( gatherer.get(), &QgsStatisticsValueGatherer::taskCompleted, this, &QgsStatisticalSummaryDockWidget::updateDateTimeStatistics );
        break;
#if 0 // not required for now - we can handle all known types
      default:
        //don't know how to handle stats for this field!
        mStatisticsTable->setRowCount( 0 );
        return;
#endif
    }
    connect( gatherer.get(), &QgsStatisticsValueGatherer::progressChanged, mCalculatingProgressBar, &QProgressBar::setValue );
    connect( gatherer.get(), &QgsStatisticsValueGatherer::taskTerminated, this, &QgsStatisticalSummaryDockWidget::gathererFinished );
    connect( mCancelButton, &QPushButton::clicked, gatherer.get(), &QgsStatisticsValueGatherer::cancel );
    mCalculatingProgressBar->setMinimum( 0 );
    mCalculatingProgressBar->setMaximum( featureCount > 0 ? 100 : 0 );
    mCalculatingProgressBar->setValue( 0 );
    mCancelButton->show();
    mCalculatingProgressBar->show();

    mGatherer = gatherer.get();
    QgsApplication::taskManager()->addTask( gatherer.release() );
  }
}

void QgsStatisticalSummaryDockWidget::gathererFinished()
{
  QgsStatisticsValueGatherer *gatherer = qobject_cast<QgsStatisticsValueGatherer *>( QObject::sender() );
  // this may have been sent from a gatherer which was canceled previously and we don't care
  // about it anymore...
  if ( gatherer == mGatherer )
  {
    mGatherer = nullptr;
    mCalculatingProgressBar->setValue( -1 );
    mCancelButton->hide();
    mCalculatingProgressBar->hide();
  }
}

void QgsStatisticalSummaryDockWidget::updateNumericStatistics()
{
  QgsStatisticsValueGatherer *gatherer = qobject_cast<QgsStatisticsValueGatherer *>( QObject::sender() );
  // this may have been sent from a gatherer which was canceled previously and we don't care
  // about it anymore...
  if ( gatherer != mGatherer )
    return;

  QList< QVariant > variantValues = mGatherer->values();

  QList<double> values;
  bool convertOk;
  int missingValues = 0;
  Q_FOREACH ( const QVariant &value, variantValues )
  {
    double val = value.toDouble( &convertOk );
    if ( convertOk )
      values << val;
    else if ( value.isNull() )
    {
      missingValues += 1;
    }
  }

  QList< QgsStatisticalSummary::Statistic > statsToDisplay;
  QgsStatisticalSummary::Statistics statsToCalc = nullptr;
  Q_FOREACH ( QgsStatisticalSummary::Statistic stat, sDisplayStats )
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
    double val = stats.statistic( stat );
    addRow( row, QgsStatisticalSummary::displayName( stat ),
            std::isnan( val ) ? QString() : QString::number( val ),
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

  mStatisticsTable->resizeColumnsToContents();

  gathererFinished();
}

void QgsStatisticalSummaryDockWidget::updateStringStatistics()
{
  QgsStatisticsValueGatherer *gatherer = qobject_cast<QgsStatisticsValueGatherer *>( QObject::sender() );
  // this may have been sent from a gatherer which was canceled previously and we don't care
  // about it anymore...
  if ( gatherer != mGatherer )
    return;

  QVariantList values = mGatherer->values();

  QList< QgsStringStatisticalSummary::Statistic > statsToDisplay;
  QgsStringStatisticalSummary::Statistics statsToCalc = nullptr;
  Q_FOREACH ( QgsStringStatisticalSummary::Statistic stat, sDisplayStringStats )
  {
    if ( mStatsActions.value( stat )->isChecked() )
    {
      statsToDisplay << stat;
      statsToCalc |= stat;
    }
  }

  QgsStringStatisticalSummary stats;
  stats.setStatistics( statsToCalc );
  stats.calculateFromVariants( values );

  mStatisticsTable->setRowCount( statsToDisplay.count() );
  mStatisticsTable->setColumnCount( 2 );

  int row = 0;
  Q_FOREACH ( QgsStringStatisticalSummary::Statistic stat, statsToDisplay )
  {
    addRow( row, QgsStringStatisticalSummary::displayName( stat ),
            stats.statistic( stat ).toString(),
            stats.count() != 0 );
    row++;
  }

  mStatisticsTable->resizeColumnsToContents();

  gathererFinished();
}

void QgsStatisticalSummaryDockWidget::layerChanged( QgsMapLayer *layer )
{
  QgsVectorLayer *newLayer = qobject_cast< QgsVectorLayer * >( layer );
  if ( mLayer && mLayer != newLayer )
  {
    disconnect( mLayer, &QgsVectorLayer::selectionChanged, this, &QgsStatisticalSummaryDockWidget::layerSelectionChanged );
  }

  mLayer = newLayer;

  // clear expression, so that we don't force an unwanted recalculation
  mFieldExpressionWidget->setExpression( QString() );
  mFieldExpressionWidget->setLayer( mLayer );

  if ( mLayer )
  {
    connect( mLayer, &QgsVectorLayer::selectionChanged, this, &QgsStatisticalSummaryDockWidget::layerSelectionChanged );
  }

  if ( mGatherer )
  {
    mGatherer->cancel();
  }

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
  QAction *action = dynamic_cast<QAction *>( sender() );
  int stat = action->data().toInt();

  QString settingsKey;
  switch ( mFieldType )
  {
    case DataType::Numeric:
      settingsKey = QStringLiteral( "numeric" );
      break;
    case DataType::String:
      settingsKey = QStringLiteral( "string" );
      break;
    case DataType::DateTime:
      settingsKey = QStringLiteral( "datetime" );
      break;
  }

  QgsSettings settings;
  if ( stat >= 0 )
  {
    settings.setValue( QStringLiteral( "StatisticalSummaryDock/%1_%2" ).arg( settingsKey ).arg( stat ), checked );
  }
  else if ( stat == MISSING_VALUES )
  {
    settings.setValue( QStringLiteral( "StatisticalSummaryDock/numeric_missing_values" ), checked );
  }

  refreshStatistics();
}

void QgsStatisticalSummaryDockWidget::layersRemoved( const QStringList &layers )
{
  if ( mLayer && layers.contains( mLayer->id() ) )
  {
    disconnect( mLayer, &QgsVectorLayer::selectionChanged, this, &QgsStatisticalSummaryDockWidget::layerSelectionChanged );
    mLayer = nullptr;
  }
}

void QgsStatisticalSummaryDockWidget::layerSelectionChanged()
{
  if ( mSelectedOnlyCheckBox->isChecked() )
    refreshStatistics();
}

void QgsStatisticalSummaryDockWidget::updateDateTimeStatistics()
{
  QgsStatisticsValueGatherer *gatherer = qobject_cast<QgsStatisticsValueGatherer *>( QObject::sender() );
  // this may have been sent from a gatherer which was canceled previously and we don't care
  // about it anymore...
  if ( gatherer != mGatherer )
    return;

  QVariantList values = mGatherer->values();

  QList< QgsDateTimeStatisticalSummary::Statistic > statsToDisplay;
  QgsDateTimeStatisticalSummary::Statistics statsToCalc = nullptr;
  Q_FOREACH ( QgsDateTimeStatisticalSummary::Statistic stat, sDisplayDateTimeStats )
  {
    if ( mStatsActions.value( stat )->isChecked() )
    {
      statsToDisplay << stat;
      statsToCalc |= stat;
    }
  }


  QgsDateTimeStatisticalSummary stats;
  stats.setStatistics( statsToCalc );
  stats.calculate( values );

  mStatisticsTable->setRowCount( statsToDisplay.count() );
  mStatisticsTable->setColumnCount( 2 );

  int row = 0;
  Q_FOREACH ( QgsDateTimeStatisticalSummary::Statistic stat, statsToDisplay )
  {
    QString value = ( stat == QgsDateTimeStatisticalSummary::Range
                      ? tr( "%1 seconds" ).arg( stats.range().seconds() )
                      : stats.statistic( stat ).toString() );

    addRow( row, QgsDateTimeStatisticalSummary::displayName( stat ),
            value,
            stats.count() != 0 );
    row++;
  }

  mStatisticsTable->resizeColumnsToContents();

  gathererFinished();
}

void QgsStatisticalSummaryDockWidget::addRow( int row, const QString &name, const QString &value,
    bool showValue )
{
  QTableWidgetItem *nameItem = new QTableWidgetItem( name );
  nameItem->setToolTip( name );
  nameItem->setFlags( Qt::ItemIsSelectable | Qt::ItemIsEnabled );
  mStatisticsTable->setItem( row, 0, nameItem );

  QTableWidgetItem *valueItem = new QTableWidgetItem();
  if ( showValue )
  {
    valueItem->setText( value );
  }
  valueItem->setToolTip( value );
  valueItem->setFlags( Qt::ItemIsSelectable | Qt::ItemIsEnabled );
  mStatisticsTable->setItem( row, 1, valueItem );
}

void QgsStatisticalSummaryDockWidget::refreshStatisticsMenu()
{
  mStatisticsMenu->clear();
  mStatsActions.clear();

  QgsSettings settings;
  switch ( mFieldType )
  {
    case DataType::Numeric:
    {
      Q_FOREACH ( QgsStatisticalSummary::Statistic stat, sDisplayStats )
      {
        QAction *action = new QAction( QgsStatisticalSummary::displayName( stat ), mStatisticsMenu );
        action->setCheckable( true );
        bool checked = settings.value( QStringLiteral( "StatisticalSummaryDock/numeric_%1" ).arg( stat ), true ).toBool();
        action->setChecked( checked );
        action->setData( stat );
        mStatsActions.insert( stat, action );
        connect( action, &QAction::toggled, this, &QgsStatisticalSummaryDockWidget::statActionTriggered );
        mStatisticsMenu->addAction( action );
      }

      //count of null values statistic
      QAction *nullCountAction = new QAction( tr( "Missing (null) values" ), mStatisticsMenu );
      nullCountAction->setCheckable( true );
      bool checked = settings.value( QStringLiteral( "StatisticalSummaryDock/numeric_missing_values" ), true ).toBool();
      nullCountAction->setChecked( checked );
      nullCountAction->setData( MISSING_VALUES );
      mStatsActions.insert( MISSING_VALUES, nullCountAction );
      connect( nullCountAction, &QAction::toggled, this, &QgsStatisticalSummaryDockWidget::statActionTriggered );
      mStatisticsMenu->addAction( nullCountAction );

      break;
    }
    case DataType::String:
    {
      Q_FOREACH ( QgsStringStatisticalSummary::Statistic stat, sDisplayStringStats )
      {
        QAction *action = new QAction( QgsStringStatisticalSummary::displayName( stat ), mStatisticsMenu );
        action->setCheckable( true );
        bool checked = settings.value( QStringLiteral( "StatisticalSummaryDock/string_%1" ).arg( stat ), true ).toBool();
        action->setChecked( checked );
        action->setData( stat );
        mStatsActions.insert( stat, action );
        connect( action, &QAction::toggled, this, &QgsStatisticalSummaryDockWidget::statActionTriggered );
        mStatisticsMenu->addAction( action );
      }
      break;
    }
    case DataType::DateTime:
    {
      Q_FOREACH ( QgsDateTimeStatisticalSummary::Statistic stat, sDisplayDateTimeStats )
      {
        QAction *action = new QAction( QgsDateTimeStatisticalSummary::displayName( stat ), mStatisticsMenu );
        action->setCheckable( true );
        bool checked = settings.value( QStringLiteral( "StatisticalSummaryDock/datetime_%1" ).arg( stat ), true ).toBool();
        action->setChecked( checked );
        action->setData( stat );
        mStatsActions.insert( stat, action );
        connect( action, &QAction::toggled, this, &QgsStatisticalSummaryDockWidget::statActionTriggered );
        mStatisticsMenu->addAction( action );
      }
      break;
    }
  }
}

QgsStatisticalSummaryDockWidget::DataType QgsStatisticalSummaryDockWidget::fieldType( const QString &fieldName )
{
  QgsField field = mLayer->fields().field( mLayer->fields().lookupField( fieldName ) );
  if ( field.isNumeric() )
  {
    return DataType::Numeric;
  }

  switch ( field.type() )
  {
    case QVariant::String:
      return DataType::String;
    case QVariant::Date:
    case QVariant::DateTime:
      return DataType::DateTime;
    default:
      break;
  }

  return DataType::Numeric;
}

QgsStatisticsValueGatherer::QgsStatisticsValueGatherer( QgsVectorLayer *layer, const QgsFeatureIterator &fit, long featureCount, const QString &sourceFieldExp )
  : QgsTask( tr( "Fetching statistic values" ) )
  , mFeatureIterator( fit )
  , mFeatureCount( featureCount )
  , mFieldExpression( sourceFieldExp )
{
  mFieldIndex = layer->fields().lookupField( mFieldExpression );
  if ( mFieldIndex == -1 )
  {
    // use expression, already validated
    mExpression.reset( new QgsExpression( mFieldExpression ) );
    mContext.appendScopes( QgsExpressionContextUtils::globalProjectLayerScopes( layer ) );
  }
}

bool QgsStatisticsValueGatherer::run()
{
  QgsFeature f;
  int current = 0;
  while ( mFeatureIterator.nextFeature( f ) )
  {
    if ( mExpression )
    {
      mContext.setFeature( f );
      QVariant v = mExpression->evaluate( &mContext );
      mValues << v;
    }
    else
    {
      mValues << f.attribute( mFieldIndex );
    }

    if ( isCanceled() )
    {
      return false;
    }

    current++;
    if ( mFeatureCount > 0 )
    {
      setProgress( 100.0 * static_cast< double >( current ) / mFeatureCount );
    }
  }
  return true;
}
