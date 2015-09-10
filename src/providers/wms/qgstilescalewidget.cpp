/***************************************************************************
    qgstilescalewidget.cpp  - slider to choose wms-c resolutions
                             -------------------
    begin    : 28 Mar 2010
    copyright: (C) 2010 Juergen E. Fischer < jef at norbit dot de >

 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgstilescalewidget.h"
#include "qgsmapcanvas.h"
#include "qgsrasterlayer.h"
#include "qgsmessagelog.h"
#include "qgslogger.h"

#include <QDockWidget>
#include <QMainWindow>
#include <QMenu>
#include <QGraphicsView>

QgsTileScaleWidget::QgsTileScaleWidget( QgsMapCanvas * mapCanvas, QWidget * parent, Qt::WindowFlags f )
    : QWidget( parent, f )
    , mMapCanvas( mapCanvas )
{
  setupUi( this );

  connect( mMapCanvas, SIGNAL( scaleChanged( double ) ), this, SLOT( scaleChanged( double ) ) );

  layerChanged( mMapCanvas->currentLayer() );
}

QgsTileScaleWidget::~QgsTileScaleWidget()
{
}

void QgsTileScaleWidget::layerChanged( QgsMapLayer *layer )
{
  mSlider->setDisabled( true );

  QgsRasterLayer *rl = qobject_cast<QgsRasterLayer *>( layer );
  if ( !rl || rl->providerType() != "wms" || !rl->dataProvider() )
    return;

  QVariant res = rl->dataProvider()->property( "resolutions" );

  mResolutions.clear();
  Q_FOREACH ( const QVariant& r, res.toList() )
  {
    QgsDebugMsg( QString( "found resolution: %1" ).arg( r.toDouble() ) );
    mResolutions << r.toDouble();
  }

  if ( mResolutions.size() == 0 )
    return;

  mSlider->setRange( 0, mResolutions.size() - 1 );
  mSlider->setTickInterval( 1 );
  mSlider->setInvertedAppearance( true );
  mSlider->setPageStep( 1 );
  mSlider->setTracking( false );

  scaleChanged( mMapCanvas->scale() );

  mSlider->setEnabled( true );
  show();
}

void QgsTileScaleWidget::scaleChanged( double scale )
{
  Q_UNUSED( scale );

  if ( mResolutions.size() == 0 )
    return;

  double mupp = mMapCanvas->mapUnitsPerPixel();
  QgsDebugMsg( QString( "resolution changed to %1" ).arg( mupp ) );

  int i;
  for ( i = 0; i < mResolutions.size() && mResolutions[i] < mupp; i++ )
    QgsDebugMsg( QString( "test resolution %1: %2 d:%3" ).arg( i ).arg( mResolutions[i] ).arg( mupp - mResolutions[i] ) );

  if ( i == mResolutions.size() ||
       ( i > 0 && mResolutions[i] - mupp > mupp - mResolutions[i-1] ) )
  {
    QgsDebugMsg( "previous resolution" );
    i--;
  }

  QgsDebugMsg( QString( "selected resolution %1: %2" ).arg( i ).arg( mResolutions[i] ) );
  mSlider->blockSignals( true );
  mSlider->setValue( i );
  mSlider->blockSignals( false );
}

void QgsTileScaleWidget::on_mSlider_valueChanged( int value )
{
  Q_UNUSED( value );
  QgsDebugMsg( QString( "slider released at %1: %2" ).arg( mSlider->value() ).arg( mResolutions[mSlider->value()] ) );
  mMapCanvas->zoomByFactor( mResolutions[mSlider->value()] / mMapCanvas->mapUnitsPerPixel() );
}

void QgsTileScaleWidget::showTileScale( QMainWindow *mainWindow )
{
  QDockWidget *dock = mainWindow->findChild<QDockWidget *>( "theTileScaleDock" );
  if ( dock )
  {
    dock->setVisible( dock->isHidden() );
    return;
  }

  QgsMapCanvas *canvas = mainWindow->findChild<QgsMapCanvas *>( "theMapCanvas" );
  QgsDebugMsg( QString( "canvas:%1 [%2]" ).arg(( ulong ) canvas, 0, 16 ).arg( canvas ? canvas->objectName() : "" ) );
  if ( !canvas )
  {
    QgsDebugMsg( "map canvas theMapCanvas not found" );
    return;
  }

  QgsTileScaleWidget *tws = new QgsTileScaleWidget( canvas );
  tws->setObjectName( "theTileScaleWidget" );

  QObject *legend = mainWindow->findChild<QObject*>( "theLayerTreeView" );
  if ( legend )
  {
    connect( legend, SIGNAL( currentLayerChanged( QgsMapLayer* ) ),
             tws, SLOT( layerChanged( QgsMapLayer* ) ) );
  }
  else
  {
    QgsDebugMsg( "legend not found" );
  }

  //create the dock widget
  dock = new QDockWidget( tr( "Tile Scale Panel" ), mainWindow );
  dock->setObjectName( "theTileScaleDock" );
  dock->setAllowedAreas( Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea );
  mainWindow->addDockWidget( Qt::RightDockWidgetArea, dock );

  // add to the Panel submenu
  QMenu *panelMenu = mainWindow->findChild<QMenu *>( "mPanelMenu" );
  if ( panelMenu )
  {
    // add to the Panel submenu
    panelMenu->addAction( dock->toggleViewAction() );
  }
  else
  {
    QgsDebugMsg( "panel menu not found" );
  }

  dock->setWidget( tws );

  connect( dock, SIGNAL( visibilityChanged( bool ) ), tws, SLOT( scaleEnabled( bool ) ) );

  QSettings settings;
  dock->setVisible( settings.value( "/UI/tileScaleEnabled", false ).toBool() );
}

void QgsTileScaleWidget::scaleEnabled( bool enabled )
{
  QSettings settings;
  settings.setValue( "/UI/tileScaleEnabled", enabled );
}
