/***************************************************************************
     testqgslayerpropertiesdialogs.cpp
     ------------------------------
    Date                 : October 2023
    Copyright            : (C) 2023 by Nyall Dawson
    Email                : nyall dot dawson at gmail dot com
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

#include "qgisapp.h"
#include "qgsapplication.h"
#include "qgsmapcanvas.h"
#include "qgsmessagebar.h"
#include "qgsvectorlayer.h"
#include "qgsvectorlayerproperties.h"
#include "qgsrasterlayer.h"
#include "qgsrasterlayerproperties.h"
#include "qgsmeshlayer.h"
#include "qgsmeshlayerproperties.h"
#include "qgspointcloudlayer.h"
#include "qgspointcloudlayerproperties.h"
#include "qgsvectortilelayer.h"
#include "qgsvectortilelayerproperties.h"
#include "qgstiledscenelayer.h"
#include "qgstiledscenelayerproperties.h"
#include "qgsannotationlayer.h"
#include "annotations/qgsannotationlayerproperties.h"
#include "qgssinglesymbolrenderer.h"
#include "qgssymbol.h"
#include "qgsmarkersymbol.h"
#include "qgsprovidersourcewidget.h"

class DummySourceWidget : public QgsProviderSourceWidget
{
    Q_OBJECT
  public:

    DummySourceWidget( QWidget *parent ) : QgsProviderSourceWidget( parent )
    {

    }

    void setSourceUri( const QString &uri ) override { Q_UNUSED( uri ); }

    QString sourceUri() const override
    {
      return newSource;
    }

    QString newSource;

};

class TestQgsLayerPropertiesDialogs : public QgsTest
{
    Q_OBJECT

  public:
    TestQgsLayerPropertiesDialogs()
      : QgsTest( QStringLiteral( "Layer properties dialogs" ) )
    {}

  private:
    QString mTestDataDir;
    QgisApp *mQgisApp = nullptr;

  private slots:

    void initTestCase()
    {
      QgsApplication::init();
      QgsApplication::initQgis();
      mQgisApp = new QgisApp();

      const QString myDataDir( TEST_DATA_DIR );
      mTestDataDir = myDataDir + '/';
    }

    void cleanupTestCase()
    {
      QgsApplication::exitQgis();
    }

    void testValidVectorProperties()
    {
      // valid vector layer
      const QString pointFileName = mTestDataDir + "points.shp";
      const QFileInfo pointFileInfo( pointFileName );
      std::unique_ptr< QgsVectorLayer > vl = std::make_unique< QgsVectorLayer >( pointFileInfo.filePath(),
                                             pointFileInfo.completeBaseName(), QStringLiteral( "ogr" ) );
      QVERIFY( vl->isValid() );

      QgsMapCanvas canvas;
      QgsMessageBar messageBar;
      QgsVectorLayerProperties dialog( &canvas, &messageBar, vl.get() );
      dialog.show();
      dialog.accept();
    }

    void testInvalidVectorProperties()
    {
      // invalid vector layer
      const QString pointFileName = mTestDataDir + "xxpoints.shp";
      const QFileInfo pointFileInfo( pointFileName );
      std::unique_ptr< QgsVectorLayer > vl = std::make_unique< QgsVectorLayer >( pointFileInfo.filePath(),
                                             pointFileInfo.completeBaseName(), QStringLiteral( "xxogr" ) );
      QVERIFY( !vl->isValid() );

      QgsMapCanvas canvas;
      QgsMessageBar messageBar;
      QgsVectorLayerProperties dialog( &canvas, &messageBar, vl.get() );
      dialog.show();
      dialog.accept();
    }

    void testChangeVectorSubset()
    {
      // start with a point layer
      const QString pointFileName = mTestDataDir + "points.shp";
      const QFileInfo pointFileInfo( pointFileName );
      std::unique_ptr< QgsVectorLayer > vl = std::make_unique< QgsVectorLayer >( pointFileInfo.filePath(),
                                             pointFileInfo.completeBaseName(), QStringLiteral( "ogr" ) );
      QVERIFY( vl->isValid() );
      vl->setSubsetString( QStringLiteral( "\"class\"='Biplane'" ) );
      QCOMPARE( vl->subsetString(), QStringLiteral( "\"class\"='Biplane'" ) );

      // no change to filter
      QgsMapCanvas canvas;
      QgsMessageBar messageBar;
      {
        QgsVectorLayerProperties dialog( &canvas, &messageBar, vl.get() );
        dialog.show();
        dialog.accept();
      }

      QCOMPARE( vl->subsetString(), QStringLiteral( "\"class\"='Biplane'" ) );

      // change the filter to a line layer:
      {
        QgsVectorLayerProperties dialog( &canvas, &messageBar, vl.get() );
        dialog.txtSubsetSQL->setText( QStringLiteral( "\"class\"='B52'" ) );
        dialog.show();
        dialog.accept();
      }
      QCOMPARE( vl->subsetString(), QStringLiteral( "\"class\"='B52'" ) );

      // try with BOTH a filter change and the source widget present, to check interaction of the two
      {
        QgsVectorLayerProperties dialog( &canvas, &messageBar, vl.get() );
        DummySourceWidget *sourceWidget = new DummySourceWidget( &dialog );
        sourceWidget->newSource = mTestDataDir + "points.shp";
        dialog.mSourceWidget = sourceWidget;
        dialog.show();
        dialog.txtSubsetSQL->setText( QStringLiteral( "\"class\"='Biplane'" ) );
        dialog.accept();
      }
      QCOMPARE( vl->source(), mTestDataDir + "points.shp|subset=\"class\"='Biplane'" );
      QCOMPARE( vl->subsetString(), QStringLiteral( "\"class\"='Biplane'" ) );

      // try with BOTH a filter change AND a source change
      {
        QgsVectorLayerProperties dialog( &canvas, &messageBar, vl.get() );
        DummySourceWidget *sourceWidget = new DummySourceWidget( &dialog );
        sourceWidget->newSource = mTestDataDir + "lines.shp";
        dialog.mSourceWidget = sourceWidget;
        dialog.show();
        dialog.txtSubsetSQL->setText( QStringLiteral( "\"Name\" = 'Highway'" ) );
        dialog.accept();
      }
      QCOMPARE( vl->source(), mTestDataDir + "lines.shp|subset=\"Name\" = 'Highway'" );
      QCOMPARE( vl->subsetString(), QStringLiteral( "\"Name\" = 'Highway'" ) );
    }

    void testChangeVectorDataSource()
    {
      // start with a point layer
      const QString pointFileName = mTestDataDir + "points.shp";
      const QFileInfo pointFileInfo( pointFileName );
      std::unique_ptr< QgsVectorLayer > vl = std::make_unique< QgsVectorLayer >( pointFileInfo.filePath(),
                                             pointFileInfo.completeBaseName(), QStringLiteral( "ogr" ) );
      QVERIFY( vl->isValid() );
      // point layer should have a marker symbol
      vl->setRenderer( new QgsSingleSymbolRenderer( new QgsMarkerSymbol() ) );
      QCOMPARE( dynamic_cast< QgsSingleSymbolRenderer * >( vl->renderer() )->symbol()->type(), Qgis::SymbolType::Marker );

      // no change to data source
      QgsMapCanvas canvas;
      QgsMessageBar messageBar;
      {
        QgsVectorLayerProperties dialog( &canvas, &messageBar, vl.get() );
        dialog.show();
        dialog.accept();
      }

      // renderer should still be a marker type
      QCOMPARE( dynamic_cast< QgsSingleSymbolRenderer * >( vl->renderer() )->symbol()->type(), Qgis::SymbolType::Marker );

      // change the data source to a line layer:
      {
        QgsVectorLayerProperties dialog( &canvas, &messageBar, vl.get() );
        DummySourceWidget *sourceWidget = new DummySourceWidget( &dialog );
        sourceWidget->newSource = mTestDataDir + "lines_touching.shp";
        dialog.mSourceWidget = sourceWidget;
        dialog.show();
        dialog.accept();
      }

      QCOMPARE( vl->source(), mTestDataDir + "lines_touching.shp" );
      QCOMPARE( vl->geometryType(), Qgis::GeometryType::Line );
      // single symbol renderer with marker symbol would be nonsense now, we expected a line symbol
      // ie the settings for the renderer which were present in the dialog MUST be ignored and overwritten
      // by the logic which triggers when the geometry type is changed via a data source change
      QCOMPARE( dynamic_cast< QgsSingleSymbolRenderer * >( vl->renderer() )->symbol()->type(), Qgis::SymbolType::Line );
    }

    void testValidRasterProperties()
    {
      // valid raster layer
      QTemporaryDir tmpDir;
      QFile::copy( mTestDataDir + "landsat_4326.tif", tmpDir.filePath( QStringLiteral( "landsat_4326.tif" ) ) );
      const QString rasterFileName = tmpDir.filePath( QStringLiteral( "landsat_4326.tif" ) );
      std::unique_ptr< QgsRasterLayer > rl = std::make_unique< QgsRasterLayer >( rasterFileName, QStringLiteral( "test" ), QStringLiteral( "gdal" ) );
      QVERIFY( rl->isValid() );

      QgsMapCanvas canvas;
      QgsMessageBar messageBar;
      QgsRasterLayerProperties dialog( rl.get(), &canvas );
      dialog.show();
      dialog.accept();
    }

    void testInvalidRasterProperties()
    {
      // invalid raster layer
      const QString rasterFileName = mTestDataDir + "xxlandsat_4326.tif";
      std::unique_ptr< QgsRasterLayer > rl = std::make_unique< QgsRasterLayer >( rasterFileName, QStringLiteral( "test" ), QStringLiteral( "xxgdal" ) );
      QVERIFY( !rl->isValid() );

      QgsMapCanvas canvas;
      QgsMessageBar messageBar;
      QgsRasterLayerProperties dialog( rl.get(), &canvas );
      dialog.show();
      dialog.accept();
    }

    void testValidMeshProperties()
    {
      // valid mesh layer
      QString uri( mTestDataDir + "/mesh/quad_and_triangle.2dm" );
      std::unique_ptr< QgsMeshLayer > ml = std::make_unique< QgsMeshLayer >( uri, QStringLiteral( "test" ), QStringLiteral( "mdal" ) );
      QVERIFY( ml->isValid() );

      QgsMapCanvas canvas;
      QgsMessageBar messageBar;
      QgsMeshLayerProperties dialog( ml.get(), &canvas );
      dialog.show();
      dialog.accept();
    }

    void testInvalidMeshProperties()
    {
      // invalid mesh layer
      QString uri( mTestDataDir + "/mesh/xxquad_and_triangle.2dm" );
      std::unique_ptr< QgsMeshLayer > ml = std::make_unique< QgsMeshLayer >( uri, QStringLiteral( "test" ), QStringLiteral( "xmdal" ) );
      QVERIFY( !ml->isValid() );

      QgsMapCanvas canvas;
      QgsMessageBar messageBar;
      QgsMeshLayerProperties dialog( ml.get(), &canvas );
      dialog.show();
      dialog.accept();
    }

    void testValidPointCloudProperties()
    {
      // valid point cloud layer
      std::unique_ptr< QgsPointCloudLayer > layer = std::make_unique< QgsPointCloudLayer >( mTestDataDir + QStringLiteral( "point_clouds/ept/sunshine-coast/ept.json" ), QStringLiteral( "layer" ), QStringLiteral( "ept" ) );
      QVERIFY( layer->isValid() );

      QgsMapCanvas canvas;
      QgsMessageBar messageBar;
      QgsPointCloudLayerProperties dialog( layer.get(), &canvas, &messageBar );
      dialog.show();
      dialog.accept();
    }

    void testInvalidPointCloudProperties()
    {
      // invalid point cloud layer
      std::unique_ptr< QgsPointCloudLayer > layer = std::make_unique< QgsPointCloudLayer >( mTestDataDir + QStringLiteral( "xxpoint_clouds/ept/sunshine-coast/ept.json" ), QStringLiteral( "layer" ), QStringLiteral( "xxept" ) );
      QVERIFY( !layer->isValid() );

      QgsMapCanvas canvas;
      QgsMessageBar messageBar;
      QgsPointCloudLayerProperties dialog( layer.get(), &canvas, &messageBar );
      dialog.show();
      dialog.accept();
    }

    void testValidVectorTileProperties()
    {
      // valid vector tile layer
      const QString srcMbtiles = QStringLiteral( "type=mbtiles&url=%1/vector_tile/mbtiles_vt.mbtiles" ).arg( TEST_DATA_DIR );
      std::unique_ptr< QgsVectorTileLayer > layer = std::make_unique< QgsVectorTileLayer >( srcMbtiles );
      QVERIFY( layer->isValid() );

      QgsMapCanvas canvas;
      QgsMessageBar messageBar;
      QgsVectorTileLayerProperties dialog( layer.get(), &canvas, &messageBar );
      dialog.show();
      dialog.accept();
    }

    void testInvalidVectorTileProperties()
    {
      // invalid vector tile layer
      const QString srcMbtiles = QStringLiteral( "type=mbtiles&url=%1/vector_tile/xxmbtiles_vt.mbtiles" ).arg( TEST_DATA_DIR );
      std::unique_ptr< QgsVectorTileLayer > layer = std::make_unique< QgsVectorTileLayer >( srcMbtiles );
      QVERIFY( !layer->isValid() );

      QgsMapCanvas canvas;
      QgsMessageBar messageBar;
      QgsVectorTileLayerProperties dialog( layer.get(), &canvas, &messageBar );
      dialog.show();
      dialog.accept();
    }

    void testInvalidTileSceneProperties()
    {
      // invalid tiled scene layer
      std::unique_ptr< QgsTiledSceneLayer > layer = std::make_unique< QgsTiledSceneLayer >( QStringLiteral( "xxx" ), QStringLiteral( "test" ), QStringLiteral( "xxx" ) );
      QVERIFY( !layer->isValid() );

      QgsMapCanvas canvas;
      QgsMessageBar messageBar;
      QgsTiledSceneLayerProperties dialog( layer.get(), &canvas, &messageBar );
      dialog.show();
      dialog.accept();
    }

    void testValidAnnotationLayerProperties()
    {
      // valid annotation layer
      std::unique_ptr< QgsAnnotationLayer > layer = std::make_unique< QgsAnnotationLayer >( QStringLiteral( "xxx" ), QgsAnnotationLayer::LayerOptions( QgsCoordinateTransformContext() ) );
      QVERIFY( layer->isValid() );

      QgsMapCanvas canvas;
      QgsMessageBar messageBar;
      QgsAnnotationLayerProperties dialog( layer.get(), &canvas, &messageBar );
      dialog.show();
      dialog.accept();
    }
};

QGSTEST_MAIN( TestQgsLayerPropertiesDialogs )
#include "testqgslayerpropertiesdialogs.moc"
