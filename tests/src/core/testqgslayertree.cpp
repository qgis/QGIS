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

#include "qgstest.h"

#include <qgsapplication.h>
#include <qgslayertree.h>
#include <qgsproject.h>
#include <qgsvectorlayer.h>
#include <qgsvectorlayerdiagramprovider.h>
#include <qgsvectorlayerlabelprovider.h>
#include <qgscategorizedsymbolrenderer.h>
#include <qgsgraduatedsymbolrenderer.h>
#include <qgsrulebasedrenderer.h>
#include <qgslayertreemodel.h>
#include <qgslayertreemodellegendnode.h>
#include <qgslayertreeutils.h>
#include <qgssettings.h>
#include "qgslegendsettings.h"
#include "qgsmarkersymbol.h"
#include "qgsannotationlayer.h"
#include <QSignalSpy>

class TestQgsLayerTree : public QObject
{
    Q_OBJECT
  public:
    TestQgsLayerTree() = default;
  private slots:
    void initTestCase();
    void cleanupTestCase();
    void testGroupNameChanged();
    void testLayerNameChanged();
    void testCheckStateHiearchical();
    void testCheckStateMutuallyExclusive();
    void testCheckStateMutuallyExclusiveEdgeCases();
    void testRestrictedSymbolSize();
    void testShowHideAllSymbolNodes();
    void testFindLegendNode();
    void testLegendSymbolCategorized();
    void testLegendSymbolGraduated();
    void testLegendSymbolRuleBased();
    void testResolveReferences();
    void testEmbeddedGroup();
    void testFindLayer();
    void testLayerDeleted();
    void testFindGroups();
    void testFindNestedGroups();
    void testUtilsCollectMapLayers();
    void testUtilsCountMapLayers();
    void testSymbolText();
    void testNodeDepth();
    void testRasterSymbolNode();
    void testLayersEditable();
    void testInsertLayerBelow();

  private:

    QgsLayerTreeGroup *mRoot = nullptr;

    void testRendererLegend( QgsFeatureRenderer *renderer );

    bool childVisiblity( int childIndex ) const
    {
      return mRoot->children().at( childIndex )->isVisible();
    }

    bool visibilityChecked( int childIndex ) const
    {
      return mRoot->children().at( childIndex )->itemVisibilityChecked();
    }

    void setVisibilityChecked( int childIndex, bool state )
    {
      mRoot->children().at( childIndex )->setItemVisibilityChecked( state );
    }
};

void TestQgsLayerTree::initTestCase()
{
  QgsApplication::init();
  QgsApplication::initQgis();

  QCoreApplication::setOrganizationName( QStringLiteral( "QGIS" ) );
  QCoreApplication::setOrganizationDomain( QStringLiteral( "qgis.org" ) );
  QCoreApplication::setApplicationName( QStringLiteral( "QGIS-TEST" ) );

  mRoot = new QgsLayerTreeGroup();
  mRoot->addGroup( QStringLiteral( "grp1" ) );
  mRoot->addGroup( QStringLiteral( "grp2" ) );
  mRoot->addGroup( QStringLiteral( "grp3" ) );

  // all cases start with all items checked
}

void TestQgsLayerTree::cleanupTestCase()
{
  delete mRoot;
  QgsApplication::exitQgis();
}

void TestQgsLayerTree::testGroupNameChanged()
{
  QgsLayerTreeNode *secondGroup = mRoot->children()[1];

  QSignalSpy spy( mRoot, SIGNAL( nameChanged( QgsLayerTreeNode *, QString ) ) );
  secondGroup->setName( QStringLiteral( "grp2+" ) );

  QCOMPARE( secondGroup->name(), QString( "grp2+" ) );

  QCOMPARE( spy.count(), 1 );
  const QList<QVariant> arguments = spy.takeFirst();
  QCOMPARE( arguments.at( 0 ).value<QgsLayerTreeNode *>(), secondGroup );
  QCOMPARE( arguments.at( 1 ).toString(), QString( "grp2+" ) );

  secondGroup->setName( QStringLiteral( "grp2" ) );
  QCOMPARE( secondGroup->name(), QString( "grp2" ) );
}

void TestQgsLayerTree::testLayerNameChanged()
{
  QgsVectorLayer *vl = new QgsVectorLayer( QStringLiteral( "Point?field=col1:integer" ), QStringLiteral( "vl" ), QStringLiteral( "memory" ) );
  QVERIFY( vl->isValid() );

  QgsLayerTreeLayer *n = new QgsLayerTreeLayer( vl->id(), vl->name() );
  mRoot->addChildNode( n );

  QSignalSpy spy( mRoot, SIGNAL( nameChanged( QgsLayerTreeNode *, QString ) ) );

  QCOMPARE( n->name(), QString( "vl" ) );
  n->setName( QStringLiteral( "changed 1" ) );

  QCOMPARE( n->name(), QString( "changed 1" ) );
  QCOMPARE( spy.count(), 1 );
  QList<QVariant> arguments = spy.takeFirst();
  QCOMPARE( arguments.at( 0 ).value<QgsLayerTreeNode *>(), n );
  QCOMPARE( arguments.at( 1 ).toString(), QString( "changed 1" ) );

  QgsProject project;
  project.addMapLayer( vl );
  n->resolveReferences( &project );

  // set name via map layer
  vl->setName( QStringLiteral( "changed 2" ) );
  QCOMPARE( n->name(), QString( "changed 2" ) );
  QCOMPARE( spy.count(), 1 );
  arguments = spy.takeFirst();
  QCOMPARE( arguments.at( 0 ).value<QgsLayerTreeNode *>(), n );
  QCOMPARE( arguments.at( 1 ).toString(), QString( "changed 2" ) );

  // set name via layer tree
  n->setName( QStringLiteral( "changed 3" ) );
  QCOMPARE( n->name(), QString( "changed 3" ) );
  QCOMPARE( spy.count(), 1 );
  arguments = spy.takeFirst();
  QCOMPARE( arguments.at( 0 ).value<QgsLayerTreeNode *>(), n );
  QCOMPARE( arguments.at( 1 ).toString(), QString( "changed 3" ) );

  mRoot->removeChildNode( n );
}

