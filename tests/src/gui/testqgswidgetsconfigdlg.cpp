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

#include "qgstest.h"

#include "qgsgui.h"
#include "qgseditorwidgetregistry.h"
#include "qgsapplication.h"
#include "qgsproject.h"
#include "qgsvectorlayer.h"
#include "qgsattributeform.h"
#include "editorwidgets/qgsuniquevaluesconfigdlg.h"

#include <QComboBox>

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
    void testAllowNull( const QString &widgetType );

    QgsVectorLayer *mLayer;
    QgsAttributeForm *mForm;
    QVariantMap *mConfig;
    QComboBox *mComboBox;
    QgsUniqueValuesConfigDlg *mUniqueValuesConfigDlg;
};

void TestQgsWidgetsConfigDlg::initTestCase()
{
  QgsApplication::init();
  QgsApplication::initQgis();
  QgsGui::editorWidgetRegistry()->initEditors();

  // make a layer to check through
  const QString def = QStringLiteral( "Point?field=col0:integer&field=col1:integer" );
  mLayer = new QgsVectorLayer( def, QStringLiteral( "test" ), QStringLiteral( "memory" ) );

  // set default value
  mLayer->setDefaultValueDefinition( 1, QgsDefaultValue( QStringLiteral( "\"col0\"+1" ), true ) );

  // build a form, set a feature
  QgsFeature ft( mLayer->fields(), 1 );
  mForm = new QgsAttributeForm( mLayer );
  mForm->setMode( QgsAttributeEditorContext::AddFeatureMode );
  mForm->setFeature( ft );
}

void TestQgsWidgetsConfigDlg::cleanupTestCase()
{
  delete mLayer;
  delete mUniqueValuesConfigDlg;
  QgsApplication::exitQgis();
}

void TestQgsWidgetsConfigDlg::init()
{
}

void TestQgsWidgetsConfigDlg::cleanup()
{
}

void TestQgsWidgetsConfigDlg::testAllowNull( const QString &widgetType )
{
  // set allow null in configuration and apply it to the layer
  mConfig->insert( QStringLiteral( "AllowNull" ), true );
  mLayer->setEditorWidgetSetup( 1, QgsEditorWidgetSetup( widgetType, *mConfig ) );

  //set value in col0:
  QgsEditorWidgetWrapper *ww0;
  ww0 = qobject_cast<QgsEditorWidgetWrapper *>( mForm->mWidgets[0] );
  ww0->setValue( 1 );
  mComboBox = mForm->findChild<QComboBox *>();

  mComboBox->setCurrentIndex( 0 );
  QCOMPARE( mComboBox->currentText(), "(NULL)" );
  mComboBox->setCurrentIndex( 1 );
  QCOMPARE( mComboBox->currentText(), "(2)" );

  mConfig->insert( QStringLiteral( "AllowNull" ), false );
  QCOMPARE( mComboBox->currentText(), "(2)" );
}

void TestQgsWidgetsConfigDlg::testUniqueValuesAllowNull()
{
  //create a configuration
  const QString widgetType = QStringLiteral( "UniqueValues" );
  mUniqueValuesConfigDlg = static_cast<QgsUniqueValuesConfigDlg *>( QgsGui::editorWidgetRegistry()->createConfigWidget( widgetType, mLayer, 1, nullptr ) );
  mConfig = new QVariantMap( mUniqueValuesConfigDlg->config() );
  testAllowNull( widgetType );
}

QGSTEST_MAIN( TestQgsWidgetsConfigDlg )
#include "testqgswidgetsconfigdlg.moc"
