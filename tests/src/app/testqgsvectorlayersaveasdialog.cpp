/***************************************************************************
     testqgsvectorlayersaveasdialog.cpp
     -------------------------
    Date                 : 2016-05-05
    Copyright            : (C) 2016 by Even Rouault
    Email                : even.rouault at spatials.com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include <QtTest/QtTest>
#include "qgisapp.h"
#include "qgsapplication.h"
#include "qgsvectorlayer.h"
#include "qgsfeature.h"
#include "qgsgeometry.h"
#include "qgsvectordataprovider.h"
#include "ogr/qgsvectorlayersaveasdialog.h"
#include "qgseditorwidgetregistry.h"
#include "qgsproject.h"
#include "qgsmapcanvas.h"

/** \ingroup UnitTests
 * This is a unit test for the save as dialog
 */
class TestQgsVectorLayerSaveAsDialog : public QObject
{
    Q_OBJECT
  public:
    TestQgsVectorLayerSaveAsDialog();

  private slots:
    void initTestCase();// will be called before the first testfunction is executed.
    void cleanupTestCase();// will be called after the last testfunction was executed.
    void init() {} // will be called before each testfunction is executed.
    void cleanup() {} // will be called after every testfunction.

    void testAttributesAsDisplayedValues();

  private:
    QgisApp * mQgisApp;
};

TestQgsVectorLayerSaveAsDialog::TestQgsVectorLayerSaveAsDialog()
    : mQgisApp( nullptr )
{

}

//runs before all tests
void TestQgsVectorLayerSaveAsDialog::initTestCase()
{
  qDebug() << "TestQgsVectorLayerSaveAsDialog::initTestCase()";
  // init QGIS's paths - true means that all path will be inited from prefix
  QgsApplication::init();
  QgsApplication::initQgis();
  mQgisApp = new QgisApp();
  QgsEditorWidgetRegistry::initEditors();
}

//runs after all tests
void TestQgsVectorLayerSaveAsDialog::cleanupTestCase()
{
  QgsApplication::exitQgis();
}

void TestQgsVectorLayerSaveAsDialog::testAttributesAsDisplayedValues()
{
  //create a temporary layer
  QScopedPointer< QgsVectorLayer> tempLayer( new QgsVectorLayer( "none?field=code:int&field=regular:string", "vl", "memory" ) );
  QVERIFY( tempLayer->isValid() );

  // Set a widget
  tempLayer->editFormConfig()->setWidgetType( 0, "ValueRelation" );

  QgsVectorLayerSaveAsDialog d( tempLayer.data() );

  QPushButton* mDeselectAllAttributes = d.findChild<QPushButton*>( "mDeselectAllAttributes" );
  QTest::mouseClick( mDeselectAllAttributes, Qt::LeftButton );

  QTableWidget* mAttributeTable = d.findChild<QTableWidget*>( "mAttributeTable" );
  QCheckBox* mReplaceRawFieldValues = d.findChild<QCheckBox*>( "mReplaceRawFieldValues" );

  QCOMPARE( mAttributeTable->rowCount(), 2 );
  QCOMPARE( mAttributeTable->columnCount(), 3 );

  QCOMPARE( d.attributesAsDisplayedValues().size(), 0 );

  QCOMPARE( mReplaceRawFieldValues->checkState(), Qt::Unchecked );
  QCOMPARE( mReplaceRawFieldValues->isEnabled(), false );
  QCOMPARE( mAttributeTable->item( 0, 0 )->checkState(), Qt::Unchecked );
  QCOMPARE( mAttributeTable->item( 0, 2 )->checkState(), Qt::Unchecked );
  QCOMPARE( mAttributeTable->item( 0, 2 )->flags(), Qt::ItemIsUserCheckable );
  QCOMPARE( mAttributeTable->item( 1, 2 )->flags(), 0 );

  // Activate item
  mAttributeTable->item( 0, 0 )->setCheckState( Qt::Checked );

  QCOMPARE( mReplaceRawFieldValues->checkState(), Qt::Unchecked );
  QCOMPARE( mReplaceRawFieldValues->isEnabled(), true );
  QCOMPARE( mAttributeTable->item( 0, 0 )->checkState(), Qt::Checked );
  QCOMPARE( mAttributeTable->item( 0, 2 )->checkState(), Qt::Unchecked );
  QCOMPARE( mAttributeTable->item( 0, 2 )->flags(), Qt::ItemIsEnabled | Qt::ItemIsUserCheckable );

  // Activate "Replace with displayed value" column
  mAttributeTable->item( 0, 2 )->setCheckState( Qt::Checked );

  QCOMPARE( mReplaceRawFieldValues->checkState(), Qt::Checked );
  QCOMPARE( mReplaceRawFieldValues->isEnabled(), true );

  // Uncheck mReplaceRawFieldValues
  mReplaceRawFieldValues->setCheckState( Qt::Unchecked );
  QCOMPARE( mAttributeTable->item( 0, 2 )->checkState(), Qt::Unchecked );
  QCOMPARE( mAttributeTable->item( 0, 2 )->flags(), Qt::ItemIsEnabled | Qt::ItemIsUserCheckable );

  // Check mReplaceRawFieldValues
  mReplaceRawFieldValues->setCheckState( Qt::Checked );
  QCOMPARE( mAttributeTable->item( 0, 2 )->checkState(), Qt::Checked );
  QCOMPARE( mAttributeTable->item( 0, 2 )->flags(), Qt::ItemIsEnabled | Qt::ItemIsUserCheckable );

  QCOMPARE( d.attributesAsDisplayedValues().size(), 1 );
  QCOMPARE( d.attributesAsDisplayedValues()[0], 0 );

  // Disable item
  mAttributeTable->item( 0, 0 )->setCheckState( Qt::Unchecked );
  QCOMPARE( mAttributeTable->item( 0, 2 )->checkState(), Qt::Unchecked );
  QCOMPARE( mAttributeTable->item( 0, 2 )->flags(), Qt::ItemIsUserCheckable );

  //d.exec();
}

QTEST_MAIN( TestQgsVectorLayerSaveAsDialog )
#include "testqgsvectorlayersaveasdialog.moc"
