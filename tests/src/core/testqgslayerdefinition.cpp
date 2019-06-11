/***************************************************************************
    testqgslayerdefinition.cpp
     --------------------------------------
    Date                 : 07.06.2018
    Copyright            : (C) 2018 Alessandro Pasotti
    Email                : elpaso at itopen dot it
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
#include <QTemporaryFile>
#include <QTemporaryDir>

#include <qgsapplication.h>
#include <qgsproject.h>
#include <qgslayertree.h>
#include <qgslayerdefinition.h>

class TestQgsLayerDefinition: public QObject
{
    Q_OBJECT
  public:
    TestQgsLayerDefinition() = default;

  private slots:
    void initTestCase(); // will be called before the first testfunction is executed.
    void cleanupTestCase(); // will be called after the last testfunction was executed.
    void init(); // will be called before each testfunction is executed.
    void cleanup(); // will be called after every testfunction.

    /**
     * test findLayers()
     */
    void testFindLayers();

    /**
     * test that export does not crash: regression #18981
     * https://github.com/qgis/QGIS/issues/26812 - Save QLR crashes QGIS 3
     */
    void testExportDoesNotCrash();

  private:
    QTemporaryFile *mTempFile;
};


void TestQgsLayerDefinition::initTestCase()
{
  QgsApplication::init();
  QgsApplication::initQgis();

}

void TestQgsLayerDefinition::cleanupTestCase()
{
  QgsApplication::exitQgis();
}

void TestQgsLayerDefinition::init()
{
  mTempFile = new QTemporaryFile( QStringLiteral( "qgis_XXXXXX.qlr" ) );
  QVERIFY( mTempFile->open() );
  mTempFile->close();
  QString errorMessage;
  const QString path = QString( TEST_DATA_DIR + QStringLiteral( "/bug_18981_broken.qlr" ) );
  QgsProject::instance()->removeAllMapLayers();
  QgsProject::instance()->layerTreeRoot()->removeAllChildren();
  QgsLayerDefinition::loadLayerDefinition( path, QgsProject::instance(), QgsProject::instance()->layerTreeRoot(), errorMessage );
  QVERIFY( errorMessage.isEmpty() );
}


void TestQgsLayerDefinition::cleanup()
{
  delete mTempFile;
}

void TestQgsLayerDefinition::testFindLayers()
{
  QCOMPARE( QgsProject::instance()->layerTreeRoot()->findLayers().count(), 2 );
  QCOMPARE( QgsProject::instance()->layerTreeRoot()->findLayers().at( 0 )->name(), QStringLiteral( "DTK/D850" ) );
  QCOMPARE( QgsProject::instance()->layerTreeRoot()->findLayers().at( 1 )->name(), QStringLiteral( "NewMemory" ) );
}

void TestQgsLayerDefinition::testExportDoesNotCrash()
{
  QString errorMessage;
  QVERIFY( QgsLayerDefinition::exportLayerDefinition( mTempFile->fileName(), QgsProject::instance()->layerTreeRoot()->children(), errorMessage ) );
  QVERIFY( errorMessage.isEmpty() );
  // Reload
  QgsProject::instance()->removeAllMapLayers();
  QgsProject::instance()->layerTreeRoot()->removeAllChildren();
  QgsLayerDefinition::loadLayerDefinition( mTempFile->fileName(), QgsProject::instance(), QgsProject::instance()->layerTreeRoot(), errorMessage );
  testFindLayers();
}



QGSTEST_MAIN( TestQgsLayerDefinition )
#include "testqgslayerdefinition.moc"


