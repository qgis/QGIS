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
#include "ogr/qgsvectorlayersaveasdialog.h"
#include "qgisapp.h"
#include "qgsapplication.h"
#include "qgseditorwidgetregistry.h"
#include "qgsfeature.h"
#include "qgsgeometry.h"
#include "qgsgui.h"
#include "qgsmapcanvas.h"
#include "qgsproject.h"
#include "qgstest.h"
#include "qgsvectordataprovider.h"
#include "qgsvectorlayer.h"

#include <QString>

using namespace Qt::StringLiterals;

/**
 * \ingroup UnitTests
 * This is a unit test for the save as dialog
 */
class TestQgsVectorLayerSaveAsDialog : public QObject
{
    Q_OBJECT
  public:
    TestQgsVectorLayerSaveAsDialog();

  private slots:
    void initTestCase();    // will be called before the first testfunction is executed.
    void cleanupTestCase(); // will be called after the last testfunction was executed.
    void init() {}          // will be called before each testfunction is executed.
    void cleanup() {}       // will be called after every testfunction.

    void testAttributesAsDisplayedValues();
    void testFieldSelectionPreservedOnFormatChange();

  private:
    QgisApp *mQgisApp = nullptr;
};

TestQgsVectorLayerSaveAsDialog::TestQgsVectorLayerSaveAsDialog() = default;

//runs before all tests
void TestQgsVectorLayerSaveAsDialog::initTestCase()
{
  qDebug() << "TestQgsVectorLayerSaveAsDialog::initTestCase()";
  // init QGIS's paths - true means that all path will be inited from prefix
  QgsApplication::init();
  QgsApplication::initQgis();
  mQgisApp = new QgisApp();
  QgsGui::editorWidgetRegistry()->initEditors();
}

//runs after all tests
void TestQgsVectorLayerSaveAsDialog::cleanupTestCase()
{
  QgsApplication::exitQgis();
}

void TestQgsVectorLayerSaveAsDialog::testAttributesAsDisplayedValues()
{
  //create a temporary layer
  auto tempLayer = std::make_unique<QgsVectorLayer>( u"none?field=code:int&field=regular:string"_s, u"vl"_s, u"memory"_s );
  QVERIFY( tempLayer->isValid() );

  // Assign a custom CRS to the layer
  QgsCoordinateReferenceSystem crs;
  crs.createFromString( "PROJ:+proj=merc +a=1" );
  tempLayer->setCrs( crs );

  // Set a widget
  tempLayer->setEditorWidgetSetup( 0, QgsEditorWidgetSetup( u"ValueRelation"_s, QVariantMap() ) );

  const QgsVectorLayerSaveAsDialog d( tempLayer.get() );

  QPushButton *mDeselectAllAttributes = d.findChild<QPushButton *>( u"mDeselectAllAttributes"_s );
  QTest::mouseClick( mDeselectAllAttributes, Qt::LeftButton );

  QTableWidget *mAttributeTable = d.findChild<QTableWidget *>( u"mAttributeTable"_s );
  QCheckBox *mReplaceRawFieldValues = d.findChild<QCheckBox *>( u"mReplaceRawFieldValues"_s );

  QCOMPARE( mAttributeTable->rowCount(), 2 );
  QCOMPARE( mAttributeTable->isColumnHidden( 3 ), false );

  QCOMPARE( d.attributesAsDisplayedValues().size(), 0 );

  QCOMPARE( mReplaceRawFieldValues->checkState(), Qt::Unchecked );
  QCOMPARE( mReplaceRawFieldValues->isEnabled(), false );
  QCOMPARE( mAttributeTable->item( 0, 0 )->checkState(), Qt::Unchecked );
  QCOMPARE( mAttributeTable->item( 0, 3 )->checkState(), Qt::Unchecked );
  QCOMPARE( mAttributeTable->item( 0, 3 )->flags(), Qt::ItemIsUserCheckable );
  QCOMPARE( mAttributeTable->item( 1, 3 )->flags(), 0 );

  // Activate item
  mAttributeTable->item( 0, 0 )->setCheckState( Qt::Checked );

  QCOMPARE( mReplaceRawFieldValues->checkState(), Qt::Unchecked );
  QCOMPARE( mReplaceRawFieldValues->isEnabled(), true );
  QCOMPARE( mAttributeTable->item( 0, 0 )->checkState(), Qt::Checked );
  QCOMPARE( mAttributeTable->item( 0, 3 )->checkState(), Qt::Unchecked );
  QCOMPARE( mAttributeTable->item( 0, 3 )->flags(), Qt::ItemIsEnabled | Qt::ItemIsUserCheckable );

  // Activate "Replace with displayed value" column
  mAttributeTable->item( 0, 3 )->setCheckState( Qt::Checked );

  QCOMPARE( mReplaceRawFieldValues->checkState(), Qt::Checked );
  QCOMPARE( mReplaceRawFieldValues->isEnabled(), true );

  // Uncheck mReplaceRawFieldValues
  mReplaceRawFieldValues->setCheckState( Qt::Unchecked );
  QCOMPARE( mAttributeTable->item( 0, 3 )->checkState(), Qt::Unchecked );
  QCOMPARE( mAttributeTable->item( 0, 3 )->flags(), Qt::ItemIsEnabled | Qt::ItemIsUserCheckable );

  // Check mReplaceRawFieldValues
  mReplaceRawFieldValues->setCheckState( Qt::Checked );
  QCOMPARE( mAttributeTable->item( 0, 3 )->checkState(), Qt::Checked );
  QCOMPARE( mAttributeTable->item( 0, 3 )->flags(), Qt::ItemIsEnabled | Qt::ItemIsUserCheckable );

  QCOMPARE( d.attributesAsDisplayedValues().size(), 1 );
  QCOMPARE( d.attributesAsDisplayedValues()[0], 0 );

  // Disable item
  mAttributeTable->item( 0, 0 )->setCheckState( Qt::Unchecked );
  QCOMPARE( mAttributeTable->item( 0, 3 )->checkState(), Qt::Unchecked );
  QCOMPARE( mAttributeTable->item( 0, 3 )->flags(), Qt::ItemIsUserCheckable );

  // Check that we can get a custom CRS with crsObject()
  QCOMPARE( d.crs(), crs );

  //d.exec();
}

