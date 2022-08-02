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

#include "qgstest.h"
#include "qgsapplication.h"
#include "qgsvectorlayer.h"
#include "qgsproject.h"
#include "qgsweakrelation.h"
#include "qgsrelation.h"
#include "qgsrelationmanager.h"

class TestQgsWeakRelation: public QObject
{
    Q_OBJECT

  private slots:
    void initTestCase();// will be called before the first testfunction is executed.
    void cleanupTestCase();// will be called after the last testfunction was executed.
    void init();// will be called before each testfunction is executed.
    void cleanup();// will be called after every testfunction.

    void testResolved(); // Test if relation can be resolved
    void testResolvedManyToMany();
    void testReadWrite(); // Test if relation can be read and write

    void testWriteStyleCategoryRelations(); // Test if weak relation attrs are written well in the style "relations"

  private:
};

void TestQgsWeakRelation::initTestCase()
{
  // Set up the QgsSettings environment
  QCoreApplication::setOrganizationName( QStringLiteral( "QGIS" ) );
  QCoreApplication::setOrganizationDomain( QStringLiteral( "qgis.org" ) );
  QCoreApplication::setApplicationName( QStringLiteral( "QGIS-TEST" ) );
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

void TestQgsWeakRelation::testResolved()
{
  QgsWeakRelation weakRel( QStringLiteral( "my_relation_id" ),
                           QStringLiteral( "my_relation_name" ),
                           Qgis::RelationshipStrength::Association,
                           QStringLiteral( "referencingLayerId" ),
                           QStringLiteral( "referencingLayerName" ),
                           QStringLiteral( "Point?crs=epsg:4326&field=pk:int&field=fk_province:int&field=fk_municipality:int" ),
                           QStringLiteral( "memory" ),
                           QStringLiteral( "referencedLayerId" ),
                           QStringLiteral( "referencedLayerName" ),
                           QStringLiteral( "Polygon?crs=epsg:4326&field=pk:int&field=province:int&field=municipality:string" ),
                           QStringLiteral( "memory" )
                         );
  weakRel.setReferencingLayerFields( { "fk_province" } );
  weakRel.setReferencedLayerFields( { "pk" } );

  QList< QgsRelation > res = weakRel.resolvedRelations( QgsProject::instance(), QgsVectorLayerRef::MatchType::Name );
  QCOMPARE( res.size(), 1 );
  QVERIFY( ! res.at( 0 ).isValid() );

  // create a vector layer
  QgsVectorLayer referencedLayer( QStringLiteral( "Polygon?crs=epsg:4326&field=pk:int&field=province:int&field=municipality:string" ), QStringLiteral( "referencedLayerName" ), QStringLiteral( "memory" ) );
  QgsProject::instance()->addMapLayer( &referencedLayer, false, false );
  res = weakRel.resolvedRelations( QgsProject::instance(), QgsVectorLayerRef::MatchType::Name );
  QCOMPARE( res.size(), 1 );
  QVERIFY( ! res.at( 0 ).isValid() );

  QgsVectorLayer referencingLayer( QStringLiteral( "Point?crs=epsg:4326&field=pk:int&field=fk_province:int&field=fk_municipality:int" ), QStringLiteral( "referencingLayerName" ), QStringLiteral( "memory" ) );
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
  QgsWeakRelation weakRel( QStringLiteral( "my_relation_id" ),
                           QStringLiteral( "my_relation_name" ),
                           Qgis::RelationshipStrength::Association,
                           QStringLiteral( "referencingLayerId" ),
                           QStringLiteral( "referencingLayerName" ),
                           QStringLiteral( "Point?crs=epsg:4326&field=pk:int&field=fk_province:int&field=fk_municipality:int" ),
                           QStringLiteral( "memory" ),
                           QStringLiteral( "referencedLayerId" ),
                           QStringLiteral( "referencedLayerName" ),
                           QStringLiteral( "Polygon?crs=epsg:4326&field=pk:int&field=province:int&field=municipality:string" ),
                           QStringLiteral( "memory" )
                         );
  weakRel.setCardinality( Qgis::RelationshipCardinality::ManyToMany );
  weakRel.setMappingTable( QgsVectorLayerRef( QStringLiteral( "mappingTableId" ), QStringLiteral( "mappingTableName" ), QStringLiteral( "None?field=origin_key:int&field=destination_key:int" ), QStringLiteral( "memory" ) ) );

  weakRel.setReferencingLayerFields( { "fk_province" } );
  weakRel.setMappingReferencingLayerFields( { "destination_key" } );
  weakRel.setReferencedLayerFields( { "pk" } );
  weakRel.setMappingReferencedLayerFields( { "origin_key" } );

  QList< QgsRelation > res = weakRel.resolvedRelations( QgsProject::instance(), QgsVectorLayerRef::MatchType::Name );
  QCOMPARE( res.size(), 2 );
  QVERIFY( ! res.at( 0 ).isValid() );
  QVERIFY( ! res.at( 1 ).isValid() );

  // create a vector layer
  QgsVectorLayer referencedLayer( QStringLiteral( "Polygon?crs=epsg:4326&field=pk:int&field=province:int&field=municipality:string" ), QStringLiteral( "referencedLayerName" ), QStringLiteral( "memory" ) );
  QgsProject::instance()->addMapLayer( &referencedLayer, false, false );
  res = weakRel.resolvedRelations( QgsProject::instance(), QgsVectorLayerRef::MatchType::Name );
  QCOMPARE( res.size(), 2 );
  QVERIFY( ! res.at( 0 ).isValid() );
  QVERIFY( ! res.at( 1 ).isValid() );

  QgsVectorLayer referencingLayer( QStringLiteral( "Point?crs=epsg:4326&field=pk:int&field=fk_province:int&field=fk_municipality:int" ), QStringLiteral( "referencingLayerName" ), QStringLiteral( "memory" ) );
  QgsProject::instance()->addMapLayer( &referencingLayer, false, false );
  res = weakRel.resolvedRelations( QgsProject::instance(), QgsVectorLayerRef::MatchType::Name );
  QCOMPARE( res.size(), 2 );
  QVERIFY( ! res.at( 0 ).isValid() );
  QVERIFY( ! res.at( 1 ).isValid() );

  QgsVectorLayer mappingTable( QStringLiteral( "None?field=origin_key:int&field=destination_key:int" ), QStringLiteral( "mappingTableName" ), QStringLiteral( "memory" ) );
  QgsProject::instance()->addMapLayer( &mappingTable, false, false );
  res = weakRel.resolvedRelations( QgsProject::instance(), QgsVectorLayerRef::MatchType::Name );
  QCOMPARE( res.size(), 2 );
  QVERIFY( res.at( 0 ).isValid() );
  QVERIFY( res.at( 1 ).isValid() );

  QCOMPARE( res.at( 0 ).referencedLayerId(), referencedLayer.id() );
  QCOMPARE( res.at( 0 ).referencingLayerId(), mappingTable.id() );
  QCOMPARE( res.at( 0 ).referencingFields(), {0} );
  QCOMPARE( res.at( 0 ).referencedFields(), {0} );

  QCOMPARE( res.at( 1 ).referencedLayerId(), referencingLayer.id() );
  QCOMPARE( res.at( 1 ).referencingLayerId(), mappingTable.id() );
  QCOMPARE( res.at( 1 ).referencingFields(), {1} );
  QCOMPARE( res.at( 1 ).referencedFields(), {1} );

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
  QgsWeakRelation weakRel( QStringLiteral( "my_relation_id" ),
                           QStringLiteral( "my_relation_name" ),
                           Qgis::RelationshipStrength::Association,
                           QStringLiteral( "referencingLayerId" ),
                           QStringLiteral( "referencingLayerName" ),
                           QStringLiteral( "Point?crs=epsg:4326&field=pk:int&field=fk_province:int&field=fk_municipality:int" ),
                           QStringLiteral( "memory" ),
                           QStringLiteral( "referencedLayerId" ),
                           QStringLiteral( "referencedLayerName" ),
                           QStringLiteral( "Polygon?crs=epsg:4326&field=pk:int&field=province:int&field=municipality:string" ),
                           QStringLiteral( "memory" )
                         );
  weakRel.setReferencingLayerFields( {"fk_province" } );
  weakRel.setReferencedLayerFields( {"pk" } );

  QgsVectorLayer referencedLayer( QStringLiteral( "Polygon?crs=epsg:4326&field=pk:int&field=province:int&field=municipality:string" ), QStringLiteral( "referencedLayerName" ), QStringLiteral( "memory" ) );
  QgsProject::instance()->addMapLayer( &referencedLayer, false, false );

  QgsVectorLayer referencingLayer( QStringLiteral( "Point?crs=epsg:4326&field=pk:int&field=fk_province:int&field=fk_municipality:int" ), QStringLiteral( "referencingLayerName" ), QStringLiteral( "memory" ) );
  QgsProject::instance()->addMapLayer( &referencingLayer, false, false );

  const QList< QgsRelation > relations( weakRel.resolvedRelations( QgsProject::instance(), QgsVectorLayerRef::MatchType::Name ) );
  QCOMPARE( relations.size(), 1 );
  QVERIFY( relations.at( 0 ).isValid() );

  QDomImplementation DomImplementation;
  const QDomDocumentType documentType =
    DomImplementation.createDocumentType(
      QStringLiteral( "qgis" ), QStringLiteral( "http://mrcc.com/qgis.dtd" ), QStringLiteral( "SYSTEM" ) );
  QDomDocument doc( documentType );

  // Check the XML is written for the referenced layer
  QDomElement node = doc.createElement( QStringLiteral( "relation" ) );
  QgsWeakRelation::writeXml( &referencedLayer, QgsWeakRelation::Referenced, relations.at( 0 ), node, doc );
  const QgsWeakRelation weakRelReferenced( QgsWeakRelation::readXml( &referencedLayer, QgsWeakRelation::Referenced, node,  QgsProject::instance()->pathResolver() ) );
  QCOMPARE( weakRelReferenced.referencingLayerFields(), {"fk_province" } );
  QCOMPARE( weakRelReferenced.referencedLayerFields(), {"pk" } );
  QCOMPARE( weakRelReferenced.strength(), Qgis::RelationshipStrength::Association );
  QCOMPARE( weakRelReferenced.referencedLayer().resolve( QgsProject::instance() ), &referencedLayer );

  // Check the XML is written for the referencing layer
  node = doc.createElement( QStringLiteral( "relation" ) );
  QgsWeakRelation::writeXml( &referencingLayer, QgsWeakRelation::Referencing, relations.at( 0 ), node, doc );
  const QgsWeakRelation weakRelReferencing( QgsWeakRelation::readXml( &referencingLayer, QgsWeakRelation::Referencing, node,  QgsProject::instance()->pathResolver() ) );
  QCOMPARE( weakRelReferencing.referencingLayerFields(), {"fk_province" } );
  QCOMPARE( weakRelReferencing.referencedLayerFields(), {"pk" } );
  QCOMPARE( weakRelReferencing.strength(), Qgis::RelationshipStrength::Association );
  QCOMPARE( weakRelReferencing.referencingLayer().resolve( QgsProject::instance() ), &referencingLayer );
}

void TestQgsWeakRelation::testWriteStyleCategoryRelations()
{
  // Create a referencing layer and two referenced layers
  QgsVectorLayer mLayer1( QStringLiteral( "Point?crs=epsg:3111&field=pk:int&field=fk1:int&field=fk2:int" ), QStringLiteral( "vl1" ), QStringLiteral( "memory" ) );
  QgsProject::instance()->addMapLayer( &mLayer1, false, false );

  QgsVectorLayer mLayer2( QStringLiteral( "None?field=pk:int&field=name:string" ), QStringLiteral( "vl2" ), QStringLiteral( "memory" ) );
  QgsProject::instance()->addMapLayer( &mLayer2, false, false );

  QgsVectorLayer mLayer3( QStringLiteral( "None?field=pk:int&field=name:string" ), QStringLiteral( "vl3" ), QStringLiteral( "memory" ) );
  QgsProject::instance()->addMapLayer( &mLayer3, false, false );

  // Create relation 1
  QgsRelation mRelation1;
  mRelation1.setId( QStringLiteral( "vl1.vl2" ) );
  mRelation1.setName( QStringLiteral( "vl1.vl2" ) );
  mRelation1.setReferencingLayer( mLayer1.id() );
  mRelation1.setReferencedLayer( mLayer2.id() );
  mRelation1.addFieldPair( QStringLiteral( "fk1" ), QStringLiteral( "pk" ) );
  QVERIFY( mRelation1.isValid() );
  QgsProject::instance()->relationManager()->addRelation( mRelation1 );

  // Create relation 2
  QgsRelation mRelation2;
  mRelation2.setId( QStringLiteral( "vl1.vl3" ) );
  mRelation2.setName( QStringLiteral( "vl1.vl3" ) );
  mRelation2.setReferencingLayer( mLayer1.id() );
  mRelation2.setReferencedLayer( mLayer3.id() );
  mRelation2.addFieldPair( QStringLiteral( "fk2" ), QStringLiteral( "pk" ) );
  QVERIFY( mRelation2.isValid() );
  QgsProject::instance()->relationManager()->addRelation( mRelation2 );

  // Write to XML
  QDomImplementation DomImplementation;
  const QDomDocumentType documentType =
    DomImplementation.createDocumentType(
      QStringLiteral( "qgis" ), QStringLiteral( "http://mrcc.com/qgis.dtd" ), QStringLiteral( "SYSTEM" ) );
  QDomDocument doc( documentType );
  QDomElement node = doc.createElement( QStringLiteral( "style_categories_relations" ) );
  QString errorMessage;
  const QgsReadWriteContext context = QgsReadWriteContext();

  mLayer1.writeSymbology( node, doc, errorMessage, context, QgsMapLayer::Relations );

  // Check XML tags and attributes
  const QDomElement referencedLayersElement = node.firstChildElement( QStringLiteral( "referencedLayers" ) );
  Q_ASSERT( !referencedLayersElement.isNull() );
  const QDomNodeList relationsNodeList = referencedLayersElement.elementsByTagName( QStringLiteral( "relation" ) );
  QCOMPARE( relationsNodeList.size(), 2 );
  QDomElement relationElement;

  int visitedCount = 0;
  for ( int i = 0; i < relationsNodeList.size(); ++i )
  {
    relationElement = relationsNodeList.at( i ).toElement();

    // Common checks
    Q_ASSERT( relationElement.hasAttribute( QStringLiteral( "id" ) ) );
    Q_ASSERT( relationElement.hasAttribute( QStringLiteral( "name" ) ) );
    Q_ASSERT( relationElement.hasAttribute( QStringLiteral( "referencingLayer" ) ) );
    Q_ASSERT( relationElement.hasAttribute( QStringLiteral( "referencedLayer" ) ) );
    Q_ASSERT( relationElement.hasAttribute( QStringLiteral( "layerId" ) ) );  // Weak relation attribute
    Q_ASSERT( relationElement.hasAttribute( QStringLiteral( "layerName" ) ) );  // Weak relation attribute
    Q_ASSERT( relationElement.hasAttribute( QStringLiteral( "dataSource" ) ) );  // Weak relation attribute
    Q_ASSERT( relationElement.hasAttribute( QStringLiteral( "providerKey" ) ) );  // Weak relation attribute

    QCOMPARE( relationElement.attribute( QStringLiteral( "providerKey" ) ), QStringLiteral( "memory" ) );
    QCOMPARE( relationElement.attribute( QStringLiteral( "referencingLayer" ) ), mLayer1.id() );

    if ( relationElement.attribute( QStringLiteral( "id" ) ) == mRelation1.id() )
    {
      QCOMPARE( relationElement.attribute( QStringLiteral( "referencedLayer" ) ), mLayer2.id() );
      QCOMPARE( relationElement.attribute( QStringLiteral( "dataSource" ) ), mLayer2.publicSource() );
      QCOMPARE( relationElement.attribute( QStringLiteral( "layerId" ) ), mLayer2.id() );
      QCOMPARE( relationElement.attribute( QStringLiteral( "layerName" ) ), mLayer2.name() );
      visitedCount++;
    }
    else if ( relationElement.attribute( QStringLiteral( "id" ) ) == mRelation2.id() )
    {
      QCOMPARE( relationElement.attribute( QStringLiteral( "referencedLayer" ) ), mLayer3.id() );
      QCOMPARE( relationElement.attribute( QStringLiteral( "dataSource" ) ), mLayer3.publicSource() );
      QCOMPARE( relationElement.attribute( QStringLiteral( "layerId" ) ), mLayer3.id() );
      QCOMPARE( relationElement.attribute( QStringLiteral( "layerName" ) ), mLayer3.name() );
      visitedCount++;
    }
  }
  QCOMPARE( visitedCount, 2 );
}

QGSTEST_MAIN( TestQgsWeakRelation )
#include "testqgsweakrelation.moc"
