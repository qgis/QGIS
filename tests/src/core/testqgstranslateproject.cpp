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

class TestQgsTranslateProject : public QObject
{
    Q_OBJECT

  public:

  private slots:
    void initTestCase();
    void generateTsFile();
    void registerTranslatableObjects( QgsTranslationContext *translationContext );
    void cleanupTestCase();
};

void TestQgsTranslateProject::initTestCase()
{
  QgsApplication::init();
  QgsApplication::initQgis();

  connect( QgsApplication::instance(), &QgsApplication::requestForTranslatableObjects, this, &TestQgsTranslateProject::registerTranslatableObjects );
}

void TestQgsTranslateProject::generateTsFile()
{
  QgsProject *prj = new QgsProject;
  prj->setFileName( "/home/qgis/a-project-file.qgs" ); // not expected to exist

  prj->generateTsFile();

  QFile tsFile( "/home/qgis/a-project-file.ts." );

  //test whether the file has been created
  QVERIFY( tsFile.exists() );

  //test wheter there is the entry for our honeybee

  // QgsProject::instance()->generateTsFile();

  // all cases start with all items checked
}

void TestQgsTranslateProject::registerTranslatableObjects( QgsTranslationContext *translationContext )
{
  translationContext->registerTranslation( QStringLiteral( "testqgstranslateproject:test01" ), "honeybee" );
}

void TestQgsTranslateProject::cleanupTestCase()
{
  //delete ts file
}

QGSTEST_MAIN( TestQgsTranslateProject )
#include "testqgstranslateproject.moc"
