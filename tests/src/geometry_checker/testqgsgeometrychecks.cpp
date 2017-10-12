/***************************************************************************
     testqgsgeometrychecks.cpp
     --------------------------------------
    Date                 : September 2017
    Copyright            : (C) 2017 Sandro Mani
    Email                : manisandro at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgstest.h"
#include "qgscrscache.h"
#include "qgsfeature.h"
#include "qgsfeaturepool.h"
#include "qgsvectorlayer.h"

#include "qgsgeometryanglecheck.h"
#include "qgsgeometryareacheck.h"
#include "qgsgeometrycontainedcheck.h"
#include "qgsgeometrydanglecheck.h"
#include "qgsgeometrydegeneratepolygoncheck.h"
#include "qgsgeometryduplicatecheck.h"
#include "qgsgeometryduplicatenodescheck.h"
#include "qgsgeometrygapcheck.h"
#include "qgsgeometryholecheck.h"
#include "qgsgeometrylineintersectioncheck.h"
#include "qgsgeometrylinelayerintersectioncheck.h"
#include "qgsgeometrymultipartcheck.h"
#include "qgsgeometryoverlapcheck.h"
#include "qgsgeometrypointcoveredbylinecheck.h"
#include "qgsgeometrypointinpolygoncheck.h"
#include "qgsgeometrysegmentlengthcheck.h"
#include "qgsgeometryselfcontactcheck.h"
#include "qgsgeometryselfintersectioncheck.h"
#include "qgsgeometrysliverpolygoncheck.h"

#include "qgsgeometrytypecheck.h"


class TestQgsGeometryChecks: public QObject
{
    Q_OBJECT
  private:
    double layerToMapUnits( const QgsMapLayer *layer, const QString &mapCrs ) const;
    QgsFeaturePool *createFeaturePool( QgsVectorLayer *layer, const QString &mapCrs, bool selectedOnly = false ) const;
    QgsGeometryCheckerContext *createTestContext( QMap<QString, QString> &layers, const QString &mapCrs = "EPSG:4326", double prec = 8 ) const;
    void cleanupTestContext( QgsGeometryCheckerContext *ctx ) const;
    void listErrors( const QList<QgsGeometryCheckError *> &checkErrors, const QStringList &messages ) const;
    int searchCheckError( const QList<QgsGeometryCheckError *> &checkErrors, const QString &layerId, const QgsFeatureId &featureId = -1, const QgsPointXY &pos = QgsPointXY(), const QgsVertexId &vid = QgsVertexId(), const QVariant &value = QVariant(), double tol = 1E-4 ) const;

    QMap<QString, QString> mLayers;
    QgsGeometryCheckerContext *mContext;

  private slots:
    // will be called before the first testfunction is executed.
    void initTestCase();
    // will be called after the last testfunction is executed.
    void cleanupTestCase();

    void testAngleCheck();
    void testAreaCheck();
    void testContainedCheck();
    void testDangleCheck();
    void testDegeneratePolygonCheck();
    void testDuplicateCheck();
    void testDuplicateNodesCheck();
    void testGapCheck();
    void testHoleCheck();
    void testLineIntersectionCheck();
    void testLineLayerIntersectionCheck();
    void testMultipartCheck();
    void testOverlapCheck();
    void testPointCoveredByLineCheck();
    void testPointInPolygonCheck();
    void testSegmentLengthCheck();
    void testSelfContactCheck();
    void testSelfIntersectionCheck();
    void testSliverPolygonCheck();
};

void TestQgsGeometryChecks::initTestCase()
{
  QgsApplication::initQgis();

  mLayers.insert( "point_layer.shp", "" );
  mLayers.insert( "line_layer.shp", "" );
  mLayers.insert( "polygon_layer.shp", "" );
  mContext = createTestContext( mLayers );
}

void TestQgsGeometryChecks::cleanupTestCase()
{
  cleanupTestContext( mContext );
  QgsApplication::exitQgis();
}

///////////////////////////////////////////////////////////////////////////////

void TestQgsGeometryChecks::testAngleCheck()
{
  QList<QgsGeometryCheckError *> checkErrors;
  QStringList messages;

  QgsGeometryAngleCheck( mContext, 15 ).collectErrors( checkErrors, messages );
  listErrors( checkErrors, messages );

  QCOMPARE( checkErrors.size(), 8 );
  QVERIFY( searchCheckError( checkErrors, mLayers["point_layer.shp"] ) == 0 );
  QVERIFY( searchCheckError( checkErrors, mLayers["line_layer.shp"], 0, QgsPointXY( -0.2225,  0.5526 ), QgsVertexId( 0, 0, 3 ), 10.5865 ) == 1 );
  QVERIFY( searchCheckError( checkErrors, mLayers["line_layer.shp"], 0, QgsPointXY( -0.94996, 0.99967 ), QgsVertexId( 1, 0, 1 ), 8.3161 ) == 1 );
  QVERIFY( searchCheckError( checkErrors, mLayers["line_layer.shp"], 2, QgsPointXY( -0.4547, -0.3059 ), QgsVertexId( 0, 0, 1 ), 5.4165 ) == 1 );
  QVERIFY( searchCheckError( checkErrors, mLayers["line_layer.shp"], 2, QgsPointXY( -0.7594, -0.1971 ), QgsVertexId( 0, 0, 2 ), 12.5288 ) == 1 );
  QVERIFY( searchCheckError( checkErrors, mLayers["polygon_layer.shp"], 0, QgsPointXY( 0.2402, 1.0786 ), QgsVertexId( 0, 0, 1 ), 13.5140 ) == 1 );
  QVERIFY( searchCheckError( checkErrors, mLayers["polygon_layer.shp"], 1, QgsPointXY( 0.6960, 0.5908 ), QgsVertexId( 0, 0, 0 ), 7.0556 ) == 1 );
  QVERIFY( searchCheckError( checkErrors, mLayers["polygon_layer.shp"], 1, QgsPointXY( 0.98690, 0.55699 ), QgsVertexId( 1, 0, 5 ), 7.7351 ) == 1 );
  QVERIFY( searchCheckError( checkErrors, mLayers["polygon_layer.shp"], 11, QgsPointXY( -0.3186, 1.6734 ), QgsVertexId( 0, 0, 1 ), 3.5092 ) == 1 );
}

void TestQgsGeometryChecks::testAreaCheck()
{
  QList<QgsGeometryCheckError *> checkErrors;
  QStringList messages;

  QgsGeometryAreaCheck( mContext, 0.04 ).collectErrors( checkErrors, messages );
  listErrors( checkErrors, messages );

  QCOMPARE( checkErrors.size(), 4 );
  QVERIFY( searchCheckError( checkErrors, mLayers["point_layer.shp"] ) == 0 );
  QVERIFY( searchCheckError( checkErrors, mLayers["line_layer.shp"] ) == 0 );
  QVERIFY( searchCheckError( checkErrors, mLayers["polygon_layer.shp"], 1, QgsPointXY( 1.0068, 0.3635 ), QgsVertexId( 1 ), 0.0105 ) == 1 );
  QVERIFY( searchCheckError( checkErrors, mLayers["polygon_layer.shp"], 2, QgsPointXY( 0.9739, 1.0983 ), QgsVertexId( 0 ), 0.0141 ) == 1 );
  QVERIFY( searchCheckError( checkErrors, mLayers["polygon_layer.shp"], 6, QgsPointXY( 0.9968, 1.7584 ), QgsVertexId( 0 ), 0 ) == 1 );
  QVERIFY( searchCheckError( checkErrors, mLayers["polygon_layer.shp"], 11, QgsPointXY( -0.2941, 1.4614 ), QgsVertexId( 0 ), 0.0031 ) == 1 );
}

void TestQgsGeometryChecks::testContainedCheck()
{
  QList<QgsGeometryCheckError *> checkErrors;
  QStringList messages;

  QgsGeometryContainedCheck( mContext ).collectErrors( checkErrors, messages );
  listErrors( checkErrors, messages );

  QCOMPARE( checkErrors.size(), 4 );
  QVERIFY( searchCheckError( checkErrors, mLayers["point_layer.shp"], 4, QgsPointXY(), QgsVertexId(), QVariant( "polygon_layer.shp:5" ) ) == 1 );
  QVERIFY( searchCheckError( checkErrors, mLayers["point_layer.shp"], 5, QgsPointXY(), QgsVertexId(), QVariant( "polygon_layer.shp:0" ) ) == 1 );
  QVERIFY( searchCheckError( checkErrors, mLayers["line_layer.shp"], 3, QgsPointXY(), QgsVertexId(), QVariant( "polygon_layer.shp:0" ) ) == 1 );
  QVERIFY( searchCheckError( checkErrors, mLayers["polygon_layer.shp"], 3, QgsPointXY(), QgsVertexId(), QVariant( "polygon_layer.shp:0" ) ) == 1 );
  QVERIFY( messages.contains( "Contained check failed for (polygon_layer.shp:1): the geometry is invalid" ) );
}

void TestQgsGeometryChecks::testDangleCheck()
{
  QList<QgsGeometryCheckError *> checkErrors;
  QStringList messages;

  QgsGeometryDangleCheck( mContext ).collectErrors( checkErrors, messages );
  listErrors( checkErrors, messages );

  QCOMPARE( checkErrors.size(), 4 );
  QVERIFY( searchCheckError( checkErrors, mLayers["point_layer.shp"] ) == 0 );
  QVERIFY( searchCheckError( checkErrors, mLayers["polygon_layer.shp"] ) == 0 );
  QVERIFY( searchCheckError( checkErrors, mLayers["line_layer.shp"], 0, QgsPointXY( -0.7558, 0.7648 ), QgsVertexId( 1, 0, 0 ) ) == 1 );
  QVERIFY( searchCheckError( checkErrors, mLayers["line_layer.shp"], 2, QgsPointXY( -0.7787, -0.2237 ), QgsVertexId( 0, 0, 0 ) ) == 1 );
  QVERIFY( searchCheckError( checkErrors, mLayers["line_layer.shp"], 3, QgsPointXY( -0.2326, 0.9537 ), QgsVertexId( 0, 0, 0 ) ) == 1 );
  QVERIFY( searchCheckError( checkErrors, mLayers["line_layer.shp"], 3, QgsPointXY( 0.0715, 0.7830 ), QgsVertexId( 0, 0, 3 ) ) == 1 );
}

void TestQgsGeometryChecks::testDegeneratePolygonCheck()
{
  QList<QgsGeometryCheckError *> checkErrors;
  QStringList messages;

  QgsGeometryDegeneratePolygonCheck( mContext ).collectErrors( checkErrors, messages );
  listErrors( checkErrors, messages );

  QCOMPARE( checkErrors.size(), 1 );
  QVERIFY( searchCheckError( checkErrors, mLayers["point_layer.shp"] ) == 0 );
  QVERIFY( searchCheckError( checkErrors, mLayers["line_layer.shp"] ) == 0 );
  QVERIFY( searchCheckError( checkErrors, mLayers["polygon_layer.shp"], 6, QgsPointXY(), QgsVertexId( 0, 0 ) ) == 1 );
}

void TestQgsGeometryChecks::testDuplicateCheck()
{
  QList<QgsGeometryCheckError *> checkErrors;
  QStringList messages;

  QgsGeometryDuplicateCheck( mContext ).collectErrors( checkErrors, messages );
  listErrors( checkErrors, messages );

  QCOMPARE( checkErrors.size(), 3 );
  QVERIFY(
    searchCheckError( checkErrors, mLayers["point_layer.shp"], 6, QgsPoint(), QgsVertexId(), QVariant( "point_layer.shp:2" ) ) == 1
    || searchCheckError( checkErrors, mLayers["point_layer.shp"], 2, QgsPoint(), QgsVertexId(), QVariant( "point_layer.shp:6" ) ) == 1 );
  QVERIFY(
    searchCheckError( checkErrors, mLayers["line_layer.shp"], 4, QgsPoint(), QgsVertexId(), QVariant( "line_layer.shp:7" ) ) == 1
    || searchCheckError( checkErrors, mLayers["line_layer.shp"], 7, QgsPoint(), QgsVertexId(), QVariant( "line_layer.shp:4" ) ) == 1 );
  QVERIFY(
    searchCheckError( checkErrors, mLayers["polygon_layer.shp"], 8, QgsPoint(), QgsVertexId(), QVariant( "polygon_layer.shp:7" ) ) == 1
    || searchCheckError( checkErrors, mLayers["polygon_layer.shp"], 7, QgsPoint(), QgsVertexId(), QVariant( "polygon_layer.shp:8" ) ) == 1 );
}

void TestQgsGeometryChecks::testDuplicateNodesCheck()
{
  QList<QgsGeometryCheckError *> checkErrors;
  QStringList messages;

  QgsGeometryDuplicateNodesCheck( mContext ).collectErrors( checkErrors, messages );
  listErrors( checkErrors, messages );

  QCOMPARE( checkErrors.size(), 4 );
  QVERIFY( searchCheckError( checkErrors, mLayers["point_layer.shp"] ) == 0 );
  QVERIFY( searchCheckError( checkErrors, mLayers["line_layer.shp"], 0, QgsPointXY( -0.6360, 0.6203 ), QgsVertexId( 0, 0, 5 ) ) == 1 );
  QVERIFY( searchCheckError( checkErrors, mLayers["line_layer.shp"], 6, QgsPointXY( 0.2473, 2.0821 ), QgsVertexId( 0, 0, 1 ) ) == 1 );
  QVERIFY( searchCheckError( checkErrors, mLayers["line_layer.shp"], 6, QgsPointXY( 0.5158, 2.0930 ), QgsVertexId( 0, 0, 3 ) ) == 1 );
  QVERIFY( searchCheckError( checkErrors, mLayers["polygon_layer.shp"], 4, QgsPointXY( 1.6319, 0.5642 ), QgsVertexId( 0, 0, 1 ) ) == 1 );
}

void TestQgsGeometryChecks::testGapCheck()
{
  QMap<QString, QString> layers;
  layers.insert( "gap_layer.shp", "" );
  QgsGeometryCheckerContext *context = createTestContext( layers );

  QList<QgsGeometryCheckError *> checkErrors;
  QStringList messages;

  QgsGeometryGapCheck( context, 0.01 ).collectErrors( checkErrors, messages );
  listErrors( checkErrors, messages );

  QCOMPARE( checkErrors.size(), 5 );
  QVERIFY( searchCheckError( checkErrors, "", -1, QgsPointXY( 0.2924, -0.8798 ), QgsVertexId(), 0.0027 ) == 1 );
  QVERIFY( searchCheckError( checkErrors, "", -1, QgsPointXY( 0.4238, -0.7479 ), QgsVertexId(), 0.0071 ) == 1 );
  QVERIFY( searchCheckError( checkErrors, "", -1, QgsPointXY( 0.0094, -0.4448 ), QgsVertexId(), 0.0033 ) == 1 );
  QVERIFY( searchCheckError( checkErrors, "", -1, QgsPointXY( 0.2939, -0.4694 ), QgsVertexId(), 0.0053 ) == 1 );
  QVERIFY( searchCheckError( checkErrors, "", -1, QgsPointXY( 0.6284, -0.3641 ), QgsVertexId(), 0.0018 ) == 1 );

  cleanupTestContext( context );
}

void TestQgsGeometryChecks::testHoleCheck()
{
  QList<QgsGeometryCheckError *> checkErrors;
  QStringList messages;

  QgsGeometryHoleCheck( mContext ).collectErrors( checkErrors, messages );
  listErrors( checkErrors, messages );

  QCOMPARE( checkErrors.size(), 1 );
  QVERIFY( searchCheckError( checkErrors, mLayers["point_layer.shp"] ) == 0 );
  QVERIFY( searchCheckError( checkErrors, mLayers["line_layer.shp"] ) == 0 );
  QVERIFY( searchCheckError( checkErrors, mLayers["polygon_layer.shp"], 12, QgsPointXY( 0.1772, 0.10476 ), QgsVertexId( 0, 1 ) ) == 1 );
}

void TestQgsGeometryChecks::testLineIntersectionCheck()
{
  QList<QgsGeometryCheckError *> checkErrors;
  QStringList messages;

  QgsGeometryLineIntersectionCheck( mContext ).collectErrors( checkErrors, messages );
  listErrors( checkErrors, messages );

  QCOMPARE( checkErrors.size(), 1 );
  QVERIFY( searchCheckError( checkErrors, mLayers["point_layer.shp"] ) == 0 );
  QVERIFY( searchCheckError( checkErrors, mLayers["polygon_layer.shp"] ) == 0 );
  QVERIFY( searchCheckError( checkErrors, mLayers["line_layer.shp"], 1, QgsPointXY( -0.5594, 0.4098 ), QgsVertexId( 0 ), QVariant( "line_layer.shp:0" ) ) == 1 );
}

void TestQgsGeometryChecks::testLineLayerIntersectionCheck()
{
  QList<QgsGeometryCheckError *> checkErrors;
  QStringList messages;

  QgsGeometryLineLayerIntersectionCheck( mContext, mLayers["polygon_layer.shp"] ).collectErrors( checkErrors, messages );
  listErrors( checkErrors, messages );

  QCOMPARE( checkErrors.size(), 5 );
  QVERIFY( searchCheckError( checkErrors, mLayers["point_layer.shp"] ) == 0 );
  QVERIFY( searchCheckError( checkErrors, mLayers["polygon_layer.shp"] ) == 0 );
  QVERIFY( searchCheckError( checkErrors, mLayers["line_layer.shp"], 3, QgsPointXY( -0.1890, 0.9043 ), QgsVertexId( 0 ), QVariant( "polygon_layer.shp:3" ) ) == 1 );
  QVERIFY( searchCheckError( checkErrors, mLayers["line_layer.shp"], 4, QgsPointXY( 0.9906, 1.1169 ), QgsVertexId( 0 ), QVariant( "polygon_layer.shp:2" ) ) == 1 );
  QVERIFY( searchCheckError( checkErrors, mLayers["line_layer.shp"], 4, QgsPointXY( 1.0133, 1.0772 ), QgsVertexId( 0 ), QVariant( "polygon_layer.shp:2" ) ) == 1 );
  QVERIFY( searchCheckError( checkErrors, mLayers["line_layer.shp"], 7, QgsPointXY( 0.9906, 1.1169 ), QgsVertexId( 0 ), QVariant( "polygon_layer.shp:2" ) ) == 1 );
  QVERIFY( searchCheckError( checkErrors, mLayers["line_layer.shp"], 7, QgsPointXY( 1.0133, 1.0772 ), QgsVertexId( 0 ), QVariant( "polygon_layer.shp:2" ) ) == 1 );
}

void TestQgsGeometryChecks::testMultipartCheck()
{
  QList<QgsGeometryCheckError *> checkErrors;
  QStringList messages;

  QgsGeometryMultipartCheck( mContext ).collectErrors( checkErrors, messages );
  listErrors( checkErrors, messages );

  QVERIFY( searchCheckError( checkErrors, mLayers["point_layer.shp"] ) == 0 );
  // Easier to ensure that multipart features don't appear as errors than verifying each single-part multi-type feature
  QVERIFY( QgsWkbTypes::isSingleType( mContext->featurePools[mLayers["point_layer.shp"]]->getLayer()->wkbType() ) );
  QVERIFY( QgsWkbTypes::isMultiType( mContext->featurePools[mLayers["line_layer.shp"]]->getLayer()->wkbType() ) );
  QVERIFY( QgsWkbTypes::isMultiType( mContext->featurePools[mLayers["polygon_layer.shp"]]->getLayer()->wkbType() ) );
  QVERIFY( searchCheckError( checkErrors, mLayers["line_layer.shp"] ) > 0 );
  QVERIFY( searchCheckError( checkErrors, mLayers["polygon_layer.shp"] ) > 0 );
  QVERIFY( searchCheckError( checkErrors, mLayers["line_layer.shp"], 0 ) == 0 );
  QVERIFY( searchCheckError( checkErrors, mLayers["polygon_layer.shp"], 1 ) == 0 );
}

void TestQgsGeometryChecks::testOverlapCheck()
{
  QList<QgsGeometryCheckError *> checkErrors;
  QStringList messages;

  QgsGeometryOverlapCheck( mContext, 0.01 ).collectErrors( checkErrors, messages );
  listErrors( checkErrors, messages );

  QCOMPARE( checkErrors.size(), 2 );
  QVERIFY( searchCheckError( checkErrors, mLayers["point_layer.shp"] ) == 0 );
  QVERIFY( searchCheckError( checkErrors, mLayers["line_layer.shp"] ) == 0 );
  QVERIFY( searchCheckError( checkErrors, mLayers["polygon_layer.shp"], 10 ) == 2 );
  QVERIFY( messages.contains( "Overlap check failed for (polygon_layer.shp:1): the geometry is invalid" ) );
}

void TestQgsGeometryChecks::testPointCoveredByLineCheck()
{
  QList<QgsGeometryCheckError *> checkErrors;
  QStringList messages;

  QgsGeometryPointCoveredByLineCheck( mContext ).collectErrors( checkErrors, messages );
  listErrors( checkErrors, messages );

  QVERIFY( searchCheckError( checkErrors, mLayers["line_layer.shp"] ) == 0 );
  QVERIFY( searchCheckError( checkErrors, mLayers["polygon_layer.shp"] ) == 0 );
  // Easier to test that no points which are covered by a line are marked as errors
  QVERIFY( searchCheckError( checkErrors, mLayers["point_layer.shp"], 0 ) == 0 );
  QVERIFY( searchCheckError( checkErrors, mLayers["point_layer.shp"], 1 ) == 0 );
}

void TestQgsGeometryChecks::testPointInPolygonCheck()
{
  QList<QgsGeometryCheckError *> checkErrors;
  QStringList messages;

  QgsGeometryPointInPolygonCheck( mContext ).collectErrors( checkErrors, messages );
  listErrors( checkErrors, messages );

  QVERIFY( searchCheckError( checkErrors, mLayers["line_layer.shp"] ) == 0 );
  QVERIFY( searchCheckError( checkErrors, mLayers["polygon_layer.shp"] ) == 0 );
  // Check that the point which is properly inside a polygon is not listed as error
  QVERIFY( searchCheckError( checkErrors, mLayers["point_layer.shp"], 5 ) == 0 );
  QVERIFY( messages.contains( "Point in polygon check failed for (polygon_layer.shp:1): the geometry is invalid" ) );
}

void TestQgsGeometryChecks::testSegmentLengthCheck()
{
  QList<QgsGeometryCheckError *> checkErrors;
  QStringList messages;

  QgsGeometrySegmentLengthCheck( mContext, 0.03 ).collectErrors( checkErrors, messages );
  listErrors( checkErrors, messages );

  QCOMPARE( checkErrors.size(), 4 );
  QVERIFY( searchCheckError( checkErrors, mLayers["point_layer.shp"] ) == 0 );
  QVERIFY( searchCheckError( checkErrors, mLayers["line_layer.shp"], 3, QgsPointXY( 0.0753, 0.7921 ), QgsVertexId( 0, 0, 3 ), 0.0197 ) == 1 );
  QVERIFY( searchCheckError( checkErrors, mLayers["polygon_layer.shp"], 2, QgsPointXY( 0.7807, 1.1009 ), QgsVertexId( 0, 0, 0 ), 0.0176 ) == 1 );
  QVERIFY( searchCheckError( checkErrors, mLayers["polygon_layer.shp"], 10, QgsPointXY( -0.2819, 1.3553 ), QgsVertexId( 0, 0, 2 ), 0.0281 ) == 1 );
  QVERIFY( searchCheckError( checkErrors, mLayers["polygon_layer.shp"], 11, QgsPointXY( -0.2819, 1.3553 ), QgsVertexId( 0, 0, 0 ), 0.0281 ) == 1 );
}

void TestQgsGeometryChecks::testSelfContactCheck()
{
  QList<QgsGeometryCheckError *> checkErrors;
  QStringList messages;

  QgsGeometrySelfContactCheck( mContext ).collectErrors( checkErrors, messages );
  listErrors( checkErrors, messages );

  QCOMPARE( checkErrors.size(), 3 );
  QVERIFY( searchCheckError( checkErrors, mLayers["point_layer.shp"] ) == 0 );
  QVERIFY( searchCheckError( checkErrors, mLayers["line_layer.shp"], 5, QgsPointXY( -1.2280, -0.8654 ), QgsVertexId( 0, 0, 0 ) ) == 1 );
  QVERIFY( searchCheckError( checkErrors, mLayers["line_layer.shp"], 5, QgsPointXY( -1.2399, -1.0502 ), QgsVertexId( 0, 0, 6 ) ) == 1 );
  QVERIFY( searchCheckError( checkErrors, mLayers["polygon_layer.shp"], 9, QgsPointXY( -0.2080, 1.9830 ), QgsVertexId( 0, 0, 3 ) ) == 1 );
}

void TestQgsGeometryChecks::testSelfIntersectionCheck()
{
  QList<QgsGeometryCheckError *> checkErrors;
  QStringList messages;

  QgsGeometrySelfIntersectionCheck( mContext ).collectErrors( checkErrors, messages );
  listErrors( checkErrors, messages );

  QCOMPARE( checkErrors.size(), 3 );
  QVERIFY( searchCheckError( checkErrors, mLayers["point_layer.shp"] ) == 0 );
  QVERIFY( searchCheckError( checkErrors, mLayers["line_layer.shp"], 1, QgsPointXY( -0.1199, 0.1440 ), QgsVertexId( 0, 0 ) ) == 1 );
  QVERIFY( searchCheckError( checkErrors, mLayers["line_layer.shp"], 1, QgsPointXY( -0.1997, 0.1044 ), QgsVertexId( 0, 0 ) ) == 1 );
  QVERIFY( searchCheckError( checkErrors, mLayers["polygon_layer.shp"], 1, QgsPointXY( 1.2592, 0.0888 ), QgsVertexId( 0, 0 ) ) == 1 );
}

void TestQgsGeometryChecks::testSliverPolygonCheck()
{
  QList<QgsGeometryCheckError *> checkErrors;
  QStringList messages;

  QgsGeometrySliverPolygonCheck( mContext, 20, 0.04 ).collectErrors( checkErrors, messages );
  listErrors( checkErrors, messages );

  QCOMPARE( checkErrors.size(), 2 );
  QVERIFY( searchCheckError( checkErrors, mLayers["point_layer.shp"] ) == 0 );
  QVERIFY( searchCheckError( checkErrors, mLayers["line_layer.shp"] ) == 0 );
  QVERIFY( searchCheckError( checkErrors, mLayers["polygon_layer.shp"], 6, QgsPointXY(), QgsVertexId( 0 ) ) == 1 );
  QVERIFY( searchCheckError( checkErrors, mLayers["polygon_layer.shp"], 11, QgsPointXY(), QgsVertexId( 0 ) ) == 1 );
}

///////////////////////////////////////////////////////////////////////////////

double TestQgsGeometryChecks::layerToMapUnits( const QgsMapLayer *layer, const QString &mapCrs ) const
{
  QgsCoordinateTransform crst = QgsCoordinateTransformCache::instance()->transform( layer->crs().authid(), mapCrs );
  QgsRectangle extent = layer->extent();
  QgsPointXY l1( extent.xMinimum(), extent.yMinimum() );
  QgsPointXY l2( extent.xMaximum(), extent.yMaximum() );
  double distLayerUnits = std::sqrt( l1.sqrDist( l2 ) );
  QgsPointXY m1 = crst.transform( l1 );
  QgsPointXY m2 = crst.transform( l2 );
  double distMapUnits = std::sqrt( m1.sqrDist( m2 ) );
  return distMapUnits / distLayerUnits;
}

QgsFeaturePool *TestQgsGeometryChecks::createFeaturePool( QgsVectorLayer *layer, const QString &mapCrs, bool selectedOnly ) const
{
  double layerToMapUntis = layerToMapUnits( layer, mapCrs );
  QgsCoordinateTransform layerToMapTransform = QgsCoordinateTransformCache::instance()->transform( layer->crs().authid(), mapCrs );
  return new QgsFeaturePool( layer, layerToMapUntis, layerToMapTransform, selectedOnly );
}

QgsGeometryCheckerContext *TestQgsGeometryChecks::createTestContext( QMap<QString, QString> &layers, const QString &mapCrs, double prec ) const
{
  QDir testDataDir( QDir( TEST_DATA_DIR ).absoluteFilePath( "geometry_checker" ) );

  QMap<QString, QgsFeaturePool *> featurePools;
  for ( const QString &layerFile : layers.keys() )
  {
    QgsVectorLayer *layer = new QgsVectorLayer( testDataDir.absoluteFilePath( layerFile ), layerFile );
    Q_ASSERT( layer && layer->isValid() );
    layers[layerFile] = layer->id();
    featurePools.insert( layer->id(), createFeaturePool( layer, mapCrs ) );
  }
  return new QgsGeometryCheckerContext( prec, mapCrs, featurePools );
}

void TestQgsGeometryChecks::cleanupTestContext( QgsGeometryCheckerContext *ctx ) const
{
  for ( const QgsFeaturePool *pool : ctx->featurePools )
  {
    delete pool->getLayer();
  }
  qDeleteAll( ctx->featurePools );
  delete ctx;
}

void TestQgsGeometryChecks::listErrors( const QList<QgsGeometryCheckError *> &checkErrors, const QStringList &messages ) const
{
  QTextStream( stdout ) << " - Check result:" << endl;
  for ( const QgsGeometryCheckError *error : checkErrors )
  {
    QTextStream( stdout ) << "   * " << error->layerId() << ":" << error->featureId() << " @[" << error->vidx().part << ", " << error->vidx().ring << ", " << error->vidx().vertex << "](" << error->location().x() << ", " << error->location().y() << ") = " << error->value().toString() << endl;
  }
  if ( !messages.isEmpty() )
  {
    QTextStream( stdout ) << " - Check messages:" << endl << "   * " << messages.join( "\n   * " ) << endl;
  }
}

int TestQgsGeometryChecks::searchCheckError( const QList<QgsGeometryCheckError *> &checkErrors, const QString &layerId, const QgsFeatureId &featureId, const QgsPointXY &pos, const QgsVertexId &vid, const QVariant &value, double tol ) const
{
  int matching = 0;
  for ( const QgsGeometryCheckError *error : checkErrors )
  {
    if ( error->layerId() != layerId )
    {
      continue;
    }
    if ( featureId != -1 && error->featureId() != featureId )
    {
      continue;
    }
    if ( pos != QgsPointXY() && ( !qgsDoubleNear( error->location().x(), pos.x(), tol ) || !qgsDoubleNear( error->location().y(), pos.y(), tol ) ) )
    {
      continue;
    }
    if ( vid.isValid() && vid != error->vidx() )
    {
      continue;
    }
    if ( !value.isNull() )
    {
      if ( value.type() == QVariant::Double )
      {
        if ( !qgsDoubleNear( value.toDouble(), error->value().toDouble(), tol ) )
        {
          continue;
        }
      }
      else if ( value != error->value() )
      {
        continue;
      }
    }
    ++matching;
  }
  return matching;
}

QGSTEST_MAIN( TestQgsGeometryChecks )
#include "testqgsgeometrychecks.moc"
