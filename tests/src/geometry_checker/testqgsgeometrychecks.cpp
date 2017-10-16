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
#include "qgsgeometryfollowboundariescheck.h"
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
    struct ExpectedChange
    {
      QString layerId;
      QgsFeatureId fid;
      QgsGeometryCheck::ChangeWhat what;
      QgsGeometryCheck::ChangeType type;
      QgsVertexId vidx;
    };
    double layerToMapUnits( const QgsMapLayer *layer, const QString &mapCrs ) const;
    QgsFeaturePool *createFeaturePool( QgsVectorLayer *layer, const QString &mapCrs, bool selectedOnly = false ) const;
    QgsGeometryCheckerContext *createTestContext( QMap<QString, QString> &layers, const QString &mapCrs = "EPSG:4326", double prec = 8 ) const;
    QgsGeometryCheckerContext *createTestContext( QTemporaryDir &tempDir, QMap<QString, QString> &layers, const QString &mapCrs = "EPSG:4326", double prec = 8 ) const;
    void cleanupTestContext( QgsGeometryCheckerContext *ctx ) const;
    void listErrors( const QList<QgsGeometryCheckError *> &checkErrors, const QStringList &messages ) const;
    QList<QgsGeometryCheckError *> searchCheckErrors( const QList<QgsGeometryCheckError *> &checkErrors, const QString &layerId, const QgsFeatureId &featureId = -1, const QgsPointXY &pos = QgsPointXY(), const QgsVertexId &vid = QgsVertexId(), const QVariant &value = QVariant(), double tol = 1E-4 ) const;
    bool fixCheckError( QgsGeometryCheckError *error, int method, const QgsGeometryCheckError::Status &expectedStatus, const QVector<ExpectedChange> &expectedChanges, const QMap<QString, int> &mergeAttr = QMap<QString, int>() );

    QMap<QString, QString> mLayers;
    QgsGeometryCheckerContext *mContext;

  private slots:
    // will be called before the first testfunction is executed.
    void initTestCase();
    // will be called after the last testfunction is executed.
    void cleanupTestCase();

  private slots:
    void testAngleCheck();
    void testAreaCheck();
    void testContainedCheck();
    void testDangleCheck();
    void testDegeneratePolygonCheck();
    void testDuplicateCheck();
    void testDuplicateNodesCheck();
    void testFollowBoundariesCheck();
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
  QTemporaryDir dir;
  QMap<QString, QString> layers;
  layers.insert( "point_layer.shp", "" );
  layers.insert( "line_layer.shp", "" );
  layers.insert( "polygon_layer.shp", "" );
  QgsGeometryCheckerContext *context = createTestContext( dir, layers );

  QList<QgsGeometryCheckError *> checkErrors;
  QStringList messages;

  QgsGeometryAngleCheck check( context, 15 );
  check.collectErrors( checkErrors, messages );
  listErrors( checkErrors, messages );

  QList<QgsGeometryCheckError *> lineErrs1;
  QList<QgsGeometryCheckError *> polyErrs1;

  QCOMPARE( checkErrors.size(), 8 );
  QVERIFY( searchCheckErrors( checkErrors, layers["point_layer.shp"] ).isEmpty() );
  QVERIFY( ( lineErrs1 = searchCheckErrors( checkErrors, layers["line_layer.shp"], 0, QgsPointXY( -0.2225,  0.5526 ), QgsVertexId( 0, 0, 3 ), 10.5865 ) ).size() == 1 );
  QVERIFY( searchCheckErrors( checkErrors, layers["line_layer.shp"], 0, QgsPointXY( -0.94996, 0.99967 ), QgsVertexId( 1, 0, 1 ), 8.3161 ).size() == 1 );
  QVERIFY( searchCheckErrors( checkErrors, layers["line_layer.shp"], 2, QgsPointXY( -0.4547, -0.3059 ), QgsVertexId( 0, 0, 1 ), 5.4165 ).size() == 1 );
  QVERIFY( searchCheckErrors( checkErrors, layers["line_layer.shp"], 2, QgsPointXY( -0.7594, -0.1971 ), QgsVertexId( 0, 0, 2 ), 12.5288 ).size() == 1 );
  QVERIFY( ( polyErrs1 = searchCheckErrors( checkErrors, layers["polygon_layer.shp"], 0, QgsPointXY( 0.2402, 1.0786 ), QgsVertexId( 0, 0, 1 ), 13.5140 ) ).size() == 1 );
  QVERIFY( searchCheckErrors( checkErrors, layers["polygon_layer.shp"], 1, QgsPointXY( 0.6960, 0.5908 ), QgsVertexId( 0, 0, 0 ), 7.0556 ).size() == 1 );
  QVERIFY( searchCheckErrors( checkErrors, layers["polygon_layer.shp"], 1, QgsPointXY( 0.98690, 0.55699 ), QgsVertexId( 1, 0, 5 ), 7.7351 ).size() == 1 );
  QVERIFY( searchCheckErrors( checkErrors, layers["polygon_layer.shp"], 11, QgsPointXY( -0.3186, 1.6734 ), QgsVertexId( 0, 0, 1 ), 3.5092 ).size() == 1 );

  QgsFeature f;
  int n1, n2;

  context->featurePools[lineErrs1[0]->layerId()]->get( lineErrs1[0]->featureId(), f );
  n1 = f.geometry().geometry()->vertexCount( lineErrs1[0]->vidx().part, lineErrs1[0]->vidx().ring );
  QVERIFY( fixCheckError( lineErrs1[0],
                          QgsGeometryAngleCheck::DeleteNode, QgsGeometryCheckError::StatusFixed,
  {{lineErrs1[0]->layerId(), lineErrs1[0]->featureId(), QgsGeometryCheck::ChangeNode, QgsGeometryCheck::ChangeRemoved, lineErrs1[0]->vidx()}} ) );
  context->featurePools[lineErrs1[0]->layerId()]->get( lineErrs1[0]->featureId(), f );
  n2 = f.geometry().geometry()->vertexCount( lineErrs1[0]->vidx().part, lineErrs1[0]->vidx().ring );
  QCOMPARE( n1, n2 + 1 );

  context->featurePools[polyErrs1[0]->layerId()]->get( polyErrs1[0]->featureId(), f );
  n1 = f.geometry().geometry()->vertexCount( polyErrs1[0]->vidx().part, polyErrs1[0]->vidx().ring );
  QVERIFY( fixCheckError( polyErrs1[0],
                          QgsGeometryAngleCheck::DeleteNode, QgsGeometryCheckError::StatusFixed,
  {{polyErrs1[0]->layerId(), polyErrs1[0]->featureId(), QgsGeometryCheck::ChangeNode, QgsGeometryCheck::ChangeRemoved, polyErrs1[0]->vidx()}} ) );
  context->featurePools[polyErrs1[0]->layerId()]->get( polyErrs1[0]->featureId(), f );
  n2 = f.geometry().geometry()->vertexCount( polyErrs1[0]->vidx().part, polyErrs1[0]->vidx().ring );
  QCOMPARE( n1, n2 + 1 );

  cleanupTestContext( context );
}

