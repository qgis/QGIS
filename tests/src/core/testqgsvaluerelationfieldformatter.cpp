/***************************************************************************
     testqgsvaluerelationfieldformatter.cpp
     --------------------------------------
    Date                 : 13/12/2019
    Copyright            : (C) 2019 by Alessandro Pasotti
    Email                : elpaso at itopen dot it
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <QObject>
#include <QString>

#include "qgstest.h"
#include "qgsapplication.h"
#include "qgsproject.h"
#include "qgsrelationmanager.h"
#include "qgsfieldformatterregistry.h"


//header for class being tested
#include "fieldformatter/qgsvaluerelationfieldformatter.h"

class TestQgsValueRelationFieldFormatter : public QObject
{
    Q_OBJECT

  private slots:

    void initTestCase();    // will be called before the first testfunction is executed.
    void cleanupTestCase(); // will be called after the last testfunction was executed.
    void init();            // will be called before each testfunction is executed.
    void cleanup();         // will be called after every testfunction.
    void testDependencies();
    void testSortValueNull();
    void testGroup();
    void testOrderBy_data();
    void testOrderBy();

  private:
    std::unique_ptr<QgsVectorLayer> mLayer1;
    std::unique_ptr<QgsVectorLayer> mLayer2;
    std::unique_ptr<QgsRelation> mRelation;
};


void TestQgsValueRelationFieldFormatter::initTestCase()
{
  QgsApplication::init();
  QgsApplication::initQgis();
}

void TestQgsValueRelationFieldFormatter::cleanupTestCase()
{
  QgsApplication::exitQgis();
}

void TestQgsValueRelationFieldFormatter::cleanup()
{
  QgsProject::instance()->removeMapLayer( mLayer1.get() );
  QgsProject::instance()->removeMapLayer( mLayer2.get() );
}


void TestQgsValueRelationFieldFormatter::init()
{
  // create layer
  mLayer1.reset( new QgsVectorLayer( QStringLiteral( "LineString?crs=epsg:3111&field=pk:int&field=fk:int" ), QStringLiteral( "vl1" ), QStringLiteral( "memory" ) ) );
  QgsProject::instance()->addMapLayer( mLayer1.get(), false, false );

  mLayer2.reset( new QgsVectorLayer( QStringLiteral( "LineString?field=pk:int&field=material:string&field=diameter:int&field=raccord:string" ), QStringLiteral( "vl2" ), QStringLiteral( "memory" ) ) );
  QgsProject::instance()->addMapLayer( mLayer2.get(), false, false );

  // create relation
  mRelation.reset( new QgsRelation() );
  mRelation->setId( QStringLiteral( "vl1.vl2" ) );
  mRelation->setName( QStringLiteral( "vl1.vl2" ) );
  mRelation->setReferencingLayer( mLayer1->id() );
  mRelation->setReferencedLayer( mLayer2->id() );
  mRelation->addFieldPair( QStringLiteral( "fk" ), QStringLiteral( "pk" ) );
  QVERIFY( mRelation->isValid() );
  QgsProject::instance()->relationManager()->addRelation( *mRelation );

  // add features
  QgsFeature ft0( mLayer1->fields() );
  ft0.setAttribute( QStringLiteral( "pk" ), 0 );
  ft0.setAttribute( QStringLiteral( "fk" ), 0 );
  mLayer1->startEditing();
  mLayer1->addFeature( ft0 );
  mLayer1->commitChanges();

  QgsFeature ft1( mLayer1->fields() );
  ft1.setAttribute( QStringLiteral( "pk" ), 1 );
  ft1.setAttribute( QStringLiteral( "fk" ), 1 );
  mLayer1->startEditing();
  mLayer1->addFeature( ft1 );
  mLayer1->commitChanges();

  QgsFeature ft2( mLayer2->fields() );
  ft2.setAttribute( QStringLiteral( "pk" ), 10 );
  ft2.setAttribute( QStringLiteral( "material" ), "iron" );
  ft2.setAttribute( QStringLiteral( "diameter" ), 120 );
  ft2.setAttribute( QStringLiteral( "raccord" ), "brides" );
  mLayer2->startEditing();
  mLayer2->addFeature( ft2 );
  mLayer2->commitChanges();

  QgsFeature ft3( mLayer2->fields() );
  ft3.setAttribute( QStringLiteral( "pk" ), 11 );
  ft3.setAttribute( QStringLiteral( "material" ), "iron" );
  ft3.setAttribute( QStringLiteral( "diameter" ), 110 );
  ft3.setAttribute( QStringLiteral( "raccord" ), "sleeve" );
  mLayer2->startEditing();
  mLayer2->addFeature( ft3 );
  mLayer2->commitChanges();

  QgsFeature ft4( mLayer2->fields() );
  ft4.setAttribute( QStringLiteral( "pk" ), 12 );
  ft4.setAttribute( QStringLiteral( "material" ), "steel" );
  ft4.setAttribute( QStringLiteral( "diameter" ), 100 );
  ft4.setAttribute( QStringLiteral( "raccord" ), "collar" );
  mLayer2->startEditing();
  mLayer2->addFeature( ft4 );
  mLayer2->commitChanges();
}

void TestQgsValueRelationFieldFormatter::testDependencies()
{
  // Test dependencies

  const QgsEditorWidgetSetup setup { QStringLiteral( "ValueRelation" ), { { QStringLiteral( "LayerSource" ), mLayer2->publicSource() }, { QStringLiteral( "LayerProviderName" ), mLayer2->providerType() }, { QStringLiteral( "LayerName" ), mLayer2->name() }, { QStringLiteral( "Layer" ), mLayer2->id() } } };
  QgsFieldFormatter *fieldFormatter = QgsApplication::fieldFormatterRegistry()->fieldFormatter( setup.type() );
  const QList<QgsVectorLayerRef> dependencies = fieldFormatter->layerDependencies( setup.config() );
  QVERIFY( dependencies.count() == 1 );
  const QgsVectorLayerRef dependency = dependencies.first();
  QCOMPARE( dependency.layerId, mLayer2->id() );
  QCOMPARE( dependency.name, mLayer2->name() );
  QCOMPARE( dependency.provider, mLayer2->providerType() );
  QCOMPARE( dependency.source, mLayer2->publicSource() );
}

void TestQgsValueRelationFieldFormatter::testSortValueNull()
{
  const QgsValueRelationFieldFormatter formatter;
  QVariantMap config;
  config.insert( QStringLiteral( "Layer" ), mLayer2->id() );
  config.insert( QStringLiteral( "Key" ), QStringLiteral( "pk" ) );
  config.insert( QStringLiteral( "Value" ), QStringLiteral( "material" ) );

  // when sorting, a null value is represented with a null QString, not "NULL" string
  // if not, the NULL values will take place between M and O (see https://github.com/qgis/QGIS/issues/36114)
  QVariant value = formatter.sortValue( mLayer2.get(), 1, config, QVariant(), QVariant() );
  QCOMPARE( value, QVariant( QString() ) );

  value = formatter.sortValue( mLayer2.get(), 1, config, QVariant(), QVariant( 10 ) );
  QCOMPARE( value, QVariant( QString( "iron" ) ) );
}

void TestQgsValueRelationFieldFormatter::testGroup()
{
  const QgsValueRelationFieldFormatter formatter;
  QVariantMap config;
  config.insert( QStringLiteral( "Layer" ), mLayer2->id() );
  config.insert( QStringLiteral( "Key" ), QStringLiteral( "pk" ) );
  config.insert( QStringLiteral( "Value" ), QStringLiteral( "raccord" ) );
  config.insert( QStringLiteral( "Group" ), QStringLiteral( "material" ) );

  QgsValueRelationFieldFormatter::ValueRelationCache cache = formatter.createCache( config );
  QVERIFY( !cache.isEmpty() );
  QCOMPARE( cache.at( 0 ).group, QVariant( QStringLiteral( "iron" ) ) );
  QCOMPARE( cache.at( cache.size() - 1 ).group, QVariant( QStringLiteral( "steel" ) ) );
}

void TestQgsValueRelationFieldFormatter::testOrderBy_data()
{
  QTest::addColumn<QString>( "orderBy" );
  QTest::addColumn<QString>( "fieldName" );
  QTest::addColumn<QString>( "expectedFirst" );
  QTest::addColumn<QString>( "expectedLast" );

  QTest::newRow( "orderByDefault(pk)" ) << QString() << QString() << "brides" << "collar";
  QTest::newRow( "orderByKey(pk)" ) << QString() << QStringLiteral( "Key" ) << "brides" << "collar";
  QTest::newRow( "orderByValue(raccord)" ) << QString() << QStringLiteral( "Value" ) << "brides" << "collar";
  QTest::newRow( "orderByField(raccord)" ) << QStringLiteral( "Field" ) << QStringLiteral( "raccord" ) << "brides" << "sleeve";
  QTest::newRow( "orderByField(diameter)" ) << QStringLiteral( "Field" ) << QStringLiteral( "diameter" ) << "collar" << "brides";
  QTest::newRow( "orderByField(material)" ) << QStringLiteral( "Field" ) << QStringLiteral( "material" ) << "brides" << "collar";
}

void TestQgsValueRelationFieldFormatter::testOrderBy()
{
  QFETCH( QString, orderBy );
  QFETCH( QString, fieldName );
  QFETCH( QString, expectedFirst );
  QFETCH( QString, expectedLast );

  QVariantMap config;
  config.insert( QStringLiteral( "Layer" ), mLayer2->id() );
  config.insert( QStringLiteral( "Key" ), QStringLiteral( "pk" ) );
  config.insert( QStringLiteral( "Value" ), QStringLiteral( "raccord" ) );

  if ( !orderBy.isEmpty() )
  {
    config.insert( QStringLiteral( "OrderBy%1" ).arg( orderBy ), true );
  }
  if ( !fieldName.isEmpty() )
  {
    config.insert( QStringLiteral( "OrderByFieldName" ), fieldName );
  }

  // Ascending
  {
    const QgsValueRelationFieldFormatter formatter;
    QgsValueRelationFieldFormatter::ValueRelationCache cache = formatter.createCache( config );
    QVERIFY( !cache.isEmpty() );
    QCOMPARE( cache.at( 0 ).value, expectedFirst );
    QCOMPARE( cache.at( mLayer2->featureCount() - 1 ).value, expectedLast );
  }

  // Descending
  {
    config.insert( QStringLiteral( "OrderByDescending" ), true );
    const QgsValueRelationFieldFormatter formatter;
    QgsValueRelationFieldFormatter::ValueRelationCache cache = formatter.createCache( config );
    QVERIFY( !cache.isEmpty() );
    QCOMPARE( cache.at( 0 ).value, expectedLast );
    QCOMPARE( cache.at( mLayer2->featureCount() - 1 ).value, expectedFirst );
  }
}

QGSTEST_MAIN( TestQgsValueRelationFieldFormatter )
#include "testqgsvaluerelationfieldformatter.moc"
