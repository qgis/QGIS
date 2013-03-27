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
//#include <qgssearchstring.h>
//#include <qgssearchtreenode.h>
#include <qgsmaplayer.h>
#include <qgsmaplayerregistry.h>

#include <qgsproviderregistry.h>
#include <qgslogger.h>
#include <qgisinterface.h>
#include <qgsproject.h>

//#include "../../app/qgisapp.h"

#include "rulesDialog.h"
#include "topolTest.h"

rulesDialog::rulesDialog( QList<QString> layerList, QMap<QString, TopologyRule> testMap, QgisInterface* theQgisIface, QWidget *parent )
    : QDialog( parent ), Ui::rulesDialog()
{
  setupUi( this );

  mQgisIface = theQgisIface;

  //setHorizontalHeaderItems();
  mTestTable->hideColumn( 4 );
  mTestTable->hideColumn( 5 );

  mTestConfMap = testMap;
  mTestTable->setSelectionBehavior( QAbstractItemView::SelectRows );
  mTestBox->addItems( mTestConfMap.keys() );

  QgsMapLayerRegistry* layerRegistry = QgsMapLayerRegistry::instance();

  for ( int i = 0; i < layerList.size(); ++i )
  {
    // add layer ID to the layerId list
    mLayerIds << layerList[i];

    QgsVectorLayer* v1 = ( QgsVectorLayer* )layerRegistry->mapLayers()[layerList[i]];
    qDebug() << "layerid = " + layerList[i];

    // add layer name to the layer combo boxes
    mLayer1Box->addItem( v1->name(), v1->id() );
    //mLayer2Box->addItem((( QgsVectorLayer* )layerRegistry->mapLayers()[layerList[i]] )->name() );


  }

  connect( mLayer1Box, SIGNAL( currentIndexChanged( const QString& ) ), this, SLOT( updateRuleItems( const QString& ) ) );

  connect( mAddTestButton, SIGNAL( clicked() ), this, SLOT( addTest() ) );
  connect( mAddTestButton, SIGNAL( clicked() ), mTestTable, SLOT( resizeColumnsToContents() ) );
  // attempt to add new test when Ok clicked
  connect( buttonBox, SIGNAL( accepted() ), this, SLOT( addTest() ) );
  connect( mDeleteTestButton, SIGNAL( clicked() ), this, SLOT( deleteTest() ) );
  connect( mTestBox, SIGNAL( currentIndexChanged( const QString& ) ), this, SLOT( showControls( const QString& ) ) );
  mTestBox->setCurrentIndex( 4 );

  //this resets this plugin up if a project is loaded
  connect( mQgisIface->mainWindow(), SIGNAL( projectRead() ), this, SLOT( projectRead() ) );
  projectRead();
}

rulesDialog::~rulesDialog()
{
}

void rulesDialog::setHorizontalHeaderItems()
{
  QStringList labels;
  labels << "Test" << "Layer #1" << "Layer #2" << "Tolerance" << "" << "";
  mTestTable->setHorizontalHeaderLabels( labels );
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
  mTestTable->insertRow( row );

  QTableWidgetItem* newItem;
  newItem = new QTableWidgetItem( testName );
  newItem->setFlags( newItem->flags() & ~Qt::ItemIsEditable );
  mTestTable->setItem( row, 0, newItem );

  newItem = new QTableWidgetItem( layer1Name );
  newItem->setFlags( newItem->flags() & ~Qt::ItemIsEditable );
  mTestTable->setItem( row, 1, newItem );

  newItem = new QTableWidgetItem( layer2Name );
  newItem->setFlags( newItem->flags() & ~Qt::ItemIsEditable );
  mTestTable->setItem( row, 2, newItem );

  if ( mTestConfMap[testName].useTolerance )
    newItem = new QTableWidgetItem( tolerance );
  else
    newItem = new QTableWidgetItem( QString( "No tolerance" ) );

  newItem->setFlags( newItem->flags() & ~Qt::ItemIsEditable );
  mTestTable->setItem( row, 3, newItem );

  // add layer ids to hidden columns
  newItem = new QTableWidgetItem( layer1Id );
  mTestTable->setItem( row, 4, newItem );
  newItem = new QTableWidgetItem( layer2Id );
  mTestTable->setItem( row, 5, newItem );
}

void rulesDialog::projectRead()
{
  QgsMapLayerRegistry* layerRegistry = QgsMapLayerRegistry::instance();
  int testCount = QgsProject::instance()->readNumEntry( "Topol", "/testCount" );
  mTestTable->clearContents();

  for ( int i = 0; i < testCount; ++i )
    readTest( i, layerRegistry );
}

