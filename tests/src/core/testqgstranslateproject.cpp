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
#include "qgstest.h"

#include <QObject>
#include <QDir>

#include "qgsapplication.h"
#include "qgstranslationcontext.h"
#include "qgsproject.h"
#include "qgssettings.h"
#include <qgsmaplayer.h>
#include <qgsvectorlayer.h>
#include <qgslayertree.h>
#include <qgslayertreegroup.h>
#include "qgsrelationmanager.h"

class TestQgsTranslateProject : public QObject
{
    Q_OBJECT

  public:

  private slots:
    void initTestCase();// will be called before the first testfunction is executed.
    void cleanupTestCase();// will be called after the last testfunction was executed.
    void init(); // will be called before each testfunction is executed.
    void cleanup(); // will be called after every testfunction.

    void createTsFile();
    void translateProject();

  private:
    QgsSettings settings;
    QString original_locale;

};

void TestQgsTranslateProject::initTestCase()
{
  //start application
  QgsApplication::init();
  QgsApplication::initQgis();

  original_locale = settings.value( QStringLiteral( "locale/userLocale" ), "" ).toString() ;
}

void TestQgsTranslateProject::cleanupTestCase()
{
  settings.setValue( QStringLiteral( "locale/userLocale" ), original_locale );
  QgsApplication::exitQgis();

  //delete translated project file
  QString translatedProjectFileName( TEST_DATA_DIR );
  translatedProjectFileName = translatedProjectFileName + "/project_translation/points_translation_de.qgs";
  QFile translatedProjectFile( translatedProjectFileName );
  translatedProjectFile.remove();

  //delete created ts file
  QString tsFileName( TEST_DATA_DIR );
  tsFileName = tsFileName + "/project_translation/points_translation_de.ts";
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
  settings.setValue( QStringLiteral( "locale/userLocale" ), "en" );
  QString projectFileName( TEST_DATA_DIR );
  projectFileName = projectFileName + "/project_translation/points_translation.qgs";
  QgsProject::instance()->read( projectFileName );

  //create ts file for german
  QgsProject::instance()->generateTsFile( "de" );

  //check if ts file is created
  QString tsFileName( TEST_DATA_DIR );
  tsFileName = tsFileName + "/project_translation/points_translation_de.ts";
  QFile tsFile( tsFileName );
  QVERIFY( tsFile.exists() );

  tsFile.open( QIODevice::ReadWrite );

  QString tsFileContent( tsFile.readAll() );

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

  tsFile.close();
}

void TestQgsTranslateProject::translateProject()
{
  //open project in german
  settings.setValue( QStringLiteral( "locale/userLocale" ), "de" );
  QString projectFileName( TEST_DATA_DIR );
  projectFileName = projectFileName + "/project_translation/points_translation.qgs";
  QgsProject::instance()->read( projectFileName );

  //with the qm file containing translation from en to de, the project should be in german and renamed with postfix .de
  QgsVectorLayer *points_layer = qobject_cast<QgsVectorLayer *>( QgsProject::instance()->mapLayer( "points_240d6bd6_9203_470a_994a_aae13cd9fa04" ) );
  QgsVectorLayer *lines_layer = qobject_cast<QgsVectorLayer *>( QgsProject::instance()->mapLayer( "lines_a677672a_bf5d_410d_98c9_d326a5719a1b" ) );

  //LAYER NAMES
  //lines -> Linien
  QCOMPARE( lines_layer->name(), QStringLiteral( "Linien" ) );
  //points -> Punkte
  QCOMPARE( points_layer->name(), QStringLiteral( "Punkte" ) );

  //LAYER GROUPS AND SUBGROUPS
  //Points:
  //Planes and Roads -> Flugzeuge und Strassen
  QVERIFY( QgsProject::instance()->layerTreeRoot()->findGroup( QStringLiteral( "Flugzeuge und Strassen" ) ) );
  //Little bit of nothing -> Bisschen nichts
  QVERIFY( QgsProject::instance()->layerTreeRoot()->findGroup( QStringLiteral( "Bisschen nichts" ) ) );

  //FIELDS AND ALIASES
  //Lines:
  const QgsFields lines_fields = lines_layer->fields();
  //Name (Alias: Runwayid) -> Pistenid
  QCOMPARE( lines_fields.field( QStringLiteral( "Name" ) ).alias(), QStringLiteral( "Pistenid" ) );
  //Value (Alias: Name) -> Pistenname
  QCOMPARE( lines_fields.field( QStringLiteral( "Value" ) ).alias(), QStringLiteral( "Pistenname" ) );

  //Points:
  const QgsFields points_fields = points_layer->fields();
  //Class (Alias: Level) -> Klasse
  QCOMPARE( points_fields.field( QStringLiteral( "Class" ) ).alias(), QStringLiteral( "Klasse" ) );
  //Heading -> Titel
  QCOMPARE( points_fields.field( QStringLiteral( "Heading" ) ).alias(), QStringLiteral( "Titel" ) );
  //Importance -> Wichtigkeit
  QCOMPARE( points_fields.field( QStringLiteral( "Importance" ) ).alias(), QStringLiteral( "Wichtigkeit" ) );
  //Pilots -> Piloten
  QCOMPARE( points_fields.field( QStringLiteral( "Pilots" ) ).alias(), QStringLiteral( "Piloten" ) );
  //Cabin Crew -> Kabinenpersonal
  QCOMPARE( points_fields.field( QStringLiteral( "Cabin Crew" ) ).alias(), QStringLiteral( "Kabinenpersonal" ) );
  //Staff -> Mitarbeiter
  QCOMPARE( points_fields.field( QStringLiteral( "Staff" ) ).alias(), QStringLiteral( "Mitarbeiter" ) );

  //FORMCONTAINERS
  QList<QgsAttributeEditorElement *> elements = points_layer->editFormConfig().invisibleRootContainer()->children();
  QList<QgsAttributeEditorContainer *> containers;
  for ( QgsAttributeEditorElement *element : elements )
  {
    if ( element->type() == QgsAttributeEditorElement::AeTypeContainer )
      containers.append( dynamic_cast<QgsAttributeEditorContainer *>( element ) );
  }

  //Plane -> Flugzeug
  QCOMPARE( containers.at( 0 )->name(), QStringLiteral( "Flugzeug" ) );
  //Employees -> Angestellte
  QCOMPARE( containers.at( 1 )->name(), QStringLiteral( "Angestellte" ) );
  //Flightattends -> Flugbegleitung
  for ( QgsAttributeEditorElement *element : containers.at( 1 )->children() )
  {
    if ( element->type() == QgsAttributeEditorElement::AeTypeContainer )
      QCOMPARE( element->name(), QStringLiteral( "Flugbegleitung" ) );
  }

  //RELATIONS
  //Runway -> Piste
  QCOMPARE( QgsProject::instance()->relationManager()->relation( QStringLiteral( "points_240_Importance_lines_a677_Value" ) ).name(), QStringLiteral( "Piste" ) );
  //Sheepwalk -> Schafweide
  QCOMPARE( QgsProject::instance()->relationManager()->relation( QStringLiteral( "points_240_Importance_lines_a677_Value_1" ) ).name(), QStringLiteral( "Schafweide" ) );

  QString deProjectFileName( TEST_DATA_DIR );
  deProjectFileName = deProjectFileName + "/project_translation/points_translation_de.qgs";
  QFile deProjectFile( deProjectFileName );
  QVERIFY( deProjectFile.exists() );
}

QGSTEST_MAIN( TestQgsTranslateProject )
#include "testqgstranslateproject.moc"
