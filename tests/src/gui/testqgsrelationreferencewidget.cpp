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

#include <QtTest/QtTest>
#include <editorwidgets/core/qgseditorwidgetregistry.h>
#include <qgsapplication.h>
#include "qgseditorwidgetwrapper.h"
#include <qgsmaplayerregistry.h>
#include <editorwidgets/qgsrelationreferencewidget.h>
#include <qgsproject.h>
#include <qgsattributeform.h>
#include <qgsrelationmanager.h>
#include <attributetable/qgsattributetablefiltermodel.h>

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
    void testChainFilterRefreshed();

  private:
    QgsVectorLayer* mLayer1;
    QgsVectorLayer* mLayer2;
    std::unique_ptr<QgsRelation> mRelation;
};

void TestQgsRelationReferenceWidget::initTestCase()
{
  QgsApplication::init();
  QgsApplication::initQgis();
  QgsEditorWidgetRegistry::initEditors();
}

void TestQgsRelationReferenceWidget::cleanupTestCase()
{
  QgsApplication::exitQgis();
}

void TestQgsRelationReferenceWidget::init()
{
  // create layer
  mLayer1 = new QgsVectorLayer( QString( "LineString?crs=epsg:3111&field=pk:int&field=fk:int" ), QString( "vl1" ), QString( "memory" ) );
  QgsMapLayerRegistry::instance()->addMapLayer( mLayer1 );

  mLayer2 = new QgsVectorLayer( QString( "LineString?field=pk:int&field=material:string&field=diameter:int&field=raccord:string" ), QString( "vl2" ), QString( "memory" ) );
  QgsMapLayerRegistry::instance()->addMapLayer( mLayer2 );

  // create relation
  mRelation.reset( new QgsRelation() );
  mRelation->setRelationId( QString( "vl1.vl2" ) );
  mRelation->setRelationName( QString( "vl1.vl2" ) );
  mRelation->setReferencingLayer( mLayer1->id() );
  mRelation->setReferencedLayer( mLayer2->id() );
  mRelation->addFieldPair( "fk", "pk" );
  QVERIFY( mRelation->isValid() );
  QgsProject::instance()->relationManager()->addRelation( *mRelation.get() );

  // add features
  QgsFeature ft0( mLayer1->fields() );
  ft0.setAttribute( QString( "pk" ), 0 );
  ft0.setAttribute( QString( "fk" ), 0 );
  mLayer1->startEditing();
  mLayer1->addFeature( ft0 );
  mLayer1->commitChanges();

  QgsFeature ft1( mLayer1->fields() );
  ft1.setAttribute( QString( "pk" ), 1 );
  ft1.setAttribute( QString( "fk" ), 1 );
  mLayer1->startEditing();
  mLayer1->addFeature( ft1 );
  mLayer1->commitChanges();

  QgsFeature ft2( mLayer2->fields() );
  ft2.setAttribute( QString( "pk" ), 10 );
  ft2.setAttribute( QString( "material" ), "iron" );
  ft2.setAttribute( QString( "diameter" ), 120 );
  ft2.setAttribute( QString( "raccord" ), "brides" );
  mLayer2->startEditing();
  mLayer2->addFeature( ft2 );
  mLayer2->commitChanges();

  QgsFeature ft3( mLayer2->fields() );
  ft3.setAttribute( QString( "pk" ), 11 );
  ft3.setAttribute( QString( "material" ), "iron" );
  ft3.setAttribute( QString( "diameter" ), 120 );
  ft3.setAttribute( QString( "raccord" ), "sleeve" );
  mLayer2->startEditing();
  mLayer2->addFeature( ft3 );
  mLayer2->commitChanges();

  QgsFeature ft4( mLayer2->fields() );
  ft4.setAttribute( QString( "pk" ), 12 );
  ft4.setAttribute( QString( "material" ), "steel" );
  ft4.setAttribute( QString( "diameter" ), 120 );
  ft4.setAttribute( QString( "raccord" ), "collar" );
  mLayer2->startEditing();
  mLayer2->addFeature( ft4 );
  mLayer2->commitChanges();
}

void TestQgsRelationReferenceWidget::cleanup()
{
}

void TestQgsRelationReferenceWidget::testChainFilter()
{
  // init a relation reference widget
  QStringList filterFields = QStringList() << "material" << "diameter" << "raccord";

  QgsRelationReferenceWidget w( new QWidget() );
  w.setChainFilters( true );
  w.setFilterFields( filterFields );
  w.setRelation( *mRelation, true );
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
      QCOMPARE(( bool )items.contains( "collar" ), false );
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

void TestQgsRelationReferenceWidget::testChainFilterRefreshed()
{
  // init a relation reference widget
  QStringList filterFields = QStringList() << "material" << "diameter" << "raccord";

  QgsRelationReferenceWidget w( new QWidget() );
  w.setChainFilters( true );
  w.setFilterFields( filterFields );
  w.setRelation( *mRelation, true );
  w.init();

  // check default status for comboboxes
  QList<QComboBox *> cbs = w.mFilterComboBoxes;
  QCOMPARE( cbs.count(), 3 );
  QCOMPARE( cbs[0]->currentText(), QString( "material" ) );
  QCOMPARE( cbs[1]->currentText(), QString( "diameter" ) );
  QCOMPARE( cbs[2]->currentText(), QString( "raccord" ) );

  // update foreign key
  w.setForeignKey( QVariant( 12 ) );
  QCOMPARE( cbs[0]->currentText(), QString( "steel" ) );
  QCOMPARE( cbs[1]->currentText(), QString( "120" ) );
  QCOMPARE( cbs[2]->currentText(), QString( "collar" ) );

  w.setForeignKey( QVariant( 10 ) );
  QCOMPARE( cbs[0]->currentText(), QString( "iron" ) );
  QCOMPARE( cbs[1]->currentText(), QString( "120" ) );
  QCOMPARE( cbs[2]->currentText(), QString( "brides" ) );

  w.setForeignKey( QVariant( 11 ) );
  QCOMPARE( cbs[0]->currentText(), QString( "iron" ) );
  QCOMPARE( cbs[1]->currentText(), QString( "120" ) );
  QCOMPARE( cbs[2]->currentText(), QString( "sleeve" ) );
}

QTEST_MAIN( TestQgsRelationReferenceWidget )
#include "testqgsrelationreferencewidget.moc"
