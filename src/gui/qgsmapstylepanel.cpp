#include "qgsmapstylepanel.h"

QgsMapStylePanel::QgsMapStylePanel( QgsMapLayer *layer, QgsMapCanvas *canvas, QWidget *parent )
    : QWidget( parent )
{
  Q_UNUSED( layer );
  Q_UNUSED( canvas );
}

QgsMapStylePanelFactory::QgsMapStylePanelFactory()
{

}

QgsMapStylePanelFactory::~QgsMapStylePanelFactory()
{

}
