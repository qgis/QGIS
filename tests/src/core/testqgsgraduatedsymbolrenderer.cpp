/***************************************************************************
     testqgsgraduatedsymbolrenderer.cpp
     ----------------------------------
    Date                 : May 2015
    Copyright            : (C) 2015 Nyall Dawson
    Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include <QtTest/QtTest>
#include <QObject>
#include <QString>
#include <QStringList>
#include <QSettings>
#include <QSharedPointer>

#include "qgsgraduatedsymbolrendererv2.h"

class TestQgsGraduatedSymbolRenderer: public QObject
{
    Q_OBJECT

  private slots:
    void initTestCase();// will be called before the first testfunction is executed.
    void cleanupTestCase();// will be called after the last testfunction was executed.
    void init();// will be called before each testfunction is executed.
    void cleanup();// will be called after every testfunction.
    void rangesOverlap();
    void rangesHaveGaps();


  private:
};

void TestQgsGraduatedSymbolRenderer::initTestCase()
{

}

void TestQgsGraduatedSymbolRenderer::cleanupTestCase()
{

}

void TestQgsGraduatedSymbolRenderer::init()
{

}

void TestQgsGraduatedSymbolRenderer::cleanup()
{

}

void TestQgsGraduatedSymbolRenderer::rangesOverlap()
{
  QgsGraduatedSymbolRendererV2 renderer;
  //test with no ranges
  QVERIFY( !renderer.rangesOverlap() );

  //test with inverted range
  QgsRendererRangeV2 inverted;
  inverted.setLowerValue( 3.1 );
  inverted.setUpperValue( 1.2 );
  renderer.addClass( inverted );
  QVERIFY( renderer.rangesOverlap() );
  renderer.deleteAllClasses();

  //test non-overlapping ranges
  QgsRendererRangeV2 range1;
  range1.setLowerValue( 1.1 );
  range1.setUpperValue( 3.2 );
  QgsRendererRangeV2 range2;
  range2.setLowerValue( 6.4 );
  range2.setUpperValue( 7.2 );
  QgsRendererRangeV2 range3;
  range3.setLowerValue( 3.2 );
  range3.setUpperValue( 6.4 );

  renderer.addClass( range1 );
  renderer.addClass( range2 );
  renderer.addClass( range3 );

  QVERIFY( !renderer.rangesOverlap() );

  //add overlapping class
  QgsRendererRangeV2 range4;
  range4.setLowerValue( 7.0 );
  range4.setUpperValue( 8.4 );
  renderer.addClass( range4 );

  QVERIFY( renderer.rangesOverlap() );
}

void TestQgsGraduatedSymbolRenderer::rangesHaveGaps()
{
  QgsGraduatedSymbolRendererV2 renderer;
  //test with no ranges
  QVERIFY( !renderer.rangesHaveGaps() );

  //test with inverted range
  QgsRendererRangeV2 inverted;
  inverted.setLowerValue( 3.1 );
  inverted.setUpperValue( 1.2 );
  renderer.addClass( inverted );
  QVERIFY( !renderer.rangesHaveGaps() );
  renderer.deleteAllClasses();

  //test ranges without gaps ranges
  QgsRendererRangeV2 range1;
  range1.setLowerValue( 1.1 );
  range1.setUpperValue( 3.2 );
  QgsRendererRangeV2 range2;
  range2.setLowerValue( 6.4 );
  range2.setUpperValue( 7.2 );
  QgsRendererRangeV2 range3;
  range3.setLowerValue( 3.2 );
  range3.setUpperValue( 6.4 );

  renderer.addClass( range1 );
  renderer.addClass( range2 );
  renderer.addClass( range3 );

  QVERIFY( !renderer.rangesHaveGaps() );

  //add gaps in ranges
  QgsRendererRangeV2 range4;
  range4.setLowerValue( 8.0 );
  range4.setUpperValue( 8.4 );
  renderer.addClass( range4 );

  QVERIFY( renderer.rangesHaveGaps() );
}

QTEST_MAIN( TestQgsGraduatedSymbolRenderer )
#include "testqgsgraduatedsymbolrenderer.moc"