void TestQgsGeometryChecks::testAreaCheck()
{
  QTemporaryDir dir;
  QMap<QString, QString> layers;
  layers.insert( "point_layer.shp", "" );
  layers.insert( "line_layer.shp", "" );
  layers.insert( "polygon_layer.shp", "" );
  QgsGeometryCheckerContext *context = createTestContext( dir, layers );

  QList<QgsGeometryCheckError *> checkErrors;
  QStringList messages;

  QgsGeometryAreaCheck check( context, 0.04 );
  check.collectErrors( checkErrors, messages );
  listErrors( checkErrors, messages );

  QList<QgsGeometryCheckError *> polyErrs1;
  QList<QgsGeometryCheckError *> polyErrs2;
  QList<QgsGeometryCheckError *> polyErrs3;
  QList<QgsGeometryCheckError *> polyErrs4;

  QCOMPARE( checkErrors.size(), 7 );
  QVERIFY( searchCheckErrors( checkErrors, layers["point_layer.shp"] ).isEmpty() );
  QVERIFY( searchCheckErrors( checkErrors, layers["line_layer.shp"] ).isEmpty() );
  QVERIFY( searchCheckErrors( checkErrors, layers["polygon_layer.shp"], 1, QgsPointXY( 1.0068, 0.3635 ), QgsVertexId( 1 ), 0.0105 ).size() == 1 );
  QVERIFY( ( polyErrs1 = searchCheckErrors( checkErrors, layers["polygon_layer.shp"], 2, QgsPointXY( 0.9739, 1.0983 ), QgsVertexId( 0 ), 0.0141 ) ).size() == 1 );
  QVERIFY( searchCheckErrors( checkErrors, layers["polygon_layer.shp"], 6, QgsPointXY( 0.9968, 1.7584 ), QgsVertexId( 0 ), 0 ).size() == 1 );
  QVERIFY( searchCheckErrors( checkErrors, layers["polygon_layer.shp"], 11, QgsPointXY( -0.2941, 1.4614 ), QgsVertexId( 0 ), 0.0031 ).size() == 1 );
  QVERIFY( ( polyErrs2 = searchCheckErrors( checkErrors, layers["polygon_layer.shp"], 13, QgsPointXY( 0.5026, 3.0267 ), QgsVertexId( 0 ), 0.0013 ) ).size() == 1 );
  QVERIFY( ( polyErrs3 = searchCheckErrors( checkErrors, layers["polygon_layer.shp"], 20, QgsPointXY( 0.4230, 3.4688 ), QgsVertexId( 0 ), 0.0013 ) ).size() == 1 );
  QVERIFY( ( polyErrs4 = searchCheckErrors( checkErrors, layers["polygon_layer.shp"], 24, QgsPointXY( 1.4186, 3.0905 ), QgsVertexId( 0 ), 0.0013 ) ).size() == 1 );

  QgsFeature f;
  bool valid;

  QVERIFY( fixCheckError( polyErrs1[0],
                          QgsGeometryAreaCheck::Delete, QgsGeometryCheckError::StatusFixed,
  {{polyErrs1[0]->layerId(), polyErrs1[0]->featureId(), QgsGeometryCheck::ChangeFeature, QgsGeometryCheck::ChangeRemoved, QgsVertexId()}} ) );
  valid = context->featurePools[polyErrs1[0]->layerId()]->get( polyErrs1[0]->featureId(), f );
  QVERIFY( !valid );

  // Try merging a small geometry by longest edge, largest area and common value
  context->featurePools[layers["polygon_layer.shp"]]->get( 15, f );
  double area15 = f.geometry().area();
  QVERIFY( fixCheckError( polyErrs2[0],
                          QgsGeometryAreaCheck::MergeLargestArea, QgsGeometryCheckError::StatusFixed,
  {
    {polyErrs2[0]->layerId(), polyErrs2[0]->featureId(), QgsGeometryCheck::ChangeFeature, QgsGeometryCheck::ChangeRemoved, QgsVertexId()},
    {layers["polygon_layer.shp"], 15, QgsGeometryCheck::ChangeFeature, QgsGeometryCheck::ChangeRemoved, QgsVertexId( 0 )},
    {layers["polygon_layer.shp"], 15, QgsGeometryCheck::ChangeFeature, QgsGeometryCheck::ChangeAdded, QgsVertexId( 0 )}
  } ) );
  context->featurePools[layers["polygon_layer.shp"]]->get( 15, f );
  QVERIFY( f.geometry().area() > area15 );
  valid = context->featurePools[polyErrs2[0]->layerId()]->get( polyErrs2[0]->featureId(), f );
  QVERIFY( !valid );

  context->featurePools[layers["polygon_layer.shp"]]->get( 18, f );
  double area18 = f.geometry().area();
  QVERIFY( fixCheckError( polyErrs3[0],
                          QgsGeometryAreaCheck::MergeLongestEdge, QgsGeometryCheckError::StatusFixed,
  {
    {polyErrs3[0]->layerId(), polyErrs3[0]->featureId(), QgsGeometryCheck::ChangeFeature, QgsGeometryCheck::ChangeRemoved, QgsVertexId()},
    {layers["polygon_layer.shp"], 18, QgsGeometryCheck::ChangeFeature, QgsGeometryCheck::ChangeRemoved, QgsVertexId( 0 )},
    {layers["polygon_layer.shp"], 18, QgsGeometryCheck::ChangeFeature, QgsGeometryCheck::ChangeAdded, QgsVertexId( 0 )}
  } ) );
  context->featurePools[layers["polygon_layer.shp"]]->get( 18, f );
  QVERIFY( f.geometry().area() > area18 );
  valid = context->featurePools[polyErrs3[0]->layerId()]->get( polyErrs3[0]->featureId(), f );
  QVERIFY( !valid );

  context->featurePools[layers["polygon_layer.shp"]]->get( 21, f );
  double area21 = f.geometry().area();
  QMap<QString, int> mergeIdx;
  mergeIdx.insert( layers["polygon_layer.shp"], 1 ); // 1: attribute "attr"
  QVERIFY( fixCheckError( polyErrs4[0],
                          QgsGeometryAreaCheck::MergeIdenticalAttribute, QgsGeometryCheckError::StatusFixed,
  {
    {polyErrs4[0]->layerId(), polyErrs4[0]->featureId(), QgsGeometryCheck::ChangeFeature, QgsGeometryCheck::ChangeRemoved, QgsVertexId()},
    {layers["polygon_layer.shp"], 21, QgsGeometryCheck::ChangeFeature, QgsGeometryCheck::ChangeRemoved, QgsVertexId( 0 )},
    {layers["polygon_layer.shp"], 21, QgsGeometryCheck::ChangeFeature, QgsGeometryCheck::ChangeAdded, QgsVertexId( 0 )}
  }, mergeIdx ) );
  context->featurePools[layers["polygon_layer.shp"]]->get( 21, f );
  QVERIFY( f.geometry().area() > area21 );
  valid = context->featurePools[polyErrs4[0]->layerId()]->get( polyErrs4[0]->featureId(), f );
  QVERIFY( !valid );

  cleanupTestContext( context );
}

