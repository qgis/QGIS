/***************************************************************************
     testqgsvectorlayerupdater.cpp
     --------------------------------------
    Date                 : Feb 2018
    Copyright            : (C) 2018 by Arnaud Morvan
    Email                : arnaud dot morvan at coamptocamp dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsvectorlayerupdater.h"
#include "qgsproject.h"
#include "qgsvectorlayer.h"
#include "qgsvectorfilewriter.h"
#include "qgsapplication.h"
#include <QObject>
#include "qgstest.h"


class TestQgsVectorLayerUpdater : public QObject
{
    Q_OBJECT
  public:

  private slots:
    void initTestCase();// will be called before the first testfunction is executed.
    void cleanupTestCase();// will be called after the last testfunction was executed.
    void init();// will be called before each testfunction is executed.
    void cleanup();// will be called after every testfunction.

    void update();

  private:
    QgsVectorLayer mLayer;
};

void TestQgsVectorLayerUpdater::initTestCase()
{
  QgsApplication::init();
  QgsApplication::initQgis();

  QString testDataDir( TEST_DATA_DIR );  //defined in CMakeLists.txt
  //QString templatePath = testDataDir + "/polys_with_id.shp";
  //QString templatePath = testDataDir + "/points.shp";
  QString tempDir = QDir::tempPath();
  QFile::copy( testDataDir + "/points.shp", tempDir + "/test_updater.shp" );
  QFile::copy( testDataDir + "/points.dbf", tempDir + "/test_updater.dbf" );
  QFile::copy( testDataDir + "/points.shx", tempDir + "/test_updater.shx" );
  QFile::copy( testDataDir + "/points.prj", tempDir + "/test_updater.prj" );

  QString layerPath =  QDir::tempPath() + "/test_updater.shp";

  QgsVectorLayer *layer = new QgsVectorLayer( layerPath, QStringLiteral( "layer1" ), QStringLiteral( "ogr" ) );
  QVERIFY( layer->isValid() );

  layer->beginEditCommand( "Add primary key" );
  //QgsVectorDataProvider *dp = layer->dataProvider();
  layer->addAttribute( QgsField( QStringLiteral( "id" ), QVariant::Int ) );
  QgsFeatureIterator it = layer->getFeatures();
  QgsFeature f;
  int i = 0;
  while ( it.nextFeature( f ) )
  {
    f.setAttribute( "id", i );
  }
  layer->endEditCommand();
  layer->commitChanges();
}

void TestQgsVectorLayerUpdater::cleanupTestCase()
{
  QgsApplication::exitQgis();

  //QString layerPath =  QDir::tempPath() + "/test_updater.shp";
  //QFile::remove( layerPath );
}

void TestQgsVectorLayerUpdater::init()
{

}

void TestQgsVectorLayerUpdater::cleanup()
{

}


void TestQgsVectorLayerUpdater::update()
{
  QString testDataDir( TEST_DATA_DIR );  //defined in CMakeLists.txt
  QString srcPath =  testDataDir + "/lines.shp";
  QString dstPath =  QDir::tempPath() + "/test_updater.shp";
  const QgsVectorLayer &srcLayer = QgsVectorLayer( srcPath, QStringLiteral( "layer1" ), QStringLiteral( "ogr" ) );
  const QgsVectorLayer &dstLayer = QgsVectorLayer( dstPath, QStringLiteral( "layer1" ), QStringLiteral( "ogr" ) );
  QVERIFY( srcLayer.isValid() );
  QVERIFY( dstLayer.isValid() );

  const QMap<QString, QgsExpression> map = QMap<QString, QgsExpression>();
  const QList<QString> primaryKeys = QList<QString>() << QStringLiteral( "id" );

  const QgsVectorLayerUpdater &updater = QgsVectorLayerUpdater( dstLayer, map, srcLayer.createExpressionContext(), primaryKeys );

  /*
  QgsVectorLayerUpdater(
        QgsVectorLayer &dstlayer,
        const QMap<QString, QgsExpression> fieldsMap,
        QgsExpressionContext &context,
        const QList<QString> primaryKey = QList<QString>(),
        const bool bypassEditBuffer = false
      );
      */
}

QGSTEST_MAIN( TestQgsVectorLayerUpdater )
#include "testqgsvectorlayerupdater.moc"
