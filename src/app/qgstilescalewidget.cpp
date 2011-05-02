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
/* $Id$ */

#include "qgstilescalewidget.h"
#include "qgsmapcanvas.h"
#include "qgsrasterlayer.h"
#include "qgslogger.h"

QgsTileScaleWidget::QgsTileScaleWidget( QgsMapCanvas * mapCanvas, QWidget * parent, Qt::WindowFlags f )
    : QWidget( parent, f )
    , mMapCanvas( mapCanvas )
{
  setupUi( this );

  connect( mMapCanvas, SIGNAL( scaleChanged( double ) ), this, SLOT( scaleChanged( double ) ) );

  layerChanged( mMapCanvas->currentLayer() );
}

void QgsTileScaleWidget::layerChanged( QgsMapLayer *layer )
{
  QgsRasterLayer *rl = qobject_cast<QgsRasterLayer *>( layer );

  mResolutions.clear();
  mSlider->setDisabled( true );

  if ( !rl || rl->providerKey() != "wms" )
    return;

  QString uri = rl->source();
  int tiledpos = uri.indexOf( "tiled=" );
  int urlpos = uri.indexOf( "url=" );

  if ( tiledpos >= 0 && urlpos >= 0 && urlpos > tiledpos )
  {
    uri = uri.mid( tiledpos + 6 );
    int pos = uri.indexOf( "," );
    if ( pos >= 0 )
      uri = uri.left( pos );
    QStringList params = uri.split( ";" );
    if ( params.size() < 3 )
      return;

    params.takeFirst();
    params.takeFirst();

    mResolutions.clear();
    foreach( QString r, params )
    {
      mResolutions << r.toDouble();
    }
    qSort( mResolutions );

    for ( int i = 0; i < mResolutions.size(); i++ )
      QgsDebugMsg( QString( "found resolution %1: %2" ).arg( i ).arg( mResolutions[i] ) );

    mSlider->setRange( 0, mResolutions.size() - 1 );
    mSlider->setTickInterval( 1 );
    mSlider->setInvertedAppearance( true );
    mSlider->setPageStep( 1 );
    mSlider->setTracking( false );

    scaleChanged( mMapCanvas->scale() );

    mSlider->setEnabled( true );
    show();
  }
}

void QgsTileScaleWidget::scaleChanged( double scale )
{
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
  mSlider->setValue( i );
}

void QgsTileScaleWidget::on_mSlider_valueChanged( int value )
{
  QgsDebugMsg( QString( "slider released at %1: %2" ).arg( mSlider->value() ).arg( mResolutions[mSlider->value()] ) );
  mMapCanvas->zoomByFactor( mResolutions[mSlider->value()] / mMapCanvas->mapUnitsPerPixel() );
}
