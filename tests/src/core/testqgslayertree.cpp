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

#include "qgsannotationlayer.h"
#include "qgsapplication.h"
#include "qgscategorizedsymbolrenderer.h"
#include "qgsfillsymbol.h"
#include "qgsfillsymbollayer.h"
#include "qgsgeometrygeneratorsymbollayer.h"
#include "qgsgraduatedsymbolrenderer.h"
#include "qgslayertree.h"
#include "qgslayertreemodel.h"
#include "qgslayertreemodellegendnode.h"
#include "qgslayertreeutils.h"
#include "qgslegendsettings.h"
#include "qgsmarkersymbol.h"
#include "qgsproject.h"
#include "qgsrulebasedrenderer.h"
#include "qgssettings.h"
#include "qgstest.h"
#include "qgsvectorlayer.h"
#include "qgsvectorlayerdiagramprovider.h"
#include "qgsvectorlayerlabelprovider.h"

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
    void testRestrictedSymbolSize_data();
    void testRestrictedSymbolSize();
    void testRestrictedSymbolSizeWithGeometryGenerator_data();
    void testRestrictedSymbolSizeWithGeometryGenerator();
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
    void testCustomNodes();
    void testCustomNodeOrderAndFinding();
    void testCustomNodeDeleted();
    void testUtilsCollectMapLayers();
    void testUtilsCountMapLayers();
    void testSymbolText();
    void testNodeDepth();
    void testRasterSymbolNode();
    void testLayersEditable();
    void testInsertLayerBelow();
    void testGroupReadWriteXMlServerProperties();

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

  QCoreApplication::setOrganizationName( u"QGIS"_s );
  QCoreApplication::setOrganizationDomain( u"qgis.org"_s );
  QCoreApplication::setApplicationName( u"QGIS-TEST"_s );

  mRoot = new QgsLayerTreeGroup();
  mRoot->addGroup( u"grp1"_s );
  mRoot->addGroup( u"grp2"_s );
  mRoot->addGroup( u"grp3"_s );

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
  secondGroup->setName( u"grp2+"_s );

  QCOMPARE( secondGroup->name(), QString( "grp2+" ) );

  QCOMPARE( spy.count(), 1 );
  const QList<QVariant> arguments = spy.takeFirst();
  QCOMPARE( arguments.at( 0 ).value<QgsLayerTreeNode *>(), secondGroup );
  QCOMPARE( arguments.at( 1 ).toString(), QString( "grp2+" ) );

  secondGroup->setName( u"grp2"_s );
  QCOMPARE( secondGroup->name(), QString( "grp2" ) );
}

void TestQgsLayerTree::testLayerNameChanged()
{
  QgsVectorLayer *vl = new QgsVectorLayer( u"Point?field=col1:integer"_s, u"vl"_s, u"memory"_s );
  QVERIFY( vl->isValid() );

  QgsLayerTreeLayer *n = new QgsLayerTreeLayer( vl->id(), vl->name() );
  mRoot->addChildNode( n );

  QSignalSpy spy( mRoot, SIGNAL( nameChanged( QgsLayerTreeNode *, QString ) ) );

  QCOMPARE( n->name(), QString( "vl" ) );
  n->setName( u"changed 1"_s );

  QCOMPARE( n->name(), QString( "changed 1" ) );
  QCOMPARE( spy.count(), 1 );
  QList<QVariant> arguments = spy.takeFirst();
  QCOMPARE( arguments.at( 0 ).value<QgsLayerTreeNode *>(), n );
  QCOMPARE( arguments.at( 1 ).toString(), QString( "changed 1" ) );

  QgsProject project;
  project.addMapLayer( vl );
  n->resolveReferences( &project );

  // set name via map layer
  vl->setName( u"changed 2"_s );
  QCOMPARE( n->name(), QString( "changed 2" ) );
  QCOMPARE( spy.count(), 1 );
  arguments = spy.takeFirst();
  QCOMPARE( arguments.at( 0 ).value<QgsLayerTreeNode *>(), n );
  QCOMPARE( arguments.at( 1 ).toString(), QString( "changed 2" ) );

  // set name via layer tree
  n->setName( u"changed 3"_s );
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
  mRoot->insertGroup( 0, u"grp1"_s );
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
  mRoot->addGroup( u"grp3"_s );
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
  root2->addGroup( u"1"_s );
  QCOMPARE( QgsLayerTree::toGroup( root2->children().at( 0 ) )->isVisible(), true );
  root2->addGroup( u"2"_s );
  QCOMPARE( QgsLayerTree::toGroup( root2->children().at( 0 ) )->isVisible(), true );
  QCOMPARE( QgsLayerTree::toGroup( root2->children().at( 1 ) )->isVisible(), false );
  delete root2;

  // check-uncheck the only child
  QgsLayerTreeGroup *root3 = new QgsLayerTreeGroup();
  root3->setIsMutuallyExclusive( true );
  root3->addGroup( u"1"_s );
  QCOMPARE( QgsLayerTree::toGroup( root3->children().at( 0 ) )->isVisible(), true );
  QgsLayerTree::toGroup( root3->children().at( 0 ) )->setItemVisibilityChecked( false );
  QCOMPARE( QgsLayerTree::toGroup( root3->children().at( 0 ) )->isVisible(), false );
  QCOMPARE( root3->isVisible(), true );
  QgsLayerTree::toGroup( root3->children().at( 0 ) )->setItemVisibilityChecked( true );
  QCOMPARE( QgsLayerTree::toGroup( root3->children().at( 0 ) )->isVisible(), true );
  QCOMPARE( root3->isVisible(), true );
  delete root3;
}

