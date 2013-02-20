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

class TestQgsDualView: public QObject
{
    Q_OBJECT;
  private slots:
    void initTestCase(); // will be called before the first testfunction is executed.
    void cleanupTestCase(); // will be called after the last testfunction was executed.
    void init(); // will be called before each testfunction is executed.
    void cleanup(); // will be called after every testfunction.

    void testSelection();

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

void TestQgsDualView::testSelection()
{
  // Select some features on the map canvas
  QgsFeatureIds selectedIds;
  selectedIds << 1 << 2 << 3;

  mPointsLayer->setSelectedFeatures( selectedIds );

  // Verify the same selection applies to the table
  QVERIFY( mDualView->mFilterModel->masterSelection()->selectedIndexes().count() == 3 );
  QVERIFY( mDualView->mTableView->selectionModel()->selectedRows().count() == 3 );

  // Set the extent, so all features are visible
  mCanvas->setExtent( QgsRectangle( -139, 22, -64, 48 ) );
  // The table should also only show visible items (still all)
  mDualView->setFilterMode( QgsAttributeTableFilterModel::ShowVisible );
  QVERIFY( mDualView->mFilterModel->masterSelection()->selectedIndexes().count() == 3 );
  QVERIFY( mDualView->mTableView->selectionModel()->selectedRows().count() == 3 );

  // Set the extent, so no features are visible
  mCanvas->setExtent( QgsRectangle( 0, 1, 0, 1 ) );

  // The master selection should still hold all the features, while the currently visible should not
  QVERIFY( mDualView->mFilterModel->masterSelection()->selectedIndexes().count() == 3 );
  QVERIFY( mDualView->mTableView->selectionModel()->selectedRows().count() == 0 );

  // Only show parts of the canvas, so only one selected feature is visible
  mCanvas->setExtent( QgsRectangle( -139, 22, -100, 48 ) );
  QVERIFY( mDualView->mFilterModel->masterSelection()->selectedIndexes().count() == 3 );
  QVERIFY( mDualView->mTableView->selectionModel()->selectedRows().count() == 1 );
  QVERIFY( mDualView->mTableView->selectionModel()->selectedRows().first().row() == 1 );

  // Now the other way round...
  // TODO: Fixme
  mDualView->mTableView->selectAll();
  QVERIFY( mPointsLayer->selectedFeaturesIds().count() == 12 );

  // Deselect a previously selected row
  // List index 2 => fid 2
  QVERIFY( mPointsLayer->selectedFeaturesIds().contains( 2 ) );
  mDualView->mTableView->selectRow( 2 );
  QVERIFY( false == mPointsLayer->selectedFeaturesIds().contains( 2 ) );

  // Select a previously not selected row
  // List index 6 => fid 13
  QVERIFY( false == mPointsLayer->selectedFeaturesIds().contains( 13 ) );
  mDualView->mTableView->selectRow( 6 );
  QVERIFY( mPointsLayer->selectedFeaturesIds().contains( 13 ) );
}

QTEST_MAIN( TestQgsDualView )
#include "moc_testqgsdualview.cxx"