void TestQgsLayerTree::testCheckStateHiearchical()
{
  mRoot->setItemVisibilityCheckedRecursive( false );
  QCOMPARE( mRoot->isItemVisibilityCheckedRecursive(), false );
  QCOMPARE( mRoot->isItemVisibilityUncheckedRecursive(), true );
  QCOMPARE( visibilityChecked( 0 ), false );
  QCOMPARE( visibilityChecked( 1 ), false );
  QCOMPARE( visibilityChecked( 2 ), false );

  mRoot->children().at( 0 )->setItemVisibilityCheckedParentRecursive( true );
  QCOMPARE( mRoot->itemVisibilityChecked(), true );

  QCOMPARE( mRoot->isItemVisibilityCheckedRecursive(), false );
  QCOMPARE( mRoot->isItemVisibilityUncheckedRecursive(), false );

  mRoot->setItemVisibilityCheckedRecursive( true );
  QCOMPARE( mRoot->isItemVisibilityCheckedRecursive(), true );
  QCOMPARE( mRoot->isItemVisibilityUncheckedRecursive(), false );
  QCOMPARE( visibilityChecked( 0 ), true );
  QCOMPARE( visibilityChecked( 1 ), true );
  QCOMPARE( visibilityChecked( 2 ), true );
}

void TestQgsLayerTree::testCheckStateMutuallyExclusive()
{
  mRoot->setIsMutuallyExclusive( true );

  // only first should be enabled
  QCOMPARE( childVisiblity( 0 ), true );
  QCOMPARE( childVisiblity( 1 ), false );
  QCOMPARE( childVisiblity( 2 ), false );
  QCOMPARE( mRoot->isVisible(), true ); // fully checked, not just partial

  // switch to some other child
  setVisibilityChecked( 2, true );
  QCOMPARE( childVisiblity( 0 ), false );
  QCOMPARE( childVisiblity( 1 ), false );
  QCOMPARE( childVisiblity( 2 ), true );
  QCOMPARE( mRoot->isVisible(), true );

  // now uncheck the root
  mRoot->setItemVisibilityChecked( false );
  QCOMPARE( mRoot->itemVisibilityChecked(), false );

  QCOMPARE( childVisiblity( 0 ), false );
  QCOMPARE( childVisiblity( 1 ), false );
  QCOMPARE( visibilityChecked( 2 ), true );
  QCOMPARE( childVisiblity( 2 ), false );
  QCOMPARE( mRoot->isVisible(), false );

  // check one of the children - should not modify the root
  setVisibilityChecked( 2, true );
  QCOMPARE( childVisiblity( 0 ), false );
  QCOMPARE( childVisiblity( 1 ), false );
  QCOMPARE( childVisiblity( 2 ), false );
  QCOMPARE( mRoot->itemVisibilityChecked(), false );
  QCOMPARE( mRoot->isVisible(), false );

  // uncheck the child - should not modify the root
  setVisibilityChecked( 2, false );
  QCOMPARE( childVisiblity( 0 ), false );
  QCOMPARE( childVisiblity( 1 ), false );
  QCOMPARE( childVisiblity( 2 ), false );
  QCOMPARE( mRoot->itemVisibilityChecked(), false );
  QCOMPARE( mRoot->isVisible(), false );

  // check the root back
  mRoot->setItemVisibilityChecked( true );
  setVisibilityChecked( 2, true );
  QCOMPARE( childVisiblity( 0 ), false );
  QCOMPARE( childVisiblity( 1 ), false );
  QCOMPARE( childVisiblity( 2 ), true );
  QCOMPARE( mRoot->isVisible(), true );

  // remove a child
  mRoot->removeChildNode( mRoot->children().at( 0 ) );
  QCOMPARE( childVisiblity( 0 ), false );
  QCOMPARE( childVisiblity( 1 ), true );
  QCOMPARE( mRoot->isVisible(), true );

  // add the group back - will not be checked
  mRoot->insertGroup( 0, QStringLiteral( "grp1" ) );
  QCOMPARE( childVisiblity( 0 ), false );
  QCOMPARE( childVisiblity( 1 ), false );
  QCOMPARE( childVisiblity( 2 ), true );
  QCOMPARE( mRoot->isVisible(), true );

  // remove a child that is checked
  mRoot->removeChildNode( mRoot->children().at( 2 ) );
  QCOMPARE( childVisiblity( 0 ), false );
  QCOMPARE( childVisiblity( 1 ), false );
  QCOMPARE( mRoot->isVisible(), true );

  // add the item back
  setVisibilityChecked( 0, true );
  mRoot->addGroup( QStringLiteral( "grp3" ) );
  QCOMPARE( childVisiblity( 0 ), true );
  QCOMPARE( childVisiblity( 1 ), false );
  QCOMPARE( childVisiblity( 2 ), false );
  QCOMPARE( mRoot->isVisible(), true );

  mRoot->setIsMutuallyExclusive( false );

  // go back to original state
  mRoot->setItemVisibilityChecked( true );
}

void TestQgsLayerTree::testCheckStateMutuallyExclusiveEdgeCases()
{
  // starting with empty mutually exclusive group
  QgsLayerTreeGroup *root2 = new QgsLayerTreeGroup();
  root2->setIsMutuallyExclusive( true );
  root2->addGroup( QStringLiteral( "1" ) );
  QCOMPARE( QgsLayerTree::toGroup( root2->children().at( 0 ) )->isVisible(), true );
  root2->addGroup( QStringLiteral( "2" ) );
  QCOMPARE( QgsLayerTree::toGroup( root2->children().at( 0 ) )->isVisible(), true );
  QCOMPARE( QgsLayerTree::toGroup( root2->children().at( 1 ) )->isVisible(), false );
  delete root2;

  // check-uncheck the only child
  QgsLayerTreeGroup *root3 = new QgsLayerTreeGroup();
  root3->setIsMutuallyExclusive( true );
  root3->addGroup( QStringLiteral( "1" ) );
  QCOMPARE( QgsLayerTree::toGroup( root3->children().at( 0 ) )->isVisible(), true );
  QgsLayerTree::toGroup( root3->children().at( 0 ) )->setItemVisibilityChecked( false );
  QCOMPARE( QgsLayerTree::toGroup( root3->children().at( 0 ) )->isVisible(), false );
  QCOMPARE( root3->isVisible(), true );
  QgsLayerTree::toGroup( root3->children().at( 0 ) )->setItemVisibilityChecked( true );
  QCOMPARE( QgsLayerTree::toGroup( root3->children().at( 0 ) )->isVisible(), true );
  QCOMPARE( root3->isVisible(), true );
  delete root3;
}