void TestQgsLayerTree::testRestrictedSymbolSize_data()
{
  QTest::addColumn<double>( "maxSize" );
  QTest::addColumn<int>( "expectedSize" );

  // QTest::newRow( "smaller than max" ) << 15. << 52;
  QTest::newRow( "bigger than max" ) << 10. << 40;
}

void TestQgsLayerTree::testRestrictedSymbolSize()
{
  QFETCH( double, maxSize );
  QFETCH( int, expectedSize );

  // to force re-read of max/min size in QgsSymbolLegendNode constructor
  QgsSymbolLegendNode::MINIMUM_SIZE = -1;
  QgsSymbolLegendNode::MAXIMUM_SIZE = -1;

  QgsSettings settings;
  settings.setValue( "/qgis/legendsymbolMaximumSize", maxSize );

  //new memory layer
  QgsVectorLayer *vl = new QgsVectorLayer( u"Point?field=col1:integer"_s, u"vl"_s, u"memory"_s );
  QVERIFY( vl->isValid() );

  QgsProject project;
  project.addMapLayer( vl );

  QgsMarkerSymbol *symbol = static_cast<QgsMarkerSymbol *>( QgsSymbol::defaultSymbol( Qgis::GeometryType::Point ) );
  symbol->setSize( 500.0 );
  symbol->setSizeUnit( Qgis::RenderUnit::MapUnits );

  //create a categorized renderer for layer
  QgsCategorizedSymbolRenderer *renderer = new QgsCategorizedSymbolRenderer();
  renderer->setClassAttribute( u"col1"_s );
  renderer->setSourceSymbol( symbol->clone() );
  renderer->addCategory( QgsRendererCategory( "a", symbol->clone(), u"a"_s ) );
  renderer->addCategory( QgsRendererCategory( "b", symbol->clone(), u"b"_s ) );
  vl->setRenderer( renderer );

  //create legend with symbology nodes for categorized renderer
  QgsLayerTree *root = new QgsLayerTree();
  QgsLayerTreeLayer *n = new QgsLayerTreeLayer( vl );
  root->addChildNode( n );
  QgsLayerTreeModel *m = new QgsLayerTreeModel( root, nullptr );
  m->setLegendMapViewData( 10, 96, 10 );

  const QList<QgsLayerTreeModelLegendNode *> nodes = m->layerLegendNodes( n );
  const QSize minimumSize = static_cast<QgsSymbolLegendNode *>( nodes.at( 0 ) )->minimumIconSize();
  QCOMPARE( minimumSize.width(), expectedSize );

  //cleanup
  delete m;
  delete root;
}

void TestQgsLayerTree::testRestrictedSymbolSizeWithGeometryGenerator_data()
{
  QTest::addColumn<double>( "maxSize" );
  QTest::addColumn<int>( "expectedSize" );

  QTest::newRow( "smaller than max" ) << 15. << 42;
  QTest::newRow( "bigger than max" ) << 10. << 38;
}

void TestQgsLayerTree::testRestrictedSymbolSizeWithGeometryGenerator()
{
  QFETCH( double, maxSize );
  QFETCH( int, expectedSize );

  // to force re-read of max/min size in QgsSymbolLegendNode constructor
  QgsSymbolLegendNode::MINIMUM_SIZE = -1;
  QgsSymbolLegendNode::MAXIMUM_SIZE = -1;

  QgsSettings settings;
  settings.setValue( "/qgis/legendsymbolMaximumSize", maxSize );

  //new memory layer
  QgsVectorLayer *vl = new QgsVectorLayer( u"Point?field=col1:integer"_s, u"vl"_s, u"memory"_s );
  QVERIFY( vl->isValid() );

  QgsProject project;
  project.addMapLayer( vl );

  //create a categorized renderer with geometry generator for layer

  QVariantMap ggProps;
  ggProps.insert( u"SymbolType"_s, u"Fill"_s );
  ggProps.insert( u"geometryModifier"_s, u"buffer( $geometry, 200 )"_s );
  QgsSymbolLayer *ggSymbolLayer = QgsGeometryGeneratorSymbolLayer::create( ggProps );
  QgsSymbolLayerList fillSymbolLayerList;
  fillSymbolLayerList << new QgsSimpleFillSymbolLayer();
  ggSymbolLayer->setSubSymbol( new QgsFillSymbol( fillSymbolLayerList ) );
  QgsSymbolLayerList slList;
  slList << ggSymbolLayer;
  QgsMarkerSymbol *symbol = new QgsMarkerSymbol( slList );

  QgsCategorizedSymbolRenderer *renderer = new QgsCategorizedSymbolRenderer();
  renderer->setClassAttribute( u"col1"_s );
  renderer->setSourceSymbol( symbol->clone() );
  renderer->addCategory( QgsRendererCategory( "a", symbol->clone(), u"a"_s ) );
  renderer->addCategory( QgsRendererCategory( "b", symbol->clone(), u"b"_s ) );
  vl->setRenderer( renderer );

  //create legend with symbology nodes for categorized renderer
  QgsLayerTree *root = new QgsLayerTree();
  QgsLayerTreeLayer *n = new QgsLayerTreeLayer( vl );
  root->addChildNode( n );
  QgsLayerTreeModel *m = new QgsLayerTreeModel( root, nullptr );
  m->setLegendMapViewData( 10, 96, 10 );

  const QList<QgsLayerTreeModelLegendNode *> nodes = m->layerLegendNodes( n );
  const QSize minimumSize = static_cast<QgsSymbolLegendNode *>( nodes.at( 0 ) )->minimumIconSize();
  QCOMPARE( minimumSize.width(), expectedSize );

  //cleanup
  delete m;
  delete root;
}

