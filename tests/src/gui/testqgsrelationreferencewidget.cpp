/***************************************************************************
    testqgsrelationreferencewidget.cpp
     --------------------------------------
    Date                 : 21 07 2017
    Copyright            : (C) 2017 Paul Blottiere
    Email                : paul dot blottiere at oslandia dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/


#include "qgstest.h"

#include <editorwidgets/core/qgseditorwidgetregistry.h>
#include <qgsapplication.h>
#include "qgseditorwidgetwrapper.h"
#include <editorwidgets/qgsrelationreferencewidget.h>
#include <qgsproject.h>
#include <qgsattributeform.h>
#include <qgsrelationmanager.h>
#include <attributetable/qgsattributetablefiltermodel.h>
#include "qgsgui.h"

class TestQgsRelationReferenceWidget : public QObject
{
    Q_OBJECT
  public:
    TestQgsRelationReferenceWidget() {}

  private slots:
    void initTestCase(); // will be called before the first testfunction is executed.
    void cleanupTestCase(); // will be called after the last testfunction was executed.
    void init(); // will be called before each testfunction is executed.
    void cleanup(); // will be called after every testfunction.

    void testChainFilter();
};

void TestQgsRelationReferenceWidget::initTestCase()
{
  QgsApplication::init();
  QgsApplication::initQgis();
  QgsGui::editorWidgetRegistry()->initEditors();
}

void TestQgsRelationReferenceWidget::cleanupTestCase()
{
  QgsApplication::exitQgis();
}

void TestQgsRelationReferenceWidget::init()
{
}

void TestQgsRelationReferenceWidget::cleanup()
{
}

void TestQgsRelationReferenceWidget::testChainFilter()
{
  // create layers
  QgsVectorLayer vl1( QStringLiteral( "LineString?crs=epsg:3111&field=pk:int&field=fk:int" ), QStringLiteral( "vl1" ), QStringLiteral( "memory" ) );
  QgsVectorLayer vl2( QStringLiteral( "LineString?field=pk:int&field=material:string&field=diameter:int&field=raccord:string" ), QStringLiteral( "vl2" ), QStringLiteral( "memory" ) );
  QgsProject::instance()->addMapLayer( &vl1, false, false );
  QgsProject::instance()->addMapLayer( &vl2, false, false );

  // create a relation between them
  QgsRelation relation;
  relation.setId( QStringLiteral( "vl1.vl2" ) );
  relation.setName( QStringLiteral( "vl1.vl2" ) );
  relation.setReferencingLayer( vl1.id() );
  relation.setReferencedLayer( vl2.id() );
  relation.addFieldPair( "fk", "pk" );
  QVERIFY( relation.isValid() );
  QgsProject::instance()->relationManager()->addRelation( relation );

  // add features
  QgsFeature ft0( vl1.fields() );
  ft0.setAttribute( QStringLiteral( "pk" ), 0 );
  ft0.setAttribute( QStringLiteral( "fk" ), 0 );
  vl1.startEditing();
  vl1.addFeature( ft0 );
  vl1.commitChanges();

  QgsFeature ft1( vl1.fields() );
  ft1.setAttribute( QStringLiteral( "pk" ), 1 );
  ft1.setAttribute( QStringLiteral( "fk" ), 1 );
  vl1.startEditing();
  vl1.addFeature( ft1 );
  vl1.commitChanges();

  QgsFeature ft2( vl2.fields() );
  ft2.setAttribute( QStringLiteral( "pk" ), 10 );
  ft2.setAttribute( QStringLiteral( "material" ), "iron" );
  ft2.setAttribute( QStringLiteral( "diameter" ), 120 );
  ft2.setAttribute( QStringLiteral( "raccord" ), "brides" );
  vl2.startEditing();
  vl2.addFeature( ft2 );
  vl2.commitChanges();

  QgsFeature ft3( vl2.fields() );
  ft3.setAttribute( QStringLiteral( "pk" ), 11 );
  ft3.setAttribute( QStringLiteral( "material" ), "iron" );
  ft3.setAttribute( QStringLiteral( "diameter" ), 120 );
  ft3.setAttribute( QStringLiteral( "raccord" ), "sleeve" );
  vl2.startEditing();
  vl2.addFeature( ft3 );
  vl2.commitChanges();

  QgsFeature ft4( vl2.fields() );
  ft4.setAttribute( QStringLiteral( "pk" ), 12 );
  ft4.setAttribute( QStringLiteral( "material" ), "steel" );
  ft4.setAttribute( QStringLiteral( "diameter" ), 120 );
  ft4.setAttribute( QStringLiteral( "raccord" ), "collar" );
  vl2.startEditing();
  vl2.addFeature( ft4 );
  vl2.commitChanges();

  // init a relation reference widget
  QStringList filterFields = { "material", "diameter", "raccord" };

  QgsRelationReferenceWidget w( new QWidget() );
  w.setChainFilters( true );
  w.setFilterFields( filterFields );
  w.setRelation( relation, true );
  w.init();

  // check default status for comboboxes
  QList<QComboBox *> cbs = w.mFilterComboBoxes;
  QCOMPARE( cbs.count(), 3 );
  Q_FOREACH ( const QComboBox *cb, cbs )
  {
    if ( cb->currentText() == "raccord" )
      QCOMPARE( cb->count(), 5 );
    else if ( cb->currentText() == "material" )
      QCOMPARE( cb->count(), 4 );
    else if ( cb->currentText() == "diameter" )
      QCOMPARE( cb->count(), 3 );
  }

  // set first filter
  cbs[0]->setCurrentIndex( cbs[0]->findText( "iron" ) );
  cbs[1]->setCurrentIndex( cbs[1]->findText( "120" ) );

  Q_FOREACH ( const QComboBox *cb, cbs )
  {
    if ( cb->itemText( 0 ) == "material" )
      QCOMPARE( cb->count(), 4 );
    else if ( cb->itemText( 0 ) == "diameter" )
      QCOMPARE( cb->count(), 2 );
    else if ( cb->itemText( 0 ) == "raccord" )
    {
      QStringList items;
      for ( int i = 0; i < cb->count(); i++ )
        items << cb->itemText( i );

      QCOMPARE( cb->count(), 3 );
      QCOMPARE( items.contains( "collar" ), false );
      // collar should not be available in combobox as there's no existing
      // feature with the filter expression:
      // "material" ==  'iron' AND "diameter" == '120' AND "raccord" = 'collar'
    }
  }

  // set the filter for "raccord" and then reset filter for "diameter". As
  // chain filter is activated, the filter on "raccord" field should be reset
  cbs[2]->setCurrentIndex( cbs[2]->findText( "brides" ) );
  cbs[1]->setCurrentIndex( cbs[1]->findText( "diameter" ) );

  // combobox should propose NULL, 10 and 11 because the filter is now:
  // "material" == 'iron'
  QCOMPARE( w.mComboBox->count(), 3 );

  // if there's no filter at all, all features' id should be proposed
  cbs[0]->setCurrentIndex( cbs[0]->findText( "material" ) );
  QCOMPARE( w.mComboBox->count(), 4 );
}

QGSTEST_MAIN( TestQgsRelationReferenceWidget )
#include "testqgsrelationreferencewidget.moc"
