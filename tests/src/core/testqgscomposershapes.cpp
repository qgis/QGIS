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
#include "qgscompositionchecker.h"
#include "qgscomposershape.h"
#include "qgsmapsettings.h"
#include "qgssymbolv2.h"
#include "qgssinglesymbolrendererv2.h"
#include "qgsfillsymbollayerv2.h"
#include <QObject>
#include <QtTest/QtTest>
#include <QColor>
#include <QPainter>

class TestQgsComposerShapes : public QObject
{
    Q_OBJECT
  private slots:
    void initTestCase();// will be called before the first testfunction is executed.
    void cleanupTestCase();// will be called after the last testfunction was executed.
    void init();// will be called before each testfunction is executed.
    void cleanup();// will be called after every testfunction.
    void rectangle(); //test if rectangle shape is functioning
    void triangle(); //test if triange shape is functioning
    void ellipse(); //test if ellipse shape is functioning
    void roundedRectangle(); //test if rounded rectangle shape is functioning
    void symbolV2(); //test is styling shapes via symbolv2 is working

  private:
    QgsComposition* mComposition;
    QgsComposerShape* mComposerShape;
    QgsMapSettings mMapSettings;
    QgsSimpleFillSymbolLayerV2* mSimpleFill;
    QgsFillSymbolV2* mFillSymbol;
    QString mReport;
};

void TestQgsComposerShapes::initTestCase()
{
  QgsApplication::init();
  QgsApplication::initQgis();

  //create composition with two rectangles
  mComposition = new QgsComposition( mMapSettings );
  mComposition->setPaperSize( 297, 210 ); //A4 landscape
  mComposerShape = new QgsComposerShape( 20, 20, 150, 100, mComposition );
  mComposerShape->setBackgroundColor( QColor::fromRgb( 255, 150, 0 ) );
  mComposition->addComposerShape( mComposerShape );

  //setup simple fill
  mSimpleFill = new QgsSimpleFillSymbolLayerV2();
  mFillSymbol = new QgsFillSymbolV2();
  mFillSymbol->changeSymbolLayer( 0, mSimpleFill );

  mReport = "<h1>Composer Shape Tests</h1>\n";
}

void TestQgsComposerShapes::cleanupTestCase()
{
  delete mComposition;

  QString myReportFile = QDir::tempPath() + QDir::separator() + "qgistest.html";
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

  QgsCompositionChecker checker( "composershapes_rectangle", mComposition );
  QVERIFY( checker.testComposition( mReport ) );
}

void TestQgsComposerShapes::triangle()
{
  mComposerShape->setShapeType( QgsComposerShape::Triangle );

  QgsCompositionChecker checker( "composershapes_triangle", mComposition );
  QVERIFY( checker.testComposition( mReport ) );
}

void TestQgsComposerShapes::ellipse()
{
  mComposerShape->setShapeType( QgsComposerShape::Ellipse );

  QgsCompositionChecker checker( "composershapes_ellipse", mComposition );
  QVERIFY( checker.testComposition( mReport ) );
}

void TestQgsComposerShapes::roundedRectangle()
{
  mComposerShape->setShapeType( QgsComposerShape::Rectangle );
  mComposerShape->setCornerRadius( 30 );

  QgsCompositionChecker checker( "composershapes_roundedrect", mComposition );
  QVERIFY( checker.testComposition( mReport ) );
  mComposerShape->setCornerRadius( 0 );
}

void TestQgsComposerShapes::symbolV2()
{
  mComposerShape->setShapeType( QgsComposerShape::Rectangle );

  mSimpleFill->setColor( Qt::green );
  mSimpleFill->setBorderColor( Qt::yellow );
  mSimpleFill->setBorderWidth( 6 );

  mComposerShape->setShapeStyleSymbol( mFillSymbol );
  mComposerShape->setUseSymbolV2( true );

  QgsCompositionChecker checker( "composershapes_symbolv2", mComposition );
  QVERIFY( checker.testComposition( mReport ) );
}

QTEST_MAIN( TestQgsComposerShapes )
#include "testqgscomposershapes.moc"