void TestQgsLayerTree::testShowHideAllSymbolNodes()
{
  //new memory layer
  QgsVectorLayer *vl = new QgsVectorLayer( u"Point?field=col1:integer"_s, u"vl"_s, u"memory"_s );
  QVERIFY( vl->isValid() );

  QgsProject project;
  project.addMapLayer( vl );

  //create a categorized renderer for layer
  QgsCategorizedSymbolRenderer *renderer = new QgsCategorizedSymbolRenderer();
  renderer->setClassAttribute( u"col1"_s );
  renderer->setSourceSymbol( QgsSymbol::defaultSymbol( Qgis::GeometryType::Point ) );
  renderer->addCategory( QgsRendererCategory( "a", QgsSymbol::defaultSymbol( Qgis::GeometryType::Point ), u"a"_s ) );
  renderer->addCategory( QgsRendererCategory( "b", QgsSymbol::defaultSymbol( Qgis::GeometryType::Point ), u"b"_s ) );
  renderer->addCategory( QgsRendererCategory( "c", QgsSymbol::defaultSymbol( Qgis::GeometryType::Point ), u"c"_s ) );
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
  static_cast<QgsSymbolLegendNode *>( nodes.at( 0 ) )->uncheckAllItems();
  for ( QgsLayerTreeModelLegendNode *ln : nodes )
  {
    QVERIFY( ln->data( Qt::CheckStateRole ) == Qt::Unchecked );
  }
  //check all and test that all nodes are checked
  static_cast<QgsSymbolLegendNode *>( nodes.at( 0 ) )->checkAllItems();
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
  QgsVectorLayer *vl = new QgsVectorLayer( u"Point?field=col1:integer"_s, u"vl"_s, u"memory"_s );
  QVERIFY( vl->isValid() );

  QgsProject project;
  project.addMapLayer( vl );

  //create a categorized renderer for layer
  QgsCategorizedSymbolRenderer *renderer = new QgsCategorizedSymbolRenderer();
  renderer->setClassAttribute( u"col1"_s );
  renderer->setSourceSymbol( QgsSymbol::defaultSymbol( Qgis::GeometryType::Point ) );
  renderer->addCategory( QgsRendererCategory( "a", QgsSymbol::defaultSymbol( Qgis::GeometryType::Point ), u"a"_s ) );
  renderer->addCategory( QgsRendererCategory( "b", QgsSymbol::defaultSymbol( Qgis::GeometryType::Point ), u"b"_s ) );
  renderer->addCategory( QgsRendererCategory( "c", QgsSymbol::defaultSymbol( Qgis::GeometryType::Point ), u"c"_s ) );
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
  renderer->setClassAttribute( u"col1"_s );
  renderer->setSourceSymbol( QgsSymbol::defaultSymbol( Qgis::GeometryType::Point ) );
  QVariantMap props;
  props.insert( u"color"_s, u"#ff0000"_s );
  props.insert( u"outline_color"_s, u"#000000"_s );
  renderer->addCategory( QgsRendererCategory( "a", QgsMarkerSymbol::createSimple( props ).release(), u"a"_s ) );
  props.insert( u"color"_s, u"#00ff00"_s );
  renderer->addCategory( QgsRendererCategory( "b", QgsMarkerSymbol::createSimple( props ).release(), u"b"_s ) );
  props.insert( u"color"_s, u"#0000ff"_s );
  renderer->addCategory( QgsRendererCategory( "c", QgsMarkerSymbol::createSimple( props ).release(), u"c"_s ) );
  testRendererLegend( renderer );
}

void TestQgsLayerTree::testLegendSymbolGraduated()
{
  //test retrieving/setting a graduated renderer's symbol through the legend node
  QgsGraduatedSymbolRenderer *renderer = new QgsGraduatedSymbolRenderer();
  renderer->setClassAttribute( u"col1"_s );
  renderer->setSourceSymbol( QgsSymbol::defaultSymbol( Qgis::GeometryType::Point ) );
  QVariantMap props;
  props.insert( u"color"_s, u"#ff0000"_s );
  props.insert( u"outline_color"_s, u"#000000"_s );
  renderer->addClass( QgsRendererRange( 1, 2, QgsMarkerSymbol::createSimple( props ).release(), u"a"_s ) );
  props.insert( u"color"_s, u"#00ff00"_s );
  renderer->addClass( QgsRendererRange( 2, 3, QgsMarkerSymbol::createSimple( props ).release(), u"b"_s ) );
  props.insert( u"color"_s, u"#0000ff"_s );
  renderer->addClass( QgsRendererRange( 3, 4, QgsMarkerSymbol::createSimple( props ).release(), u"c"_s ) );
  testRendererLegend( renderer );
}

