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
#include "qgslayoutpagecollection.h"
#include "qgslayoutundostack.h"
#include "qgslayoutitemlabel.h"
#include "qgslayoutitempolyline.h"
#include "qgslayoutitemhtml.h"
#include "qgslayoutframe.h"
#include "qgsprintlayout.h"
#include "qgslayoutatlas.h"

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
    void layoutItemById();
    void undoRedoOccurred();
    void itemsOnPage(); //test fetching matching items on a set page
    void shouldExportPage();
    void pageIsEmpty();
    void clear();
    void georeference();
    void clone();

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
  layout.renderContext().setDpi( 96.0 );
  QCOMPARE( layout.renderContext().dpi(), 96.0 );
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

  QgsLayoutItemMap *map = new QgsLayoutItemMap( &l );
  map->attemptSetSceneRect( QRectF( 30, 60, 200, 100 ) );
  map->setExtent( extent );
  l.addLayoutItem( map );
  QCOMPARE( l.referenceMap(), map );

  // add a larger map
  QgsLayoutItemMap *map2 = new QgsLayoutItemMap( &l );
  map2->attemptSetSceneRect( QRectF( 30, 60, 250, 150 ) );
  map2->setExtent( extent );
  l.addLayoutItem( map2 );

  QCOMPARE( l.referenceMap(), map2 );
  // explicitly set reference map
  l.setReferenceMap( map );
  QCOMPARE( l.referenceMap(), map );
}

