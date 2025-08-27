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
#include "moc_qgslayoutitemchart.cpp"
#include "qgsapplication.h"
#include "qgslayoutitemregistry.h"
#include "qgslayout.h"
#include "qgslayoutrendercontext.h"
#include "qgslayoutreportcontext.h"
#include "qgslayoututils.h"
#include "qgsplotregistry.h"

#include <QDomDocument>
#include <QDomElement>
#include <QPainter>

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
  return QgsApplication::getThemeIcon( QStringLiteral( "/mLayoutItemChart.svg" ) );
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

  // Logic to minimise plot data refresh to bare minimum
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
      if ( oldPlot2dXy && oldPlot2dXy->xAxis().type() != newPlot2dXy->xAxis().type() )
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
      QgsApplication::instance()->taskManager()->addTask( mGatherer.data() );
      mGatherer->waitForFinished( 60000 );
    }
  }

  const double scaleFactor = QgsLayoutUtils::scaleFactorFromItemStyle( itemStyle, painter );
  const QSizeF size = mLayout->convertToLayoutUnits( sizeWithUnits() ) * scaleFactor;
  if ( size.width() == 0 || size.height() == 0 )
    return;

  mPlot->setSize( size );

  QgsRenderContext renderContext = QgsLayoutUtils::createRenderContextForLayout( mLayout, painter );
  renderContext.setExpressionContext( createExpressionContext() );

  QgsScopedQPainterState painterState( painter );
  painter->scale( 1 / scaleFactor, 1 / scaleFactor );

  QgsPlotRenderContext plotRenderContext;
  mPlot->render( renderContext, plotRenderContext, mPlotData );
}

void QgsLayoutItemChart::refresh()
{
  QgsLayoutItem::refresh();
  mNeedsGathering = true;
}

void QgsLayoutItemChart::refreshData()
{
  mGathererTimer.start();
}

void QgsLayoutItemChart::gatherData()
{
  prepareGatherer();
  QgsApplication::instance()->taskManager()->addTask( mGatherer.data() );

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

  QList<QgsVectorLayerXyPlotDataGatherer::XySeriesDetails> xYSeriesList;
  for ( const SeriesDetails &series : mSeriesList )
  {
    xYSeriesList << QgsVectorLayerXyPlotDataGatherer::XySeriesDetails( series.xExpression(), series.yExpression(), series.filterExpression() );
  }

  if ( Qgs2DXyPlot *xyPlot = dynamic_cast<Qgs2DXyPlot *>( mPlot.get() ) )
  {
    QgsFeatureRequest request;
    if ( mSortFeatures && !mSortExpression.isEmpty() )
    {
      request.addOrderBy( mSortExpression, mSortAscending );
    }

    QgsFeatureIterator featureIterator = mVectorLayer->getFeatures( request );
    mGatherer = new QgsVectorLayerXyPlotDataGatherer( featureIterator, createExpressionContext(), xYSeriesList, xyPlot->xAxis().type() );
    connect( mGatherer.data(), &QgsTask::taskCompleted, this, &QgsLayoutItemChart::processData );
  }
  else
  {
    mPlotData.clearSeries();
    mIsGathering = false;
    update();
  }
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
    QDomElement plotElement = document.createElement( QStringLiteral( "plot" ) );
    mPlot->writeXml( plotElement, document, context );
    element.appendChild( plotElement );
  }

  QDomElement seriesListElement = document.createElement( QStringLiteral( "seriesList" ) );
  for ( const SeriesDetails &series : mSeriesList )
  {
    QDomElement seriesElement = document.createElement( QStringLiteral( "series" ) );
    seriesElement.setAttribute( QStringLiteral( "name" ), series.name() );
    seriesElement.setAttribute( QStringLiteral( "xExpression" ), series.xExpression() );
    seriesElement.setAttribute( QStringLiteral( "yExpression" ), series.yExpression() );
    seriesElement.setAttribute( QStringLiteral( "filterExpression" ), series.filterExpression() );
    seriesListElement.appendChild( seriesElement );
  }
  element.appendChild( seriesListElement );

  if ( mVectorLayer )
  {
    element.setAttribute( QStringLiteral( "vectorLayer" ), mVectorLayer.layerId );
    element.setAttribute( QStringLiteral( "vectorLayerName" ), mVectorLayer.name );
    element.setAttribute( QStringLiteral( "vectorLayerSource" ), mVectorLayer.source );
    element.setAttribute( QStringLiteral( "vectorLayerProvider" ), mVectorLayer.provider );
  }

  element.setAttribute( QStringLiteral( "sortFeatures" ), mSortFeatures ? QStringLiteral( "1" ) : QStringLiteral( "0" ) );
  element.setAttribute( QStringLiteral( "sortAscending" ), mSortAscending ? QStringLiteral( "1" ) : QStringLiteral( "0" ) );
  element.setAttribute( QStringLiteral( "sortExpression" ), mSortExpression );

  element.setAttribute( QStringLiteral( "sortExpression" ), mSortExpression );

  return true;
}

bool QgsLayoutItemChart::readPropertiesFromElement( const QDomElement &element, const QDomDocument &, const QgsReadWriteContext &context )
{
  QDomElement plotElement = element.firstChildElement( QStringLiteral( "plot" ) );
  if ( !plotElement.isNull() )
  {
    mPlot.reset( dynamic_cast<Qgs2DPlot *>( QgsApplication::instance()->plotRegistry()->createPlot( plotElement.attribute( QStringLiteral( "plotType" ) ) ) ) );
    if ( mPlot )
    {
      mPlot->readXml( plotElement, context );
    }
  }

  mSeriesList.clear();
  const QDomNodeList seriesNodeList = element.firstChildElement( QStringLiteral( "seriesList" ) ).childNodes();
  for ( int i = 0; i < seriesNodeList.count(); i++ )
  {
    const QDomElement seriesElement = seriesNodeList.at( i ).toElement();
    SeriesDetails series( seriesElement.attribute( "name" ) );
    series.setXExpression( seriesElement.attribute( "xExpression" ) );
    series.setYExpression( seriesElement.attribute( "yExpression" ) );
    series.setFilterExpression( seriesElement.attribute( "filterExpression" ) );
    mSeriesList << series;
  }

  QString layerId = element.attribute( QStringLiteral( "vectorLayer" ) );
  QString layerName = element.attribute( QStringLiteral( "vectorLayerName" ) );
  QString layerSource = element.attribute( QStringLiteral( "vectorLayerSource" ) );
  QString layerProvider = element.attribute( QStringLiteral( "vectorLayerProvider" ) );
  mVectorLayer = QgsVectorLayerRef( layerId, layerName, layerSource, layerProvider );
  mVectorLayer.resolveWeakly( mLayout->project() );

  mSortFeatures = element.attribute( QStringLiteral( "sortFeatures" ), QStringLiteral( "0" ) ).toInt();
  mSortAscending = element.attribute( QStringLiteral( "sortAscending" ), QStringLiteral( "1" ) ).toInt();
  mSortExpression = element.attribute( QStringLiteral( "sortExpression" ) );

  mNeedsGathering = true;

  return true;
}
