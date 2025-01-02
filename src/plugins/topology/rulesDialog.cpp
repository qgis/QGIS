/***************************************************************************
  rulesDialog.cpp
  TOPOLogy checker
  -------------------
         date                 : May 2009
         copyright            : (C) 2009 by Vita Cizek
         email                : weetya (at) gmail.com

 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <set>

#include <QDebug>
#include <QTableWidgetItem>

#include "qgsvectorlayer.h"
#include "qgsmaplayer.h"
#include "qgisinterface.h"
#include "qgsproject.h"
#include "qgsapplication.h"
#include "qgshelp.h"
#include "rulesDialog.h"
#include "moc_rulesDialog.cpp"
#include "topolTest.h"

rulesDialog::rulesDialog( const QMap<QString, TopologyRule> &testMap, QgisInterface *qgisIface, QWidget *parent )
  : QDialog( parent ), Ui::rulesDialog()
{
  setupUi( this );

  mQgisIface = qgisIface;

  mContextMenu = new QMenu( this );

  QAction *selectAllAction = new QAction( tr( "Select All" ), this );
  connect( selectAllAction, &QAction::triggered, this, [=] {
    mRulesTable->setRangeSelected( QTableWidgetSelectionRange( 0, 0, mRulesTable->rowCount() - 1, mRulesTable->columnCount() - 1 ), true );
  } );
  mContextMenu->addAction( selectAllAction );
  mContextMenu->addSeparator();

  QAction *enableAction = new QAction( tr( "Activate" ), this );
  connect( enableAction, &QAction::triggered, this, [=] {
    const QModelIndexList selectedIndexes = mRulesTable->selectionModel()->selectedRows();
    for ( const QModelIndex index : selectedIndexes )
    {
      if ( QTableWidgetItem *item = mRulesTable->item( index.row(), 0 ) )
        item->setCheckState( Qt::Checked );
    }
  } );
  mContextMenu->addAction( enableAction );
  QAction *disableAction = new QAction( tr( "Deactivate" ), this );
  connect( disableAction, &QAction::triggered, this, [=] {
    const QModelIndexList selectedIndexes = mRulesTable->selectionModel()->selectedRows();
    for ( const QModelIndex index : selectedIndexes )
    {
      if ( QTableWidgetItem *item = mRulesTable->item( index.row(), 0 ) )
        item->setCheckState( Qt::Unchecked );
    }
  } );
  mContextMenu->addAction( disableAction );
  QAction *toggleAction = new QAction( tr( "Toggle Activation" ), this );
  connect( toggleAction, &QAction::triggered, this, [=] {
    const QModelIndexList selectedIndexes = mRulesTable->selectionModel()->selectedRows();
    for ( const QModelIndex index : selectedIndexes )
    {
      if ( QTableWidgetItem *item = mRulesTable->item( index.row(), 0 ) )
        item->setCheckState( item->checkState() == Qt::Checked ? Qt::Unchecked : Qt::Checked );
    }
  } );
  mContextMenu->addAction( toggleAction );
  mContextMenu->addSeparator();

  QAction *deleteAction = new QAction( tr( "Delete" ), this );
  connect( deleteAction, &QAction::triggered, this, &rulesDialog::deleteTests );
  mContextMenu->addAction( deleteAction );

  connect( mContextMenu, &QMenu::aboutToShow, this, [=] {
    selectAllAction->setEnabled( mRulesTable->rowCount() > 0 );
    const bool hasSelectedItems = !mRulesTable->selectionModel()->selectedIndexes().isEmpty();
    enableAction->setEnabled( hasSelectedItems );
    disableAction->setEnabled( hasSelectedItems );
    toggleAction->setEnabled( hasSelectedItems );
    deleteAction->setEnabled( hasSelectedItems );
  } );

  mRulesTable->setContextMenuPolicy( Qt::CustomContextMenu );
  connect( mRulesTable, &QTableWidget::customContextMenuRequested, this, [=] {
    mContextMenu->exec( QCursor::pos() );
  } );

  //setHorizontalHeaderItems();
  mRulesTable->hideColumn( 3 );
  mRulesTable->hideColumn( 4 );

  mTestConfMap = testMap;
  mRulesTable->setSelectionBehavior( QAbstractItemView::SelectRows );
  mRuleBox->addItems( mTestConfMap.keys() );

  mAddTestButton->setIcon( QIcon( QgsApplication::iconPath( "symbologyAdd.svg" ) ) );
  mDeleteTestButton->setIcon( QIcon( QgsApplication::iconPath( "symbologyRemove.svg" ) ) );

  connect( mAddTestButton, &QAbstractButton::clicked, this, &rulesDialog::addRule );
  connect( mAddTestButton, &QAbstractButton::clicked, mRulesTable, &QTableView::resizeColumnsToContents );

  connect( mRulesTable->selectionModel(), &QItemSelectionModel::selectionChanged, this, [=]() {
    bool enabled = !mRulesTable->selectionModel()->selectedIndexes().isEmpty();
    mDeleteTestButton->setEnabled( enabled );
  } );
  mDeleteTestButton->setEnabled( !mRulesTable->selectionModel()->selectedIndexes().isEmpty() );
  connect( mDeleteTestButton, &QAbstractButton::clicked, this, &rulesDialog::deleteTests );

  connect( buttonBox, &QDialogButtonBox::helpRequested, this, &rulesDialog::showHelp );

  connect( mLayer1Box, &QComboBox::currentTextChanged, this, &rulesDialog::updateRuleItems );
  connect( mRuleBox, &QComboBox::currentTextChanged, this, &rulesDialog::showControls );

  mRuleBox->setCurrentIndex( 0 );

  //this resets this plugin up if a project is loaded
  connect( mQgisIface, &QgisInterface::projectRead, this, &rulesDialog::projectRead );
  //reset plugin if new project is activated
  projectRead();
}

void rulesDialog::setHorizontalHeaderItems()
{
  QStringList labels;
  labels << tr( "Test" ) << tr( "Layer #1" ) << tr( "Layer #2" ) << QString() << QString();
  mRulesTable->setHorizontalHeaderLabels( labels );
}

void rulesDialog::readTest( int index, QgsProject *project )
{
  const QString postfix = QString::number( index );
  const bool testEnabled = project->readBoolEntry( QStringLiteral( "Topol" ), "/testenabled_" + postfix, true );
  const QString testName = project->readEntry( QStringLiteral( "Topol" ), "/testname_" + postfix, QString() );
  const QString layer1Id = project->readEntry( QStringLiteral( "Topol" ), "/layer1_" + postfix, QString() );
  const QString layer2Id = project->readEntry( QStringLiteral( "Topol" ), "/layer2_" + postfix, QString() );

  QgsVectorLayer *l1 = nullptr;
  if ( !( QgsVectorLayer * ) project->mapLayers().contains( layer1Id ) )
    return;

  l1 = ( QgsVectorLayer * ) project->mapLayer( layer1Id );
  if ( !l1 )
    return;

  const QString layer1Name = l1->name();
  QString layer2Name;
  QgsVectorLayer *l2 = nullptr;

  if ( mTestConfMap[testName].useSecondLayer )
  {
    if ( !( QgsVectorLayer * ) project->mapLayers().contains( layer2Id ) )
      return;
    else
    {
      l2 = ( QgsVectorLayer * ) project->mapLayer( layer2Id );
      layer2Name = l2->name();
    }
  }
  else
    layer2Name = QStringLiteral( "No layer" );

  const int row = index;
  mRulesTable->insertRow( row );

  QTableWidgetItem *newItem = nullptr;
  newItem = new QTableWidgetItem( testName );
  newItem->setFlags( newItem->flags() & ~Qt::ItemIsEditable );
  newItem->setCheckState( testEnabled ? Qt::Checked : Qt::Unchecked );
  mRulesTable->setItem( row, 0, newItem );

  newItem = new QTableWidgetItem( layer1Name );
  newItem->setFlags( newItem->flags() & ~Qt::ItemIsEditable );
  mRulesTable->setItem( row, 1, newItem );

  newItem = new QTableWidgetItem( layer2Name );
  newItem->setFlags( newItem->flags() & ~Qt::ItemIsEditable );
  mRulesTable->setItem( row, 2, newItem );

  // add layer ids to hidden columns
  newItem = new QTableWidgetItem( layer1Id );
  mRulesTable->setItem( row, 3, newItem );
  newItem = new QTableWidgetItem( layer2Id );
  mRulesTable->setItem( row, 4, newItem );
}

void rulesDialog::projectRead()
{
  clearRules();
  QgsProject *project = QgsProject::instance();
  const int testCount = QgsProject::instance()->readNumEntry( QStringLiteral( "Topol" ), QStringLiteral( "/testCount" ) );
  mRulesTable->clearContents();

  for ( int i = 0; i < testCount; ++i )
    readTest( i, project );
}

void rulesDialog::showControls( const QString &testName )
{
  if ( testName.isEmpty() )
  {
    return;
  }

  mLayer2Box->clear();
  mLayer2Box->addItem( tr( "No layer" ) );
  TopologyRule topologyRule = mTestConfMap[testName];
  QList<QString> layerList = QgsProject::instance()->mapLayers().keys();

  if ( topologyRule.useSecondLayer )
  {
    mLayer2Box->setVisible( true );
    for ( int i = 0; i < layerList.count(); ++i )
    {
      QgsVectorLayer *v1 = ( QgsVectorLayer * ) QgsProject::instance()->mapLayer( layerList[i] );

      if ( !v1 )
      {
        continue;
      }


      if ( v1->name() == mLayer1Box->currentText() )
      {
        continue;
      }


      if ( v1->type() == Qgis::LayerType::Vector )
      {
        if ( topologyRule.layer2AcceptsType( v1->geometryType() ) )
        {
          mLayer2Box->addItem( v1->name(), v1->id() );
        }
      }
    }
  }
  else
  {
    mLayer2Box->setVisible( false );
  }
}

void rulesDialog::addRule()
{
  //sanity checks
  const QString test = mRuleBox->currentText();
  const QString layer1 = mLayer1Box->currentText();
  if ( layer1 == tr( "No layer" ) )
    return;

  const QString layer2 = mLayer2Box->currentText();
  if ( layer2 == tr( "No layer" ) && mTestConfMap[test].useSecondLayer )
    return;

  for ( int i = 0; i < mRulesTable->rowCount(); ++i )
  {
    if ( mRulesTable->item( i, 0 )->text() == test && mRulesTable->item( i, 1 )->text() == layer1 && mRulesTable->item( i, 2 )->text() == layer2 )
    {
      return;
    }
  }

  const int row = mRulesTable->rowCount();
  mRulesTable->insertRow( row );

  QTableWidgetItem *newItem = nullptr;
  newItem = new QTableWidgetItem( test );
  newItem->setFlags( newItem->flags() & ~Qt::ItemIsEditable );
  newItem->setCheckState( Qt::Checked );
  mRulesTable->setItem( row, 0, newItem );
  newItem = new QTableWidgetItem( layer1 );
  newItem->setFlags( newItem->flags() & ~Qt::ItemIsEditable );
  mRulesTable->setItem( row, 1, newItem );
  newItem = new QTableWidgetItem( mTestConfMap[test].useSecondLayer ? layer2 : tr( "No layer" ) );
  newItem->setFlags( newItem->flags() & ~Qt::ItemIsEditable );
  mRulesTable->setItem( row, 2, newItem );

  QString layer1ID, layer2ID;
  // add layer ids to hidden columns
  // -1 for "No layer" string
  if ( mTestConfMap[test].useSecondLayer )
    layer2ID = mLayer2Box->currentData().toString();
  else
    layer2ID = tr( "No layer" );

  layer1ID = mLayer1Box->currentData().toString();

  //TODO: use setItemData (or something like that) instead of hidden columns
  newItem = new QTableWidgetItem( layer1ID );
  mRulesTable->setItem( row, 3, newItem );
  newItem = new QTableWidgetItem( layer2ID );
  mRulesTable->setItem( row, 4, newItem );

  // save state to the project file.....
  const QString postfix = QString::number( row );
  QgsProject *project = QgsProject::instance();

  project->writeEntry( QStringLiteral( "Topol" ), QStringLiteral( "/testCount" ), row + 1 );
  project->writeEntry( QStringLiteral( "Topol" ), "/testenabled_" + postfix, true );
  project->writeEntry( QStringLiteral( "Topol" ), "/testname_" + postfix, test );
  project->writeEntry( QStringLiteral( "Topol" ), "/layer1_" + postfix, layer1ID );
  project->writeEntry( QStringLiteral( "Topol" ), "/layer2_" + postfix, layer2ID );

  // reset controls to default
  mRuleBox->setCurrentIndex( 0 );
  mLayer1Box->setCurrentIndex( 0 );
  mLayer2Box->setCurrentIndex( 0 );
}

void rulesDialog::deleteTests()
{
  std::set<int, std::greater<int>> selectedRows;
  const QModelIndexList selectedIndexes = mRulesTable->selectionModel()->selectedRows();
  for ( const QModelIndex index : selectedIndexes )
  {
    selectedRows.insert( index.row() );
  }

  for ( int row : selectedRows )
  {
    mRulesTable->removeRow( row );
  }
}

void rulesDialog::updateRuleItems( const QString &layerName )
{
  if ( layerName.isEmpty() )
  {
    return;
  }

  mRuleBox->clear();

  if ( layerName == tr( "No layer" ) )
  {
    return;
  }

  const QString layerId = mLayer1Box->currentData().toString();

  QgsVectorLayer *vlayer = ( QgsVectorLayer * ) QgsProject::instance()->mapLayer( layerId );

  if ( !vlayer )
  {
    qDebug() << "not a vector layer";
    return;
  }

  for ( QMap<QString, TopologyRule>::iterator it = mTestConfMap.begin(); it != mTestConfMap.end(); ++it )
  {
    TopologyRule &rule = it.value();
    if ( rule.layer1AcceptsType( vlayer->geometryType() ) )
    {
      mRuleBox->addItem( it.key() );
    }
  }
}

void rulesDialog::initGui()
{
  QList<QString> layerList = QgsProject::instance()->mapLayers().keys();

  mLayer1Box->clear();
  mLayer1Box->addItem( tr( "No layer" ) );

  mLayer2Box->clear();
  mLayer2Box->addItem( tr( "No layer" ) );

  mLayer1Box->blockSignals( true );
  for ( int i = 0; i < layerList.size(); ++i )
  {
    QgsVectorLayer *v1 = ( QgsVectorLayer * ) QgsProject::instance()->mapLayer( layerList[i] );

    // add layer name to the layer combo boxes
    if ( v1->type() == Qgis::LayerType::Vector )
    {
      mLayer1Box->addItem( v1->name(), v1->id() );
    }
  }
  mLayer1Box->blockSignals( false );
}

void rulesDialog::clearRules()
{
  while ( mRulesTable->rowCount() > 0 )
  {
    mRulesTable->removeRow( 0 );
  }
}

void rulesDialog::showHelp()
{
  QgsHelp::openHelp( QStringLiteral( "plugins/core_plugins/plugins_topology_checker.html" ) );
}
