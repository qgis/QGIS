/***************************************************************************
    TestQgsRelationEditorWidget.cpp
     --------------------------------------
    Date                 : 19 11 2021
    Copyright            : (C) 2021 Damiano Lombardi
    Email                : damiano at opengis dot ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <memory>

#include "editorwidgets/core/qgseditorwidgetregistry.h"
#include "qgsapplication.h"
#include "qgsattributeform.h"
#include "qgsgui.h"
#include "qgsproject.h"
#include "qgsrelationeditorwidget.h"
#include "qgsrelationmanager.h"
#include "qgsrelationreferencewidget.h"
#include "qgstest.h"
#include "qgstrackedvectorlayertools.h"

#include <QTreeWidgetItem>

class TestQgsRelationEditorWidget : public QObject
{
    Q_OBJECT
  public:
    TestQgsRelationEditorWidget() = default;

  private slots:
    void initTestCase();    // will be called before the first testfunction is executed.
    void cleanupTestCase(); // will be called after the last testfunction was executed.
    void init();            // will be called before each testfunction is executed.
    void cleanup();         // will be called after every testfunction.

    void testMultiEdit1N();
    void testMultiEditNM();
    void testFeatureRequest();
    void testUpdateUi();

  private:
    std::unique_ptr<QgsVectorLayer> mLayer1;
    std::unique_ptr<QgsVectorLayer> mLayer2;
    std::unique_ptr<QgsVectorLayer> mLayerJoin;
    std::unique_ptr<QgsRelation> mRelation;
    std::unique_ptr<QgsRelation> mRelation1N;
    std::unique_ptr<QgsRelation> mRelationNM;
};

void TestQgsRelationEditorWidget::initTestCase()
{
  QgsApplication::init();
  QgsApplication::initQgis();
  QgsGui::editorWidgetRegistry()->initEditors();
}

void TestQgsRelationEditorWidget::cleanupTestCase()
{
  QgsApplication::exitQgis();
}

void TestQgsRelationEditorWidget::init()
{
  // create layer
  mLayer1 = std::make_unique<QgsVectorLayer>( u"LineString?field=pk:int&field=fk:int"_s, u"vl1"_s, u"memory"_s );
  mLayer1->setDisplayExpression( u"'Layer1-' || pk"_s );
  QgsProject::instance()->addMapLayer( mLayer1.get(), false, false );

  mLayer2 = std::make_unique<QgsVectorLayer>( u"LineString?field=pk:int"_s, u"vl2"_s, u"memory"_s );
  mLayer2->setDisplayExpression( u"'Layer2-' || pk"_s );
  QgsProject::instance()->addMapLayer( mLayer2.get(), false, false );

  mLayerJoin = std::make_unique<QgsVectorLayer>( u"LineString?field=pk:int&field=fk_layer1:int&field=fk_layer2:int"_s, u"join_layer"_s, u"memory"_s );
  mLayerJoin->setDisplayExpression( u"'LayerJoin-' || pk"_s );
  QgsProject::instance()->addMapLayer( mLayerJoin.get(), false, false );

  // create relation
  mRelation = std::make_unique<QgsRelation>();
  mRelation->setId( u"vl1.vl2"_s );
  mRelation->setName( u"vl1.vl2"_s );
  mRelation->setReferencingLayer( mLayer1->id() );
  mRelation->setReferencedLayer( mLayer2->id() );
  mRelation->addFieldPair( u"fk"_s, u"pk"_s );
  QVERIFY( mRelation->isValid() );
  QgsProject::instance()->relationManager()->addRelation( *mRelation );

  // create nm relations
  mRelation1N = std::make_unique<QgsRelation>();
  mRelation1N->setId( u"join_layer.vl1"_s );
  mRelation1N->setName( u"join_layer.vl1"_s );
  mRelation1N->setReferencingLayer( mLayerJoin->id() );
  mRelation1N->setReferencedLayer( mLayer1->id() );
  mRelation1N->addFieldPair( u"fk_layer1"_s, u"pk"_s );
  QVERIFY( mRelation1N->isValid() );
  QgsProject::instance()->relationManager()->addRelation( *mRelation1N );

  mRelationNM = std::make_unique<QgsRelation>();
  mRelationNM->setId( u"join_layer.vl2"_s );
  mRelationNM->setName( u"join_layer.vl2"_s );
  mRelationNM->setReferencingLayer( mLayerJoin->id() );
  mRelationNM->setReferencedLayer( mLayer2->id() );
  mRelationNM->addFieldPair( u"fk_layer2"_s, u"pk"_s );
  QVERIFY( mRelationNM->isValid() );
  QgsProject::instance()->relationManager()->addRelation( *mRelationNM );

  // add features
  QgsFeature ft0( mLayer1->fields() );
  ft0.setAttribute( u"pk"_s, 0 );
  ft0.setAttribute( u"fk"_s, 10 );
  mLayer1->startEditing();
  mLayer1->addFeature( ft0 );
  mLayer1->commitChanges();

  QgsFeature ft1( mLayer1->fields() );
  ft1.setAttribute( u"pk"_s, 1 );
  ft1.setAttribute( u"fk"_s, 11 );
  mLayer1->startEditing();
  mLayer1->addFeature( ft1 );
  mLayer1->commitChanges();

  QgsFeature ft2( mLayer2->fields() );
  ft2.setAttribute( u"pk"_s, 10 );
  mLayer2->startEditing();
  mLayer2->addFeature( ft2 );
  mLayer2->commitChanges();

  QgsFeature ft3( mLayer2->fields() );
  ft3.setAttribute( u"pk"_s, 11 );
  mLayer2->startEditing();
  mLayer2->addFeature( ft3 );
  mLayer2->commitChanges();

  QgsFeature ft4( mLayer2->fields() );
  ft4.setAttribute( u"pk"_s, 12 );
  mLayer2->startEditing();
  mLayer2->addFeature( ft4 );
  mLayer2->commitChanges();

  // Add join features
  QgsFeature jft1( mLayerJoin->fields() );
  jft1.setAttribute( u"pk"_s, 101 );
  jft1.setAttribute( u"fk_layer1"_s, 0 );
  jft1.setAttribute( u"fk_layer2"_s, 10 );
  mLayerJoin->startEditing();
  mLayerJoin->addFeature( jft1 );
  mLayerJoin->commitChanges();

  QgsFeature jft2( mLayerJoin->fields() );
  jft2.setAttribute( u"pk"_s, 102 );
  jft2.setAttribute( u"fk_layer1"_s, 1 );
  jft2.setAttribute( u"fk_layer2"_s, 11 );
  mLayerJoin->startEditing();
  mLayerJoin->addFeature( jft2 );
  mLayerJoin->commitChanges();

  QgsFeature jft3( mLayerJoin->fields() );
  jft3.setAttribute( u"pk"_s, 103 );
  jft3.setAttribute( u"fk_layer1"_s, 0 );
  jft3.setAttribute( u"fk_layer2"_s, 11 );
  mLayerJoin->startEditing();
  mLayerJoin->addFeature( jft3 );
  mLayerJoin->commitChanges();
}

void TestQgsRelationEditorWidget::cleanup()
{
  QgsProject::instance()->removeMapLayer( mLayer1.get() );
  QgsProject::instance()->removeMapLayer( mLayer2.get() );
  QgsProject::instance()->removeMapLayer( mLayerJoin.get() );
}

void TestQgsRelationEditorWidget::testMultiEdit1N()
{
  // Init a relation editor widget
  QgsRelationEditorWidget relationEditorWidget( QVariantMap(), new QWidget() );
  relationEditorWidget.setRelations( *mRelation, QgsRelation() );

  QVERIFY( !relationEditorWidget.multiEditModeActive() );

  QgsFeatureIds featureIds;
  QgsFeatureIterator featureIterator = mLayer2->getFeatures();
  QgsFeature feature;
  while ( featureIterator.nextFeature( feature ) )
    featureIds.insert( feature.id() );
  relationEditorWidget.setMultiEditFeatureIds( featureIds );

  // Update ui
  relationEditorWidget.updateUiMultiEdit();

  QVERIFY( relationEditorWidget.multiEditModeActive() );

  QSet<QString> setParentItemsText;
  QSet<QString> setChildrenItemsText;
  for ( int parentIndex = 0; parentIndex < relationEditorWidget.mMultiEditTreeWidget->topLevelItemCount(); ++parentIndex )
  {
    QTreeWidgetItem *parentItem = relationEditorWidget.mMultiEditTreeWidget->topLevelItem( parentIndex );
    setParentItemsText.insert( parentItem->text( 0 ) );
    QCOMPARE( parentItem->data( 0, static_cast<int>( QgsRelationEditorWidget::MultiEditTreeWidgetRole::FeatureType ) ).toInt(), static_cast<int>( QgsRelationEditorWidget::MultiEditFeatureType::Parent ) );
    for ( int childIndex = 0; childIndex < parentItem->childCount(); ++childIndex )
    {
      QTreeWidgetItem *childItem = parentItem->child( childIndex );
      setChildrenItemsText.insert( childItem->text( 0 ) );
      QCOMPARE( childItem->data( 0, static_cast<int>( QgsRelationEditorWidget::MultiEditTreeWidgetRole::FeatureType ) ).toInt(), static_cast<int>( QgsRelationEditorWidget::MultiEditFeatureType::Child ) );

      if ( childItem->text( 0 ) == "Layer1-0"_L1 )
        QCOMPARE( parentItem->text( 0 ), u"Layer2-10"_s );

      if ( childItem->text( 0 ) == "Layer1-1"_L1 )
        QCOMPARE( parentItem->text( 0 ), u"Layer2-11"_s );
    }
  }

  QCOMPARE( setParentItemsText, QSet<QString>() << u"Layer2-10"_s << u"Layer2-11"_s << u"Layer2-12"_s );

  QCOMPARE( setChildrenItemsText, QSet<QString>() << u"Layer1-0"_s << u"Layer1-1"_s );
}

void TestQgsRelationEditorWidget::testMultiEditNM()
{
  // Init a relation editor widget
  QgsRelationEditorWidget relationEditorWidget( QVariantMap(), new QWidget() );
  relationEditorWidget.setRelations( *mRelation1N, *mRelationNM );

  QVERIFY( !relationEditorWidget.multiEditModeActive() );

  QgsFeatureIds featureIds;
  QgsFeatureIterator featureIterator = mLayer1->getFeatures();
  QgsFeature feature;
  while ( featureIterator.nextFeature( feature ) )
    featureIds.insert( feature.id() );
  relationEditorWidget.setMultiEditFeatureIds( featureIds );

  // Update ui
  relationEditorWidget.updateUiMultiEdit();

  QVERIFY( relationEditorWidget.multiEditModeActive() );

  QSet<QString> setParentItemsText;
  QStringList listChildrenItemsText;
  for ( int parentIndex = 0; parentIndex < relationEditorWidget.mMultiEditTreeWidget->topLevelItemCount(); ++parentIndex )
  {
    QTreeWidgetItem *parentItem = relationEditorWidget.mMultiEditTreeWidget->topLevelItem( parentIndex );
    setParentItemsText.insert( parentItem->text( 0 ) );
    QCOMPARE( parentItem->data( 0, static_cast<int>( QgsRelationEditorWidget::MultiEditTreeWidgetRole::FeatureType ) ).toInt(), static_cast<int>( QgsRelationEditorWidget::MultiEditFeatureType::Parent ) );
    for ( int childIndex = 0; childIndex < parentItem->childCount(); ++childIndex )
    {
      QTreeWidgetItem *childItem = parentItem->child( childIndex );
      listChildrenItemsText.append( childItem->text( 0 ) );
      QCOMPARE( childItem->data( 0, static_cast<int>( QgsRelationEditorWidget::MultiEditTreeWidgetRole::FeatureType ) ).toInt(), static_cast<int>( QgsRelationEditorWidget::MultiEditFeatureType::Child ) );

      if ( childItem->text( 0 ) == "Layer2-10"_L1 )
        QCOMPARE( parentItem->text( 0 ), u"Layer1-0"_s );

      if ( childItem->text( 0 ) == "Layer2-11"_L1 )
      {
        QStringList possibleParents;
        possibleParents << u"Layer1-0"_s
                        << u"Layer1-1"_s;
        QVERIFY( possibleParents.contains( parentItem->text( 0 ) ) );
      }
    }
  }

  QCOMPARE( setParentItemsText, QSet<QString>() << u"Layer1-0"_s << u"Layer1-1"_s );

  listChildrenItemsText.sort();
  QCOMPARE( listChildrenItemsText, QStringList() << u"Layer2-10"_s << u"Layer2-11"_s << u"Layer2-11"_s );
}

void TestQgsRelationEditorWidget::testFeatureRequest()
{
  // Init a relation editor widget
  QgsRelationEditorWidget relationEditorWidget( QVariantMap(), new QWidget() );
  relationEditorWidget.setRelations( *mRelation1N, *mRelationNM );

  QVERIFY( !relationEditorWidget.multiEditModeActive() );

  QgsFeatureIterator featureIterator = mLayer1->getFeatures();
  QgsFeature feature;
  featureIterator.nextFeature( feature );
  relationEditorWidget.setFeature( feature );

  QgsAttributeEditorContext context;
  QgsTrackedVectorLayerTools tools;
  context.setVectorLayerTools( &tools );
  relationEditorWidget.setEditorContext( context );

  relationEditorWidget.updateUiSingleEdit();
  QCOMPARE( relationEditorWidget.mDualView->masterModel()->request().filterExpression()->expression(), u"\"pk\" IN (10,11)"_s );
}

void TestQgsRelationEditorWidget::testUpdateUi()
{
  // Test that we don't recreate all the widget when only the request have been updated

  QgsTrackedVectorLayerTools tools;

  // Init a relation editor widget
  QgsRelationEditorWidget relationEditorWidget( ( QVariantMap() ) );
  relationEditorWidget.setRelations( *mRelation, QgsRelation() );
  QgsAttributeEditorContext context;
  context.setVectorLayerTools( &tools );
  relationEditorWidget.setEditorContext( context );

  QVERIFY( !relationEditorWidget.multiEditModeActive() );

  relationEditorWidget.setVisible( true );

  //get feature with pk 10
  QgsFeature feat = mLayer2->getFeature( 1 );
  QVERIFY( feat.isValid() );
  relationEditorWidget.setFeature( feat );

  QVERIFY( relationEditorWidget.mDualView );

  QPointer<QAbstractItemModel> model = relationEditorWidget.mDualView->masterModel();
  QVERIFY( model );

  QCOMPARE( model->rowCount(), 1 );
  //referencing feature on mLayer1 should be pk 0 (fid 1)
  QCOMPARE( model->data( model->index( 0, 0 ) ), 0 );
  //as well be found with FilteredSelectionManager (fid 1)
  relationEditorWidget.relation().referencingLayer()->selectAll();
  QVERIFY( relationEditorWidget.selectedChildFeatureIds().contains( 1 ) );

  //get feature with pk 11
  feat = mLayer2->getFeature( 2 );
  QVERIFY( feat.isValid() );
  relationEditorWidget.setFeature( feat );

  // model hasn't changed (old one has not being destroyed)
  QVERIFY( model );
  QCOMPARE( model->rowCount(), 1 );
  //referencing feature on mLayer1 should be pk 1 (fid 2)
  QCOMPARE( model->data( model->index( 0, 0 ) ), 1 );
  //as well be found with FilteredSelectionManager (fid 2)
  relationEditorWidget.relation().referencingLayer()->selectAll();
  QVERIFY( relationEditorWidget.selectedChildFeatureIds().contains( 2 ) );

  // Now try with NM relations
  relationEditorWidget.setRelations( *mRelationNM, *mRelation1N );
  // model has not been destroyed (mLayer1 is still the displayed layer)
  QVERIFY( model );

  //get feature with pk 10
  feat = mLayer2->getFeature( 1 );
  QVERIFY( feat.isValid() );
  relationEditorWidget.setFeature( feat );

  // model not destroyed, set request only
  QCOMPARE( model->rowCount(), 1 );
  //referencing feature on mLayer1 should be pk 0 (fid 1)
  QCOMPARE( model->data( model->index( 0, 0 ) ), 0 );
  //as well be found with FilteredSelectionManager (fid 1)
  relationEditorWidget.nmRelation().referencedLayer()->selectAll();
  QVERIFY( relationEditorWidget.selectedChildFeatureIds().contains( 1 ) );

  relationEditorWidget.setRelations( *mRelation1N, *mRelationNM );
  // model has been destroyed (mLayer2 is now the displayed layer)
  QVERIFY( !model );
  model = relationEditorWidget.mDualView->masterModel();

  //get feature with pk 0
  feat = mLayer1->getFeature( 1 );
  QVERIFY( feat.isValid() );
  relationEditorWidget.setFeature( feat );

  // model not destroyed, set request only
  QVERIFY( model );
  QCOMPARE( model->rowCount(), 2 );
  //referencing feature on mLayer2 should be pk 10 (fid 1) and pk 11 (fid 2)
  QCOMPARE( model->data( model->index( 0, 0 ) ), 10 );
  QCOMPARE( model->data( model->index( 1, 0 ) ), 11 );

  //as well be found with FilteredSelectionManager (fid 1 and fid 2)
  relationEditorWidget.nmRelation().referencedLayer()->selectAll();
  QVERIFY( relationEditorWidget.selectedChildFeatureIds().contains( 1 ) );
  QVERIFY( relationEditorWidget.selectedChildFeatureIds().contains( 2 ) );
}

QGSTEST_MAIN( TestQgsRelationEditorWidget )
#include "testqgsrelationeditorwidget.moc"