void TestQgsGeometryChecks::testContainedCheck()
{
  QTemporaryDir dir;
  QMap<QString, QString> layers;
  layers.insert( "point_layer.shp", "" );
  layers.insert( "line_layer.shp", "" );
  layers.insert( "polygon_layer.shp", "" );
  QgsGeometryCheckerContext *context = createTestContext( dir, layers );

  QList<QgsGeometryCheckError *> checkErrors;
  QStringList messages;

  QgsGeometryContainedCheck check( context );
  check.collectErrors( checkErrors, messages );
  listErrors( checkErrors, messages );

  QList<QgsGeometryCheckError *> polyErrs1;

  QCOMPARE( checkErrors.size(), 4 );
  QVERIFY( searchCheckErrors( checkErrors, layers["point_layer.shp"], 4, QgsPointXY(), QgsVertexId(), QVariant( "polygon_layer.shp:5" ) ).size() == 1 );
  QVERIFY( searchCheckErrors( checkErrors, layers["point_layer.shp"], 5, QgsPointXY(), QgsVertexId(), QVariant( "polygon_layer.shp:0" ) ).size() == 1 );
  QVERIFY( searchCheckErrors( checkErrors, layers["line_layer.shp"], 3, QgsPointXY(), QgsVertexId(), QVariant( "polygon_layer.shp:0" ) ).size() == 1 );
  QVERIFY( ( polyErrs1 = searchCheckErrors( checkErrors, layers["polygon_layer.shp"], 3, QgsPointXY(), QgsVertexId(), QVariant( "polygon_layer.shp:0" ) ) ).size() == 1 );
  QVERIFY( messages.contains( "Contained check failed for (polygon_layer.shp:1): the geometry is invalid" ) );

  QVERIFY( fixCheckError( polyErrs1[0],
                          QgsGeometryContainedCheck::Delete, QgsGeometryCheckError::StatusFixed,
  {{polyErrs1[0]->layerId(), polyErrs1[0]->featureId(), QgsGeometryCheck::ChangeFeature, QgsGeometryCheck::ChangeRemoved, QgsVertexId()}} ) );
  QgsFeature f;
  bool valid = context->featurePools[polyErrs1[0]->layerId()]->get( polyErrs1[0]->featureId(), f );
  QVERIFY( !valid );

  cleanupTestContext( context );
}

