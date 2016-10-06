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
#include <qgscategorizedsymbolrenderer.h>
#include <qgsgraduatedsymbolrenderer.h>
#include <qgsrulebasedrenderer.h>
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
    void testFindLegendNode();
    void testLegendSymbolCategorized();
    void testLegendSymbolGraduated();
    void testLegendSymbolRuleBased();

  private:

    QgsLayerTreeGroup* mRoot;

    void testRendererLegend( QgsFeatureRenderer* renderer );

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
  QgsCategorizedSymbolRenderer* renderer = new QgsCategorizedSymbolRenderer();
  renderer->setClassAttribute( "col1" );
  renderer->setSourceSymbol( QgsSymbol::defaultSymbol( QgsWkbTypes::PointGeometry ) );
  renderer->addCategory( QgsRendererCategory( "a", QgsSymbol::defaultSymbol( QgsWkbTypes::PointGeometry ), "a" ) );
  renderer->addCategory( QgsRendererCategory( "b", QgsSymbol::defaultSymbol( QgsWkbTypes::PointGeometry ), "b" ) );
  renderer->addCategory( QgsRendererCategory( "c", QgsSymbol::defaultSymbol( QgsWkbTypes::PointGeometry ), "c" ) );
  vl->setRenderer( renderer );

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
  static_cast< QgsSymbolLegendNode* >( nodes.at( 0 ) )->uncheckAllItems();
  Q_FOREACH ( QgsLayerTreeModelLegendNode* ln, nodes )
  {
    QVERIFY( ln->data( Qt::CheckStateRole ) == Qt::Unchecked );
  }
  //check all and test that all nodes are checked
  static_cast< QgsSymbolLegendNode* >( nodes.at( 0 ) )->checkAllItems();
  Q_FOREACH ( QgsLayerTreeModelLegendNode* ln, nodes )
  {
    QVERIFY( ln->data( Qt::CheckStateRole ) == Qt::Checked );
  }

  //cleanup
  delete m;
  delete root;
  QgsMapLayerRegistry::instance()->removeMapLayers( QList<QgsMapLayer*>() << vl );
}

void TestQgsLayerTree::testFindLegendNode()
{
  //new memory layer
  QgsVectorLayer* vl = new QgsVectorLayer( "Point?field=col1:integer", "vl", "memory" );
  QVERIFY( vl->isValid() );

  QgsMapLayerRegistry::instance()->addMapLayers( QList<QgsMapLayer*>() << vl );

  //create a categorized renderer for layer
  QgsCategorizedSymbolRenderer* renderer = new QgsCategorizedSymbolRenderer();
  renderer->setClassAttribute( "col1" );
  renderer->setSourceSymbol( QgsSymbol::defaultSymbol( QgsWkbTypes::PointGeometry ) );
  renderer->addCategory( QgsRendererCategory( "a", QgsSymbol::defaultSymbol( QgsWkbTypes::PointGeometry ), "a" ) );
  renderer->addCategory( QgsRendererCategory( "b", QgsSymbol::defaultSymbol( QgsWkbTypes::PointGeometry ), "b" ) );
  renderer->addCategory( QgsRendererCategory( "c", QgsSymbol::defaultSymbol( QgsWkbTypes::PointGeometry ), "c" ) );
  vl->setRenderer( renderer );

  //create legend with symbology nodes for categorized renderer
  QgsLayerTreeGroup* root = new QgsLayerTreeGroup();
  QgsLayerTreeModel* m = new QgsLayerTreeModel( root, 0 );
  QVERIFY( !m->findLegendNode( QString( "id" ), QString( "rule" ) ) );
  QgsLayerTreeLayer* n = new QgsLayerTreeLayer( vl );
  root->addChildNode( n );
  m->refreshLayerLegend( n );
  QVERIFY( !m->findLegendNode( QString( "id" ), QString( "rule" ) ) );
  QVERIFY( !m->findLegendNode( QString( "vl" ), QString( "rule" ) ) );

  QgsLegendSymbolListV2 symbolList = renderer->legendSymbolItemsV2();
  Q_FOREACH ( const QgsLegendSymbolItem& symbol, symbolList )
  {
    QgsLayerTreeModelLegendNode* found = m->findLegendNode( vl->id(), symbol.ruleKey() );
    QVERIFY( found );
    QCOMPARE( found->layerNode()->layerId(), vl->id() );
    QCOMPARE( found->data( Qt::DisplayRole ).toString(), symbol.label() );
  }

  //cleanup
  delete m;
  delete root;
  QgsMapLayerRegistry::instance()->removeMapLayers( QList<QgsMapLayer*>() << vl );
}

void TestQgsLayerTree::testLegendSymbolCategorized()
{
  //test retrieving/setting a categorized renderer's symbol through the legend node
  QgsCategorizedSymbolRenderer* renderer = new QgsCategorizedSymbolRenderer();
  renderer->setClassAttribute( "col1" );
  renderer->setSourceSymbol( QgsSymbol::defaultSymbol( QgsWkbTypes::PointGeometry ) );
  QgsStringMap props;
  props.insert( "color", "#ff0000" );
  renderer->addCategory( QgsRendererCategory( "a", QgsMarkerSymbol::createSimple( props ), "a" ) );
  props.insert( "color", "#00ff00" );
  renderer->addCategory( QgsRendererCategory( "b", QgsMarkerSymbol::createSimple( props ), "b" ) );
  props.insert( "color", "#0000ff" );
  renderer->addCategory( QgsRendererCategory( "c", QgsMarkerSymbol::createSimple( props ), "c" ) );
  testRendererLegend( renderer );
}

