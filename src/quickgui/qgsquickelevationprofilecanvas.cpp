/***************************************************************************
                          QgsQuickElevationProfileCanvas.cpp
                          -----------------
    begin                : October 2022
    copyright            : (C) 2022 by Mathieu Pellerin
    email                : mathieu at opengis dot ch
***************************************************************************/


/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsabstractprofilegenerator.h"
#include "qgsabstractprofilesource.h"
#include "qgsexpressioncontextutils.h"
#include "qgsmaplayerelevationproperties.h"
#include "qgsmaplayerutils.h"
#include "qgsmaplayerlistutils_p.h"
#include "qgsplot.h"
#include "qgsprofilerenderer.h"
#include "qgsprofilerequest.h"
#include "qgsprojectelevationproperties.h"
#include "qgsquickelevationprofilecanvas.h"
#include "moc_qgsquickelevationprofilecanvas.cpp"
#include "qgsterrainprovider.h"

#include <QQuickWindow>
#include <QSGSimpleRectNode>
#include <QSGSimpleTextureNode>
#include <QScreen>
#include <QTimer>


///@cond PRIVATE
class QgsElevationProfilePlotItem : public Qgs2DPlot
{
  public:
    explicit QgsElevationProfilePlotItem( QgsQuickElevationProfileCanvas *canvas )
      : mCanvas( canvas )
    {
      setYMinimum( 0 );
      setYMaximum( 100 );
      setSize( mCanvas->boundingRect().size() );
    }

    void setRenderer( QgsProfilePlotRenderer *renderer )
    {
      mRenderer = renderer;
    }

    void updateRect()
    {
      setSize( mCanvas->boundingRect().size() );
      mCachedImages.clear();
      mPlotArea = QRectF();
    }

    void updatePlot()
    {
      mCachedImages.clear();
      mPlotArea = QRectF();
    }

    bool redrawResults( const QString &sourceId )
    {
      auto it = mCachedImages.find( sourceId );
      if ( it == mCachedImages.end() )
        return false;

      mCachedImages.erase( it );
      return true;
    }

    QRectF plotArea()
    {
      if ( !mPlotArea.isNull() )
        return mPlotArea;

      // force immediate recalculation of plot area
      QgsRenderContext context;
      context.setScaleFactor( ( mCanvas->window()->screen()->physicalDotsPerInch() * mCanvas->window()->screen()->devicePixelRatio() ) / 25.4 );

      calculateOptimisedIntervals( context );
      mPlotArea = interiorPlotArea( context );
      return mPlotArea;
    }

    void renderContent( QgsRenderContext &rc, const QRectF &plotArea ) override
    {
      mPlotArea = plotArea;

      if ( !mRenderer )
        return;

      const QStringList sourceIds = mRenderer->sourceIds();
      for ( const QString &source : sourceIds )
      {
        QImage plot;
        auto it = mCachedImages.constFind( source );
        if ( it != mCachedImages.constEnd() )
        {
          plot = it.value();
        }
        else
        {
          const float devicePixelRatio = static_cast<float>( mCanvas->window()->screen()->devicePixelRatio() );
          plot = QImage( static_cast<int>( plotArea.width() * devicePixelRatio ), static_cast<int>( plotArea.height() * devicePixelRatio ), QImage::Format_ARGB32_Premultiplied );
          plot.setDevicePixelRatio( devicePixelRatio );
          plot.fill( Qt::transparent );

          QPainter plotPainter( &plot );
          plotPainter.setRenderHint( QPainter::Antialiasing, true );
          QgsRenderContext plotRc = QgsRenderContext::fromQPainter( &plotPainter );
          plotRc.setDevicePixelRatio( devicePixelRatio );

          const double mapUnitsPerPixel = ( xMaximum() - xMinimum() ) / plotArea.width();
          plotRc.setMapToPixel( QgsMapToPixel( mapUnitsPerPixel ) );

          mRenderer->render( plotRc, plotArea.width(), plotArea.height(), xMinimum(), xMaximum(), yMinimum(), yMaximum(), source );
          plotPainter.end();

          mCachedImages.insert( source, plot );
        }
        rc.painter()->drawImage( static_cast<int>( plotArea.left() ), static_cast<int>( plotArea.top() ), plot );
      }
    }

