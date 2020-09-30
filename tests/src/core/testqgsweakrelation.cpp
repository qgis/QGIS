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
  QList<QgsRelation::FieldPair> fieldPairs {{ "fk_province", "pk" }};

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
  QList<QgsRelation::FieldPair> fieldPairs {{ "fk_province", "pk" }};

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

  QgsVectorLayer referencedLayer( QStringLiteral( "Polygon?crs=epsg:4326&field=pk:int&field=province:int&field=municipality:string" ), QStringLiteral( "referencedLayerName" ), QStringLiteral( "memory" ) );
  QgsProject::instance()->addMapLayer( &referencedLayer, false, false );

  QgsVectorLayer referencingLayer( QStringLiteral( "Point?crs=epsg:4326&field=pk:int&field=fk_province:int&field=fk_municipality:int" ), QStringLiteral( "referencingLayerName" ), QStringLiteral( "memory" ) );
  QgsProject::instance()->addMapLayer( &referencingLayer, false, false );

  QgsRelation relation( weakRel.resolvedRelation( QgsProject::instance(), QgsVectorLayerRef::MatchType::Name ) );
  QVERIFY( relation.isValid() );

  QDomImplementation DomImplementation;
  QDomDocumentType documentType =
    DomImplementation.createDocumentType(
      QStringLiteral( "qgis" ), QStringLiteral( "http://mrcc.com/qgis.dtd" ), QStringLiteral( "SYSTEM" ) );
  QDomDocument doc( documentType );

  // Check the XML is written for the referenced layer
  QDomElement node = doc.createElement( QStringLiteral( "relation" ) );
  QgsWeakRelation::writeXml( &referencedLayer, QgsWeakRelation::Referenced, relation, node, doc );
  QgsWeakRelation weakRelReferenced( QgsWeakRelation::readXml( &referencedLayer, QgsWeakRelation::Referenced, node,  QgsProject::instance()->pathResolver() ) );
  QCOMPARE( weakRelReferenced.fieldPairs(), fieldPairs );
  QCOMPARE( weakRelReferenced.strength(), QgsRelation::RelationStrength::Association );
  QCOMPARE( weakRelReferenced.referencedLayer().resolve( QgsProject::instance() ), &referencedLayer );

  // Check the XML is written for the referencing layer
  node = doc.createElement( QStringLiteral( "relation" ) );
  QgsWeakRelation::writeXml( &referencingLayer, QgsWeakRelation::Referencing, relation, node, doc );
  QgsWeakRelation weakRelReferencing( QgsWeakRelation::readXml( &referencingLayer, QgsWeakRelation::Referencing, node,  QgsProject::instance()->pathResolver() ) );
  QCOMPARE( weakRelReferencing.fieldPairs(), fieldPairs );
  QCOMPARE( weakRelReferencing.strength(), QgsRelation::RelationStrength::Association );
  QCOMPARE( weakRelReferencing.referencingLayer().resolve( QgsProject::instance() ), &referencingLayer );
}

QGSTEST_MAIN( TestQgsWeakRelation )
#include "testqgsweakrelation.moc"