void TestQgsGeometryChecks::testDangleCheck()
{
  QTemporaryDir dir;
  QMap<QString, QString> layers;
  layers.insert( "point_layer.shp", "" );
  layers.insert( "line_layer.shp", "" );
  layers.insert( "polygon_layer.shp", "" );
  QgsGeometryCheckerContext *context = createTestContext( dir, layers );

  QList<QgsGeometryCheckError *> checkErrors;
  QStringList messages;

  QgsGeometryDangleCheck check( context );
  check.collectErrors( checkErrors, messages );
  listErrors( checkErrors, messages );

  QCOMPARE( checkErrors.size(), 4 );
  QVERIFY( searchCheckErrors( checkErrors, layers["point_layer.shp"] ).isEmpty() );
  QVERIFY( searchCheckErrors( checkErrors, layers["polygon_layer.shp"] ).isEmpty() );
  QVERIFY( searchCheckErrors( checkErrors, layers["line_layer.shp"], 0, QgsPointXY( -0.7558, 0.7648 ), QgsVertexId( 1, 0, 0 ) ).size() == 1 );
  QVERIFY( searchCheckErrors( checkErrors, layers["line_layer.shp"], 2, QgsPointXY( -0.7787, -0.2237 ), QgsVertexId( 0, 0, 0 ) ).size() == 1 );
  QVERIFY( searchCheckErrors( checkErrors, layers["line_layer.shp"], 3, QgsPointXY( -0.2326, 0.9537 ), QgsVertexId( 0, 0, 0 ) ).size() == 1 );
  QVERIFY( searchCheckErrors( checkErrors, layers["line_layer.shp"], 3, QgsPointXY( 0.0715, 0.7830 ), QgsVertexId( 0, 0, 3 ) ).size() == 1 );

  cleanupTestContext( context );
}

void TestQgsGeometryChecks::testDegeneratePolygonCheck()
{
  QTemporaryDir dir;
  QMap<QString, QString> layers;
  layers.insert( "point_layer.shp", "" );
  layers.insert( "line_layer.shp", "" );
  layers.insert( "polygon_layer.shp", "" );
  QgsGeometryCheckerContext *context = createTestContext( dir, layers );

  QList<QgsGeometryCheckError *> checkErrors;
  QStringList messages;

  QgsGeometryDegeneratePolygonCheck check( context );
  check.collectErrors( checkErrors, messages );
  listErrors( checkErrors, messages );

  QList<QgsGeometryCheckError *> polyErrs1;

  QCOMPARE( checkErrors.size(), 1 );
  QVERIFY( searchCheckErrors( checkErrors, layers["point_layer.shp"] ).isEmpty() );
  QVERIFY( searchCheckErrors( checkErrors, layers["line_layer.shp"] ).isEmpty() );
  QVERIFY( ( polyErrs1 = searchCheckErrors( checkErrors, layers["polygon_layer.shp"], 6, QgsPointXY(), QgsVertexId( 0, 0 ) ) ).size() == 1 );

  QVERIFY( fixCheckError( polyErrs1[0],
                          QgsGeometryDegeneratePolygonCheck::DeleteRing, QgsGeometryCheckError::StatusFixed,
  {{polyErrs1[0]->layerId(), polyErrs1[0]->featureId(), QgsGeometryCheck::ChangeFeature, QgsGeometryCheck::ChangeRemoved, QgsVertexId()}} ) );
  QgsFeature f;
  bool valid = context->featurePools[polyErrs1[0]->layerId()]->get( polyErrs1[0]->featureId(), f );
  QVERIFY( !valid );

  cleanupTestContext( context );
}

void TestQgsGeometryChecks::testDuplicateCheck()
{
  QTemporaryDir dir;
  QMap<QString, QString> layers;
  layers.insert( "point_layer.shp", "" );
  layers.insert( "line_layer.shp", "" );
  layers.insert( "polygon_layer.shp", "" );
  QgsGeometryCheckerContext *context = createTestContext( dir, layers );

  QList<QgsGeometryCheckError *> checkErrors;
  QStringList messages;

  QgsGeometryDuplicateCheck check( context );
  check.collectErrors( checkErrors, messages );
  listErrors( checkErrors, messages );

  QList<QgsGeometryCheckError *> polyErrs1;

  QCOMPARE( checkErrors.size(), 3 );
  QVERIFY(
    searchCheckErrors( checkErrors, layers["point_layer.shp"], 6, QgsPoint(), QgsVertexId(), QVariant( "point_layer.shp:2" ) ).size() == 1
    || searchCheckErrors( checkErrors, layers["point_layer.shp"], 2, QgsPoint(), QgsVertexId(), QVariant( "point_layer.shp:6" ) ).size() == 1 );
  QVERIFY(
    searchCheckErrors( checkErrors, layers["line_layer.shp"], 4, QgsPoint(), QgsVertexId(), QVariant( "line_layer.shp:7" ) ).size() == 1
    || searchCheckErrors( checkErrors, layers["line_layer.shp"], 7, QgsPoint(), QgsVertexId(), QVariant( "line_layer.shp:4" ) ).size() == 1 );
  QVERIFY(
    ( polyErrs1 = searchCheckErrors( checkErrors, layers["polygon_layer.shp"], 8, QgsPoint(), QgsVertexId(), QVariant( "polygon_layer.shp:7" ) ) ).size() == 1
    || ( polyErrs1 = searchCheckErrors( checkErrors, layers["polygon_layer.shp"], 7, QgsPoint(), QgsVertexId(), QVariant( "polygon_layer.shp:8" ) ) ).size() == 1 );

  QgsGeometryDuplicateCheckError *dupErr = static_cast<QgsGeometryDuplicateCheckError *>( polyErrs1[0] );
  QString dup1LayerId = dupErr->duplicates().firstKey();
  QgsFeatureId dup1Fid = dupErr->duplicates()[dup1LayerId][0];
  QVERIFY( fixCheckError( dupErr,
                          QgsGeometryDuplicateCheck::RemoveDuplicates, QgsGeometryCheckError::StatusFixed,
  {{dup1LayerId, dup1Fid, QgsGeometryCheck::ChangeFeature, QgsGeometryCheck::ChangeRemoved, QgsVertexId()}} ) );
  QgsFeature f;
  bool valid = context->featurePools[dup1LayerId]->get( dup1Fid, f );
  QVERIFY( !valid );

  cleanupTestContext( context );
}

