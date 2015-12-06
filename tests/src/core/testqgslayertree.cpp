/***************************************************************************
  testqgslayertree.cpp
  --------------------------------------
  Date                 : September 2015
  Copyright            : (C) 2015 by Martin Dobias
  Email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <QtTest/QtTest>

#include <qgsapplication.h>
#include <qgslayertree.h>
#include <qgsmaplayerregistry.h>
#include <qgsvectorlayer.h>
#include <qgsvectorlayerdiagramprovider.h>
#include <qgsvectorlayerlabelprovider.h>
#include <qgscategorizedsymbolrendererv2.h>
#include <qgslayertreemodel.h>
#include <qgslayertreemodellegendnode.h>

class TestQgsLayerTree : public QObject
{
    Q_OBJECT
  public:
    TestQgsLayerTree() : mRoot( 0 ) {}
  private slots:
    void initTestCase();
    void cleanupTestCase();
    void testCheckStateParentToChild();
    void testCheckStateChildToParent();
    void testCheckStateMutuallyExclusive();
    void testCheckStateMutuallyExclusiveEdgeCases();
    void testShowHideAllSymbolNodes();

  private:

    QgsLayerTreeGroup* mRoot;

    Qt::CheckState childState( int childIndex )
    {
      return QgsLayerTree::toGroup( mRoot->children().at( childIndex ) )->isVisible();
    }
    void setChildState( int childIndex, Qt::CheckState state )
    {
      QgsLayerTree::toGroup( mRoot->children().at( childIndex ) )->setVisible( state );
    }
};

void TestQgsLayerTree::initTestCase()
{
  QgsApplication::init();
  QgsApplication::initQgis();

  mRoot = new QgsLayerTreeGroup();
  mRoot->addGroup( "grp1" );
  mRoot->addGroup( "grp2" );
  mRoot->addGroup( "grp3" );

  // all cases start with all items checked
}

void TestQgsLayerTree::cleanupTestCase()
{
  delete mRoot;
  QgsApplication::exitQgis();
}

void TestQgsLayerTree::testCheckStateParentToChild()
{
  mRoot->setVisible( Qt::Unchecked );

  // all children unchecked
  QCOMPARE( childState( 0 ), Qt::Unchecked );
  QCOMPARE( childState( 1 ), Qt::Unchecked );
  QCOMPARE( childState( 2 ), Qt::Unchecked );

  mRoot->setVisible( Qt::Checked );

  // all children checked
  QCOMPARE( childState( 0 ), Qt::Checked );
  QCOMPARE( childState( 1 ), Qt::Checked );
  QCOMPARE( childState( 2 ), Qt::Checked );
}

void TestQgsLayerTree::testCheckStateChildToParent()
{
  QCOMPARE( mRoot->isVisible(), Qt::Checked );

  // uncheck a child - parent should be partial
  setChildState( 0, Qt::Unchecked );
  QCOMPARE( mRoot->isVisible(), Qt::PartiallyChecked );
  setChildState( 1, Qt::Unchecked );
  QCOMPARE( mRoot->isVisible(), Qt::PartiallyChecked );

  // uncheck last child - parent should be unchecked
  setChildState( 2, Qt::Unchecked );
  QCOMPARE( mRoot->isVisible(), Qt::Unchecked );

  // go back to original state
  mRoot->setVisible( Qt::Checked );
}

void TestQgsLayerTree::testCheckStateMutuallyExclusive()
{
  mRoot->setIsMutuallyExclusive( true );

  // only first should be enabled
  QCOMPARE( childState( 0 ), Qt::Checked );
  QCOMPARE( childState( 1 ), Qt::Unchecked );
  QCOMPARE( childState( 2 ), Qt::Unchecked );
  QCOMPARE( mRoot->isVisible(), Qt::Checked ); // fully checked, not just partial

  // switch to some other child
  setChildState( 2, Qt::Checked );
  QCOMPARE( childState( 0 ), Qt::Unchecked );
  QCOMPARE( childState( 1 ), Qt::Unchecked );
  QCOMPARE( childState( 2 ), Qt::Checked );
  QCOMPARE( mRoot->isVisible(), Qt::Checked );

  // now uncheck the root
  mRoot->setVisible( Qt::Unchecked );
  QCOMPARE( childState( 0 ), Qt::Unchecked );
  QCOMPARE( childState( 1 ), Qt::Unchecked );
  QCOMPARE( childState( 2 ), Qt::Unchecked );
  QCOMPARE( mRoot->isVisible(), Qt::Unchecked );

  // check one of the children - should also check the root
  setChildState( 2, Qt::Checked );
  QCOMPARE( childState( 0 ), Qt::Unchecked );
  QCOMPARE( childState( 1 ), Qt::Unchecked );
  QCOMPARE( childState( 2 ), Qt::Checked );
  QCOMPARE( mRoot->isVisible(), Qt::Checked );

  // uncheck the child - should also uncheck the root
  setChildState( 2, Qt::Unchecked );
  QCOMPARE( childState( 0 ), Qt::Unchecked );
  QCOMPARE( childState( 1 ), Qt::Unchecked );
  QCOMPARE( childState( 2 ), Qt::Unchecked );
  QCOMPARE( mRoot->isVisible(), Qt::Unchecked );

  // check the root back - should have the same node
  mRoot->setVisible( Qt::Checked );
  QCOMPARE( childState( 0 ), Qt::Unchecked );
  QCOMPARE( childState( 1 ), Qt::Unchecked );
  QCOMPARE( childState( 2 ), Qt::Checked );
  QCOMPARE( mRoot->isVisible(), Qt::Checked );

  // remove a child
  mRoot->removeChildNode( mRoot->children().at( 0 ) );
  QCOMPARE( childState( 0 ), Qt::Unchecked );
  QCOMPARE( childState( 1 ), Qt::Checked );
  QCOMPARE( mRoot->isVisible(), Qt::Checked );

  // add the group back - will not be checked
  mRoot->insertGroup( 0, "grp1" );
  QCOMPARE( childState( 0 ), Qt::Unchecked );
  QCOMPARE( childState( 1 ), Qt::Unchecked );
  QCOMPARE( childState( 2 ), Qt::Checked );
  QCOMPARE( mRoot->isVisible(), Qt::Checked );

  // remove a child that is checked
  mRoot->removeChildNode( mRoot->children().at( 2 ) );
  QCOMPARE( childState( 0 ), Qt::Unchecked );
  QCOMPARE( childState( 1 ), Qt::Unchecked );
  QCOMPARE( mRoot->isVisible(), Qt::Unchecked );

  // check the root again - first item should be checked
  mRoot->setVisible( Qt::Checked );
  QCOMPARE( childState( 0 ), Qt::Checked );
  QCOMPARE( childState( 1 ), Qt::Unchecked );
  QCOMPARE( mRoot->isVisible(), Qt::Checked );

  // add the item back
  mRoot->addGroup( "grp3" );
  QCOMPARE( childState( 0 ), Qt::Checked );
  QCOMPARE( childState( 1 ), Qt::Unchecked );
  QCOMPARE( childState( 2 ), Qt::Unchecked );
  QCOMPARE( mRoot->isVisible(), Qt::Checked );

  mRoot->setIsMutuallyExclusive( false );

  QCOMPARE( mRoot->isVisible(), Qt::PartiallyChecked );

  // go back to original state
  mRoot->setVisible( Qt::Checked );
}

void TestQgsLayerTree::testCheckStateMutuallyExclusiveEdgeCases()
{
  // starting with empty mutually exclusive group
  QgsLayerTreeGroup* root2 = new QgsLayerTreeGroup();
  root2->setIsMutuallyExclusive( true );
  root2->addGroup( "1" );
  QCOMPARE( QgsLayerTree::toGroup( root2->children().at( 0 ) )->isVisible(), Qt::Checked );
  root2->addGroup( "2" );
  QCOMPARE( QgsLayerTree::toGroup( root2->children().at( 0 ) )->isVisible(), Qt::Checked );
  QCOMPARE( QgsLayerTree::toGroup( root2->children().at( 1 ) )->isVisible(), Qt::Unchecked );
  delete root2;

  // check-uncheck the only child
  QgsLayerTreeGroup* root3 = new QgsLayerTreeGroup();
  root3->setIsMutuallyExclusive( true );
  root3->addGroup( "1" );
  QCOMPARE( QgsLayerTree::toGroup( root3->children().at( 0 ) )->isVisible(), Qt::Checked );
  QgsLayerTree::toGroup( root3->children().at( 0 ) )->setVisible( Qt::Unchecked );
  QCOMPARE( QgsLayerTree::toGroup( root3->children().at( 0 ) )->isVisible(), Qt::Unchecked );
  QCOMPARE( root3->isVisible(), Qt::Unchecked );
  QgsLayerTree::toGroup( root3->children().at( 0 ) )->setVisible( Qt::Checked );
  QCOMPARE( QgsLayerTree::toGroup( root3->children().at( 0 ) )->isVisible(), Qt::Checked );
  QCOMPARE( root3->isVisible(), Qt::Checked );
  delete root3;
}

void TestQgsLayerTree::testShowHideAllSymbolNodes()
{
  //new memory layer
  QgsVectorLayer* vl = new QgsVectorLayer( "Point?field=col1:integer", "vl", "memory" );
  QVERIFY( vl->isValid() );

  QgsMapLayerRegistry::instance()->addMapLayers( QList<QgsMapLayer*>() << vl );

  //create a categorized renderer for layer
  QgsCategorizedSymbolRendererV2* renderer = new QgsCategorizedSymbolRendererV2();
  renderer->setClassAttribute( "col1" );
  renderer->setSourceSymbol( QgsSymbolV2::defaultSymbol( QGis::Point ) );
  renderer->addCategory( QgsRendererCategoryV2( "a", QgsSymbolV2::defaultSymbol( QGis::Point ), "a" ) );
  renderer->addCategory( QgsRendererCategoryV2( "b", QgsSymbolV2::defaultSymbol( QGis::Point ), "b" ) );
  renderer->addCategory( QgsRendererCategoryV2( "c", QgsSymbolV2::defaultSymbol( QGis::Point ), "c" ) );
  vl->setRendererV2( renderer );

  //create legend with symbology nodes for categorized renderer
  QgsLayerTreeGroup* root = new QgsLayerTreeGroup();
  QgsLayerTreeLayer* n = new QgsLayerTreeLayer( vl );
  root->addChildNode( n );
  QgsLayerTreeModel* m = new QgsLayerTreeModel( root, 0 );
  m->refreshLayerLegend( n );

  //test that all nodes are initially checked
  QList<QgsLayerTreeModelLegendNode*> nodes = m->layerLegendNodes( n );
  QCOMPARE( nodes.length(), 3 );
  Q_FOREACH ( QgsLayerTreeModelLegendNode* ln, nodes )
  {
    QVERIFY( ln->data( Qt::CheckStateRole ) == Qt::Checked );
  }
  //uncheck all and test that all nodes are unchecked
  static_cast< QgsSymbolV2LegendNode* >( nodes.at( 0 ) )->uncheckAllItems();
  Q_FOREACH ( QgsLayerTreeModelLegendNode* ln, nodes )
  {
    QVERIFY( ln->data( Qt::CheckStateRole ) == Qt::Unchecked );
  }
  //check all and test that all nodes are checked
  static_cast< QgsSymbolV2LegendNode* >( nodes.at( 0 ) )->checkAllItems();
  Q_FOREACH ( QgsLayerTreeModelLegendNode* ln, nodes )
  {
    QVERIFY( ln->data( Qt::CheckStateRole ) == Qt::Checked );
  }

  //cleanup
  delete m;
  delete root;
  QgsMapLayerRegistry::instance()->removeMapLayers( QList<QgsMapLayer*>() << vl );
}


QTEST_MAIN( TestQgsLayerTree )
#include "testqgslayertree.moc"
