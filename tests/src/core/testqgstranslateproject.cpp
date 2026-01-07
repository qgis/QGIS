/***************************************************************************
  testqgstranslateproject.cpp

 ---------------------
 begin                : 24.5.2018
 copyright            : (C) 2018 by david signer
 email                : david at opengis dot ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "qgsapplication.h"
#include "qgsattributeeditorcontainer.h"
#include "qgsattributeeditorelement.h"
#include "qgslayertree.h"
#include "qgslayertreegroup.h"
#include "qgsmaplayer.h"
#include "qgsproject.h"
#include "qgsrelationmanager.h"
#include "qgssettings.h"
#include "qgstest.h"
#include "qgstranslationcontext.h"
#include "qgsvectorlayer.h"

#include <QDir>
#include <QObject>

class TestQgsTranslateProject : public QObject
{
    Q_OBJECT

  public:
  private slots:
    void initTestCase();    // will be called before the first testfunction is executed.
    void cleanupTestCase(); // will be called after the last testfunction was executed.
    void init();            // will be called before each testfunction is executed.
    void cleanup();         // will be called after every testfunction.

    void createTsFile();
    void translateProject();

  private:
    QString original_locale;
};

void TestQgsTranslateProject::initTestCase()
{
  //start application
  QgsApplication::init();
  QgsApplication::initQgis();

  original_locale = QgsApplication::settingsLocaleUserLocale->value();
}

void TestQgsTranslateProject::cleanupTestCase()
{
  QgsApplication::settingsLocaleUserLocale->setValue( original_locale );
  QgsApplication::exitQgis();

  //delete translated project file
  QString translatedProjectFileName( TEST_DATA_DIR );
  translatedProjectFileName = translatedProjectFileName + "/project_translation/points_translation_de.qgs";
  QFile translatedProjectFile( translatedProjectFileName );
  translatedProjectFile.remove();

  //delete created ts file
  QString tsFileName( TEST_DATA_DIR );
  tsFileName = tsFileName + "/project_translation/points_translation.ts";
  QFile tsFile( tsFileName );
  tsFile.remove();
}

void TestQgsTranslateProject::init()
{
  //not needed
}

void TestQgsTranslateProject::cleanup()
{
  //not needed
}

void TestQgsTranslateProject::createTsFile()
{
  //open project in english
  QgsApplication::settingsLocaleUserLocale->setValue( "en" );
  QString projectFileName( TEST_DATA_DIR );
  projectFileName = projectFileName + "/project_translation/points_translation.qgs";
  QgsProject::instance()->read( projectFileName );

  //create ts file for german
  QgsProject::instance()->generateTsFile( "de" );

  //check if ts file is created
  QString tsFileName( TEST_DATA_DIR );
  tsFileName = tsFileName + "/project_translation/points_translation.ts";
  QFile tsFile( tsFileName );
  QVERIFY( tsFile.exists() );

  QVERIFY( tsFile.open( QIODevice::ReadWrite ) );

  const QString tsFileContent( tsFile.readAll() );

  //LAYER NAMES
  //lines
  QVERIFY( tsFileContent.contains( "<source>lines</source>" ) );
  //points
  QVERIFY( tsFileContent.contains( "<source>points</source>" ) );

  //LAYER GROUPS AND SUBGROUPS
  //Points:
  //Planes and Roads
  QVERIFY( tsFileContent.contains( "<source>Planes and Roads</source>" ) );
  //Little bit of nothing
  QVERIFY( tsFileContent.contains( "<source>Little bit of nothing</source>" ) );

  //FIELDS AND ALIASES
  //Lines:
  //Name (Alias: Runwayid)
  QVERIFY( tsFileContent.contains( "<source>Runwayid</source>" ) );
  //Value (Alias: Name)
  QVERIFY( tsFileContent.contains( "<source>Name</source>" ) );

  //Points:
  //Class (Alias: Level)
  QVERIFY( tsFileContent.contains( "<source>Level</source>" ) );
  //Heading
  QVERIFY( tsFileContent.contains( "<source>Heading</source>" ) );
  //Importance
  QVERIFY( tsFileContent.contains( "<source>Importance</source>" ) );
  //Pilots
  QVERIFY( tsFileContent.contains( "<source>Pilots</source>" ) );
  //Cabin Crew
  QVERIFY( tsFileContent.contains( "<source>Cabin Crew</source>" ) );
  //Staff
  QVERIFY( tsFileContent.contains( "<source>Staff</source>" ) );

  //FORMCONTAINERS
  //Plane
  QVERIFY( tsFileContent.contains( "<source>Plane</source>" ) );
  //Employees
  QVERIFY( tsFileContent.contains( "<source>Employees</source>" ) );
  //Flightattends
  QVERIFY( tsFileContent.contains( "<source>Flightattends</source>" ) );

  //RELATIONS
  //Runway
  QVERIFY( tsFileContent.contains( "<source>Runway</source>" ) );
  //Sheepwalk
  QVERIFY( tsFileContent.contains( "<source>Sheepwalk</source>" ) );

  //WIDGETS
  //ValueRelation value
  QVERIFY( tsFileContent.contains( ":fields:Cabin Crew:valuerelationvalue</name>" ) );
  QVERIFY( tsFileContent.contains( "<source>Name</source>" ) );

  //ValueMap with descriptions
  QVERIFY( tsFileContent.contains( ":fields:Name:valuemapdescriptions</name>" ) );
  QVERIFY( tsFileContent.contains( "<source>Arterial road</source>" ) );
  QVERIFY( tsFileContent.contains( "<source>Highway road</source>" ) );
  QVERIFY( tsFileContent.contains( "<source>nothing</source>" ) );

  //METADATA
  QVERIFY( tsFileContent.contains( "<source>Metadata Title</source>" ) );
  QVERIFY( tsFileContent.contains( "<source>Metadata Type</source>" ) );
  QVERIFY( tsFileContent.contains( "<source>Metadata Abstract</source>" ) );
  QVERIFY( tsFileContent.contains( "<source>Metadata Rights</source>" ) );
  QVERIFY( tsFileContent.contains( "<source>Webmaster</source>" ) );

  tsFile.close();
}

void TestQgsTranslateProject::translateProject()
{
  //open project in german
  QgsApplication::settingsLocaleUserLocale->setValue( "de" );
  QString projectFileName( TEST_DATA_DIR );
  projectFileName = projectFileName + "/project_translation/points_translation.qgs";
  QgsProject::instance()->read( projectFileName );

  //with the qm file containing translation from en to de, the project should be in german and renamed with postfix .de
  QgsVectorLayer *points_layer = QgsProject::instance()->mapLayer<QgsVectorLayer *>( "points_240d6bd6_9203_470a_994a_aae13cd9fa04" );
  QgsVectorLayer *lines_layer = QgsProject::instance()->mapLayer<QgsVectorLayer *>( "lines_a677672a_bf5d_410d_98c9_d326a5719a1b" );

  //LAYER NAMES
  //lines -> Linien
  QCOMPARE( lines_layer->name(), u"Linien"_s );
  //points -> Punkte
  QCOMPARE( points_layer->name(), u"Punkte"_s );

  //LAYER GROUPS AND SUBGROUPS
  //Points:
  //Planes and Roads -> Flugzeuge und Strassen
  QVERIFY( QgsProject::instance()->layerTreeRoot()->findGroup( u"Flugzeuge und Strassen"_s ) );
  //Little bit of nothing -> Bisschen nichts
  QVERIFY( QgsProject::instance()->layerTreeRoot()->findGroup( u"Bisschen nichts"_s ) );

  //FIELDS AND ALIASES
  //Lines:
  const QgsFields lines_fields = lines_layer->fields();
  //Name (Alias: Runwayid) -> Pistenid
  QCOMPARE( lines_fields.field( u"Name"_s ).alias(), u"Pistenid"_s );
  //Value (Alias: Name) -> Pistenname
  QCOMPARE( lines_fields.field( u"Value"_s ).alias(), u"Pistenname"_s );

  //Points:
  const QgsFields points_fields = points_layer->fields();
  //Class (Alias: Level) -> Klasse
  QCOMPARE( points_fields.field( u"Class"_s ).alias(), u"Klasse"_s );
  //Heading -> Titel  //#spellok
  QCOMPARE( points_fields.field( u"Heading"_s ).alias(), u"Titel"_s ); //#spellok
  //Importance -> Wichtigkeit
  QCOMPARE( points_fields.field( u"Importance"_s ).alias(), u"Wichtigkeit"_s );
  //Pilots -> Piloten
  QCOMPARE( points_fields.field( u"Pilots"_s ).alias(), u"Piloten"_s );
  //Cabin Crew -> Kabinenpersonal
  QCOMPARE( points_fields.field( u"Cabin Crew"_s ).alias(), u"Kabinenpersonal"_s );
  //Staff -> Mitarbeiter
  QCOMPARE( points_fields.field( u"Staff"_s ).alias(), u"Mitarbeiter"_s );

  //FORMCONTAINERS
  const QList<QgsAttributeEditorElement *> elements = points_layer->editFormConfig().invisibleRootContainer()->children();
  QList<QgsAttributeEditorContainer *> containers;
  for ( QgsAttributeEditorElement *element : elements )
  {
    if ( element->type() == Qgis::AttributeEditorType::Container )
      containers.append( dynamic_cast<QgsAttributeEditorContainer *>( element ) );
  }

  //Plane -> Flugzeug
  QCOMPARE( containers.at( 0 )->name(), u"Flugzeug"_s );
  //Employees -> Angestellte
  QCOMPARE( containers.at( 1 )->name(), u"Angestellte"_s );
  //Flightattends -> Flugbegleitung
  for ( QgsAttributeEditorElement *element : containers.at( 1 )->children() )
  {
    if ( element->type() == Qgis::AttributeEditorType::Container )
      QCOMPARE( element->name(), u"Flugbegleitung"_s );
  }

  //RELATIONS
  //Runway -> Piste
  QCOMPARE( QgsProject::instance()->relationManager()->relation( u"points_240_Importance_lines_a677_Value"_s ).name(), u"Piste"_s );
  //Sheepwalk -> Schafweide
  QCOMPARE( QgsProject::instance()->relationManager()->relation( u"points_240_Importance_lines_a677_Value_1"_s ).name(), u"Schafweide"_s );

  //WIDGETS
  //ValueRelation value is not anymore Name but Runwayid
  QCOMPARE( points_fields.field( u"Cabin Crew"_s ).editorWidgetSetup().config().value( u"Value"_s ).toString(), u"Runwayid"_s );

  //ValueMap with descriptions
  const QList<QString> expectedStringValueList = { "Hauptstrasse", "Autobahn", "nix" };
  const QList<QVariant> valueList = lines_fields.field( u"Name"_s ).editorWidgetSetup().config().value( u"map"_s ).toList();
  QList<QString> stringValueList;
  for ( int i = 0, row = 0; i < valueList.count(); i++, row++ )
  {
    stringValueList.append( valueList[i].toMap().constBegin().key() );
  }

  //METADATA
  QCOMPARE( QgsProject::instance()->metadata().title(), u"Metadatentitel"_s );
  QCOMPARE( QgsProject::instance()->metadata().type(), u"Metadatentyp"_s );
  QCOMPARE( QgsProject::instance()->metadata().abstract(), u"Metadaten-Zusammenfassung"_s );
  QCOMPARE( QgsProject::instance()->metadata().author(), u"Webmasterin"_s );

  QCOMPARE( lines_layer->metadata().title(), u"Metadatentitel"_s );
  QCOMPARE( lines_layer->metadata().type(), u"Metadatentyp"_s );
  QCOMPARE( lines_layer->metadata().abstract(), u"Metadaten-Zusammenfassung"_s );
  QCOMPARE( lines_layer->metadata().rights(), QStringList() << u"Metadatenrechte"_s );

  QCOMPARE( stringValueList, expectedStringValueList );

  QString deProjectFileName( TEST_DATA_DIR );
  deProjectFileName = deProjectFileName + "/project_translation/points_translation_de.qgs";
  const QFile deProjectFile( deProjectFileName );
  QVERIFY( deProjectFile.exists() );
}

QGSTEST_MAIN( TestQgsTranslateProject )
#include "testqgstranslateproject.moc"