  private:
    QgsQuickElevationProfileCanvas *mCanvas = nullptr;
    QgsProfilePlotRenderer *mRenderer = nullptr;

    QRectF mPlotArea;
    QMap<QString, QImage> mCachedImages;
};
///@endcond PRIVATE


QgsQuickElevationProfileCanvas::QgsQuickElevationProfileCanvas( QQuickItem *parent )
  : QQuickItem( parent )
{
  // updating the profile plot is deferred on a timer, so that we don't trigger it too often
  mDeferredRegenerationTimer = new QTimer( this );
  mDeferredRegenerationTimer->setSingleShot( true );
  mDeferredRegenerationTimer->stop();
  connect( mDeferredRegenerationTimer, &QTimer::timeout, this, &QgsQuickElevationProfileCanvas::startDeferredRegeneration );

  mDeferredRedrawTimer = new QTimer( this );
  mDeferredRedrawTimer->setSingleShot( true );
  mDeferredRedrawTimer->stop();
  connect( mDeferredRedrawTimer, &QTimer::timeout, this, &QgsQuickElevationProfileCanvas::startDeferredRedraw );

  mPlotItem = new QgsElevationProfilePlotItem( this );

  setTransformOrigin( QQuickItem::TopLeft );
  setFlags( QQuickItem::ItemHasContents );
}

QgsQuickElevationProfileCanvas::~QgsQuickElevationProfileCanvas()
{
  if ( mCurrentJob )
  {
    mPlotItem->setRenderer( nullptr );
    mCurrentJob->deleteLater();
    mCurrentJob = nullptr;
  }
}

void QgsQuickElevationProfileCanvas::cancelJobs()
{
  if ( mCurrentJob )
  {
    mPlotItem->setRenderer( nullptr );
    disconnect( mCurrentJob, &QgsProfilePlotRenderer::generationFinished, this, &QgsQuickElevationProfileCanvas::generationFinished );
    mCurrentJob->cancelGeneration();
    mCurrentJob->deleteLater();
    mCurrentJob = nullptr;
  }
}

void QgsQuickElevationProfileCanvas::setupLayerConnections( QgsMapLayer *layer, bool isDisconnect )
{
  if ( !layer )
    return;

  if ( isDisconnect )
  {
    disconnect( layer->elevationProperties(), &QgsMapLayerElevationProperties::profileGenerationPropertyChanged, this, &QgsQuickElevationProfileCanvas::onLayerProfileGenerationPropertyChanged );
    disconnect( layer->elevationProperties(), &QgsMapLayerElevationProperties::profileRenderingPropertyChanged, this, &QgsQuickElevationProfileCanvas::onLayerProfileRendererPropertyChanged );
    disconnect( layer, &QgsMapLayer::dataChanged, this, &QgsQuickElevationProfileCanvas::regenerateResultsForLayer );
  }
  else
  {
    connect( layer->elevationProperties(), &QgsMapLayerElevationProperties::profileGenerationPropertyChanged, this, &QgsQuickElevationProfileCanvas::onLayerProfileGenerationPropertyChanged );
    connect( layer->elevationProperties(), &QgsMapLayerElevationProperties::profileRenderingPropertyChanged, this, &QgsQuickElevationProfileCanvas::onLayerProfileRendererPropertyChanged );
    connect( layer, &QgsMapLayer::dataChanged, this, &QgsQuickElevationProfileCanvas::regenerateResultsForLayer );
  }

  switch ( layer->type() )
  {
    case Qgis::LayerType::Vector:
    {
      QgsVectorLayer *vl = qobject_cast<QgsVectorLayer *>( layer );
      if ( isDisconnect )
      {
        disconnect( vl, &QgsVectorLayer::featureAdded, this, &QgsQuickElevationProfileCanvas::regenerateResultsForLayer );
        disconnect( vl, &QgsVectorLayer::featureDeleted, this, &QgsQuickElevationProfileCanvas::regenerateResultsForLayer );
        disconnect( vl, &QgsVectorLayer::geometryChanged, this, &QgsQuickElevationProfileCanvas::regenerateResultsForLayer );
        disconnect( vl, &QgsVectorLayer::attributeValueChanged, this, &QgsQuickElevationProfileCanvas::regenerateResultsForLayer );
      }
      else
      {
        connect( vl, &QgsVectorLayer::featureAdded, this, &QgsQuickElevationProfileCanvas::regenerateResultsForLayer );
        connect( vl, &QgsVectorLayer::featureDeleted, this, &QgsQuickElevationProfileCanvas::regenerateResultsForLayer );
        connect( vl, &QgsVectorLayer::geometryChanged, this, &QgsQuickElevationProfileCanvas::regenerateResultsForLayer );
        connect( vl, &QgsVectorLayer::attributeValueChanged, this, &QgsQuickElevationProfileCanvas::regenerateResultsForLayer );
      }
      break;
    }
    case Qgis::LayerType::Raster:
    case Qgis::LayerType::Plugin:
    case Qgis::LayerType::Mesh:
    case Qgis::LayerType::VectorTile:
    case Qgis::LayerType::Annotation:
    case Qgis::LayerType::PointCloud:
    case Qgis::LayerType::Group:
    case Qgis::LayerType::TiledScene:
      break;
  }
}

