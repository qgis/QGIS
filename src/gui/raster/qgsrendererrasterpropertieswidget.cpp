#include "qgsrendererrasterpropertieswidget.h"

#include "qgis.h"
#include "qgsmapcanvas.h"
#include "qgsrasterlayer.h"
#include "qgsrasterrendererwidget.h"
#include "qgsrasterrendererregistry.h"
#include "qgssinglebandgrayrendererwidget.h"
#include "qgssinglebandpseudocolorrendererwidget.h"
#include "qgsmultibandcolorrendererwidget.h"
#include "qgspalettedrendererwidget.h"


static void _initRendererWidgetFunctions()
{
  static bool initialized = false;
  if ( initialized )
    return;

  QgsRasterRendererRegistry::instance()->insertWidgetFunction( "paletted", QgsPalettedRendererWidget::create );
  QgsRasterRendererRegistry::instance()->insertWidgetFunction( "multibandcolor", QgsMultiBandColorRendererWidget::create );
  QgsRasterRendererRegistry::instance()->insertWidgetFunction( "singlebandpseudocolor", QgsSingleBandPseudoColorRendererWidget::create );
  QgsRasterRendererRegistry::instance()->insertWidgetFunction( "singlebandgray", QgsSingleBandGrayRendererWidget::create );

  initialized = true;
}



QgsRendererRasterPropertiesWidget::QgsRendererRasterPropertiesWidget( QgsRasterLayer *layer, QgsMapCanvas* canvas, QObject *parent )
    : mRasterLayer( layer )
    , mMapCanvas( canvas )
    , mRendererWidget( nullptr )
{
  setupUi( this );

  _initRendererWidgetFunctions();

  QgsRasterRendererRegistryEntry entry;
  Q_FOREACH ( const QString& name, QgsRasterRendererRegistry::instance()->renderersList() )
  {
    if ( QgsRasterRendererRegistry::instance()->rendererData( name, entry ) )
    {
      if (( mRasterLayer->rasterType() != QgsRasterLayer::ColorLayer && entry.name != "singlebandcolordata" ) ||
          ( mRasterLayer->rasterType() == QgsRasterLayer::ColorLayer && entry.name == "singlebandcolordata" ) )
      {
        cboRenderers->addItem( entry.visibleName, entry.name );
      }
    }
  }
  cboRenderers->setCurrentIndex( -1 );

  connect( cboRenderers, SIGNAL( currentIndexChanged( int ) ), this, SLOT( rendererChanged() ) );

  QgsRasterRenderer* renderer = mRasterLayer->renderer();
  if ( renderer )
  {
    setRendererWidget( renderer->type() );
  }
}

QgsRendererRasterPropertiesWidget::~QgsRendererRasterPropertiesWidget()
{

}

void QgsRendererRasterPropertiesWidget::setMapCanvas( QgsMapCanvas *canvas )
{
  mMapCanvas = canvas;
}

void QgsRendererRasterPropertiesWidget::rendererChanged()
{
  QString rendererName = cboRenderers->itemData( cboRenderers->currentIndex() ).toString();
  setRendererWidget( rendererName );
  emit widgetChanged();
}

void QgsRendererRasterPropertiesWidget::apply()
{
  QgsRasterRendererWidget* rendererWidget = dynamic_cast<QgsRasterRendererWidget*>( stackedWidget->currentWidget() );
  if ( rendererWidget )
  {
    mRasterLayer->setRenderer( rendererWidget->renderer() );
  }
}

void QgsRendererRasterPropertiesWidget::setRendererWidget( const QString &rendererName )
{
  QgsDebugMsg( "rendererName = " + rendererName );
  QgsRasterRendererWidget* oldWidget = mRendererWidget;

  QgsRasterRendererRegistryEntry rendererEntry;
  if ( QgsRasterRendererRegistry::instance()->rendererData( rendererName, rendererEntry ) )
  {
    if ( rendererEntry.widgetCreateFunction ) //single band color data renderer e.g. has no widget
    {
      QgsDebugMsg( "renderer has widgetCreateFunction" );
      // Current canvas extent (used to calc min/max) in layer CRS
      QgsRectangle myExtent = mMapCanvas->mapSettings().outputExtentToLayerExtent( mRasterLayer, mMapCanvas->extent() );
      mRendererWidget = rendererEntry.widgetCreateFunction( mRasterLayer, myExtent );
      connect( mRendererWidget, SIGNAL( widgetChanged() ), this, SIGNAL( widgetChanged() ) );
      int page = stackedWidget->addWidget( mRendererWidget );
      stackedWidget->setCurrentWidget( mRendererWidget );
      if ( oldWidget )
      {
        //compare used bands in new and old renderer and reset transparency dialog if different
        QgsRasterRenderer* oldRenderer = oldWidget->renderer();
        QgsRasterRenderer* newRenderer = mRendererWidget->renderer();
        QList<int> oldBands = oldRenderer->usesBands();
        QList<int> newBands = newRenderer->usesBands();
//        if ( oldBands != newBands )
//        {
//          populateTransparencyTable( newRenderer );
//        }
        delete oldRenderer;
        delete newRenderer;
      }
    }
  }

  if ( mRendererWidget != oldWidget )
    delete oldWidget;

  int widgetIndex = cboRenderers->findData( rendererName );
  if ( widgetIndex != -1 )
  {
    whileBlocking( cboRenderers )->setCurrentIndex( widgetIndex );
  }

}
