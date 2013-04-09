/***************************************************************************
    testqgsdualview.cpp
     --------------------------------------
    Date                 : 14.2.2013
    Copyright            : (C) 2013 Matthias Kuhn
    Email                : matthias dot kuhn at gmx dot ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/


#include <QtTest>

#include <attributetable/qgsattributetableview.h>
#include <attributetable/qgsdualview.h>
#include <qgsapplication.h>
#include <qgsvectorlayer.h>
#include <qgsmapcanvas.h>
#include <qgsfeature.h>

class TestQgsDualView: public QObject
{
    Q_OBJECT;
  private slots:
    void initTestCase(); // will be called before the first testfunction is executed.
    void cleanupTestCase(); // will be called after the last testfunction was executed.
    void init(); // will be called before each testfunction is executed.
    void cleanup(); // will be called after every testfunction.

    void testSelectAll();

  private:
    QgsMapCanvas* mCanvas;
    QgsVectorLayer* mPointsLayer;
    QString mTestDataDir;
    QgsDualView* mDualView;
};

void TestQgsDualView::initTestCase()
{
  QgsApplication::init();
  QgsApplication::initQgis();
  QgsApplication::showSettings();

  // Setup a map canvas with a vector layer loaded...
  QString myDataDir( TEST_DATA_DIR ); //defined in CmakeLists.txt
  mTestDataDir = myDataDir + QDir::separator();

  //
  // load a vector layer
  //
  QString myPointsFileName = mTestDataDir + "points.shp";
  QFileInfo myPointFileInfo( myPointsFileName );
  mPointsLayer = new QgsVectorLayer( myPointFileInfo.filePath(),
                                     myPointFileInfo.completeBaseName(), "ogr" );

  mCanvas = new QgsMapCanvas();
}

void TestQgsDualView::cleanupTestCase()
{
  delete mPointsLayer;
  delete mCanvas;
}

void TestQgsDualView::init()
{
  mDualView = new QgsDualView();
  mDualView->init( mPointsLayer, mCanvas, QgsDistanceArea() );
}

void TestQgsDualView::cleanup()
{
  delete mDualView;
}

void TestQgsDualView::testSelectAll()
{
  mDualView->setFilterMode( QgsAttributeTableFilterModel::ShowVisible );
  // Only show parts of the canvas, so only one selected feature is visible
  mCanvas->setExtent( QgsRectangle( -139, 23, -100, 48 ) );
  mDualView->mTableView->selectAll();
  QVERIFY( mPointsLayer->selectedFeatureCount() == 10 );
  
  mPointsLayer->setSelectedFeatures( QgsFeatureIds() );
  mCanvas->setExtent( QgsRectangle( -110, 40, -100, 48 ) );
  mDualView->mTableView->selectAll();
  QVERIFY( mPointsLayer->selectedFeatureCount() == 1 );
}

QTEST_MAIN( TestQgsDualView )
#include "moc_testqgsdualview.cxx"




