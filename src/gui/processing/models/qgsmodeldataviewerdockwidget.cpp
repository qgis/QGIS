/***************************************************************************
    qgsmodeldataviewerdockwidget.cpp
    -----------------------------------
    begin                : April 2026
    copyright            : (C) 2026 by Valentin Buira
    email                : valentin dot buira at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsmodeldataviewerdockwidget.h"

#include <memory>

#include "qgsattributetablefiltermodel.h"
#include "qgsattributetablemodel.h"
#include "qgsexpressionselectiondialog.h"
#include "qgslayertree.h"
#include "qgsmaptoolpan.h"
#include "qgsmaptoolselect.h"
#include "qgsspinbox.h"
#include "qgsvectorlayer.h"
#include "qgsvectorlayercache.h"
#include "qgsvectortilelayer.h"

#include <QAction>
#include <QMenu>
#include <QString>
#include <QTableWidget>
#include <qactiongroup.h>

#include "moc_qgsmodeldataviewerdockwidget.cpp"

using namespace Qt::StringLiterals;

QgsModelDataViewerDockWidget::QgsModelDataViewerDockWidget( QWidget *parent, QgsMapLayer *layer, QString childId, QString outputName )
  : QgsDockWidget( parent )
  , mLayer( layer )
  , mChildId( childId )
  , mOutputName( outputName )


{
  setupUi( this );

  setAttribute( Qt::WA_DeleteOnClose );

  mToolSelect = new QgsMapToolSelect( mMapCanvas );

  setLayer( layer );


  if ( !mLayer || !mLayer->isValid() || mLayer->type() != Qgis::LayerType::Vector )
  {
    mActionZoomToSelection->setEnabled( false );
    mActionSelectFeatures->setEnabled( false );
    mActionExpressionSelect->setEnabled( false );
    mActionSelectAll->setEnabled( false );
    mActionSelectNone->setEnabled( false );
    mActionInvertSelection->setEnabled( false );

    mShowAttributeTable->setEnabled( false );

    mSplitter->widget( 0 )->hide();
  }
  else if ( mLayer && mLayer->type() == Qgis::LayerType::Vector )
  {
    // max features
    mMaxFeaturesSpinBox = new QgsSpinBox();
    mMaxFeaturesSpinBox->setMinimum( 0 );
    mMaxFeaturesSpinBox->setMaximum( 99999999 );
    mMaxFeaturesSpinBox->setValue( 1000 );
    mMaxFeaturesSpinBox->setToolTip( tr( "Maximum Number of Features to Display" ) );
    mMaxFeaturesSpinBox->setObjectName( u"MaxFeaturesSpinBox"_s );
    connect( mMaxFeaturesSpinBox, qOverload<int>( &QgsSpinBox::valueChanged ), this, [this]( int ) { loadAttributeTable(); } );

    mToolbar->insertWidget( mShowAttributeTable, mMaxFeaturesSpinBox );
  }

  mToolPan = new QgsMapToolPan( mMapCanvas );
  mToolPan->setAction( mActionPan );

  mMapCanvas->setMapTool( mToolPan );


  connect( mActionZoomToLayer, &QAction::triggered, this, [this]() {
    if ( mLayer )
      mMapCanvas->zoomToLayers( QList<QgsMapLayer *> { mLayer } );
  } );
  connect( mActionZoomToSelection, &QAction::triggered, this, [this]() {
    if ( mLayer )
      mMapCanvas->zoomToSelected( mLayer );
  } );

  connect( mActionPan, &QAction::triggered, this, [this]() { mMapCanvas->setMapTool( mToolPan ); } );
  connect( mActionSelectFeatures, &QAction::triggered, this, [this]() { mMapCanvas->setMapTool( mToolSelect ); } );

  connect( mActionExpressionSelect, &QAction::triggered, this, &QgsModelDataViewerDockWidget::selectByExpression );
  connect( mActionSelectAll, &QAction::triggered, this, &QgsModelDataViewerDockWidget::selectAll );
  connect( mActionSelectNone, &QAction::triggered, this, &QgsModelDataViewerDockWidget::deselectAll );
  connect( mActionInvertSelection, &QAction::triggered, this, &QgsModelDataViewerDockWidget::invertSelection );


  QActionGroup *actionGroup = new QActionGroup( this );
  actionGroup->addAction( mActionPan );
  actionGroup->addAction( mActionSelectFeatures );
  actionGroup->setExclusive( true );


  connect( mActionToggleProjectLayers, &QAction::toggled, this, &QgsModelDataViewerDockWidget::toggleProjectLayer );


  QgsProject *project = QgsProject::instance();

  /* Sync mechanism with the main canvas*/
  connect( project->layerTreeRoot(), &QgsLayerTreeNode::customPropertyChanged, this, [this]( QgsLayerTreeNode *, const QString & ) { toggleProjectLayer( mActionToggleProjectLayers->isChecked() ); } );
  connect( project->layerTreeRoot(), &QgsLayerTreeNode::visibilityChanged, this, [this]( QgsLayerTreeNode * ) { toggleProjectLayer( mActionToggleProjectLayers->isChecked() ); } );
  connect( project->layerTreeRoot(), &QgsLayerTree::layerOrderChanged, this, [this]() { toggleProjectLayer( mActionToggleProjectLayers->isChecked() ); } );


  connect( project, &QgsProject::layersAdded, this, [this]( const QList<QgsMapLayer *> & ) { toggleProjectLayer( mActionToggleProjectLayers->isChecked() ); } );

  connect( mActionAddLayerToMap, &QAction::triggered, this, &QgsModelDataViewerDockWidget::addLayerToMap );

  connect( mShowAttributeTable, &QAction::toggled, this, &QgsModelDataViewerDockWidget::attributeTableToggled );
}

