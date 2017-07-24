
#include <QApplication>
#include <QBoxLayout>

#include <Qt3DRender>
#include <Qt3DExtras>

#include "abstract3drenderer.h"
#include "maptexturegenerator.h"
#include "sidepanel.h"
#include "window3d.h"
#include "map3d.h"
#include "flatterraingenerator.h"
#include "demterraingenerator.h"
//#include "quantizedmeshterraingenerator.h"

#include <qgsapplication.h>
#include <qgsmapsettings.h>
#include <qgsreadwritecontext.h>
#include <qgsrasterlayer.h>
#include <qgsproject.h>
#include <qgsvectorlayer.h>



static QgsRectangle _fullExtent( const QList<QgsMapLayer *> &layers, const QgsCoordinateReferenceSystem &crs )
{
  QgsMapSettings ms;
  ms.setLayers( layers );
  ms.setDestinationCrs( crs );
  return ms.fullExtent();
}

int main( int argc, char *argv[] )
{
  // TODO: why it does not work to create ordinary QApplication and then just call initQgis()

  // FIXME: point this path to the directory where you have cloned the repo with test data: https://github.com/wonder-sk/qgis3d
  QString dataDir = "/home/martin/tmp/qgis3d";

  QgsApplication app( argc, argv, true );
  QgsApplication::init();
  QgsApplication::initQgis();

  QgsRasterLayer *rlDtm = new QgsRasterLayer( dataDir + "/dtm.tif", "dtm", "gdal" );
  Q_ASSERT( rlDtm->isValid() );

  QgsRasterLayer *rlSat = new QgsRasterLayer( dataDir + "/ap.tif", "ap", "gdal" );
  Q_ASSERT( rlSat->isValid() );

  QgsVectorLayer *vlPolygons = new QgsVectorLayer( dataDir + "/data/buildings.shp", "buildings", "ogr" );
  Q_ASSERT( vlPolygons->isValid() );

  QgsVectorLayer *vlPoints = new QgsVectorLayer( dataDir + "/data/trees.shp", "trees", "ogr" );
  Q_ASSERT( vlPoints->isValid() );

  QgsVectorLayer *vlLines = new QgsVectorLayer( dataDir + "/data/roads.shp", "roads", "ogr" );
  Q_ASSERT( vlLines->isValid() );

  QList<QgsMapLayer *> layers;
  layers << rlDtm << rlSat << vlPolygons << vlPoints << vlLines;

  QgsProject project;
  project.addMapLayers( layers );

  Map3D map;
  map.setLayers( QList<QgsMapLayer *>() << rlSat );
  map.crs = rlSat->crs();
  map.zExaggeration = 3;
  map.setShowTerrainBoundingBoxes( true );
  map.setShowTerrainTilesInfo( true );

  TerrainGenerator::Type tt;
  //tt = TerrainGenerator::Flat;
  tt = TerrainGenerator::Dem;
  //tt = TerrainGenerator::QuantizedMesh;

  if ( tt == TerrainGenerator::Flat )
  {
    // TODO: tiling scheme - from this project's CRS + full extent
    FlatTerrainGenerator *flatTerrain = new FlatTerrainGenerator;
    map.setTerrainGenerator( flatTerrain );
  }
  else if ( tt == TerrainGenerator::Dem )
  {
    DemTerrainGenerator *demTerrain = new DemTerrainGenerator;
    demTerrain->setLayer( rlDtm );
    map.setTerrainGenerator( demTerrain );
  }
  else if ( tt == TerrainGenerator::QuantizedMesh )
  {
#if 0
    QuantizedMeshTerrainGenerator *qmTerrain = new QuantizedMeshTerrainGenerator;
    map.terrainGenerator.reset( qmTerrain );
#endif
    Q_ASSERT( false );
  }

  Q_ASSERT( map.terrainGenerator() ); // we need a terrain generator

  if ( map.terrainGenerator()->type() == TerrainGenerator::Flat )
  {
    // we are free to define terrain extent to whatever works best
    FlatTerrainGenerator *flatGen = static_cast<FlatTerrainGenerator *>( map.terrainGenerator() );
    flatGen->setCrs( map.crs );
    flatGen->setExtent( _fullExtent( map.layers(), map.crs ) );
  }

  QgsRectangle fullExtentInTerrainCrs = _fullExtent( map.layers(), map.terrainGenerator()->crs() );

#if 0
  if ( map.terrainGenerator->type() == TerrainGenerator::QuantizedMesh )
  {
    // define base terrain tile coordinates
    static_cast<QuantizedMeshTerrainGenerator *>( map.terrainGenerator.get() )->setBaseTileFromExtent( fullExtentInTerrainCrs );
  }
#endif

  // origin X,Y - at the project extent's center
  QgsPointXY centerTerrainCrs = fullExtentInTerrainCrs.center();
  QgsPointXY centerMapCrs = QgsCoordinateTransform( map.terrainGenerator()->terrainTilingScheme.crs, map.crs ).transform( centerTerrainCrs );
  map.originX = centerMapCrs.x();
  map.originY = centerMapCrs.y();

  // polygons

  PolygonRenderer pr;
  pr.setLayer( vlPolygons );
  pr.material.setAmbient( Qt::gray );
  pr.material.setDiffuse( Qt::lightGray );
  pr.material.setShininess( 0 );
  pr.height = 0;
  pr.extrusionHeight = 10;
  map.renderers << new PolygonRenderer( pr );

  // points

  PointRenderer ptr;
  ptr.setLayer( vlPoints );
  ptr.material.setDiffuse( QColor( 222, 184, 135 ) );
  ptr.material.setAmbient( ptr.material.diffuse().darker() );
  ptr.material.setShininess( 0 );
  ptr.height = 2.5;
  ptr.shapeProperties["shape"] = "cylinder";
  ptr.shapeProperties["radius"] = 1;
  ptr.shapeProperties["length"] = 5;
  //Qt3DCore::QTransform tr;
  //tr.setScale3D(QVector3D(4,1,4));
  //ptr.transform = tr.matrix();
  map.renderers << new PointRenderer( ptr );

  PointRenderer ptr2;
  ptr2.setLayer( vlPoints );
  ptr2.material.setDiffuse( QColor( 60, 179, 113 ) );
  ptr2.material.setAmbient( ptr2.material.diffuse().darker() );
  ptr2.material.setShininess( 0 );
  ptr2.height = 7.5;
  ptr2.shapeProperties["shape"] = "sphere";
  ptr2.shapeProperties["radius"] = 3.5;
  map.renderers << new PointRenderer( ptr2 );

#if 0
  // Q on top of trees - only in Qt 5.9
  PointRenderer ptr3;
  ptr3.setLayer( vlPoints );
  ptr3.material.setDiffuse( QColor( 88, 150, 50 ) );
  ptr3.material.setAmbient( ptr3.material.diffuse().darker() );
  ptr3.height = 25;
  ptr3.shapeProperties["shape"] = "extrudedText";
  ptr3.shapeProperties["text"] = "Q";
  Qt3DCore::QTransform tr;
  tr.setScale3D( QVector3D( 3, 3, 3 ) );
  tr.setTranslation( QVector3D( -4, 0, 0 ) );
  ptr3.transform = tr.matrix();
  map.pointRenderers << ptr3;
#endif

  // lines

  LineRenderer lr;
  lr.setLayer( vlLines );
  lr.material.setAmbient( Qt::yellow );
  lr.material.setShininess( 0 );
  lr.altBinding = AltBindVertex;  // follow terrain
  lr.height = 1.5;
  lr.distance = 2.5;
  map.renderers << new LineRenderer( lr );

  // skybox

  map.skybox = true;
  map.skyboxFileBase = "file://" + dataDir + "/skybox/miramar";
  map.skyboxFileExtension = ".jpg";

  //
  // map read/write test
  //

  QObject::connect( &project, &QgsProject::writeProject, [&map]( QDomDocument & doc )
  {
    QDomElement elem = map.writeXml( doc, QgsReadWriteContext() );
    doc.documentElement().appendChild( elem );
  } );
  project.write( "/tmp/3d.qgs" );

  QgsProject project2;
  Map3D map2;
  QObject::connect( &project2, &QgsProject::readProject, [&map2]( const QDomDocument & doc )
  {
    QDomElement elem = doc.documentElement().firstChildElement( "qgis3d" );
    map2.readXml( elem, QgsReadWriteContext() );
  } );
  project2.read( "/tmp/3d.qgs" );
  map2.resolveReferences( project2 );
  map2.terrainGenerator()->terrainTilingScheme = map.terrainGenerator()->terrainTilingScheme;

  //
  // GUI initialization
  //

  SidePanel *sidePanel = new SidePanel;
  sidePanel->setMinimumWidth( 150 );

  Window3D *view = new Window3D( sidePanel, map2 );
  QWidget *container = QWidget::createWindowContainer( view );

  QSize screenSize = view->screen()->size();
  container->setMinimumSize( QSize( 200, 100 ) );
  container->setMaximumSize( screenSize );

  QWidget widget;
  QHBoxLayout *hLayout = new QHBoxLayout( &widget );
  hLayout->setMargin( 0 );
  hLayout->addWidget( container, 1 );
  hLayout->addWidget( sidePanel );

  widget.resize( 800, 600 );
  widget.show();

  return app.exec();
}