void TestQgsLayerTree::testRestrictedSymbolSize()
{
  QgsSettings settings;
  settings.setValue( "/qgis/legendsymbolMaximumSize", 15.0 );

  //new memory layer
  QgsVectorLayer *vl = new QgsVectorLayer( QStringLiteral( "Point?field=col1:integer" ), QStringLiteral( "vl" ), QStringLiteral( "memory" ) );
  QVERIFY( vl->isValid() );

  QgsProject project;
  project.addMapLayer( vl );

  QgsMarkerSymbol *symbol = static_cast< QgsMarkerSymbol * >( QgsSymbol::defaultSymbol( QgsWkbTypes::PointGeometry ) );
  symbol->setSize( 500.0 );
  symbol->setSizeUnit( QgsUnitTypes::RenderMapUnits );

  //create a categorized renderer for layer
  QgsCategorizedSymbolRenderer *renderer = new QgsCategorizedSymbolRenderer();
  renderer->setClassAttribute( QStringLiteral( "col1" ) );
  renderer->setSourceSymbol( symbol->clone() );
  renderer->addCategory( QgsRendererCategory( "a", symbol->clone(), QStringLiteral( "a" ) ) );
  renderer->addCategory( QgsRendererCategory( "b", symbol->clone(), QStringLiteral( "b" ) ) );
  vl->setRenderer( renderer );

  //create legend with symbology nodes for categorized renderer
  QgsLayerTree *root = new QgsLayerTree();
  QgsLayerTreeLayer *n = new QgsLayerTreeLayer( vl );
  root->addChildNode( n );
  QgsLayerTreeModel *m = new QgsLayerTreeModel( root, nullptr );
  m->setLegendMapViewData( 10, 96, 10 );

  const QList<QgsLayerTreeModelLegendNode *> nodes = m->layerLegendNodes( n );
  const QSize minimumSize = static_cast< QgsSymbolLegendNode *>( nodes.at( 0 ) )->minimumIconSize();
  QCOMPARE( minimumSize.width(), 52 );

  //cleanup
  delete m;
  delete root;
}

void TestQgsLayerTree::testShowHideAllSymbolNodes()
{
  //new memory layer
  QgsVectorLayer *vl = new QgsVectorLayer( QStringLiteral( "Point?field=col1:integer" ), QStringLiteral( "vl" ), QStringLiteral( "memory" ) );
  QVERIFY( vl->isValid() );

  QgsProject project;
  project.addMapLayer( vl );

  //create a categorized renderer for layer
  QgsCategorizedSymbolRenderer *renderer = new QgsCategorizedSymbolRenderer();
  renderer->setClassAttribute( QStringLiteral( "col1" ) );
  renderer->setSourceSymbol( QgsSymbol::defaultSymbol( QgsWkbTypes::PointGeometry ) );
  renderer->addCategory( QgsRendererCategory( "a", QgsSymbol::defaultSymbol( QgsWkbTypes::PointGeometry ), QStringLiteral( "a" ) ) );
  renderer->addCategory( QgsRendererCategory( "b", QgsSymbol::defaultSymbol( QgsWkbTypes::PointGeometry ), QStringLiteral( "b" ) ) );
  renderer->addCategory( QgsRendererCategory( "c", QgsSymbol::defaultSymbol( QgsWkbTypes::PointGeometry ), QStringLiteral( "c" ) ) );
  vl->setRenderer( renderer );

  //create legend with symbology nodes for categorized renderer
  QgsLayerTree *root = new QgsLayerTree();
  QgsLayerTreeLayer *n = new QgsLayerTreeLayer( vl );
  root->addChildNode( n );
  QgsLayerTreeModel *m = new QgsLayerTreeModel( root, nullptr );
  m->refreshLayerLegend( n );

  //test that all nodes are initially checked
  const QList<QgsLayerTreeModelLegendNode *> nodes = m->layerLegendNodes( n );
  QCOMPARE( nodes.length(), 3 );
  for ( QgsLayerTreeModelLegendNode *ln : nodes )
  {
    QVERIFY( ln->data( Qt::CheckStateRole ) == Qt::Checked );
  }
  //uncheck all and test that all nodes are unchecked
  static_cast< QgsSymbolLegendNode * >( nodes.at( 0 ) )->uncheckAllItems();
  for ( QgsLayerTreeModelLegendNode *ln : nodes )
  {
    QVERIFY( ln->data( Qt::CheckStateRole ) == Qt::Unchecked );
  }
  //check all and test that all nodes are checked
  static_cast< QgsSymbolLegendNode * >( nodes.at( 0 ) )->checkAllItems();
  for ( QgsLayerTreeModelLegendNode *ln : nodes )
  {
    QVERIFY( ln->data( Qt::CheckStateRole ) == Qt::Checked );
  }

  //cleanup
  delete m;
  delete root;
}

void TestQgsLayerTree::testFindLegendNode()
{
  //new memory layer
  QgsVectorLayer *vl = new QgsVectorLayer( QStringLiteral( "Point?field=col1:integer" ), QStringLiteral( "vl" ), QStringLiteral( "memory" ) );
  QVERIFY( vl->isValid() );

  QgsProject project;
  project.addMapLayer( vl );

  //create a categorized renderer for layer
  QgsCategorizedSymbolRenderer *renderer = new QgsCategorizedSymbolRenderer();
  renderer->setClassAttribute( QStringLiteral( "col1" ) );
  renderer->setSourceSymbol( QgsSymbol::defaultSymbol( QgsWkbTypes::PointGeometry ) );
  renderer->addCategory( QgsRendererCategory( "a", QgsSymbol::defaultSymbol( QgsWkbTypes::PointGeometry ), QStringLiteral( "a" ) ) );
  renderer->addCategory( QgsRendererCategory( "b", QgsSymbol::defaultSymbol( QgsWkbTypes::PointGeometry ), QStringLiteral( "b" ) ) );
  renderer->addCategory( QgsRendererCategory( "c", QgsSymbol::defaultSymbol( QgsWkbTypes::PointGeometry ), QStringLiteral( "c" ) ) );
  vl->setRenderer( renderer );

  //create legend with symbology nodes for categorized renderer
  QgsLayerTree *root = new QgsLayerTree();
  QgsLayerTreeModel *m = new QgsLayerTreeModel( root, nullptr );
  QVERIFY( !m->findLegendNode( QString( "id" ), QString( "rule" ) ) );
  QgsLayerTreeLayer *n = new QgsLayerTreeLayer( vl );
  root->addChildNode( n );
  m->refreshLayerLegend( n );
  QVERIFY( !m->findLegendNode( QString( "id" ), QString( "rule" ) ) );
  QVERIFY( !m->findLegendNode( QString( "vl" ), QString( "rule" ) ) );

  const QgsLegendSymbolList symbolList = renderer->legendSymbolItems();
  for ( const QgsLegendSymbolItem &symbol : symbolList )
  {
    QgsLayerTreeModelLegendNode *found = m->findLegendNode( vl->id(), symbol.ruleKey() );
    QVERIFY( found );
    QCOMPARE( found->layerNode()->layerId(), vl->id() );
    QCOMPARE( found->data( Qt::DisplayRole ).toString(), symbol.label() );
  }

  //cleanup
  delete m;
  delete root;
}

