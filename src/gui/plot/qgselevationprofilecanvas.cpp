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
#include "qgsprofilerenderer.h"
#include "qgsplot.h"

class QgsElevationProfilePlotItem : public Qgs2DPlot, public QgsPlotCanvasItem
{
  public:

    QgsElevationProfilePlotItem( QgsElevationProfileCanvas *canvas )
      : QgsPlotCanvasItem( canvas )
    {
      setYMinimum( 0 );
      setYMaximum( 100 );
    }

    void setRenderer( QgsProfilePlotRenderer *renderer )
    {
      mRenderer = renderer;
    }

    void updateRect()
    {
      mRect = mCanvas->rect();
      setSize( mRect.size() );

      prepareGeometryChange();
      setPos( mRect.topLeft() );

      mImage = QImage();
      update();
    }

    void updatePlot()
    {
      mImage = QImage();
      update();
    }

    QRectF boundingRect() const override
    {
      return mRect;
    }

    void renderContent( QgsRenderContext &rc, const QRectF &plotArea ) override
    {
      if ( !mRenderer )
        return;

      const QImage plot = mRenderer->renderToImage( plotArea.width(), plotArea.height(), xMinimum(), xMaximum(), yMinimum(), yMaximum() );
      rc.painter()->drawImage( plotArea.left(), plotArea.top(), plot );
    }

    void paint( QPainter *painter ) override
    {
      // cache rendering to an image, so we don't need to redraw the plot
      if ( !mImage.isNull() )
      {
        painter->drawImage( 0, 0, mImage );
      }
      else
      {
        mImage = QImage( mRect.width(), mRect.height(), QImage::Format_ARGB32_Premultiplied );
        mImage.fill( Qt::transparent );

        QPainter imagePainter( &mImage );
        imagePainter.setRenderHint( QPainter::Antialiasing, true );
        QgsRenderContext rc = QgsRenderContext::fromQPainter( &imagePainter );
        calculateOptimisedIntervals( rc );
        render( rc );
        imagePainter.end();

        painter->drawImage( 0, 0, mImage );
      }
    }

  private:

    QImage mImage;
    QRectF mRect;
    QgsProfilePlotRenderer *mRenderer = nullptr;
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

  QgsProfileRequest request( profileCurve()->clone() );
  request.setCrs( mCrs );
  request.setTransformContext( mProject->transformContext() );
  request.setTerrainProvider( mProject->elevationProperties()->terrainProvider() ? mProject->elevationProperties()->terrainProvider()->clone() : nullptr );

  const QList< QgsMapLayer * > layersToUpdate = layers();
  QList< QgsAbstractProfileSource * > sources;
  sources.reserve( layersToUpdate.size() );
  for ( QgsMapLayer *layer : layersToUpdate )
  {
    if ( QgsAbstractProfileSource *source = dynamic_cast< QgsAbstractProfileSource * >( layer ) )
      sources.append( source );
  }

  mCurrentJob = new QgsProfilePlotRenderer( sources, request );
  connect( mCurrentJob, &QgsProfilePlotRenderer::generationFinished, this, &QgsElevationProfileCanvas::generationFinished );
  mCurrentJob->startGeneration();
  mPlotItem->setRenderer( mCurrentJob );
}

void QgsElevationProfileCanvas::generationFinished()
{
  if ( !mCurrentJob )
    return;

  zoomFull();
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

void QgsElevationProfileCanvas::resizeEvent( QResizeEvent *event )
{
  QgsDistanceVsElevationPlotCanvas::resizeEvent( event );
  mPlotItem->updateRect();
}

void QgsElevationProfileCanvas::showEvent( QShowEvent *event )
{
  QgsDistanceVsElevationPlotCanvas::showEvent( event );
  mPlotItem->updateRect();
}

void QgsElevationProfileCanvas::zoomFull()
{
  if ( !mCurrentJob )
    return;

  const QgsDoubleRange zRange = mCurrentJob->zRange();
  // add 5% margin to height range
  const double margin = ( zRange.upper() - zRange.lower() ) * 0.05;
  mPlotItem->setYMinimum( zRange.lower() - margin );
  mPlotItem->setYMaximum( zRange.upper() + margin );

  const double profileLength = profileCurve()->length();
  mPlotItem->setXMinimum( 0 );
  // just 2% margin to max distance -- any more is overkill and wasted space
  mPlotItem->setXMaximum( profileLength  * 1.02 );

  mPlotItem->updatePlot();
}

void QgsElevationProfileCanvas::clear()
{
  setProfileCurve( nullptr );
  mPlotItem->setRenderer( nullptr );
  mPlotItem->updatePlot();
}