void TestQgsLayout::bounds()
{
  //add some items to a layout
  QgsProject p;
  QgsLayout l( &p );
  QgsLayoutItemPage *page = new QgsLayoutItemPage( &l );
  page->setPageSize( "A4", QgsLayoutItemPage::Landscape );
  l.pageCollection()->addPage( page );
  QgsLayoutItemPage *page2 = new QgsLayoutItemPage( &l );
  page2->setPageSize( "A4", QgsLayoutItemPage::Landscape );
  l.pageCollection()->addPage( page2 );

  QgsLayoutItemShape *shape1 = new QgsLayoutItemShape( &l );
  shape1->attemptResize( QgsLayoutSize( 90, 50 ) );
  shape1->attemptMove( QgsLayoutPoint( 90, 50 ) );
  shape1->setItemRotation( 45, false );
  l.addLayoutItem( shape1 );
  QgsLayoutItemShape *shape2 = new QgsLayoutItemShape( &l );
  shape2->attemptResize( QgsLayoutSize( 110, 50 ) );
  shape2->attemptMove( QgsLayoutPoint( 100, 150 ), true, false, 0 );
  l.addLayoutItem( shape2 );
  QgsLayoutItemShape *shape3 = new QgsLayoutItemShape( &l );
  l.addLayoutItem( shape3 );
  shape3->attemptResize( QgsLayoutSize( 50, 100 ) );
  shape3->attemptMove( QgsLayoutPoint( 210, 30 ), true, false, 1 );
  QgsLayoutItemShape *shape4 = new QgsLayoutItemShape( &l );
  l.addLayoutItem( shape4 );
  shape4->attemptResize( QgsLayoutSize( 50, 30 ) );
  shape4->attemptMove( QgsLayoutPoint( 10, 120 ), true, false, 1 );
  shape4->setVisibility( false );

  //check bounds
  QRectF layoutBounds = l.layoutBounds( false );
  QGSCOMPARENEAR( layoutBounds.height(), 430, 0.01 );
  QGSCOMPARENEAR( layoutBounds.width(), 297.00, 0.01 );
  QGSCOMPARENEAR( layoutBounds.left(), 0.0, 0.01 );
  QGSCOMPARENEAR( layoutBounds.top(), 0.0, 0.01 );

  QRectF layoutBoundsNoPage = l.layoutBounds( true );
  QGSCOMPARENEAR( layoutBoundsNoPage.height(), 320.36, 0.01 );
  QGSCOMPARENEAR( layoutBoundsNoPage.width(), 250.30, 0.01 );
  QGSCOMPARENEAR( layoutBoundsNoPage.left(), 9.85, 0.01 );
  QGSCOMPARENEAR( layoutBoundsNoPage.top(), 49.79, 0.01 );

  QRectF page1Bounds = l.pageItemBounds( 0, true );
  QGSCOMPARENEAR( page1Bounds.height(), 150.36, 0.01 );
  QGSCOMPARENEAR( page1Bounds.width(), 155.72, 0.01 );
  QGSCOMPARENEAR( page1Bounds.left(), 54.43, 0.01 );
  QGSCOMPARENEAR( page1Bounds.top(), 49.79, 0.01 );

  QRectF page2Bounds = l.pageItemBounds( 1, true );
  QGSCOMPARENEAR( page2Bounds.height(), 100.30, 0.01 );
  QGSCOMPARENEAR( page2Bounds.width(), 50.30, 0.01 );
  QGSCOMPARENEAR( page2Bounds.left(), 209.85, 0.01 );
  QGSCOMPARENEAR( page2Bounds.top(), 249.85, 0.01 );

  QRectF page2BoundsWithHidden = l.pageItemBounds( 1, false );
  QGSCOMPARENEAR( page2BoundsWithHidden.height(), 120.30, 0.01 );
  QGSCOMPARENEAR( page2BoundsWithHidden.width(), 250.30, 0.01 );
  QGSCOMPARENEAR( page2BoundsWithHidden.left(), 9.85, 0.01 );
  QGSCOMPARENEAR( page2BoundsWithHidden.top(), 249.85, 0.01 );
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

void TestQgsLayout::layoutItemById()
{
  QgsProject p;
  QgsLayout l( &p );

  QgsLayoutItemShape *shape1 = new QgsLayoutItemShape( &l );
  l.addLayoutItem( shape1 );
  shape1->setId( QStringLiteral( "shape" ) );

  QgsLayoutItemShape *shape2 = new QgsLayoutItemShape( &l );
  l.addLayoutItem( shape2 );
  shape2->setId( QStringLiteral( "shape" ) );

  QgsLayoutItemMap *map1 = new QgsLayoutItemMap( &l );
  l.addLayoutItem( map1 );
  map1->setId( QStringLiteral( "map" ) );

  QVERIFY( !l.itemById( QStringLiteral( "xxx" ) ) );
  QVERIFY( l.itemById( QStringLiteral( "shape" ) ) == shape1 || l.itemById( QStringLiteral( "shape" ) ) == shape2 );
  QCOMPARE( l.itemById( map1->id() ), map1 );
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

  // blocking undo
  int before = l.undoStack()->stack()->count();
  item->setId( "xxx" );
  QCOMPARE( l.undoStack()->stack()->count(), before + 1 );
  l.undoStack()->blockCommands( true );
  QVERIFY( l.undoStack()->isBlocked() );
  item->setId( "yyy" );
  QCOMPARE( l.undoStack()->stack()->count(), before + 1 ); // no new command
  l.undoStack()->blockCommands( true ); // second stacked command
  QVERIFY( l.undoStack()->isBlocked() );
  item->setId( "ZZZ" );
  QCOMPARE( l.undoStack()->stack()->count(), before + 1 ); // no new command
  l.undoStack()->blockCommands( false ); // one stacked command left
  QVERIFY( l.undoStack()->isBlocked() );
  item->setId( "sss" );
  QCOMPARE( l.undoStack()->stack()->count(), before + 1 ); // no new command
  l.undoStack()->blockCommands( false ); // unblocked
  QVERIFY( !l.undoStack()->isBlocked() );
  item->setId( "ttt" );
  QCOMPARE( l.undoStack()->stack()->count(), before + 2 ); // new command
  l.undoStack()->blockCommands( false ); // don't allow negative stack size
  QVERIFY( !l.undoStack()->isBlocked() );
  item->setId( "uuu" );
  QCOMPARE( l.undoStack()->stack()->count(), before + 3 ); // new command
  l.undoStack()->blockCommands( true ); // should be blocked again
  QVERIFY( l.undoStack()->isBlocked() );
  item->setId( "vvv" );
  QCOMPARE( l.undoStack()->stack()->count(), before + 3 ); // no new command
  // blocked macro
  l.undoStack()->beginMacro( "macro" );
  item->setId( "lll" );
  l.undoStack()->endMacro();
  QCOMPARE( l.undoStack()->stack()->count(), before + 3 ); // no new command
}

void TestQgsLayout::itemsOnPage()
{
  QgsProject proj;
  QgsLayout l( &proj );
  QgsLayoutItemPage *page = new QgsLayoutItemPage( &l );
  page->setPageSize( "A4" );
  l.pageCollection()->addPage( page );
  QgsLayoutItemPage *page2 = new QgsLayoutItemPage( &l );
  page2->setPageSize( "A4" );
  l.pageCollection()->addPage( page2 );
  QgsLayoutItemPage *page3 = new QgsLayoutItemPage( &l );
  page3->setPageSize( "A4" );
  l.pageCollection()->addPage( page3 );

  QgsLayoutItemLabel *label1 = new QgsLayoutItemLabel( &l );
  l.addLayoutItem( label1 );
  label1->attemptMove( QgsLayoutPoint( 10, 10 ), true, false, 0 );
  QgsLayoutItemLabel *label2 = new QgsLayoutItemLabel( &l );
  l.addLayoutItem( label2 );
  label2->attemptMove( QgsLayoutPoint( 10, 10 ), true, false, 0 );
  QgsLayoutItemLabel *label3 = new QgsLayoutItemLabel( &l );
  l.addLayoutItem( label3 );
  label3->attemptMove( QgsLayoutPoint( 10, 10 ), true, false, 1 );
  QgsLayoutItemShape *shape1 = new QgsLayoutItemShape( &l );
  l.addLayoutItem( shape1 );
  shape1->attemptMove( QgsLayoutPoint( 10, 10 ), true, false, 0 );
  QgsLayoutItemShape *shape2 = new QgsLayoutItemShape( &l );
  l.addLayoutItem( shape2 );
  shape2->attemptMove( QgsLayoutPoint( 10, 10 ), true, false, 1 );
  QgsLayoutItemPolyline *arrow1 = new QgsLayoutItemPolyline( &l );
  l.addLayoutItem( arrow1 );
  arrow1->attemptMove( QgsLayoutPoint( 10, 10 ), true, false, 2 );
  QgsLayoutItemPolyline *arrow2 = new QgsLayoutItemPolyline( &l );
  l.addLayoutItem( arrow2 );
  arrow2->attemptMove( QgsLayoutPoint( 10, 10 ), true, false, 2 );

  //fetch items - remember that these numbers include the paper item!
  QList<QgsLayoutItem *> items = l.pageCollection()->itemsOnPage( 0 );
  //should be 4 items on page 1
  QCOMPARE( items.length(), 4 );
  items = l.pageCollection()->itemsOnPage( 1 );
  //should be 3 items on page 2
  QCOMPARE( items.length(), 3 );
  items = l.pageCollection()->itemsOnPage( 2 );
  //should be 3 items on page 3
  QCOMPARE( items.length(), 3 );

  //check fetching specific item types
  QList<QgsLayoutItemLabel *> labels;
  l.pageCollection()->itemsOnPage( labels, 0 );
  //should be 2 labels on page 1
  QCOMPARE( labels.length(), 2 );
  l.pageCollection()->itemsOnPage( labels, 1 );
  //should be 1 label on page 2
  QCOMPARE( labels.length(), 1 );
  l.pageCollection()->itemsOnPage( labels, 2 );
  //should be no label on page 3
  QCOMPARE( labels.length(), 0 );

  QList<QgsLayoutItemShape *> shapes;
  l.pageCollection()->itemsOnPage( shapes, 0 );
  //should be 1 shapes on page 1
  QCOMPARE( shapes.length(), 1 );
  l.pageCollection()->itemsOnPage( shapes, 1 );
  //should be 1 shapes on page 2
  QCOMPARE( shapes.length(), 1 );
  l.pageCollection()->itemsOnPage( shapes, 2 );
  //should be no shapes on page 3
  QCOMPARE( shapes.length(), 0 );

  QList<QgsLayoutItemPolyline *> arrows;
  l.pageCollection()->itemsOnPage( arrows, 0 );
  //should be no arrows on page 1
  QCOMPARE( arrows.length(), 0 );
  l.pageCollection()->itemsOnPage( arrows, 1 );
  //should be no arrows on page 2
  QCOMPARE( arrows.length(), 0 );
  l.pageCollection()->itemsOnPage( arrows, 2 );
  //should be 2 arrows on page 3
  QCOMPARE( arrows.length(), 2 );

  l.removeLayoutItem( label1 );
  l.removeLayoutItem( label2 );
  l.removeLayoutItem( label3 );
  l.removeLayoutItem( shape1 );
  l.removeLayoutItem( shape2 );
  l.removeLayoutItem( arrow1 );
  l.removeLayoutItem( arrow2 );

  //check again with removed items
  items = l.pageCollection()->itemsOnPage( 0 );
  QCOMPARE( items.length(), 1 );
  items = l.pageCollection()->itemsOnPage( 1 );
  QCOMPARE( items.length(), 1 );
  items = l.pageCollection()->itemsOnPage( 2 );
  QCOMPARE( items.length(), 1 );

  l.pageCollection()->itemsOnPage( labels, 0 );
  QCOMPARE( labels.length(), 0 );
  l.pageCollection()->itemsOnPage( labels, 1 );
  QCOMPARE( labels.length(), 0 );
  l.pageCollection()->itemsOnPage( labels, 2 );
  QCOMPARE( labels.length(), 0 );
}

void TestQgsLayout::shouldExportPage()
{
  QgsProject proj;
  QgsLayout l( &proj );
  QgsLayoutItemPage *page = new QgsLayoutItemPage( &l );
  page->setPageSize( "A4" );
  l.pageCollection()->addPage( page );
  QgsLayoutItemPage *page2 = new QgsLayoutItemPage( &l );
  page2->setPageSize( "A4" );
  l.pageCollection()->addPage( page2 );
  l.renderContext().mIsPreviewRender = false;

  QgsLayoutItemHtml *htmlItem = new QgsLayoutItemHtml( &l );
  //frame on page 1
  QgsLayoutFrame *frame1 = new QgsLayoutFrame( &l, htmlItem );
  frame1->attemptSetSceneRect( QRectF( 0, 0, 100, 100 ) );
  //frame on page 2
  QgsLayoutFrame *frame2 = new QgsLayoutFrame( &l, htmlItem );
  frame2->attemptSetSceneRect( QRectF( 0, 320, 100, 100 ) );
  frame2->setHidePageIfEmpty( true );
  htmlItem->addFrame( frame1 );
  htmlItem->addFrame( frame2 );
  htmlItem->setContentMode( QgsLayoutItemHtml::ManualHtml );
  //short content, so frame 2 should be empty
  htmlItem->setHtml( QStringLiteral( "<p><i>Test manual <b>html</b></i></p>" ) );
  htmlItem->loadHtml();

  QVERIFY( l.pageCollection()->shouldExportPage( 0 ) );
  QVERIFY( !l.pageCollection()->shouldExportPage( 1 ) );

  //long content, so frame 2 should not be empty
  htmlItem->setHtml( QStringLiteral( "<p style=\"height: 10000px\"><i>Test manual <b>html</b></i></p>" ) );
  htmlItem->loadHtml();

  QVERIFY( l.pageCollection()->shouldExportPage( 0 ) );
  QVERIFY( l.pageCollection()->shouldExportPage( 1 ) );

  //...and back again...
  htmlItem->setHtml( QStringLiteral( "<p><i>Test manual <b>html</b></i></p>" ) );
  htmlItem->loadHtml();

  QVERIFY( l.pageCollection()->shouldExportPage( 0 ) );
  QVERIFY( !l.pageCollection()->shouldExportPage( 1 ) );

  // get rid of frames
  l.removeItem( frame1 );
  l.removeItem( frame2 );
  l.removeMultiFrame( htmlItem );
  delete htmlItem;
  QgsApplication::sendPostedEvents( nullptr, QEvent::DeferredDelete );

  QVERIFY( l.pageCollection()->shouldExportPage( 0 ) );
  QVERIFY( l.pageCollection()->shouldExportPage( 1 ) );

  // explicitly set exclude from exports
  l.pageCollection()->page( 0 )->setExcludeFromExports( true );
  QVERIFY( !l.pageCollection()->shouldExportPage( 0 ) );
  QVERIFY( l.pageCollection()->shouldExportPage( 1 ) );

  l.pageCollection()->page( 0 )->setExcludeFromExports( false );
  l.pageCollection()->page( 1 )->setExcludeFromExports( true );
  QVERIFY( l.pageCollection()->shouldExportPage( 0 ) );
  QVERIFY( !l.pageCollection()->shouldExportPage( 1 ) );

  l.pageCollection()->page( 1 )->setExcludeFromExports( false );
  l.pageCollection()->page( 0 )->dataDefinedProperties().setProperty( QgsLayoutObject::ExcludeFromExports, QgsProperty::fromExpression( "1" ) );
  l.pageCollection()->page( 1 )->dataDefinedProperties().setProperty( QgsLayoutObject::ExcludeFromExports, QgsProperty::fromValue( true ) );
  l.refresh();
  QVERIFY( !l.pageCollection()->shouldExportPage( 0 ) );
  QVERIFY( !l.pageCollection()->shouldExportPage( 1 ) );
}

void TestQgsLayout::pageIsEmpty()
{
  QgsProject proj;
  QgsLayout l( &proj );
  QgsLayoutItemPage *page = new QgsLayoutItemPage( &l );
  page->setPageSize( "A4" );
  l.pageCollection()->addPage( page );
  QgsLayoutItemPage *page2 = new QgsLayoutItemPage( &l );
  page2->setPageSize( "A4" );
  l.pageCollection()->addPage( page2 );
  QgsLayoutItemPage *page3 = new QgsLayoutItemPage( &l );
  page3->setPageSize( "A4" );
  l.pageCollection()->addPage( page3 );

  //add some items to the composition
  QgsLayoutItemShape *label1 = new QgsLayoutItemShape( &l );
  l.addLayoutItem( label1 );
  label1->attemptMove( QgsLayoutPoint( 10, 10 ), true, false, 0 );
  QgsLayoutItemShape *label2 = new QgsLayoutItemShape( &l );
  l.addLayoutItem( label2 );
  label2->attemptMove( QgsLayoutPoint( 10, 10 ), true, false, 0 );
  QgsLayoutItemShape *label3 = new QgsLayoutItemShape( &l );
  l.addLayoutItem( label3 );
  label3->attemptMove( QgsLayoutPoint( 10, 10 ), true, false, 2 );

  //only page 2 should be empty
  QCOMPARE( l.pageCollection()->pageIsEmpty( 0 ), false );
  QCOMPARE( l.pageCollection()->pageIsEmpty( 1 ), true );
  QCOMPARE( l.pageCollection()->pageIsEmpty( 2 ), false );

  //remove the items
  l.removeLayoutItem( label1 );
  l.removeLayoutItem( label2 );
  l.removeLayoutItem( label3 );

  //expect everything to be empty now
  QCOMPARE( l.pageCollection()->pageIsEmpty( 0 ), true );
  QCOMPARE( l.pageCollection()->pageIsEmpty( 1 ), true );
  QCOMPARE( l.pageCollection()->pageIsEmpty( 2 ), true );
}

void TestQgsLayout::clear()
{
  QgsProject proj;
  QgsLayout l( &proj );
  QgsLayoutItemPage *page = new QgsLayoutItemPage( &l );
  page->setPageSize( "A4" );
  l.pageCollection()->addPage( page );
  QgsLayoutItemPage *page2 = new QgsLayoutItemPage( &l );
  page2->setPageSize( "A4" );
  l.pageCollection()->addPage( page2 );
  QgsLayoutItemPage *page3 = new QgsLayoutItemPage( &l );
  page3->setPageSize( "A4" );
  l.pageCollection()->addPage( page3 );

  //add some items to the composition
  QgsLayoutItemShape *label1 = new QgsLayoutItemShape( &l );
  l.addLayoutItem( label1 );
  QPointer< QgsLayoutItem > item1P = label1;
  QgsLayoutItemShape *label2 = new QgsLayoutItemShape( &l );
  l.addLayoutItem( label2 );
  QPointer< QgsLayoutItem > item2P = label2;
  QgsLayoutItemShape *label3 = new QgsLayoutItemShape( &l );
  l.addLayoutItem( label3 );
  QPointer< QgsLayoutItem > item3P = label3;

  l.clear();
  QgsApplication::sendPostedEvents( nullptr, QEvent::DeferredDelete );
  QCOMPARE( l.pageCollection()->pageCount(), 0 );
  QVERIFY( !item1P );
  QVERIFY( !item2P );
  QVERIFY( !item3P );
  QList< QgsLayoutItem * > items;
  l.layoutItems( items );
  QVERIFY( items.empty() );
  QCOMPARE( l.undoStack()->stack()->count(), 0 );
}

void TestQgsLayout::georeference()
{
  QgsRectangle extent( 2000, 2800, 2500, 2900 );
  QgsLayout l( QgsProject::instance() );
  l.initializeDefaults();

  QgsLayoutExporter exporter( &l );

  // no map
  std::unique_ptr< double [] > t = exporter.computeGeoTransform( nullptr );
  QVERIFY( !t );

  QgsLayoutItemMap *map = new QgsLayoutItemMap( &l );
  map->attemptSetSceneRect( QRectF( 30, 60, 200, 100 ) );
  map->setExtent( extent );
  l.addLayoutItem( map );

  t = exporter.computeGeoTransform( map );
  QGSCOMPARENEAR( t[0], 1925.0, 1.0 );
  QGSCOMPARENEAR( t[1], 0.211719, 0.0001 );
  QGSCOMPARENEAR( t[2], 0.0, 4 * DBL_EPSILON );
  QGSCOMPARENEAR( t[3], 3050, 1 );
  QGSCOMPARENEAR( t[4], 0.0, 4 * DBL_EPSILON );
  QGSCOMPARENEAR( t[5], -0.211694, 0.0001 );
  t.reset();

  // don't specify map
  l.setReferenceMap( map );
  t = exporter.computeGeoTransform();
  QGSCOMPARENEAR( t[0], 1925.0, 1.0 );
  QGSCOMPARENEAR( t[1], 0.211719, 0.0001 );
  QGSCOMPARENEAR( t[2], 0.0, 4 * DBL_EPSILON );
  QGSCOMPARENEAR( t[3], 3050, 1 );
  QGSCOMPARENEAR( t[4], 0.0, 4 * DBL_EPSILON );
  QGSCOMPARENEAR( t[5], -0.211694, 0.0001 );
  t.reset();

  // specify extent
  t = exporter.computeGeoTransform( map, QRectF( 70, 100, 50, 60 ) );
  QGSCOMPARENEAR( t[0], 2100.0, 1.0 );
  QGSCOMPARENEAR( t[1], 0.211864, 0.0001 );
  QGSCOMPARENEAR( t[2], 0.0, 4 * DBL_EPSILON );
  QGSCOMPARENEAR( t[3], 2800, 1 );
  QGSCOMPARENEAR( t[4], 0.0, 4 * DBL_EPSILON );
  QGSCOMPARENEAR( t[5], -0.211864, 0.0001 );
  t.reset();

  // specify dpi
  t = exporter.computeGeoTransform( map, QRectF(), 75 );
  QGSCOMPARENEAR( t[0], 1925.0, 1 );
  QGSCOMPARENEAR( t[1], 0.847603, 0.0001 );
  QGSCOMPARENEAR( t[2], 0.0, 4 * DBL_EPSILON );
  QGSCOMPARENEAR( t[3], 3050.0, 1 );
  QGSCOMPARENEAR( t[4], 0.0, 4 * DBL_EPSILON );
  QGSCOMPARENEAR( t[5], -0.846774, 0.0001 );
  t.reset();

  // rotation
  map->setMapRotation( 45 );
  t = exporter.computeGeoTransform( map );
  QGSCOMPARENEAR( t[0], 1878.768940, 1 );
  QGSCOMPARENEAR( t[1], 0.149708, 0.0001 );
  QGSCOMPARENEAR( t[2], 0.149708, 0.0001 );
  QGSCOMPARENEAR( t[3], 2761.611652, 1 );
  QGSCOMPARENEAR( t[4], 0.14969, 0.0001 );
  QGSCOMPARENEAR( t[5], -0.14969, 0.0001 );
  t.reset();
}

void TestQgsLayout::clone()
{
  QgsProject proj;
  QgsLayout l( &proj );
  QgsLayoutItemPage *page = new QgsLayoutItemPage( &l );
  page->setPageSize( "A4" );
  l.pageCollection()->addPage( page );
  QgsLayoutItemPage *page2 = new QgsLayoutItemPage( &l );
  page2->setPageSize( "A4" );
  l.pageCollection()->addPage( page2 );
  QgsLayoutItemPage *page3 = new QgsLayoutItemPage( &l );
  page3->setPageSize( "A4" );
  l.pageCollection()->addPage( page3 );

  //add some items to the composition
  QgsLayoutItemShape *label1 = new QgsLayoutItemShape( &l );
  l.addLayoutItem( label1 );
  QgsLayoutItemShape *label2 = new QgsLayoutItemShape( &l );
  l.addLayoutItem( label2 );
  QgsLayoutItemShape *label3 = new QgsLayoutItemShape( &l );
  l.addLayoutItem( label3 );

  // clone and check a few poperties
  std::unique_ptr< QgsLayout > cloned( l.clone() );
  QVERIFY( cloned.get() );
  QCOMPARE( cloned->pageCollection()->pageCount(), 3 );
  QList< QgsLayoutItem * > items;
  cloned->layoutItems( items );
  QCOMPARE( items.count(), 6 ); // 3 pages + 3 items

  // clone a print layout
  QgsPrintLayout pl( &proj );
  pl.atlas()->setPageNameExpression( QStringLiteral( "not a real expression" ) );
  std::unique_ptr< QgsPrintLayout > plClone( pl.clone() );
  QVERIFY( plClone.get() );
  QCOMPARE( plClone->atlas()->pageNameExpression(), QStringLiteral( "not a real expression" ) );
}


QGSTEST_MAIN( TestQgsLayout )
#include "testqgslayout.moc"
