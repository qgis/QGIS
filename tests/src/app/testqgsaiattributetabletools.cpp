/***************************************************************************
  testqgsaiattributetabletools.cpp
  --------------------------
  begin                : July 2026
  copyright            : (C) 2026
***************************************************************************/

#include "ai/tools/qgsaiattributetabletools.h"
#include "qgsapplication.h"
#include "qgsfeature.h"
#include "qgsgeometry.h"
#include "qgsproject.h"
#include "qgstest.h"
#include "qgsvectordataprovider.h"
#include "qgsvectorlayer.h"

#include <QJsonArray>
#include <QJsonObject>
#include <QString>

using namespace Qt::StringLiterals;

class TestQgsAiAttributeTableTools : public QObject
{
    Q_OBJECT

  private slots:
    void initTestCase();
    void cleanupTestCase();
    void queryFeaturesFiltersAndPaginates();
    void batchUpdateAttributesUpdatesAndRollsBack();
    void selectFeaturesUpdatesLayerSelection();
    void identifyFeaturesAtReturnsMatchingFeature();
};

void TestQgsAiAttributeTableTools::initTestCase()
{
  QgsApplication::initQgis();
}

void TestQgsAiAttributeTableTools::cleanupTestCase()
{
  QgsApplication::exitQgis();
}

static QgsVectorLayer *makePlacesLayer( QgsProject &project )
{
  QgsVectorLayer *layer = new QgsVectorLayer( u"Point?crs=EPSG:4326&field=name:string&field=value:integer"_s, u"Places"_s, u"memory"_s );
  Q_ASSERT( layer->isValid() );

  QgsFeature first( layer->fields() );
  first.setGeometry( QgsGeometry::fromWkt( u"Point(1 1)"_s ) );
  first.setAttribute( u"name"_s, u"one"_s );
  first.setAttribute( u"value"_s, 1 );
  QgsFeature second( layer->fields() );
  second.setGeometry( QgsGeometry::fromWkt( u"Point(2 2)"_s ) );
  second.setAttribute( u"name"_s, u"two"_s );
  second.setAttribute( u"value"_s, 2 );
  QgsFeature third( layer->fields() );
  third.setGeometry( QgsGeometry::fromWkt( u"Point(3 3)"_s ) );
  third.setAttribute( u"name"_s, u"three"_s );
  third.setAttribute( u"value"_s, 3 );
  layer->dataProvider()->addFeatures( QgsFeatureList() << first << second << third );
  project.addMapLayer( layer );
  return layer;
}

void TestQgsAiAttributeTableTools::queryFeaturesFiltersAndPaginates()
{
  QgsProject project;
  QgsVectorLayer *layer = makePlacesLayer( project );
  QVERIFY( layer );

  QgsAiQueryFeaturesTool tool( &project );
  QVERIFY( !tool.requiresApproval() );

  QJsonObject args;
  args.insert( u"layer_id"_s, layer->id() );
  args.insert( u"filter_expression"_s, u"\"value\" >= 2"_s );
  args.insert( u"offset"_s, 1 );
  args.insert( u"limit"_s, 1 );

  const QgsAiToolResult result = tool.execute( args );
  QVERIFY2( result.success, qPrintable( result.errorMessage ) );
  const QJsonObject output = result.output.toObject();
  QCOMPARE( output.value( u"feature_count"_s ).toInt(), 2 );
  const QJsonArray features = output.value( u"features"_s ).toArray();
  QCOMPARE( features.size(), 1 );
  QCOMPARE( features.at( 0 ).toObject().value( u"attributes"_s ).toObject().value( u"name"_s ).toString(), u"three"_s );
}