void TestQgsLayerTree::testLegendSymbolGraduated()
{
  //test retrieving/setting a graduated renderer's symbol through the legend node
  QgsGraduatedSymbolRenderer* renderer = new QgsGraduatedSymbolRenderer();
  renderer->setClassAttribute( "col1" );
  renderer->setSourceSymbol( QgsSymbol::defaultSymbol( QgsWkbTypes::PointGeometry ) );
  QgsStringMap props;
  props.insert( "color", "#ff0000" );
  renderer->addClass( QgsRendererRange( 1, 2, QgsMarkerSymbol::createSimple( props ), "a" ) );
  props.insert( "color", "#00ff00" );
  renderer->addClass( QgsRendererRange( 2, 3, QgsMarkerSymbol::createSimple( props ), "b" ) );
  props.insert( "color", "#0000ff" );
  renderer->addClass( QgsRendererRange( 3, 4, QgsMarkerSymbol::createSimple( props ), "c" ) );
  testRendererLegend( renderer );
}

void TestQgsLayerTree::testLegendSymbolRuleBased()
{
  //test retrieving/setting a rule based renderer's symbol through the legend node
  QgsRuleBasedRenderer::Rule* root = new QgsRuleBasedRenderer::Rule( 0 );
  QgsStringMap props;
  props.insert( "color", "#ff0000" );
  root->appendChild( new QgsRuleBasedRenderer::Rule( QgsMarkerSymbol::createSimple( props ), 0, 0, "\"col1\"=1" ) );
  props.insert( "color", "#00ff00" );
  root->appendChild( new QgsRuleBasedRenderer::Rule( QgsMarkerSymbol::createSimple( props ), 0, 0, "\"col1\"=2" ) );
  props.insert( "color", "#0000ff" );
  root->appendChild( new QgsRuleBasedRenderer::Rule( QgsMarkerSymbol::createSimple( props ), 0, 0, "ELSE" ) );
  QgsRuleBasedRenderer* renderer = new QgsRuleBasedRenderer( root );
  testRendererLegend( renderer );
}

void TestQgsLayerTree::testRendererLegend( QgsFeatureRenderer* renderer )
{
  // runs renderer legend through a bunch of legend symbol tests

  // NOTE: test expects renderer with at least 3 symbol nodes, where the initial symbol colors should be:
  // #ff0000, #00ff00, #0000ff

  //new memory layer
  QgsVectorLayer* vl = new QgsVectorLayer( "Point?field=col1:integer", "vl", "memory" );
  QVERIFY( vl->isValid() );

  QgsMapLayerRegistry::instance()->addMapLayers( QList<QgsMapLayer*>() << vl );
  vl->setRenderer( renderer );

  //create legend with symbology nodes for renderer
  QgsLayerTreeGroup* root = new QgsLayerTreeGroup();
  QgsLayerTreeLayer* n = new QgsLayerTreeLayer( vl );
  root->addChildNode( n );
  QgsLayerTreeModel* m = new QgsLayerTreeModel( root, 0 );
  m->refreshLayerLegend( n );

  //test initial symbol
  QgsLegendSymbolListV2 symbolList = renderer->legendSymbolItemsV2();
  Q_FOREACH ( const QgsLegendSymbolItem& symbol, symbolList )
  {
    QgsSymbolLegendNode* symbolNode = dynamic_cast< QgsSymbolLegendNode* >( m->findLegendNode( vl->id(), symbol.ruleKey() ) );
    QVERIFY( symbolNode );
    QCOMPARE( symbolNode->symbol()->color(), symbol.symbol()->color() );
  }
  //try changing a symbol's color
  QgsSymbolLegendNode* symbolNode = dynamic_cast< QgsSymbolLegendNode* >( m->findLegendNode( vl->id(), symbolList.at( 1 ).ruleKey() ) );
  QVERIFY( symbolNode );
  QgsSymbol* newSymbol = symbolNode->symbol()->clone();
  newSymbol->setColor( QColor( 255, 255, 0 ) );
  symbolNode->setSymbol( newSymbol );
  QCOMPARE( symbolNode->symbol()->color(), QColor( 255, 255, 0 ) );
  //test that symbol change was sent to renderer
  symbolList = renderer->legendSymbolItemsV2();
  QCOMPARE( symbolList.at( 1 ).symbol()->color(), QColor( 255, 255, 0 ) );

  //another test - check directly setting symbol at renderer
  QgsStringMap props;
  props.insert( "color", "#00ffff" );
  renderer->setLegendSymbolItem( symbolList.at( 2 ).ruleKey(), QgsMarkerSymbol::createSimple( props ) );
  m->refreshLayerLegend( n );
  symbolNode = dynamic_cast< QgsSymbolLegendNode* >( m->findLegendNode( vl->id(), symbolList.at( 2 ).ruleKey() ) );
  QVERIFY( symbolNode );
  QCOMPARE( symbolNode->symbol()->color(), QColor( 0, 255, 255 ) );

  //cleanup
  delete m;
  delete root;
  QgsMapLayerRegistry::instance()->removeMapLayers( QList<QgsMapLayer*>() << vl );
}


QTEST_MAIN( TestQgsLayerTree )
#include "testqgslayertree.moc"
