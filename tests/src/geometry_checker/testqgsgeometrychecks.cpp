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

  QCOMPARE( checkErrors.size(), 7 );
  QVERIFY( searchCheckError( checkErrors, mLayers["point_layer.shp"] ) == 0 );
  QVERIFY( searchCheckError( checkErrors, mLayers["line_layer.shp"], 0, QgsPointXY( -0.2225,  0.5526 ), QgsVertexId( 0, 0, 3 ), 10.5865 ) == 1 );
  QVERIFY( searchCheckError( checkErrors, mLayers["line_layer.shp"], 0, QgsPointXY( -0.94996, 0.99967 ), QgsVertexId( 1, 0, 1 ), 8.3161 ) == 1 );
  QVERIFY( searchCheckError( checkErrors, mLayers["line_layer.shp"], 2, QgsPointXY( -0.4547, -0.3059 ), QgsVertexId( 0, 0, 1 ), 5.4165 ) == 1 );
  QVERIFY( searchCheckError( checkErrors, mLayers["line_layer.shp"], 2, QgsPointXY( -0.7594, -0.1971 ), QgsVertexId( 0, 0, 2 ), 12.5288 ) == 1 );
  QVERIFY( searchCheckError( checkErrors, mLayers["polygon_layer.shp"], 0, QgsPointXY( 0.2402, 1.0786 ), QgsVertexId( 0, 0, 1 ), 13.5140 ) == 1 );
  QVERIFY( searchCheckError( checkErrors, mLayers["polygon_layer.shp"], 1, QgsPointXY( 0.6960, 0.5908 ), QgsVertexId( 0, 0, 0 ), 7.0556 ) == 1 );
  QVERIFY( searchCheckError( checkErrors, mLayers["polygon_layer.shp"], 1, QgsPointXY( 0.98690, 0.55699 ), QgsVertexId( 1, 0, 5 ), 7.7351 ) == 1 );
}

void TestQgsGeometryChecks::testAreaCheck()
{
  QList<QgsGeometryCheckError *> checkErrors;
  QStringList messages;

  QgsGeometryAreaCheck( mContext, 0.04 ).collectErrors( checkErrors, messages );
  listErrors( checkErrors, messages );

  QCOMPARE( checkErrors.size(), 3 );
  QVERIFY( searchCheckError( checkErrors, mLayers["point_layer.shp"] ) == 0 );
  QVERIFY( searchCheckError( checkErrors, mLayers["line_layer.shp"] ) == 0 );
  QVERIFY( searchCheckError( checkErrors, mLayers["polygon_layer.shp"], 1, QgsPointXY( 1.0068, 0.3635 ), QgsVertexId(), 0.0105 ) == 1 );
  QVERIFY( searchCheckError( checkErrors, mLayers["polygon_layer.shp"], 2, QgsPointXY( 0.9739, 1.0983 ), QgsVertexId(), 0.0141 ) == 1 );
  QVERIFY( searchCheckError( checkErrors, mLayers["polygon_layer.shp"], 6, QgsPointXY( 0.9968, 1.7584 ), QgsVertexId(), 0 ) == 1 );
}

void TestQgsGeometryChecks::testContainedCheck()
{
  QList<QgsGeometryCheckError *> checkErrors;
  QStringList messages;

  QgsGeometryContainedCheck( mContext ).collectErrors( checkErrors, messages );
  listErrors( checkErrors, messages );

  QCOMPARE( checkErrors.size(), 3 );
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
  QVERIFY( searchCheckError( checkErrors, mLayers["line_layer.shp"], 3, QgsPointXY( 0.07913, 0.8012 ), QgsVertexId( 0, 0, 2 ) ) == 1 );
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
  QVERIFY( searchCheckError( checkErrors, mLayers["polygon_layer.shp"], 6 ) == 1 );
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
