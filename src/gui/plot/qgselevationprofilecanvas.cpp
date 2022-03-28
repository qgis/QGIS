/***************************************************************************
                          qgselevationprofilecanvas.cpp
                          -----------------
    begin                : March 2022
    copyright            : (C) 2022 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
***************************************************************************/


/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgselevationprofilecanvas.h"
#include "qgsmaplayerlistutils_p.h"
#include "qgsplotcanvasitem.h"
#include "qgsprofilerequest.h"
#include "qgsabstractprofilesource.h"
#include "qgscurve.h"
#include "qgsprojectelevationproperties.h"
#include "qgsterrainprovider.h"
#include "qgsabstractprofilegenerator.h"
#include "qgsapplication.h"
#include "qgscolorschemeregistry.h"

class QgsElevationProfilePlotItem : public QgsPlotCanvasItem
{
  public:

    QgsElevationProfilePlotItem( QgsElevationProfileCanvas *canvas )
      : QgsPlotCanvasItem( canvas )
    {}

    void setContent( const QImage &image )
    {
      mImage = image;
      mRect = mCanvas->rect();

      prepareGeometryChange();
      setPos( mRect.topLeft() );

      update();
    }

    QRectF boundingRect() const override
    {
      return mRect;
    }

    void paint( QPainter *painter ) override
    {
      if ( mImage.isNull() )
        return;

      const int w = std::round( mRect.width() ) - 2;
      const int h = std::round( mRect.height() ) - 2;

      bool scale = false;
      if ( mImage.size() != QSize( w, h ) * mImage.devicePixelRatioF() )
      {
        QgsDebugMsgLevel( QStringLiteral( "map paint DIFFERENT SIZE: img %1,%2  item %3,%4" )
                          .arg( mImage.width() / mImage.devicePixelRatioF() )
                          .arg( mImage.height() / mImage.devicePixelRatioF() )
                          .arg( w ).arg( h ), 2 );
        // This happens on zoom events when ::paint is called before
        // the renderer has completed
        scale = true;
      }

      if ( scale )
        painter->drawImage( QRect( 0, 0, w, h ), mImage );
      else
        painter->drawImage( 0, 0, mImage );
    }

  private:

    QImage mImage;
    QRectF mRect;

};




QgsElevationProfileCanvas::QgsElevationProfileCanvas( QWidget *parent )
  : QgsDistanceVsElevationPlotCanvas( parent )
{
  mPlotItem = new QgsElevationProfilePlotItem( this );

}

void QgsElevationProfileCanvas::update()
{
  if ( !mProject || !profileCurve() )
    return;

  mProfileResults.clear();

  QgsProfileRequest req( profileCurve()->clone() );
  req.setCrs( mCrs );
  req.setTransformContext( mProject->transformContext() );
  req.setTerrainProvider( mProject->elevationProperties()->terrainProvider() ? mProject->elevationProperties()->terrainProvider()->clone() : nullptr );

  // TODO -- background thread!

  const QList< QgsMapLayer * > layersToUpdate = layers();
  std::vector< std::unique_ptr< QgsAbstractProfileGenerator > > generators;
  for ( QgsMapLayer *layer : layersToUpdate )
  {
    if ( QgsAbstractProfileSource *source = dynamic_cast< QgsAbstractProfileSource * >( layer ) )
    {
      std::unique_ptr< QgsAbstractProfileGenerator > generator( source->createProfileGenerator( req ) );
      if ( generator )
        generators.emplace_back( std::move( generator ) );
    }
  }

  for ( auto &it : generators )
  {
    it->generateProfile();
    std::unique_ptr< QgsAbstractProfileResults > res( it->takeResults() );
    if ( res )
      mProfileResults.emplace_back( std::move( res ) );
  }

  QImage res( width(), height(), QImage::Format_ARGB32 );
  res.fill( Qt::white );

  QPainter p( &res );
  QgsRenderContext context = QgsRenderContext::fromQPainter( &p );
  QgsProfileRenderContext profileRenderContext( context );

  const double curveLength = profileCurve()->length();
  QTransform transform;
  transform.translate( 0, mPlotContentsRect.height() );
  transform.scale( mPlotContentsRect.width() / curveLength, -mPlotContentsRect.height() / ( mZMax - mZMin ) );
  transform.translate( 0, -mZMin );
  profileRenderContext.setWorldTransform( transform );


  p.setRenderHint( QPainter::Antialiasing, true );
  p.setBrush( QBrush( Qt::red ) );
  p.setPen( Qt::NoPen );
  for ( auto &it  : mProfileResults )
  {
    it->renderResults( profileRenderContext );
  }
  p.end();

  mPlotItem->setContent( res );
}

void QgsElevationProfileCanvas::setProject( QgsProject *project )
{
  mProject = project;
}

QgsElevationProfileCanvas::~QgsElevationProfileCanvas() = default;

void QgsElevationProfileCanvas::setLayers( const QList<QgsMapLayer *> &layers )
{
  // filter list, removing null layers and invalid layers
  auto filteredList = layers;
  filteredList.erase( std::remove_if( filteredList.begin(), filteredList.end(),
                                      []( QgsMapLayer * layer )
  {
    return !layer || !layer->isValid();
  } ), filteredList.end() );

  mLayers = _qgis_listRawToQPointer( filteredList );
}

QList<QgsMapLayer *> QgsElevationProfileCanvas::layers() const
{
  return _qgis_listQPointerToRaw( mLayers );
}
