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
#include "qgsclassificationequalinterval.h"
#include "qgssymbollayerutils.h"
#include "qgsvectorlayer.h"
#include "qgsclassificationquantile.h"
#include "qgsmarkersymbol.h"

/**
 * \ingroup UnitTests
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
    void classifySymmetric();
    void testMatchingRangeForValue();

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

// this function is used only on breaks that already contain the symmetryPoint
// calcEqualIntervalBreaks takes symmetryPoint as parameter
void TestQgsGraduatedSymbolRenderer::classifySymmetric()
{
  // minimum < symmetryPointForEqualInterval < maximum
  // going below 1E-6 will result in a fail because C++ think 2.6e-06 - 2e-06 = 0
  QList<double> minimum =                       {15.30, 20,   20,     1111, 0.26, 0.000026, -1.56E10};
  QList<double> symmetryPointForEqualInterval = {122.6, 24.3, 26.3, 1563.3, 0.34, 0.000034, 0.56E10};
  QList<double> maximum =                       {253.6, 30,   30,     2222, 0.55, 0.000055, 1.25E10};

  int newPosOfSymmetryPoint = 0;
  bool astride = false;
  double symmetryPoint = 0;
  bool useSymmetricMode = true;
  QList<double> breaks = {};

  for ( int valTest = 0; valTest < minimum.size(); valTest++ )
  {
    //makes no sense with less than 3 classes
    for ( int nclasses = 3; nclasses < 30; nclasses++ )
    {
      // PRETTY BREAKS
      const QList<double> unchanged_breaks = QgsSymbolLayerUtils::prettyBreaks( minimum[valTest], maximum[valTest], nclasses );

      // user can only choose a symmetryPoint which is part of the pretty breaks (this part is not tested here)
      // makes no sense to take the extreme breaks as symmetry point
      for ( int posOfSymmetryPoint = 1; posOfSymmetryPoint < unchanged_breaks.count() - 2; posOfSymmetryPoint++ )
      {
        symmetryPoint = unchanged_breaks[posOfSymmetryPoint];

        // with astride = false
        astride = false;
        breaks = unchanged_breaks;
        QgsClassificationMethod::makeBreaksSymmetric( breaks, symmetryPoint, astride );
        QCOMPARE( breaks.count() % 2, 0 );
        // because the minimum is not in the breaks
        const int newPosOfSymmetryPoint = breaks.count() / 2;
        QCOMPARE( breaks[ newPosOfSymmetryPoint - 1 ], symmetryPoint );

        // with astride = true
        astride = true;
        breaks = unchanged_breaks;
        QgsClassificationMethod::makeBreaksSymmetric( breaks, symmetryPoint, astride );
        QCOMPARE( breaks.count() % 2, 1 );
        QVERIFY( !breaks.contains( symmetryPoint ) );
      }

      // EQUAL INTERVALS
      useSymmetricMode = true;

      // with astride = false
      astride = false;
      QgsClassificationEqualInterval method;
      method.setSymmetricMode( useSymmetricMode, symmetryPointForEqualInterval[valTest], astride );
      QList<QgsClassificationRange> ranges = method.classes( minimum[valTest], maximum[valTest], nclasses );
      breaks = QgsClassificationMethod::rangesToBreaks( ranges );
      QCOMPARE( breaks.count() % 2, 0 );
      // because the minimum is not in the breaks
      newPosOfSymmetryPoint = breaks.count() / 2 ;
      QCOMPARE( breaks[ newPosOfSymmetryPoint - 1 ], symmetryPointForEqualInterval[valTest] );

      // with astride = true
      astride = true;
      method.setSymmetricMode( useSymmetricMode, symmetryPointForEqualInterval[valTest], astride );
      ranges = method.classes( minimum[valTest], maximum[valTest], nclasses );
      breaks = QgsClassificationMethod::rangesToBreaks( ranges );
      QCOMPARE( breaks.count() % 2, 1 );
      QVERIFY( !breaks.contains( symmetryPointForEqualInterval[valTest] ) );
    }
  }
}

void TestQgsGraduatedSymbolRenderer::testMatchingRangeForValue()
{
  QgsGraduatedSymbolRenderer renderer;
  //test with no ranges
  QVERIFY( !renderer.rangeForValue( 1 ) );
  QVERIFY( !renderer.rangeForValue( 12 ) );
  QVERIFY( !renderer.rangeForValue( -1 ) );
  QVERIFY( !renderer.symbolForValue( 1 ) );
  QVERIFY( renderer.legendKeyForValue( 1 ).isEmpty() );

  QgsMarkerSymbol ms;
  ms.setColor( QColor( 255, 0, 0 ) );
  const QgsRendererRange r1( 1.1, 3.2, ms.clone(), QStringLiteral( "r1" ) );
  renderer.addClass( r1 );

  QVERIFY( !renderer.rangeForValue( 1 ) );
  QVERIFY( !renderer.rangeForValue( 12 ) );
  QVERIFY( !renderer.rangeForValue( -1 ) );
  QCOMPARE( renderer.rangeForValue( 1.1 )->label(), QStringLiteral( "r1" ) );
  QCOMPARE( renderer.rangeForValue( 2.1 )->label(), QStringLiteral( "r1" ) );
  QCOMPARE( renderer.rangeForValue( 3.2 )->label(), QStringLiteral( "r1" ) );
  QVERIFY( !renderer.symbolForValue( 1 ) );
  QCOMPARE( renderer.symbolForValue( 2.1 )->color().name(), QStringLiteral( "#ff0000" ) );
  QVERIFY( renderer.legendKeyForValue( 1 ).isEmpty() );
  QCOMPARE( renderer.legendKeyForValue( 2.1 ), QStringLiteral( "0" ) );

  ms.setColor( QColor( 255, 255, 0 ) );
  const QgsRendererRange r2( 3.2, 3.3, ms.clone(), QStringLiteral( "r2" ) );
  renderer.addClass( r2 );

  QVERIFY( !renderer.rangeForValue( 1 ) );
  QVERIFY( !renderer.rangeForValue( 12 ) );
  QVERIFY( !renderer.rangeForValue( -1 ) );
  QCOMPARE( renderer.rangeForValue( 1.1 )->label(), QStringLiteral( "r1" ) );
  QCOMPARE( renderer.rangeForValue( 2.1 )->label(), QStringLiteral( "r1" ) );
  QCOMPARE( renderer.rangeForValue( 3.2 )->label(), QStringLiteral( "r1" ) );
  QCOMPARE( renderer.rangeForValue( 3.25 )->label(), QStringLiteral( "r2" ) );
  QCOMPARE( renderer.rangeForValue( 3.3 )->label(), QStringLiteral( "r2" ) );
  QVERIFY( !renderer.symbolForValue( 1 ) );
  QCOMPARE( renderer.symbolForValue( 2.1 )->color().name(), QStringLiteral( "#ff0000" ) );
  QCOMPARE( renderer.symbolForValue( 3.25 )->color().name(), QStringLiteral( "#ffff00" ) );
  QVERIFY( renderer.legendKeyForValue( 1 ).isEmpty() );
  QCOMPARE( renderer.legendKeyForValue( 2.1 ), QStringLiteral( "0" ) );
  QCOMPARE( renderer.legendKeyForValue( 3.25 ), QStringLiteral( "1" ) );

  // disabled range
  ms.setColor( QColor( 255, 0, 255 ) );
  const QgsRendererRange r3( 3.3, 3.6, ms.clone(), QStringLiteral( "r3" ), false );
  renderer.addClass( r3 );
  QVERIFY( !renderer.rangeForValue( 1 ) );
  QVERIFY( !renderer.rangeForValue( 12 ) );
  QVERIFY( !renderer.rangeForValue( -1 ) );
  QCOMPARE( renderer.rangeForValue( 1.1 )->label(), QStringLiteral( "r1" ) );
  QCOMPARE( renderer.rangeForValue( 2.1 )->label(), QStringLiteral( "r1" ) );
  QCOMPARE( renderer.rangeForValue( 3.2 )->label(), QStringLiteral( "r1" ) );
  QCOMPARE( renderer.rangeForValue( 3.25 )->label(), QStringLiteral( "r2" ) );
  QCOMPARE( renderer.rangeForValue( 3.3 )->label(), QStringLiteral( "r2" ) );
  QVERIFY( !renderer.rangeForValue( 3.5 ) );
  QVERIFY( !renderer.symbolForValue( 1 ) );
  QCOMPARE( renderer.symbolForValue( 2.1 )->color().name(), QStringLiteral( "#ff0000" ) );
  QCOMPARE( renderer.symbolForValue( 3.25 )->color().name(), QStringLiteral( "#ffff00" ) );
  QVERIFY( !renderer.symbolForValue( 3.5 ) );
  QVERIFY( renderer.legendKeyForValue( 1 ).isEmpty() );
  QCOMPARE( renderer.legendKeyForValue( 2.1 ), QStringLiteral( "0" ) );
  QCOMPARE( renderer.legendKeyForValue( 3.25 ), QStringLiteral( "1" ) );
  QVERIFY( renderer.legendKeyForValue( 3.5 ).isEmpty() );

  // zero width range
  ms.setColor( QColor( 0, 255, 255 ) );
  const QgsRendererRange r4( 3.7, 3.7, ms.clone(), QStringLiteral( "r4" ) );
  renderer.addClass( r4 );
  QVERIFY( !renderer.rangeForValue( 1 ) );
  QVERIFY( !renderer.rangeForValue( 12 ) );
  QVERIFY( !renderer.rangeForValue( -1 ) );
  QCOMPARE( renderer.rangeForValue( 1.1 )->label(), QStringLiteral( "r1" ) );
  QCOMPARE( renderer.rangeForValue( 2.1 )->label(), QStringLiteral( "r1" ) );
  QCOMPARE( renderer.rangeForValue( 3.2 )->label(), QStringLiteral( "r1" ) );
  QCOMPARE( renderer.rangeForValue( 3.25 )->label(), QStringLiteral( "r2" ) );
  QCOMPARE( renderer.rangeForValue( 3.3 )->label(), QStringLiteral( "r2" ) );
  QCOMPARE( renderer.rangeForValue( 3.7 )->label(), QStringLiteral( "r4" ) );
  QVERIFY( !renderer.rangeForValue( 3.5 ) );
  QVERIFY( !renderer.symbolForValue( 1 ) );
  QCOMPARE( renderer.symbolForValue( 2.1 )->color().name(), QStringLiteral( "#ff0000" ) );
  QCOMPARE( renderer.symbolForValue( 3.25 )->color().name(), QStringLiteral( "#ffff00" ) );
  QCOMPARE( renderer.symbolForValue( 3.7 )->color().name(), QStringLiteral( "#00ffff" ) );
  QVERIFY( !renderer.symbolForValue( 3.5 ) );
  QVERIFY( renderer.legendKeyForValue( 1 ).isEmpty() );
  QCOMPARE( renderer.legendKeyForValue( 2.1 ), QStringLiteral( "0" ) );
  QCOMPARE( renderer.legendKeyForValue( 3.25 ), QStringLiteral( "1" ) );
  QCOMPARE( renderer.legendKeyForValue( 3.7 ), QStringLiteral( "3" ) );
  QVERIFY( renderer.legendKeyForValue( 3.5 ).isEmpty() );

  // test values which fall just outside ranges, e.g. due to double precision (refs https://github.com/qgis/QGIS/issues/27420)
  QCOMPARE( renderer.rangeForValue( 1.1 - std::numeric_limits<double>::epsilon() * 2 )->label(), QStringLiteral( "r1" ) );
  QCOMPARE( renderer.rangeForValue( 3.7 + std::numeric_limits<double>::epsilon() * 2 )->label(), QStringLiteral( "r4" ) );
}

QGSTEST_MAIN( TestQgsGraduatedSymbolRenderer )
#include "testqgsgraduatedsymbolrenderer.moc"
