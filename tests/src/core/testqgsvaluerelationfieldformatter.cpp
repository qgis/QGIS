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

#include <memory>

#include "qgsapplication.h"
#include "qgsfieldformatterregistry.h"
#include "qgsproject.h"
#include "qgsrelationmanager.h"
#include "qgstest.h"

#include <QObject>
#include <QString>

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
  mLayer1 = std::make_unique<QgsVectorLayer>( u"LineString?crs=epsg:3111&field=pk:int&field=fk:int"_s, u"vl1"_s, u"memory"_s );
  QgsProject::instance()->addMapLayer( mLayer1.get(), false, false );

  mLayer2 = std::make_unique<QgsVectorLayer>( u"LineString?field=pk:int&field=material:string&field=diameter:int&field=raccord:string"_s, u"vl2"_s, u"memory"_s );
  QgsProject::instance()->addMapLayer( mLayer2.get(), false, false );

  // create relation
  mRelation = std::make_unique<QgsRelation>();
  mRelation->setId( u"vl1.vl2"_s );
  mRelation->setName( u"vl1.vl2"_s );
  mRelation->setReferencingLayer( mLayer1->id() );
  mRelation->setReferencedLayer( mLayer2->id() );
  mRelation->addFieldPair( u"fk"_s, u"pk"_s );
  QVERIFY( mRelation->isValid() );
  QgsProject::instance()->relationManager()->addRelation( *mRelation );

  // add features
  QgsFeature ft0( mLayer1->fields() );
  ft0.setAttribute( u"pk"_s, 0 );
  ft0.setAttribute( u"fk"_s, 0 );
  mLayer1->startEditing();
  mLayer1->addFeature( ft0 );
  mLayer1->commitChanges();

  QgsFeature ft1( mLayer1->fields() );
  ft1.setAttribute( u"pk"_s, 1 );
  ft1.setAttribute( u"fk"_s, 1 );
  mLayer1->startEditing();
  mLayer1->addFeature( ft1 );
  mLayer1->commitChanges();

  QgsFeature ft2( mLayer2->fields() );
  ft2.setAttribute( u"pk"_s, 10 );
  ft2.setAttribute( u"material"_s, "iron" );
  ft2.setAttribute( u"diameter"_s, 120 );
  ft2.setAttribute( u"raccord"_s, "brides" );
  mLayer2->startEditing();
  mLayer2->addFeature( ft2 );
  mLayer2->commitChanges();

  QgsFeature ft3( mLayer2->fields() );
  ft3.setAttribute( u"pk"_s, 11 );
  ft3.setAttribute( u"material"_s, "iron" );
  ft3.setAttribute( u"diameter"_s, 110 );
  ft3.setAttribute( u"raccord"_s, "sleeve" );
  mLayer2->startEditing();
  mLayer2->addFeature( ft3 );
  mLayer2->commitChanges();

  QgsFeature ft4( mLayer2->fields() );
  ft4.setAttribute( u"pk"_s, 12 );
  ft4.setAttribute( u"material"_s, "steel" );
  ft4.setAttribute( u"diameter"_s, 100 );
  ft4.setAttribute( u"raccord"_s, "collar" );
  mLayer2->startEditing();
  mLayer2->addFeature( ft4 );
  mLayer2->commitChanges();
}

void TestQgsValueRelationFieldFormatter::testDependencies()
{
  // Test dependencies

  const QgsEditorWidgetSetup setup { u"ValueRelation"_s, { { u"LayerSource"_s, mLayer2->publicSource() }, { u"LayerProviderName"_s, mLayer2->providerType() }, { u"LayerName"_s, mLayer2->name() }, { u"Layer"_s, mLayer2->id() } } };
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
  config.insert( u"Layer"_s, mLayer2->id() );
  config.insert( u"Key"_s, u"pk"_s );
  config.insert( u"Value"_s, u"material"_s );

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
  config.insert( u"Layer"_s, mLayer2->id() );
  config.insert( u"Key"_s, u"pk"_s );
  config.insert( u"Value"_s, u"raccord"_s );
  config.insert( u"Group"_s, u"material"_s );

  QgsValueRelationFieldFormatter::ValueRelationCache cache = formatter.createCache( config );
  QVERIFY( !cache.isEmpty() );
  QCOMPARE( cache.at( 0 ).group, QVariant( u"iron"_s ) );
  QCOMPARE( cache.at( cache.size() - 1 ).group, QVariant( u"steel"_s ) );
}