void rulesDialog::showControls( const QString& testName )
{
  mLayer2Box->clear();
  mLayer2Box->addItem( "No layer" );
  TopologyRule topologyRule = mTestConfMap[testName];
  QgsMapLayerRegistry* layerRegistry = QgsMapLayerRegistry::instance();

  if ( topologyRule.useSecondLayer )
  {
    mLayer2Box->setVisible( true );
    for ( int i = 0; i < mLayerIds.count(); ++i )
    {
      QgsVectorLayer* v1 = ( QgsVectorLayer* )layerRegistry->mapLayers()[mLayerIds.at( i )];

      if ( v1->name() == mLayer1Box->currentText() )
      {
        continue;
      }

      if ( topologyRule.layer2AcceptsType( v1->geometryType() ) )
      {
        mLayer2Box->addItem( v1->name() );
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

void rulesDialog::addLayer( QgsMapLayer* layer )
{
  mLayerIds << layer->id();

  // add layer name to the layer combo boxes
  mLayer1Box->addItem( layer->name() );
  mLayer2Box->addItem( layer->name() );
}

void rulesDialog::removeLayer( QString layerId )
{
  int index = mLayerIds.indexOf( layerId );

  mLayerIds.removeAt( index );
  // + 1 for "No layer" string
  mLayer1Box->removeItem( index + 1 );
  mLayer2Box->removeItem( index + 1 );

  // TODO: Maybe tell the dock that we have no layers under
  //if (mLayer1Box->size() == 1) do something
}

void rulesDialog::addTest()
{
  //sanity checks
  QString test = mTestBox->currentText();
  QString layer1 = mLayer1Box->currentText();
  if ( layer1 == "No layer" )
    return;

  QString layer2 = mLayer2Box->currentText();
  if ( layer2 == "No layer" && mTestConfMap[test].useSecondLayer )
    return;

  for ( int i = 0; i < mTestTable->rowCount(); ++i )
  {
    if ( mTestTable->item( i, 0 )->text() == test &&
         mTestTable->item( i, 1 )->text() == layer1 &&
         mTestTable->item( i, 2 )->text() == layer2 )
    {
      return;
    }
  }

  int row = mTestTable->rowCount();
  mTestTable->insertRow( row );

  QTableWidgetItem* newItem;
  newItem = new QTableWidgetItem( test );
  mTestTable->setItem( row, 0, newItem );
  newItem = new QTableWidgetItem( layer1 );
  mTestTable->setItem( row, 1, newItem );

  if ( mTestConfMap[test].useSecondLayer )
    newItem = new QTableWidgetItem( layer2 );
  else
    newItem = new QTableWidgetItem( "No layer" );

  mTestTable->setItem( row, 2, newItem );

  if ( mTestConfMap[test].useTolerance )
    newItem = new QTableWidgetItem( QString( "%1" ).arg( mToleranceBox->value() ) );
  else
    newItem = new QTableWidgetItem( QString( "No tolerance" ) );

  mTestTable->setItem( row, 3, newItem );

  QString layer1ID, layer2ID;
  // add layer ids to hidden columns
  // -1 for "No layer" string
  if ( mTestConfMap[test].useSecondLayer )
    layer2ID = mLayerIds[mLayer2Box->currentIndex() - 1];
  else
    layer2ID = "No layer";

  layer1ID = mLayerIds[mLayer1Box->currentIndex() - 1];

  //TODO: use setItemData (or something like that) instead of hidden columns
  newItem = new QTableWidgetItem( layer1ID );
  mTestTable->setItem( row, 4, newItem );
  newItem = new QTableWidgetItem( layer2ID );
  mTestTable->setItem( row, 5, newItem );

  // save state to the project file.....
  QString postfix = QString( "%1" ).arg( row );
  QgsProject* project = QgsProject::instance();

  project->writeEntry( "Topol", "/testCount", row + 1 );
  project->writeEntry( "Topol", "/testname_" + postfix, test );
  project->writeEntry( "Topol", "/tolerance_" + postfix, QString( "%1" ).arg( mToleranceBox->value() ) );
  project->writeEntry( "Topol", "/layer1_" + postfix, layer1ID );
  project->writeEntry( "Topol", "/layer2_" + postfix, layer2ID );

  // reset controls to default
  mTestBox->setCurrentIndex( 0 );
  mLayer1Box->setCurrentIndex( 0 );
  mLayer2Box->setCurrentIndex( 0 );
  mToleranceBox->setValue( 0 );
}

void rulesDialog::deleteTest()
{
  int row = mTestTable->currentRow();
  if ( 0 <= row && row < mTestTable->rowCount() )
    mTestTable->removeRow( row );
}

void rulesDialog::updateRuleItems( const QString &layerName )
{
  mTestBox->clear();

  if ( layerName == "No layer" )
  {
    return;
  }

  QString layerId = mLayer1Box->itemData( mLayer1Box->currentIndex() ).toString();

  QgsMapLayerRegistry* layerRegistry = QgsMapLayerRegistry::instance();
  QgsVectorLayer* vlayer = ( QgsVectorLayer* )layerRegistry->mapLayers()[layerId];

  for ( QMap<QString, TopologyRule>::iterator it = mTestConfMap.begin(); it != mTestConfMap.end(); ++it )
  {
    TopologyRule rule = it.value();
    if ( rule.layer1AcceptsType( vlayer->geometryType() ) )
    {
      mTestBox->addItem( it.key() );
    }

  }
}