void TestQgsLayerTree::testLegendSymbolRuleBased()
{
  //test retrieving/setting a rule based renderer's symbol through the legend node
  QgsRuleBasedRenderer::Rule *root = new QgsRuleBasedRenderer::Rule( nullptr );
  QVariantMap props;
  props.insert( u"color"_s, u"#ff0000"_s );
  props.insert( u"outline_color"_s, u"#000000"_s );
  root->appendChild( new QgsRuleBasedRenderer::Rule( QgsMarkerSymbol::createSimple( props ).release(), 0, 0, u"\"col1\"=1"_s ) );
  props.insert( u"color"_s, u"#00ff00"_s );
  root->appendChild( new QgsRuleBasedRenderer::Rule( QgsMarkerSymbol::createSimple( props ).release(), 0, 0, u"\"col1\"=2"_s ) );
  props.insert( u"color"_s, u"#0000ff"_s );
  root->appendChild( new QgsRuleBasedRenderer::Rule( QgsMarkerSymbol::createSimple( props ).release(), 0, 0, u"ELSE"_s ) );
  QgsRuleBasedRenderer *renderer = new QgsRuleBasedRenderer( root );
  testRendererLegend( renderer );
}

void TestQgsLayerTree::testResolveReferences()
{
  QgsVectorLayer *vl = new QgsVectorLayer( u"Point?field=col1:integer"_s, u"vl"_s, u"memory"_s );
  QVERIFY( vl->isValid() );

  const QString n1id = vl->id();
  const QString n2id = u"XYZ"_s;

  QgsMapLayer *nullLayer = nullptr; // QCOMPARE does not like nullptr directly

  QgsLayerTreeGroup *root = new QgsLayerTreeGroup();
  QgsLayerTreeLayer *n1 = new QgsLayerTreeLayer( n1id, vl->name() );
  QgsLayerTreeLayer *n2 = new QgsLayerTreeLayer( n2id, u"invalid layer"_s );
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
  QgsVectorLayer *vl = new QgsVectorLayer( u"Point?field=col1:integer"_s, u"vl"_s, u"memory"_s );
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
    QgsSymbolLegendNode *symbolNode = dynamic_cast<QgsSymbolLegendNode *>( m->findLegendNode( vl->id(), symbol.ruleKey() ) );
    QVERIFY( symbolNode );
    QCOMPARE( symbolNode->symbol()->color(), symbol.symbol()->color() );
  }
  //try changing a symbol's color
  QgsSymbolLegendNode *symbolNode = dynamic_cast<QgsSymbolLegendNode *>( m->findLegendNode( vl->id(), symbolList.at( 1 ).ruleKey() ) );
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
  props.insert( u"color"_s, u"#00ffff"_s );
  props.insert( u"outline_color"_s, u"#000000"_s );
  renderer->setLegendSymbolItem( symbolList.at( 2 ).ruleKey(), QgsMarkerSymbol::createSimple( props ).release() );
  m->refreshLayerLegend( n );
  symbolNode = dynamic_cast<QgsSymbolLegendNode *>( m->findLegendNode( vl->id(), symbolList.at( 2 ).ruleKey() ) );
  QVERIFY( symbolNode );
  QCOMPARE( symbolNode->symbol()->color(), QColor( 0, 255, 255 ) );

  //cleanup
  delete m;
  delete root;
}


void TestQgsLayerTree::testEmbeddedGroup()
{
  const QString dataDir( TEST_DATA_DIR ); //defined in CmakeLists.txt
  const QString layerPath = dataDir + u"/points.shp"_s;

  // build a project with 3 layers, each having a simple renderer with SVG marker
  // - existing SVG file in project dir
  // - existing SVG file in QGIS dir
  // - non-exsiting SVG file

  const QTemporaryDir dir;
  QVERIFY( dir.isValid() );
  // on mac the returned path was not canonical and the resolver failed to convert paths properly
  const QString dirPath = QFileInfo( dir.path() ).canonicalFilePath();

  const QString projectFilename = dirPath + u"/project.qgs"_s;

  QgsVectorLayer *layer1 = new QgsVectorLayer( layerPath, u"points 1"_s, u"ogr"_s );
  QgsVectorLayer *layer2 = new QgsVectorLayer( layerPath, u"points 2"_s, u"ogr"_s );
  QgsVectorLayer *layer3 = new QgsVectorLayer( layerPath, u"points 3"_s, u"ogr"_s );

  QVERIFY( layer1->isValid() );

  QgsProject project;
  project.addMapLayers( QList<QgsMapLayer *>() << layer1 << layer2 << layer3, false );
  QgsLayerTreeGroup *grp = project.layerTreeRoot()->addGroup( u"Embed"_s );
  grp->addLayer( layer1 );
  grp->addLayer( layer2 );
  grp->addLayer( layer3 );
  project.write( projectFilename );

  //
  // now let's use the layer group embedded in another project...
  //

  QgsProject projectMaster;
  std::unique_ptr< QgsLayerTreeGroup > embeddedGroup = projectMaster.createEmbeddedGroup( grp->name(), projectFilename, QStringList() );
  QVERIFY( embeddedGroup );
  QCOMPARE( embeddedGroup->children().size(), 3 );

  for ( QgsLayerTreeNode *child : embeddedGroup->children() )
  {
    QVERIFY( QgsLayerTree::toLayer( child )->layer() );
  }
  projectMaster.layerTreeRoot()->addChildNode( embeddedGroup.release() );

  const QString projectMasterFilename = dirPath + u"/projectMaster.qgs"_s;
  projectMaster.write( projectMasterFilename );
  projectMaster.clear();

  QgsProject projectMasterCopy;
  projectMasterCopy.read( projectMasterFilename );
  QgsLayerTreeGroup *masterEmbeddedGroup = projectMasterCopy.layerTreeRoot()->findGroup( u"Embed"_s );
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
  QgsVectorLayer *vl = new QgsVectorLayer( u"Point?field=col1:integer"_s, u"vl"_s, u"memory"_s );
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
  QVERIFY( !root.findLayer( u"xxx"_s ) );
  QVERIFY( !root.findLayer( nullptr ) );
}

