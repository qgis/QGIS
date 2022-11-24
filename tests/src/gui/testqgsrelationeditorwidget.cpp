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

#include <qgstest.h>

#include <editorwidgets/core/qgseditorwidgetregistry.h>
#include <qgsapplication.h>
#include <qgsattributeform.h>
#include <qgsgui.h>
#include <qgsproject.h>
#include <qgsrelationeditorwidget.h>
#include <qgsrelationmanager.h>
#include <qgsrelationreferencewidget.h>
#include <qgstrackedvectorlayertools.h>

#include <QTreeWidgetItem>

class TestQgsRelationEditorWidget : public QObject
{
    Q_OBJECT
  public:
    TestQgsRelationEditorWidget() = default;

  private slots:
    void initTestCase(); // will be called before the first testfunction is executed.
    void cleanupTestCase(); // will be called after the last testfunction was executed.
    void init(); // will be called before each testfunction is executed.
    void cleanup(); // will be called after every testfunction.

    void testMultiEdit1N();
    void testMultiEditNM();
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
  mLayer1.reset( new QgsVectorLayer( QStringLiteral( "LineString?field=pk:int&field=fk:int" ), QStringLiteral( "vl1" ), QStringLiteral( "memory" ) ) );
  mLayer1->setDisplayExpression( QStringLiteral( "'Layer1-' || pk" ) );
  QgsProject::instance()->addMapLayer( mLayer1.get(), false, false );

  mLayer2.reset( new QgsVectorLayer( QStringLiteral( "LineString?field=pk:int" ), QStringLiteral( "vl2" ), QStringLiteral( "memory" ) ) );
  mLayer2->setDisplayExpression( QStringLiteral( "'Layer2-' || pk" ) );
  QgsProject::instance()->addMapLayer( mLayer2.get(), false, false );

  mLayerJoin.reset( new QgsVectorLayer( QStringLiteral( "LineString?field=pk:int&field=fk_layer1:int&field=fk_layer2:int" ), QStringLiteral( "join_layer" ), QStringLiteral( "memory" ) ) );
  mLayerJoin->setDisplayExpression( QStringLiteral( "'LayerJoin-' || pk" ) );
  QgsProject::instance()->addMapLayer( mLayerJoin.get(), false, false );

  // create relation
  mRelation.reset( new QgsRelation() );
  mRelation->setId( QStringLiteral( "vl1.vl2" ) );
  mRelation->setName( QStringLiteral( "vl1.vl2" ) );
  mRelation->setReferencingLayer( mLayer1->id() );
  mRelation->setReferencedLayer( mLayer2->id() );
  mRelation->addFieldPair( QStringLiteral( "fk" ), QStringLiteral( "pk" ) );
  QVERIFY( mRelation->isValid() );
  QgsProject::instance()->relationManager()->addRelation( *mRelation );

  // create nm relations
  mRelation1N.reset( new QgsRelation() );
  mRelation1N->setId( QStringLiteral( "join_layer.vl1" ) );
  mRelation1N->setName( QStringLiteral( "join_layer.vl1" ) );
  mRelation1N->setReferencingLayer( mLayerJoin->id() );
  mRelation1N->setReferencedLayer( mLayer1->id() );
  mRelation1N->addFieldPair( QStringLiteral( "fk_layer1" ), QStringLiteral( "pk" ) );
  QVERIFY( mRelation1N->isValid() );
  QgsProject::instance()->relationManager()->addRelation( *mRelation1N );

  mRelationNM.reset( new QgsRelation() );
  mRelationNM->setId( QStringLiteral( "join_layer.vl2" ) );
  mRelationNM->setName( QStringLiteral( "join_layer.vl2" ) );
  mRelationNM->setReferencingLayer( mLayerJoin->id() );
  mRelationNM->setReferencedLayer( mLayer2->id() );
  mRelationNM->addFieldPair( QStringLiteral( "fk_layer2" ), QStringLiteral( "pk" ) );
  QVERIFY( mRelationNM->isValid() );
  QgsProject::instance()->relationManager()->addRelation( *mRelationNM );

