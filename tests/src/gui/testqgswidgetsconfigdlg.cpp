/***************************************************************************
    testqgswidgetsconfigdlg.cpp
     --------------------------------------
    Date                 : 31 07 2025
    Copyright            : (C) 2025 Moritz Hackenberg
    Email                : hackenberg at dev-gis dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsapplication.h"
#include "qgsattributeform.h"
#include "qgseditorwidgetregistry.h"
#include "qgsfieldformatter.h"
#include "qgsgui.h"
#include "qgsproject.h"
#include "qgstest.h"
#include "qgsuniquevaluesconfigdlg.h"
#include "qgsvectorlayer.h"

#include <QComboBox>
#include <QString>

using namespace Qt::StringLiterals;

class TestQgsWidgetsConfigDlg : public QObject
{
    Q_OBJECT
  public:
    TestQgsWidgetsConfigDlg() = default;

  private slots:
    void initTestCase();    // will be called before the first testfunction is executed.
    void cleanupTestCase(); // will be called after the last testfunction was executed.
    void init();            // will be called before each testfunction is executed.
    void cleanup();         // will be called after every testfunction.

    void testUniqueValuesAllowNull();

  private:
    void applyConfigAndReplaceCB( const QString &widgetType );
    void editFeatureAttribute( const QVariant &value );
    void testUniqueValuesAllowNullBackwardsComp( const QString &widgetType );

    QgsVectorLayer *mLayer;
    QVariantMap *mConfig;
    QComboBox *mComboBox;
    QgsUniqueValuesConfigDlg *mUniqueValuesConfigDlg;
};

void TestQgsWidgetsConfigDlg::initTestCase()
{
  QgsApplication::initQgis();
  QgsGui::editorWidgetRegistry()->initEditors();

  // make a layer to check through
  const QString def = u"Point?field=col0:integer"_s;
  mLayer = new QgsVectorLayer( def, u"test"_s, u"memory"_s );

  // set feature
  QgsFeature ft( mLayer->fields(), 1 );
  ft.setAttributes( QgsAttributes() << 1 );
  mLayer->dataProvider()->addFeatures( QgsFeatureList() << ft );
}

void TestQgsWidgetsConfigDlg::cleanupTestCase()
{
  delete mLayer;
  delete mUniqueValuesConfigDlg;
  QgsApplication::exitQgis();
}

void TestQgsWidgetsConfigDlg::init()
{}

void TestQgsWidgetsConfigDlg::cleanup()
{}

void TestQgsWidgetsConfigDlg::applyConfigAndReplaceCB( const QString &widgetType )
{
  mLayer->setEditorWidgetSetup( 0, QgsEditorWidgetSetup( widgetType, *mConfig ) );
  QgsAttributeForm *mForm = new QgsAttributeForm( mLayer );
  mComboBox = mForm->findChild<QComboBox *>();
}

void TestQgsWidgetsConfigDlg::testUniqueValuesAllowNull()
{
  const QString widgetType = u"UniqueValues"_s;
  mUniqueValuesConfigDlg = static_cast<QgsUniqueValuesConfigDlg *>( QgsGui::editorWidgetRegistry()->createConfigWidget( widgetType, mLayer, 0, nullptr ) );
  mConfig = new QVariantMap( mUniqueValuesConfigDlg->config() );

  // test with allow null unchecked (the default state of the checkbox)
  applyConfigAndReplaceCB( widgetType );
  mComboBox->setCurrentIndex( 0 );
  QCOMPARE( mComboBox->currentText(), "1" );
  QCOMPARE( mComboBox->count(), 1 );

  // test with allow null unchecked
  mConfig->insert( u"AllowNull"_s, true );
  applyConfigAndReplaceCB( widgetType );
  mComboBox->setCurrentIndex( 0 );
  QCOMPARE( mComboBox->currentText(), "1" );
  mComboBox->setCurrentIndex( 1 );
  QCOMPARE( mComboBox->currentText(), "(NULL)" );
  QCOMPARE( mComboBox->count(), 2 );

  testUniqueValuesAllowNullBackwardsComp( widgetType );
}

void TestQgsWidgetsConfigDlg::editFeatureAttribute( const QVariant &value )
{
  mLayer->startEditing();
  QgsAttributeMap attrMap;
  attrMap.insert( 0, value );
  QgsChangedAttributesMap changes;
  changes.insert( 1, attrMap );
  mLayer->dataProvider()->changeAttributeValues( changes );
  mLayer->commitChanges();
}

void TestQgsWidgetsConfigDlg::testUniqueValuesAllowNullBackwardsComp( const QString &widgetType )
{
  // test for a layer from an older QGIS version without a null value in the field
  mConfig->remove( u"AllowNull"_s );
  applyConfigAndReplaceCB( widgetType );
  QCOMPARE( mComboBox->currentText(), "1" );
  QCOMPARE( mComboBox->count(), 1 );
  // test checkbox default state
  // simulating the call of setConfig in QgsAttributeTypeDialog::setEditorWidgetType
  mUniqueValuesConfigDlg->setConfig( *mConfig );
  QCheckBox *checkbox = mUniqueValuesConfigDlg->findChild<QCheckBox *>( u"allowNullCheckBox"_s );
  QCOMPARE( checkbox->checkState(), Qt::Unchecked );

  // test for a layer from an older QGIS version with a null value in the field
  editFeatureAttribute( QgsVariantUtils::createNullVariant( QMetaType::Type::Int ) );
  applyConfigAndReplaceCB( widgetType );
  QCOMPARE( mComboBox->currentText(), "(NULL)" );
  QCOMPARE( mComboBox->count(), 1 );
  // testCheckboxDefaultState
  mUniqueValuesConfigDlg->setConfig( *mConfig );
  QCOMPARE( checkbox->checkState(), Qt::Checked );
}

QGSTEST_MAIN( TestQgsWidgetsConfigDlg )
#include "testqgswidgetsconfigdlg.moc"