bool QgsQuickElevationProfileCanvas::isRendering() const
{
  return mCurrentJob && mCurrentJob->isActive();
}

void QgsQuickElevationProfileCanvas::refresh()
{
  if ( !mCrs.isValid() || !mProject || mProfileCurve.isEmpty() )
    return;

  if ( mCurrentJob )
  {
    mPlotItem->setRenderer( nullptr );
    disconnect( mCurrentJob, &QgsProfilePlotRenderer::generationFinished, this, &QgsQuickElevationProfileCanvas::generationFinished );
    mCurrentJob->deleteLater();
    mCurrentJob = nullptr;
  }

  QgsProfileRequest request( static_cast<QgsCurve *>( mProfileCurve.get()->clone() ) );
  request.setCrs( mCrs );
  request.setTolerance( mTolerance );
  request.setTransformContext( mProject->transformContext() );
  request.setTerrainProvider( mProject->elevationProperties()->terrainProvider() ? mProject->elevationProperties()->terrainProvider()->clone() : nullptr );

  QgsExpressionContext context;
  context.appendScope( QgsExpressionContextUtils::globalScope() );
  context.appendScope( QgsExpressionContextUtils::projectScope( mProject ) );
  request.setExpressionContext( context );

  const QList<QgsMapLayer *> layersToGenerate = layers();
  QList<QgsAbstractProfileSource *> sources;
  sources.reserve( layersToGenerate.size() );
  for ( QgsMapLayer *layer : layersToGenerate )
  {
    if ( QgsAbstractProfileSource *source = dynamic_cast<QgsAbstractProfileSource *>( layer ) )
      sources.append( source );
  }

  mCurrentJob = new QgsProfilePlotRenderer( sources, request );
  connect( mCurrentJob, &QgsProfilePlotRenderer::generationFinished, this, &QgsQuickElevationProfileCanvas::generationFinished );

  QgsProfileGenerationContext generationContext;
  generationContext.setDpi( window()->screen()->physicalDotsPerInch() * window()->screen()->devicePixelRatio() );
  generationContext.setMaximumErrorMapUnits( MAX_ERROR_PIXELS * ( mProfileCurve.get()->length() ) / mPlotItem->plotArea().width() );
  generationContext.setMapUnitsPerDistancePixel( mProfileCurve.get()->length() / mPlotItem->plotArea().width() );
  mCurrentJob->setContext( generationContext );

  mPlotItem->updatePlot();
  mCurrentJob->startGeneration();
  mPlotItem->setRenderer( mCurrentJob );

  emit activeJobCountChanged( 1 );
  emit isRenderingChanged();
}

