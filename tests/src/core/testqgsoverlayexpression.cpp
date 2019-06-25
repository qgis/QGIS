/***************************************************************************
     test_qgsoverlayexpression.cpp
     --------------------------------------
    Date                 : 20.6.2019
    Copyright            : (C) 2019 by Matthias Kuhn
    Email                : matthias@opengis.ch
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
#include <QtConcurrentMap>

#include <qgsapplication.h>
//header for class being tested
#include "qgsexpression.h"
#include "qgsfeature.h"
#include "qgsfeatureiterator.h"
#include "qgsfeaturerequest.h"
#include "qgsgeometry.h"
#include "qgsrenderchecker.h"
#include "qgsexpressioncontext.h"
#include "qgsrelationmanager.h"
#include "qgsvectorlayer.h"
#include "qgsvectordataprovider.h"
#include "qgsdistancearea.h"
#include "qgsrasterlayer.h"
#include "qgsproject.h"
#include "qgsexpressionnodeimpl.h"
#include "qgsvectorlayerutils.h"
#include "qgsexpressioncontextutils.h"

class TestQgsOverlayExpression: public QObject
{
    Q_OBJECT

  public:

    TestQgsOverlayExpression() = default;

  private:
    QgsVectorLayer *mRectanglesLayer = nullptr;

  private slots:

    void initTestCase();

    void cleanupTestCase();

    void testOverlay();
    void testOverlay_data();

    void testOverlayExpression();
    void testOverlayExpression_data();
};



void TestQgsOverlayExpression::initTestCase()
{
  QgsApplication::init();
  QgsApplication::initQgis();

  QString testDataDir = QStringLiteral( TEST_DATA_DIR ) + '/';

  QString rectanglesFileName = testDataDir + QStringLiteral( "rectangles.shp" );
  QFileInfo rectanglesFileInfo( rectanglesFileName );
  mRectanglesLayer = new QgsVectorLayer( rectanglesFileInfo.filePath(),
                                         QStringLiteral( "rectangles" ), QStringLiteral( "ogr" ) );
  QgsProject::instance()->addMapLayer( mRectanglesLayer );
}

void TestQgsOverlayExpression::cleanupTestCase()
{
  QgsApplication::exitQgis();
}

void TestQgsOverlayExpression::testOverlay()
{
  QFETCH( QString, expression );
  QFETCH( QString, geometry );
  QFETCH( bool, expectedResult );

  QgsExpressionContext context;
  context.appendScope( QgsExpressionContextUtils::projectScope( QgsProject::instance() ) );

  QgsFeature feat;
  feat.setGeometry( QgsGeometry::fromWkt( geometry ) );
  context.setFeature( feat );

  QgsExpression exp( expression );
  QVERIFY2( exp.prepare( &context ), exp.parserErrorString().toUtf8().constData() );
  const QVariant result = exp.evaluate( &context );

  QCOMPARE( result.toBool(), expectedResult );
}

void TestQgsOverlayExpression::testOverlay_data()
{
  //test passing named parameters to functions
  QTest::addColumn<QString>( "expression" );
  QTest::addColumn<QString>( "geometry" );
  QTest::addColumn<bool>( "expectedResult" );

  QTest::newRow( "intersect" ) << "test_geometry_overlay_intersects('rectangles')" << "POLYGON((-120 30, -105 30, -105 20, -120 20, -120 30))" << true;
  QTest::newRow( "intersect no match" ) << "test_geometry_overlay_intersects('rectangles')" << "POLYGON((10 0, 5 0, 5 5, 10 5, 10 0))" << false;
}


void TestQgsOverlayExpression::testOverlayExpression()
{
  QFETCH( QString, expression );
  QFETCH( QString, geometry );
  QFETCH( QVariantList, expectedResult );

  QgsExpressionContext context;
  context.appendScope( QgsExpressionContextUtils::projectScope( QgsProject::instance() ) );

  QgsFeature feat;
  feat.setGeometry( QgsGeometry::fromWkt( geometry ) );
  context.setFeature( feat );

  QgsExpression exp( expression );
  QVERIFY2( exp.prepare( &context ), exp.parserErrorString().toUtf8().constData() );
  const QVariantList result = exp.evaluate( &context ).value<QVariantList>();

  QCOMPARE( result.count(), expectedResult.count() );

  for ( int i = 0; i < expectedResult.count(); i++ )
  {
    QCOMPARE( result.at( i ).value<QgsGeometry>().asWkt( 2 ), expectedResult.at( i ).value<QgsGeometry>().asWkt( 2 ) );
  }
}

void TestQgsOverlayExpression::testOverlayExpression_data()
{
  //test passing named parameters to functions
  QTest::addColumn<QString>( "expression" );
  QTest::addColumn<QString>( "geometry" );
  QTest::addColumn<QVariantList>( "expectedResult" );

  QTest::newRow( "intersect" ) << "geometry_overlay_intersects('rectangles', $geometry)" << "POLYGON((-120 30, -105 30, -105 20, -120 20, -120 30))" << QVariantList { QgsGeometry::fromWkt( "MultiPolygon (((-130 40, -115 40, -115 25, -130 25, -130 40)))" ) };
}
QGSTEST_MAIN( TestQgsOverlayExpression )

#include "testqgsoverlayexpression.moc"