void TestQgsLayerTree::testLayerDeleted()
{
  //new memory layer
  QgsVectorLayer *vl = new QgsVectorLayer( u"Point?field=col1:integer"_s, u"vl"_s, u"memory"_s );
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
  QgsLayerTreeGroup *group1 = project.layerTreeRoot()->addGroup( u"Group_One"_s );
  QVERIFY( group1 );
  QgsLayerTreeGroup *group2 = project.layerTreeRoot()->addGroup( u"Group_Two"_s );
  QVERIFY( group2 );
  QgsLayerTreeGroup *group3 = project.layerTreeRoot()->addGroup( u"Group_Three"_s );
  QVERIFY( group3 );

  QgsLayerTreeGroup *group = project.layerTreeRoot()->findGroup( u"Group_One"_s );
  QVERIFY( group );
  group = project.layerTreeRoot()->findGroup( u"Group_Two"_s );
  QVERIFY( group );
  group = project.layerTreeRoot()->findGroup( u"Group_Three"_s );
  QVERIFY( group );

  const QList<QgsLayerTreeGroup *> groups = project.layerTreeRoot()->findGroups();

  QVERIFY( groups.contains( group1 ) );
  QVERIFY( groups.contains( group2 ) );
  QVERIFY( groups.contains( group3 ) );
}

