/***************************************************************************
                         testqgscomposershapes.cpp
                         ----------------------
    begin                : April 2013
    copyright            : (C) 2013 by Marco Hugentobler, Nyall Dawson
    email                : nyall dot dawson at gmail.com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsapplication.h"
#include "qgscomposition.h"
#include "qgsmultirenderchecker.h"
#include "qgscomposershape.h"
#include "qgsmapsettings.h"
#include "qgsproject.h"
#include "qgssymbol.h"
#include "qgssinglesymbolrenderer.h"
#include "qgsfillsymbollayer.h"
#include <QObject>
#include "qgstest.h"
#include <QColor>
#include <QPainter>

class TestQgsComposerShapes : public QObject
{
    Q_OBJECT

  public:
    TestQgsComposerShapes() = default;

  private slots:
    void initTestCase();// will be called before the first testfunction is executed.
    void cleanupTestCase();// will be called after the last testfunction was executed.
    void init();// will be called before each testfunction is executed.
    void cleanup();// will be called after every testfunction.
    void rectangle(); //test if rectangle shape is functioning
    void triangle(); //test if triange shape is functioning
    void ellipse(); //test if ellipse shape is functioning
    void roundedRectangle(); //test if rounded rectangle shape is functioning
    void symbol(); //test is styling shapes via symbol is working

  private:
    QgsComposition *mComposition = nullptr;
    QgsComposerShape *mComposerShape = nullptr;
    QString mReport;
};

void TestQgsComposerShapes::initTestCase()
{
  QgsApplication::init();
  QgsApplication::initQgis();

  //create composition with two rectangles
  mComposition = new QgsComposition( QgsProject::instance() );
  mComposition->setPaperSize( 297, 210 ); //A4 landscape
  mComposerShape = new QgsComposerShape( 20, 20, 150, 100, mComposition );
  mComposerShape->setBackgroundColor( QColor::fromRgb( 255, 150, 0 ) );
  mComposition->addComposerShape( mComposerShape );

  mReport = QStringLiteral( "<h1>Composer Shape Tests</h1>\n" );
}

void TestQgsComposerShapes::cleanupTestCase()
{
  delete mComposition;

  QString myReportFile = QDir::tempPath() + "/qgistest.html";
  QFile myFile( myReportFile );
  if ( myFile.open( QIODevice::WriteOnly | QIODevice::Append ) )
  {
    QTextStream myQTextStream( &myFile );
    myQTextStream << mReport;
    myFile.close();
  }
  QgsApplication::exitQgis();
}

void TestQgsComposerShapes::init()
{

}

void TestQgsComposerShapes::cleanup()
{

}

void TestQgsComposerShapes::rectangle()
{
  mComposerShape->setShapeType( QgsComposerShape::Rectangle );

  QgsCompositionChecker checker( QStringLiteral( "composershapes_rectangle" ), mComposition );
  checker.setControlPathPrefix( QStringLiteral( "composer_shapes" ) );
  QVERIFY( checker.testComposition( mReport ) );
}

void TestQgsComposerShapes::triangle()
{
  mComposerShape->setShapeType( QgsComposerShape::Triangle );

  QgsCompositionChecker checker( QStringLiteral( "composershapes_triangle" ), mComposition );
  checker.setControlPathPrefix( QStringLiteral( "composer_shapes" ) );
  QVERIFY( checker.testComposition( mReport ) );
}

void TestQgsComposerShapes::ellipse()
{
  mComposerShape->setShapeType( QgsComposerShape::Ellipse );

  QgsCompositionChecker checker( QStringLiteral( "composershapes_ellipse" ), mComposition );
  checker.setControlPathPrefix( QStringLiteral( "composer_shapes" ) );
  QVERIFY( checker.testComposition( mReport ) );
}

void TestQgsComposerShapes::roundedRectangle()
{
  mComposerShape->setShapeType( QgsComposerShape::Rectangle );
  mComposerShape->setCornerRadius( 30 );

  QgsCompositionChecker checker( QStringLiteral( "composershapes_roundedrect" ), mComposition );
  checker.setControlPathPrefix( QStringLiteral( "composer_shapes" ) );
  QVERIFY( checker.testComposition( mReport ) );
  mComposerShape->setCornerRadius( 0 );
}

void TestQgsComposerShapes::symbol()
{
  mComposerShape->setShapeType( QgsComposerShape::Rectangle );

  //setup simple fill
  QgsSimpleFillSymbolLayer *simpleFill = new QgsSimpleFillSymbolLayer();
  QgsFillSymbol *fillSymbol = new QgsFillSymbol();
  fillSymbol->changeSymbolLayer( 0, simpleFill );
  simpleFill->setColor( Qt::green );
  simpleFill->setStrokeColor( Qt::yellow );
  simpleFill->setStrokeWidth( 6 );

  mComposerShape->setShapeStyleSymbol( fillSymbol );
  mComposerShape->setUseSymbol( true );
  delete fillSymbol;

  QgsCompositionChecker checker( QStringLiteral( "composershapes_symbol" ), mComposition );
  checker.setControlPathPrefix( QStringLiteral( "composer_shapes" ) );
  QVERIFY( checker.testComposition( mReport ) );
}

QGSTEST_MAIN( TestQgsComposerShapes )
#include "testqgscomposershapes.moc"
