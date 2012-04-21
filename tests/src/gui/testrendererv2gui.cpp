#include "testrendererv2gui.h"

#include <qgsapplication.h>
#include <qgsmapcanvas.h>
#include <qgsvectorlayer.h>
#include <qgsmaplayerregistry.h>
#include <qgsproject.h>
#include <qgsrendererv2propertiesdialog.h>
#include <qgsstylev2.h>

#include <QApplication>
#include <QToolBar>

TestRendererV2GUI::TestRendererV2GUI( QWidget *parent ) :
    QMainWindow( parent )
{
  resize( 640, 480 );

  QToolBar* toolBar = addToolBar( "Actions" );
  toolBar->addAction( "set renderer", this, SLOT( setRenderer() ) );

  mMapCanvas = new QgsMapCanvas( this );
  mMapCanvas->setCanvasColor( Qt::white );
  setCentralWidget( mMapCanvas );

  connect( QgsProject::instance(), SIGNAL( readProject( QDomDocument ) ), mMapCanvas, SLOT( readProject( QDomDocument ) ) );
}

void TestRendererV2GUI::loadLayers()
{
  // load just first vector layer
  QList<QgsMapCanvasLayer> canvasLayers;
  foreach( QgsMapLayer* layer, QgsMapLayerRegistry::instance()->mapLayers().values() )
  {
    if ( layer->type() == QgsMapLayer::VectorLayer )
      canvasLayers << QgsMapCanvasLayer( layer );
  }

  mMapCanvas->setLayerSet( canvasLayers );
}

void TestRendererV2GUI::setRenderer()
{
  QgsMapLayer* layer = mMapCanvas->layer( 0 );
  Q_ASSERT( layer );
  Q_ASSERT( layer->type() == QgsMapLayer::VectorLayer );
  QgsVectorLayer* vlayer = static_cast<QgsVectorLayer*>( layer );

  QgsRendererV2PropertiesDialog dlg( vlayer, QgsStyleV2::defaultStyle() );
  dlg.exec();

  mMapCanvas->refresh();
}

int main( int argc, char* argv[] )
{
  QApplication app( argc, argv );

  if ( argc < 2 )
  {
    qDebug( "Provide a project file name with at least one vector layer!" );
    return 1;
  }

  QgsApplication::init();
  QgsApplication::initQgis();

  TestRendererV2GUI gui;

  QString projectFileName( argv[1] );
  QgsProject::instance()->setFileName( projectFileName );
  bool res = QgsProject::instance()->read();
  if ( !res )
  {
    qDebug( "Failed to open project!" );
    return 1;
  }

  // the layers are in the registry - now load them!
  gui.loadLayers();

  gui.show();
  return app.exec();
}
