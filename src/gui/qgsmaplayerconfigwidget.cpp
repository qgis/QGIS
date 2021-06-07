/***************************************************************************
    qgsmaplayerconfigwidget.cpp
    ---------------------------
    begin                : June 2016
    copyright            : (C) 2016 by Nathan Woodrow
    email                : woodrow dot nathan at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   *
 *  
 *        *
 *                                     *
 *                                                                         *
 ***************************************************************************/
#include "qgsmaplayerconfigwidget.h"
#include "qgspanelwidget.h"

QgsMapLayerConfigWidget::QgsMapLayerConfigWidget( QgsMapLayer *layer, QgsMapCanvas *canvas, QWidget *parent )
  : QgsPanelWidget( parent )
  , mLayer( layer )
  , mMapCanvas( canvas )
{

}