void TestQgsGeometryChecks::testDuplicateNodesCheck()
{
  QTemporaryDir dir;
  QMap<QString, QString> layers;
  layers.insert( "point_layer.shp", "" );
  layers.insert( "line_layer.shp", "" );
  layers.insert( "polygon_layer.shp", "" );
  QgsGeometryCheckerContext *context = createTestContext( dir, layers );

  QList<QgsGeometryCheckError *> checkErrors;
  QStringList messages;

  QgsGeometryDuplicateNodesCheck check( context );
  check.collectErrors( checkErrors, messages );
  listErrors( checkErrors, messages );

  QList<QgsGeometryCheckError *> polyErrs1;

  QCOMPARE( checkErrors.size(), 4 );
  QVERIFY( searchCheckErrors( checkErrors, layers["point_layer.shp"] ).isEmpty() );
  QVERIFY( searchCheckErrors( checkErrors, layers["line_layer.shp"], 0, QgsPointXY( -0.6360, 0.6203 ), QgsVertexId( 0, 0, 5 ) ).size() == 1 );
  QVERIFY( searchCheckErrors( checkErrors, layers["line_layer.shp"], 6, QgsPointXY( 0.2473, 2.0821 ), QgsVertexId( 0, 0, 1 ) ).size() == 1 );
  QVERIFY( searchCheckErrors( checkErrors, layers["line_layer.shp"], 6, QgsPointXY( 0.5158, 2.0930 ), QgsVertexId( 0, 0, 3 ) ).size() == 1 );
  QVERIFY( ( polyErrs1 = searchCheckErrors( checkErrors, layers["polygon_layer.shp"], 4, QgsPointXY( 1.6319, 0.5642 ), QgsVertexId( 0, 0, 1 ) ) ).size() == 1 );

  QgsFeature f;

  context->featurePools[polyErrs1[0]->layerId()]->get( polyErrs1[0]->featureId(), f );
  int n1 = f.geometry().geometry()->vertexCount( polyErrs1[0]->vidx().part, polyErrs1[0]->vidx().ring );
  QVERIFY( fixCheckError( polyErrs1[0],
                          QgsGeometryDuplicateNodesCheck::RemoveDuplicates, QgsGeometryCheckError::StatusFixed,
  {{polyErrs1[0]->layerId(), polyErrs1[0]->featureId(), QgsGeometryCheck::ChangeNode, QgsGeometryCheck::ChangeRemoved, polyErrs1[0]->vidx()}} ) );
  context->featurePools[polyErrs1[0]->layerId()]->get( polyErrs1[0]->featureId(), f );
  int n2 = f.geometry().geometry()->vertexCount( polyErrs1[0]->vidx().part, polyErrs1[0]->vidx().ring );
  QCOMPARE( n1, n2 + 1 );

  cleanupTestContext( context );
}

void TestQgsGeometryChecks::testFollowBoundariesCheck()
{
  QMap<QString, QString> layers;
  layers.insert( "follow_ref.shp", "" );
  layers.insert( "follow_subj.shp", "" );
  QgsGeometryCheckerContext *context = createTestContext( layers );

  QList<QgsGeometryCheckError *> checkErrors;
  QStringList messages;

  QgsGeometryFollowBoundariesCheck( context, context->featurePools[layers["follow_ref.shp"]]->getLayer() ).collectErrors( checkErrors, messages );
  listErrors( checkErrors, messages );

  QCOMPARE( checkErrors.size(), 2 );
  QVERIFY( searchCheckErrors( checkErrors, layers["follow_subj.shp"], 1 ).size() == 1 );
  QVERIFY( searchCheckErrors( checkErrors, layers["follow_subj.shp"], 3 ).size() == 1 );

  cleanupTestContext( context );
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
  QVERIFY( searchCheckErrors( checkErrors, "", -1, QgsPointXY( 0.2924, -0.8798 ), QgsVertexId(), 0.0027 ).size() == 1 );
  QVERIFY( searchCheckErrors( checkErrors, "", -1, QgsPointXY( 0.4238, -0.7479 ), QgsVertexId(), 0.0071 ).size() == 1 );
  QVERIFY( searchCheckErrors( checkErrors, "", -1, QgsPointXY( 0.0094, -0.4448 ), QgsVertexId(), 0.0033 ).size() == 1 );
  QVERIFY( searchCheckErrors( checkErrors, "", -1, QgsPointXY( 0.2939, -0.4694 ), QgsVertexId(), 0.0053 ).size() == 1 );
  QVERIFY( searchCheckErrors( checkErrors, "", -1, QgsPointXY( 0.6284, -0.3641 ), QgsVertexId(), 0.0018 ).size() == 1 );

  cleanupTestContext( context );
}

void TestQgsGeometryChecks::testHoleCheck()
{
  QList<QgsGeometryCheckError *> checkErrors;
  QStringList messages;

  QgsGeometryHoleCheck( mContext ).collectErrors( checkErrors, messages );
  listErrors( checkErrors, messages );

  QCOMPARE( checkErrors.size(), 1 );
  QVERIFY( searchCheckErrors( checkErrors, mLayers["point_layer.shp"] ).isEmpty() );
  QVERIFY( searchCheckErrors( checkErrors, mLayers["line_layer.shp"] ).isEmpty() );
  QVERIFY( searchCheckErrors( checkErrors, mLayers["polygon_layer.shp"], 12, QgsPointXY( 0.1772, 0.10476 ), QgsVertexId( 0, 1 ) ).size() == 1 );
}

