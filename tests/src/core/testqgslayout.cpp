/***************************************************************************
                         testqgslayout.cpp
                         -----------------
    begin                : June 2017
    copyright            : (C) 2017 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgslayout.h"
#include "qgstest.h"
#include "qgsproject.h"
#include "qgslayoutitemmap.h"
#include "qgslayoutitemshape.h"

class TestQgsLayout: public QObject
{
    Q_OBJECT

  private slots:
    void initTestCase();// will be called before the first testfunction is executed.
    void cleanupTestCase();// will be called after the last testfunction was executed.
    void init();// will be called before each testfunction is executed.
    void cleanup();// will be called after every testfunction.
    void creation(); //test creation of QgsLayout
    void units();
    void name();
    void customProperties();
    void variablesEdited();
    void scope();
    void referenceMap();
    void bounds();
    void addItem();
    void layoutItems();
    void layoutItemByUuid();
    void undoRedoOccurred();

  private:
    QString mReport;

};

void TestQgsLayout::initTestCase()
{
  mReport = QStringLiteral( "<h1>Layout Tests</h1>\n" );
}

void TestQgsLayout::cleanupTestCase()
{
  QString myReportFile = QDir::tempPath() + QDir::separator() + "qgistest.html";
  QFile myFile( myReportFile );
  if ( myFile.open( QIODevice::WriteOnly | QIODevice::Append ) )
  {
    QTextStream myQTextStream( &myFile );
    myQTextStream << mReport;
    myFile.close();
  }
}

void TestQgsLayout::init()
{

}

void TestQgsLayout::cleanup()
{

}

void TestQgsLayout::creation()
{
  QgsProject p;
  QgsLayout *layout = new QgsLayout( &p );
  QVERIFY( layout );
  QCOMPARE( layout->project(), &p );
  delete layout;
}

void TestQgsLayout::units()
{
  QgsProject p;
  QgsLayout layout( &p );
  layout.setUnits( QgsUnitTypes::LayoutCentimeters );
  QCOMPARE( layout.units(), QgsUnitTypes::LayoutCentimeters );
  QCOMPARE( layout.convertToLayoutUnits( QgsLayoutMeasurement( 10.0, QgsUnitTypes::LayoutMillimeters ) ), 1.0 );
  QCOMPARE( layout.convertToLayoutUnits( QgsLayoutSize( 10.0, 20.0, QgsUnitTypes::LayoutMillimeters ) ), QSizeF( 1.0, 2.0 ) );
  QCOMPARE( layout.convertToLayoutUnits( QgsLayoutPoint( 10.0, 20.0, QgsUnitTypes::LayoutMillimeters ) ), QPointF( 1.0, 2.0 ) );
  QCOMPARE( layout.convertFromLayoutUnits( 1.0, QgsUnitTypes::LayoutMillimeters ), QgsLayoutMeasurement( 10.0, QgsUnitTypes::LayoutMillimeters ) );
  QCOMPARE( layout.convertFromLayoutUnits( QSizeF( 1.0, 2.0 ), QgsUnitTypes::LayoutMillimeters ), QgsLayoutSize( 10.0, 20.0, QgsUnitTypes::LayoutMillimeters ) );
  QCOMPARE( layout.convertFromLayoutUnits( QPointF( 1.0, 2.0 ), QgsUnitTypes::LayoutMillimeters ), QgsLayoutPoint( 10.0, 20.0, QgsUnitTypes::LayoutMillimeters ) );

  //check with dpi conversion
  layout.setUnits( QgsUnitTypes::LayoutInches );
  layout.context().setDpi( 96.0 );
  QCOMPARE( layout.context().dpi(), 96.0 );
  QCOMPARE( layout.convertToLayoutUnits( QgsLayoutMeasurement( 96, QgsUnitTypes::LayoutPixels ) ), 1.0 );
  QCOMPARE( layout.convertToLayoutUnits( QgsLayoutSize( 96, 96, QgsUnitTypes::LayoutPixels ) ), QSizeF( 1.0, 1.0 ) );
  QCOMPARE( layout.convertToLayoutUnits( QgsLayoutPoint( 96, 96, QgsUnitTypes::LayoutPixels ) ), QPointF( 1.0, 1.0 ) );
  QgsLayoutMeasurement result = layout.convertFromLayoutUnits( 1.0, QgsUnitTypes::LayoutPixels );
  QCOMPARE( result.units(), QgsUnitTypes::LayoutPixels );
  QCOMPARE( result.length(), 96.0 );
  QgsLayoutSize sizeResult = layout.convertFromLayoutUnits( QSizeF( 1.0, 1.0 ), QgsUnitTypes::LayoutPixels );
  QCOMPARE( sizeResult.units(), QgsUnitTypes::LayoutPixels );
  QCOMPARE( sizeResult.width(), 96.0 );
  QCOMPARE( sizeResult.height(), 96.0 );
  QgsLayoutPoint pointResult = layout.convertFromLayoutUnits( QPointF( 1.0, 1.0 ), QgsUnitTypes::LayoutPixels );
  QCOMPARE( pointResult.units(), QgsUnitTypes::LayoutPixels );
  QCOMPARE( pointResult.x(), 96.0 );
  QCOMPARE( pointResult.y(), 96.0 );

  layout.setUnits( QgsUnitTypes::LayoutPixels );
  QCOMPARE( layout.convertToLayoutUnits( QgsLayoutMeasurement( 1, QgsUnitTypes::LayoutInches ) ), 96.0 );
  QCOMPARE( layout.convertToLayoutUnits( QgsLayoutSize( 1, 2, QgsUnitTypes::LayoutInches ) ), QSizeF( 96.0, 192.0 ) );
  QCOMPARE( layout.convertToLayoutUnits( QgsLayoutPoint( 1, 2, QgsUnitTypes::LayoutInches ) ), QPointF( 96.0, 192.0 ) );
  result = layout.convertFromLayoutUnits( 96.0, QgsUnitTypes::LayoutInches );
  QCOMPARE( result.units(), QgsUnitTypes::LayoutInches );
  QCOMPARE( result.length(), 1.0 );
  sizeResult = layout.convertFromLayoutUnits( QSizeF( 96.0, 192.0 ), QgsUnitTypes::LayoutInches );
  QCOMPARE( sizeResult.units(), QgsUnitTypes::LayoutInches );
  QCOMPARE( sizeResult.width(), 1.0 );
  QCOMPARE( sizeResult.height(), 2.0 );
  pointResult = layout.convertFromLayoutUnits( QPointF( 96.0, 192.0 ), QgsUnitTypes::LayoutInches );
  QCOMPARE( pointResult.units(), QgsUnitTypes::LayoutInches );
  QCOMPARE( pointResult.x(), 1.0 );
  QCOMPARE( pointResult.y(), 2.0 );
}

void TestQgsLayout::name()
{
  QgsProject p;
  QgsLayout layout( &p );
  QString layoutName = QStringLiteral( "test name" );
  layout.setName( layoutName );
  QCOMPARE( layout.name(), layoutName );
}

void TestQgsLayout::customProperties()
{
  QgsProject p;
  QgsLayout *layout = new QgsLayout( &p );

  QCOMPARE( layout->customProperty( "noprop", "defaultval" ).toString(), QString( "defaultval" ) );
  QVERIFY( layout->customProperties().isEmpty() );
  layout->setCustomProperty( QStringLiteral( "testprop" ), "testval" );
  QCOMPARE( layout->customProperty( "testprop", "defaultval" ).toString(), QString( "testval" ) );
  QCOMPARE( layout->customProperties().length(), 1 );
  QCOMPARE( layout->customProperties().at( 0 ), QString( "testprop" ) );

  //test no crash
  layout->removeCustomProperty( QStringLiteral( "badprop" ) );

  layout->removeCustomProperty( QStringLiteral( "testprop" ) );
  QVERIFY( layout->customProperties().isEmpty() );
  QCOMPARE( layout->customProperty( "noprop", "defaultval" ).toString(), QString( "defaultval" ) );

  layout->setCustomProperty( QStringLiteral( "testprop1" ), "testval1" );
  layout->setCustomProperty( QStringLiteral( "testprop2" ), "testval2" );
  QStringList keys = layout->customProperties();
  QCOMPARE( keys.length(), 2 );
  QVERIFY( keys.contains( "testprop1" ) );
  QVERIFY( keys.contains( "testprop2" ) );

  delete layout;
}

void TestQgsLayout::variablesEdited()
{
  QgsProject p;
  QgsLayout l( &p );
  QSignalSpy spyVariablesChanged( &l, &QgsLayout::variablesChanged );

  l.setCustomProperty( QStringLiteral( "not a variable" ), "1" );
  QVERIFY( spyVariablesChanged.count() == 0 );
  l.setCustomProperty( QStringLiteral( "variableNames" ), "1" );
  QVERIFY( spyVariablesChanged.count() == 1 );
  l.setCustomProperty( QStringLiteral( "variableValues" ), "1" );
  QVERIFY( spyVariablesChanged.count() == 2 );
}

void TestQgsLayout::scope()
{
  QgsProject p;
  QgsLayout l( &p );

  // no crash
  std::unique_ptr< QgsExpressionContextScope > scope( QgsExpressionContextUtils::layoutScope( nullptr ) );
  l.setName( QStringLiteral( "test" ) );
  scope.reset( QgsExpressionContextUtils::layoutScope( &l ) );
  QCOMPARE( scope->variable( "layout_name" ).toString(), QStringLiteral( "test" ) );

  QgsExpressionContextUtils::setLayoutVariable( &l, QStringLiteral( "new_var" ), 5 );
  QgsExpressionContextUtils::setLayoutVariable( &l, QStringLiteral( "new_var2" ), 15 );
  scope.reset( QgsExpressionContextUtils::layoutScope( &l ) );
  QCOMPARE( scope->variable( "layout_name" ).toString(), QStringLiteral( "test" ) );
  QCOMPARE( scope->variable( "new_var" ).toInt(), 5 );
  QCOMPARE( scope->variable( "new_var2" ).toInt(), 15 );

  QVariantMap newVars;
  newVars.insert( QStringLiteral( "new_var3" ), 17 );
  QgsExpressionContextUtils::setLayoutVariables( &l, newVars );
  scope.reset( QgsExpressionContextUtils::layoutScope( &l ) );
  QCOMPARE( scope->variable( "layout_name" ).toString(), QStringLiteral( "test" ) );
  QVERIFY( !scope->hasVariable( "new_var" ) );
  QVERIFY( !scope->hasVariable( "new_var2" ) );
  QCOMPARE( scope->variable( "new_var3" ).toInt(), 17 );

  p.setTitle( QStringLiteral( "my title" ) );
  QgsExpressionContext c = l.createExpressionContext();
  // should contain project variables
  QCOMPARE( c.variable( "project_title" ).toString(), QStringLiteral( "my title" ) );
  // and layout variables
  QCOMPARE( c.variable( "new_var3" ).toInt(), 17 );

}

void TestQgsLayout::referenceMap()
{
  QgsRectangle extent( 2000, 2800, 2500, 2900 );
  QgsProject p;
  QgsLayout l( &p );

  // no maps
  QVERIFY( !l.referenceMap() );
#if 0

  QgsLayoutItemMap *map = new QgsLayoutItemMap( &l );
  map->setNewExtent( extent );
  map->setSceneRect( QRectF( 30, 60, 200, 100 ) );
  l.addComposerMap( map );
  QCOMPARE( l.referenceMap(), map );
#endif
#if 0 // TODO

  // add a larger map
  QgsLayoutItemMap *map2 = new QgsLayoutItemMap( &l );
  map2->setNewExtent( extent );
  map2->setSceneRect( QRectF( 30, 60, 250, 150 ) );
  l.addComposerMap( map2 );
  QCOMPARE( l.referenceMap(), map2 );
  // explicitly set reference map
  l.setReferenceMap( map );
  QCOMPARE( l.referenceMap(), map );
#endif

}

void TestQgsLayout::bounds()
{
  //add some items to a layout
  QgsProject p;
  QgsLayout l( &p );
  l.initializeDefaults();

  QgsLayoutItemShape *shape1 = new QgsLayoutItemShape( &l );
  shape1->attemptResize( QgsLayoutSize( 90, 50 ) );
  shape1->attemptMove( QgsLayoutPoint( 90, 50 ) );
  shape1->setItemRotation( 45 );
  l.addLayoutItem( shape1 );
  QgsLayoutItemShape *shape2 = new QgsLayoutItemShape( &l );
  shape2->attemptResize( QgsLayoutSize( 110, 50 ) );
  shape2->attemptMove( QgsLayoutPoint( 100, 150 ) );
  l.addLayoutItem( shape2 );

#if 0
  QgsLayoutItemRectangularShape *shape3 = new QgsLayoutItemRectangularShape( &l );
  l.addLayoutItem( shape3 );
  shape3->setItemPosition( 210, 30, 50, 100, QgsComposerItem::UpperLeft, false, 2 );
  QgsLayoutItemRectangularShape *shape4 = new QgsLayoutItemRectangularShape( &l );
  l.addLayoutItem( shape4 );
  shape4->setItemPosition( 10, 120, 50, 30, QgsComposerItem::UpperLeft, false, 2 );
  shape4->setVisibility( false );
#endif

  //check bounds
  QRectF layoutBounds = l.layoutBounds( false );
#if 0 // correct values when 2nd page items are added back in
  QGSCOMPARENEAR( layoutBounds.height(), 372.15, 0.01 );
  QGSCOMPARENEAR( layoutBounds.width(), 301.00, 0.01 );
  QGSCOMPARENEAR( layoutBounds.left(), -2, 0.01 );
  QGSCOMPARENEAR( layoutBounds.top(), -2, 0.01 );

  QRectF compositionBoundsNoPage = l.layoutBounds( true );
  QGSCOMPARENEAR( compositionBoundsNoPage.height(), 320.36, 0.01 );
  QGSCOMPARENEAR( compositionBoundsNoPage.width(), 250.30, 0.01 );
  QGSCOMPARENEAR( compositionBoundsNoPage.left(), 9.85, 0.01 );
  QGSCOMPARENEAR( compositionBoundsNoPage.top(), 49.79, 0.01 );
#endif

  QGSCOMPARENEAR( layoutBounds.height(), 210.000000, 0.01 );
  QGSCOMPARENEAR( layoutBounds.width(), 297.000000, 0.01 );
  QGSCOMPARENEAR( layoutBounds.left(), 0.00000, 0.01 );
  QGSCOMPARENEAR( layoutBounds.top(), 0.00000, 0.01 );

  QRectF compositionBoundsNoPage = l.layoutBounds( true );
  QGSCOMPARENEAR( compositionBoundsNoPage.height(), 174.859607, 0.01 );
  QGSCOMPARENEAR( compositionBoundsNoPage.width(), 124.859607, 0.01 );
  QGSCOMPARENEAR( compositionBoundsNoPage.left(), 85.290393, 0.01 );
  QGSCOMPARENEAR( compositionBoundsNoPage.top(), 25.290393, 0.01 );

#if 0
  QRectF page1Bounds = composition->pageItemBounds( 0, true );
  QGSCOMPARENEAR( page1Bounds.height(), 150.36, 0.01 );
  QGSCOMPARENEAR( page1Bounds.width(), 155.72, 0.01 );
  QGSCOMPARENEAR( page1Bounds.left(), 54.43, 0.01 );
  QGSCOMPARENEAR( page1Bounds.top(), 49.79, 0.01 );

  QRectF page2Bounds = composition->pageItemBounds( 1, true );
  QGSCOMPARENEAR( page2Bounds.height(), 100.30, 0.01 );
  QGSCOMPARENEAR( page2Bounds.width(), 50.30, 0.01 );
  QGSCOMPARENEAR( page2Bounds.left(), 209.85, 0.01 );
  QGSCOMPARENEAR( page2Bounds.top(), 249.85, 0.01 );

  QRectF page2BoundsWithHidden = composition->pageItemBounds( 1, false );
  QGSCOMPARENEAR( page2BoundsWithHidden.height(), 120.30, 0.01 );
  QGSCOMPARENEAR( page2BoundsWithHidden.width(), 250.30, 0.01 );
  QGSCOMPARENEAR( page2BoundsWithHidden.left(), 9.85, 0.01 );
  QGSCOMPARENEAR( page2BoundsWithHidden.top(), 249.85, 0.01 );
#endif
}

void TestQgsLayout::addItem()
{
  QgsProject p;
  QgsLayout l( &p );
  l.pageCollection()->deletePage( 0 );

  QgsLayoutItemShape *shape1 = new QgsLayoutItemShape( &l );
  shape1->setFrameEnabled( false );
  shape1->attemptResize( QgsLayoutSize( 140, 70 ) );
  shape1->attemptMove( QgsLayoutPoint( 90, 50 ) );

  l.addLayoutItem( shape1 );
  QVERIFY( l.items().contains( shape1 ) );
  // bounds should be updated to include item
  QGSCOMPARENEAR( l.sceneRect().left(), 89.850, 0.001 );
  QGSCOMPARENEAR( l.sceneRect().top(), 49.85, 0.001 );
  QGSCOMPARENEAR( l.sceneRect().width(), 140.30, 0.001 );
  QGSCOMPARENEAR( l.sceneRect().height(), 70.3, 0.001 );

  QgsLayoutItemShape *shape2 = new QgsLayoutItemShape( &l );
  shape2->attemptResize( QgsLayoutSize( 240, 170 ) );
  shape2->attemptMove( QgsLayoutPoint( 30, 20 ) );
  shape2->setFrameEnabled( false );

  // don't use addLayoutItem - we want to manually trigger a bounds update
  l.addItem( shape2 );
  QGSCOMPARENEAR( l.sceneRect().left(), 89.85, 0.001 );
  QGSCOMPARENEAR( l.sceneRect().top(), 49.85, 0.001 );
  QGSCOMPARENEAR( l.sceneRect().width(), 140.3, 0.001 );
  QGSCOMPARENEAR( l.sceneRect().height(), 70.3, 0.001 );

  l.updateBounds();
  // bounds should be updated to include item
  QGSCOMPARENEAR( l.sceneRect().left(), 29.85, 0.001 );
  QGSCOMPARENEAR( l.sceneRect().top(), 19.85, 0.001 );
  QGSCOMPARENEAR( l.sceneRect().width(), 240.3, 0.001 );
  QGSCOMPARENEAR( l.sceneRect().height(), 170.3, 0.001 );
}

void TestQgsLayout::layoutItems()
{
  QgsProject p;
  QgsLayout l( &p );
  l.pageCollection()->deletePage( 0 );

  QgsLayoutItemShape *shape1 = new QgsLayoutItemShape( &l );
  l.addLayoutItem( shape1 );

  QgsLayoutItemShape *shape2 = new QgsLayoutItemShape( &l );
  l.addLayoutItem( shape2 );

  QgsLayoutItemMap *map1 = new QgsLayoutItemMap( &l );
  l.addLayoutItem( map1 );

  QList< QgsLayoutItem * > items;
  l.layoutItems( items );
  QCOMPARE( items.count(), 3 );
  QVERIFY( items.contains( shape1 ) );
  QVERIFY( items.contains( shape2 ) );
  QVERIFY( items.contains( map1 ) );

  QList< QgsLayoutItemShape * > shapes;
  l.layoutItems( shapes );
  QCOMPARE( shapes.count(), 2 );
  QVERIFY( shapes.contains( shape1 ) );
  QVERIFY( shapes.contains( shape2 ) );

  QList< QgsLayoutItemMap * > maps;
  l.layoutItems( maps );
  QCOMPARE( maps.count(), 1 );
  QVERIFY( maps.contains( map1 ) );
}

void TestQgsLayout::layoutItemByUuid()
{
  QgsProject p;
  QgsLayout l( &p );
  l.pageCollection()->deletePage( 0 );

  QgsLayoutItemShape *shape1 = new QgsLayoutItemShape( &l );
  l.addLayoutItem( shape1 );

  QgsLayoutItemShape *shape2 = new QgsLayoutItemShape( &l );
  l.addLayoutItem( shape2 );

  QgsLayoutItemMap *map1 = new QgsLayoutItemMap( &l );
  l.addLayoutItem( map1 );

  QVERIFY( !l.itemByUuid( QStringLiteral( "xxx" ) ) );
  QCOMPARE( l.itemByUuid( shape1->uuid() ), shape1 );
  QCOMPARE( l.itemByUuid( shape2->uuid() ), shape2 );
  QCOMPARE( l.itemByUuid( map1->uuid() ), map1 );
}

void TestQgsLayout::undoRedoOccurred()
{
  // test emitting undo/redo occurred signal
  QgsProject proj;
  QgsLayout l( &proj );

  QSignalSpy spyOccurred( l.undoStack(), &QgsLayoutUndoStack::undoRedoOccurredForItems );

  QgsLayoutItemShape *item = new QgsLayoutItemShape( &l );
  l.addLayoutItem( item );

  QCOMPARE( spyOccurred.count(), 0 );
  //adds a new undo command
  item->setId( "test" );
  QCOMPARE( spyOccurred.count(), 0 );

  QgsLayoutItemShape *item2 = new QgsLayoutItemShape( &l );
  l.addLayoutItem( item2 );
  item2->setId( "test2" );
  QCOMPARE( spyOccurred.count(), 0 );

  l.undoStack()->stack()->undo();
  QCOMPARE( spyOccurred.count(), 1 );
  QSet< QString > items = qvariant_cast< QSet< QString > >( spyOccurred.at( 0 ).at( 0 ) );
  QCOMPARE( items, QSet< QString >() << item2->uuid() );

  l.undoStack()->stack()->redo();
  QCOMPARE( spyOccurred.count(), 2 );
  items = qvariant_cast< QSet< QString> >( spyOccurred.at( 1 ).at( 0 ) );
  QCOMPARE( items, QSet< QString >() << item2->uuid() );

  // macro undo
  l.undoStack()->beginMacro( QString() );
  item->setId( "new id" );
  item2->setId( "new id2" );
  l.undoStack()->endMacro();
  QCOMPARE( spyOccurred.count(), 2 );

  l.undoStack()->stack()->undo();
  QCOMPARE( spyOccurred.count(), 3 );
  items = qvariant_cast< QSet< QString > >( spyOccurred.at( 2 ).at( 0 ) );
  QCOMPARE( items, QSet< QString >() << item->uuid() << item2->uuid() );
  l.undoStack()->stack()->redo();
  QCOMPARE( spyOccurred.count(), 4 );
  items = qvariant_cast< QSet< QString > >( spyOccurred.at( 3 ).at( 0 ) );
  QCOMPARE( items, QSet< QString >() << item->uuid() << item2->uuid() );

}


QGSTEST_MAIN( TestQgsLayout )
#include "testqgslayout.moc"