void TestQgsLayerTree::testLegendSymbolCategorized()
{
  //test retrieving/setting a categorized renderer's symbol through the legend node
  QgsCategorizedSymbolRenderer *renderer = new QgsCategorizedSymbolRenderer();
  renderer->setClassAttribute( QStringLiteral( "col1" ) );
  renderer->setSourceSymbol( QgsSymbol::defaultSymbol( QgsWkbTypes::PointGeometry ) );
  QVariantMap props;
  props.insert( QStringLiteral( "color" ), QStringLiteral( "#ff0000" ) );
  props.insert( QStringLiteral( "outline_color" ), QStringLiteral( "#000000" ) );
  renderer->addCategory( QgsRendererCategory( "a", QgsMarkerSymbol::createSimple( props ), QStringLiteral( "a" ) ) );
  props.insert( QStringLiteral( "color" ), QStringLiteral( "#00ff00" ) );
  renderer->addCategory( QgsRendererCategory( "b", QgsMarkerSymbol::createSimple( props ), QStringLiteral( "b" ) ) );
  props.insert( QStringLiteral( "color" ), QStringLiteral( "#0000ff" ) );
  renderer->addCategory( QgsRendererCategory( "c", QgsMarkerSymbol::createSimple( props ), QStringLiteral( "c" ) ) );
  testRendererLegend( renderer );
}

void TestQgsLayerTree::testLegendSymbolGraduated()
{
  //test retrieving/setting a graduated renderer's symbol through the legend node
  QgsGraduatedSymbolRenderer *renderer = new QgsGraduatedSymbolRenderer();
  renderer->setClassAttribute( QStringLiteral( "col1" ) );
  renderer->setSourceSymbol( QgsSymbol::defaultSymbol( QgsWkbTypes::PointGeometry ) );
  QVariantMap props;
  props.insert( QStringLiteral( "color" ), QStringLiteral( "#ff0000" ) );
  props.insert( QStringLiteral( "outline_color" ), QStringLiteral( "#000000" ) );
  renderer->addClass( QgsRendererRange( 1, 2, QgsMarkerSymbol::createSimple( props ), QStringLiteral( "a" ) ) );
  props.insert( QStringLiteral( "color" ), QStringLiteral( "#00ff00" ) );
  renderer->addClass( QgsRendererRange( 2, 3, QgsMarkerSymbol::createSimple( props ), QStringLiteral( "b" ) ) );
  props.insert( QStringLiteral( "color" ), QStringLiteral( "#0000ff" ) );
  renderer->addClass( QgsRendererRange( 3, 4, QgsMarkerSymbol::createSimple( props ), QStringLiteral( "c" ) ) );
  testRendererLegend( renderer );
}

void TestQgsLayerTree::testLegendSymbolRuleBased()
{
  //test retrieving/setting a rule based renderer's symbol through the legend node
  QgsRuleBasedRenderer::Rule *root = new QgsRuleBasedRenderer::Rule( nullptr );
  QVariantMap props;
  props.insert( QStringLiteral( "color" ), QStringLiteral( "#ff0000" ) );
  props.insert( QStringLiteral( "outline_color" ), QStringLiteral( "#000000" ) );
  root->appendChild( new QgsRuleBasedRenderer::Rule( QgsMarkerSymbol::createSimple( props ), 0, 0, QStringLiteral( "\"col1\"=1" ) ) );
  props.insert( QStringLiteral( "color" ), QStringLiteral( "#00ff00" ) );
  root->appendChild( new QgsRuleBasedRenderer::Rule( QgsMarkerSymbol::createSimple( props ), 0, 0, QStringLiteral( "\"col1\"=2" ) ) );
  props.insert( QStringLiteral( "color" ), QStringLiteral( "#0000ff" ) );
  root->appendChild( new QgsRuleBasedRenderer::Rule( QgsMarkerSymbol::createSimple( props ), 0, 0, QStringLiteral( "ELSE" ) ) );
  QgsRuleBasedRenderer *renderer = new QgsRuleBasedRenderer( root );
  testRendererLegend( renderer );
}

void TestQgsLayerTree::testResolveReferences()
{
  QgsVectorLayer *vl = new QgsVectorLayer( QStringLiteral( "Point?field=col1:integer" ), QStringLiteral( "vl" ), QStringLiteral( "memory" ) );
  QVERIFY( vl->isValid() );

  const QString n1id = vl->id();
  const QString n2id = QStringLiteral( "XYZ" );

  QgsMapLayer *nullLayer = nullptr; // QCOMPARE does not like nullptr directly

  QgsLayerTreeGroup *root = new QgsLayerTreeGroup();
  QgsLayerTreeLayer *n1 = new QgsLayerTreeLayer( n1id, vl->name() );
  QgsLayerTreeLayer *n2 = new QgsLayerTreeLayer( n2id, QStringLiteral( "invalid layer" ) );
  root->addChildNode( n1 );
  root->addChildNode( n2 );

  // layer object not yet accessible
  QCOMPARE( n1->layer(), nullLayer );
  QCOMPARE( n1->layerId(), n1id );
  QCOMPARE( n2->layer(), nullLayer );
  QCOMPARE( n2->layerId(), n2id );

  QgsProject project;
  project.addMapLayer( vl );

  root->resolveReferences( &project );

  // now the layer should be accessible
  QCOMPARE( n1->layer(), vl );
  QCOMPARE( n1->layerId(), n1id );
  QCOMPARE( n2->layer(), nullLayer );
  QCOMPARE( n2->layerId(), n2id );

  project.removeMapLayer( vl ); // deletes the layer

  // layer object not accessible anymore
  QCOMPARE( n1->layer(), nullLayer );
  QCOMPARE( n1->layerId(), n1id );
  QCOMPARE( n2->layer(), nullLayer );
  QCOMPARE( n2->layerId(), n2id );

  delete root;
}