void TestQgsGeometryChecks::testLineIntersectionCheck()
{
  QList<QgsGeometryCheckError *> checkErrors;
  QStringList messages;

  QgsGeometryLineIntersectionCheck( mContext ).collectErrors( checkErrors, messages );
  listErrors( checkErrors, messages );

  QCOMPARE( checkErrors.size(), 1 );
  QVERIFY( searchCheckErrors( checkErrors, mLayers["point_layer.shp"] ).isEmpty() );
  QVERIFY( searchCheckErrors( checkErrors, mLayers["polygon_layer.shp"] ).isEmpty() );
  QVERIFY( searchCheckErrors( checkErrors, mLayers["line_layer.shp"], 1, QgsPointXY( -0.5594, 0.4098 ), QgsVertexId( 0 ), QVariant( "line_layer.shp:0" ) ).size() == 1 );
}

void TestQgsGeometryChecks::testLineLayerIntersectionCheck()
{
  QList<QgsGeometryCheckError *> checkErrors;
  QStringList messages;

  QgsGeometryLineLayerIntersectionCheck( mContext, mLayers["polygon_layer.shp"] ).collectErrors( checkErrors, messages );
  listErrors( checkErrors, messages );

  QCOMPARE( checkErrors.size(), 5 );
  QVERIFY( searchCheckErrors( checkErrors, mLayers["point_layer.shp"] ).isEmpty() );
  QVERIFY( searchCheckErrors( checkErrors, mLayers["polygon_layer.shp"] ).isEmpty() );
  QVERIFY( searchCheckErrors( checkErrors, mLayers["line_layer.shp"], 3, QgsPointXY( -0.1890, 0.9043 ), QgsVertexId( 0 ), QVariant( "polygon_layer.shp:3" ) ).size() == 1 );
  QVERIFY( searchCheckErrors( checkErrors, mLayers["line_layer.shp"], 4, QgsPointXY( 0.9906, 1.1169 ), QgsVertexId( 0 ), QVariant( "polygon_layer.shp:2" ) ).size() == 1 );
  QVERIFY( searchCheckErrors( checkErrors, mLayers["line_layer.shp"], 4, QgsPointXY( 1.0133, 1.0772 ), QgsVertexId( 0 ), QVariant( "polygon_layer.shp:2" ) ).size() == 1 );
  QVERIFY( searchCheckErrors( checkErrors, mLayers["line_layer.shp"], 7, QgsPointXY( 0.9906, 1.1169 ), QgsVertexId( 0 ), QVariant( "polygon_layer.shp:2" ) ).size() == 1 );
  QVERIFY( searchCheckErrors( checkErrors, mLayers["line_layer.shp"], 7, QgsPointXY( 1.0133, 1.0772 ), QgsVertexId( 0 ), QVariant( "polygon_layer.shp:2" ) ).size() == 1 );
}

void TestQgsGeometryChecks::testMultipartCheck()
{
  QList<QgsGeometryCheckError *> checkErrors;
  QStringList messages;

  QgsGeometryMultipartCheck( mContext ).collectErrors( checkErrors, messages );
  listErrors( checkErrors, messages );

  QVERIFY( searchCheckErrors( checkErrors, mLayers["point_layer.shp"] ).isEmpty() );
  // Easier to ensure that multipart features don't appear as errors than verifying each single-part multi-type feature
  QVERIFY( QgsWkbTypes::isSingleType( mContext->featurePools[mLayers["point_layer.shp"]]->getLayer()->wkbType() ) );
  QVERIFY( QgsWkbTypes::isMultiType( mContext->featurePools[mLayers["line_layer.shp"]]->getLayer()->wkbType() ) );
  QVERIFY( QgsWkbTypes::isMultiType( mContext->featurePools[mLayers["polygon_layer.shp"]]->getLayer()->wkbType() ) );
  QVERIFY( searchCheckErrors( checkErrors, mLayers["line_layer.shp"] ).size() > 0 );
  QVERIFY( searchCheckErrors( checkErrors, mLayers["polygon_layer.shp"] ).size() > 0 );
  QVERIFY( searchCheckErrors( checkErrors, mLayers["line_layer.shp"], 0 ).isEmpty() );
  QVERIFY( searchCheckErrors( checkErrors, mLayers["polygon_layer.shp"], 1 ).isEmpty() );
}

void TestQgsGeometryChecks::testOverlapCheck()
{
  QList<QgsGeometryCheckError *> checkErrors;
  QStringList messages;

  QgsGeometryOverlapCheck( mContext, 0.01 ).collectErrors( checkErrors, messages );
  listErrors( checkErrors, messages );

  QCOMPARE( checkErrors.size(), 2 );
  QVERIFY( searchCheckErrors( checkErrors, mLayers["point_layer.shp"] ).isEmpty() );
  QVERIFY( searchCheckErrors( checkErrors, mLayers["line_layer.shp"] ).isEmpty() );
  QVERIFY( searchCheckErrors( checkErrors, mLayers["polygon_layer.shp"], 10 ).size() == 2 );
  QVERIFY( messages.contains( "Overlap check failed for (polygon_layer.shp:1): the geometry is invalid" ) );
}

