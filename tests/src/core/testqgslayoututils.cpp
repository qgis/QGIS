/***************************************************************************
                         testqgslayoututils.cpp
                         ---------------------
    begin                : July 2017
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
#include "qgslayoututils.h"
#include "qgstestutils.h"
#include "qgsproject.h"
#include "qgslayoutitemmap.h"

class TestQgsLayoutUtils: public QObject
{
    Q_OBJECT

  private slots:
    void initTestCase();// will be called before the first testfunction is executed.
    void cleanupTestCase();// will be called after the last testfunction was executed.
    void init();// will be called before each testfunction is executed.
    void cleanup();// will be called after every testfunction.
    void normalizedAngle(); //test normalised angle function
    void createRenderContextFromLayout();
    void createRenderContextFromMap();

  private:
    QString mReport;

};

void TestQgsLayoutUtils::initTestCase()
{
  mReport = "<h1>Layout Utils Tests</h1>\n";
}

void TestQgsLayoutUtils::cleanupTestCase()
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

void TestQgsLayoutUtils::init()
{

}

void TestQgsLayoutUtils::cleanup()
{

}

void TestQgsLayoutUtils::normalizedAngle()
{
  QList< QPair< double, double > > testVals;
  testVals << qMakePair( 0.0, 0.0 );
  testVals << qMakePair( 90.0, 90.0 );
  testVals << qMakePair( 180.0, 180.0 );
  testVals << qMakePair( 270.0, 270.0 );
  testVals << qMakePair( 360.0, 0.0 );
  testVals << qMakePair( 390.0, 30.0 );
  testVals << qMakePair( 720.0, 0.0 );
  testVals << qMakePair( 730.0, 10.0 );
  testVals << qMakePair( -10.0, 350.0 );
  testVals << qMakePair( -360.0, 0.0 );
  testVals << qMakePair( -370.0, 350.0 );
  testVals << qMakePair( -760.0, 320.0 );

  //test normalized angle helper function
  QList< QPair< double, double > >::const_iterator it = testVals.constBegin();
  for ( ; it != testVals.constEnd(); ++it )

  {
    double result = QgsLayoutUtils::normalizedAngle( ( *it ).first );
    qDebug() << QString( "actual: %1 expected: %2" ).arg( result ).arg( ( *it ).second );
    QVERIFY( qgsDoubleNear( result, ( *it ).second ) );

  }

  //test with allowing negative angles
  QList< QPair< double, double > > negativeTestVals;
  negativeTestVals << qMakePair( 0.0, 0.0 );
  negativeTestVals << qMakePair( 90.0, 90.0 );
  negativeTestVals << qMakePair( 360.0, 0.0 );
  negativeTestVals << qMakePair( -10.0, -10.0 );
  negativeTestVals << qMakePair( -359.0, -359.0 );
  negativeTestVals << qMakePair( -360.0, 0.0 );
  negativeTestVals << qMakePair( -361.0, -1.0 );
  negativeTestVals << qMakePair( -370.0, -10.0 );
  negativeTestVals << qMakePair( -760.0, -40.0 );
  it = negativeTestVals.constBegin();
  for ( ; it != negativeTestVals.constEnd(); ++it )

  {
    double result = QgsLayoutUtils::normalizedAngle( ( *it ).first, true );
    qDebug() << QString( "actual: %1 expected: %2" ).arg( result ).arg( ( *it ).second );
    QVERIFY( qgsDoubleNear( result, ( *it ).second ) );

  }
}


void TestQgsLayoutUtils::createRenderContextFromLayout()
{
  QImage testImage = QImage( 250, 250, QImage::Format_RGB32 );
  testImage.setDotsPerMeterX( 150 / 25.4 * 1000 );
  testImage.setDotsPerMeterY( 150 / 25.4 * 1000 );
  QPainter p( &testImage );

  // no composition
  QgsRenderContext rc = QgsLayoutUtils::createRenderContextForLayout( nullptr, &p );
  QGSCOMPARENEAR( rc.scaleFactor(), 150 / 25.4, 0.001 );
  QCOMPARE( rc.painter(), &p );

  // no composition, no painter
  rc = QgsLayoutUtils::createRenderContextForLayout( nullptr, nullptr );
  QGSCOMPARENEAR( rc.scaleFactor(), 88 / 25.4, 0.001 );
  QVERIFY( !rc.painter() );

  //create composition with no reference map
  QgsRectangle extent( 2000, 2800, 2500, 2900 );
  QgsProject project;
  QgsLayout l( &project );
  rc = QgsLayoutUtils::createRenderContextForLayout( &l, &p );
  QGSCOMPARENEAR( rc.scaleFactor(), 150 / 25.4, 0.001 );
  QCOMPARE( rc.painter(), &p );

  // layout, no map, no painter
  rc = QgsLayoutUtils::createRenderContextForLayout( &l, nullptr );
  QGSCOMPARENEAR( rc.scaleFactor(), 88 / 25.4, 0.001 );
  QVERIFY( !rc.painter() );

  // add a reference map
  QgsLayoutItemMap *map = new QgsLayoutItemMap( &l );
#if 0 // TODO
  map->setNewExtent( extent );
  map->setSceneRect( QRectF( 30, 60, 200, 100 ) );
  composition->addComposerMap( map );
#endif
  l.setReferenceMap( map );

  rc = QgsLayoutUtils::createRenderContextForLayout( &l, &p );
  QGSCOMPARENEAR( rc.scaleFactor(), 150 / 25.4, 0.001 );
  QGSCOMPARENEAR( rc.rendererScale(), map->scale(), 1000000 );
  QCOMPARE( rc.painter(), &p );

  // layout, reference map, no painter
  rc = QgsLayoutUtils::createRenderContextForLayout( &l, nullptr );
  QGSCOMPARENEAR( rc.scaleFactor(), 88 / 25.4, 0.001 );
  QGSCOMPARENEAR( rc.rendererScale(), map->scale(), 1000000 );
  QVERIFY( !rc.painter() );

  p.end();
}

void TestQgsLayoutUtils::createRenderContextFromMap()
{
  QImage testImage = QImage( 250, 250, QImage::Format_RGB32 );
  testImage.setDotsPerMeterX( 150 / 25.4 * 1000 );
  testImage.setDotsPerMeterY( 150 / 25.4 * 1000 );
  QPainter p( &testImage );

  // no map
  QgsRenderContext rc = QgsLayoutUtils::createRenderContextForMap( nullptr, &p );
  QGSCOMPARENEAR( rc.scaleFactor(), 150 / 25.4, 0.001 );
  QCOMPARE( rc.painter(), &p );

  // no map, no painter
  rc = QgsLayoutUtils::createRenderContextForMap( nullptr, nullptr );
  QGSCOMPARENEAR( rc.scaleFactor(), 88 / 25.4, 0.001 );
  QVERIFY( !rc.painter() );

  //create composition with no reference map
  QgsRectangle extent( 2000, 2800, 2500, 2900 );
  QgsProject project;
  QgsLayout l( &project );

#if 0 // TODO
  // add a map
  QgsLayoutItemMap *map = new QgsLayoutItemMap( &l );

  map->setNewExtent( extent );
  map->setSceneRect( QRectF( 30, 60, 200, 100 ) );
  l.addComposerMap( map );
#endif

#if 0 //TODO
  rc = QgsLayoutUtils::createRenderContextForMap( map, &p );
  QGSCOMPARENEAR( rc.scaleFactor(), 150 / 25.4, 0.001 );
  QGSCOMPARENEAR( rc.rendererScale(), map->scale(), 1000000 );
  QCOMPARE( rc.painter(), &p );

  // map, no painter
  rc = QgsLayoutUtils::createRenderContextForMap( map, nullptr );
  QGSCOMPARENEAR( rc.scaleFactor(), 88 / 25.4, 0.001 );
  QGSCOMPARENEAR( rc.rendererScale(), map->scale(), 1000000 );
  QVERIFY( !rc.painter() );

  // secondary map
  QgsLayoutItemMap *map2 = new QgsLayoutItemMap( &l );

  map2->setNewExtent( extent );
  map2->setSceneRect( QRectF( 30, 60, 100, 50 ) );
  composition->addComposerMap( map2 );

  rc = QgsLayoutUtils::createRenderContextForMap( map2, &p );
  QGSCOMPARENEAR( rc.scaleFactor(), 150 / 25.4, 0.001 );
  QGSCOMPARENEAR( rc.rendererScale(), map2->scale(), 1000000 );
  QVERIFY( rc.painter() );
#endif
  p.end();
}

QGSTEST_MAIN( TestQgsLayoutUtils )
#include "testqgslayoututils.moc"
