/***************************************************************************
    testqgsuniquevaluesconfigdlg.cpp
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

#include "editorwidgets/qgsvaluemapconfigdlg.h"
#include "qgsgui.h"
#include "qgseditorwidgetregistry.h"
#include "qgsapplication.h"
#include "qgsproject.h"
#include "qgsvectorlayer.h"
#include "qgsattributeform.h"

#include <QComboBox>

class TestQgsUniqueValuesConfigDlg : public QObject
{
    Q_OBJECT
  public:
    TestQgsUniqueValuesConfigDlg() = default;

  private slots:
    void initTestCase();    // will be called before the first testfunction is executed.
    void cleanupTestCase(); // will be called after the last testfunction was executed.
    void init();            // will be called before each testfunction is executed.
    void cleanup();         // will be called after every testfunction.

    void testUniqueValuesAllowNull();
};

void TestQgsUniqueValuesConfigDlg::initTestCase()
{
  QgsApplication::init();
  QgsApplication::initQgis();
  QgsGui::editorWidgetRegistry()->initEditors();
}

void TestQgsUniqueValuesConfigDlg::cleanupTestCase()
{
  QgsApplication::exitQgis();
}

void TestQgsUniqueValuesConfigDlg::init()
{
}

void TestQgsUniqueValuesConfigDlg::cleanup()
{
}

void TestQgsUniqueValuesConfigDlg::testUniqueValuesAllowNull()
{
  // make a temporary layer to check through
  const QString def = QStringLiteral( "Point?field=col0:integer&field=col1:integer" );
  QgsVectorLayer *layer = new QgsVectorLayer( def, QStringLiteral( "test" ), QStringLiteral( "memory" ) );

  bool allowNull = true;
  while( true )
  {
    // set default value
    layer->setDefaultValueDefinition( 1, QgsDefaultValue( QStringLiteral( "\"col0\"+1" ), true ) );

    // build a form, set a feature
    QgsFeature ft( layer->fields(), 1 );
    QgsAttributeForm form( layer );
    form.setMode( QgsAttributeEditorContext::AddFeatureMode );
    form.setFeature( ft );

    // create a value map configuration
    QgsValueMapConfigDlg *valueMapConfig = static_cast<QgsValueMapConfigDlg *>( QgsGui::editorWidgetRegistry()->createConfigWidget( QStringLiteral( "UniqueValues" ), layer, 1, nullptr ) );
    QVariantMap config = valueMapConfig->config();

    // set allow null in value map configuration and apply it to the layer
    config.remove(QStringLiteral("AllowNull"));
    config.insert( QStringLiteral( "AllowNull" ), allowNull );
    layer->setEditorWidgetSetup( 1, QgsEditorWidgetSetup( QStringLiteral( "UniqueValues" ), config ) );

    //set value in col0:
    QgsEditorWidgetWrapper *ww0;
    ww0 = qobject_cast<QgsEditorWidgetWrapper *>( form.mWidgets[0] );
    ww0->setValue( 1 );

    QComboBox *cb = form.findChild<QComboBox*>();
    cb->setCurrentIndex(0);
    if ( allowNull )
    {
      QCOMPARE( cb->currentText(), "(NULL)" );
      cb->setCurrentIndex(1);
      QCOMPARE( cb->currentText(), "(NULL)" );
      cb->setCurrentIndex(2);
      QCOMPARE( cb->currentText(), "(2)" );
    }
    else
      QCOMPARE( cb->currentText(), "(2)" );

    delete valueMapConfig;

    if ( allowNull )
      allowNull = false;
    else
      break;
  }
}

QGSTEST_MAIN( TestQgsUniqueValuesConfigDlg )
#include "testqgsuniquevaluesconfigdlg.moc"
