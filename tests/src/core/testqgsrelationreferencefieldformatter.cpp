/***************************************************************************
     testqgsrelationreferencefieldformatter.cpp
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
#include "fieldformatter/qgsrelationreferencefieldformatter.h"

class TestQgsRelationReferenceFieldFormatter : public QObject
{
    Q_OBJECT

  private slots:

    void initTestCase();    // will be called before the first testfunction is executed.
    void cleanupTestCase(); // will be called after the last testfunction was executed.
    void init();            // will be called before each testfunction is executed.
    void cleanup();         // will be called after every testfunction.
    void testDependencies();

  private:
    std::unique_ptr<QgsVectorLayer> mLayer1;
    std::unique_ptr<QgsVectorLayer> mLayer2;
    std::unique_ptr<QgsRelation> mRelation;
};


void TestQgsRelationReferenceFieldFormatter::initTestCase()
{
  QgsApplication::init();
  QgsApplication::initQgis();
}

void TestQgsRelationReferenceFieldFormatter::cleanupTestCase()
{
  QgsApplication::exitQgis();
}

void TestQgsRelationReferenceFieldFormatter::cleanup()
{
  QgsProject::instance()->removeMapLayer( mLayer1.get() );
  QgsProject::instance()->removeMapLayer( mLayer2.get() );
}


void TestQgsRelationReferenceFieldFormatter::init()
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
  ft3.setAttribute( u"diameter"_s, 120 );
  ft3.setAttribute( u"raccord"_s, "sleeve" );
  mLayer2->startEditing();
  mLayer2->addFeature( ft3 );
  mLayer2->commitChanges();

  QgsFeature ft4( mLayer2->fields() );
  ft4.setAttribute( u"pk"_s, 12 );
  ft4.setAttribute( u"material"_s, "steel" );
  ft4.setAttribute( u"diameter"_s, 120 );
  ft4.setAttribute( u"raccord"_s, "collar" );
  mLayer2->startEditing();
  mLayer2->addFeature( ft4 );
  mLayer2->commitChanges();
}

void TestQgsRelationReferenceFieldFormatter::testDependencies()
{
  // Test dependencies

  const QgsEditorWidgetSetup setup { u"RelationReference"_s, {
                                                               { u"ReferencedLayerDataSource"_s, mLayer2->publicSource() },
                                                               { u"ReferencedLayerProviderKey"_s, mLayer2->providerType() },
                                                               { u"ReferencedLayerId"_s, mLayer2->id() },
                                                               { u"ReferencedLayerName"_s, mLayer2->name() },
                                                             } };
  QgsFieldFormatter *fieldFormatter = QgsApplication::fieldFormatterRegistry()->fieldFormatter( setup.type() );
  const QList<QgsVectorLayerRef> dependencies = fieldFormatter->layerDependencies( setup.config() );
  QVERIFY( dependencies.count() == 1 );
  const QgsVectorLayerRef dependency = dependencies.first();
  QCOMPARE( dependency.layerId, mLayer2->id() );
  QCOMPARE( dependency.name, mLayer2->name() );
  QCOMPARE( dependency.provider, mLayer2->providerType() );
  QCOMPARE( dependency.source, mLayer2->publicSource() );
}

QGSTEST_MAIN( TestQgsRelationReferenceFieldFormatter )
#include "testqgsrelationreferencefieldformatter.moc"
