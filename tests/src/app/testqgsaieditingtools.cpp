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
    void updateFeatureAttributesUpdatesAndRollsBack();
    void updateFeatureAttributesRejectsIncompatibleType();
    void calculateFieldCreatesFieldForFilteredFeaturesAndRollsBack();
    void calculateFieldRejectsInvalidExpression();
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

void TestQgsAiEditingTools::updateFeatureAttributesUpdatesAndRollsBack()
{
  QgsProject project;
  QgsVectorLayer *layer = new QgsVectorLayer( u"Point?crs=EPSG:4326&field=name:string&field=height:double"_s, u"Places"_s, u"memory"_s );
  QVERIFY( layer->isValid() );

  QgsFeature feature( layer->fields() );
  feature.setGeometry( QgsGeometry::fromWkt( u"Point(1 2)"_s ) );
  feature.setAttribute( u"name"_s, u"old"_s );
  feature.setAttribute( u"height"_s, 1.5 );
  QVERIFY( layer->dataProvider()->addFeatures( QgsFeatureList() << feature ) );
  project.addMapLayer( layer );

  QgsFeature storedFeature;
  QVERIFY( layer->getFeatures().nextFeature( storedFeature ) );
  const QgsFeatureId featureId = storedFeature.id();

  QgsAiUpdateFeatureAttributesTool tool( &project );
  QVERIFY( tool.requiresApproval() );
  QCOMPARE( tool.riskLevel(), QgsAiToolRiskLevel::High );

  QJsonObject attributes;
  attributes.insert( u"name"_s, u"new"_s );
  attributes.insert( u"height"_s, 7.25 );

  QJsonObject args;
  args.insert( u"layer_id"_s, layer->id() );
  args.insert( u"feature_id"_s, static_cast<qint64>( featureId ) );
  args.insert( u"attributes"_s, attributes );

  const QgsAiToolResult result = tool.execute( args );
  QVERIFY2( result.success, qPrintable( result.errorMessage ) );
  const QString rollbackToken = result.output.toObject().value( u"rollback_token"_s ).toString();
  QVERIFY( !rollbackToken.isEmpty() );

  QgsFeature editedFeature;
  QVERIFY( layer->getFeatures( QgsFeatureRequest().setFilterFid( featureId ) ).nextFeature( editedFeature ) );
  QCOMPARE( editedFeature.attribute( u"name"_s ).toString(), u"new"_s );
  QCOMPARE( editedFeature.attribute( u"height"_s ).toDouble(), 7.25 );

  QJsonObject rollbackArgs;
  rollbackArgs.insert( u"rollback_token"_s, rollbackToken );
  const QgsAiToolResult rollback = tool.execute( rollbackArgs );
  QVERIFY2( rollback.success, qPrintable( rollback.errorMessage ) );

  QgsFeature rolledBackFeature;
  QVERIFY( layer->getFeatures( QgsFeatureRequest().setFilterFid( featureId ) ).nextFeature( rolledBackFeature ) );
  QCOMPARE( rolledBackFeature.attribute( u"name"_s ).toString(), u"old"_s );
  QCOMPARE( rolledBackFeature.attribute( u"height"_s ).toDouble(), 1.5 );
}

void TestQgsAiEditingTools::updateFeatureAttributesRejectsIncompatibleType()
{
  QgsProject project;
  QgsVectorLayer *layer = new QgsVectorLayer( u"Point?crs=EPSG:4326&field=name:string&field=height:double"_s, u"Places"_s, u"memory"_s );
  QVERIFY( layer->isValid() );

  QgsFeature feature( layer->fields() );
  feature.setGeometry( QgsGeometry::fromWkt( u"Point(1 2)"_s ) );
  feature.setAttribute( u"name"_s, u"old"_s );
  feature.setAttribute( u"height"_s, 1.5 );
  QVERIFY( layer->dataProvider()->addFeatures( QgsFeatureList() << feature ) );
  project.addMapLayer( layer );

  QgsFeature storedFeature;
  QVERIFY( layer->getFeatures().nextFeature( storedFeature ) );
  const QgsFeatureId featureId = storedFeature.id();

  QgsAiUpdateFeatureAttributesTool tool( &project );
  QJsonObject attributes;
  attributes.insert( u"height"_s, u"not-a-number"_s );

  QJsonObject args;
  args.insert( u"layer_id"_s, layer->id() );
  args.insert( u"feature_id"_s, static_cast<qint64>( featureId ) );
  args.insert( u"attributes"_s, attributes );

  const QgsAiToolResult result = tool.execute( args );
  QVERIFY( !result.success );
  QVERIFY( result.errorMessage.contains( u"height"_s ) );

  QgsFeature unchangedFeature;
  QVERIFY( layer->getFeatures( QgsFeatureRequest().setFilterFid( featureId ) ).nextFeature( unchangedFeature ) );
  QCOMPARE( unchangedFeature.attribute( u"height"_s ).toDouble(), 1.5 );
}

