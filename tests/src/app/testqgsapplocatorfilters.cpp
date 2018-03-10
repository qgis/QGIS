/***************************************************************************
     testqgsapplocatorfilters.cpp
     --------------------------
    Date                 : 2018-02-24
    Copyright            : (C) 2018 by Nyall Dawson
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
#include "qgisapp.h"
#include "qgslocatorfilter.h"
#include "qgslocator.h"
#include "qgsmapcanvas.h"
#include "locator/qgsinbuiltlocatorfilters.h"
#include <QSignalSpy>
#include <QClipboard>

/**
 * \ingroup UnitTests
 * This is a unit test for the field calculator
 */
class TestQgsAppLocatorFilters : public QObject
{
    Q_OBJECT

  private slots:
    void initTestCase();// will be called before the first testfunction is executed.
    void cleanupTestCase();// will be called after the last testfunction was executed.
    void testCalculator();
    void testGoto();

  private:
    QgisApp *mQgisApp = nullptr;
    QgsMapCanvas *canvas = nullptr;

    QList< QgsLocatorResult > gatherResults( QgsLocatorFilter *filter, const QString &string, const QgsLocatorContext &context );
};

//runs before all tests
void TestQgsAppLocatorFilters::initTestCase()
{
  // init QGIS's paths - true means that all path will be inited from prefix
  QgsApplication::init();
  QgsApplication::initQgis();
  mQgisApp = new QgisApp();
  canvas  = mQgisApp->mapCanvas();
  canvas->setDestinationCrs( QgsCoordinateReferenceSystem( 4326 ) );
}

//runs after all tests
void TestQgsAppLocatorFilters::cleanupTestCase()
{
  QgsApplication::exitQgis();
}

void TestQgsAppLocatorFilters::testCalculator()
{
  QgsExpressionCalculatorLocatorFilter filter;

  // valid expression
  QList< QgsLocatorResult > results = gatherResults( &filter, QStringLiteral( "1+2" ), QgsLocatorContext() );
  QCOMPARE( results.count(), 1 );
  QCOMPARE( results.at( 0 ).userData.toInt(), 3 );

  // trigger result
  filter.triggerResult( results.at( 0 ) );
  QCOMPARE( QApplication::clipboard()->text(), QStringLiteral( "3" ) );

  // invalid expression
  results = gatherResults( &filter, QStringLiteral( "1+" ), QgsLocatorContext() );
  QVERIFY( results.empty() );

}

void TestQgsAppLocatorFilters::testGoto()
{
  QgsGotoLocatorFilter filter;

  // goto X,Y
  QList< QgsLocatorResult > results = gatherResults( &filter, QStringLiteral( "4,5" ), QgsLocatorContext() );
  QCOMPARE( results.count(), 1 );
  QCOMPARE( results.at( 0 ).displayString, QString( "Go to 4:5 (EPSG:4326)" ) );

  // Leaflet and OpenLayers
  results = gatherResults( &filter, QStringLiteral( "https://www.openstreetmap.org/#map=15/44.5546/6.4936" ), QgsLocatorContext() );
  QCOMPARE( results.count(), 1 );
  QCOMPARE( results.at( 0 ).displayString, QString( "Go to 6.494,44.555 scale 22569" ) );

}

QList<QgsLocatorResult> TestQgsAppLocatorFilters::gatherResults( QgsLocatorFilter *filter, const QString &string, const QgsLocatorContext &context )
{
  QSignalSpy spy( filter, &QgsLocatorFilter::resultFetched );
  QgsFeedback f;
  filter->fetchResults( string, context, &f );

  QList< QgsLocatorResult > results;
  for ( int i = 0; i < spy.count(); ++ i )
  {
    QVariant v = spy.at( i ).at( i );
    QgsLocatorResult result = v.value<QgsLocatorResult>();
    results.append( result );
  }
  return results;
}

QGSTEST_MAIN( TestQgsAppLocatorFilters )
#include "testqgsapplocatorfilters.moc"
