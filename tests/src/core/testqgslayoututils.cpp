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
    void snappedAngle();
    void createRenderContextFromLayout();
    void createRenderContextFromMap();
    void relativePosition();
    void relativeResizeRect();

  private:
    QString mReport;

};

void TestQgsLayoutUtils::initTestCase()
{
  mReport = QStringLiteral( "<h1>Layout Utils Tests</h1>\n" );
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
    qDebug() << QStringLiteral( "actual: %1 expected: %2" ).arg( result ).arg( ( *it ).second );
    QGSCOMPARENEAR( result, ( *it ).second, 4 * DBL_EPSILON );

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
    qDebug() << QStringLiteral( "actual: %1 expected: %2" ).arg( result ).arg( ( *it ).second );
    QGSCOMPARENEAR( result, ( *it ).second, 4 * DBL_EPSILON );

  }
}

void TestQgsLayoutUtils::snappedAngle()
{
  QList< QPair< double, double > > testVals;
  testVals << qMakePair( 0.0, 0.0 );
  testVals << qMakePair( 10.0, 0.0 );
  testVals << qMakePair( 20.0, 0.0 );
  testVals << qMakePair( 30.0, 45.0 );
  testVals << qMakePair( 40.0, 45.0 );
  testVals << qMakePair( 50.0, 45.0 );
  testVals << qMakePair( 60.0, 45.0 );
  testVals << qMakePair( 70.0, 90.0 );
  testVals << qMakePair( 80.0, 90.0 );
  testVals << qMakePair( 90.0, 90.0 );
  testVals << qMakePair( 100.0, 90.0 );
  testVals << qMakePair( 110.0, 90.0 );
  testVals << qMakePair( 120.0, 135.0 );
  testVals << qMakePair( 130.0, 135.0 );
  testVals << qMakePair( 140.0, 135.0 );
  testVals << qMakePair( 150.0, 135.0 );
  testVals << qMakePair( 160.0, 180.0 );
  testVals << qMakePair( 170.0, 180.0 );
  testVals << qMakePair( 180.0, 180.0 );
  testVals << qMakePair( 190.0, 180.0 );
  testVals << qMakePair( 200.0, 180.0 );
  testVals << qMakePair( 210.0, 225.0 );
  testVals << qMakePair( 220.0, 225.0 );
  testVals << qMakePair( 230.0, 225.0 );
  testVals << qMakePair( 240.0, 225.0 );
  testVals << qMakePair( 250.0, 270.0 );
  testVals << qMakePair( 260.0, 270.0 );
  testVals << qMakePair( 270.0, 270.0 );
  testVals << qMakePair( 280.0, 270.0 );
  testVals << qMakePair( 290.0, 270.0 );
  testVals << qMakePair( 300.0, 315.0 );
  testVals << qMakePair( 310.0, 315.0 );
  testVals << qMakePair( 320.0, 315.0 );
  testVals << qMakePair( 330.0, 315.0 );
  testVals << qMakePair( 340.0, 0.0 );
  testVals << qMakePair( 350.0, 0.0 );
  testVals << qMakePair( 360.0, 0.0 );

  //test snapped angle helper function
  QList< QPair< double, double > >::const_iterator it = testVals.constBegin();
  for ( ; it != testVals.constEnd(); ++it )
  {
    QGSCOMPARENEAR( QgsLayoutUtils::snappedAngle( ( *it ).first ), ( *it ).second, 4 * DBL_EPSILON );
  }
}


