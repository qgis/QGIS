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
#include "qgsexpressionutils.h"
#include "qgsmapcanvas.h"
#include "qgsproject.h"
#include "qgssettings.h"
#include "qgsstatisticalsummarydockwidget.h"
#include "qgsstatisticalsummary.h"
#include "qgsvectorlayer.h"
#include "qgsfeedback.h"
#include "qgsvectorlayerutils.h"
#include "qgsapplication.h"
#include "qgsexpressioncontextutils.h"

#include <QTableWidget>
#include <QAction>
#include <QMenu>

typedef QList< QgsStatisticalSummary::Statistic > StatsList;
typedef QList< QgsStringStatisticalSummary::Statistic > StringStatsList;
typedef QList< QgsDateTimeStatisticalSummary::Statistic > DateTimeStatsList;
Q_GLOBAL_STATIC_WITH_ARGS( StatsList, sDisplayStats, ( {QgsStatisticalSummary::Count, QgsStatisticalSummary::Sum, QgsStatisticalSummary::Mean, QgsStatisticalSummary::Median, QgsStatisticalSummary::StDev, QgsStatisticalSummary::StDevSample, QgsStatisticalSummary::Min, QgsStatisticalSummary::Max, QgsStatisticalSummary::Range, QgsStatisticalSummary::Minority, QgsStatisticalSummary::Majority, QgsStatisticalSummary::Variety, QgsStatisticalSummary::FirstQuartile, QgsStatisticalSummary::ThirdQuartile, QgsStatisticalSummary::InterQuartileRange} ) )
Q_GLOBAL_STATIC_WITH_ARGS( StringStatsList, sDisplayStringStats, ( {QgsStringStatisticalSummary::Count, QgsStringStatisticalSummary::CountDistinct, QgsStringStatisticalSummary::CountMissing, QgsStringStatisticalSummary::Min, QgsStringStatisticalSummary::Max, QgsStringStatisticalSummary::Minority, QgsStringStatisticalSummary::Majority, QgsStringStatisticalSummary::MinimumLength, QgsStringStatisticalSummary::MaximumLength, QgsStringStatisticalSummary::MeanLength} ) )
Q_GLOBAL_STATIC_WITH_ARGS( DateTimeStatsList, sDisplayDateTimeStats, ( {QgsDateTimeStatisticalSummary::Count, QgsDateTimeStatisticalSummary::CountDistinct, QgsDateTimeStatisticalSummary::CountMissing, QgsDateTimeStatisticalSummary::Min, QgsDateTimeStatisticalSummary::Max, QgsDateTimeStatisticalSummary::Range} ) )

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

  connect( this, &QgsDockWidget::visibilityChanged, this, [ = ]( bool visible )
  {
    if ( mPendingCalculate && visible )
      refreshStatistics();
  } );
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
    rows += columns.join( QLatin1Char( '\t' ) );
    columns.clear();
  }

  if ( !rows.isEmpty() )
  {
    const QString text = QStringLiteral( "%1\t%2\n%3" ).arg( mStatisticsTable->horizontalHeaderItem( 0 )->text(),
                         mStatisticsTable->horizontalHeaderItem( 1 )->text(),
                         rows.join( QLatin1Char( '\n' ) ) );
    QString html = QStringLiteral( "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.0 Transitional//EN\"><html><head><meta http-equiv=\"content-type\" content=\"text/html; charset=utf-8\"/></head><body><table border=\"1\"><tr><td>%1</td></tr></table></body></html>" ).arg( text );
    html.replace( QLatin1String( "\t" ), QLatin1String( "</td><td>" ) ).replace( QLatin1String( "\n" ), QLatin1String( "</td></tr><tr><td>" ) );

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

  if ( !isUserVisible() )
  {
    //defer calculation until dock is visible -- no point calculating stats if the user can't
    //see them!
    mPendingCalculate = true;
    return;
  }

  mPendingCalculate = false;

  // determine field type
  mFieldType = DataType::Numeric;
  if ( !mFieldExpressionWidget->isExpression() )
  {
    mFieldType = fieldType( mFieldExpressionWidget->currentField() );
  }
  else
  {
    QgsExpressionContext context;
    context.appendScopes( QgsExpressionContextUtils::globalProjectLayerScopes( mLayer ) );
    QgsFeatureRequest request;
    if ( mSelectedOnlyCheckBox->isChecked() )
      request.setFilterFids( mLayer->selectedFeatureIds() );

    std::tuple<QVariant::Type, int> returnType = QgsExpressionUtils::determineResultType( mFieldExpressionWidget->expression(), mLayer, request, context );
    switch ( std::get<0>( returnType ) )
    {
      case QVariant::String:
        mFieldType = DataType::String;
        break;
      case QVariant::Date:
      case QVariant::DateTime:
        mFieldType = DataType::DateTime;
        break;
      default:
        mFieldType = DataType::Numeric;
        break;
    }
  }

  if ( mFieldType != mPreviousFieldType )
  {
    refreshStatisticsMenu();
    mPreviousFieldType = mFieldType;
  }

  const QString sourceFieldExp = mFieldExpressionWidget->currentField();
  const bool selectedOnly = mSelectedOnlyCheckBox->isChecked();

  if ( mGatherer )
  {
    mGatherer->cancel();
  }

  bool ok;
  const QgsFeatureIterator fit = QgsVectorLayerUtils::getValuesIterator( mLayer, sourceFieldExp, ok, selectedOnly );
  if ( ok )
  {
    const long featureCount = selectedOnly ? mLayer->selectedFeatureCount() : mLayer->featureCount();
    std::unique_ptr< QgsStatisticsValueGatherer > gatherer = std::make_unique< QgsStatisticsValueGatherer >( mLayer, fit, featureCount, sourceFieldExp );
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

  const QList< QVariant > variantValues = mGatherer->values();

  QList<double> values;
  bool convertOk;
  int missingValues = 0;
  const auto constVariantValues = variantValues;
  for ( const QVariant &value : constVariantValues )
  {
    const double val = value.toDouble( &convertOk );
    if ( convertOk )
      values << val;
    else if ( value.isNull() )
    {
      missingValues += 1;
    }
  }

  QList< QgsStatisticalSummary::Statistic > statsToDisplay;
  QgsStatisticalSummary::Statistics statsToCalc = QgsStatisticalSummary::Statistics();
  const auto displayStats = *sDisplayStats();
  for ( const QgsStatisticalSummary::Statistic stat : displayStats )
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
  const auto constStatsToDisplay = statsToDisplay;
  for ( const QgsStatisticalSummary::Statistic stat : constStatsToDisplay )
  {
    const double val = stats.statistic( stat );
    addRow( row, QgsStatisticalSummary::displayName( stat ),
            std::isnan( val ) ? QString() : QLocale().toString( val ),
            stats.count() != 0 );
    row++;
  }

  if ( mStatsActions.value( MISSING_VALUES )->isChecked() )
  {
    addRow( row, tr( "Missing (null) values" ),
            QLocale().toString( missingValues ),
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

  const QVariantList values = mGatherer->values();

  QList< QgsStringStatisticalSummary::Statistic > statsToDisplay;
  QgsStringStatisticalSummary::Statistics statsToCalc = QgsStringStatisticalSummary::Statistics();
  const auto displayStringStats = *sDisplayStringStats();
  for ( const QgsStringStatisticalSummary::Statistic stat : displayStringStats )
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
  const auto constStatsToDisplay = statsToDisplay;
  for ( const QgsStringStatisticalSummary::Statistic stat : constStatsToDisplay )
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
  QAction *action = qobject_cast<QAction *>( sender() );
  const int stat = action->data().toInt();

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

  const QVariantList values = mGatherer->values();

  QList< QgsDateTimeStatisticalSummary::Statistic > statsToDisplay;
  QgsDateTimeStatisticalSummary::Statistics statsToCalc = QgsDateTimeStatisticalSummary::Statistics();
  const auto displayDateTimeStats = *sDisplayDateTimeStats();
  for ( const QgsDateTimeStatisticalSummary::Statistic stat : displayDateTimeStats )
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
  const auto constStatsToDisplay = statsToDisplay;
  for ( const QgsDateTimeStatisticalSummary::Statistic stat : constStatsToDisplay )
  {
    const QString value = ( stat == QgsDateTimeStatisticalSummary::Range
                            ? tr( "%n second(s)", nullptr, stats.range().seconds() )
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

  const QgsSettings settings;
  switch ( mFieldType )
  {
    case DataType::Numeric:
    {
      const auto displayStats = *sDisplayStats();
      for ( const QgsStatisticalSummary::Statistic stat : displayStats )
      {
        QAction *action = new QAction( QgsStatisticalSummary::displayName( stat ), mStatisticsMenu );
        action->setCheckable( true );
        const bool checked = settings.value( QStringLiteral( "StatisticalSummaryDock/numeric_%1" ).arg( stat ), true ).toBool();
        action->setChecked( checked );
        action->setData( stat );
        mStatsActions.insert( stat, action );
        connect( action, &QAction::toggled, this, &QgsStatisticalSummaryDockWidget::statActionTriggered );
        mStatisticsMenu->addAction( action );
      }

      //count of null values statistic
      QAction *nullCountAction = new QAction( tr( "Missing (null) values" ), mStatisticsMenu );
      nullCountAction->setCheckable( true );
      const bool checked = settings.value( QStringLiteral( "StatisticalSummaryDock/numeric_missing_values" ), true ).toBool();
      nullCountAction->setChecked( checked );
      nullCountAction->setData( MISSING_VALUES );
      mStatsActions.insert( MISSING_VALUES, nullCountAction );
      connect( nullCountAction, &QAction::toggled, this, &QgsStatisticalSummaryDockWidget::statActionTriggered );
      mStatisticsMenu->addAction( nullCountAction );

      break;
    }
    case DataType::String:
    {
      const auto displayStringStats = *sDisplayStringStats();
      for ( const QgsStringStatisticalSummary::Statistic stat : displayStringStats )
      {
        QAction *action = new QAction( QgsStringStatisticalSummary::displayName( stat ), mStatisticsMenu );
        action->setCheckable( true );
        const bool checked = settings.value( QStringLiteral( "StatisticalSummaryDock/string_%1" ).arg( stat ), true ).toBool();
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
      const auto displayDateTimeStats = *sDisplayDateTimeStats();
      for ( const QgsDateTimeStatisticalSummary::Statistic stat : displayDateTimeStats )
      {
        QAction *action = new QAction( QgsDateTimeStatisticalSummary::displayName( stat ), mStatisticsMenu );
        action->setCheckable( true );
        const bool checked = settings.value( QStringLiteral( "StatisticalSummaryDock/datetime_%1" ).arg( stat ), true ).toBool();
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
  const QgsField field = mLayer->fields().field( mLayer->fields().lookupField( fieldName ) );
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
  : QgsTask( tr( "Fetching statistic values" ), QgsTask::CanCancel | QgsTask::CancelWithoutPrompt )
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
      const QVariant v = mExpression->evaluate( &mContext );
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
