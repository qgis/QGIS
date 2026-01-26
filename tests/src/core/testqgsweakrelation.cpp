/***************************************************************************
     testqgsweakrelation.cpp
     ----------------
    Date                 : December 2019
    Copyright            : (C) 2019 Alessandro Pasotti
    Email                : elpaso at itopen dot it
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsapplication.h"
#include "qgsproject.h"
#include "qgsrelation.h"
#include "qgsrelationmanager.h"
#include "qgstest.h"
#include "qgsvectorlayer.h"
#include "qgsweakrelation.h"

class TestQgsWeakRelation : public QObject
{
    Q_OBJECT

  private slots:
    void initTestCase();    // will be called before the first testfunction is executed.
    void cleanupTestCase(); // will be called after the last testfunction was executed.
    void init();            // will be called before each testfunction is executed.
    void cleanup();         // will be called after every testfunction.

    void testSetters();

    void testResolved(); // Test if relation can be resolved
    void testResolvedManyToMany();
    void testReadWrite(); // Test if relation can be read and write

    void testWriteStyleCategoryRelations(); // Test if weak relation attrs are written well in the style "relations"

  private:
};

void TestQgsWeakRelation::initTestCase()
{
  // Set up the QgsSettings environment
  QCoreApplication::setOrganizationName( u"QGIS"_s );
  QCoreApplication::setOrganizationDomain( u"qgis.org"_s );
  QCoreApplication::setApplicationName( u"QGIS-TEST"_s );
  QgsApplication::init();
  QgsApplication::initQgis();
}

void TestQgsWeakRelation::cleanupTestCase()
{
  QgsApplication::exitQgis();
}

void TestQgsWeakRelation::init()
{
  QLocale::setDefault( QLocale::English );
}

void TestQgsWeakRelation::cleanup()
{
  QLocale::setDefault( QLocale::English );
}

void TestQgsWeakRelation::testSetters()
{
  QgsWeakRelation weakRel;
  QCOMPARE( weakRel.referencedLayerSource(), QString() );
  QCOMPARE( weakRel.referencedLayerProvider(), QString() );
  weakRel.setReferencedLayer( u"referenced_source"_s, u"referenced_provider"_s );
  QCOMPARE( weakRel.referencedLayerSource(), u"referenced_source"_s );
  QCOMPARE( weakRel.referencedLayerProvider(), u"referenced_provider"_s );

  QCOMPARE( weakRel.referencingLayerSource(), QString() );
  QCOMPARE( weakRel.referencingLayerProvider(), QString() );
  weakRel.setReferencingLayer( u"referencing_source"_s, u"referencing_provider"_s );
  QCOMPARE( weakRel.referencingLayerSource(), u"referencing_source"_s );
  QCOMPARE( weakRel.referencingLayerProvider(), u"referencing_provider"_s );

  QCOMPARE( weakRel.mappingTableSource(), QString() );
  QCOMPARE( weakRel.mappingTableProvider(), QString() );
  weakRel.setMappingTable( u"mapping_source"_s, u"mapping_provider"_s );
  QCOMPARE( weakRel.mappingTableSource(), u"mapping_source"_s );
  QCOMPARE( weakRel.mappingTableProvider(), u"mapping_provider"_s );
}

void TestQgsWeakRelation::testResolved()
{
  QgsWeakRelation weakRel( u"my_relation_id"_s, u"my_relation_name"_s, Qgis::RelationshipStrength::Association, u"referencingLayerId"_s, u"referencingLayerName"_s, u"Point?crs=epsg:4326&field=pk:int&field=fk_province:int&field=fk_municipality:int"_s, u"memory"_s, u"referencedLayerId"_s, u"referencedLayerName"_s, u"Polygon?crs=epsg:4326&field=pk:int&field=province:int&field=municipality:string"_s, u"memory"_s );
  weakRel.setReferencingLayerFields( { "fk_province" } );
  weakRel.setReferencedLayerFields( { "pk" } );

  QList<QgsRelation> res = weakRel.resolvedRelations( QgsProject::instance(), QgsVectorLayerRef::MatchType::Name );
  QCOMPARE( res.size(), 1 );
  QVERIFY( !res.at( 0 ).isValid() );

  // create a vector layer
  QgsVectorLayer referencedLayer( u"Polygon?crs=epsg:4326&field=pk:int&field=province:int&field=municipality:string"_s, u"referencedLayerName"_s, u"memory"_s );
  QgsProject::instance()->addMapLayer( &referencedLayer, false, false );
  res = weakRel.resolvedRelations( QgsProject::instance(), QgsVectorLayerRef::MatchType::Name );
  QCOMPARE( res.size(), 1 );
  QVERIFY( !res.at( 0 ).isValid() );

  QgsVectorLayer referencingLayer( u"Point?crs=epsg:4326&field=pk:int&field=fk_province:int&field=fk_municipality:int"_s, u"referencingLayerName"_s, u"memory"_s );
  QgsProject::instance()->addMapLayer( &referencingLayer, false, false );
  res = weakRel.resolvedRelations( QgsProject::instance(), QgsVectorLayerRef::MatchType::Name );
  QCOMPARE( res.size(), 1 );
  QVERIFY( res.at( 0 ).isValid() );

  res = weakRel.resolvedRelations( QgsProject::instance(), static_cast<QgsVectorLayerRef::MatchType>( QgsVectorLayerRef::MatchType::Name | QgsVectorLayerRef::MatchType::Provider ) );
  QCOMPARE( res.size(), 1 );
  QVERIFY( res.at( 0 ).isValid() );

  // This fails because memory provider stores an UUID in the data source definition ...
  res = weakRel.resolvedRelations( QgsProject::instance(), static_cast<QgsVectorLayerRef::MatchType>( QgsVectorLayerRef::MatchType::Name | QgsVectorLayerRef::MatchType::Source ) );
  QCOMPARE( res.size(), 1 );
  QVERIFY( !res.at( 0 ).isValid() );

  // ... let's fix it
  weakRel.mReferencedLayer.source = referencedLayer.publicSource();
  weakRel.mReferencingLayer.source = referencingLayer.publicSource();
  res = weakRel.resolvedRelations( QgsProject::instance(), static_cast<QgsVectorLayerRef::MatchType>( QgsVectorLayerRef::MatchType::Name | QgsVectorLayerRef::MatchType::Source ) );
  QCOMPARE( res.size(), 1 );
  QVERIFY( res.at( 0 ).isValid() );

  // Just to be sure
  res = weakRel.resolvedRelations( QgsProject::instance() );
  QCOMPARE( res.size(), 1 );
  QVERIFY( res.at( 0 ).isValid() );
}

void TestQgsWeakRelation::testResolvedManyToMany()
{
  QgsWeakRelation weakRel( u"my_relation_id"_s, u"my_relation_name"_s, Qgis::RelationshipStrength::Association, u"referencingLayerId"_s, u"referencingLayerName"_s, u"Point?crs=epsg:4326&field=pk:int&field=fk_province:int&field=fk_municipality:int"_s, u"memory"_s, u"referencedLayerId"_s, u"referencedLayerName"_s, u"Polygon?crs=epsg:4326&field=pk:int&field=province:int&field=municipality:string"_s, u"memory"_s );
  weakRel.setCardinality( Qgis::RelationshipCardinality::ManyToMany );
  weakRel.setMappingTable( QgsVectorLayerRef( u"mappingTableId"_s, u"mappingTableName"_s, u"None?field=origin_key:int&field=destination_key:int"_s, u"memory"_s ) );

  weakRel.setReferencingLayerFields( { "fk_province" } );
  weakRel.setMappingReferencingLayerFields( { "destination_key" } );
  weakRel.setReferencedLayerFields( { "pk" } );
  weakRel.setMappingReferencedLayerFields( { "origin_key" } );

  QList<QgsRelation> res = weakRel.resolvedRelations( QgsProject::instance(), QgsVectorLayerRef::MatchType::Name );
  QCOMPARE( res.size(), 2 );
  QVERIFY( !res.at( 0 ).isValid() );
  QVERIFY( !res.at( 1 ).isValid() );

  // create a vector layer
  QgsVectorLayer referencedLayer( u"Polygon?crs=epsg:4326&field=pk:int&field=province:int&field=municipality:string"_s, u"referencedLayerName"_s, u"memory"_s );
  QgsProject::instance()->addMapLayer( &referencedLayer, false, false );
  res = weakRel.resolvedRelations( QgsProject::instance(), QgsVectorLayerRef::MatchType::Name );
  QCOMPARE( res.size(), 2 );
  QVERIFY( !res.at( 0 ).isValid() );
  QVERIFY( !res.at( 1 ).isValid() );

  QgsVectorLayer referencingLayer( u"Point?crs=epsg:4326&field=pk:int&field=fk_province:int&field=fk_municipality:int"_s, u"referencingLayerName"_s, u"memory"_s );
  QgsProject::instance()->addMapLayer( &referencingLayer, false, false );
  res = weakRel.resolvedRelations( QgsProject::instance(), QgsVectorLayerRef::MatchType::Name );
  QCOMPARE( res.size(), 2 );
  QVERIFY( !res.at( 0 ).isValid() );
  QVERIFY( !res.at( 1 ).isValid() );

  QgsVectorLayer mappingTable( u"None?field=origin_key:int&field=destination_key:int"_s, u"mappingTableName"_s, u"memory"_s );
  QgsProject::instance()->addMapLayer( &mappingTable, false, false );
  res = weakRel.resolvedRelations( QgsProject::instance(), QgsVectorLayerRef::MatchType::Name );
  QCOMPARE( res.size(), 2 );
  QVERIFY( res.at( 0 ).isValid() );
  QVERIFY( res.at( 1 ).isValid() );

  QCOMPARE( res.at( 0 ).referencedLayerId(), referencedLayer.id() );
  QCOMPARE( res.at( 0 ).referencingLayerId(), mappingTable.id() );
  QCOMPARE( res.at( 0 ).referencingFields(), { 0 } );
  QCOMPARE( res.at( 0 ).referencedFields(), { 0 } );

  QCOMPARE( res.at( 1 ).referencedLayerId(), referencingLayer.id() );
  QCOMPARE( res.at( 1 ).referencingLayerId(), mappingTable.id() );
  QCOMPARE( res.at( 1 ).referencingFields(), { 1 } );
  QCOMPARE( res.at( 1 ).referencedFields(), { 1 } );

  res = weakRel.resolvedRelations( QgsProject::instance(), static_cast<QgsVectorLayerRef::MatchType>( QgsVectorLayerRef::MatchType::Name | QgsVectorLayerRef::MatchType::Provider ) );
  QCOMPARE( res.size(), 2 );
  QVERIFY( res.at( 0 ).isValid() );
  QVERIFY( res.at( 1 ).isValid() );

  // This fails because memory provider stores an UUID in the data source definition ...
  res = weakRel.resolvedRelations( QgsProject::instance(), static_cast<QgsVectorLayerRef::MatchType>( QgsVectorLayerRef::MatchType::Name | QgsVectorLayerRef::MatchType::Source ) );
  QCOMPARE( res.size(), 2 );
  QVERIFY( !res.at( 0 ).isValid() );
  QVERIFY( !res.at( 1 ).isValid() );

  // ... let's fix it
  weakRel.mReferencedLayer.source = referencedLayer.publicSource();
  weakRel.mReferencingLayer.source = referencingLayer.publicSource();
  weakRel.mMappingTable.source = mappingTable.publicSource();
  res = weakRel.resolvedRelations( QgsProject::instance(), static_cast<QgsVectorLayerRef::MatchType>( QgsVectorLayerRef::MatchType::Name | QgsVectorLayerRef::MatchType::Source ) );
  QCOMPARE( res.size(), 2 );
  QVERIFY( res.at( 0 ).isValid() );
  QVERIFY( res.at( 1 ).isValid() );

  // Just to be sure
  res = weakRel.resolvedRelations( QgsProject::instance() );
  QCOMPARE( res.size(), 2 );
  QVERIFY( res.at( 0 ).isValid() );
  QVERIFY( res.at( 1 ).isValid() );
}

void TestQgsWeakRelation::testReadWrite()
{
  QgsWeakRelation weakRel( u"my_relation_id"_s, u"my_relation_name"_s, Qgis::RelationshipStrength::Association, u"referencingLayerId"_s, u"referencingLayerName"_s, u"Point?crs=epsg:4326&field=pk:int&field=fk_province:int&field=fk_municipality:int"_s, u"memory"_s, u"referencedLayerId"_s, u"referencedLayerName"_s, u"Polygon?crs=epsg:4326&field=pk:int&field=province:int&field=municipality:string"_s, u"memory"_s );
  weakRel.setReferencingLayerFields( { "fk_province" } );
  weakRel.setReferencedLayerFields( { "pk" } );

  QgsVectorLayer referencedLayer( u"Polygon?crs=epsg:4326&field=pk:int&field=province:int&field=municipality:string"_s, u"referencedLayerName"_s, u"memory"_s );
  QgsProject::instance()->addMapLayer( &referencedLayer, false, false );

  QgsVectorLayer referencingLayer( u"Point?crs=epsg:4326&field=pk:int&field=fk_province:int&field=fk_municipality:int"_s, u"referencingLayerName"_s, u"memory"_s );
  QgsProject::instance()->addMapLayer( &referencingLayer, false, false );

  const QList<QgsRelation> relations( weakRel.resolvedRelations( QgsProject::instance(), QgsVectorLayerRef::MatchType::Name ) );
  QCOMPARE( relations.size(), 1 );
  QVERIFY( relations.at( 0 ).isValid() );

  QDomImplementation DomImplementation;
  const QDomDocumentType documentType = DomImplementation.createDocumentType(
    u"qgis"_s, u"http://mrcc.com/qgis.dtd"_s, u"SYSTEM"_s
  );
  QDomDocument doc( documentType );

  // Check the XML is written for the referenced layer
  QDomElement node = doc.createElement( u"relation"_s );
  QgsWeakRelation::writeXml( &referencedLayer, QgsWeakRelation::Referenced, relations.at( 0 ), node, doc );
  const QgsWeakRelation weakRelReferenced( QgsWeakRelation::readXml( &referencedLayer, QgsWeakRelation::Referenced, node, QgsProject::instance()->pathResolver() ) );
  QCOMPARE( weakRelReferenced.referencingLayerFields(), { "fk_province" } );
  QCOMPARE( weakRelReferenced.referencedLayerFields(), { "pk" } );
  QCOMPARE( weakRelReferenced.strength(), Qgis::RelationshipStrength::Association );
  QCOMPARE( weakRelReferenced.referencedLayer().resolve( QgsProject::instance() ), &referencedLayer );

  // Check the XML is written for the referencing layer
  node = doc.createElement( u"relation"_s );
  QgsWeakRelation::writeXml( &referencingLayer, QgsWeakRelation::Referencing, relations.at( 0 ), node, doc );
  const QgsWeakRelation weakRelReferencing( QgsWeakRelation::readXml( &referencingLayer, QgsWeakRelation::Referencing, node, QgsProject::instance()->pathResolver() ) );
  QCOMPARE( weakRelReferencing.referencingLayerFields(), { "fk_province" } );
  QCOMPARE( weakRelReferencing.referencedLayerFields(), { "pk" } );
  QCOMPARE( weakRelReferencing.strength(), Qgis::RelationshipStrength::Association );
  QCOMPARE( weakRelReferencing.referencingLayer().resolve( QgsProject::instance() ), &referencingLayer );
}

void TestQgsWeakRelation::testWriteStyleCategoryRelations()
{
  // Create a referencing layer and two referenced layers
  QgsVectorLayer *layer1 = new QgsVectorLayer( u"Point?crs=epsg:3111&field=pk:int&field=fk1:int&field=fk2:int"_s, u"vl1"_s, u"memory"_s );
  QgsProject::instance()->addMapLayer( layer1 );

  QgsVectorLayer *layer2 = new QgsVectorLayer( u"None?field=pk:int&field=name:string"_s, u"vl2"_s, u"memory"_s );
  QgsProject::instance()->addMapLayer( layer2 );

  QgsVectorLayer *layer3 = new QgsVectorLayer( u"None?field=pk:int&field=name:string"_s, u"vl3"_s, u"memory"_s );
  QgsProject::instance()->addMapLayer( layer3 );

  // Create relation 1
  QgsRelation mRelation1;
  mRelation1.setId( u"vl1.vl2"_s );
  mRelation1.setName( u"vl1.vl2"_s );
  mRelation1.setReferencingLayer( layer1->id() );
  mRelation1.setReferencedLayer( layer2->id() );
  mRelation1.addFieldPair( u"fk1"_s, u"pk"_s );
  QVERIFY( mRelation1.isValid() );
  QgsProject::instance()->relationManager()->addRelation( mRelation1 );

  // Create relation 2
  QgsRelation mRelation2;
  mRelation2.setId( u"vl1.vl3"_s );
  mRelation2.setName( u"vl1.vl3"_s );
  mRelation2.setReferencingLayer( layer1->id() );
  mRelation2.setReferencedLayer( layer3->id() );
  mRelation2.addFieldPair( u"fk2"_s, u"pk"_s );
  QVERIFY( mRelation2.isValid() );
  QgsProject::instance()->relationManager()->addRelation( mRelation2 );

  // Write to XML
  QDomImplementation DomImplementation;
  const QDomDocumentType documentType = DomImplementation.createDocumentType(
    u"qgis"_s, u"http://mrcc.com/qgis.dtd"_s, u"SYSTEM"_s
  );
  QDomDocument doc( documentType );
  QDomElement node = doc.createElement( u"style_categories_relations"_s );
  QString errorMessage;
  const QgsReadWriteContext context = QgsReadWriteContext();

  layer1->writeSymbology( node, doc, errorMessage, context, QgsMapLayer::Relations );

  // Check XML tags and attributes
  const QDomElement referencedLayersElement = node.firstChildElement( u"referencedLayers"_s );
  Q_ASSERT( !referencedLayersElement.isNull() );
  const QDomNodeList relationsNodeList = referencedLayersElement.elementsByTagName( u"relation"_s );
  QCOMPARE( relationsNodeList.size(), 2 );
  QDomElement relationElement;

  int visitedCount = 0;
  for ( int i = 0; i < relationsNodeList.size(); ++i )
  {
    relationElement = relationsNodeList.at( i ).toElement();

    // Common checks
    Q_ASSERT( relationElement.hasAttribute( u"id"_s ) );
    Q_ASSERT( relationElement.hasAttribute( u"name"_s ) );
    Q_ASSERT( relationElement.hasAttribute( u"referencingLayer"_s ) );
    Q_ASSERT( relationElement.hasAttribute( u"referencedLayer"_s ) );
    Q_ASSERT( relationElement.hasAttribute( u"layerId"_s ) );     // Weak relation attribute
    Q_ASSERT( relationElement.hasAttribute( u"layerName"_s ) );   // Weak relation attribute
    Q_ASSERT( relationElement.hasAttribute( u"dataSource"_s ) );  // Weak relation attribute
    Q_ASSERT( relationElement.hasAttribute( u"providerKey"_s ) ); // Weak relation attribute

    QCOMPARE( relationElement.attribute( u"providerKey"_s ), u"memory"_s );
    QCOMPARE( relationElement.attribute( u"referencingLayer"_s ), layer1->id() );

    if ( relationElement.attribute( u"id"_s ) == mRelation1.id() )
    {
      QCOMPARE( relationElement.attribute( u"referencedLayer"_s ), layer2->id() );
      QCOMPARE( relationElement.attribute( u"dataSource"_s ), layer2->publicSource() );
      QCOMPARE( relationElement.attribute( u"layerId"_s ), layer2->id() );
      QCOMPARE( relationElement.attribute( u"layerName"_s ), layer2->name() );
      visitedCount++;
    }
    else if ( relationElement.attribute( u"id"_s ) == mRelation2.id() )
    {
      QCOMPARE( relationElement.attribute( u"referencedLayer"_s ), layer3->id() );
      QCOMPARE( relationElement.attribute( u"dataSource"_s ), layer3->publicSource() );
      QCOMPARE( relationElement.attribute( u"layerId"_s ), layer3->id() );
      QCOMPARE( relationElement.attribute( u"layerName"_s ), layer3->name() );
      visitedCount++;
    }
  }
  QCOMPARE( visitedCount, 2 );
}

QGSTEST_MAIN( TestQgsWeakRelation )
#include "testqgsweakrelation.moc"
