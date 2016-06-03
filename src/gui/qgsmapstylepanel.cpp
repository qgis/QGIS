#include "qgsmapstylepanel.h"

QgsMapStylePanel::QgsMapStylePanel( QgsMapLayer *layer, QgsMapCanvas *canvas, QWidget *parent )
    : QWidget( parent )
    , mLayer( layer )
    , mMapCanvas( canvas )
{

}

QgsMapStylePanelFactory::QgsMapStylePanelFactory()
{

}

QgsMapStylePanelFactory::~QgsMapStylePanelFactory()
{

}