void TestQgsLayerTree::testRendererLegend( QgsFeatureRenderer *renderer )
{
  // runs renderer legend through a bunch of legend symbol tests

  // NOTE: test expects renderer with at least 3 symbol nodes, where the initial symbol colors should be:
  // #ff0000, #00ff00, #0000ff

  //new memory layer
  QgsVectorLayer *vl = new QgsVectorLayer( QStringLiteral( "Point?field=col1:integer" ), QStringLiteral( "vl" ), QStringLiteral( "memory" ) );
  QVERIFY( vl->isValid() );

  QgsProject project;
  project.addMapLayer( vl );

  vl->setRenderer( renderer );

  //create legend with symbology nodes for renderer
  QgsLayerTree *root = new QgsLayerTree();
  QgsLayerTreeLayer *n = new QgsLayerTreeLayer( vl );
  root->addChildNode( n );
  QgsLayerTreeModel *m = new QgsLayerTreeModel( root, nullptr );
  m->refreshLayerLegend( n );

  //test initial symbol
  QgsLegendSymbolList symbolList = renderer->legendSymbolItems();
  for ( const QgsLegendSymbolItem &symbol : symbolList )
  {
    QgsSymbolLegendNode *symbolNode = dynamic_cast< QgsSymbolLegendNode * >( m->findLegendNode( vl->id(), symbol.ruleKey() ) );
    QVERIFY( symbolNode );
    QCOMPARE( symbolNode->symbol()->color(), symbol.symbol()->color() );
  }
  //try changing a symbol's color
  QgsSymbolLegendNode *symbolNode = dynamic_cast< QgsSymbolLegendNode * >( m->findLegendNode( vl->id(), symbolList.at( 1 ).ruleKey() ) );
  QVERIFY( symbolNode );
  QgsSymbol *newSymbol = symbolNode->symbol()->clone();
  newSymbol->setColor( QColor( 255, 255, 0 ) );
  symbolNode->setSymbol( newSymbol );
  QCOMPARE( symbolNode->symbol()->color(), QColor( 255, 255, 0 ) );
  //test that symbol change was sent to renderer
  symbolList = renderer->legendSymbolItems();
  QCOMPARE( symbolList.at( 1 ).symbol()->color(), QColor( 255, 255, 0 ) );

  //another test - check directly setting symbol at renderer
  QVariantMap props;
  props.insert( QStringLiteral( "color" ), QStringLiteral( "#00ffff" ) );
  props.insert( QStringLiteral( "outline_color" ), QStringLiteral( "#000000" ) );
  renderer->setLegendSymbolItem( symbolList.at( 2 ).ruleKey(), QgsMarkerSymbol::createSimple( props ) );
  m->refreshLayerLegend( n );
  symbolNode = dynamic_cast< QgsSymbolLegendNode * >( m->findLegendNode( vl->id(), symbolList.at( 2 ).ruleKey() ) );
  QVERIFY( symbolNode );
  QCOMPARE( symbolNode->symbol()->color(), QColor( 0, 255, 255 ) );

  //cleanup
  delete m;
  delete root;
}


void TestQgsLayerTree::testEmbeddedGroup()
{
  const QString dataDir( TEST_DATA_DIR ); //defined in CmakeLists.txt
  const QString layerPath = dataDir + QStringLiteral( "/points.shp" );

  // build a project with 3 layers, each having a simple renderer with SVG marker
  // - existing SVG file in project dir
  // - existing SVG file in QGIS dir
  // - non-exsiting SVG file

  const QTemporaryDir dir;
  QVERIFY( dir.isValid() );
  // on mac the returned path was not canonical and the resolver failed to convert paths properly
  const QString dirPath = QFileInfo( dir.path() ).canonicalFilePath();

  const QString projectFilename = dirPath + QStringLiteral( "/project.qgs" );

  QgsVectorLayer *layer1 = new QgsVectorLayer( layerPath, QStringLiteral( "points 1" ), QStringLiteral( "ogr" ) );
  QgsVectorLayer *layer2 = new QgsVectorLayer( layerPath, QStringLiteral( "points 2" ), QStringLiteral( "ogr" ) );
  QgsVectorLayer *layer3 = new QgsVectorLayer( layerPath, QStringLiteral( "points 3" ), QStringLiteral( "ogr" ) );

  QVERIFY( layer1->isValid() );

  QgsProject project;
  project.addMapLayers( QList<QgsMapLayer *>() << layer1 << layer2 << layer3, false );
  QgsLayerTreeGroup *grp = project.layerTreeRoot()->addGroup( QStringLiteral( "Embed" ) );
  grp->addLayer( layer1 );
  grp->addLayer( layer2 );
  grp->addLayer( layer3 );
  project.write( projectFilename );

  //
  // now let's use the layer group embedded in another project...
  //

  QgsProject projectMaster;
  QgsLayerTreeGroup *embeddedGroup = projectMaster.createEmbeddedGroup( grp->name(), projectFilename, QStringList() );
  QVERIFY( embeddedGroup );
  QCOMPARE( embeddedGroup->children().size(), 3 );

  for ( QgsLayerTreeNode *child : embeddedGroup->children() )
  {
    QVERIFY( QgsLayerTree::toLayer( child )->layer() );
  }
  projectMaster.layerTreeRoot()->addChildNode( embeddedGroup );

  const QString projectMasterFilename = dirPath + QStringLiteral( "/projectMaster.qgs" );
  projectMaster.write( projectMasterFilename );
  projectMaster.clear();

  QgsProject projectMasterCopy;
  projectMasterCopy.read( projectMasterFilename );
  QgsLayerTreeGroup *masterEmbeddedGroup = projectMasterCopy.layerTreeRoot()->findGroup( QStringLiteral( "Embed" ) );
  QVERIFY( masterEmbeddedGroup );
  QCOMPARE( masterEmbeddedGroup->children().size(), 3 );

  for ( QgsLayerTreeNode *child : masterEmbeddedGroup->children() )
  {
    QVERIFY( QgsLayerTree::toLayer( child )->layer() );
  }
}

