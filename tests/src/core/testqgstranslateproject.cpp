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
#include "qgsaction.h"
#include "qgsactionmanager.h"
#include "qgsapplication.h"
#include "qgsattributeeditorcontainer.h"
#include "qgsattributeeditorelement.h"
#include "qgslayertree.h"
#include "qgslayertreegroup.h"
#include "qgslegendsymbolitem.h"
#include "qgsmaplayer.h"
#include "qgsproject.h"
#include "qgsrelationmanager.h"
#include "qgsrenderer.h"
#include "qgssettings.h"
#include "qgstest.h"
#include "qgstranslationcontext.h"
#include "qgsvectorlayer.h"

#include <QDir>
#include <QObject>
#include <QString>

using namespace Qt::StringLiterals;

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
  //purple lines
  QVERIFY( tsFileContent.contains( "<source>purple lines</source>" ) );
  //purple points
  QVERIFY( tsFileContent.contains( "<source>purple points</source>" ) );

  //LAYER GROUPS AND SUBGROUPS
  //Planes and Roads
  QVERIFY( tsFileContent.contains( "<source>Planes and Roads</source>" ) );
  //Purple Marks
  QVERIFY( tsFileContent.contains( "<source>Purple Marks</source>" ) );
  //Little bit of nothing
  QVERIFY( tsFileContent.contains( "<source>Little bit of nothing</source>" ) );

  //LEGEND ITEMS
  //lines:
  QVERIFY( tsFileContent.contains( "<source>Arterial</source>" ) );
  QVERIFY( tsFileContent.contains( "<source>Highway</source>" ) );
  //purple lines:
  QVERIFY( tsFileContent.contains( "<source>Arterial purple</source>" ) );
  QVERIFY( tsFileContent.contains( "<source>Highway purple</source>" ) );
  QVERIFY( tsFileContent.contains( "<source>Low Highway purple</source>" ) );
  QVERIFY( tsFileContent.contains( "<source>High Highway purple</source>" ) );
  //purple points:
  QVERIFY( tsFileContent.contains( "<source>From 1 to 1</source>" ) );
  QVERIFY( tsFileContent.contains( "<source>From 1 to 3</source>" ) );
  QVERIFY( tsFileContent.contains( "<source>From 3 to 3.6</source>" ) );
  QVERIFY( tsFileContent.contains( "<source>From 3.6 to 10</source>" ) );
  QVERIFY( tsFileContent.contains( "<source>From 10 to 20</source>" ) );

  //ACTIONS
  //descriptions (names)
  QVERIFY( tsFileContent.contains( "<source>Run an application</source>" ) );
  QVERIFY( tsFileContent.contains( "<source>Clicked coordinates (Run feature actions tool)</source>" ) );
  QVERIFY( tsFileContent.contains( "<source>Open file</source>" ) );
  //shorttitles
  QVERIFY( tsFileContent.contains( "<source>Run application</source>" ) );
  QVERIFY( tsFileContent.contains( "<source>Clicked Coordinate</source>" ) );
  QVERIFY( tsFileContent.contains( "<source>Open file</source>" ) );

  //FIELDNAMES AND ALIASES
  //Lines:
  //Name (Alias: Runwayid)
  QVERIFY( tsFileContent.contains( "<source>Runwayid</source>" ) );
  //Value (Alias: Name)
  QVERIFY( tsFileContent.contains( "<source>Name</source>" ) );
  //Name (Constraint Description: Road needs a type)
  QVERIFY( tsFileContent.contains( "<source>Road needs a type</source>" ) );
  //Value (Constraint Description: Value should not be 1337)
  QVERIFY( tsFileContent.contains( "<source>Value should not be 1337</source>" ) );

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
  //ValueRelation description
  QVERIFY( tsFileContent.contains( ":fields:Cabin Crew:valuerelationdescription</name>" ) );
  QVERIFY( tsFileContent.contains( "<source>'The cabin Crew Member is now a '||\"RunwayId\"</source>" ) );

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
  QgsVectorLayer *purple_points_layer = QgsProject::instance()->mapLayer<QgsVectorLayer *>( "points_1c05d011_813c_4234_9b80_7aa5fa9c6d8d" );
  QgsVectorLayer *purple_lines_layer = QgsProject::instance()->mapLayer<QgsVectorLayer *>( "lines_5238db60_470c_41a5_b1d1_059e70f93b99" );

  //LAYER NAMES
  //lines -> Linien
  QCOMPARE( lines_layer->name(), u"Linien"_s ); //#spellok
  //points -> Punkte
  QCOMPARE( points_layer->name(), u"Punkte"_s ); //#spellok
  // purple_lines
  QCOMPARE( purple_lines_layer->name(), u"Lila Linien"_s ); //#spellok
  //purple_points -> Punkte
  QCOMPARE( purple_points_layer->name(), u"Lila Punkte"_s ); //#spellok

  //LAYER GROUPS AND SUBGROUPS
  //Points:
  //Planes and Roads -> Flugzeuge und Strassen
  QVERIFY( QgsProject::instance()->layerTreeRoot()->findGroup( u"Flugzeuge und Strassen"_s ) ); //#spellok
  //Little bit of nothing -> Bisschen nichts
  QVERIFY( QgsProject::instance()->layerTreeRoot()->findGroup( u"Bisschen nichts"_s ) ); //#spellok
  //Purple Marks -> Lila Markierungen
  QVERIFY( QgsProject::instance()->layerTreeRoot()->findGroup( u"Lila Markierungen"_s ) ); //#spellok

  //LEGEND ITEMS
  //lines:
  QList<QString> legendItemsOfLines;
  for ( const QgsLegendSymbolItem &item : lines_layer->renderer()->legendSymbolItems() )
  {
    legendItemsOfLines.append( item.label() );
  }
  QVERIFY( legendItemsOfLines.contains( u"Hauptstrasse"_s ) ); //Arterial //#spellok
  QVERIFY( legendItemsOfLines.contains( u"Autobahn"_s ) );     //Highway //#spellok
  //purple lines:
  QList<QString> legendItemsOfPurpleLine;
  for ( const QgsLegendSymbolItem &item : purple_lines_layer->renderer()->legendSymbolItems() )
  {
    legendItemsOfPurpleLine.append( item.label() );
  }
  QVERIFY( legendItemsOfPurpleLine.contains( u"Lila Hauptstrasse"_s ) );   //Arterial purple //#spellok
  QVERIFY( legendItemsOfPurpleLine.contains( u"Lila Autobahn"_s ) );       //Highway purple //#spellok
  QVERIFY( legendItemsOfPurpleLine.contains( u"Tiefe Lila Autobahn"_s ) ); //Low Highway purple //#spellok
  QVERIFY( legendItemsOfPurpleLine.contains( u"Hohe Lila Autobahn"_s ) );  //High Highway purple //#spellok

  //purple points:
  QList<QString> legendItemsOfPurplePoints;
  for ( const QgsLegendSymbolItem &item : purple_points_layer->renderer()->legendSymbolItems() )
  {
    legendItemsOfPurplePoints.append( item.label() );
  }
  QVERIFY( legendItemsOfPurplePoints.contains( u"Von 1 bis 1"_s ) );    //From 1 to 1 //#spellok
  QVERIFY( legendItemsOfPurplePoints.contains( u"Von 1 bis 3"_s ) );    //From 1 to 3 //#spellok
  QVERIFY( legendItemsOfPurplePoints.contains( u"Von 3 bis 3.6"_s ) );  //From 3 to 3.6 //#spellok
  QVERIFY( legendItemsOfPurplePoints.contains( u"Von 3.6 bis 10"_s ) ); //From 3.6 to 10 //#spellok
  QVERIFY( legendItemsOfPurplePoints.contains( u"Von 10 bis 20"_s ) );  //From 10 to 20 //#spellok

  //lines:
  QList<QString> legendItemsOfLine;
  for ( const QgsLegendSymbolItem &item : lines_layer->renderer()->legendSymbolItems() )
  {
    legendItemsOfLine.append( item.label() );
  }
  QVERIFY( legendItemsOfLine.contains( u"Hauptstrasse"_s ) ); //Arterial //#spellok
  QVERIFY( legendItemsOfLine.contains( u"Autobahn"_s ) );     //Highway //#spellok

  //ACTIONS
  //lines:
  QList<QString> actionDescriptionsOfLine;
  QList<QString> actionShortTitlesOfLine;
  for ( const QgsAction &action : lines_layer->actions()->actions() )
  {
    actionDescriptionsOfLine.append( action.name() );
    actionShortTitlesOfLine.append( action.shortTitle() );
  }

  QVERIFY( actionDescriptionsOfLine.contains( u"Starte ein Programm"_s ) );                            //Run an application //#spellok
  QVERIFY( actionShortTitlesOfLine.contains( u"Starte Programm"_s ) );                                 //Run application //#spellok
  QVERIFY( actionDescriptionsOfLine.contains( u"Clicked coordinates (Run feature actions tool)"_s ) ); //Untranslated: Clicked coordinates (Run feature actions tool) //#spellok
  QVERIFY( actionShortTitlesOfLine.contains( u"Clicked Coordinate"_s ) );                              //Untranslated: Clicked Coordinate //#spellok
  QVERIFY( actionDescriptionsOfLine.contains( u"Dies öffnet eine Datei für Lines"_s ) );               //Open file //#spellok
  QVERIFY( actionShortTitlesOfLine.contains( u"Öffne Datei"_s ) );                                     //Open file //#spellok

  //points:
  QList<QString> actionDescriptionsOfPoints;
  QList<QString> actionShortTitlesOfPoints;
  for ( const QgsAction &action : points_layer->actions()->actions() )
  {
    actionDescriptionsOfPoints.append( action.name() );
    actionShortTitlesOfPoints.append( action.shortTitle() );
  }
  QVERIFY( actionDescriptionsOfPoints.contains( u"Dies öffnet eine Datei für Points"_s ) ); //Open file //#spellok
  QVERIFY( actionShortTitlesOfPoints.contains( u"Open file"_s ) );                          //Untranslated: Open file //#spellok

  //FIELDNAMES AND ALIASES
  //Lines:
  const QgsFields lines_fields = lines_layer->fields();
  //Name (Alias: Runwayid) -> Pistenid
  QCOMPARE( lines_fields.field( u"Name"_s ).alias(), u"Pistenid"_s ); //#spellok
  //Value (Alias: Name) -> Pistenname
  QCOMPARE( lines_fields.field( u"Value"_s ).alias(), u"Pistenname"_s ); //#spellok
  //Name (Constraint Description: Road needs a type)
  QCOMPARE( lines_fields.field( u"Name"_s ).constraints().constraintDescription(), u"Piste braucht eine Art"_s ); //#spellok
  //Value (Constraint Description: Value should not be 1337)
  QCOMPARE( lines_fields.field( u"Value"_s ).constraints().constraintDescription(), u"Wert sollte nicht 1337 sein"_s ); //#spellok

  //Points:
  const QgsFields points_fields = points_layer->fields();
  //Class (Alias: Level) -> Klasse
  QCOMPARE( points_fields.field( u"Class"_s ).alias(), u"Klasse"_s ); //#spellok
  //Heading -> Titel  //#spellok
  QCOMPARE( points_fields.field( u"Heading"_s ).alias(), u"Titel"_s ); //#spellok
  //Importance -> Wichtigkeit
  QCOMPARE( points_fields.field( u"Importance"_s ).alias(), u"Wichtigkeit"_s ); //#spellok
  //Pilots -> Piloten
  QCOMPARE( points_fields.field( u"Pilots"_s ).alias(), u"Piloten"_s ); //#spellok
  //Cabin Crew -> Kabinenpersonal
  QCOMPARE( points_fields.field( u"Cabin Crew"_s ).alias(), u"Kabinenpersonal"_s ); //#spellok
  //Staff -> Mitarbeiter
  QCOMPARE( points_fields.field( u"Staff"_s ).alias(), u"Mitarbeiter"_s ); //#spellok

  //FORMCONTAINERS
  const QList<QgsAttributeEditorElement *> elements = points_layer->editFormConfig().invisibleRootContainer()->children();
  QList<QgsAttributeEditorContainer *> containers;
  for ( QgsAttributeEditorElement *element : elements )
  {
    if ( element->type() == Qgis::AttributeEditorType::Container )
      containers.append( dynamic_cast<QgsAttributeEditorContainer *>( element ) );
  }

  //Plane -> Flugzeug
  QCOMPARE( containers.at( 0 )->name(), u"Flugzeug"_s ); //#spellok
  //Employees -> Angestellte
  QCOMPARE( containers.at( 1 )->name(), u"Angestellte"_s ); //#spellok
  //Flightattends -> Flugbegleitung
  for ( QgsAttributeEditorElement *element : containers.at( 1 )->children() )
  {
    if ( element->type() == Qgis::AttributeEditorType::Container )
      QCOMPARE( element->name(), u"Flugbegleitung"_s ); //#spellok
  }

  //RELATIONS
  //Runway -> Piste
  QCOMPARE( QgsProject::instance()->relationManager()->relation( u"points_240_Importance_lines_a677_Value"_s ).name(), u"Piste"_s ); //#spellok
  //Sheepwalk -> Schafweide
  QCOMPARE( QgsProject::instance()->relationManager()->relation( u"points_240_Importance_lines_a677_Value_1"_s ).name(), u"Schafweide"_s ); //#spellok

  //WIDGETS
  //ValueRelation value is not anymore Name but Runwayid
  QCOMPARE( points_fields.field( u"Cabin Crew"_s ).editorWidgetSetup().config().value( u"Value"_s ).toString(), u"Pistenid"_s ); //#spellok
  //ValueRelation description is not anymore 'The cabin Crew Member is now a '||"RunwayId" but 'Das Mitglied des Kabinenpersonals ist jetzt eine '||"Pistenid"
  QCOMPARE( points_fields.field( u"Cabin Crew"_s ).editorWidgetSetup().config().value( u"Description"_s ).toString(), u"'Das Mitglied des Kabinenpersonals ist jetzt eine '||\"Pistenid\""_s ); //#spellok

  //ValueMap with descriptions
  const QList<QString> expectedStringValueList = { "Hauptstrasse", "Autobahn", "nix" }; //#spellok
  const QList<QVariant> valueList = lines_fields.field( u"Name"_s ).editorWidgetSetup().config().value( u"map"_s ).toList();
  QList<QString> stringValueList;
  for ( int i = 0, row = 0; i < valueList.count(); i++, row++ )
  {
    stringValueList.append( valueList[i].toMap().constBegin().key() );
  }

  //METADATA
  QCOMPARE( QgsProject::instance()->metadata().title(), u"Metadatentitel"_s );               //#spellok
  QCOMPARE( QgsProject::instance()->metadata().type(), u"Metadatentyp"_s );                  //#spellok
  QCOMPARE( QgsProject::instance()->metadata().abstract(), u"Metadaten-Zusammenfassung"_s ); //#spellok
  QCOMPARE( QgsProject::instance()->metadata().author(), u"Webmasterin"_s );                 //#spellok

  QCOMPARE( lines_layer->metadata().title(), u"Metadatentitel"_s );                    //#spellok
  QCOMPARE( lines_layer->metadata().type(), u"Metadatentyp"_s );                       //#spellok
  QCOMPARE( lines_layer->metadata().abstract(), u"Metadaten-Zusammenfassung"_s );      //#spellok
  QCOMPARE( lines_layer->metadata().rights(), QStringList() << u"Metadatenrechte"_s ); //#spellok

  QCOMPARE( stringValueList, expectedStringValueList );

  QString deProjectFileName( TEST_DATA_DIR );
  deProjectFileName = deProjectFileName + "/project_translation/points_translation_de.qgs";
  const QFile deProjectFile( deProjectFileName );
  QVERIFY( deProjectFile.exists() );
}

QGSTEST_MAIN( TestQgsTranslateProject )
#include "testqgstranslateproject.moc"