  // add features
  QgsFeature ft0( mLayer1->fields() );
  ft0.setAttribute( QStringLiteral( "pk" ), 0 );
  ft0.setAttribute( QStringLiteral( "fk" ), 10 );
  mLayer1->startEditing();
  mLayer1->addFeature( ft0 );
  mLayer1->commitChanges();

  QgsFeature ft1( mLayer1->fields() );
  ft1.setAttribute( QStringLiteral( "pk" ), 1 );
  ft1.setAttribute( QStringLiteral( "fk" ), 11 );
  mLayer1->startEditing();
  mLayer1->addFeature( ft1 );
  mLayer1->commitChanges();

  QgsFeature ft2( mLayer2->fields() );
  ft2.setAttribute( QStringLiteral( "pk" ), 10 );
  mLayer2->startEditing();
  mLayer2->addFeature( ft2 );
  mLayer2->commitChanges();

  QgsFeature ft3( mLayer2->fields() );
  ft3.setAttribute( QStringLiteral( "pk" ), 11 );
  mLayer2->startEditing();
  mLayer2->addFeature( ft3 );
  mLayer2->commitChanges();

  QgsFeature ft4( mLayer2->fields() );
  ft4.setAttribute( QStringLiteral( "pk" ), 12 );
  mLayer2->startEditing();
  mLayer2->addFeature( ft4 );
  mLayer2->commitChanges();

  // Add join features
  QgsFeature jft1( mLayerJoin->fields() );
  jft1.setAttribute( QStringLiteral( "pk" ), 101 );
  jft1.setAttribute( QStringLiteral( "fk_layer1" ), 0 );
  jft1.setAttribute( QStringLiteral( "fk_layer2" ), 10 );
  mLayerJoin->startEditing();
  mLayerJoin->addFeature( jft1 );
  mLayerJoin->commitChanges();

  QgsFeature jft2( mLayerJoin->fields() );
  jft2.setAttribute( QStringLiteral( "pk" ), 102 );
  jft2.setAttribute( QStringLiteral( "fk_layer1" ), 1 );
  jft2.setAttribute( QStringLiteral( "fk_layer2" ), 11 );
  mLayerJoin->startEditing();
  mLayerJoin->addFeature( jft2 );
  mLayerJoin->commitChanges();

  QgsFeature jft3( mLayerJoin->fields() );
  jft3.setAttribute( QStringLiteral( "pk" ), 102 );
  jft3.setAttribute( QStringLiteral( "fk_layer1" ), 0 );
  jft3.setAttribute( QStringLiteral( "fk_layer2" ), 11 );
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
  QgsRelationEditorWidget relationEditorWidget( QVariantMap(),
      new QWidget() );
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
    QCOMPARE( parentItem->data( 0, static_cast<int>( QgsRelationEditorWidget::MultiEditTreeWidgetRole::FeatureType ) ).toInt(),
              static_cast<int>( QgsRelationEditorWidget::MultiEditFeatureType::Parent ) );
    for ( int childIndex = 0; childIndex < parentItem->childCount(); ++childIndex )
    {
      QTreeWidgetItem *childItem = parentItem->child( childIndex );
      setChildrenItemsText.insert( childItem->text( 0 ) );
      QCOMPARE( childItem->data( 0, static_cast<int>( QgsRelationEditorWidget::MultiEditTreeWidgetRole::FeatureType ) ).toInt(),
                static_cast<int>( QgsRelationEditorWidget::MultiEditFeatureType::Child ) );

      if ( childItem->text( 0 ) == QLatin1String( "Layer1-0" ) )
        QCOMPARE( parentItem->text( 0 ), QStringLiteral( "Layer2-10" ) );

      if ( childItem->text( 0 ) == QLatin1String( "Layer1-1" ) )
        QCOMPARE( parentItem->text( 0 ), QStringLiteral( "Layer2-11" ) );
    }
  }

  QCOMPARE( setParentItemsText, QSet<QString>() << QStringLiteral( "Layer2-10" )
            << QStringLiteral( "Layer2-11" )
            << QStringLiteral( "Layer2-12" ) );

  QCOMPARE( setChildrenItemsText, QSet<QString>() << QStringLiteral( "Layer1-0" )
            << QStringLiteral( "Layer1-1" ) );
}

