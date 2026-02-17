/***************************************************************************
                         qgslayoutchartwidget.cpp
                         --------------------------
     begin                : August 2025
     copyright            : (C) 2025 by Mathieu
     email                : mathieu at opengis dot ch
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgslayoutchartwidget.h"

#include "qgsapplication.h"
#include "qgsgui.h"
#include "qgslayout.h"
#include "qgslayoutchartseriesdetailswidget.h"
#include "qgslayoutitemchart.h"
#include "qgsplotregistry.h"
#include "qgsplotwidget.h"

#include "moc_qgslayoutchartwidget.cpp"

QgsLayoutChartWidget::QgsLayoutChartWidget( QgsLayoutItemChart *chartItem )
  : QgsLayoutItemBaseWidget( nullptr, chartItem )
  , mChartItem( chartItem )
{
  setupUi( this );

  //add widget for general composer item properties
  mItemPropertiesWidget = new QgsLayoutItemPropertiesWidget( this, chartItem );
  mainLayout->addWidget( mItemPropertiesWidget );

  QMap<QString, QString> plotTypes = QgsApplication::instance()->plotRegistry()->plotTypes();
  for ( auto plotTypesIterator = plotTypes.keyValueBegin(); plotTypesIterator != plotTypes.keyValueEnd(); ++plotTypesIterator )
  {
    mChartTypeComboBox->addItem( plotTypesIterator->second, plotTypesIterator->first );
  }

  mLayerComboBox->setFilters( Qgis::LayerFilter::VectorLayer );

  connect( mChartTypeComboBox, static_cast<void ( QComboBox::* )( int )>( &QComboBox::currentIndexChanged ), this, &QgsLayoutChartWidget::mChartTypeComboBox_currentIndexChanged );
  connect( mChartPropertiesButton, &QPushButton::clicked, this, &QgsLayoutChartWidget::mChartPropertiesButton_clicked );

  connect( mLayerComboBox, &QgsMapLayerComboBox::layerChanged, this, &QgsLayoutChartWidget::changeLayer );
  connect( mLayerComboBox, &QgsMapLayerComboBox::layerChanged, mSortExpressionWidget, &QgsFieldExpressionWidget::setLayer );
  connect( mSortCheckBox, &QCheckBox::stateChanged, this, &QgsLayoutChartWidget::mSortCheckBox_stateChanged );
  connect( mSortExpressionWidget, static_cast<void ( QgsFieldExpressionWidget::* )( const QString &, bool )>( &QgsFieldExpressionWidget::fieldChanged ), this, &QgsLayoutChartWidget::changeSortExpression );
  connect( mSortDirectionButton, &QToolButton::clicked, this, &QgsLayoutChartWidget::mSortDirectionButton_clicked );

  connect( mSeriesListWidget, &QListWidget::currentItemChanged, this, &QgsLayoutChartWidget::mSeriesListWidget_currentItemChanged );
  connect( mSeriesListWidget, &QListWidget::itemChanged, this, &QgsLayoutChartWidget::mSeriesListWidget_itemChanged );
  connect( mAddSeriesPushButton, &QPushButton::clicked, this, &QgsLayoutChartWidget::mAddSeriesPushButton_clicked );
  connect( mRemoveSeriesPushButton, &QPushButton::clicked, this, &QgsLayoutChartWidget::mRemoveSeriesPushButton_clicked );
  connect( mSeriesPropertiesButton, &QPushButton::clicked, this, &QgsLayoutChartWidget::mSeriesPropertiesButton_clicked );

  mLinkedMapComboBox->setCurrentLayout( mChartItem->layout() );
  mLinkedMapComboBox->setItemType( QgsLayoutItemRegistry::LayoutMap );
  connect( mLinkedMapComboBox, &QgsLayoutItemComboBox::itemChanged, this, &QgsLayoutChartWidget::mLinkedMapComboBox_itemChanged );

  connect( mFilterOnlyVisibleFeaturesCheckBox, &QCheckBox::stateChanged, this, &QgsLayoutChartWidget::mFilterOnlyVisibleFeaturesCheckBox_stateChanged );
  connect( mIntersectAtlasCheckBox, &QCheckBox::stateChanged, this, &QgsLayoutChartWidget::mIntersectAtlasCheckBox_stateChanged );

  setGuiElementValues();

  connect( mChartItem, &QgsLayoutObject::changed, this, &QgsLayoutChartWidget::setGuiElementValues );
}

void QgsLayoutChartWidget::setMasterLayout( QgsMasterLayoutInterface *masterLayout )
{
  if ( mItemPropertiesWidget )
    mItemPropertiesWidget->setMasterLayout( masterLayout );
}

bool QgsLayoutChartWidget::setNewItem( QgsLayoutItem *item )
{
  if ( item->type() != QgsLayoutItemRegistry::LayoutChart )
    return false;

  if ( mChartItem )
  {
    disconnect( mChartItem, &QgsLayoutObject::changed, this, &QgsLayoutChartWidget::setGuiElementValues );
  }

  mChartItem = qobject_cast<QgsLayoutItemChart *>( item );
  mItemPropertiesWidget->setItem( mChartItem );

  if ( mChartItem )
  {
    connect( mChartItem, &QgsLayoutObject::changed, this, &QgsLayoutChartWidget::setGuiElementValues );
  }

  setGuiElementValues();

  return true;
}

void QgsLayoutChartWidget::setGuiElementValues()
{
  if ( mChartItem )
  {
    whileBlocking( mChartTypeComboBox )->setCurrentIndex( mChartTypeComboBox->findData( mChartItem->plot()->type() ) );
    whileBlocking( mLayerComboBox )->setLayer( mChartItem->sourceLayer() );

    whileBlocking( mSortCheckBox )->setCheckState( mChartItem->sortFeatures() ? Qt::Checked : Qt::Unchecked );

    whileBlocking( mSortDirectionButton )->setEnabled( mChartItem->sortFeatures() );
    whileBlocking( mSortDirectionButton )->setArrowType( mChartItem->sortAscending() ? Qt::UpArrow : Qt::DownArrow );

    whileBlocking( mSortExpressionWidget )->setEnabled( mChartItem->sortFeatures() );
    whileBlocking( mSortExpressionWidget )->setLayer( mChartItem->sourceLayer() );
    whileBlocking( mSortExpressionWidget )->setField( mChartItem->sortExpression() );

    mSeriesListWidget->clear();
    const QList<QgsLayoutItemChart::SeriesDetails> seriesList = mChartItem->seriesList();
    for ( const QgsLayoutItemChart::SeriesDetails &series : seriesList )
    {
      addSeriesListItem( series.name() );
    }

    whileBlocking( mFilterOnlyVisibleFeaturesCheckBox )->setChecked( mChartItem->filterOnlyVisibleFeatures() );
    mLinkedMapLabel->setEnabled( mFilterOnlyVisibleFeaturesCheckBox->isChecked() );
    mLinkedMapComboBox->setEnabled( mFilterOnlyVisibleFeaturesCheckBox->isChecked() );
    whileBlocking( mLinkedMapComboBox )->setItem( mChartItem->map() );

    whileBlocking( mIntersectAtlasCheckBox )->setChecked( mChartItem->filterToAtlasFeature() );
  }
  else
  {
    mSeriesListWidget->clear();
  }
}

void QgsLayoutChartWidget::mChartTypeComboBox_currentIndexChanged( int )
{
  if ( !mChartItem )
  {
    return;
  }

  const QString plotType = mChartTypeComboBox->currentData().toString();
  if ( mChartItem->plot()->type() == plotType )
  {
    return;
  }

  QgsPlot *newPlot = QgsApplication::instance()->plotRegistry()->createPlot( plotType );
  Qgs2DXyPlot *newPlot2DXy = dynamic_cast<Qgs2DXyPlot *>( newPlot );
  if ( newPlot2DXy )
  {
    Qgs2DXyPlot *oldPlot2DXy = dynamic_cast<Qgs2DXyPlot *>( mChartItem->plot() );
    if ( oldPlot2DXy )
    {
      // Transfer a few basic details for a nicer UX
      newPlot2DXy->setXMinimum( oldPlot2DXy->xMinimum() );
      newPlot2DXy->setXMaximum( oldPlot2DXy->xMaximum() );
      newPlot2DXy->setYMinimum( oldPlot2DXy->yMinimum() );
      newPlot2DXy->setYMaximum( oldPlot2DXy->yMaximum() );

      newPlot2DXy->xAxis().setType( oldPlot2DXy->xAxis().type() );
      newPlot2DXy->xAxis().setGridIntervalMajor( oldPlot2DXy->xAxis().gridIntervalMajor() );
      newPlot2DXy->xAxis().setGridIntervalMinor( oldPlot2DXy->xAxis().gridIntervalMinor() );
      newPlot2DXy->xAxis().setLabelInterval( oldPlot2DXy->xAxis().labelInterval() );

      newPlot2DXy->yAxis().setGridIntervalMajor( oldPlot2DXy->yAxis().gridIntervalMajor() );
      newPlot2DXy->yAxis().setGridIntervalMinor( oldPlot2DXy->yAxis().gridIntervalMinor() );
      newPlot2DXy->yAxis().setLabelInterval( oldPlot2DXy->yAxis().labelInterval() );
    }
  }

  mChartItem->beginCommand( tr( "Change Chart Type" ) );
  mChartItem->setPlot( newPlot );
  mChartItem->update();
  mChartItem->endCommand();
}

void QgsLayoutChartWidget::mChartPropertiesButton_clicked()
{
  if ( !mChartItem )
  {
    return;
  }

  QgsGui::initPlotWidgets();

  const QString plotType = mChartTypeComboBox->currentData().toString();
  QgsPlotAbstractMetadata *abstractMetadata = QgsApplication::instance()->plotRegistry()->plotMetadata( plotType );
  if ( !abstractMetadata )
  {
    return;
  }

  QgsPlotWidget *widget = abstractMetadata->createPlotWidget( this );
  if ( !widget )
  {
    return;
  }

  widget->registerExpressionContextGenerator( mChartItem );
  widget->setPlot( mChartItem->plot() );

  connect( widget, &QgsPanelWidget::widgetChanged, this, [this, widget]() {
    if ( !mChartItem )
    {
      return;
    }

    mChartItem->beginCommand( tr( "Modify Chart" ) );
    mChartItem->setPlot( widget->createPlot() );
    mChartItem->endCommand();
    mChartItem->update();
  } );

  openPanel( widget );
}

void QgsLayoutChartWidget::changeLayer( QgsMapLayer *layer )
{
  if ( !mChartItem )
  {
    return;
  }

  QgsVectorLayer *vl = qobject_cast<QgsVectorLayer *>( layer );
  if ( !vl )
  {
    return;
  }

  mChartItem->beginCommand( tr( "Change Chart Source Layer" ) );
  mChartItem->setSourceLayer( vl );
  mChartItem->update();
  mChartItem->endCommand();
}

void QgsLayoutChartWidget::changeSortExpression( const QString &expression, bool )
{
  if ( !mChartItem )
  {
    return;
  }

  mChartItem->beginCommand( tr( "Change Chart Source Sort Expression" ) );
  mChartItem->setSortExpression( expression );
  mChartItem->update();
  mChartItem->endCommand();
}

void QgsLayoutChartWidget::mSortCheckBox_stateChanged( int state )
{
  if ( !mChartItem )
    return;

  if ( state == Qt::Checked )
  {
    mSortDirectionButton->setEnabled( true );
    mSortExpressionWidget->setEnabled( true );
  }
  else
  {
    mSortDirectionButton->setEnabled( false );
    mSortExpressionWidget->setEnabled( false );
  }

  mChartItem->beginCommand( tr( "Toggle Atlas Sorting" ) );
  mChartItem->setSortFeatures( state == Qt::Checked );
  mChartItem->update();
  mChartItem->endCommand();
}

void QgsLayoutChartWidget::mSortDirectionButton_clicked()
{
  if ( !mChartItem )
  {
    return;
  }

  Qt::ArrowType at = mSortDirectionButton->arrowType();
  at = ( at == Qt::UpArrow ) ? Qt::DownArrow : Qt::UpArrow;
  mSortDirectionButton->setArrowType( at );

  mChartItem->beginCommand( tr( "Change Chart Source Sort Direction" ) );
  mChartItem->setSortAscending( at == Qt::UpArrow );
  mChartItem->update();
  mChartItem->endCommand();
}

QListWidgetItem *QgsLayoutChartWidget::addSeriesListItem( const QString &name )
{
  QListWidgetItem *item = new QListWidgetItem( name, nullptr );
  item->setFlags( Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable );
  mSeriesListWidget->addItem( item );
  return item;
}

void QgsLayoutChartWidget::mSeriesListWidget_currentItemChanged( QListWidgetItem *current, QListWidgetItem * )
{
  mSeriesPropertiesButton->setEnabled( static_cast<bool>( current ) );
}

void QgsLayoutChartWidget::mSeriesListWidget_itemChanged( QListWidgetItem *item )
{
  if ( !mChartItem )
  {
    return;
  }

  QList<QgsLayoutItemChart::SeriesDetails> seriesList = mChartItem->seriesList();
  const int idx = mSeriesListWidget->row( item );
  if ( idx >= seriesList.size() )
  {
    return;
  }

  mChartItem->beginCommand( tr( "Rename Chart Series" ) );
  seriesList[idx].setName( item->text() );
  mChartItem->setSeriesList( seriesList );
  mChartItem->endCommand();
}

void QgsLayoutChartWidget::mAddSeriesPushButton_clicked()
{
  if ( !mChartItem )
  {
    return;
  }

  QList<QgsLayoutItemChart::SeriesDetails> seriesList = mChartItem->seriesList();
  const QString itemName = tr( "Series %1" ).arg( seriesList.size() + 1 );
  addSeriesListItem( itemName );

  mChartItem->beginCommand( tr( "Add Chart Series" ) );
  seriesList << QgsLayoutItemChart::SeriesDetails( itemName );
  mChartItem->setSeriesList( seriesList );
  mChartItem->endCommand();
  mChartItem->update();

  mSeriesListWidget->setCurrentRow( mSeriesListWidget->count() - 1 );
  mSeriesListWidget_currentItemChanged( mSeriesListWidget->currentItem(), nullptr );
}

void QgsLayoutChartWidget::mRemoveSeriesPushButton_clicked()
{
  QListWidgetItem *item = mSeriesListWidget->currentItem();
  if ( !item || !mChartItem )
  {
    return;
  }

  QList<QgsLayoutItemChart::SeriesDetails> seriesList = mChartItem->seriesList();
  const int idx = mSeriesListWidget->row( item );
  if ( idx >= seriesList.size() )
  {
    return;
  }

  QListWidgetItem *deletedItem = mSeriesListWidget->takeItem( mSeriesListWidget->row( item ) );
  delete deletedItem;

  mChartItem->beginCommand( tr( "Remove Chart Series" ) );
  seriesList.removeAt( idx );
  mChartItem->setSeriesList( seriesList );
  mChartItem->endCommand();
  mChartItem->update();
}

void QgsLayoutChartWidget::mSeriesPropertiesButton_clicked()
{
  QListWidgetItem *item = mSeriesListWidget->currentItem();
  if ( !item || !mChartItem )
  {
    return;
  }

  QList<QgsLayoutItemChart::SeriesDetails> seriesList = mChartItem->seriesList();
  const int idx = mSeriesListWidget->row( item );
  if ( idx >= seriesList.size() )
  {
    return;
  }

  QgsLayoutChartSeriesDetailsWidget *widget = new QgsLayoutChartSeriesDetailsWidget( mChartItem->sourceLayer(), idx, seriesList[idx], this );
  widget->registerExpressionContextGenerator( mChartItem );
  widget->setPanelTitle( tr( "Series Details" ) );
  connect( widget, &QgsPanelWidget::widgetChanged, this, [this, widget]() {
    if ( !mChartItem )
    {
      return;
    }

    QList<QgsLayoutItemChart::SeriesDetails> seriesList = mChartItem->seriesList();
    const int idx = widget->index();
    if ( idx >= seriesList.size() )
    {
      return;
    }

    mChartItem->beginCommand( tr( "Modify Chart Series" ) );
    seriesList[idx].setXExpression( widget->xExpression() );
    seriesList[idx].setYExpression( widget->yExpression() );
    seriesList[idx].setFilterExpression( widget->filterExpression() );
    mChartItem->setSeriesList( seriesList );
    mChartItem->endCommand();
    mChartItem->update();
  } );

  openPanel( widget );
}

void QgsLayoutChartWidget::mLinkedMapComboBox_itemChanged( QgsLayoutItem *item )
{
  if ( !mChartItem )
  {
    return;
  }

  mChartItem->beginCommand( tr( "Change Chart Map Item" ) );
  mChartItem->setMap( qobject_cast<QgsLayoutItemMap *>( item ) );
  mChartItem->endCommand();
  mChartItem->update();
}

void QgsLayoutChartWidget::mFilterOnlyVisibleFeaturesCheckBox_stateChanged( int state )
{
  if ( !mChartItem )
  {
    return;
  }

  mChartItem->beginCommand( tr( "Toggle Visible Features Only Filter" ) );
  const bool useOnlyVisibleFeatures = ( state == Qt::Checked );
  mChartItem->setFilterOnlyVisibleFeatures( useOnlyVisibleFeatures );
  mChartItem->endCommand();
  mChartItem->update();

  //enable/disable map combobox based on state of checkbox
  mLinkedMapComboBox->setEnabled( state == Qt::Checked );
  mLinkedMapLabel->setEnabled( state == Qt::Checked );
}

void QgsLayoutChartWidget::mIntersectAtlasCheckBox_stateChanged( int state )
{
  if ( !mChartItem )
  {
    return;
  }

  mChartItem->beginCommand( tr( "Toggle Chart Atlas Filter" ) );
  const bool filterToAtlas = ( state == Qt::Checked );
  mChartItem->setFilterToAtlasFeature( filterToAtlas );
  mChartItem->endCommand();
  mChartItem->update();
}
