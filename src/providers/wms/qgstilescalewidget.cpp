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

#include "layertree/qgslayertreeview.h"
#include "qgsdockwidget.h"
#include "qgsgui.h"
#include "qgslogger.h"
#include "qgsmapcanvas.h"
#include "qgsmessagelog.h"
#include "qgsrasterdataprovider.h"
#include "qgsrasterlayer.h"
#include "qgssettings.h"

#include <QGraphicsView>
#include <QMainWindow>
#include <QMenu>
#include <QToolTip>

#include "moc_qgstilescalewidget.cpp"

QgsTileScaleWidget::QgsTileScaleWidget( QgsMapCanvas *mapCanvas, QWidget *parent, Qt::WindowFlags f )
  : QWidget( parent, f )
  , mMapCanvas( mapCanvas )
{
  setupUi( this );
  QgsGui::enableAutoGeometryRestore( this );

  connect( mSlider, &QSlider::valueChanged, this, &QgsTileScaleWidget::mSlider_valueChanged );
  connect( mMapCanvas, &QgsMapCanvas::scaleChanged, this, &QgsTileScaleWidget::scaleChanged );

  layerChanged( mMapCanvas->currentLayer() );
}

void QgsTileScaleWidget::layerChanged( QgsMapLayer *layer )
{
  mSlider->setDisabled( true );

  QgsRasterLayer *rl = qobject_cast<QgsRasterLayer *>( layer );
  if ( !rl || !rl->dataProvider() )
    return;

  const QList<double> resolutions = rl->dataProvider()->nativeResolutions();
  if ( resolutions.isEmpty() )
    return;

  mResolutions.clear();
  for ( const double res : resolutions )
  {
    QgsDebugMsgLevel( u"found resolution: %1"_s.arg( res ), 2 );
    mResolutions << res;
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
  Q_UNUSED( scale )

  if ( mResolutions.isEmpty() )
    return;

  const double mupp = mMapCanvas->mapUnitsPerPixel();
  QgsDebugMsgLevel( u"resolution changed to %1"_s.arg( mupp ), 2 );

  int i;
  for ( i = 0; i < mResolutions.size() && mResolutions.at( i ) < mupp; i++ )
  {
    QgsDebugMsgLevel( u"test resolution %1: %2 d:%3"_s.arg( i ).arg( mResolutions.at( i ) ).arg( mupp - mResolutions.at( i ) ), 2 );
  }

  if ( i == mResolutions.size() || ( i > 0 && mResolutions.at( i ) - mupp > mupp - mResolutions.at( i - 1 ) ) )
  {
    QgsDebugMsgLevel( u"previous resolution"_s, 2 );
    i--;
  }

  QgsDebugMsgLevel( u"selected resolution %1: %2"_s.arg( i ).arg( mResolutions.at( i ) ), 2 );
  mSlider->blockSignals( true );
  mSlider->setValue( i );
  mSlider->blockSignals( false );


  mLabelScale->setText( tr( "Zoom: %1" ).arg( i ) );
}

void QgsTileScaleWidget::mSlider_valueChanged( int value )
{
  QgsDebugMsgLevel( u"slider released at %1: %2"_s.arg( mSlider->value() ).arg( mResolutions.at( mSlider->value() ) ), 2 );

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
  QgsDockWidget *dock = mainWindow->findChild<QgsDockWidget *>( u"theTileScaleDock"_s );
  if ( dock )
  {
    dock->setVisible( dock->isHidden() );
    return;
  }

  QgsMapCanvas *canvas = mainWindow->findChild<QgsMapCanvas *>( u"theMapCanvas"_s );
  QgsDebugMsgLevel( u"canvas:%1 [%2]"_s.arg( ( quint64 ) canvas, 0, 16 ).arg( canvas ? canvas->objectName() : QString() ), 4 );
  if ( !canvas )
  {
    QgsDebugError( u"map canvas mapCanvas not found"_s );
    return;
  }

  QgsTileScaleWidget *tws = new QgsTileScaleWidget( canvas );
  tws->setObjectName( u"theTileScaleWidget"_s );

  QgsLayerTreeView *legend = mainWindow->findChild<QgsLayerTreeView *>( u"theLayerTreeView"_s );
  if ( legend )
  {
    connect( legend, &QgsLayerTreeView::currentLayerChanged, tws, &QgsTileScaleWidget::layerChanged );
  }
  else
  {
    QgsDebugError( u"legend not found"_s );
  }

  //create the dock widget
  dock = new QgsDockWidget( tr( "Tile Scale" ), mainWindow );
  dock->setObjectName( u"theTileScaleDock"_s );

  connect( dock, &QDockWidget::dockLocationChanged, tws, &QgsTileScaleWidget::locationChanged );

  mainWindow->addDockWidget( Qt::RightDockWidgetArea, dock );

  // add to the Panel submenu
  QMenu *panelMenu = mainWindow->findChild<QMenu *>( u"mPanelMenu"_s );
  if ( panelMenu )
  {
    // add to the Panel submenu
    panelMenu->addAction( dock->toggleViewAction() );
  }
  else
  {
    QgsDebugError( u"panel menu not found"_s );
  }

  dock->setWidget( tws );

  connect( dock, &QDockWidget::visibilityChanged, tws, &QgsTileScaleWidget::scaleEnabled );

  const QgsSettings settings;
  dock->setVisible( settings.value( u"UI/tileScaleEnabled"_s, false ).toBool() );
}

void QgsTileScaleWidget::scaleEnabled( bool enabled )
{
  QgsSettings settings;
  settings.setValue( u"UI/tileScaleEnabled"_s, enabled );
}