void QgsQuickElevationProfileCanvas::generationFinished()
{
  if ( !mCurrentJob )
    return;

  emit activeJobCountChanged( 0 );

  if ( mZoomFullWhenJobFinished )
  {
    mZoomFullWhenJobFinished = false;
    zoomFull();
  }

  QRectF rect = boundingRect();
  const float devicePixelRatio = static_cast<float>( window()->screen()->devicePixelRatio() );
  mImage = QImage( static_cast<int>( rect.width() * devicePixelRatio ), static_cast<int>( rect.height() * devicePixelRatio ), QImage::Format_ARGB32_Premultiplied );
  mImage.setDevicePixelRatio( devicePixelRatio );
  mImage.fill( Qt::transparent );

  QPainter imagePainter( &mImage );
  imagePainter.setRenderHint( QPainter::Antialiasing, true );
  QgsRenderContext rc = QgsRenderContext::fromQPainter( &imagePainter );
  rc.setDevicePixelRatio( devicePixelRatio );

  rc.expressionContext().appendScope( QgsExpressionContextUtils::globalScope() );
  rc.expressionContext().appendScope( QgsExpressionContextUtils::projectScope( mProject ) );

  mPlotItem->calculateOptimisedIntervals( rc );
  mPlotItem->render( rc );
  imagePainter.end();

  mDirty = true;
  update();

  if ( mForceRegenerationAfterCurrentJobCompletes )
  {
    mForceRegenerationAfterCurrentJobCompletes = false;
    mCurrentJob->invalidateAllRefinableSources();
    scheduleDeferredRegeneration();
  }
  else
  {
    emit isRenderingChanged();
  }
}

void QgsQuickElevationProfileCanvas::onLayerProfileGenerationPropertyChanged()
{
  // TODO -- handle nicely when existing job is in progress
  if ( !mCurrentJob || mCurrentJob->isActive() )
    return;

  QgsMapLayerElevationProperties *properties = qobject_cast<QgsMapLayerElevationProperties *>( sender() );
  if ( !properties )
    return;

  if ( QgsMapLayer *layer = qobject_cast<QgsMapLayer *>( properties->parent() ) )
  {
    if ( QgsAbstractProfileSource *source = dynamic_cast<QgsAbstractProfileSource *>( layer ) )
    {
      if ( mCurrentJob->invalidateResults( source ) )
        scheduleDeferredRegeneration();
    }
  }
}

void QgsQuickElevationProfileCanvas::onLayerProfileRendererPropertyChanged()
{
  // TODO -- handle nicely when existing job is in progress
  if ( !mCurrentJob || mCurrentJob->isActive() )
    return;

  QgsMapLayerElevationProperties *properties = qobject_cast<QgsMapLayerElevationProperties *>( sender() );
  if ( !properties )
    return;

  if ( QgsMapLayer *layer = qobject_cast<QgsMapLayer *>( properties->parent() ) )
  {
    if ( QgsAbstractProfileSource *source = dynamic_cast<QgsAbstractProfileSource *>( layer ) )
    {
      mCurrentJob->replaceSource( source );
    }
    if ( mPlotItem->redrawResults( layer->id() ) )
      scheduleDeferredRedraw();
  }
}

void QgsQuickElevationProfileCanvas::regenerateResultsForLayer()
{
  if ( !mCurrentJob )
    return;

  if ( QgsMapLayer *layer = qobject_cast<QgsMapLayer *>( sender() ) )
  {
    if ( QgsAbstractProfileSource *source = dynamic_cast<QgsAbstractProfileSource *>( layer ) )
    {
      if ( mCurrentJob->invalidateResults( source ) )
        scheduleDeferredRegeneration();
    }
  }
}

void QgsQuickElevationProfileCanvas::scheduleDeferredRegeneration()
{
  if ( !mDeferredRegenerationScheduled )
  {
    mDeferredRegenerationTimer->start( 1 );
    mDeferredRegenerationScheduled = true;
  }
}

void QgsQuickElevationProfileCanvas::scheduleDeferredRedraw()
{
  if ( !mDeferredRedrawScheduled )
  {
    mDeferredRedrawTimer->start( 1 );
    mDeferredRedrawScheduled = true;
  }
}