QgsModelDataViewerDockWidget::~QgsModelDataViewerDockWidget()
{
  delete mToolPan;
  delete mToolSelect;
}


void QgsModelDataViewerDockWidget::setLayer( QgsMapLayer *layer )
{
  mLayer = layer->clone();


  mMapCanvas->setLayers( QList<QgsMapLayer *>( { mLayer } ) );
  mMapCanvas->setCurrentLayer( mLayer );

  mMapCanvas->setDestinationCrs( mLayer->crs() );
  mMapCanvas->setExtent( mMapCanvas->fullExtent() );
  mMapCanvas->refresh();

  loadAttributeTable();
}

void QgsModelDataViewerDockWidget::loadAttributeTable()
{
  if ( !mLayer || !mLayer->isValid() || mLayer->type() != Qgis::LayerType::Vector )
  {
    return;
  }


  int maxFeatures = 1000;
  if ( mMaxFeaturesSpinBox )
  {
    maxFeatures = static_cast<int>( mMaxFeaturesSpinBox->value() );
  }

  mSplitter->setSizes( QList<int> { 180, 100 } );
  // Initialize the cache
  QgsVectorLayerCache *layerCache = new QgsVectorLayerCache( qobject_cast<QgsVectorLayer *>( mLayer ), maxFeatures, this );
  layerCache->setCacheGeometry( false );
  QgsAttributeTableModel *tableModel = new QgsAttributeTableModel( layerCache, this );
  tableModel->setRequest( QgsFeatureRequest().setFlags( Qgis::FeatureRequestFlag::NoGeometry ).setLimit( maxFeatures ) );

  QgsAttributeTableFilterModel *attributeTableFilterModel = new QgsAttributeTableFilterModel( mMapCanvas, tableModel, this );
  layerCache->setParent( tableModel );
  tableModel->setParent( attributeTableFilterModel );

  mTableView->setModel( attributeTableFilterModel );

  tableModel->loadLayer();
}

void QgsModelDataViewerDockWidget::attributeTableToggled( bool checked )
{
  mTableView->setEnabled( checked );
  mTableView->setVisible( checked );
}