void TestQgsLayerTree::testFindLayer()
{
  //new memory layer
  QgsVectorLayer *vl = new QgsVectorLayer( QStringLiteral( "Point?field=col1:integer" ), QStringLiteral( "vl" ), QStringLiteral( "memory" ) );
  QVERIFY( vl->isValid() );

  QgsProject project;
  project.addMapLayer( vl );

  QgsLayerTree root;
  const QgsLayerTreeModel model( &root );

  QVERIFY( !root.findLayer( vl->id() ) );
  QVERIFY( !root.findLayer( nullptr ) );

  root.addLayer( vl );

  QCOMPARE( root.findLayer( vl->id() )->layer(), vl );
  QCOMPARE( root.findLayer( vl )->layer(), vl );
  QVERIFY( !root.findLayer( QStringLiteral( "xxx" ) ) );
  QVERIFY( !root.findLayer( nullptr ) );
}

void TestQgsLayerTree::testLayerDeleted()
{
  //new memory layer
  QgsVectorLayer *vl = new QgsVectorLayer( QStringLiteral( "Point?field=col1:integer" ), QStringLiteral( "vl" ), QStringLiteral( "memory" ) );
  QVERIFY( vl->isValid() );

  QgsProject project;
  project.addMapLayer( vl );

  QgsLayerTree root;
  QgsLayerTreeModel model( &root );

  root.addLayer( vl );
  QgsLayerTreeLayer *tl( root.findLayer( vl->id() ) );
  QCOMPARE( tl->layer(), vl );

  QCOMPARE( model.layerLegendNodes( tl ).count(), 1 );

  project.removeMapLayer( vl );

  QCOMPARE( model.layerLegendNodes( tl ).count(), 0 );
}

void TestQgsLayerTree::testFindGroups()
{
  const QgsProject project;
  QgsLayerTreeGroup *group1 = project.layerTreeRoot()->addGroup( QStringLiteral( "Group_One" ) );
  QVERIFY( group1 );
  QgsLayerTreeGroup *group2 = project.layerTreeRoot()->addGroup( QStringLiteral( "Group_Two" ) );
  QVERIFY( group2 );
  QgsLayerTreeGroup *group3 = project.layerTreeRoot()->addGroup( QStringLiteral( "Group_Three" ) );
  QVERIFY( group3 );

  QgsLayerTreeGroup *group = project.layerTreeRoot()->findGroup( QStringLiteral( "Group_One" ) );
  QVERIFY( group );
  group = project.layerTreeRoot()->findGroup( QStringLiteral( "Group_Two" ) );
  QVERIFY( group );
  group = project.layerTreeRoot()->findGroup( QStringLiteral( "Group_Three" ) );
  QVERIFY( group );

  const QList<QgsLayerTreeGroup *> groups = project.layerTreeRoot()->findGroups();

  QVERIFY( groups.contains( group1 ) );
  QVERIFY( groups.contains( group2 ) );
  QVERIFY( groups.contains( group3 ) );
}

void TestQgsLayerTree::testFindNestedGroups()
{
  const QgsProject project;
  QgsLayerTreeGroup *group1 = project.layerTreeRoot()->addGroup( QStringLiteral( "Group_One" ) );
  QVERIFY( group1 );
  QgsLayerTreeGroup *group2 = group1->addGroup( QStringLiteral( "Group_Two" ) );
  QVERIFY( group2 );
  QgsLayerTreeGroup *group3 = group2->addGroup( QStringLiteral( "Group_Three" ) );
  QVERIFY( group3 );

  const QList<QgsLayerTreeGroup *> groups = project.layerTreeRoot()->findGroups();

  QVERIFY( groups.contains( group1 ) );
  QVERIFY( groups.contains( group2 ) == 0 );
  QVERIFY( groups.contains( group3 ) == 0 );

  const QList<QgsLayerTreeGroup *> all = project.layerTreeRoot()->findGroups( true );

  QVERIFY( all.contains( group1 ) );
  QVERIFY( all.contains( group2 ) );
  QVERIFY( all.contains( group3 ) );
}

void TestQgsLayerTree::testUtilsCollectMapLayers()
{
  QgsVectorLayer *vl1 = new QgsVectorLayer( QStringLiteral( "Point?field=col1:integer" ), QStringLiteral( "vl1" ), QStringLiteral( "memory" ) );
  QgsVectorLayer *vl2 = new QgsVectorLayer( QStringLiteral( "Point?field=col1:integer" ), QStringLiteral( "vl1" ), QStringLiteral( "memory" ) );

  QgsProject project;
  project.addMapLayer( vl1 );
  project.addMapLayer( vl2 );

  QgsLayerTree root;
  QgsLayerTreeLayer *nodeVl1 = root.addLayer( vl1 );
  QgsLayerTreeGroup *nodeGrp = root.addGroup( "grp" );
  QgsLayerTreeLayer *nodeVl2 = nodeGrp->addLayer( vl2 );
  Q_UNUSED( nodeVl2 );

  const QSet<QgsMapLayer *> set1 = QgsLayerTreeUtils::collectMapLayersRecursive( QList<QgsLayerTreeNode *>() << &root );
  const QSet<QgsMapLayer *> set2 = QgsLayerTreeUtils::collectMapLayersRecursive( QList<QgsLayerTreeNode *>() << nodeVl1 );
  const QSet<QgsMapLayer *> set3 = QgsLayerTreeUtils::collectMapLayersRecursive( QList<QgsLayerTreeNode *>() << nodeGrp );

  QCOMPARE( set1, QSet<QgsMapLayer *>() << vl1 << vl2 );
  QCOMPARE( set2, QSet<QgsMapLayer *>() << vl1 );
  QCOMPARE( set3, QSet<QgsMapLayer *>() << vl2 );
}

