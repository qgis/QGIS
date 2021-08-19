/***************************************************************************
     testqgsmeshcalculator.cpp
     -------------------------
    Date                 : January 2019
    Copyright            : (C) 2019 by Peter Petrik
    Email                : zilolv at gmail dot com
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
#include "qgsapplication.h"
#include "qgsvectorlayer.h"
#include "qgsmeshlayer.h"
#include "qgsmeshdataprovider.h"
#include "qgsmeshcalculatordialog.h"
#include "qgsfeedback.h"

#include <QTemporaryFile>

/**
 * \ingroup UnitTests
 * This is a unit test for the mesh calculator
 */
class TestQgsMeshCalculatorDialog : public QObject
{
    Q_OBJECT
  public:
    TestQgsMeshCalculatorDialog();

  private slots:
    void initTestCase();// will be called before the first testfunction is executed.
    void cleanupTestCase();// will be called after the last testfunction was executed.
    void init() {} // will be called before each testfunction is executed.
    void cleanup() {} // will be called after every testfunction.

    void testCalc();

  private:
    QgisApp *mQgisApp = nullptr;
    QgsMeshLayer *mpMeshLayer = nullptr;
};

TestQgsMeshCalculatorDialog::TestQgsMeshCalculatorDialog() = default;

//runs before all tests
void TestQgsMeshCalculatorDialog::initTestCase()
{
  qDebug() << "TestQgisAppClipboard::initTestCase()";
  // init QGIS's paths - true means that all path will be inited from prefix
  QgsApplication::init();
  QgsApplication::initQgis();
  mQgisApp = new QgisApp();

  const QString testDataDir = QStringLiteral( TEST_DATA_DIR ) + QStringLiteral( "/mesh/" );
  const QString uri( testDataDir + "/quad_and_triangle.2dm" );
  mpMeshLayer = new QgsMeshLayer( uri, "Triangle and Quad MDAL", "mdal" );
  mpMeshLayer->dataProvider()->addDataset( testDataDir + "/quad_and_triangle_vertex_scalar.dat" );
  mpMeshLayer->dataProvider()->addDataset( testDataDir + "/quad_and_triangle_vertex_scalar2.dat" );
  mpMeshLayer->dataProvider()->addDataset( testDataDir + "/quad_and_triangle_vertex_scalar_max.dat" );
  mpMeshLayer->dataProvider()->addDataset( testDataDir + "/quad_and_triangle_vertex_vector.dat" );
  mpMeshLayer->dataProvider()->addDataset( testDataDir + "/quad_and_triangle_vertex_vector2.dat" );
  mpMeshLayer->dataProvider()->addDataset( testDataDir + "/quad_and_triangle_vertex_vector_max.dat" );
  mpMeshLayer->dataProvider()->addDataset( testDataDir + "/quad_and_triangle_els_face_scalar.dat" );
  mpMeshLayer->dataProvider()->addDataset( testDataDir + "/quad_and_triangle_els_face_vector.dat" );

  QgsProject::instance()->addMapLayers(
    QList<QgsMapLayer *>() << mpMeshLayer );
}

//runs after all tests
void TestQgsMeshCalculatorDialog::cleanupTestCase()
{
  QgsApplication::exitQgis();
}

void TestQgsMeshCalculatorDialog::testCalc()
{
  if ( !QgsTest::runFlakyTests() )
    QSKIP( "This test is disabled on Travis CI environment" );

  std::unique_ptr< QgsMeshCalculatorDialog > dialog( new QgsMeshCalculatorDialog( mpMeshLayer ) );

  const int groupCount = mpMeshLayer->dataProvider()->datasetGroupCount();

  QTemporaryFile tmpFile;
  tmpFile.open(); // fileName is not available until open
  const QString tmpName = tmpFile.fileName();
  tmpFile.close();

  // this next part is fragile, and may need to be modified if the dialog changes:
  dialog->mOutputDatasetFileWidget->setFilePath( tmpName );
  dialog->mExpressionTextEdit->setText( QStringLiteral( "\"VertexScalarDataset\" * 2 " ) );
  dialog->accept();
  std::unique_ptr<QgsMeshCalculator> calculator = dialog->calculator();

  QgsFeedback feedback;
  const QgsMeshCalculator::Result res = calculator->processCalculation( &feedback );
  QCOMPARE( res, QgsMeshCalculator::Success );

  // check result
  const int newGroupCount = mpMeshLayer->dataProvider()->datasetGroupCount();
  QCOMPARE( groupCount + 1, newGroupCount );
}

QGSTEST_MAIN( TestQgsMeshCalculatorDialog )
#include "testqgsmeshcalculatordialog.moc"
