/***************************************************************************
  rulesDialog.cpp
  TOPOLogy checker
  -------------------
         date                 : May 2009
         copyright            : Vita Cizek
         email                : weetya (at) gmail.com

 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <QtGui>
#include <qgsvectordataprovider.h>
#include <qgsvectorlayer.h>
#include <qgsmaplayer.h>
#include <qgsmaplayerregistry.h>
#include <qgsproviderregistry.h>
#include <qgslogger.h>
#include <qgisinterface.h>
#include <qgsproject.h>
#include "qgsapplication.h"
#include "rulesDialog.h"
#include "topolTest.h"

rulesDialog::rulesDialog( QMap<QString, TopologyRule> testMap, QgisInterface *theQgisIface, QWidget *parent )
    : QDialog( parent ), Ui::rulesDialog()
{
  setupUi( this );

  mQgisIface = theQgisIface;

  //setHorizontalHeaderItems();
  mRulesTable->hideColumn( 4 );
  mRulesTable->hideColumn( 5 );

  mTestConfMap = testMap;
  mRulesTable->setSelectionBehavior( QAbstractItemView::SelectRows );
  mRuleBox->addItems( mTestConfMap.keys() );

  mAddTestButton->setIcon( QIcon( QgsApplication::iconPath( "symbologyAdd.png" ) ) );
  mDeleteTestButton->setIcon( QIcon( QgsApplication::iconPath( "symbologyRemove.png" ) ) );

  connect( mAddTestButton, SIGNAL( clicked() ), this, SLOT( addRule() ) );
  connect( mAddTestButton, SIGNAL( clicked() ), mRulesTable, SLOT( resizeColumnsToContents() ) );
  // attempt to add new test when Ok clicked
  //connect( buttonBox, SIGNAL( accepted() ), this, SLOT( addTest() ) );
  connect( mDeleteTestButton, SIGNAL( clicked() ), this, SLOT( deleteTest() ) );

  connect( mLayer1Box, SIGNAL( currentIndexChanged( const QString& ) ), this, SLOT( updateRuleItems( const QString& ) ) );
  connect( mRuleBox, SIGNAL( currentIndexChanged( const QString& ) ), this, SLOT( showControls( const QString& ) ) );

  mRuleBox->setCurrentIndex( 0 );

  //this resets this plugin up if a project is loaded
  connect( mQgisIface, SIGNAL( projectRead() ), this, SLOT( projectRead() ) );
  //reset plugin if new project is activated
  projectRead();
}

rulesDialog::~rulesDialog()
{
}

void rulesDialog::setHorizontalHeaderItems()
{
  QStringList labels;
  labels << "Test" << "Layer #1" << "Layer #2" << "Tolerance" << "" << "";
  mRulesTable->setHorizontalHeaderLabels( labels );
}

void rulesDialog::readTest( int index, QgsMapLayerRegistry* layerRegistry )
{
  QString testName;
  QString layer1Id;
  QString layer2Id;
  QString tolerance;
  QgsProject* project = QgsProject::instance();
  QString postfix = QString( "%1" ).arg( index );

  testName = project->readEntry( "Topol", "/testname_" + postfix, "" );
  tolerance = project->readEntry( "Topol", "/tolerance_" + postfix, "" );
  layer1Id = project->readEntry( "Topol", "/layer1_" + postfix, "" );
  layer2Id = project->readEntry( "Topol", "/layer2_" + postfix, "" );

  QgsVectorLayer* l1;
  if ( !( QgsVectorLayer* )layerRegistry->mapLayers().contains( layer1Id ) )
    return;

  l1 = ( QgsVectorLayer* )layerRegistry->mapLayers()[layer1Id];
  if ( !l1 )
    return;

  QString layer1Name = l1->name();
  QString layer2Name;
  QgsVectorLayer* l2;

  if ( mTestConfMap[testName].useSecondLayer )
  {
    if ( !( QgsVectorLayer* )layerRegistry->mapLayers().contains( layer2Id ) )
      return;
    else
    {
      l2 = ( QgsVectorLayer* )layerRegistry->mapLayers()[layer2Id];
      layer2Name = l2->name();
    }
  }
  else
    layer2Name = "No layer";

  int row = index;
  mRulesTable->insertRow( row );

  QTableWidgetItem* newItem;
  newItem = new QTableWidgetItem( testName );
  newItem->setFlags( newItem->flags() & ~Qt::ItemIsEditable );
  mRulesTable->setItem( row, 0, newItem );

  newItem = new QTableWidgetItem( layer1Name );
  newItem->setFlags( newItem->flags() & ~Qt::ItemIsEditable );
  mRulesTable->setItem( row, 1, newItem );

  newItem = new QTableWidgetItem( layer2Name );
  newItem->setFlags( newItem->flags() & ~Qt::ItemIsEditable );
  mRulesTable->setItem( row, 2, newItem );

  if ( mTestConfMap[testName].useTolerance )
    newItem = new QTableWidgetItem( tolerance );
  else
    newItem = new QTableWidgetItem( QString( "No tolerance" ) );

  newItem->setFlags( newItem->flags() & ~Qt::ItemIsEditable );
  mRulesTable->setItem( row, 3, newItem );

  // add layer ids to hidden columns
  newItem = new QTableWidgetItem( layer1Id );
  mRulesTable->setItem( row, 4, newItem );
  newItem = new QTableWidgetItem( layer2Id );
  mRulesTable->setItem( row, 5, newItem );
}

void rulesDialog::projectRead()
{
  clearRules();
  QgsMapLayerRegistry* layerRegistry = QgsMapLayerRegistry::instance();
  int testCount = QgsProject::instance()->readNumEntry( "Topol", "/testCount" );
  mRulesTable->clearContents();

  for ( int i = 0; i < testCount; ++i )
    readTest( i, layerRegistry );
}

void rulesDialog::showControls( const QString& testName )
{
  if ( testName.isEmpty() )
  {
    return;
  }

  mLayer2Box->clear();
  mLayer2Box->addItem( "No layer" );
  TopologyRule topologyRule = mTestConfMap[testName];
  QgsMapLayerRegistry* layerRegistry = QgsMapLayerRegistry::instance();
  QList<QString> layerList = layerRegistry->mapLayers().keys();

  if ( topologyRule.useSecondLayer )
  {
    mLayer2Box->setVisible( true );
    for ( int i = 0; i < layerList.count(); ++i )
    {
      QgsVectorLayer* v1 = ( QgsVectorLayer* )layerRegistry->mapLayers()[layerList[i]];

      if ( !v1 )
      {
        continue;
      }


      if ( v1->name() == mLayer1Box->currentText() )
      {
        continue;
      }

      if ( topologyRule.layer2AcceptsType( v1->geometryType() ) )
      {
        mLayer2Box->addItem( v1->name() , v1->id() );
      }
    }
  }
  else
  {
    mLayer2Box->setVisible( false );
  }


  if ( topologyRule.useTolerance )
  {
    mToleranceBox->setVisible( true );
    mToleranceLabel->setVisible( true );
  }
  else
  {
    mToleranceBox->setVisible( false );
    mToleranceLabel->setVisible( false );
  }

}

void rulesDialog::addRule()
{
  //sanity checks
  QString test = mRuleBox->currentText();
  QString layer1 = mLayer1Box->currentText();
  if ( layer1 == "No layer" )
    return;

  QString layer2 = mLayer2Box->currentText();
  if ( layer2 == "No layer" && mTestConfMap[test].useSecondLayer )
    return;

  for ( int i = 0; i < mRulesTable->rowCount(); ++i )
  {
    if ( mRulesTable->item( i, 0 )->text() == test &&
         mRulesTable->item( i, 1 )->text() == layer1 &&
         mRulesTable->item( i, 2 )->text() == layer2 )
    {
      return;
    }
  }

  int row = mRulesTable->rowCount();
  mRulesTable->insertRow( row );

  QTableWidgetItem* newItem;
  newItem = new QTableWidgetItem( test );
  mRulesTable->setItem( row, 0, newItem );
  newItem = new QTableWidgetItem( layer1 );
  mRulesTable->setItem( row, 1, newItem );

  if ( mTestConfMap[test].useSecondLayer )
    newItem = new QTableWidgetItem( layer2 );
  else
    newItem = new QTableWidgetItem( "No layer" );

  mRulesTable->setItem( row, 2, newItem );

  if ( mTestConfMap[test].useTolerance )
    newItem = new QTableWidgetItem( QString( "%1" ).arg( mToleranceBox->value() ) );
  else
    newItem = new QTableWidgetItem( QString( "No tolerance" ) );

  mRulesTable->setItem( row, 3, newItem );

  QString layer1ID, layer2ID;
  // add layer ids to hidden columns
  // -1 for "No layer" string
  if ( mTestConfMap[test].useSecondLayer )
    layer2ID = mLayer2Box->itemData( mLayer2Box->currentIndex() ).toString();
  else
    layer2ID = "No layer";

  layer1ID =  mLayer1Box->itemData( mLayer1Box->currentIndex() ).toString();

  //TODO: use setItemData (or something like that) instead of hidden columns
  newItem = new QTableWidgetItem( layer1ID );
  mRulesTable->setItem( row, 4, newItem );
  newItem = new QTableWidgetItem( layer2ID );
  mRulesTable->setItem( row, 5, newItem );

  // save state to the project file.....
  QString postfix = QString( "%1" ).arg( row );
  QgsProject* project = QgsProject::instance();

  project->writeEntry( "Topol", "/testCount", row + 1 );
  project->writeEntry( "Topol", "/testname_" + postfix, test );
  project->writeEntry( "Topol", "/tolerance_" + postfix, QString( "%1" ).arg( mToleranceBox->value() ) );
  project->writeEntry( "Topol", "/layer1_" + postfix, layer1ID );
  project->writeEntry( "Topol", "/layer2_" + postfix, layer2ID );

  // reset controls to default
  mRuleBox->setCurrentIndex( 0 );
  mLayer1Box->setCurrentIndex( 0 );
  mLayer2Box->setCurrentIndex( 0 );
  mToleranceBox->setValue( 0 );
}

void rulesDialog::deleteTest()
{
  int row = mRulesTable->currentRow();
  if ( 0 <= row && row < mRulesTable->rowCount() )
    mRulesTable->removeRow( row );
}

void rulesDialog::updateRuleItems( const QString &layerName )
{
  if ( layerName.isEmpty() )
  {
    return;
  }

  mRuleBox->clear();

  if ( layerName == "No layer" )
  {
    return;
  }

  QString layerId = mLayer1Box->itemData( mLayer1Box->currentIndex() ).toString();

  QgsMapLayerRegistry* layerRegistry = QgsMapLayerRegistry::instance();
  QgsVectorLayer* vlayer = ( QgsVectorLayer* )layerRegistry->mapLayers()[layerId];

  if ( !vlayer )
  {
    qDebug() << "not a vector layer";
    return;
  }

  for ( QMap<QString, TopologyRule>::iterator it = mTestConfMap.begin(); it != mTestConfMap.end(); ++it )
  {
    TopologyRule rule = it.value();
    if ( rule.layer1AcceptsType( vlayer->geometryType() ) )
    {
      mRuleBox->addItem( it.key() );
    }

  }
}

void rulesDialog::initGui()
{
  QgsMapLayerRegistry* layerRegistry = QgsMapLayerRegistry::instance();

  QList<QString> layerList = layerRegistry->mapLayers().keys();

  mLayer1Box->clear();
  mLayer1Box->addItem( "No layer" );

  mLayer2Box->clear();
  mLayer2Box->addItem( "No layer" );

  mLayer1Box->blockSignals( true );
  for ( int i = 0; i < layerList.size(); ++i )
  {
    QgsVectorLayer* v1 = ( QgsVectorLayer* )layerRegistry->mapLayers()[layerList[i]];
    qDebug() << "layerid = " + layerList[i];

    // add layer name to the layer combo boxes

    mLayer1Box->addItem( v1->name(), v1->id() );
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