void TestQgsAiEditingTools::calculateFieldCreatesFieldForFilteredFeaturesAndRollsBack()
{
  QgsProject project;
  QgsVectorLayer *layer = new QgsVectorLayer( u"Point?crs=EPSG:4326&field=value:double"_s, u"Places"_s, u"memory"_s );
  QVERIFY( layer->isValid() );

  QgsFeature first( layer->fields() );
  first.setGeometry( QgsGeometry::fromWkt( u"Point(1 1)"_s ) );
  first.setAttribute( u"value"_s, 2.0 );
  QgsFeature second( layer->fields() );
  second.setGeometry( QgsGeometry::fromWkt( u"Point(2 2)"_s ) );
  second.setAttribute( u"value"_s, 3.0 );
  QVERIFY( layer->dataProvider()->addFeatures( QgsFeatureList() << first << second ) );
  project.addMapLayer( layer );

  QgsAiCalculateFieldTool tool( &project );
  QVERIFY( tool.requiresApproval() );
  QCOMPARE( tool.riskLevel(), QgsAiToolRiskLevel::High );

  QJsonObject args;
  args.insert( u"layer_id"_s, layer->id() );
  args.insert( u"field_name"_s, u"double_value"_s );
  args.insert( u"expression"_s, u"\"value\" * 2"_s );
  args.insert( u"create_field"_s, true );
  args.insert( u"field_type"_s, u"double"_s );
  args.insert( u"filter_expression"_s, u"\"value\" > 2"_s );

  const QgsAiToolResult result = tool.execute( args );
  QVERIFY2( result.success, qPrintable( result.errorMessage ) );
  QCOMPARE( result.output.toObject().value( u"updated_feature_count"_s ).toInt(), 1 );
  const QString rollbackToken = result.output.toObject().value( u"rollback_token"_s ).toString();
  QVERIFY( !rollbackToken.isEmpty() );

  const int calculatedIndex = layer->fields().lookupField( u"double_value"_s );
  QVERIFY( calculatedIndex >= 0 );

  QgsFeatureIterator it = layer->getFeatures();
  QgsFeature feature;
  int updatedCount = 0;
  while ( it.nextFeature( feature ) )
  {
    if ( feature.attribute( u"value"_s ).toDouble() > 2.0 )
    {
      QCOMPARE( feature.attribute( calculatedIndex ).toDouble(), 6.0 );
      updatedCount++;
    }
    else
    {
      QVERIFY( feature.attribute( calculatedIndex ).isNull() );
    }
  }
  QCOMPARE( updatedCount, 1 );

  QJsonObject rollbackArgs;
  rollbackArgs.insert( u"rollback_token"_s, rollbackToken );
  const QgsAiToolResult rollback = tool.execute( rollbackArgs );
  QVERIFY2( rollback.success, qPrintable( rollback.errorMessage ) );
  QCOMPARE( layer->fields().lookupField( u"double_value"_s ), -1 );
}

void TestQgsAiEditingTools::calculateFieldRejectsInvalidExpression()
{
  QgsProject project;
  QgsVectorLayer *layer = new QgsVectorLayer( u"Point?crs=EPSG:4326&field=value:double"_s, u"Places"_s, u"memory"_s );
  QVERIFY( layer->isValid() );

  QgsFeature feature( layer->fields() );
  feature.setGeometry( QgsGeometry::fromWkt( u"Point(1 1)"_s ) );
  feature.setAttribute( u"value"_s, 2.0 );
  QVERIFY( layer->dataProvider()->addFeatures( QgsFeatureList() << feature ) );
  project.addMapLayer( layer );

  QgsAiCalculateFieldTool tool( &project );
  QJsonObject args;
  args.insert( u"layer_id"_s, layer->id() );
  args.insert( u"field_name"_s, u"broken"_s );
  args.insert( u"expression"_s, u"\"value\" *"_s );
  args.insert( u"create_field"_s, true );

  const QgsAiToolResult result = tool.execute( args );
  QVERIFY( !result.success );
  QVERIFY( result.errorMessage.contains( u"parser error"_s ) );
  QCOMPARE( layer->fields().lookupField( u"broken"_s ), -1 );
}

QGSTEST_MAIN( TestQgsAiEditingTools )
#include "testqgsaieditingtools.moc"
