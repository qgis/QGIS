/***************************************************************************
    testqgsmeshrendererscalarsettingswidget
     --------------------------------------
    Date                 : May 2025
    Copyright            : (C) 2025 Jan Caha
    Email                : jan.caha@outlook.com
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
#include <QString>

#include "qgsapplication.h"
#include "qgsmeshlayer.h"
#include "qgsmaplayertemporalproperties.h"
#include "qgsmeshrendererscalarsettingswidget.h"

/**
 * \ingroup UnitTests
 * This is a unit test to verify that raster histogram works
 */
class TestQgsMeshRendererScalarSettingsWidget : public QObject
{
    Q_OBJECT

  public:
    TestQgsMeshRendererScalarSettingsWidget() {}

  private:
    QgsMeshLayer *mMeshLayer = nullptr;
    const QString mTestDataDir = QStringLiteral( TEST_DATA_DIR ) + QStringLiteral( "/mesh/" );

  private slots:

    // init / cleanup
    void initTestCase();    // will be called before the first testfunction is executed.
    void cleanupTestCase(); // will be called after the last testfunction was executed.

    // tests
    void testScalarSettingsShader();
};

void TestQgsMeshRendererScalarSettingsWidget::initTestCase()
{
  QgsApplication::init();
  QgsApplication::initQgis();

  const QString mMeshFile = mTestDataDir + QStringLiteral( "quad_and_triangle.2dm" );

  mMeshLayer = new QgsMeshLayer( mMeshFile, QStringLiteral( "mesh" ), QStringLiteral( "mdal" ) );
  QVERIFY( mMeshLayer->isValid() );

  mMeshLayer->updateTriangularMesh();
  mMeshLayer->temporalProperties()->setIsActive( false );

  QgsMeshDatasetIndex meshDatasetIndex = QgsMeshDatasetIndex( 0, 0 );
  mMeshLayer->setStaticScalarDatasetIndex( meshDatasetIndex );
}

void TestQgsMeshRendererScalarSettingsWidget::cleanupTestCase()
{
  delete mMeshLayer;
}

void TestQgsMeshRendererScalarSettingsWidget::testScalarSettingsShader()
{
  // related to https://github.com/qgis/QGIS/issues/60864
  // mesh scalar settings shader improperly created resulting in changes in labels and values of the shader

  QgsMeshRendererSettings settings = mMeshLayer->rendererSettings();
  int activeScalarDatasetGroup = settings.activeScalarDatasetGroup();

  // manual styling for layer
  const QString style = mTestDataDir + QStringLiteral( "mesh_custom_shader.qml" );

  bool res;
  mMeshLayer->loadNamedStyle( style, res );

  QVERIFY( res );

  // check ramp shader after the the style load
  QgsMeshRendererScalarSettings scalarSettings = mMeshLayer->rendererSettings().scalarSettings( activeScalarDatasetGroup );
  QgsColorRampShader shader = scalarSettings.colorRampShader();
  qgsDoubleNear( shader.minimumValue(), 24, 0.1 );
  qgsDoubleNear( shader.maximumValue(), 35, 0.1 );
  QCOMPARE( shader.colorRampItemList().size(), 30 );
  QCOMPARE( shader.colorRampItemList().first().label, QStringLiteral( "<=24" ) );
  QCOMPARE( shader.colorRampItemList().last().label, QStringLiteral( ">=35" ) );

  // create widget, sync it to layer
  QgsMeshRendererScalarSettingsWidget *widget = new QgsMeshRendererScalarSettingsWidget( nullptr );
  widget->setLayer( mMeshLayer );
  widget->setActiveDatasetGroup( activeScalarDatasetGroup );
  widget->syncToLayer();

  // get scalar renderer settings from layer - this was source of error as shader was recalculated here (labels and values would change)
  QgsMeshRendererScalarSettings widgetMeshRendererScalarSettings = widget->settings();

  // check that it fits what was loaded from qml file, no changes and recalculations
  shader = widgetMeshRendererScalarSettings.colorRampShader();
  qgsDoubleNear( shader.minimumValue(), 24, 0.1 );
  qgsDoubleNear( shader.maximumValue(), 35, 0.1 );
  QCOMPARE( shader.colorRampItemList().size(), 30 );
  QCOMPARE( shader.colorRampItemList().first().label, QStringLiteral( "<=24" ) );
  QCOMPARE( shader.colorRampItemList().last().label, QStringLiteral( ">=35" ) );
}

QGSTEST_MAIN( TestQgsMeshRendererScalarSettingsWidget )
#include "testqgsmeshrendererscalarsettingswidget.moc"
