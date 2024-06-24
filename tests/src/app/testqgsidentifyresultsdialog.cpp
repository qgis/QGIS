/***************************************************************************
     testqgsidentifyresultsdialog.cpp
     --------------------------------
    Date                 : 2024-06-21
    Copyright            : (C) 2024 by Even Rouault
    Email                : even.rouault at spatialys.com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgstest.h"

#include "qgisapp.h"
#include "qgsapplication.h"
#include "qgsidentifyresultsdialog.h"
#include "qgsmapcanvas.h"
#include "qgsproject.h"
#include "qgsrelation.h"
#include "qgsrelationmanager.h"
#include "qgsvectorlayer.h"

class TestQgsIdentifyResultsDialog : public QObject
{
    Q_OBJECT
  public:
    TestQgsIdentifyResultsDialog() = default;

  private slots:
    void initTestCase(); // will be called before the first testfunction is executed.
    void cleanupTestCase(); // will be called after the last testfunction was executed.
    void init(); // will be called before each testfunction is executed.
    void cleanup(); // will be called after every testfunction.

    void testRelations();

  private:

    QgsMapCanvas *mCanvas = nullptr;
    QgisApp *mQgisApp = nullptr;
};

void TestQgsIdentifyResultsDialog::initTestCase()
{
  QgsApplication::init();
  QgsApplication::initQgis();
  // Set up the QgsSettings environment
  QCoreApplication::setOrganizationName( QStringLiteral( "QGIS" ) );
  QCoreApplication::setOrganizationDomain( QStringLiteral( "qgis.org" ) );
  QCoreApplication::setApplicationName( QStringLiteral( "QGIS-TEST" ) );

  QgsApplication::showSettings();

  // enforce C locale because the tests expect it
  // (decimal separators / thousand separators)
  QLocale::setDefault( QLocale::c() );

  mQgisApp = new QgisApp();
}

void TestQgsIdentifyResultsDialog::cleanupTestCase()
{
  QgsApplication::exitQgis();
}

void TestQgsIdentifyResultsDialog::init()
{
  mCanvas = new QgsMapCanvas();
}

void TestQgsIdentifyResultsDialog::cleanup()
{
  delete mCanvas;
}

void TestQgsIdentifyResultsDialog::testRelations()
{
  QgsVectorLayer *layerA = new QgsVectorLayer( QStringLiteral( "Point?crs=epsg:4326&field=pk_id:integer" ), QStringLiteral( "layerA" ), QStringLiteral( "memory" ) );
  QVERIFY( layerA->isValid() );
  QgsFeature featureA( layerA->dataProvider()->fields() );
  constexpr int PK_ID_A = 1;
  constexpr int PK_ID_C = 2;
  featureA.setAttribute( 0, PK_ID_A );
  layerA->dataProvider()->addFeature( featureA );

  QgsVectorLayer *layerB = new QgsVectorLayer( QStringLiteral( "Point?crs=epsg:4326&field=fk_id_to_A:integer&field=fk_id_to_C:integer&field=other_field:integer" ), QStringLiteral( "layerB" ), QStringLiteral( "memory" ) );
  QVERIFY( layerB->isValid() );
  constexpr int IDX_OTHER_FIELD = 2;
  constexpr int OTHER_FIELD = 100;
  {
    QgsFeature featureB( layerB->dataProvider()->fields() );
    featureB.setAttribute( 0, PK_ID_A );
    featureB.setAttribute( 1, PK_ID_C );
    featureB.setAttribute( IDX_OTHER_FIELD, OTHER_FIELD );
    layerB->dataProvider()->addFeature( featureB );
  }
  {
    QgsFeature featureB( layerB->dataProvider()->fields() );
    featureB.setAttribute( 0, PK_ID_A );
    featureB.setAttribute( 1, PK_ID_C + 1 );
    featureB.setAttribute( IDX_OTHER_FIELD, OTHER_FIELD + 1 );
    layerB->dataProvider()->addFeature( featureB );
  }

  QgsVectorLayer *layerC = new QgsVectorLayer( QStringLiteral( "Point?crs=epsg:4326&field=pk_id:integer" ), QStringLiteral( "layerC" ), QStringLiteral( "memory" ) );
  QVERIFY( layerC->isValid() );
  {
    QgsFeature featureC( layerC->dataProvider()->fields() );
    featureC.setAttribute( 0, PK_ID_C );
    layerC->dataProvider()->addFeature( featureC );
  }
  {
    QgsFeature featureC( layerC->dataProvider()->fields() );
    featureC.setAttribute( 0, PK_ID_C + 1 );
    layerC->dataProvider()->addFeature( featureC );
  }

  QgsProject::instance()->layerStore()->addMapLayer( layerA, true );
  QgsProject::instance()->layerStore()->addMapLayer( layerB, true );
  QgsProject::instance()->layerStore()->addMapLayer( layerC, true );

  QgsRelationManager *relationManager = QgsProject::instance()->relationManager();
  {
    QgsRelation relation;
    relation.setId( "B-A-id" );
    relation.setName( "B-A" );
    relation.setReferencingLayer( layerB->id() );
    relation.setReferencedLayer( layerA->id() );
    relation.addFieldPair( QStringLiteral( "fk_id_to_A" ), QStringLiteral( "pk_id" ) );

    relationManager->addRelation( relation );
  }
  {
    QgsRelation relation;
    relation.setId( "A-B-id" );
    relation.setName( "A-B" );
    relation.setReferencingLayer( layerA->id() );
    relation.setReferencedLayer( layerB->id() );
    relation.addFieldPair( QStringLiteral( "pk_id" ), QStringLiteral( "fk_id_to_A" ) );

    relationManager->addRelation( relation );
  }
  {
    QgsRelation relation;
    relation.setId( "B-C-id" );
    relation.setName( "B-C" );
    relation.setReferencingLayer( layerB->id() );
    relation.setReferencedLayer( layerC->id() );
    relation.addFieldPair( QStringLiteral( "fk_id_to_C" ), QStringLiteral( "pk_id" ) );

    relationManager->addRelation( relation );
  }

  std::unique_ptr<QgsIdentifyResultsDialog> dialog = std::make_unique<QgsIdentifyResultsDialog>( mCanvas );
  dialog->addFeature( layerA, featureA, QMap< QString, QString>() );

  QCOMPARE( dialog->lstResults->topLevelItemCount(), 1 );
  QTreeWidgetItem *topLevelItem = dialog->lstResults->topLevelItem( 0 );
  QCOMPARE( topLevelItem->childCount(), 1 );
  QgsIdentifyResultsFeatureItem *featureItem = dynamic_cast<QgsIdentifyResultsFeatureItem *>( topLevelItem->child( 0 ) );
  QVERIFY( featureItem );
  std::vector<QgsIdentifyResultsRelationItem *> relationItems;
  for ( int i = 0; i < featureItem->childCount(); ++i )
  {
    QgsIdentifyResultsRelationItem *relationItem = dynamic_cast<QgsIdentifyResultsRelationItem *>( featureItem->child( i ) );
    if ( relationItem )
      relationItems.push_back( relationItem );
  }
  QCOMPARE( relationItems.size(), 2 );

  QCOMPARE( relationItems[0]->text( 0 ), QStringLiteral( "layerB through B-A […]" ) );
  QCOMPARE( relationItems[0]->childCount(), 0 );
  QCOMPARE( relationItems[0]->childIndicatorPolicy(), QTreeWidgetItem::ShowIndicator );
  QCOMPARE( relationItems[0]->isExpanded(), false );

  QCOMPARE( relationItems[1]->text( 0 ), QStringLiteral( "layerB through A-B [1]" ) );
  QCOMPARE( relationItems[1]->childCount(), 0 );
  QCOMPARE( relationItems[1]->childIndicatorPolicy(), QTreeWidgetItem::ShowIndicator );
  QCOMPARE( relationItems[1]->isExpanded(), false );

  // Check referenced relation

  // Check that expandAll() doesn't result in automatic resolution of relations
  dialog->expandAll();
  QCOMPARE( relationItems[0]->childCount(), 0 );

  relationItems[0]->setExpanded( true );
  QCOMPARE( relationItems[0]->text( 0 ), QStringLiteral( "layerB through B-A [2]" ) );
  QCOMPARE( relationItems[0]->childCount(), 2 );

  // Check that folding/unfolding after initial expansion works
  relationItems[0]->setExpanded( false );
  relationItems[0]->setExpanded( true );
  QCOMPARE( relationItems[0]->text( 0 ), QStringLiteral( "layerB through B-A [2]" ) );
  QCOMPARE( relationItems[0]->childCount(), 2 );

  {
    QgsIdentifyResultsFeatureItem *relatedFeatureItem = dynamic_cast<QgsIdentifyResultsFeatureItem *>( relationItems[0]->child( 0 ) );
    QVERIFY( relatedFeatureItem );
    QVERIFY( relatedFeatureItem->data( 0, QgsIdentifyResultsDialog::FeatureRole ).isValid() );
    const QgsFeature relatedFeature = relatedFeatureItem->data( 0, QgsIdentifyResultsDialog::FeatureRole ).value< QgsFeature >();
    QCOMPARE( relatedFeature.attribute( IDX_OTHER_FIELD ), OTHER_FIELD );

    {
      std::vector<QgsIdentifyResultsRelationItem *> childRelationItems;
      for ( int i = 0; i < relatedFeatureItem->childCount(); ++i )
      {
        QgsIdentifyResultsRelationItem *relationItem = dynamic_cast<QgsIdentifyResultsRelationItem *>( relatedFeatureItem->child( i ) );
        if ( relationItem )
          childRelationItems.push_back( relationItem );
      }
      QCOMPARE( childRelationItems.size(), 1 );

      QCOMPARE( childRelationItems[0]->childCount(), 0 );
      QCOMPARE( childRelationItems[0]->text( 0 ), QStringLiteral( "layerC through B-C [1]" ) );

      childRelationItems[0]->setExpanded( true );
      QCOMPARE( childRelationItems[0]->childCount(), 1 );
      QCOMPARE( childRelationItems[0]->text( 0 ), QStringLiteral( "layerC through B-C [1]" ) );

      {
        QgsIdentifyResultsFeatureItem *childRelatedFeatureItem = dynamic_cast<QgsIdentifyResultsFeatureItem *>( childRelationItems[0]->child( 0 ) );
        QVERIFY( childRelatedFeatureItem );
        QVERIFY( childRelatedFeatureItem->data( 0, QgsIdentifyResultsDialog::FeatureRole ).isValid() );
        const QgsFeature relatedFeature = childRelatedFeatureItem->data( 0, QgsIdentifyResultsDialog::FeatureRole ).value< QgsFeature >();
        QCOMPARE( relatedFeature.attribute( 0 ), PK_ID_C );

        // Check that this child doesn't link back to parent feature A
        std::vector<QgsIdentifyResultsRelationItem *> childChildRelationItems;
        for ( int i = 0; i < childRelatedFeatureItem->childCount(); ++i )
        {
          QgsIdentifyResultsRelationItem *relationItem = dynamic_cast<QgsIdentifyResultsRelationItem *>( childRelatedFeatureItem->child( i ) );
          if ( relationItem )
            childChildRelationItems.push_back( relationItem );
        }
        QCOMPARE( childChildRelationItems.size(), 0 );
      }
    }

  }

  {
    QgsIdentifyResultsFeatureItem *relatedFeatureItem = dynamic_cast<QgsIdentifyResultsFeatureItem *>( relationItems[0]->child( 1 ) );
    QVERIFY( relatedFeatureItem );
    QVERIFY( relatedFeatureItem->data( 0, QgsIdentifyResultsDialog::FeatureRole ).isValid() );
    const QgsFeature relatedFeature = relatedFeatureItem->data( 0, QgsIdentifyResultsDialog::FeatureRole ).value< QgsFeature >();
    QCOMPARE( relatedFeature.attribute( IDX_OTHER_FIELD ), OTHER_FIELD + 1 );
  }

  // Check referencing relation
  relationItems[1]->setExpanded( true );
  QCOMPARE( relationItems[1]->text( 0 ), QStringLiteral( "layerB through A-B [1]" ) );
  QCOMPARE( relationItems[1]->childCount(), 1 );

  {
    QgsIdentifyResultsFeatureItem *relatedFeatureItem = dynamic_cast<QgsIdentifyResultsFeatureItem *>( relationItems[1]->child( 0 ) );
    QVERIFY( relatedFeatureItem );
    QVERIFY( relatedFeatureItem->data( 0, QgsIdentifyResultsDialog::FeatureRole ).isValid() );
    const QgsFeature relatedFeature = relatedFeatureItem->data( 0, QgsIdentifyResultsDialog::FeatureRole ).value< QgsFeature >();
    QCOMPARE( relatedFeature.attribute( IDX_OTHER_FIELD ), OTHER_FIELD );
  }
}


QGSTEST_MAIN( TestQgsIdentifyResultsDialog )
#include "testqgsidentifyresultsdialog.moc"
