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
  const QList<QgsRelation::FieldPair> fieldPairs {{ "fk_province", "pk" }};

  QgsWeakRelation weakRel( QStringLiteral( "my_relation_id" ),
                           QStringLiteral( "my_relation_name" ),
                           QgsRelation::RelationStrength::Association,
                           QStringLiteral( "referencingLayerId" ),
                           QStringLiteral( "referencingLayerName" ),
                           QStringLiteral( "Point?crs=epsg:4326&field=pk:int&field=fk_province:int&field=fk_municipality:int" ),
                           QStringLiteral( "memory" ),
                           QStringLiteral( "referencedLayerId" ),
                           QStringLiteral( "referencedLayerName" ),
                           QStringLiteral( "Polygon?crs=epsg:4326&field=pk:int&field=province:int&field=municipality:string" ),
                           QStringLiteral( "memory" ),
                           fieldPairs
                         );
  QVERIFY( ! weakRel.resolvedRelation( QgsProject::instance(), QgsVectorLayerRef::MatchType::Name ).isValid() );

  // create a vector layer
  QgsVectorLayer referencedLayer( QStringLiteral( "Polygon?crs=epsg:4326&field=pk:int&field=province:int&field=municipality:string" ), QStringLiteral( "referencedLayerName" ), QStringLiteral( "memory" ) );
  QgsProject::instance()->addMapLayer( &referencedLayer, false, false );
  QVERIFY( ! weakRel.resolvedRelation( QgsProject::instance(), QgsVectorLayerRef::MatchType::Name ).isValid() );

  QgsVectorLayer referencingLayer( QStringLiteral( "Point?crs=epsg:4326&field=pk:int&field=fk_province:int&field=fk_municipality:int" ), QStringLiteral( "referencingLayerName" ), QStringLiteral( "memory" ) );
  QgsProject::instance()->addMapLayer( &referencingLayer, false, false );
  QVERIFY( weakRel.resolvedRelation( QgsProject::instance(), QgsVectorLayerRef::MatchType::Name ).isValid() );

  QVERIFY( weakRel.resolvedRelation( QgsProject::instance(), static_cast<QgsVectorLayerRef::MatchType>( QgsVectorLayerRef::MatchType::Name | QgsVectorLayerRef::MatchType::Provider ) ).isValid() );

  // This fails because memory provider stores an UUID in the data source definition ...
  QVERIFY( !weakRel.resolvedRelation( QgsProject::instance(), static_cast<QgsVectorLayerRef::MatchType>( QgsVectorLayerRef::MatchType::Name | QgsVectorLayerRef::MatchType::Source ) ).isValid() );

  // ... let's fix it
  weakRel.mReferencedLayer.source = referencedLayer.publicSource();
  weakRel.mReferencingLayer.source = referencingLayer.publicSource();
  QVERIFY( weakRel.resolvedRelation( QgsProject::instance(), static_cast<QgsVectorLayerRef::MatchType>( QgsVectorLayerRef::MatchType::Name | QgsVectorLayerRef::MatchType::Source ) ).isValid() );

  // Just to be sure
  QVERIFY( weakRel.resolvedRelation( QgsProject::instance() ).isValid() );
}

void TestQgsWeakRelation::testReadWrite()
{
  const QList<QgsRelation::FieldPair> fieldPairs {{ "fk_province", "pk" }};

  const QgsWeakRelation weakRel( QStringLiteral( "my_relation_id" ),
                                 QStringLiteral( "my_relation_name" ),
                                 QgsRelation::RelationStrength::Association,
                                 QStringLiteral( "referencingLayerId" ),
                                 QStringLiteral( "referencingLayerName" ),
                                 QStringLiteral( "Point?crs=epsg:4326&field=pk:int&field=fk_province:int&field=fk_municipality:int" ),
                                 QStringLiteral( "memory" ),
                                 QStringLiteral( "referencedLayerId" ),
                                 QStringLiteral( "referencedLayerName" ),
                                 QStringLiteral( "Polygon?crs=epsg:4326&field=pk:int&field=province:int&field=municipality:string" ),
                                 QStringLiteral( "memory" ),
                                 fieldPairs
                               );

  QgsVectorLayer referencedLayer( QStringLiteral( "Polygon?crs=epsg:4326&field=pk:int&field=province:int&field=municipality:string" ), QStringLiteral( "referencedLayerName" ), QStringLiteral( "memory" ) );
  QgsProject::instance()->addMapLayer( &referencedLayer, false, false );

  QgsVectorLayer referencingLayer( QStringLiteral( "Point?crs=epsg:4326&field=pk:int&field=fk_province:int&field=fk_municipality:int" ), QStringLiteral( "referencingLayerName" ), QStringLiteral( "memory" ) );
  QgsProject::instance()->addMapLayer( &referencingLayer, false, false );

  const QgsRelation relation( weakRel.resolvedRelation( QgsProject::instance(), QgsVectorLayerRef::MatchType::Name ) );
  QVERIFY( relation.isValid() );

  QDomImplementation DomImplementation;
  const QDomDocumentType documentType =
    DomImplementation.createDocumentType(
      QStringLiteral( "qgis" ), QStringLiteral( "http://mrcc.com/qgis.dtd" ), QStringLiteral( "SYSTEM" ) );
  QDomDocument doc( documentType );

  // Check the XML is written for the referenced layer
  QDomElement node = doc.createElement( QStringLiteral( "relation" ) );
  QgsWeakRelation::writeXml( &referencedLayer, QgsWeakRelation::Referenced, relation, node, doc );
  const QgsWeakRelation weakRelReferenced( QgsWeakRelation::readXml( &referencedLayer, QgsWeakRelation::Referenced, node,  QgsProject::instance()->pathResolver() ) );
  QCOMPARE( weakRelReferenced.fieldPairs(), fieldPairs );
  QCOMPARE( weakRelReferenced.strength(), QgsRelation::RelationStrength::Association );
  QCOMPARE( weakRelReferenced.referencedLayer().resolve( QgsProject::instance() ), &referencedLayer );

  // Check the XML is written for the referencing layer
  node = doc.createElement( QStringLiteral( "relation" ) );
  QgsWeakRelation::writeXml( &referencingLayer, QgsWeakRelation::Referencing, relation, node, doc );
  const QgsWeakRelation weakRelReferencing( QgsWeakRelation::readXml( &referencingLayer, QgsWeakRelation::Referencing, node,  QgsProject::instance()->pathResolver() ) );
  QCOMPARE( weakRelReferencing.fieldPairs(), fieldPairs );
  QCOMPARE( weakRelReferencing.strength(), QgsRelation::RelationStrength::Association );
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
