/***************************************************************************
                         qgslayoutitemchart.cpp
                         -------------------
     begin                : August 2025
     copyright            : (C) 2025 by Mathieu
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

#include "qgslayoutitemchart.h"

#include "qgsapplication.h"
#include "qgslayout.h"
#include "qgslayoutitemregistry.h"
#include "qgslayoutrendercontext.h"
#include "qgslayoutreportcontext.h"
#include "qgslayoututils.h"
#include "qgsplotregistry.h"

#include <QDomDocument>
#include <QDomElement>
#include <QPainter>
#include <QString>

#include "moc_qgslayoutitemchart.cpp"

using namespace Qt::StringLiterals;

QgsLayoutItemChart::QgsLayoutItemChart( QgsLayout *layout )
  : QgsLayoutItem( layout )
{
  // default to no background
  setBackgroundEnabled( false );

  mPlot.reset( dynamic_cast<Qgs2DPlot *>( QgsApplication::instance()->plotRegistry()->createPlot( "line" ) ) );

  mGathererTimer.setInterval( 10 );
  mGathererTimer.setSingleShot( true );
  connect( &mGathererTimer, &QTimer::timeout, this, &QgsLayoutItemChart::gatherData );
}

int QgsLayoutItemChart::type() const
{
  return QgsLayoutItemRegistry::LayoutChart;
}

QIcon QgsLayoutItemChart::icon() const
{
  return QgsApplication::getThemeIcon( u"/mLayoutItemChart.svg"_s );
}

QgsLayoutItemChart *QgsLayoutItemChart::create( QgsLayout *layout )
{
  return new QgsLayoutItemChart( layout );
}

void QgsLayoutItemChart::setPlot( QgsPlot *plot )
{
  Qgs2DPlot *plot2d = dynamic_cast<Qgs2DPlot *>( plot );
  if ( !plot2d )
  {
    delete plot;
    return;
  }

  // Logic to minimize plot data refresh to bare minimum
  bool requireRefresh = !mPlot || !plot;
  if ( mPlot && plot )
  {
    if ( mPlot->type() != plot->type() )
    {
      requireRefresh = true;
    }
    else
    {
      Qgs2DXyPlot *oldPlot2dXy = dynamic_cast<Qgs2DXyPlot *>( mPlot.get() );
      Qgs2DXyPlot *newPlot2dXy = dynamic_cast<Qgs2DXyPlot *>( plot2d );
      if ( oldPlot2dXy && newPlot2dXy && oldPlot2dXy->xAxis().type() == newPlot2dXy->xAxis().type() )
      {
        // this is a case in which we don't need to refresh the plot data.
        requireRefresh = false;
      }
      else
      {
        requireRefresh = true;
      }
    }
  }

  mPlot.reset( plot2d );
  if ( requireRefresh )
  {
    refresh();
  }

  emit changed();
}

void QgsLayoutItemChart::setSourceLayer( QgsVectorLayer *layer )
{
  if ( layer == mVectorLayer.get() )
  {
    return;
  }

  mVectorLayer.setLayer( layer );
  refresh();

  emit changed();
}


void QgsLayoutItemChart::setSortFeatures( bool sorted )
{
  if ( mSortFeatures == sorted )
  {
    return;
  }

  mSortFeatures = sorted;
  refresh();

  emit changed();
}

void QgsLayoutItemChart::setSortAscending( bool ascending )
{
  if ( mSortAscending == ascending )
  {
    return;
  }

  mSortAscending = ascending;
  refresh();

  emit changed();
}

void QgsLayoutItemChart::setSortExpression( const QString &expression )
{
  if ( mSortExpression == expression )
  {
    return;
  }

  mSortExpression = expression;
  refresh();

  emit changed();
}

void QgsLayoutItemChart::setMap( QgsLayoutItemMap *map )
{
  if ( mMap == map )
  {
    return;
  }

  if ( mMap )
  {
    disconnect( mMap, &QgsLayoutItemMap::extentChanged, this, &QgsLayoutItemChart::refresh );
    disconnect( mMap, &QgsLayoutItemMap::mapRotationChanged, this, &QgsLayoutItemChart::refresh );
  }
  mMap = map;
  if ( mMap )
  {
    connect( mMap, &QgsLayoutItemMap::extentChanged, this, &QgsLayoutItemChart::refresh );
    connect( mMap, &QgsLayoutItemMap::mapRotationChanged, this, &QgsLayoutItemChart::refresh );
  }
  refresh();

  emit changed();
}

void QgsLayoutItemChart::setFilterOnlyVisibleFeatures( const bool visibleOnly )
{
  if ( mFilterOnlyVisibleFeatures == visibleOnly )
  {
    return;
  }

  mFilterOnlyVisibleFeatures = visibleOnly;
  refresh();

  emit changed();
}

void QgsLayoutItemChart::setFilterToAtlasFeature( const bool filterToAtlas )
{
  if ( mFilterToAtlasIntersection == filterToAtlas )
  {
    return;
  }

  mFilterToAtlasIntersection = filterToAtlas;
  refresh();

  emit changed();
}

void QgsLayoutItemChart::setSeriesList( const QList<QgsLayoutItemChart::SeriesDetails> &seriesList )
{
  if ( mSeriesList == seriesList )
  {
    return;
  }

  mSeriesList = seriesList;
  refresh();

  emit changed();
}

void QgsLayoutItemChart::draw( QgsLayoutItemRenderContext & )
{
}

void QgsLayoutItemChart::paint( QPainter *painter, const QStyleOptionGraphicsItem *itemStyle, QWidget * )
{
  if ( !mLayout || !painter || !painter->device() )
  {
    return;
  }

  if ( !shouldDrawItem() )
  {
    return;
  }

  if ( !mPlot )
    return;

  QPaintDevice *paintDevice = painter->device();
  if ( !paintDevice )
    return;

  QRectF thisPaintRect = rect();
  if ( qgsDoubleNear( thisPaintRect.width(), 0.0 ) || qgsDoubleNear( thisPaintRect.height(), 0 ) )
    return;

  if ( mLayout->renderContext().isPreviewRender() )
  {
    if ( mNeedsGathering || mIsGathering )
    {
      if ( mNeedsGathering )
      {
        mNeedsGathering = false;
        refreshData();
      }

      QgsScopedQPainterState painterState( painter );
      painter->setClipRect( thisPaintRect );

      painter->setBrush( QBrush( QColor( 125, 125, 125, 125 ) ) );
      painter->drawRect( thisPaintRect );
      painter->setBrush( Qt::NoBrush );
      QFont messageFont;
      messageFont.setPointSize( 12 );
      painter->setFont( messageFont );
      painter->setPen( QColor( 255, 255, 255, 255 ) );
      painter->drawText( thisPaintRect, Qt::AlignCenter | Qt::AlignHCenter, tr( "Rendering chart" ) );
      return;
    }
  }
  else
  {
    if ( mNeedsGathering )
    {
      mNeedsGathering = false;
      prepareGatherer();
      if ( mGatherer )
      {
        QgsApplication::instance()->taskManager()->addTask( mGatherer.data() );
        mGatherer->waitForFinished( 60000 );
      }
    }
  }

  const double scaleFactor = QgsLayoutUtils::scaleFactorFromItemStyle( itemStyle, painter );
  const QSizeF size = mLayout->convertToLayoutUnits( sizeWithUnits() ) * scaleFactor;
  if ( size.width() == 0 || size.height() == 0 )
    return;

  mPlot->setSize( size );

  {
    QgsScopedQPainterState painterState( painter );
    painter->scale( 1 / scaleFactor, 1 / scaleFactor );

    QgsRenderContext renderContext = QgsLayoutUtils::createRenderContextForLayout( mLayout, painter );
    renderContext.setScaleFactor( scaleFactor );
    renderContext.setExpressionContext( createExpressionContext() );

    QgsPlotRenderContext plotRenderContext;
    mPlot->render( renderContext, plotRenderContext, mPlotData );
  }

  if ( mSeriesList.isEmpty() )
  {
    QFont messageFont;
    messageFont.setPointSize( 8 );
    painter->setFont( messageFont );
    painter->setPen( QColor( 125, 125, 125, 125 ) );
    painter->drawText( thisPaintRect, Qt::AlignCenter | Qt::AlignHCenter, tr( "Missing chart data" ) );
  }
}

void QgsLayoutItemChart::refresh()
{
  QgsLayoutItem::refresh();
  if ( mVectorLayer && !mSeriesList.isEmpty() )
  {
    mNeedsGathering = true;
  }
}

void QgsLayoutItemChart::refreshData()
{
  mGathererTimer.start();
}

void QgsLayoutItemChart::gatherData()
{
  prepareGatherer();
  if ( mGatherer )
  {
    QgsApplication::instance()->taskManager()->addTask( mGatherer.data() );
  }

  mIsGathering = true;
  update();
}

void QgsLayoutItemChart::prepareGatherer()
{
  if ( mGatherer )
  {
    disconnect( mGatherer.data(), &QgsTask::taskCompleted, this, &QgsLayoutItemChart::processData );
    mGatherer->cancel();
    mGatherer.clear();
  }

  if ( !mVectorLayer || !mPlot || mSeriesList.isEmpty() )
  {
    mPlotData.clearSeries();
    mIsGathering = false;
    update();
  }

  QgsPlotAbstractMetadata *metadata = QgsApplication::instance()->plotRegistry()->plotMetadata( mPlot->type() );
  if ( !metadata )
  {
    mPlotData.clearSeries();
    mIsGathering = false;
    update();
  }

  if ( !metadata )
  {
    QgsDebugError( "Could not find plot metadata" );
    return;
  }

  mGatherer = metadata->createPlotDataGatherer( mPlot.get() );
  if ( !mGatherer )
  {
    mPlotData.clearSeries();
    mIsGathering = false;
    update();
  }

  if ( QgsVectorLayerXyPlotDataGatherer *xyGatherer = dynamic_cast<QgsVectorLayerXyPlotDataGatherer *>( mGatherer.data() ) )
  {
    QList<QgsVectorLayerXyPlotDataGatherer::XySeriesDetails> xYSeriesList;
    for ( const SeriesDetails &series : mSeriesList )
    {
      xYSeriesList << QgsVectorLayerXyPlotDataGatherer::XySeriesDetails( series.xExpression(), series.yExpression(), series.filterExpression() );
    }

    QgsFeatureRequest request;
    QStringList filterExpressions;
    for ( QgsLayoutItemChart::SeriesDetails &series : mSeriesList )
    {
      if ( !series.filterExpression().isEmpty() )
      {
        filterExpressions << series.filterExpression();
      }
    }
    if ( !filterExpressions.isEmpty() )
    {
      request.setFilterExpression( u"(%1)"_s.arg( filterExpressions.join( ") OR ("_L1 ) ) );
    }

    if ( mSortFeatures && !mSortExpression.isEmpty() )
    {
      request.addOrderBy( mSortExpression, mSortAscending );
    }

    if ( mFilterToAtlasIntersection )
    {
      const QgsGeometry atlasGeometry = mLayout->reportContext().currentGeometry( mVectorLayer->crs() );
      if ( !atlasGeometry.isNull() )
      {
        request.setDistanceWithin( atlasGeometry, 0.0 );
      }
    }
    else if ( mMap && mFilterOnlyVisibleFeatures )
    {
      QgsGeometry visibleRegionGeometry = QgsGeometry::fromQPolygonF( mMap->visibleExtentPolygon() );
      if ( mVectorLayer->crs() != mMap->crs() )
      {
        const QgsCoordinateTransform transform( mVectorLayer->crs(), mMap->crs(), mLayout->project() );
        if ( visibleRegionGeometry.transform( transform ) != Qgis::GeometryOperationResult ::Success )
        {
          visibleRegionGeometry = QgsGeometry();
        }
      }
      if ( !visibleRegionGeometry.isNull() )
      {
        request.setDistanceWithin( visibleRegionGeometry, 0.0 );
      }
    }
    request.setExpressionContext( createExpressionContext() );

    QgsFeatureIterator featureIterator = mVectorLayer->getFeatures( request );

    xyGatherer->setFeatureIterator( featureIterator );
    xyGatherer->setExpressionContext( createExpressionContext() );
    xyGatherer->setSeriesDetails( xYSeriesList );
  }

  connect( mGatherer.data(), &QgsTask::taskCompleted, this, &QgsLayoutItemChart::processData );
}

void QgsLayoutItemChart::processData()
{
  mPlotData = mGatherer->data();
  mGatherer.clear();

  mIsGathering = false;
  update();
}

bool QgsLayoutItemChart::writePropertiesToElement( QDomElement &element, QDomDocument &document, const QgsReadWriteContext &context ) const
{
  if ( mPlot )
  {
    QDomElement plotElement = document.createElement( u"plot"_s );
    mPlot->writeXml( plotElement, document, context );
    element.appendChild( plotElement );
  }

  QDomElement seriesListElement = document.createElement( u"seriesList"_s );
  for ( const SeriesDetails &series : mSeriesList )
  {
    QDomElement seriesElement = document.createElement( u"series"_s );
    seriesElement.setAttribute( u"name"_s, series.name() );
    seriesElement.setAttribute( u"xExpression"_s, series.xExpression() );
    seriesElement.setAttribute( u"yExpression"_s, series.yExpression() );
    seriesElement.setAttribute( u"filterExpression"_s, series.filterExpression() );
    seriesListElement.appendChild( seriesElement );
  }
  element.appendChild( seriesListElement );

  if ( mVectorLayer )
  {
    element.setAttribute( u"vectorLayer"_s, mVectorLayer.layerId );
    element.setAttribute( u"vectorLayerName"_s, mVectorLayer.name );
    element.setAttribute( u"vectorLayerSource"_s, mVectorLayer.source );
    element.setAttribute( u"vectorLayerProvider"_s, mVectorLayer.provider );
  }

  element.setAttribute( u"sortFeatures"_s, mSortFeatures ? u"1"_s : u"0"_s );
  element.setAttribute( u"sortAscending"_s, mSortAscending ? u"1"_s : u"0"_s );
  element.setAttribute( u"sortExpression"_s, mSortExpression );

  element.setAttribute( u"sortExpression"_s, mSortExpression );

  element.setAttribute( u"filterOnlyVisibleFeatures"_s, mFilterOnlyVisibleFeatures );
  element.setAttribute( u"filterToAtlasIntersection"_s, mFilterToAtlasIntersection );

  if ( mMap )
  {
    element.setAttribute( u"mapUuid"_s, mMap->uuid() );
  }

  return true;
}

bool QgsLayoutItemChart::readPropertiesFromElement( const QDomElement &element, const QDomDocument &, const QgsReadWriteContext &context )
{
  QDomElement plotElement = element.firstChildElement( u"plot"_s );
  if ( !plotElement.isNull() )
  {
    mPlot.reset( dynamic_cast<Qgs2DPlot *>( QgsApplication::instance()->plotRegistry()->createPlot( plotElement.attribute( u"plotType"_s ) ) ) );
    if ( mPlot )
    {
      mPlot->readXml( plotElement, context );
    }
  }

  mSeriesList.clear();
  const QDomNodeList seriesNodeList = element.firstChildElement( u"seriesList"_s ).childNodes();
  for ( int i = 0; i < seriesNodeList.count(); i++ )
  {
    const QDomElement seriesElement = seriesNodeList.at( i ).toElement();
    SeriesDetails series( seriesElement.attribute( "name" ) );
    series.setXExpression( seriesElement.attribute( "xExpression" ) );
    series.setYExpression( seriesElement.attribute( "yExpression" ) );
    series.setFilterExpression( seriesElement.attribute( "filterExpression" ) );
    mSeriesList << series;
  }

  QString layerId = element.attribute( u"vectorLayer"_s );
  QString layerName = element.attribute( u"vectorLayerName"_s );
  QString layerSource = element.attribute( u"vectorLayerSource"_s );
  QString layerProvider = element.attribute( u"vectorLayerProvider"_s );
  mVectorLayer = QgsVectorLayerRef( layerId, layerName, layerSource, layerProvider );
  mVectorLayer.resolveWeakly( mLayout->project() );

  mSortFeatures = element.attribute( u"sortFeatures"_s, u"0"_s ).toInt();
  mSortAscending = element.attribute( u"sortAscending"_s, u"1"_s ).toInt();
  mSortExpression = element.attribute( u"sortExpression"_s );

  mFilterOnlyVisibleFeatures = element.attribute( u"filterOnlyVisibleFeatures"_s, u"1"_s ).toInt();
  mFilterToAtlasIntersection = element.attribute( u"filterToAtlasIntersection"_s, u"0"_s ).toInt();

  mMapUuid = element.attribute( u"mapUuid"_s );
  if ( mMap )
  {
    disconnect( mMap, &QgsLayoutItemMap::extentChanged, this, &QgsLayoutItemChart::refresh );
    disconnect( mMap, &QgsLayoutItemMap::mapRotationChanged, this, &QgsLayoutItemChart::refresh );
    mMap = nullptr;
  }

  mNeedsGathering = true;

  return true;
}

void QgsLayoutItemChart::finalizeRestoreFromXml()
{
  if ( !mMap && !mMapUuid.isEmpty() && mLayout )
  {
    mMap = qobject_cast< QgsLayoutItemMap *>( mLayout->itemByUuid( mMapUuid, true ) );
    if ( mMap )
    {
      connect( mMap, &QgsLayoutItemMap::extentChanged, this, &QgsLayoutItemChart::refresh );
      connect( mMap, &QgsLayoutItemMap::mapRotationChanged, this, &QgsLayoutItemChart::refresh );
    }
  }
}