void TestQgsLayoutUtils::createRenderContextFromLayout()
{
  QImage testImage = QImage( 250, 250, QImage::Format_RGB32 );
  testImage.setDotsPerMeterX( 150 / 25.4 * 1000 );
  testImage.setDotsPerMeterY( 150 / 25.4 * 1000 );
  QPainter p( &testImage );

  // no layout
  QgsRenderContext rc = QgsLayoutUtils::createRenderContextForLayout( nullptr, &p );
  QGSCOMPARENEAR( rc.scaleFactor(), 150 / 25.4, 0.001 );
  QCOMPARE( rc.painter(), &p );

  // no layout, no painter
  rc = QgsLayoutUtils::createRenderContextForLayout( nullptr, nullptr );
  QGSCOMPARENEAR( rc.scaleFactor(), 88 / 25.4, 0.001 );
  QVERIFY( !rc.painter() );

  //create layout with no reference map
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

  // check render context flags are correctly set
  l.context().setFlags( 0 );
  rc = QgsLayoutUtils::createRenderContextForLayout( &l, nullptr );
  QVERIFY( !( rc.flags() & QgsRenderContext::Antialiasing ) );
  QVERIFY( !( rc.flags() & QgsRenderContext::UseAdvancedEffects ) );
  QVERIFY( ( rc.flags() & QgsRenderContext::ForceVectorOutput ) );

  l.context().setFlag( QgsLayoutContext::FlagAntialiasing );
  rc = QgsLayoutUtils::createRenderContextForLayout( &l, nullptr );
  QVERIFY( ( rc.flags() & QgsRenderContext::Antialiasing ) );
  QVERIFY( !( rc.flags() & QgsRenderContext::UseAdvancedEffects ) );
  QVERIFY( ( rc.flags() & QgsRenderContext::ForceVectorOutput ) );

  l.context().setFlag( QgsLayoutContext::FlagUseAdvancedEffects );
  rc = QgsLayoutUtils::createRenderContextForLayout( &l, nullptr );
  QVERIFY( ( rc.flags() & QgsRenderContext::Antialiasing ) );
  QVERIFY( ( rc.flags() & QgsRenderContext::UseAdvancedEffects ) );
  QVERIFY( ( rc.flags() & QgsRenderContext::ForceVectorOutput ) );

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

  // check render context flags are correctly set
  l.context().setFlags( 0 );
  rc = QgsLayoutUtils::createRenderContextForLayout( &l, nullptr );
  QVERIFY( !( rc.flags() & QgsRenderContext::Antialiasing ) );
  QVERIFY( !( rc.flags() & QgsRenderContext::UseAdvancedEffects ) );
  QVERIFY( ( rc.flags() & QgsRenderContext::ForceVectorOutput ) );

  l.context().setFlag( QgsLayoutContext::FlagAntialiasing );
  rc = QgsLayoutUtils::createRenderContextForLayout( &l, nullptr );
  QVERIFY( ( rc.flags() & QgsRenderContext::Antialiasing ) );
  QVERIFY( !( rc.flags() & QgsRenderContext::UseAdvancedEffects ) );
  QVERIFY( ( rc.flags() & QgsRenderContext::ForceVectorOutput ) );

  l.context().setFlag( QgsLayoutContext::FlagUseAdvancedEffects );
  rc = QgsLayoutUtils::createRenderContextForLayout( &l, nullptr );
  QVERIFY( ( rc.flags() & QgsRenderContext::Antialiasing ) );
  QVERIFY( ( rc.flags() & QgsRenderContext::UseAdvancedEffects ) );
  QVERIFY( ( rc.flags() & QgsRenderContext::ForceVectorOutput ) );
#endif
  p.end();
}


