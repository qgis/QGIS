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
  //start application in english
  QgsApplication::init();
  QgsApplication::initQgis();

  original_locale = settings.value( QStringLiteral( "locale/userLocale" ), "" ).toString() ;
  settings.setValue( QStringLiteral( "locale/userLocale" ), "de" );
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
  //open project
  QString projectFileName( TEST_DATA_DIR );
  projectFileName = projectFileName + "/project_translation/points_translation.qgs";
  QgsProject::instance()->read( projectFileName );
}

void TestQgsTranslateProject::cleanup()
{
  //not needed
}


void TestQgsTranslateProject::createTsFile()
{
  //the project should be translated and renamed
  //so base is points_translation_de.qgs and german values
  //then we generate a ts file for spanish

  //create ts-file
  QgsProject::instance()->generateTsFile( "es" );

  //check if tsfile is created
  QString tsFileName( TEST_DATA_DIR );
  tsFileName = tsFileName + "/project_translation/points_translation_de_es.ts";
  QFile tsFile( tsFileName );
  QVERIFY( tsFile.exists() );

  tsFile.open( QIODevice::ReadWrite );

  QString tsFileContent( tsFile.readAll() );

  //check if tsFile contains layer name Punkte
  QVERIFY( tsFileContent.contains( "<source>Punkte</source>" ) );

  //check if tsFile contains layer group name

  //check if tsFile contains alias value

  //check if tsFile contains field name
  // QVERIFY( tsFileContent.contains( "<source>klaso</source>" ) );

  //check if tsFile contains relation name

  tsFile.close();
}

void TestQgsTranslateProject::translateProject()
{
  //it should translate the project to german
  QgsVectorLayer *points_layer = qobject_cast<QgsVectorLayer *>( QgsProject::instance()->mapLayer( "points_240d6bd6_9203_470a_994a_aae13cd9fa04" ) );
  QgsVectorLayer *lines_layer = qobject_cast<QgsVectorLayer *>( QgsProject::instance()->mapLayer( "lines_a677672a_bf5d_410d_98c9_d326a5719a1b" ) );

  //check if layer names translated
  QCOMPARE( points_layer->name(), QStringLiteral( "Punkte" ) );
  QCOMPARE( lines_layer->name(), QStringLiteral( "Linien" ) );

  //check if group name translated

  //check if second group name translated

  //check if first alias value translated

  //check if second alias value translated

  //check if first field name translated to the alias

  //check if second field name translated to the alias

  //check if first relation name translated

  //check if second relation name translated
}

QGSTEST_MAIN( TestQgsTranslateProject )
#include "testqgstranslateproject.moc"