void TestQgsGeometryChecks::testPointCoveredByLineCheck()
{
  QList<QgsGeometryCheckError *> checkErrors;
  QStringList messages;

  QgsGeometryPointCoveredByLineCheck( mContext ).collectErrors( checkErrors, messages );
  listErrors( checkErrors, messages );

  QVERIFY( searchCheckErrors( checkErrors, mLayers["line_layer.shp"] ).isEmpty() );
  QVERIFY( searchCheckErrors( checkErrors, mLayers["polygon_layer.shp"] ).isEmpty() );
  // Easier to test that no points which are covered by a line are marked as errors
  QVERIFY( searchCheckErrors( checkErrors, mLayers["point_layer.shp"], 0 ).isEmpty() );
  QVERIFY( searchCheckErrors( checkErrors, mLayers["point_layer.shp"], 1 ).isEmpty() );
}

void TestQgsGeometryChecks::testPointInPolygonCheck()
{
  QList<QgsGeometryCheckError *> checkErrors;
  QStringList messages;

  QgsGeometryPointInPolygonCheck( mContext ).collectErrors( checkErrors, messages );
  listErrors( checkErrors, messages );

  QVERIFY( searchCheckErrors( checkErrors, mLayers["line_layer.shp"] ).isEmpty() );
  QVERIFY( searchCheckErrors( checkErrors, mLayers["polygon_layer.shp"] ).isEmpty() );
  // Check that the point which is properly inside a polygon is not listed as error
  QVERIFY( searchCheckErrors( checkErrors, mLayers["point_layer.shp"], 5 ).isEmpty() );
  QVERIFY( messages.contains( "Point in polygon check failed for (polygon_layer.shp:1): the geometry is invalid" ) );
}

void TestQgsGeometryChecks::testSegmentLengthCheck()
{
  QList<QgsGeometryCheckError *> checkErrors;
  QStringList messages;

  QgsGeometrySegmentLengthCheck( mContext, 0.03 ).collectErrors( checkErrors, messages );
  listErrors( checkErrors, messages );

  QCOMPARE( checkErrors.size(), 4 );
  QVERIFY( searchCheckErrors( checkErrors, mLayers["point_layer.shp"] ).isEmpty() );
  QVERIFY( searchCheckErrors( checkErrors, mLayers["line_layer.shp"], 3, QgsPointXY( 0.0753, 0.7921 ), QgsVertexId( 0, 0, 3 ), 0.0197 ).size() == 1 );
  QVERIFY( searchCheckErrors( checkErrors, mLayers["polygon_layer.shp"], 2, QgsPointXY( 0.7807, 1.1009 ), QgsVertexId( 0, 0, 0 ), 0.0176 ).size() == 1 );
  QVERIFY( searchCheckErrors( checkErrors, mLayers["polygon_layer.shp"], 10, QgsPointXY( -0.2819, 1.3553 ), QgsVertexId( 0, 0, 2 ), 0.0281 ).size() == 1 );
  QVERIFY( searchCheckErrors( checkErrors, mLayers["polygon_layer.shp"], 11, QgsPointXY( -0.2819, 1.3553 ), QgsVertexId( 0, 0, 0 ), 0.0281 ).size() == 1 );
}

void TestQgsGeometryChecks::testSelfContactCheck()
{
  QList<QgsGeometryCheckError *> checkErrors;
  QStringList messages;

  QgsGeometrySelfContactCheck( mContext ).collectErrors( checkErrors, messages );
  listErrors( checkErrors, messages );

  QCOMPARE( checkErrors.size(), 3 );
  QVERIFY( searchCheckErrors( checkErrors, mLayers["point_layer.shp"] ).isEmpty() );
  QVERIFY( searchCheckErrors( checkErrors, mLayers["line_layer.shp"], 5, QgsPointXY( -1.2280, -0.8654 ), QgsVertexId( 0, 0, 0 ) ).size() == 1 );
  QVERIFY( searchCheckErrors( checkErrors, mLayers["line_layer.shp"], 5, QgsPointXY( -1.2399, -1.0502 ), QgsVertexId( 0, 0, 6 ) ).size() == 1 );
  QVERIFY( searchCheckErrors( checkErrors, mLayers["polygon_layer.shp"], 9, QgsPointXY( -0.2080, 1.9830 ), QgsVertexId( 0, 0, 3 ) ).size() == 1 );
}

void TestQgsGeometryChecks::testSelfIntersectionCheck()
{
  QList<QgsGeometryCheckError *> checkErrors;
  QStringList messages;

  QgsGeometrySelfIntersectionCheck( mContext ).collectErrors( checkErrors, messages );
  listErrors( checkErrors, messages );

  QCOMPARE( checkErrors.size(), 3 );
  QVERIFY( searchCheckErrors( checkErrors, mLayers["point_layer.shp"] ).isEmpty() );
  QVERIFY( searchCheckErrors( checkErrors, mLayers["line_layer.shp"], 1, QgsPointXY( -0.1199, 0.1440 ), QgsVertexId( 0, 0 ) ).size() == 1 );
  QVERIFY( searchCheckErrors( checkErrors, mLayers["line_layer.shp"], 1, QgsPointXY( -0.1997, 0.1044 ), QgsVertexId( 0, 0 ) ).size() == 1 );
  QVERIFY( searchCheckErrors( checkErrors, mLayers["polygon_layer.shp"], 1, QgsPointXY( 1.2592, 0.0888 ), QgsVertexId( 0, 0 ) ).size() == 1 );
}