void QgsQuickElevationProfileCanvas::startDeferredRegeneration()
{
  if ( mCurrentJob && !mCurrentJob->isActive() )
  {
    emit activeJobCountChanged( 1 );
    mCurrentJob->regenerateInvalidatedResults();
  }
  else if ( mCurrentJob )
  {
    mForceRegenerationAfterCurrentJobCompletes = true;
  }

  mDeferredRegenerationScheduled = false;
}

void QgsQuickElevationProfileCanvas::startDeferredRedraw()
{
  refresh();
  mDeferredRedrawScheduled = false;
}

void QgsQuickElevationProfileCanvas::refineResults()
{
  if ( mCurrentJob )
  {
    QgsProfileGenerationContext context;
    context.setDpi( window()->screen()->physicalDotsPerInch() * window()->screen()->devicePixelRatio() );
    const double plotDistanceRange = mPlotItem->xMaximum() - mPlotItem->xMinimum();
    const double plotElevationRange = mPlotItem->yMaximum() - mPlotItem->yMinimum();
    const double plotDistanceUnitsPerPixel = plotDistanceRange / mPlotItem->plotArea().width();

    // we round the actual desired map error down to just one significant figure, to avoid tiny differences
    // as the plot is panned
    const double targetMaxErrorInMapUnits = MAX_ERROR_PIXELS * plotDistanceUnitsPerPixel;
    const double factor = std::pow( 10.0, 1 - std::ceil( std::log10( std::fabs( targetMaxErrorInMapUnits ) ) ) );
    const double roundedErrorInMapUnits = std::floor( targetMaxErrorInMapUnits * factor ) / factor;
    context.setMaximumErrorMapUnits( roundedErrorInMapUnits );

    context.setMapUnitsPerDistancePixel( plotDistanceUnitsPerPixel );

    // for similar reasons we round the minimum distance off to multiples of the maximum error in map units
    const double distanceMin = std::floor( ( mPlotItem->xMinimum() - plotDistanceRange * 0.05 ) / context.maximumErrorMapUnits() ) * context.maximumErrorMapUnits();
    context.setDistanceRange( QgsDoubleRange( std::max( 0.0, distanceMin ), mPlotItem->xMaximum() + plotDistanceRange * 0.05 ) );

    context.setElevationRange( QgsDoubleRange( mPlotItem->yMinimum() - plotElevationRange * 0.05, mPlotItem->yMaximum() + plotElevationRange * 0.05 ) );
    mCurrentJob->setContext( context );
  }
  scheduleDeferredRegeneration();
}

void QgsQuickElevationProfileCanvas::setProject( QgsProject *project )
{
  if ( mProject == project )
    return;

  mProject = project;

  emit projectChanged();
}

void QgsQuickElevationProfileCanvas::setCrs( const QgsCoordinateReferenceSystem &crs )
{
  if ( mCrs == crs )
    return;

  mCrs = crs;

  emit crsChanged();
}

void QgsQuickElevationProfileCanvas::setProfileCurve( QgsGeometry curve )
{
  if ( mProfileCurve.equals( curve ) )
    return;

  mProfileCurve = curve.type() == Qgis::GeometryType::Line ? curve : QgsGeometry();

  emit profileCurveChanged();
}

void QgsQuickElevationProfileCanvas::setTolerance( double tolerance )
{
  if ( mTolerance == tolerance )
    return;

  mTolerance = tolerance;

  emit toleranceChanged();
}