void TestQgsVectorLayerSaveAsDialog::testFieldSelectionPreservedOnFormatChange()
{
  auto tempLayer = std::make_unique<QgsVectorLayer>( u"Point?crs=EPSG:4326&field=id:int&field=name:string&field=descr:string"_s, u"vl"_s, u"memory"_s );
  QVERIFY( tempLayer->isValid() );

  // Set a widget on the first and last fields to check that the ExportAsDisplayedValue column state is preserved
  tempLayer->setEditorWidgetSetup( 0, QgsEditorWidgetSetup( u"ValueRelation"_s, QVariantMap() ) );
  tempLayer->setEditorWidgetSetup( 2, QgsEditorWidgetSetup( u"ValueMap"_s, QVariantMap() ) );

  const QgsVectorLayerSaveAsDialog d( tempLayer.get() );

  QTableWidget *mAttributeTable = d.findChild<QTableWidget *>( u"mAttributeTable"_s );
  QCheckBox *mReplaceRawFieldValues = d.findChild<QCheckBox *>( u"mReplaceRawFieldValues"_s );
  QComboBox *mFormatComboBox = d.findChild<QComboBox *>( u"mFormatComboBox"_s );

  // Initially all fields are checked and ExportAsDisplayedValue is disabled
  QCOMPARE( mAttributeTable->item( 0, 0 )->checkState(), Qt::Checked );
  QCOMPARE( mAttributeTable->item( 0, 3 )->checkState(), Qt::Unchecked );
  QCOMPARE( mAttributeTable->item( 1, 0 )->checkState(), Qt::Checked );
  QCOMPARE( mAttributeTable->item( 2, 0 )->checkState(), Qt::Checked );
  QCOMPARE( mAttributeTable->item( 2, 3 )->checkState(), Qt::Unchecked );
  QCOMPARE( mReplaceRawFieldValues->checkState(), Qt::Unchecked );

  // Switch format to CSV, ExportAsDisplayedValue should be checked for first and last field
  mFormatComboBox->setCurrentIndex( mFormatComboBox->findData( u"CSV"_s ) );
  QCOMPARE( mAttributeTable->item( 0, 0 )->checkState(), Qt::Checked );
  QCOMPARE( mAttributeTable->item( 0, 3 )->checkState(), Qt::Checked );
  QCOMPARE( mAttributeTable->item( 1, 0 )->checkState(), Qt::Checked );
  QCOMPARE( mAttributeTable->item( 2, 0 )->checkState(), Qt::Checked );
  QCOMPARE( mAttributeTable->item( 2, 3 )->checkState(), Qt::Checked );
  QCOMPARE( mReplaceRawFieldValues->checkState(), Qt::Checked );

  // Disable ExportAsDisplayedValue for the third field
  mAttributeTable->item( 2, 3 )->setCheckState( Qt::Unchecked );
  QCOMPARE( mAttributeTable->item( 2, 0 )->checkState(), Qt::Checked );
  QCOMPARE( mAttributeTable->item( 2, 3 )->checkState(), Qt::Unchecked );
  QCOMPARE( mReplaceRawFieldValues->checkState(), Qt::PartiallyChecked );

  // Switch format to GeoPackage, field selection should be preserved
  mFormatComboBox->setCurrentIndex( mFormatComboBox->findData( u"GeoPackage"_s ) );
  QCOMPARE( mAttributeTable->item( 0, 0 )->checkState(), Qt::Checked );
  QCOMPARE( mAttributeTable->item( 0, 3 )->checkState(), Qt::Checked );
  QCOMPARE( mAttributeTable->item( 1, 0 )->checkState(), Qt::Checked );
  QCOMPARE( mAttributeTable->item( 2, 0 )->checkState(), Qt::Checked );
  QCOMPARE( mAttributeTable->item( 2, 3 )->checkState(), Qt::Unchecked );
  QCOMPARE( mReplaceRawFieldValues->checkState(), Qt::PartiallyChecked );

  // Switch back to CSV, field selection should be preserved
  mFormatComboBox->setCurrentIndex( mFormatComboBox->findData( u"Comma Separated Values [CSV]"_s ) );
  QCOMPARE( mAttributeTable->item( 0, 0 )->checkState(), Qt::Checked );
  QCOMPARE( mAttributeTable->item( 0, 3 )->checkState(), Qt::Checked );
  QCOMPARE( mAttributeTable->item( 1, 0 )->checkState(), Qt::Checked );
  QCOMPARE( mAttributeTable->item( 2, 0 )->checkState(), Qt::Checked );
  QCOMPARE( mAttributeTable->item( 2, 3 )->checkState(), Qt::Unchecked );
  QCOMPARE( mReplaceRawFieldValues->checkState(), Qt::PartiallyChecked );

  // Disable ExportAsDisplayedValue for the first field
  mAttributeTable->item( 0, 3 )->setCheckState( Qt::Unchecked );
  QCOMPARE( mAttributeTable->item( 0, 0 )->checkState(), Qt::Checked );
  QCOMPARE( mAttributeTable->item( 0, 3 )->checkState(), Qt::Unchecked );
  QCOMPARE( mReplaceRawFieldValues->checkState(), Qt::Unchecked );

  // Switch format to GeoPackage, field selection should be preserved
  mFormatComboBox->setCurrentIndex( mFormatComboBox->findData( u"GeoPackage"_s ) );
  QCOMPARE( mAttributeTable->item( 0, 0 )->checkState(), Qt::Checked );
  QCOMPARE( mAttributeTable->item( 0, 3 )->checkState(), Qt::Unchecked );
  QCOMPARE( mAttributeTable->item( 1, 0 )->checkState(), Qt::Checked );
  QCOMPARE( mAttributeTable->item( 2, 0 )->checkState(), Qt::Checked );
  QCOMPARE( mAttributeTable->item( 2, 3 )->checkState(), Qt::Unchecked );
  QCOMPARE( mReplaceRawFieldValues->checkState(), Qt::Unchecked );

  // Switch back to CSV, field selection should be preserved
  mFormatComboBox->setCurrentIndex( mFormatComboBox->findData( u"Comma Separated Values [CSV]"_s ) );
  QCOMPARE( mAttributeTable->item( 0, 0 )->checkState(), Qt::Checked );
  QCOMPARE( mAttributeTable->item( 0, 3 )->checkState(), Qt::Unchecked );
  QCOMPARE( mAttributeTable->item( 1, 0 )->checkState(), Qt::Checked );
  QCOMPARE( mAttributeTable->item( 2, 0 )->checkState(), Qt::Checked );
  QCOMPARE( mAttributeTable->item( 2, 3 )->checkState(), Qt::Unchecked );
  QCOMPARE( mReplaceRawFieldValues->checkState(), Qt::Unchecked );
}

QGSTEST_MAIN( TestQgsVectorLayerSaveAsDialog )
#include "testqgsvectorlayersaveasdialog.moc"