void QgsModelDataViewerDockWidget::toggleProjectLayer( bool checked )
{
  if ( !checked )
  {
    // We only care about our current mLayer being inspected
    mMapCanvas->setLayers( { mLayer } );
    mMapCanvas->setCurrentLayer( mLayer );
    return;
  }

  QList<QgsMapLayer *> canvasLayers;
  QgsLayerTree *rootNode = QgsProject::instance()->layerTreeRoot();

  if ( rootNode->hasCustomLayerOrder() )
  {
    const QList<QgsMapLayer *> customOrderLayers = rootNode->customLayerOrder();
    for ( const QgsMapLayer *layer : customOrderLayers )
    {
      QgsLayerTreeLayer *nodeLayer = rootNode->findLayer( layer->id() );
      if ( nodeLayer )
      {
        if ( !nodeLayer->layer()->isSpatial() )
          continue;

        if ( nodeLayer->isVisible() )
          canvasLayers << nodeLayer->layer();
      }
    }
  }
  else
  {
    QList< QgsLayerTreeNode * > queue;
    queue.append( rootNode );

    // BFS to get all the layers in the node tree
    while ( !queue.isEmpty() )
    {
      QgsLayerTreeNode *node = queue.takeFirst();
      if ( QgsLayerTree::isLayer( node ) )
      {
        QgsLayerTreeLayer *nodeLayer = QgsLayerTree::toLayer( node );
        if ( nodeLayer->isVisible() )
        {
          canvasLayers << nodeLayer->layer();
        }
      }
      else if ( QgsLayerTree::isGroup( node ) )
      {
        if ( QgsGroupLayer *groupLayer = QgsLayerTree::toGroup( node )->groupLayer() )
        {
          if ( node->isVisible() )
          {
            canvasLayers << groupLayer;
            continue;
          }
        }
      }

      queue.append( node->children() );
    }


    canvasLayers.prepend( mLayer );
    mMapCanvas->setLayers( canvasLayers );
    mMapCanvas->setCurrentLayer( mLayer );
  }
}


void QgsModelDataViewerDockWidget::selectAll()
{
  if ( !mLayer )
    return;

  if ( QgsVectorLayer *vl = qobject_cast<QgsVectorLayer *>( mLayer ) )
    vl->selectAll();
}


void QgsModelDataViewerDockWidget::selectByExpression()
{
  if ( !mLayer )
    return;


  QgsVectorLayer *vl = qobject_cast<QgsVectorLayer *>( mLayer );
  if ( !vl )
  {
    return;
  }

  QgsExpressionSelectionDialog *dlg = new QgsExpressionSelectionDialog( vl, QString(), this );
  dlg->setAttribute( Qt::WA_DeleteOnClose );
  dlg->show();
}


void QgsModelDataViewerDockWidget::deselectAll()
{
  if ( !mLayer )
    return;

  if ( QgsVectorLayer *vl = qobject_cast<QgsVectorLayer *>( mLayer ) )
    vl->removeSelection();
  else if ( QgsVectorTileLayer *vtl = qobject_cast<QgsVectorTileLayer *>( mLayer ) )
    vtl->removeSelection();
  else
    return;
}

void QgsModelDataViewerDockWidget::invertSelection()
{
  if ( !mLayer )
    return;

  if ( QgsVectorLayer *vl = qobject_cast<QgsVectorLayer *>( mLayer ) )
    vl->invertSelection();
}

void QgsModelDataViewerDockWidget::addLayerToMap()
{
  if ( !mLayer )
    return;

  std::unique_ptr<QgsMapLayer> layer( mLayer->clone() );


  // make name unique, so that's it's easy to see which is the most recent result.
  QString baseName = windowTitle();
  QString name = baseName;
  int counter = 1;
  while ( !QgsProject::instance()->mapLayersByName( name ).empty() )
  {
    counter += 1;
    name = tr( "%1 (%2)" ).arg( baseName ).arg( counter );
  }

  layer->setName( name );

  QgsProject::instance()->addMapLayer( layer.release() );
}
