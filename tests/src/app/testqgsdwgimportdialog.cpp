/***************************************************************************
  testqgsdwgimportdialog.cpp

 ---------------------
 begin                : 23.5.2023
 copyright            : (C) 2023 by Damiano Lombardi
 email                : damiano@opengis.ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgstest.h"
#include "qgisapp.h"
#include "testqgsmaptoolutils.h"
#include "qgslayertree.h"
#include "qgslayertreemodel.h"
#include "qgslayertreeview.h"
#include "qgsdwgimportdialog.h"

class TestQgsDwgImportDialog : public QObject
{
    Q_OBJECT
  public:
    TestQgsDwgImportDialog() = default;

  private slots:
    void initTestCase();// will be called before the first testfunction is executed.
    void cleanupTestCase() {} // will be called after the last testfunction was executed.
    void cleanup(); // will be called after every testfunction.

    void importDxfDocument();

  private:
    QgisApp *mQgisApp = nullptr;
    QString mDataDir;
};


//runs before all tests
void TestQgsDwgImportDialog::initTestCase()
{
  QgsApplication::init();
  QgsApplication::initQgis();
  mQgisApp = new QgisApp();
  mDataDir = QString( TEST_DATA_DIR ); //defined in CmakeLists.txt
  mDataDir += "/dwg";
}

void TestQgsDwgImportDialog::cleanup()
{
  QgsProject::instance()->removeAllMapLayers();
}

void TestQgsDwgImportDialog::importDxfDocument()
{

  QgsDwgImportDialog dwgImportDialog( nullptr, Qt::WindowFlags() );

  // Set target gpkg
  QTemporaryDir temporaryDir;
  dwgImportDialog.mDatabaseFileWidget->setFilePath( temporaryDir.filePath( "entities.gpkg" ) );

  // Set source drawing and import
  QString uri = QString( mDataDir + "/entities.dwg" );
  dwgImportDialog.leDrawing->setText( uri );
  dwgImportDialog.pbImportDrawing_clicked();

  // Check that a default group name was assigned
  QCOMPARE( dwgImportDialog.leLayerGroup->text(), "entities" );

  dwgImportDialog.buttonBox_accepted();

  QgsLayerTreeGroup *rootGroup = QgsLayerTree::toGroup( QgisApp::instance()->layerTreeView()->layerTreeModel()->rootGroup() );
  QVERIFY( rootGroup );

  QgsLayerTreeGroup *groupEntities = rootGroup->findGroup( "entities" );
  QVERIFY( groupEntities );

  // Group 0
  QgsLayerTreeGroup *group0 = groupEntities->findGroup( "0" );
  QVERIFY( group0 );

  QStringList layerNames = QStringList() << "hatches"
                           << "lines"
                           << "polylines"
                           << "texts"
                           << "points"
                           << "inserts";
  const auto layerTreeLayers0 = group0->findLayers();
  for ( QgsLayerTreeLayer *layerTreeLayer : layerTreeLayers0 )
    QVERIFY2( layerNames.removeOne( layerTreeLayer->name() ), QString( "Unexpected layer name: '%1'" ).arg( layerTreeLayer->name() ).toLatin1() );

  QVERIFY2( layerNames.isEmpty(), QString( "Missing layers in group '0': '%1'" ).arg( layerNames.join( ", " ) ).toLatin1() );

  // Group grün
  QgsLayerTreeGroup *groupGruen = groupEntities->findGroup( "grün" );
  QVERIFY( groupGruen );

  layerNames = QStringList() << "hatches"
               << "lines"
               << "polylines";
  const auto layerTreeLayersGruen = group0->findLayers();
  for ( QgsLayerTreeLayer *layerTreeLayer : layerTreeLayersGruen )
    QVERIFY2( layerNames.removeOne( layerTreeLayer->name() ), QString( "Unexpected layer name: '%1'" ).arg( layerTreeLayer->name() ).toLatin1() );

  QVERIFY2( layerNames.isEmpty(), QString( "Missing layers in group 'grün': '%1'" ).arg( layerNames.join( ", " ) ).toLatin1() );
}

QGSTEST_MAIN( TestQgsDwgImportDialog )
#include "testqgsdwgimportdialog.moc"
