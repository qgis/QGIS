
#include "maprenderertest.h"

#include <QApplication>
#include <QPainter>
#include <QPaintEvent>

#include "qgsapplication.h"
#include "qgsvectorlayer.h"
#include "qgsmaplayerregistry.h"

#include "qgsmapcanvas.h"
#include "qgsmapoverviewcanvas.h"
#include "qgsmaptoolpan.h"

int main( int argc, char* argv[] )
{
  QApplication app( argc, argv );

  QgsApplication::setPrefixPath( "/home/martin/qgis/git-master/creator/output", true );
  QgsApplication::initQgis();

  QString uri = "dbname='/data/gis/praha.osm.db' table=\"praha_polygons\" (geometry) sql=";
  QString uri2 = "dbname='/data/gis/praha.osm.db' table=\"praha_polylines\" (geometry) sql=";
  //QString uri = "/data/gis/cr-shp-wgs84/plochy/kraje_pseudo.shp";
  QgsVectorLayer* layer = new QgsVectorLayer( uri, "praha", "spatialite" );
  if ( !layer->isValid() )
  {
    qDebug( "invalid layer" );
    return 1;
  }
  QgsMapLayerRegistry::instance()->addMapLayer( layer );

  QgsVectorLayer* layer2 = new QgsVectorLayer( uri2, "praha", "spatialite" );
  if ( !layer2->isValid() )
  {
    qDebug( "invalid layer" );
    return 1;
  }
  QgsMapLayerRegistry::instance()->addMapLayer( layer2 );

  // open a window and do the rendering!
  /*TestWidget l(layer);
  l.resize(360,360);
  l.show();*/


  QgsMapCanvas canvas;
  canvas.setCanvasColor( Qt::white );
  canvas.setExtent( layer->extent() );
  canvas.show();

  // test overview
  QgsMapOverviewCanvas overview( 0, &canvas );
  overview.resize( 200, 200 );
  canvas.enableOverviewMode( &overview );
  overview.show();

  QList<QgsMapCanvasLayer> layers;
  layers.append( QgsMapCanvasLayer( layer2, true, true ) );
  layers.append( QgsMapCanvasLayer( layer ) );
  canvas.setLayerSet( layers );

  QgsMapTool* pan = new QgsMapToolPan( &canvas );
  canvas.setMapTool( pan );

  return app.exec();
}
