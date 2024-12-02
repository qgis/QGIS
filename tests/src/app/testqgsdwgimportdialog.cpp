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
    void initTestCase();      // will be called before the first testfunction is executed.
    void cleanupTestCase() {} // will be called after the last testfunction was executed.
    void cleanup();           // will be called after every testfunction.

    void importDwgDocument();
    void importDwgDocumentExpandBlockGeometries();
    void importDwgDocumentBlockOnlyInsertPoints();

  private:
    QgisApp *mQgisApp = nullptr;
    QString mDataDir;

    void checkGroupContent( QgsLayerTreeGroup *parentGroup, const QString &groupName, const QStringList &contentLayerNames );
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

void TestQgsDwgImportDialog::importDwgDocument()
{
  QgsDwgImportDialog dwgImportDialog( nullptr, Qt::WindowFlags() );

  // Set target gpkg
  QTemporaryDir temporaryDir;
  dwgImportDialog.mDatabaseFileWidget->setFilePath( temporaryDir.filePath( "entities.gpkg" ) );

  // Set Expand Blocks mode
  const QgsDwgImportDialog::BlockImportFlags blockImportMode( QgsDwgImportDialog::BlockImportExpandGeometry | QgsDwgImportDialog::BlockImportAddInsertPoints );
  const int index = dwgImportDialog.mBlockModeComboBox->findData( static_cast<int>( blockImportMode ) );
  dwgImportDialog.mBlockModeComboBox->setCurrentIndex( index );

  // Set source drawing and import
  QString uri = QString( mDataDir + "/entities.dwg" );
  dwgImportDialog.mSourceDrawingFileWidget->setFilePath( uri );
  dwgImportDialog.pbImportDrawing_clicked();

  // Check that a default group name was assigned
  QCOMPARE( dwgImportDialog.leLayerGroup->text(), "entities" );

  dwgImportDialog.buttonBox_accepted();

  QgsLayerTreeGroup *rootGroup = QgsLayerTree::toGroup( QgisApp::instance()->layerTreeView()->layerTreeModel()->rootGroup() );
  QVERIFY( rootGroup );

  QgsLayerTreeGroup *groupEntities = rootGroup->findGroup( "entities" );
  QVERIFY( groupEntities );

  // Group 0
  checkGroupContent( groupEntities, "0", QStringList() << "hatches" << "lines" << "polylines" << "texts" << "points" << "inserts" );

  // Group grün
  checkGroupContent( groupEntities, "grün", QStringList() << "hatches" << "lines" << "polylines" );
}

void TestQgsDwgImportDialog::importDwgDocumentExpandBlockGeometries()
{
  QgsDwgImportDialog dwgImportDialog( nullptr, Qt::WindowFlags() );

  // Set target gpkg
  QTemporaryDir temporaryDir;
  dwgImportDialog.mDatabaseFileWidget->setFilePath( temporaryDir.filePath( "entitiesExpandBlockGeometries.gpkg" ) );

  // Set Expand Blocks mode
  const QgsDwgImportDialog::BlockImportFlags blockImportMode = QgsDwgImportDialog::BlockImportExpandGeometry;
  const int index = dwgImportDialog.mBlockModeComboBox->findData( static_cast<int>( blockImportMode ) );
  dwgImportDialog.mBlockModeComboBox->setCurrentIndex( index );

  // Set source drawing and import
  QString uri = QString( mDataDir + "/entities.dwg" );
  dwgImportDialog.mSourceDrawingFileWidget->setFilePath( uri );
  dwgImportDialog.pbImportDrawing_clicked();

  // Check that a default group name was assigned
  QCOMPARE( dwgImportDialog.leLayerGroup->text(), "entities" );

  dwgImportDialog.leLayerGroup->setText( "entitiesExpandBlockGeometries" );

  dwgImportDialog.buttonBox_accepted();

  QgsLayerTreeGroup *rootGroup = QgsLayerTree::toGroup( QgisApp::instance()->layerTreeView()->layerTreeModel()->rootGroup() );
  QVERIFY( rootGroup );

  QgsLayerTreeGroup *groupEntities = rootGroup->findGroup( "entitiesExpandBlockGeometries" );
  QVERIFY( groupEntities );

  // Group 0
  checkGroupContent( groupEntities, "0", QStringList() << "hatches" << "lines" << "polylines" << "texts" << "points" );

  // Group grün
  checkGroupContent( groupEntities, "grün", QStringList() << "hatches" << "lines" << "polylines" );
}

void TestQgsDwgImportDialog::importDwgDocumentBlockOnlyInsertPoints()
{
  QgsDwgImportDialog dwgImportDialog( nullptr, Qt::WindowFlags() );

  // Set target gpkg
  QTemporaryDir temporaryDir;
  dwgImportDialog.mDatabaseFileWidget->setFilePath( temporaryDir.filePath( "entitiesBlockOnlyInsertPoints.gpkg" ) );

  // Set Expand Blocks mode
  const QgsDwgImportDialog::BlockImportFlags blockImportMode = QgsDwgImportDialog::BlockImportAddInsertPoints;
  const int index = dwgImportDialog.mBlockModeComboBox->findData( static_cast<int>( blockImportMode ) );
  dwgImportDialog.mBlockModeComboBox->setCurrentIndex( index );

  // Set source drawing and import
  QString uri = QString( mDataDir + "/entities.dwg" );
  dwgImportDialog.mSourceDrawingFileWidget->setFilePath( uri );
  dwgImportDialog.pbImportDrawing_clicked();

  // Check that a default group name was assigned
  QCOMPARE( dwgImportDialog.leLayerGroup->text(), "entities" );

  dwgImportDialog.leLayerGroup->setText( "entitiesBlockOnlyInsertPoints" );

  dwgImportDialog.buttonBox_accepted();

  QgsLayerTreeGroup *rootGroup = QgsLayerTree::toGroup( QgisApp::instance()->layerTreeView()->layerTreeModel()->rootGroup() );
  QVERIFY( rootGroup );

  QgsLayerTreeGroup *groupEntities = rootGroup->findGroup( "entitiesBlockOnlyInsertPoints" );
  QVERIFY( groupEntities );

  // Group 0
  checkGroupContent( groupEntities, "0", QStringList() << "hatches" << "lines" << "polylines" << "texts" << "points" << "inserts" );

  // Group grün
  checkGroupContent( groupEntities, "grün", QStringList() << "hatches" << "lines" << "polylines" );
}

void TestQgsDwgImportDialog::checkGroupContent( QgsLayerTreeGroup *parentGroup, const QString &groupName, const QStringList &contentLayerNames )
{
  // Group
  QgsLayerTreeGroup *group = parentGroup->findGroup( groupName );
  QVERIFY( group );

  QStringList layerNames = contentLayerNames;

  const auto layerTreeLayers = group->findLayers();
  for ( QgsLayerTreeLayer *layerTreeLayer : layerTreeLayers )
    QVERIFY2( layerNames.removeOne( layerTreeLayer->name() ), QString( "Unexpected layer '%1' in group '%2'" ).arg( layerTreeLayer->name(), groupName ).toLatin1() );

  QVERIFY2( layerNames.isEmpty(), QString( "Missing layers in group '%1': '%2'" ).arg( groupName, layerNames.join( ", " ) ).toLatin1() );
}

QGSTEST_MAIN( TestQgsDwgImportDialog )
#include "testqgsdwgimportdialog.moc"