void TestQgsValueRelationFieldFormatter::testOrderBy_data()
{
  QTest::addColumn<QString>( "orderBy" );
  QTest::addColumn<QString>( "fieldName" );
  QTest::addColumn<QStringList>( "expectedFirst" );
  QTest::addColumn<QStringList>( "expectedLast" );

  QTest::newRow( "orderByDefault(pk)" ) << QString() << QString() << QStringList { "brides" } << QStringList { "collar" };
  QTest::newRow( "orderByKey(pk)" ) << QString() << u"Key"_s << QStringList { "brides" } << QStringList { "collar" };
  QTest::newRow( "orderByValue(raccord)" ) << QString() << u"Value"_s << QStringList { "brides" } << QStringList { "collar" };
  QTest::newRow( "orderByField(raccord)" ) << u"Field"_s << u"raccord"_s << QStringList { "brides" } << QStringList { "sleeve" };
  QTest::newRow( "orderByField(diameter)" ) << u"Field"_s << u"diameter"_s << QStringList { "collar" } << QStringList { "brides" };

  // material field has two duplicate values (for "iron"), so the ordering here is not well defined. Accept either "brides" OR "sleeve" as first value, as they both have material = "iron" and may be in either order.
  QTest::newRow( "orderByField(material)" ) << u"Field"_s << u"material"_s << QStringList { "brides", "sleeve" } << QStringList { "collar" };
}

void TestQgsValueRelationFieldFormatter::testOrderBy()
{
  QFETCH( QString, orderBy );
  QFETCH( QString, fieldName );
  QFETCH( QStringList, expectedFirst );
  QFETCH( QStringList, expectedLast );

  QVariantMap config;
  config.insert( u"Layer"_s, mLayer2->id() );
  config.insert( u"Key"_s, u"pk"_s );
  config.insert( u"Value"_s, u"raccord"_s );

  if ( !orderBy.isEmpty() )
  {
    config.insert( u"OrderBy%1"_s.arg( orderBy ), true );
  }
  if ( !fieldName.isEmpty() )
  {
    config.insert( u"OrderByFieldName"_s, fieldName );
  }

  // Ascending
  {
    const QgsValueRelationFieldFormatter formatter;
    QgsValueRelationFieldFormatter::ValueRelationCache cache = formatter.createCache( config );
    QVERIFY( !cache.isEmpty() );

    if ( expectedFirst.size() == 1 )
    {
      QCOMPARE( cache.at( 0 ).value, expectedFirst.at( 0 ) );
    }
    else
    {
      QVERIFY( expectedFirst.contains( cache.at( 0 ).value ) );
    }
    if ( expectedLast.size() == 1 )
    {
      QCOMPARE( cache.at( mLayer2->featureCount() - 1 ).value, expectedLast.at( 0 ) );
    }
    else
    {
      QVERIFY( expectedLast.contains( cache.at( mLayer2->featureCount() - 1 ).value ) );
    }
  }

  // Descending
  {
    config.insert( u"OrderByDescending"_s, true );
    const QgsValueRelationFieldFormatter formatter;
    QgsValueRelationFieldFormatter::ValueRelationCache cache = formatter.createCache( config );
    QVERIFY( !cache.isEmpty() );
    if ( expectedLast.size() == 1 )
    {
      QCOMPARE( cache.at( 0 ).value, expectedLast.at( 0 ) );
    }
    else
    {
      QVERIFY( expectedLast.contains( cache.at( 0 ).value ) );
    }
    if ( expectedFirst.size() == 1 )
    {
      QCOMPARE( cache.at( mLayer2->featureCount() - 1 ).value, expectedFirst.at( 0 ) );
    }
    else
    {
      QVERIFY( expectedFirst.contains( cache.at( mLayer2->featureCount() - 1 ).value ) );
    }
  }
}

QGSTEST_MAIN( TestQgsValueRelationFieldFormatter )
#include "testqgsvaluerelationfieldformatter.moc"
