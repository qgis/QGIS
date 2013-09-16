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
#include <QObject>
#include <QtTest>
#include <QColor>
#include <QPainter>

class TestQgsComposerShapes: public QObject
{
    Q_OBJECT;
  private slots:
    void initTestCase();// will be called before the first testfunction is executed.
    void cleanupTestCase();// will be called after the last testfunction was executed.
    void init();// will be called before each testfunction is executed.
    void cleanup();// will be called after every testfunction.
    void rectangle(); //test if rectangle shape is functioning
    void triangle(); //test if triange shape is functioning
    void ellipse(); //test if ellipse shape is functioning
    void roundedRectangle(); //test if rounded rectangle shape is functioning

  private:
    QgsComposition* mComposition;
    QgsComposerShape* mComposerShape;
};

void TestQgsComposerShapes::initTestCase()
{
  QgsApplication::init();
  QgsApplication::initQgis();

  //create composition with two rectangles
  mComposition = new QgsComposition( 0 );
  mComposition->setPaperSize( 297, 210 ); //A4 landscape
  mComposerShape = new QgsComposerShape( 20, 20, 150, 100, mComposition );
  mComposerShape->setBackgroundColor( QColor::fromRgb( 255, 150, 0 ) );  
  mComposition->addComposerShape( mComposerShape );  
}

void TestQgsComposerShapes::cleanupTestCase()
{
  delete mComposition;
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

  QgsCompositionChecker checker( "Composer shapes", mComposition, QString( QString( TEST_DATA_DIR ) + QDir::separator() +
                                 "control_images" + QDir::separator() + "expected_composershapes" + QDir::separator() + "composershape_rectangle.png" ) );
  QVERIFY( checker.testComposition() );
}

void TestQgsComposerShapes::triangle()
{
  mComposerShape->setShapeType( QgsComposerShape::Triangle );

  QgsCompositionChecker checker( "Composer shapes", mComposition, QString( QString( TEST_DATA_DIR ) + QDir::separator() +
                                 "control_images" + QDir::separator() + "expected_composershapes" + QDir::separator() + "composershape_triangle.png" ) );
  QVERIFY( checker.testComposition() );
}

void TestQgsComposerShapes::ellipse()
{
  mComposerShape->setShapeType( QgsComposerShape::Ellipse );

  QgsCompositionChecker checker( "Composer shapes", mComposition, QString( QString( TEST_DATA_DIR ) + QDir::separator() +
                                 "control_images" + QDir::separator() + "expected_composershapes" + QDir::separator() + "composershape_ellipse.png" ) );
  QVERIFY( checker.testComposition() );
}

void TestQgsComposerShapes::roundedRectangle()
{
  mComposerShape->setShapeType( QgsComposerShape::Rectangle );
  mComposerShape->setCornerRadius( 30 );
  
  QgsCompositionChecker checker( "Composer shapes", mComposition, QString( QString( TEST_DATA_DIR ) + QDir::separator() +
                                 "control_images" + QDir::separator() + "expected_composershapes" + QDir::separator() + "composershape_roundedrectangle.png" ) );
  QVERIFY( checker.testComposition() );
  mComposerShape->setCornerRadius( 0 );
}

QTEST_MAIN( TestQgsComposerShapes )
#include "moc_testqgscomposershapes.cxx"