void TestQgsRelationEditorWidget::testMultiEditNM()
{
  // Init a relation editor widget
  QgsRelationEditorWidget relationEditorWidget( QVariantMap(),
      new QWidget() );
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
    QCOMPARE( parentItem->data( 0, static_cast<int>( QgsRelationEditorWidget::MultiEditTreeWidgetRole::FeatureType ) ).toInt(),
              static_cast<int>( QgsRelationEditorWidget::MultiEditFeatureType::Parent ) );
    for ( int childIndex = 0; childIndex < parentItem->childCount(); ++childIndex )
    {
      QTreeWidgetItem *childItem = parentItem->child( childIndex );
      listChildrenItemsText.append( childItem->text( 0 ) );
      QCOMPARE( childItem->data( 0, static_cast<int>( QgsRelationEditorWidget::MultiEditTreeWidgetRole::FeatureType ) ).toInt(),
                static_cast<int>( QgsRelationEditorWidget::MultiEditFeatureType::Child ) );

      if ( childItem->text( 0 ) == QLatin1String( "Layer2-10" ) )
        QCOMPARE( parentItem->text( 0 ), QStringLiteral( "Layer1-0" ) );

      if ( childItem->text( 0 ) == QLatin1String( "Layer2-11" ) )
      {
        QStringList possibleParents;
        possibleParents << QStringLiteral( "Layer1-0" )
                        << QStringLiteral( "Layer1-1" );
        QVERIFY( possibleParents.contains( parentItem->text( 0 ) ) );
      }
    }
  }

  QCOMPARE( setParentItemsText, QSet<QString>() << QStringLiteral( "Layer1-0" )
            << QStringLiteral( "Layer1-1" ) );

  listChildrenItemsText.sort();
  QCOMPARE( listChildrenItemsText, QStringList() << QStringLiteral( "Layer2-10" )
            << QStringLiteral( "Layer2-11" )
            << QStringLiteral( "Layer2-11" ) );

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

  QgsFeature feat = mLayer2->getFeature( 1 );
  QVERIFY( feat.isValid() );
  relationEditorWidget.setFeature( feat );

  QVERIFY( relationEditorWidget.mDualView );

  QPointer<QAbstractItemModel> model = relationEditorWidget.mDualView->masterModel();
  QVERIFY( model );

  QCOMPARE( model->rowCount(), 1 );
  QCOMPARE( model->data( model->index( 0, 0 ) ), 0 );

  feat = mLayer2->getFeature( 2 );
  QVERIFY( feat.isValid() );
  relationEditorWidget.setFeature( feat );

  // model hasn't changed (old one has not being destroyed)
  QVERIFY( model );
  QCOMPARE( model->rowCount(), 1 );
  QCOMPARE( model->data( model->index( 0, 0 ) ), 1 );

  // Now try with NM relations
  relationEditorWidget.setRelations( *mRelationNM, *mRelation1N );
  // model has not been destroyed (mLayer1 is still the displayed layer)
  QVERIFY( model );

  feat = mLayer2->getFeature( 1 );
  QVERIFY( feat.isValid() );
  relationEditorWidget.setFeature( feat );

  // model not destroyed, set request only
  QCOMPARE( model->rowCount(), 1 );
  QCOMPARE( model->data( model->index( 0, 0 ) ), 0 );

  relationEditorWidget.setRelations( *mRelation1N, *mRelationNM );
  // model has been destroyed (mLayer2 is now the displayed layer)
  QVERIFY( !model );
  model = relationEditorWidget.mDualView->masterModel();

  feat = mLayer1->getFeature( 1 );
  QVERIFY( feat.isValid() );
  relationEditorWidget.setFeature( feat );

  // model not destroyed, set request only
  QVERIFY( model );
  QCOMPARE( model->rowCount(), 2 );
  QCOMPARE( model->data( model->index( 0, 0 ) ), 10 );
  QCOMPARE( model->data( model->index( 1, 0 ) ), 11 );
}

QGSTEST_MAIN( TestQgsRelationEditorWidget )
#include "testqgsrelationeditorwidget.moc"
