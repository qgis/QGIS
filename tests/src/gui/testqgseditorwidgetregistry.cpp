/***************************************************************************
    testqgseditorwidgetregistry.cpp
    ---------------------
    begin                : July 2016
    copyright            : (C) 2016 by Patrick Valsecchi
    email                : patrick.valsecchi at camptocamp.com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "qgseditorwidgetautoconf.h"
#include "qgseditorwidgetregistry.h"
#include "qgsgui.h"
#include "qgsproject.h"
#include "qgsrelationmanager.h"
#include "qgstest.h"

class TestQgsEditorWidgetRegistry : public QObject
{
    Q_OBJECT

    class DummyPlugin : public QgsEditorWidgetAutoConfPlugin
    {
      public:
        QgsEditorWidgetSetup editorWidgetSetup( const QgsVectorLayer *vl, const QString &fieldName, int &score ) const override
        {
          Q_UNUSED( vl )
          if ( fieldName == "special"_L1 )
          {
            score = 100;
            return QgsEditorWidgetSetup( u"Special"_s, QVariantMap() );
          }
          score = 0;
          return QgsEditorWidgetSetup();
        }
    };


  private slots:
    void initTestCase()
    {
      QgsApplication::init();
      QgsApplication::initQgis();
      QgsGui::editorWidgetRegistry()->initEditors();
      QgsGui::editorWidgetRegistry()->registerAutoConfPlugin( new DummyPlugin() );
    }

    void cleanupTestCase()
    {
      QgsApplication::exitQgis();
    }

    void stringType()
    {
      checkSimple( u"string"_s, u"TextEdit"_s );
    }

    void datetimeType()
    {
      checkSimple( u"datetime"_s, u"DateTime"_s );
    }

    void integerType()
    {
      checkSimple( u"integer"_s, u"Range"_s );
    }

    void longLongType()
    {
      checkSimple( u"int8"_s, u"TextEdit"_s ); // no current widget supports 64 bit integers => default to TextEdit
    }

    void doubleType()
    {
      checkSimple( u"double"_s, u"TextEdit"_s );
    }

    void arrayType()
    {
      checkSimple( u"double[]"_s, u"List"_s );
      checkSimple( u"int[]"_s, u"List"_s );
      checkSimple( u"string[]"_s, u"List"_s );
    }

    void binaryType()
    {
      checkSimple( u"binary"_s, u"Binary"_s );
    }

    void configuredType()
    {
      QgsVectorLayer vl( u"LineString?crs=epsg:3111&field=pk:int&field=col1:string"_s, u"vl"_s, u"memory"_s );

      QVariantMap config;
      config[u"a"_s] = QVariant( 12 );
      config[u"b"_s] = QVariant( "bar" );

      vl.setEditorWidgetSetup( 1, QgsEditorWidgetSetup( u"FooEdit"_s, config ) );

      const QgsEditorWidgetSetup setup = QgsGui::editorWidgetRegistry()->findBest( &vl, u"col1"_s );
      QCOMPARE( setup.type(), QString( "FooEdit" ) );
      QCOMPARE( setup.config(), config );
    }

    void wrongFieldName()
    {
      const QgsVectorLayer vl( u"LineString?crs=epsg:3111&field=pk:int&field=col1:string"_s, u"vl"_s, u"memory"_s );
      const QgsEditorWidgetSetup setup = QgsGui::editorWidgetRegistry()->findBest( &vl, u"col2"_s );
      // an unknown fields leads to a default setup with a TextEdit
      QCOMPARE( setup.type(), QString( "TextEdit" ) );
      QCOMPARE( setup.config().count(), 0 );
    }

    void referencedLayers()
    {
      //build two layers
      QgsVectorLayer *vl1 = new QgsVectorLayer( u"LineString?crs=epsg:3111&field=pk:int&field=name:string&field=fk:int"_s, u"vl1"_s, u"memory"_s );
      QgsVectorLayer *vl2 = new QgsVectorLayer( u"LineString?crs=epsg:3111&field=pk:int&field=col1:string"_s, u"vl2"_s, u"memory"_s );
      QgsProject::instance()->addMapLayer( vl1 );
      QgsProject::instance()->addMapLayer( vl2 );

      //create a relation between them
      QgsRelation relation;
      relation.setId( u"vl1->vl2"_s );
      relation.setName( u"vl1->vl2"_s );
      relation.setReferencingLayer( vl1->id() );
      relation.setReferencedLayer( vl2->id() );
      relation.addFieldPair( u"fk"_s, u"pk"_s );
      QVERIFY( relation.isValid() );
      QgsProject::instance()->relationManager()->addRelation( relation );

      //check the guessed editor widget type for vl1.fk is RelationReference
      const QgsEditorWidgetSetup setup = QgsGui::editorWidgetRegistry()->findBest( vl1, u"fk"_s );
      QCOMPARE( setup.type(), QString( "RelationReference" ) );
      QCOMPARE( setup.config(), QVariantMap() );
    }

    void typeFromPlugin()
    {
      const QgsVectorLayer vl( u"LineString?crs=epsg:3111&field=pk:int&field=special:string"_s, u"vl"_s, u"memory"_s );
      const QgsEditorWidgetSetup setup = QgsGui::editorWidgetRegistry()->findBest( &vl, u"special"_s );
      QCOMPARE( setup.type(), QString( "Special" ) );
    }

  private:
    static void checkSimple( const QString &dataType, const QString &widgetType )
    {
      const QgsVectorLayer vl( "LineString?crs=epsg:3111&field=pk:int&field=col1:" + dataType, u"vl"_s, u"memory"_s );
      const QgsEditorWidgetSetup setup = QgsGui::editorWidgetRegistry()->findBest( &vl, u"col1"_s );
      QCOMPARE( setup.type(), widgetType );
      QCOMPARE( setup.config().count(), 0 );
    }
};

QGSTEST_MAIN( TestQgsEditorWidgetRegistry )
#include "testqgseditorwidgetregistry.moc"
