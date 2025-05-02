/***************************************************************************
    testqgsvaluemapconfigdlg.cpp
     --------------------------------------
    Date                 : 14 02 2021
    Copyright            : (C) 2019 Stephen Knox
    Email                : stephenknox73 at gmail dot com
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
#include "qgsogrproviderutils.h"
#include "qgsattributeform.h"
#include "qgseditformconfig.h"

#include <QComboBox>

class TestQgsValueMapConfigDlg : public QObject
{
    Q_OBJECT
  public:
    TestQgsValueMapConfigDlg() = default;

  private slots:
    void initTestCase();    // will be called before the first testfunction is executed.
    void cleanupTestCase(); // will be called after the last testfunction was executed.
    void init();            // will be called before each testfunction is executed.
    void cleanup();         // will be called after every testfunction.

    void testLoadFromCSV();
    void testTestTrimValues();
    void testValueMapAllowNull();
};

void TestQgsValueMapConfigDlg::initTestCase()
{
  QgsApplication::init();
  QgsApplication::initQgis();
  QgsGui::editorWidgetRegistry()->initEditors();
}

void TestQgsValueMapConfigDlg::cleanupTestCase()
{
  QgsApplication::exitQgis();
}

void TestQgsValueMapConfigDlg::init()
{
}

void TestQgsValueMapConfigDlg::cleanup()
{
}

void TestQgsValueMapConfigDlg::testLoadFromCSV()
{
  const QString dataDir( TEST_DATA_DIR );
  QgsVectorLayer vl( QStringLiteral( "LineString?crs=epsg:3111&field=pk:int&field=name:string" ), QStringLiteral( "vl1" ), QStringLiteral( "memory" ) );

  QList<QVariant> valueList;
  QVariantMap value;
  value.insert( QStringLiteral( "Basic unquoted record" ), QString( "1" ) );
  valueList << value;
  value.clear();
  value.insert( QStringLiteral( "Forest type" ), QString( "2" ) );
  valueList << value;
  value.clear();
  value.insert( QStringLiteral( "So-called \"data\"" ), QString( "three" ) );
  valueList << value;
  value.clear();
  value.insert( QStringLiteral( "444" ), QString( "4" ) );
  valueList << value;
  value.clear();
  value.insert( QStringLiteral( "five" ), QString( "5" ) );
  valueList << value;

  QgsValueMapConfigDlg *valueMapConfig = static_cast<QgsValueMapConfigDlg *>( QgsGui::editorWidgetRegistry()->createConfigWidget( QStringLiteral( "ValueMap" ), &vl, 1, nullptr ) );
  valueMapConfig->loadMapFromCSV( dataDir + QStringLiteral( "/valuemapsample.csv" ) );
  QCOMPARE( valueMapConfig->config().value( QStringLiteral( "map" ) ).toList(), valueList );
  delete valueMapConfig;
}

void TestQgsValueMapConfigDlg::testTestTrimValues()
{
  // Create a GPKG layer in a temporary file using GDAL
  QTemporaryDir tempDir;
  const QString tempFile = tempDir.path() + QDir::separator() + QStringLiteral( "test.gpkg" );

  QList<QPair<QString, QString>> fields;
  fields << QPair<QString, QString>( QStringLiteral( "key" ), QStringLiteral( "String;1" ) );
  QString error;
  QVERIFY( QgsOgrProviderUtils::createEmptyDataSource( tempFile, QStringLiteral( "GPKG" ), QStringLiteral( "UTF-8" ), Qgis::WkbType::Point, fields, QgsCoordinateReferenceSystem::fromEpsgId( 4326 ), error ) );
  QgsVectorLayer vl { tempFile, QStringLiteral( "vl1" ), QStringLiteral( "ogr" ) };
  QVERIFY( vl.isValid() );
  QCOMPARE( vl.fields().count(), 2 );

  QMap<QString, QVariant> valueList;
  valueList[QStringLiteral( "1" )] = QStringLiteral( "Choice 1" );
  valueList[QStringLiteral( "2" )] = QStringLiteral( "Choice 2" );

  std::unique_ptr<QgsValueMapConfigDlg> valueMapConfig;
  valueMapConfig.reset( static_cast<QgsValueMapConfigDlg *>( QgsGui::editorWidgetRegistry()->createConfigWidget( QStringLiteral( "ValueMap" ), &vl, 1, nullptr ) ) );

  valueMapConfig->updateMap( valueList, false );
  valueMapConfig->addNullButtonPushed();
  QVERIFY( !valueMapConfig->mValueMapErrorsLabel->text().contains( QStringLiteral( "trimmed" ) ) );

  valueList[QStringLiteral( "33" )] = QStringLiteral( "Choice 33" );
  valueMapConfig->updateMap( valueList, false );
  QVERIFY( valueMapConfig->mValueMapErrorsLabel->text().contains( QStringLiteral( "trimmed" ) ) );
}

void TestQgsValueMapConfigDlg::testValueMapAllowNull()
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
  QgsValueMapConfigDlg *valueMapConfig = static_cast<QgsValueMapConfigDlg *>( QgsGui::editorWidgetRegistry()->createConfigWidget( QStringLiteral( "ValueMap" ), layer, 1, nullptr ) );
  QVariantMap config = valueMapConfig->config();

  // set allow null in value map configuration and apply it to the layer
  config.remove(QStringLiteral("AllowNull"));
  config.insert( QStringLiteral( "AllowNull" ), allowNull );
  layer->setEditorWidgetSetup( 1, QgsEditorWidgetSetup( QStringLiteral( "ValueMap" ), config ) );

  //set value in col0:
  QgsEditorWidgetWrapper *ww0;
  ww0 = qobject_cast<QgsEditorWidgetWrapper *>( form.mWidgets[0] );
  ww0->setValue( 1 );

  QComboBox *cb = form.findChild<QComboBox*>();
  for (int i = 0; i<cb->count(); i++)
    qDebug()<<"comboItem "<<i<<": "<<cb->itemText( i );

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

QGSTEST_MAIN( TestQgsValueMapConfigDlg )
#include "testqgsvaluemapconfigdlg.moc"