void QgsQuickElevationProfileCanvas::populateLayersFromProject()
{
  for ( QgsMapLayer *layer : std::as_const( mLayers ) )
  {
    setupLayerConnections( layer, true );
  }

  if ( !mProject )
  {
    mLayers.clear();
    return;
  }

  const QList<QgsMapLayer *> projectLayers = QgsProject::instance()->layers<QgsMapLayer *>().toList();
  // sort layers so that types which are more likely to obscure others are rendered below
  // e.g. vector features should be drawn above raster DEMS, or the DEM line may completely obscure
  // the vector feature
  QList<QgsMapLayer *> sortedLayers = QgsMapLayerUtils::sortLayersByType( projectLayers, { Qgis::LayerType::Raster, Qgis::LayerType::Mesh, Qgis::LayerType::Vector, Qgis::LayerType::PointCloud } );

  // filter list, removing null layers and invalid layers
  auto filteredList = sortedLayers;
  filteredList.erase( std::remove_if( filteredList.begin(), filteredList.end(), []( QgsMapLayer *layer ) {
                        return !layer || !layer->isValid() || !layer->elevationProperties() || !layer->elevationProperties()->showByDefaultInElevationProfilePlots();
                      } ),
                      filteredList.end() );

  mLayers = _qgis_listRawToQPointer( filteredList );
  for ( QgsMapLayer *layer : std::as_const( mLayers ) )
  {
    setupLayerConnections( layer, false );
  }
}

QList<QgsMapLayer *> QgsQuickElevationProfileCanvas::layers() const
{
  return _qgis_listQPointerToRaw( mLayers );
}