void TestQgsAiAttributeTableTools::batchUpdateAttributesUpdatesAndRollsBack()
{
  QgsProject project;
  QgsVectorLayer *layer = makePlacesLayer( project );
  QVERIFY( layer );

  QgsAiBatchUpdateAttributesTool tool( &project );
  QVERIFY( tool.requiresApproval() );
  QCOMPARE( tool.riskLevel(), QgsAiToolRiskLevel::High );

  QJsonObject args;
  args.insert( u"layer_id"_s, layer->id() );
  args.insert( u"filter_expression"_s, u"\"value\" >= 2"_s );
  args.insert( u"field_name"_s, u"name"_s );
  args.insert( u"value"_s, u"updated"_s );

  const QgsAiToolResult result = tool.execute( args );
  QVERIFY2( result.success, qPrintable( result.errorMessage ) );
  QCOMPARE( result.output.toObject().value( u"updated_feature_count"_s ).toInt(), 2 );
  const QString rollbackToken = result.output.toObject().value( u"rollback_token"_s ).toString();
  QVERIFY( !rollbackToken.isEmpty() );

  QgsFeatureIterator it = layer->getFeatures();
  QgsFeature feature;
  int updatedCount = 0;
  while ( it.nextFeature( feature ) )
  {
    if ( feature.attribute( u"value"_s ).toInt() >= 2 )
    {
      QCOMPARE( feature.attribute( u"name"_s ).toString(), u"updated"_s );
      updatedCount++;
    }
  }
  QCOMPARE( updatedCount, 2 );

  QJsonObject rollbackArgs;
  rollbackArgs.insert( u"rollback_token"_s, rollbackToken );
  const QgsAiToolResult rollback = tool.execute( rollbackArgs );
  QVERIFY2( rollback.success, qPrintable( rollback.errorMessage ) );

  QJsonObject queryArgs;
  queryArgs.insert( u"layer_id"_s, layer->id() );
  queryArgs.insert( u"filter_expression"_s, u"\"value\" >= 2"_s );
  QgsAiQueryFeaturesTool queryTool( &project );
  const QgsAiToolResult query = queryTool.execute( queryArgs );
  QVERIFY2( query.success, qPrintable( query.errorMessage ) );
  const QJsonArray features = query.output.toObject().value( u"features"_s ).toArray();
  QCOMPARE( features.at( 0 ).toObject().value( u"attributes"_s ).toObject().value( u"name"_s ).toString(), u"two"_s );
  QCOMPARE( features.at( 1 ).toObject().value( u"attributes"_s ).toObject().value( u"name"_s ).toString(), u"three"_s );
}

void TestQgsAiAttributeTableTools::selectFeaturesUpdatesLayerSelection()
{
  QgsProject project;
  QgsVectorLayer *layer = makePlacesLayer( project );
  QVERIFY( layer );

  QgsAiSelectFeaturesTool tool( &project );
  QVERIFY( tool.requiresApproval() );
  QCOMPARE( tool.riskLevel(), QgsAiToolRiskLevel::Low );

  QJsonObject args;
  args.insert( u"layer_id"_s, layer->id() );
  args.insert( u"mode"_s, u"replace"_s );
  args.insert( u"filter_expression"_s, u"\"value\" >= 2"_s );
  const QgsAiToolResult result = tool.execute( args );
  QVERIFY2( result.success, qPrintable( result.errorMessage ) );
  QCOMPARE( result.output.toObject().value( u"selected_count"_s ).toInt(), 2 );
  QCOMPARE( layer->selectedFeatureIds().size(), 2 );

  QJsonObject emptyArgs;
  emptyArgs.insert( u"layer_id"_s, layer->id() );
  emptyArgs.insert( u"mode"_s, u"replace"_s );
  emptyArgs.insert( u"filter_expression"_s, u"\"value\" > 100"_s );
  const QgsAiToolResult empty = tool.execute( emptyArgs );
  QVERIFY2( empty.success, qPrintable( empty.errorMessage ) );
  QCOMPARE( empty.output.toObject().value( u"selected_count"_s ).toInt(), 0 );
  QCOMPARE( layer->selectedFeatureIds().size(), 0 );
}

void TestQgsAiAttributeTableTools::identifyFeaturesAtReturnsMatchingFeature()
{
  QgsProject project;
  QgsVectorLayer *layer = makePlacesLayer( project );
  QVERIFY( layer );

  QgsAiIdentifyFeaturesAtTool tool( &project );
  QVERIFY( !tool.requiresApproval() );

  QJsonObject args;
  args.insert( u"layer_id"_s, layer->id() );
  args.insert( u"x"_s, 2.0 );
  args.insert( u"y"_s, 2.0 );
  args.insert( u"crs"_s, u"EPSG:4326"_s );
  args.insert( u"tolerance"_s, 0.05 );
  const QgsAiToolResult result = tool.execute( args );
  QVERIFY2( result.success, qPrintable( result.errorMessage ) );
  const QJsonArray features = result.output.toObject().value( u"features"_s ).toArray();
  QCOMPARE( features.size(), 1 );
  QCOMPARE( features.at( 0 ).toObject().value( u"attributes"_s ).toObject().value( u"name"_s ).toString(), u"two"_s );
  QCOMPARE( features.at( 0 ).toObject().value( u"attributes"_s ).toObject().value( u"value"_s ).toInt(), 2 );
}

QGSTEST_MAIN( TestQgsAiAttributeTableTools )
#include "testqgsaiattributetabletools.moc"
