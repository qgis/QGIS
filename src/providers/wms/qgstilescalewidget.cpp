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
#include "qgsrasterdataprovider.h"
#include "qgsmessagelog.h"
#include "qgslogger.h"
#include "qgsdockwidget.h"
#include "qgssettings.h"
#include "layertree/qgslayertreeview.h"

#include <QMainWindow>
#include <QMenu>
#include <QGraphicsView>
#include <QToolTip>

QgsTileScaleWidget::QgsTileScaleWidget( QgsMapCanvas *mapCanvas, QWidget *parent, Qt::WindowFlags f )
  : QWidget( parent, f )
  , mMapCanvas( mapCanvas )
{
  setupUi( this );

  connect( mSlider, &QSlider::valueChanged, this, &QgsTileScaleWidget::mSlider_valueChanged );
  connect( mMapCanvas, &QgsMapCanvas::scaleChanged, this, &QgsTileScaleWidget::scaleChanged );

  layerChanged( mMapCanvas->currentLayer() );
}

void QgsTileScaleWidget::layerChanged( QgsMapLayer *layer )
{
  mSlider->setDisabled( true );

  QgsRasterLayer *rl = qobject_cast<QgsRasterLayer *>( layer );
  if ( !rl || rl->providerType() != QLatin1String( "wms" ) || !rl->dataProvider() )
    return;

  QVariant res = rl->dataProvider()->property( "resolutions" );

  mResolutions.clear();
  Q_FOREACH ( const QVariant &r, res.toList() )
  {
    QgsDebugMsg( QString( "found resolution: %1" ).arg( r.toDouble() ) );
    mResolutions << r.toDouble();
  }

  if ( mResolutions.isEmpty() )
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

  if ( mResolutions.isEmpty() )
    return;

  double mupp = mMapCanvas->mapUnitsPerPixel();
  QgsDebugMsg( QString( "resolution changed to %1" ).arg( mupp ) );

  int i;
  for ( i = 0; i < mResolutions.size() && mResolutions.at( i ) < mupp; i++ )
    QgsDebugMsg( QString( "test resolution %1: %2 d:%3" ).arg( i ).arg( mResolutions.at( i ) ).arg( mupp - mResolutions.at( i ) ) );

  if ( i == mResolutions.size() ||
       ( i > 0 && mResolutions.at( i ) - mupp > mupp - mResolutions.at( i - 1 ) ) )
  {
    QgsDebugMsg( "previous resolution" );
    i--;
  }

  QgsDebugMsg( QString( "selected resolution %1: %2" ).arg( i ).arg( mResolutions.at( i ) ) );
  mSlider->blockSignals( true );
  mSlider->setValue( i );
  mSlider->blockSignals( false );
}

void QgsTileScaleWidget::mSlider_valueChanged( int value )
{
  QgsDebugMsg( QString( "slider released at %1: %2" ).arg( mSlider->value() ).arg( mResolutions.at( mSlider->value() ) ) );

  // Invert value in tooltip to match expectation (i.e. 0 = zoomed out, maximum = zoomed in)
  QToolTip::showText( QCursor::pos(), tr( "Zoom level: %1" ).arg( mSlider->maximum() - value ) + "\n" + tr( "Resolution: %1" ).arg( mResolutions.at( value ) ), this );
  mMapCanvas->zoomByFactor( mResolutions.at( mSlider->value() ) / mMapCanvas->mapUnitsPerPixel() );
}

void QgsTileScaleWidget::locationChanged( Qt::DockWidgetArea area )
{
  mSlider->setOrientation( area == Qt::TopDockWidgetArea || area == Qt::BottomDockWidgetArea ? Qt::Horizontal : Qt::Vertical );
}

void QgsTileScaleWidget::showTileScale( QMainWindow *mainWindow )
{
  QgsDockWidget *dock = mainWindow->findChild<QgsDockWidget *>( QStringLiteral( "theTileScaleDock" ) );
  if ( dock )
  {
    dock->setVisible( dock->isHidden() );
    return;
  }

  QgsMapCanvas *canvas = mainWindow->findChild<QgsMapCanvas *>( QStringLiteral( "theMapCanvas" ) );
  QgsDebugMsg( QString( "canvas:%1 [%2]" ).arg( ( quint64 ) canvas, 0, 16 ).arg( canvas ? canvas->objectName() : "" ) );
  if ( !canvas )
  {
    QgsDebugMsg( "map canvas mapCanvas not found" );
    return;
  }

  QgsTileScaleWidget *tws = new QgsTileScaleWidget( canvas );
  tws->setObjectName( QStringLiteral( "theTileScaleWidget" ) );

  QgsLayerTreeView *legend = mainWindow->findChild<QgsLayerTreeView *>( QStringLiteral( "theLayerTreeView" ) );
  if ( legend )
  {
    connect( legend, &QgsLayerTreeView::currentLayerChanged,
             tws, &QgsTileScaleWidget::layerChanged );
  }
  else
  {
    QgsDebugMsg( "legend not found" );
  }

  //create the dock widget
  dock = new QgsDockWidget( tr( "Tile Scale" ), mainWindow );
  dock->setObjectName( QStringLiteral( "theTileScaleDock" ) );

  connect( dock, &QDockWidget::dockLocationChanged, tws, &QgsTileScaleWidget::locationChanged );

  mainWindow->addDockWidget( Qt::RightDockWidgetArea, dock );

  // add to the Panel submenu
  QMenu *panelMenu = mainWindow->findChild<QMenu *>( QStringLiteral( "mPanelMenu" ) );
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

  connect( dock, &QDockWidget::visibilityChanged, tws, &QgsTileScaleWidget::scaleEnabled );

  QgsSettings settings;
  dock->setVisible( settings.value( QStringLiteral( "UI/tileScaleEnabled" ), false ).toBool() );
}

void QgsTileScaleWidget::scaleEnabled( bool enabled )
{
  QgsSettings settings;
  settings.setValue( QStringLiteral( "UI/tileScaleEnabled" ), enabled );
}
