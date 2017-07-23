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

  private:
    QString mReport;

};

void TestQgsLayout::initTestCase()
{
  mReport = "<h1>Layout Tests</h1>\n";
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
  QString layoutName = "test name";
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
  l.setName( "test" );
  scope.reset( QgsExpressionContextUtils::layoutScope( &l ) );
  QCOMPARE( scope->variable( "layout_name" ).toString(), QStringLiteral( "test" ) );

  QgsExpressionContextUtils::setLayoutVariable( &l, "new_var", 5 );
  QgsExpressionContextUtils::setLayoutVariable( &l, "new_var2", 15 );
  scope.reset( QgsExpressionContextUtils::layoutScope( &l ) );
  QCOMPARE( scope->variable( "layout_name" ).toString(), QStringLiteral( "test" ) );
  QCOMPARE( scope->variable( "new_var" ).toInt(), 5 );
  QCOMPARE( scope->variable( "new_var2" ).toInt(), 15 );

  QVariantMap newVars;
  newVars.insert( "new_var3", 17 );
  QgsExpressionContextUtils::setLayoutVariables( &l, newVars );
  scope.reset( QgsExpressionContextUtils::layoutScope( &l ) );
  QCOMPARE( scope->variable( "layout_name" ).toString(), QStringLiteral( "test" ) );
  QVERIFY( !scope->hasVariable( "new_var" ) );
  QVERIFY( !scope->hasVariable( "new_var2" ) );
  QCOMPARE( scope->variable( "new_var3" ).toInt(), 17 );

  p.setTitle( "my title" );
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


QGSTEST_MAIN( TestQgsLayout )
#include "testqgslayout.moc"
