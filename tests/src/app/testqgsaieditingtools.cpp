/***************************************************************************
  testqgsaieditingtools.cpp
  --------------------------
  begin                : July 2026
  copyright            : (C) 2026
***************************************************************************/

#include "ai/tools/qgsaieditingtools.h"
#include "qgsapplication.h"
#include "qgsfeature.h"
#include "qgsgeometry.h"
#include "qgsproject.h"
#include "qgstest.h"
#include "qgsvectorlayer.h"
#include "qgsvectordataprovider.h"

#include <QJsonObject>

using namespace Qt::StringLiterals;

class TestQgsAiEditingTools : public QObject
{
    Q_OBJECT

  private slots:
    void initTestCase();
    void cleanupTestCase();
    void editFeatureGeometryMovesVertexAndRollsBack();
    void editFeatureGeometryRejectsInvalidResult();
};

void TestQgsAiEditingTools::initTestCase()
{
  QgsApplication::initQgis();
}

void TestQgsAiEditingTools::cleanupTestCase()
{
  QgsApplication::exitQgis();
}

void TestQgsAiEditingTools::editFeatureGeometryMovesVertexAndRollsBack()
{
  QgsProject project;
  QgsVectorLayer *layer = new QgsVectorLayer( u"Polygon?crs=EPSG:4326"_s, u"Parcels"_s, u"memory"_s );
  QVERIFY( layer->isValid() );

  QgsFeature feature;
  feature.setGeometry( QgsGeometry::fromWkt( u"Polygon((0 0, 10 0, 10 10, 0 10, 0 0))"_s ) );
  QVERIFY( layer->dataProvider()->addFeatures( QgsFeatureList() << feature ) );
  layer->updateExtents();
  project.addMapLayer( layer );

  QgsFeature storedFeature;
  QVERIFY( layer->getFeatures().nextFeature( storedFeature ) );
  const QgsFeatureId featureId = storedFeature.id();
  const QString originalWkt = storedFeature.geometry().asWkt();

  QgsAiEditFeatureGeometryTool tool( &project );
  QVERIFY( tool.requiresApproval() );
  QCOMPARE( tool.riskLevel(), QgsAiToolRiskLevel::High );

  QJsonObject args;
  args.insert( u"layer_id"_s, layer->id() );
  args.insert( u"feature_id"_s, static_cast<qint64>( featureId ) );
  args.insert( u"operation"_s, u"move_vertex"_s );
  args.insert( u"vertex_index"_s, 1 );
  args.insert( u"x"_s, 12.0 );
  args.insert( u"y"_s, 0.0 );

  const QgsAiToolResult result = tool.execute( args );
  QVERIFY2( result.success, qPrintable( result.errorMessage ) );
  const QString rollbackToken = result.output.toObject().value( u"rollback_token"_s ).toString();
  QVERIFY( !rollbackToken.isEmpty() );
  QVERIFY( result.output.toObject().contains( u"diff"_s ) );

  QgsFeature editedFeature;
  QVERIFY( layer->getFeatures( QgsFeatureRequest().setFilterFid( featureId ) ).nextFeature( editedFeature ) );
  QCOMPARE( editedFeature.geometry().vertexAt( 1 ).x(), 12.0 );
  QCOMPARE( editedFeature.geometry().vertexAt( 1 ).y(), 0.0 );

  QJsonObject rollbackArgs;
  rollbackArgs.insert( u"rollback_token"_s, rollbackToken );
  const QgsAiToolResult rollback = tool.execute( rollbackArgs );
  QVERIFY2( rollback.success, qPrintable( rollback.errorMessage ) );

  QgsFeature rolledBackFeature;
  QVERIFY( layer->getFeatures( QgsFeatureRequest().setFilterFid( featureId ) ).nextFeature( rolledBackFeature ) );
  QCOMPARE( rolledBackFeature.geometry().asWkt(), originalWkt );
}

void TestQgsAiEditingTools::editFeatureGeometryRejectsInvalidResult()
{
  QgsProject project;
  QgsVectorLayer *layer = new QgsVectorLayer( u"Polygon?crs=EPSG:4326"_s, u"Parcels"_s, u"memory"_s );
  QVERIFY( layer->isValid() );

  QgsFeature feature;
  feature.setGeometry( QgsGeometry::fromWkt( u"Polygon((0 0, 10 0, 10 10, 0 10, 0 0))"_s ) );
  QVERIFY( layer->dataProvider()->addFeatures( QgsFeatureList() << feature ) );
  layer->updateExtents();
  project.addMapLayer( layer );

  QgsFeature storedFeature;
  QVERIFY( layer->getFeatures().nextFeature( storedFeature ) );
  const QgsFeatureId featureId = storedFeature.id();
  const QString originalWkt = storedFeature.geometry().asWkt();

  QgsAiEditFeatureGeometryTool tool( &project );
  QJsonObject args;
  args.insert( u"layer_id"_s, layer->id() );
  args.insert( u"feature_id"_s, static_cast<qint64>( featureId ) );
  args.insert( u"operation"_s, u"insert_vertex"_s );
  args.insert( u"before_vertex"_s, 2 );
  args.insert( u"x"_s, 0.0 );
  args.insert( u"y"_s, 10.0 );

  const QgsAiToolResult result = tool.execute( args );
  QVERIFY( !result.success );
  QVERIFY( result.errorMessage.contains( u"invalid geometry"_s ) );

  QgsFeature unchangedFeature;
  QVERIFY( layer->getFeatures( QgsFeatureRequest().setFilterFid( featureId ) ).nextFeature( unchangedFeature ) );
  QCOMPARE( unchangedFeature.geometry().asWkt(), originalWkt );
}

QGSTEST_MAIN( TestQgsAiEditingTools )
#include "testqgsaieditingtools.moc"