void TestQgsLayerTree::testFindNestedGroups()
{
  const QgsProject project;
  QgsLayerTreeGroup *group1 = project.layerTreeRoot()->addGroup( u"Group_One"_s );
  QVERIFY( group1 );
  QgsLayerTreeGroup *group2 = group1->addGroup( u"Group_Two"_s );
  QVERIFY( group2 );
  QgsLayerTreeGroup *group3 = group2->addGroup( u"Group_Three"_s );
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

void TestQgsLayerTree::testCustomNodes()
{
  auto custom = std::make_unique< QgsLayerTreeCustomNode >( u"custom-id"_s );
  QVERIFY( QgsLayerTree::isCustomNode( custom.get() ) );
  QCOMPARE( custom->nodeId(), u"custom-id"_s );
  QCOMPARE( custom->name(), u"custom-id"_s );
  custom->setName( u"Custom Name"_s );
  QCOMPARE( custom->name(), u"Custom Name"_s );

  QgsLayerTreeCustomNode *custom2 = custom->clone();
  QCOMPARE( custom2->nodeId(), u"custom-id"_s );
  QCOMPARE( custom2->name(), u"Custom Name"_s );

  QgsLayerTree root;
  QgsLayerTreeCustomNode *custom3 = root.insertCustomNode( -1, u"custom-id-3"_s, u"Custom Name 3"_s );
  QVERIFY( custom3 );
  QCOMPARE( custom3->nodeId(), u"custom-id-3"_s );
  QCOMPARE( custom3->name(), u"Custom Name 3"_s );
  QCOMPARE( root.children().count(), 1 );

  int count = 0;
  const QList< QgsLayerTreeNode * > nodes = root.children();
  for ( QgsLayerTreeNode *node : nodes )
  {
    if ( QgsLayerTree::isCustomNode( node ) )
    {
      QgsLayerTreeCustomNode *customNode = QgsLayerTree::toCustomNode( node );
      QCOMPARE( customNode->nodeId(), u"custom-id-3"_s );
      QCOMPARE( customNode->name(), u"Custom Name 3"_s );
      count++;
    }
  }
  QVERIFY( count );

  // Avoid adding 2 custom nodes with the same id.
  QgsLayerTreeCustomNode *custom4 = root.insertCustomNode( -1, u"custom-id-3"_s, u"Custom Name 4"_s );
  QVERIFY( !custom4 );
  QCOMPARE( root.children().count(), 1 );

  custom4 = root.addCustomNode( u"custom-id-3"_s, u"Custom Name 4"_s );
  QVERIFY( !custom4 );
  QCOMPARE( root.children().count(), 1 );

  // Avoid adding custom nodes with empty ids
  custom4 = root.insertCustomNode( -1, u" "_s, u"Custom Name 4"_s );
  QVERIFY( !custom4 );
  QCOMPARE( root.children().count(), 1 );

  custom4 = root.addCustomNode( QString( "" ), u"Custom Name 4"_s );
  QVERIFY( !custom4 );
  QCOMPARE( root.children().count(), 1 );
}

void TestQgsLayerTree::testCustomNodeOrderAndFinding()
{
  // c1
  // g1
  //  | vl
  //  | g2
  //     | c3
  //     | vl2
  //     | c2
  QgsLayerTree root;
  QVERIFY( !root.findCustomNode( u"custom-id"_s ) );
  QgsLayerTreeCustomNode *custom = root.insertCustomNode( -1, u"custom-id"_s, u"Custom Name"_s );

  QgsLayerTreeGroup *group = root.addGroup( u"gr1"_s );
  QgsVectorLayer *vl = new QgsVectorLayer( u"Point?field=col1:integer"_s, u"vl"_s, u"memory"_s );
  QgsLayerTreeLayer *vlNode = group->addLayer( vl );

  QgsLayerTreeGroup *group2 = group->addGroup( u"gr2"_s );
  QgsVectorLayer *vl2 = new QgsVectorLayer( u"Point?field=col1:integer"_s, u"vl2"_s, u"memory"_s );
  QgsLayerTreeLayer *vlNode2 = group2->addLayer( vl2 );
  QgsLayerTreeCustomNode *custom2 = group2->insertCustomNode( 1, u"custom-id-2"_s, u"Custom Name 2"_s );
  QgsLayerTreeCustomNode *custom3 = group2->insertCustomNode( 0, u"custom-id-3"_s, u"Custom Name 3"_s );

  QCOMPARE( root.findCustomNode( u"custom-id"_s ), custom );
  QCOMPARE( root.findCustomNode( u"custom-id-2"_s ), custom2 );
  QCOMPARE( root.findCustomNode( u"custom-id-3"_s ), custom3 );

  // Check layer order
  const QStringList expectedLayerOrder { u"vl"_s, u"vl2"_s };
  QList< QgsMapLayer * > layerOrder = root.layerOrder();
  QStringList orderedLayers;
  for ( auto orderedLayer : layerOrder )
  {
    orderedLayers << orderedLayer->name();
  }
  QCOMPARE( orderedLayers, expectedLayerOrder );

  // Check layer and custom node order
  QStringList expectedOrder { u"Custom Name"_s, u"vl"_s, u"Custom Name 3"_s, u"vl2"_s, u"Custom Name 2"_s };
  QList< QgsLayerTreeNode * > order = root.layerAndCustomNodeOrder();
  QStringList orderedNodes;
  for ( auto orderedNode : order )
  {
    orderedNodes << orderedNode->name();
  }
  QCOMPARE( orderedNodes, expectedOrder );

  // Check reorder group 1 (does not reorder the group node, so no changes in node order)
  QList< QgsLayerTreeNode * > newOrder = { group2, vlNode };
  group->reorderGroupLayersAndCustomNodes( newOrder );
  expectedOrder.clear();
  expectedOrder << u"Custom Name"_s << u"vl"_s << u"Custom Name 3"_s << u"vl2"_s << u"Custom Name 2"_s;
  order = root.layerAndCustomNodeOrder();
  orderedNodes.clear();
  for ( const auto orderedNode : std::as_const( order ) )
  {
    orderedNodes << orderedNode->name();
  }
  QCOMPARE( orderedNodes, expectedOrder );

  // Check reorder group 2
  QList< QgsLayerTreeNode * > newOrder2 = { vlNode2, custom2, custom3 };
  group2->reorderGroupLayersAndCustomNodes( newOrder2 );
  expectedOrder.clear();
  expectedOrder << u"Custom Name"_s << u"vl"_s << u"vl2"_s << u"Custom Name 2"_s << u"Custom Name 3"_s;
  order = root.layerAndCustomNodeOrder();
  orderedNodes.clear();
  for ( const auto orderedNode : std::as_const( order ) )
  {
    orderedNodes << orderedNode->name();
  }
  QCOMPARE( orderedNodes, expectedOrder );
}

void TestQgsLayerTree::testCustomNodeDeleted()
{
  QgsLayerTree root;
  QVERIFY( !root.findCustomNode( u"custom-id"_s ) );
  root.insertCustomNode( -1, u"custom-id"_s, u"Custom Name"_s );

  QgsLayerTreeGroup *group = root.addGroup( "gr1" );
  QgsVectorLayer *vl = new QgsVectorLayer( u"Point?field=col1:integer"_s, u"vl"_s, u"memory"_s );
  group->addLayer( vl );
  group->insertCustomNode( -1, u"custom-id-2"_s, u"Custom Name 2"_s );

  QList< QgsLayerTreeNode * > order = root.layerAndCustomNodeOrder();
  group->removeCustomNode( u"non-existent"_s );
  QCOMPARE( order, root.layerAndCustomNodeOrder() );

  QVERIFY( group->findCustomNodeIds().contains( u"custom-id-2"_s ) );
  group->removeCustomNode( u"custom-id-2"_s );
  QVERIFY( order != root.layerAndCustomNodeOrder() );
  QVERIFY( !group->findCustomNodeIds().contains( u"custom-id-2"_s ) );
}

void TestQgsLayerTree::testUtilsCollectMapLayers()
{
  QgsVectorLayer *vl1 = new QgsVectorLayer( u"Point?field=col1:integer"_s, u"vl1"_s, u"memory"_s );
  QgsVectorLayer *vl2 = new QgsVectorLayer( u"Point?field=col1:integer"_s, u"vl1"_s, u"memory"_s );

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
  QgsVectorLayer *vl = new QgsVectorLayer( u"Point?field=col1:integer"_s, u"vl"_s, u"memory"_s );

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
  QgsVectorLayer *vl = new QgsVectorLayer( u"Point?field=col1:integer"_s, u"vl"_s, u"memory"_s );
  QVERIFY( vl->isValid() );

  QgsProject project;
  project.addMapLayer( vl );

  //create a categorized renderer for layer
  QgsCategorizedSymbolRenderer *renderer = new QgsCategorizedSymbolRenderer();
  renderer->setClassAttribute( u"col1"_s );
  renderer->setSourceSymbol( QgsSymbol::defaultSymbol( Qgis::GeometryType::Point ) );
  renderer->addCategory( QgsRendererCategory( "a", QgsSymbol::defaultSymbol( Qgis::GeometryType::Point ), u"a [% 1 + 2 %]"_s ) );
  renderer->addCategory( QgsRendererCategory( "b", QgsSymbol::defaultSymbol( Qgis::GeometryType::Point ), u"b,c"_s ) );
  renderer->addCategory( QgsRendererCategory( "c", QgsSymbol::defaultSymbol( Qgis::GeometryType::Point ), u"c"_s ) );
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
  settings.setWrapChar( u","_s );
  QCOMPARE( nodes.at( 0 )->data( Qt::DisplayRole ).toString(), u"a [% 1 + 2 %]"_s );
  QCOMPARE( nodes.at( 1 )->data( Qt::DisplayRole ).toString(), u"b,c"_s );
  QCOMPARE( nodes.at( 2 )->data( Qt::DisplayRole ).toString(), u"c"_s );
  nodes.at( 2 )->setUserLabel( u"[% 2+3 %] x [% 3+4 %]"_s );
  QCOMPARE( nodes.at( 2 )->data( Qt::DisplayRole ).toString(), u"[% 2+3 %] x [% 3+4 %]"_s );

  QgsExpressionContext context;
  QCOMPARE( settings.evaluateItemText( nodes.at( 0 )->data( Qt::DisplayRole ).toString(), context ), QStringList() << u"a 3"_s );
  QCOMPARE( settings.evaluateItemText( nodes.at( 1 )->data( Qt::DisplayRole ).toString(), context ), QStringList() << u"b"_s << u"c"_s );
  QCOMPARE( settings.evaluateItemText( nodes.at( 2 )->data( Qt::DisplayRole ).toString(), context ), QStringList() << u"5 x 7"_s );

  // split string should happen after expression evaluation
  QgsExpressionContextScope *scope = new QgsExpressionContextScope();
  scope->setVariable( u"bbbb"_s, u"aaaa,bbbb,cccc"_s );
  context.appendScope( scope );
  nodes.at( 2 )->setUserLabel( u"[% @bbbb %],[% 3+4 %]"_s );
  QCOMPARE( settings.evaluateItemText( nodes.at( 2 )->data( Qt::DisplayRole ).toString(), context ), QStringList() << u"aaaa"_s << u"bbbb"_s << u"cccc"_s << u"7"_s );
  //cleanup
  delete m;
  delete root;
}

void TestQgsLayerTree::testNodeDepth()
{
  QCOMPARE( mRoot->depth(), 0 );
  QgsLayerTreeNode *secondGroup = mRoot->children()[1];
  QCOMPARE( secondGroup->depth(), 1 );

  QgsVectorLayer *vl = new QgsVectorLayer( u"Point?field=col1:integer"_s, u"vl"_s, u"memory"_s );
  QVERIFY( vl->isValid() );

  QgsLayerTreeLayer *n = new QgsLayerTreeLayer( vl->id(), vl->name() );
  mRoot->addChildNode( n );
  QCOMPARE( n->depth(), 1 );

  QgsLayerTreeGroup *g1 = mRoot->addGroup( u"g1"_s );
  QCOMPARE( g1->depth(), 1 );
  QgsLayerTreeLayer *n1 = n->clone();
  g1->addChildNode( n1 );
  QCOMPARE( n1->depth(), 2 );
  QgsLayerTreeGroup *g2 = g1->addGroup( u"g2"_s );
  QCOMPARE( g2->depth(), 2 );
  QgsLayerTreeLayer *n2 = n->clone();
  g2->addChildNode( n2 );
  QCOMPARE( n2->depth(), 3 );
  QgsLayerTreeGroup *g3 = g2->addGroup( u"g3"_s );
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

  auto rl = std::make_unique<QgsRasterLayer>( QStringLiteral( TEST_DATA_DIR ) + "/tenbytenraster.asc", u"rl"_s, u"gdal"_s );
  QVERIFY( rl->isValid() );

  const std::unique_ptr<QgsLayerTreeLayer> n = std::make_unique<QgsLayerTreeLayer>( rl.get() );

  // not checkable
  QgsRasterSymbolLegendNode rasterNode( n.get(), QColor( 255, 0, 0 ), u"my node"_s, nullptr, false, u"key"_s, u"parentKey"_s );
  QVERIFY( !rasterNode.isCheckable() );
  QCOMPARE( rasterNode.ruleKey(), u"key"_s );
  QCOMPARE( static_cast<int>( rasterNode.flags() ), static_cast<int>( Qt::ItemIsEnabled | Qt::ItemIsSelectable ) );
  QCOMPARE( rasterNode.data( Qt::DisplayRole ).toString(), u"my node"_s );
  QCOMPARE( rasterNode.data( static_cast<int>( QgsLayerTreeModelLegendNode::CustomRole::NodeType ) ).toInt(), static_cast<int>( QgsLayerTreeModelLegendNode::RasterSymbolLegend ) );
  QCOMPARE( rasterNode.data( static_cast<int>( QgsLayerTreeModelLegendNode::CustomRole::RuleKey ) ).toString(), u"key"_s );
  QCOMPARE( rasterNode.data( static_cast<int>( QgsLayerTreeModelLegendNode::CustomRole::ParentRuleKey ) ).toString(), u"parentKey"_s );
  QCOMPARE( rasterNode.data( Qt::CheckStateRole ), QVariant() );
  QVERIFY( !rasterNode.setData( true, Qt::CheckStateRole ) );

  // checkable
  const QgsRasterSymbolLegendNode rasterNode2( n.get(), QColor( 255, 0, 0 ), u"my node"_s, nullptr, true, u"key"_s, u"parentKey"_s );
  QVERIFY( rasterNode2.isCheckable() );
  QCOMPARE( static_cast<int>( rasterNode2.flags() ), static_cast<int>( Qt::ItemIsEnabled | Qt::ItemIsUserCheckable | Qt::ItemIsSelectable ) );
}

void TestQgsLayerTree::testLayersEditable()
{
  QgsProject project;

  QgsVectorLayer *vl1 = new QgsVectorLayer( u"Point?field=col1:integer"_s, u"vl1"_s, u"memory"_s );
  QgsVectorLayer *vl2 = new QgsVectorLayer( u"Point?field=col1:integer"_s, u"vl1"_s, u"memory"_s );
  QgsAnnotationLayer *al = new QgsAnnotationLayer( u"al"_s, QgsAnnotationLayer::LayerOptions( project.transformContext() ) );

  project.addMapLayer( vl1 );
  project.addMapLayer( vl2 );
  project.addMapLayer( al );

  QgsLayerTree root;
  QgsLayerTreeLayer *nodeVl1 = root.addLayer( vl1 );
  QgsLayerTreeGroup *nodeGrp = root.addGroup( u"grp"_s );
  QgsLayerTreeLayer *nodeVl2 = nodeGrp->addLayer( vl2 );
  QgsLayerTreeLayer *nodeAl = nodeGrp->addLayer( al );
  QVERIFY( !QgsLayerTreeUtils::layersEditable( {} ) );
  QVERIFY( !QgsLayerTreeUtils::layersEditable( { nodeVl1, nodeVl2 } ) );
  vl1->startEditing();
  QVERIFY( QgsLayerTreeUtils::layersEditable( { nodeVl1 } ) );
  QVERIFY( QgsLayerTreeUtils::layersEditable( { nodeVl1, nodeVl2 } ) );
  QVERIFY( QgsLayerTreeUtils::layersEditable( { nodeVl2, nodeVl1 } ) );

  QVERIFY( QgsLayerTreeUtils::layersEditable( { nodeAl } ) );
  QVERIFY( QgsLayerTreeUtils::layersEditable( { nodeAl, nodeVl1 } ) );
  QVERIFY( QgsLayerTreeUtils::layersEditable( { nodeAl, nodeVl2 } ) );

  // ignore layers which can't be toggled (the annotation layer)
  QVERIFY( !QgsLayerTreeUtils::layersEditable( { nodeAl }, true ) );
  QVERIFY( QgsLayerTreeUtils::layersEditable( { nodeAl, nodeVl1 }, true ) );
  QVERIFY( !QgsLayerTreeUtils::layersEditable( { nodeAl, nodeVl2 }, true ) );
}

void TestQgsLayerTree::testInsertLayerBelow()
{
  QgsVectorLayer *topLayer = new QgsVectorLayer( u"Point?field=col1:integer"_s, u"Top Layer"_s, u"memory"_s );
  QVERIFY( topLayer->isValid() );
  QgsVectorLayer *bottomLayer = new QgsVectorLayer( u"Point?field=col1:integer"_s, u"Bottom Layer"_s, u"memory"_s );
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

void TestQgsLayerTree::testGroupReadWriteXMlServerProperties()
{
  // test retro compatibility for project before 3.44

  QgsLayerTreeGroup *group = mRoot->insertGroup( 0, u"test_grp"_s );

  QDomDocument doc( u"test-layer-tree-group"_s );
  QDomElement parentElem = doc.createElement( "root" );
  QgsReadWriteContext context;

  group->setCustomProperty( u"wmsShortName"_s, u"testShortName"_s );
  group->setCustomProperty( u"wmsTitle"_s, u"testTitle"_s );
  group->setCustomProperty( u"wmsAbstract"_s, u"testAbstract"_s );

  group->writeXml( parentElem, context );

  std::unique_ptr<QgsLayerTreeGroup> newGroup( QgsLayerTreeGroup::readXml( parentElem.firstChildElement(), context ) );
  QCOMPARE( newGroup->serverProperties()->shortName(), u"testShortName"_s );
  QCOMPARE( newGroup->serverProperties()->title(), u"testTitle"_s );
  QCOMPARE( newGroup->serverProperties()->abstract(), u"testAbstract"_s );

  QVERIFY( !newGroup->customProperty( u"wmsShortName"_s ).isValid() );
  QVERIFY( !newGroup->customProperty( u"wmsTitle"_s ).isValid() );
  QVERIFY( !newGroup->customProperty( u"wmsAbstract"_s ).isValid() );

  mRoot->removeChildNode( group );
}


QGSTEST_MAIN( TestQgsLayerTree )
#include "testqgslayertree.moc"