void TestQgsLayoutUtils::relativePosition()
{
  //+ve gradient
  QGSCOMPARENEAR( QgsLayoutUtils::relativePosition( 1, 0, 2, 0, 4 ), 2, 0.001 );
  QGSCOMPARENEAR( QgsLayoutUtils::relativePosition( 0, 0, 2, 0, 4 ), 0, 0.001 );
  QGSCOMPARENEAR( QgsLayoutUtils::relativePosition( 2, 0, 2, 0, 4 ), 4, 0.001 );
  QGSCOMPARENEAR( QgsLayoutUtils::relativePosition( 4, 0, 2, 0, 4 ), 8, 0.001 );
  QGSCOMPARENEAR( QgsLayoutUtils::relativePosition( -2, 0, 2, 0, 4 ), -4, 0.001 );
  //-ve gradient
  QGSCOMPARENEAR( QgsLayoutUtils::relativePosition( 1, 0, 2, 4, 0 ), 2, 0.001 );
  QGSCOMPARENEAR( QgsLayoutUtils::relativePosition( 0, 0, 2, 4, 0 ), 4, 0.001 );
  QGSCOMPARENEAR( QgsLayoutUtils::relativePosition( 2, 0, 2, 4, 0 ), 0, 0.001 );
  QGSCOMPARENEAR( QgsLayoutUtils::relativePosition( 4, 0, 2, 4, 0 ), -4, 0.001 );
  QGSCOMPARENEAR( QgsLayoutUtils::relativePosition( -2, 0, 2, 4, 0 ), 8, 0.001 );
  //-ve domain
  QGSCOMPARENEAR( QgsLayoutUtils::relativePosition( 1, 2, 0, 0, 4 ), 2, 0.001 );
  QGSCOMPARENEAR( QgsLayoutUtils::relativePosition( 0, 2, 0, 0, 4 ), 4, 0.001 );
  QGSCOMPARENEAR( QgsLayoutUtils::relativePosition( 2, 2, 0, 0, 4 ), 0, 0.001 );
  QGSCOMPARENEAR( QgsLayoutUtils::relativePosition( 4, 2, 0, 0, 4 ), -4, 0.001 );
  QGSCOMPARENEAR( QgsLayoutUtils::relativePosition( -2, 2, 0, 0, 4 ), 8, 0.001 );
  //-ve domain and gradient
  QGSCOMPARENEAR( QgsLayoutUtils::relativePosition( 1, 2, 0, 4, 0 ), 2, 0.001 );
  QGSCOMPARENEAR( QgsLayoutUtils::relativePosition( 0, 2, 0, 4, 0 ), 0, 0.001 );
  QGSCOMPARENEAR( QgsLayoutUtils::relativePosition( 2, 2, 0, 4, 0 ), 4, 0.001 );
  QGSCOMPARENEAR( QgsLayoutUtils::relativePosition( 4, 2, 0, 4, 0 ), 8, 0.001 );
  QGSCOMPARENEAR( QgsLayoutUtils::relativePosition( -2, 2, 0, 4, 0 ), -4, 0.001 );
}

void TestQgsLayoutUtils::relativeResizeRect()
{
  //test rectangle which fills bounds
  QRectF testRect = QRectF( 0, 0, 1, 1 );
  QRectF boundsBefore = QRectF( 0, 0, 1, 1 );
  QRectF boundsAfter = QRectF( 0, 0, 1, 1 );
  QgsLayoutUtils::relativeResizeRect( testRect, boundsBefore, boundsAfter );
  QCOMPARE( testRect, QRectF( 0, 0, 1, 1 ) );
  testRect = QRectF( 0, 0, 1, 1 );
  boundsAfter = QRectF( 0, 0, 2, 2 );
  QgsLayoutUtils::relativeResizeRect( testRect, boundsBefore, boundsAfter );
  QCOMPARE( testRect, QRectF( 0, 0, 2, 2 ) );
  testRect = QRectF( 0, 0, 1, 1 );
  boundsAfter = QRectF( 0, 0, 0.5, 4 );
  QgsLayoutUtils::relativeResizeRect( testRect, boundsBefore, boundsAfter );
  QCOMPARE( testRect, QRectF( 0, 0, 0.5, 4 ) );

  //test rectangle which doesn't fill bounds
  testRect = QRectF( 1, 2, 1, 2 );
  boundsBefore = QRectF( 0, 0, 4, 8 );
  boundsAfter = QRectF( 0, 0, 2, 4 );
  QgsLayoutUtils::relativeResizeRect( testRect, boundsBefore, boundsAfter );
  QCOMPARE( testRect, QRectF( 0.5, 1, 0.5, 1 ) );
}

QGSTEST_MAIN( TestQgsLayoutUtils )
#include "testqgslayoututils.moc"
