/***************************************************************************
    testqgsfieldpropertycollectortask.cpp
     ----------------------
    Date                 : November 2026
    Copyright            : (C) 2026 Nyall Dawson
    Email                : nyall.dawson@gmail.com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsfieldpropertycollectortask.h"
#include "qgstest.h"
#include "qgsvectorlayer.h"
#include "qgsvectorlayerfeatureiterator.h"

#include <QAction>
#include <QApplication>
#include <QMainWindow>
#include <QSignalSpy>
#include <QString>

using namespace Qt::StringLiterals;

class TestQgsFieldPropertyCollectorTask : public QObject
{
    Q_OBJECT
  private slots:
    void initTestCase();    // will be called before the first testfunction is executed.
    void cleanupTestCase(); // will be called after the last testfunction was executed.
    void init();            // will be called before each testfunction is executed.
    void cleanup();         // will be called after every testfunction.

    void testFieldPropertyCollectorTask();

  private:
};

void TestQgsFieldPropertyCollectorTask::initTestCase()
{}

void TestQgsFieldPropertyCollectorTask::cleanupTestCase()
{}

void TestQgsFieldPropertyCollectorTask::init()
{}

void TestQgsFieldPropertyCollectorTask::cleanup()
{}

void TestQgsFieldPropertyCollectorTask::testFieldPropertyCollectorTask()
{
  auto vl = std::make_unique< QgsVectorLayer >( u"LineString?crs=epsg:3111&field=pk:int&field=name:string(0)&field=description:string(0)"_s, u"vl1"_s, u"memory"_s );
  QVERIFY( vl->isValid() );

  QgsFeatureList features;
  features.reserve( 1000 );

  const QgsFields fields = vl->fields();
  for ( int i = 0; i < 1000; ++i )
  {
    QgsFeature f( fields );
    f.setGeometry( QgsGeometry::fromPolylineXY( { QgsPointXY( i, i ), QgsPointXY( i + 1, i + 1 ) } ) );

    // some non-random but varying attribute lengths:
    const int nameLen = ( i * 7 ) % 45 + 1;
    const int descLen = ( i * 37 + 13 ) % 250 + 5;
    f.setAttributes( QgsAttributes() << i << u"ab"_s.repeated( nameLen ) << u"cd"_s.repeated( descLen ) );
    features.append( f );
  }

  QVERIFY( vl->dataProvider()->addFeatures( features ) );
  QCOMPARE( vl->featureCount(), 1000LL );

  QgsFieldPropertyCollectorTask task( vl.get() );
  vl.reset();

  QVERIFY( task.run() );

  QCOMPARE( task.maximumStringFieldLength( 0 ), 0 );
  QCOMPARE( task.maximumStringFieldLength( 1 ), 90 );
  QCOMPARE( task.maximumStringFieldLength( 2 ), 508 );
}

QGSTEST_MAIN( TestQgsFieldPropertyCollectorTask )
#include "testqgsfieldpropertycollectortask.moc"