void TestQgsLayerTree::testUtilsCountMapLayers()
{
  QgsVectorLayer *vl = new QgsVectorLayer( QStringLiteral( "Point?field=col1:integer" ), QStringLiteral( "vl" ), QStringLiteral( "memory" ) );

  QgsProject project;
  project.addMapLayer( vl );

  QgsLayerTree root;
  QgsLayerTreeGroup *nodeGrp = root.addGroup( "grp" );

  QCOMPARE( QgsLayerTreeUtils::countMapLayerInTree( &root, vl ), 0 );

  root.addLayer( vl );
  QCOMPARE( QgsLayerTreeUtils::countMapLayerInTree( &root, vl ), 1 );

  nodeGrp->addLayer( vl );
  QCOMPARE( QgsLayerTreeUtils::countMapLayerInTree( &root, vl ), 2 );
}

void TestQgsLayerTree::testSymbolText()
{
  //new memory layer
  QgsVectorLayer *vl = new QgsVectorLayer( QStringLiteral( "Point?field=col1:integer" ), QStringLiteral( "vl" ), QStringLiteral( "memory" ) );
  QVERIFY( vl->isValid() );

  QgsProject project;
  project.addMapLayer( vl );

  //create a categorized renderer for layer
  QgsCategorizedSymbolRenderer *renderer = new QgsCategorizedSymbolRenderer();
  renderer->setClassAttribute( QStringLiteral( "col1" ) );
  renderer->setSourceSymbol( QgsSymbol::defaultSymbol( QgsWkbTypes::PointGeometry ) );
  renderer->addCategory( QgsRendererCategory( "a", QgsSymbol::defaultSymbol( QgsWkbTypes::PointGeometry ), QStringLiteral( "a [% 1 + 2 %]" ) ) );
  renderer->addCategory( QgsRendererCategory( "b", QgsSymbol::defaultSymbol( QgsWkbTypes::PointGeometry ), QStringLiteral( "b,c" ) ) );
  renderer->addCategory( QgsRendererCategory( "c", QgsSymbol::defaultSymbol( QgsWkbTypes::PointGeometry ), QStringLiteral( "c" ) ) );
  vl->setRenderer( renderer );

  //create legend with symbology nodes for categorized renderer
  QgsLayerTree *root = new QgsLayerTree();
  QgsLayerTreeLayer *n = new QgsLayerTreeLayer( vl );
  root->addChildNode( n );
  QgsLayerTreeModel *m = new QgsLayerTreeModel( root, nullptr );
  m->refreshLayerLegend( n );

  const QList<QgsLayerTreeModelLegendNode *> nodes = m->layerLegendNodes( n );
  QCOMPARE( nodes.length(), 3 );

  QgsLegendSettings settings;
  settings.setWrapChar( QStringLiteral( "," ) );
  QCOMPARE( nodes.at( 0 )->data( Qt::DisplayRole ).toString(), QStringLiteral( "a [% 1 + 2 %]" ) );
  QCOMPARE( nodes.at( 1 )->data( Qt::DisplayRole ).toString(), QStringLiteral( "b,c" ) );
  QCOMPARE( nodes.at( 2 )->data( Qt::DisplayRole ).toString(), QStringLiteral( "c" ) );
  nodes.at( 2 )->setUserLabel( QStringLiteral( "[% 2+3 %] x [% 3+4 %]" ) );
  QCOMPARE( nodes.at( 2 )->data( Qt::DisplayRole ).toString(), QStringLiteral( "[% 2+3 %] x [% 3+4 %]" ) );

  QgsExpressionContext context;
  QCOMPARE( settings.evaluateItemText( nodes.at( 0 )->data( Qt::DisplayRole ).toString(), context ), QStringList() << QStringLiteral( "a 3" ) );
  QCOMPARE( settings.evaluateItemText( nodes.at( 1 )->data( Qt::DisplayRole ).toString(), context ), QStringList() << QStringLiteral( "b" ) << QStringLiteral( "c" ) );
  QCOMPARE( settings.evaluateItemText( nodes.at( 2 )->data( Qt::DisplayRole ).toString(), context ), QStringList() << QStringLiteral( "5 x 7" ) );

  // split string should happen after expression evaluation
  QgsExpressionContextScope *scope = new QgsExpressionContextScope();
  scope->setVariable( QStringLiteral( "bbbb" ), QStringLiteral( "aaaa,bbbb,cccc" ) );
  context.appendScope( scope );
  nodes.at( 2 )->setUserLabel( QStringLiteral( "[% @bbbb %],[% 3+4 %]" ) );
  QCOMPARE( settings.evaluateItemText( nodes.at( 2 )->data( Qt::DisplayRole ).toString(), context ), QStringList() << QStringLiteral( "aaaa" )
            << QStringLiteral( "bbbb" )
            << QStringLiteral( "cccc" )
            << QStringLiteral( "7" ) );
  //cleanup
  delete m;
  delete root;
}

void TestQgsLayerTree::testNodeDepth()
{
  QCOMPARE( mRoot->depth(), 0 );
  QgsLayerTreeNode *secondGroup = mRoot->children()[1];
  QCOMPARE( secondGroup->depth(), 1 );

  QgsVectorLayer *vl = new QgsVectorLayer( QStringLiteral( "Point?field=col1:integer" ), QStringLiteral( "vl" ), QStringLiteral( "memory" ) );
  QVERIFY( vl->isValid() );

  QgsLayerTreeLayer *n = new QgsLayerTreeLayer( vl->id(), vl->name() );
  mRoot->addChildNode( n );
  QCOMPARE( n->depth(), 1 );

  QgsLayerTreeGroup *g1 = mRoot->addGroup( QStringLiteral( "g1" ) );
  QCOMPARE( g1->depth(), 1 );
  QgsLayerTreeLayer *n1 = n->clone();
  g1->addChildNode( n1 );
  QCOMPARE( n1->depth(), 2 );
  QgsLayerTreeGroup *g2 = g1->addGroup( QStringLiteral( "g2" ) );
  QCOMPARE( g2->depth(), 2 );
  QgsLayerTreeLayer *n2 = n->clone();
  g2->addChildNode( n2 );
  QCOMPARE( n2->depth(), 3 );
  QgsLayerTreeGroup *g3 = g2->addGroup( QStringLiteral( "g3" ) );
  QCOMPARE( g3->depth(), 3 );
  QgsLayerTreeLayer *n3 = n->clone();
  g3->addChildNode( n3 );
  QCOMPARE( n3->depth(), 4 );

  mRoot->removeChildNode( n );
  mRoot->removeChildNode( g1 );
  delete vl;
}