#if QT_VERSION < QT_VERSION_CHECK( 6, 0, 0 )
void QgsQuickElevationProfileCanvas::geometryChanged( const QRectF &newGeometry, const QRectF &oldGeometry )
{
  QQuickItem::geometryChanged( newGeometry, oldGeometry );
#else
void QgsQuickElevationProfileCanvas::geometryChange( const QRectF &newGeometry, const QRectF &oldGeometry )
{
  QQuickItem::geometryChange( newGeometry, oldGeometry );
#endif
  mPlotItem->updateRect();
  mDirty = true;
  refresh();
}

QSGNode *QgsQuickElevationProfileCanvas::updatePaintNode( QSGNode *oldNode, QQuickItem::UpdatePaintNodeData * )
{
  if ( mDirty )
  {
    delete oldNode;
    oldNode = nullptr;
    mDirty = false;
  }

  QSGNode *newNode = nullptr;
  if ( !mImage.isNull() )
  {
    QSGSimpleTextureNode *node = static_cast<QSGSimpleTextureNode *>( oldNode );
    if ( !node )
    {
      node = new QSGSimpleTextureNode();
      QSGTexture *texture = window()->createTextureFromImage( mImage );
      node->setTexture( texture );
      node->setOwnsTexture( true );
    }

    QRectF rect( boundingRect() );
    QSizeF size = mImage.size();
    if ( !size.isEmpty() )
      size /= window()->screen()->devicePixelRatio();

    // Check for resizes that change the w/h ratio
    if ( !rect.isEmpty() && !size.isEmpty() && !qgsDoubleNear( rect.width() / rect.height(), ( size.width() ) / static_cast<double>( size.height() ), 3 ) )
    {
      if ( qgsDoubleNear( rect.height(), mImage.height() ) )
      {
        rect.setHeight( rect.width() / size.width() * size.height() );
      }
      else
      {
        rect.setWidth( rect.height() / size.height() * size.width() );
      }
    }
    node->setRect( rect );
    newNode = node;
  }
  else
  {
    QSGSimpleRectNode *node = static_cast<QSGSimpleRectNode *>( oldNode );
    if ( !node )
    {
      node = new QSGSimpleRectNode();
      node->setColor( Qt::transparent );
    }
    node->setRect( boundingRect() );
    newNode = node;
  }

  return newNode;
}

void QgsQuickElevationProfileCanvas::zoomFull()
{
  if ( !mCurrentJob )
    return;

  const QgsDoubleRange zRange = mCurrentJob->zRange();

  if ( zRange.upper() < zRange.lower() )
  {
    // invalid range, e.g. no features found in plot!
    mPlotItem->setYMinimum( 0 );
    mPlotItem->setYMaximum( 10 );
  }
  else if ( qgsDoubleNear( zRange.lower(), zRange.upper(), 0.0000001 ) )
  {
    // corner case ... a zero height plot! Just pick an arbitrary +/- 5 height range.
    mPlotItem->setYMinimum( zRange.lower() - 5 );
    mPlotItem->setYMaximum( zRange.lower() + 5 );
  }
  else
  {
    // add 5% margin to height range
    const double margin = ( zRange.upper() - zRange.lower() ) * 0.05;
    mPlotItem->setYMinimum( zRange.lower() - margin );
    mPlotItem->setYMaximum( zRange.upper() + margin );
  }

  const double profileLength = mProfileCurve.get()->length();
  mPlotItem->setXMinimum( 0 );
  // just 2% margin to max distance -- any more is overkill and wasted space
  mPlotItem->setXMaximum( profileLength * 1.02 );

  refineResults();
}

void QgsQuickElevationProfileCanvas::zoomFullInRatio()
{
  if ( !mCurrentJob )
    return;

  const QgsDoubleRange zRange = mCurrentJob->zRange();
  double xLength = mProfileCurve.get()->length();
  double yLength = zRange.upper() - zRange.lower();
  qDebug() << yLength;
  if ( yLength < 0.0 )
  {
    // invalid range, e.g. no features found in plot!
    mPlotItem->setYMinimum( 0 );
    mPlotItem->setYMaximum( 10 );

    mPlotItem->setXMinimum( 0 );
    // just 2% margin to max distance -- any more is overkill and wasted space
    mPlotItem->setXMaximum( xLength * 1.02 );
  }
  else
  {
    double yInRatioLength = xLength * mPlotItem->size().height() / mPlotItem->size().width();
    double xInRatioLength = yLength * mPlotItem->size().width() / mPlotItem->size().height();
    if ( yInRatioLength > yLength )
    {
      qDebug() << "yInRatioLength";
      mPlotItem->setYMinimum( zRange.lower() - ( yInRatioLength / 2 ) );
      qDebug() << mPlotItem->yMinimum();
      mPlotItem->setYMaximum( zRange.upper() + ( yInRatioLength / 2 ) );
      qDebug() << mPlotItem->yMaximum();

      mPlotItem->setXMinimum( 0 );
      // just 2% margin to max distance -- any more is overkill and wasted space
      mPlotItem->setXMaximum( xLength * 1.02 );
    }
    else
    {
      qDebug() << "xInRatioLength";
      // add 5% margin to height range
      const double margin = yLength * 0.05;
      mPlotItem->setYMinimum( zRange.lower() - margin );
      qDebug() << mPlotItem->yMinimum();
      mPlotItem->setYMaximum( zRange.upper() + margin );
      qDebug() << mPlotItem->yMaximum();

      mPlotItem->setXMinimum( 0 - ( xInRatioLength / 2 ) );
      mPlotItem->setXMaximum( xLength + ( xInRatioLength / 2 ) );
    }
  }

  refineResults();
}

void QgsQuickElevationProfileCanvas::setVisiblePlotRange( double minimumDistance, double maximumDistance, double minimumElevation, double maximumElevation )
{
  mPlotItem->setYMinimum( minimumElevation );
  mPlotItem->setYMaximum( maximumElevation );
  mPlotItem->setXMinimum( minimumDistance );
  mPlotItem->setXMaximum( maximumDistance );
  refineResults();
}

QgsDoubleRange QgsQuickElevationProfileCanvas::visibleDistanceRange() const
{
  return QgsDoubleRange( mPlotItem->xMinimum(), mPlotItem->xMaximum() );
}

QgsDoubleRange QgsQuickElevationProfileCanvas::visibleElevationRange() const
{
  return QgsDoubleRange( mPlotItem->yMinimum(), mPlotItem->yMaximum() );
}

void QgsQuickElevationProfileCanvas::clear()
{
  setProfileCurve( QgsGeometry() );
  if ( mCurrentJob )
  {
    mPlotItem->setRenderer( nullptr );
    disconnect( mCurrentJob, &QgsProfilePlotRenderer::generationFinished, this, &QgsQuickElevationProfileCanvas::generationFinished );
    mCurrentJob->deleteLater();
    mCurrentJob = nullptr;
  }

  mZoomFullWhenJobFinished = true;

  mImage = QImage();
  mDirty = true;
  update();
}