void TestQgsGeometryChecks::testSliverPolygonCheck()
{
  QList<QgsGeometryCheckError *> checkErrors;
  QStringList messages;

  QgsGeometrySliverPolygonCheck( mContext, 20, 0.04 ).collectErrors( checkErrors, messages );
  listErrors( checkErrors, messages );

  QCOMPARE( checkErrors.size(), 2 );
  QVERIFY( searchCheckErrors( checkErrors, mLayers["point_layer.shp"] ).isEmpty() );
  QVERIFY( searchCheckErrors( checkErrors, mLayers["line_layer.shp"] ).isEmpty() );
  QVERIFY( searchCheckErrors( checkErrors, mLayers["polygon_layer.shp"], 6, QgsPointXY(), QgsVertexId( 0 ) ).size() == 1 );
  QVERIFY( searchCheckErrors( checkErrors, mLayers["polygon_layer.shp"], 11, QgsPointXY(), QgsVertexId( 0 ) ).size() == 1 );
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
    layer->dataProvider()->enterUpdateMode();
    featurePools.insert( layer->id(), createFeaturePool( layer, mapCrs ) );
  }
  return new QgsGeometryCheckerContext( prec, mapCrs, featurePools );
}

QgsGeometryCheckerContext *TestQgsGeometryChecks::createTestContext( QTemporaryDir &tempDir, QMap<QString, QString> &layers, const QString &mapCrs, double prec ) const
{
  QDir testDataDir( QDir( TEST_DATA_DIR ).absoluteFilePath( "geometry_checker" ) );

  QMap<QString, QgsFeaturePool *> featurePools;
  for ( const QString &layerFile : layers.keys() )
  {
    QFile( testDataDir.absoluteFilePath( layerFile ) ).copy( tempDir.filePath( layerFile ) );
    if ( layerFile.endsWith( ".shp", Qt::CaseInsensitive ) )
    {
      QString baseName = QFileInfo( layerFile ).baseName();
      QFile( testDataDir.absoluteFilePath( baseName + ".dbf" ) ).copy( tempDir.filePath( baseName + ".dbf" ) );
      QFile( testDataDir.absoluteFilePath( baseName + ".pri" ) ).copy( tempDir.filePath( baseName + ".pri" ) );
      QFile( testDataDir.absoluteFilePath( baseName + ".qpj" ) ).copy( tempDir.filePath( baseName + ".qpj" ) );
      QFile( testDataDir.absoluteFilePath( baseName + ".shx" ) ).copy( tempDir.filePath( baseName + ".shx" ) );
    }
    QgsVectorLayer *layer = new QgsVectorLayer( tempDir.filePath( layerFile ), layerFile );
    Q_ASSERT( layer && layer->isValid() );
    layers[layerFile] = layer->id();
    layer->dataProvider()->enterUpdateMode();
    featurePools.insert( layer->id(), createFeaturePool( layer, mapCrs ) );
  }
  return new QgsGeometryCheckerContext( prec, mapCrs, featurePools );
}

void TestQgsGeometryChecks::cleanupTestContext( QgsGeometryCheckerContext *ctx ) const
{
  for ( const QgsFeaturePool *pool : ctx->featurePools )
  {
    pool->getLayer()->dataProvider()->leaveUpdateMode();
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

QList<QgsGeometryCheckError *> TestQgsGeometryChecks::searchCheckErrors( const QList<QgsGeometryCheckError *> &checkErrors, const QString &layerId, const QgsFeatureId &featureId, const QgsPointXY &pos, const QgsVertexId &vid, const QVariant &value, double tol ) const
{
  QList<QgsGeometryCheckError *> matching;
  for ( QgsGeometryCheckError *error : checkErrors )
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
    matching.append( error );
  }
  return matching;
}

bool TestQgsGeometryChecks::fixCheckError( QgsGeometryCheckError *error, int method, const QgsGeometryCheckError::Status &expectedStatus, const QVector<ExpectedChange> &expectedChanges, const QMap<QString, int> &mergeAttrs )
{
  QTextStream( stdout ) << " - Fixing " << error->layerId() << ":" << error->featureId() << " @[" << error->vidx().part << ", " << error->vidx().ring << ", " << error->vidx().vertex << "](" << error->location().x() << ", " << error->location().y() << ") = " << error->value().toString() << endl;
  QgsGeometryCheck::Changes changes;
  error->check()->fixError( error, method, mergeAttrs, changes );
  QTextStream( stdout ) << "   * Fix status: " << error->status() << endl;
  if ( error->status() != expectedStatus )
  {
    return false;
  }
  QString strChangeWhat[] = { "ChangeFeature", "ChangePart", "ChangeRing", "ChangeNode" };
  QString strChangeType[] = { "ChangeAdded", "ChangeRemoved", "ChangeChanged" };
  int totChanges = 0;
  for ( const QString &layerId : changes.keys() )
  {
    for ( const QgsFeatureId &fid : changes[layerId].keys() )
    {
      for ( const QgsGeometryCheck::Change &change : changes[layerId][fid] )
      {
        QTextStream( stdout ) << "   * Change: " << layerId << ":" << fid << " :: " << strChangeWhat[change.what] << ", " << strChangeType[change.type] << ", " << change.vidx.part << ":" << change.vidx.ring << ":" << change.vidx.vertex << endl;
      }
      totChanges += changes[layerId][fid].size();
    }
  }
  QTextStream( stdout ) << "   * Num changes: " << totChanges << ", expected num changes: " << expectedChanges.size() << endl;
  if ( expectedChanges.size() != totChanges )
  {
    return false;
  }
  for ( const ExpectedChange &expectedChange : expectedChanges )
  {
    if ( !changes.contains( expectedChange.layerId ) )
    {
      return false;
    }
    if ( !changes[expectedChange.layerId].contains( expectedChange.fid ) )
    {
      return false;
    }
    QgsGeometryCheck::Change change( expectedChange.what, expectedChange.type, expectedChange.vidx );
    if ( !changes[expectedChange.layerId][expectedChange.fid].contains( change ) )
    {
      return false;
    }
  }
  return true;
}

QGSTEST_MAIN( TestQgsGeometryChecks )
#include "testqgsgeometrychecks.moc"