void TestQgsLayerTree::testRasterSymbolNode()
{
  QCOMPARE( mRoot->depth(), 0 );
  QgsLayerTreeNode *secondGroup = mRoot->children()[1];
  QCOMPARE( secondGroup->depth(), 1 );

  std::unique_ptr< QgsRasterLayer > rl = std::make_unique< QgsRasterLayer >( QStringLiteral( TEST_DATA_DIR ) + "/tenbytenraster.asc", QStringLiteral( "rl" ), QStringLiteral( "gdal" ) );
  QVERIFY( rl->isValid() );

  const std::unique_ptr< QgsLayerTreeLayer > n = std::make_unique< QgsLayerTreeLayer >( rl.get() );

  // not checkable
  QgsRasterSymbolLegendNode rasterNode( n.get(), QColor( 255, 0, 0 ), QStringLiteral( "my node" ), nullptr, false, QStringLiteral( "key" ) );
  QVERIFY( !rasterNode.isCheckable() );
  QCOMPARE( rasterNode.ruleKey(), QStringLiteral( "key" ) );
  QCOMPARE( static_cast< int >( rasterNode.flags() ), static_cast< int >( Qt::ItemIsEnabled ) );
  QCOMPARE( rasterNode.data( Qt::DisplayRole ).toString(), QStringLiteral( "my node" ) );
  QCOMPARE( rasterNode.data( QgsLayerTreeModelLegendNode::NodeTypeRole ).toInt(), static_cast< int >( QgsLayerTreeModelLegendNode::RasterSymbolLegend ) );
  QCOMPARE( rasterNode.data( QgsLayerTreeModelLegendNode::RuleKeyRole ).toString(), QStringLiteral( "key" ) );
  QCOMPARE( rasterNode.data( Qt::CheckStateRole ), QVariant() );
  QVERIFY( !rasterNode.setData( true, Qt::CheckStateRole ) );

  // checkable
  const QgsRasterSymbolLegendNode rasterNode2( n.get(), QColor( 255, 0, 0 ), QStringLiteral( "my node" ), nullptr, true, QStringLiteral( "key" ) );
  QVERIFY( rasterNode2.isCheckable() );
  QCOMPARE( static_cast< int >( rasterNode2.flags() ), static_cast< int >( Qt::ItemIsEnabled | Qt::ItemIsUserCheckable ) );
}

void TestQgsLayerTree::testLayersEditable()
{
  QgsProject project;

  QgsVectorLayer *vl1 = new QgsVectorLayer( QStringLiteral( "Point?field=col1:integer" ), QStringLiteral( "vl1" ), QStringLiteral( "memory" ) );
  QgsVectorLayer *vl2 = new QgsVectorLayer( QStringLiteral( "Point?field=col1:integer" ), QStringLiteral( "vl1" ), QStringLiteral( "memory" ) );
  QgsAnnotationLayer *al = new QgsAnnotationLayer( QStringLiteral( "al" ), QgsAnnotationLayer::LayerOptions( project.transformContext() ) );

  project.addMapLayer( vl1 );
  project.addMapLayer( vl2 );
  project.addMapLayer( al );

  QgsLayerTree root;
  QgsLayerTreeLayer *nodeVl1 = root.addLayer( vl1 );
  QgsLayerTreeGroup *nodeGrp = root.addGroup( QStringLiteral( "grp" ) );
  QgsLayerTreeLayer *nodeVl2 = nodeGrp->addLayer( vl2 );
  QgsLayerTreeLayer *nodeAl = nodeGrp->addLayer( al );
  QVERIFY( !QgsLayerTreeUtils::layersEditable( {} ) );
  QVERIFY( !QgsLayerTreeUtils::layersEditable( {nodeVl1, nodeVl2} ) );
  vl1->startEditing();
  QVERIFY( QgsLayerTreeUtils::layersEditable( {nodeVl1} ) );
  QVERIFY( QgsLayerTreeUtils::layersEditable( {nodeVl1, nodeVl2} ) );
  QVERIFY( QgsLayerTreeUtils::layersEditable( {nodeVl2, nodeVl1 } ) );

  QVERIFY( QgsLayerTreeUtils::layersEditable( {nodeAl} ) );
  QVERIFY( QgsLayerTreeUtils::layersEditable( {nodeAl, nodeVl1} ) );
  QVERIFY( QgsLayerTreeUtils::layersEditable( {nodeAl, nodeVl2} ) );

  // ignore layers which can't be toggled (the annotation layer)
  QVERIFY( !QgsLayerTreeUtils::layersEditable( {nodeAl}, true ) );
  QVERIFY( QgsLayerTreeUtils::layersEditable( {nodeAl, nodeVl1}, true ) );
  QVERIFY( !QgsLayerTreeUtils::layersEditable( {nodeAl, nodeVl2}, true ) );
}

void TestQgsLayerTree::testInsertLayerBelow()
{
  QgsVectorLayer *topLayer = new QgsVectorLayer( QStringLiteral( "Point?field=col1:integer" ), QStringLiteral( "Top Layer" ), QStringLiteral( "memory" ) );
  QVERIFY( topLayer->isValid() );
  QgsVectorLayer *bottomLayer = new QgsVectorLayer( QStringLiteral( "Point?field=col1:integer" ), QStringLiteral( "Bottom Layer" ), QStringLiteral( "memory" ) );
  QVERIFY( bottomLayer->isValid() );

  QgsLayerTree root;
  root.addLayer( topLayer );
  QCOMPARE( QgsLayerTreeUtils::countMapLayerInTree( &root, topLayer ), 1 );
  QCOMPARE( QgsLayerTreeUtils::countMapLayerInTree( &root, bottomLayer ), 0 );

  QgsLayerTreeUtils::insertLayerBelow( &root, topLayer, bottomLayer );
  QCOMPARE( QgsLayerTreeUtils::countMapLayerInTree( &root, bottomLayer ), 1 );

  // Check the order of the layers
  QCOMPARE( root.findLayerIds(), QStringList() << topLayer->id() << bottomLayer->id() );
}

QGSTEST_MAIN( TestQgsLayerTree )
#include "testqgslayertree.moc"
