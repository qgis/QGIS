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
#include "qgstest.h"
#include <QObject>
#include <QString>
#include <QStringList>
#include <QSettings>

#include "qgsgraduatedsymbolrenderer.h"

/** \ingroup UnitTests
 * This is a unit test for the qgsGraduatedSymbolRenderer class.
 */

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
    void _makeBreaksSymmetric(QList<double> &breaks, double symmetryPoint, bool astride);


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
  QgsGraduatedSymbolRenderer renderer;
  //test with no ranges
  QVERIFY( !renderer.rangesOverlap() );

  //test with inverted range
  QgsRendererRange inverted;
  inverted.setLowerValue( 3.1 );
  inverted.setUpperValue( 1.2 );
  renderer.addClass( inverted );
  QVERIFY( renderer.rangesOverlap() );
  renderer.deleteAllClasses();

  //test non-overlapping ranges
  QgsRendererRange range1;
  range1.setLowerValue( 1.1 );
  range1.setUpperValue( 3.2 );
  QgsRendererRange range2;
  range2.setLowerValue( 6.4 );
  range2.setUpperValue( 7.2 );
  QgsRendererRange range3;
  range3.setLowerValue( 3.2 );
  range3.setUpperValue( 6.4 );

  renderer.addClass( range1 );
  renderer.addClass( range2 );
  renderer.addClass( range3 );

  QVERIFY( !renderer.rangesOverlap() );

  //add overlapping class
  QgsRendererRange range4;
  range4.setLowerValue( 7.0 );
  range4.setUpperValue( 8.4 );
  renderer.addClass( range4 );

  QVERIFY( renderer.rangesOverlap() );
}

void TestQgsGraduatedSymbolRenderer::rangesHaveGaps()
{
  QgsGraduatedSymbolRenderer renderer;
  //test with no ranges
  QVERIFY( !renderer.rangesHaveGaps() );

  //test with inverted range
  QgsRendererRange inverted;
  inverted.setLowerValue( 3.1 );
  inverted.setUpperValue( 1.2 );
  renderer.addClass( inverted );
  QVERIFY( !renderer.rangesHaveGaps() );
  renderer.deleteAllClasses();

  //test ranges without gaps ranges
  QgsRendererRange range1;
  range1.setLowerValue( 1.1 );
  range1.setUpperValue( 3.2 );
  QgsRendererRange range2;
  range2.setLowerValue( 6.4 );
  range2.setUpperValue( 7.2 );
  QgsRendererRange range3;
  range3.setLowerValue( 3.2 );
  range3.setUpperValue( 6.4 );

  renderer.addClass( range1 );
  renderer.addClass( range2 );
  renderer.addClass( range3 );

  QVERIFY( !renderer.rangesHaveGaps() );

  //add gaps in ranges
  QgsRendererRange range4;
  range4.setLowerValue( 8.0 );
  range4.setUpperValue( 8.4 );
  renderer.addClass( range4 );

  QVERIFY( renderer.rangesHaveGaps() );
}

void TestQgsGraduatedSymbolRenderer::_makeBreaksSymmetric(QList<double> &breaks, double symmetryPoint, bool astride)
{
  const QList<double> unchanged_breaks = {1235, 1023, 997, 800, 555, 10, 1, -5, -11, -423, -811};
  
  // with astride = false
  breaks = unchanged_breaks;
  symmetryPoint = 12.0;
  astride = false;
  _makeBreaksSymmetric( breaks, symmetryPoint, astride );
  
  QVERIFY( breaks.contains( symmetryPoint) );
  // /!\ breaks contain the maximum of the distrib but not the minimum ?
  QVERIFY( breaks.count() % 2 == 0 ); 
  
  // with astride = true
  breaks = unchanged_breaks;
  symmetryPoint = 666.3;
  astride = true;  
  _makeBreaksSymmetric( breaks, symmetryPoint, astride );
  
  QVERIFY( breaks.contains( symmetryPoint) );
  QVERIFY( breaks.count() % 2 != 0 );   
}

QGSTEST_MAIN( TestQgsGraduatedSymbolRenderer )
#include "testqgsgraduatedsymbolrenderer.moc"
