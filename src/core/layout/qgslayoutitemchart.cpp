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
#include "qgsbarchartplot.h"
#include "qgscategorizedsymbolrenderer.h"
#include "qgsfillsymbol.h"
#include "qgsgraduatedsymbolrenderer.h"
#include "qgslayout.h"
#include "qgslayoutitemregistry.h"
#include "qgslayoutrendercontext.h"
#include "qgslayoutreportcontext.h"
#include "qgslayoututils.h"
#include "qgslinechartplot.h"
#include "qgsmarkersymbol.h"
#include "qgspiechartplot.h"
#include "qgsplotregistry.h"
#include "qgspointdistancerenderer.h"
#include "qgsrenderer.h"
#include "qgsrulebasedrenderer.h"
#include "qgssymbollayer.h"

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

  SeriesDetails series( tr( "Series 1" ) );
  mSeriesList << series;

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

void QgsLayoutItemChart::setGenerateCategoriesFromRenderer( bool generateCategoriesFromRenderer )
{
  if ( mGenerateCategoriesFromRenderer == generateCategoriesFromRenderer )
  {
    return;
  }

  mGenerateCategoriesFromRenderer = generateCategoriesFromRenderer;
  refresh();

  emit changed();
}

void QgsLayoutItemChart::setApplyRendererStyle( bool applyRendererStyle )
{
  if ( mApplyRendererStyle == applyRendererStyle )
  {
    return;
  }

  mApplyRendererStyle = applyRendererStyle;

  if ( mGenerateCategoriesFromRenderer )
  {
    refresh();
  }

  emit changed();
}

void QgsLayoutItemChart::setSeriesList( const QList<QgsLayoutItemChart::SeriesDetails> &seriesList )
{
  if ( mSeriesList == seriesList )
  {
    return;
  }

  mSeriesList = seriesList;
  mPlotData = QgsPlotData();
  refresh();

  emit changed();
}

void QgsLayoutItemChart::draw( QgsLayoutItemRenderContext & )
{}

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
  {
    return;
  }

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

  Qgs2DPlot *plot = mPlot.get();

  std::unique_ptr< Qgs2DPlot > newPlot;
  if ( mGenerateCategoriesFromRenderer && mVectorLayer )
  {
    newPlot.reset( dynamic_cast<Qgs2DPlot *>( QgsApplication::plotRegistry()->createPlot( mPlot->type() ) ) );
    plot = newPlot.get();
    plot->initFromPlot( mPlot.get() );
    if ( Qgs2DXyPlot *plotXy = dynamic_cast<Qgs2DXyPlot *>( plot ) )
    {
      plotXy->xAxis().setType( Qgis::PlotAxisType::Categorical );
    }

    if ( mApplyRendererStyle && mVectorLayer->renderer() )
    {
      applyRendererStyleToPlot( plot );
    }
  }

  plot->setSize( size );
  {
    QgsScopedQPainterState painterState( painter );
    painter->scale( 1 / scaleFactor, 1 / scaleFactor );

    QgsRenderContext renderContext = QgsLayoutUtils::createRenderContextForLayout( mLayout, painter );
    renderContext.setScaleFactor( scaleFactor );
    renderContext.setExpressionContext( createExpressionContext() );

    QgsPlotRenderContext plotRenderContext;
    plot->render( renderContext, plotRenderContext, mPlotData );
  }

  if ( mSeriesList.isEmpty() || ( mSeriesList.size() == 1 && !mGenerateCategoriesFromRenderer && ( mSeriesList[0].xExpression().isEmpty() || mSeriesList[0].yExpression().isEmpty() ) ) )
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
    if ( mGenerateCategoriesFromRenderer && mVectorLayer->renderer() )
    {
      QString rendererXExpression;
      QStringList rendererCategories;
      createRendererSeriesDetails( rendererXExpression, rendererCategories );

      xyGatherer->setXAxisType( Qgis::PlotAxisType::Categorical );
      xyGatherer->setPredefinedCategories( rendererCategories );
      for ( const SeriesDetails &series : mSeriesList )
      {
        xYSeriesList << QgsVectorLayerXyPlotDataGatherer::XySeriesDetails( series.name(), rendererXExpression, !series.yExpression().isEmpty() ? series.yExpression() : "1"_L1, series.filterExpression() );
      }
    }
    else
    {
      for ( const SeriesDetails &series : mSeriesList )
      {
        xYSeriesList << QgsVectorLayerXyPlotDataGatherer::XySeriesDetails( series.name(), series.xExpression(), series.yExpression(), series.filterExpression() );
      }
    }
    xyGatherer->setSeriesDetails( xYSeriesList );

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

  if ( mGenerateCategoriesFromRenderer )
  {
    element.setAttribute( u"generateCategoriesFromRenderer"_s, mGenerateCategoriesFromRenderer );
  }
  if ( mApplyRendererStyle )
  {
    element.setAttribute( u"applyRendererStyle"_s, mApplyRendererStyle );
  }

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

  mGenerateCategoriesFromRenderer = element.attribute( u"generateCategoriesFromRenderer"_s, u"0"_s ).toInt();
  mApplyRendererStyle = element.attribute( u"applyRendererStyle"_s, u"1"_s ).toInt();

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

void QgsLayoutItemChart::createRendererSeriesDetails( QString &rendererXExpression, QStringList &rendererCategories )
{
  const QgsFeatureRenderer *renderer = mVectorLayer->renderer();
  if ( const QgsFeatureRenderer *embeddedRenderer = renderer->embeddedRenderer() )
  {
    renderer = embeddedRenderer;
  }

  QStringList expressionCases;
  if ( const QgsCategorizedSymbolRenderer *categorizedRenderer = dynamic_cast<const QgsCategorizedSymbolRenderer *>( renderer ) )
  {
    const QgsCategoryList categories = categorizedRenderer->categories();
    bool ok;
    for ( const QgsRendererCategory &category : categories )
    {
      rendererCategories << category.label();
      expressionCases << u"WHEN %1 THEN %2"_s.arg( renderer->legendKeyToExpression( category.uuid(), mVectorLayer.get(), ok ), QgsExpression::quotedString( category.label() ) );
    }
  }
  else if ( const QgsGraduatedSymbolRenderer *graduatedRenderer = dynamic_cast<const QgsGraduatedSymbolRenderer *>( renderer ) )
  {
    const QgsRangeList ranges = graduatedRenderer->ranges();
    bool ok;
    for ( const QgsRendererRange &range : ranges )
    {
      rendererCategories << range.label();
      expressionCases << u"WHEN %1 THEN %2"_s.arg( renderer->legendKeyToExpression( range.uuid(), mVectorLayer.get(), ok ), QgsExpression::quotedString( range.label() ) );
    }
  }
  else if ( const QgsRuleBasedRenderer *ruleBasedRenderer = dynamic_cast<const QgsRuleBasedRenderer *>( renderer ) )
  {
    bool proceed = true;
    bool hasElse = false;
    QString elseLabel;

    const QList< QgsRuleBasedRenderer::Rule * > rules = const_cast< QgsRuleBasedRenderer * >( ruleBasedRenderer )->rootRule()->children();
    for ( const QgsRuleBasedRenderer::Rule *rule : rules )
    {
      if ( rule->hasActiveChildren() )
      {
        // We do not support multi-level rules configuration
        proceed = false;
        break;
      }

      if ( rule->isElse() )
      {
        hasElse = true;
        elseLabel = rule->label();
        rendererCategories << rule->label();
        continue;
      }

      rendererCategories << rule->label();
      expressionCases << u"WHEN %1 THEN %2"_s.arg( rule->filterExpression(), QgsExpression::quotedString( rule->label() ) );
    }

    if ( proceed )
    {
      if ( hasElse )
      {
        expressionCases << u"ELSE %1"_s.arg( QgsExpression::quotedString( elseLabel ) );
      }
    }
    else
    {
      rendererCategories.clear();
      expressionCases.clear();
    }
  }

  rendererXExpression = !expressionCases.isEmpty() ? u"CASE %1 END"_s.arg( expressionCases.join( ' ' ) ) : QString();
}

void QgsLayoutItemChart::applyRendererStyleToPlot( Qgs2DPlot *plot ) const
{
  Q_ASSERT( plot != mPlot.get() );

  const QgsFeatureRenderer *renderer = mVectorLayer->renderer();
  if ( const QgsFeatureRenderer *embeddedRenderer = renderer->embeddedRenderer() )
  {
    renderer = embeddedRenderer;
  }

  QStringList expressionCases;
  if ( const QgsCategorizedSymbolRenderer *categorizedRenderer = dynamic_cast<const QgsCategorizedSymbolRenderer *>( renderer ) )
  {
    const QgsCategoryList categories = categorizedRenderer->categories();
    for ( const QgsRendererCategory &category : categories )
    {
      const QColor color = category.symbol() ? category.symbol()->color() : QColor( 90, 90, 90 );
      expressionCases << u"WHEN @chart_category = %1 THEN color_rgbf(%2, %3, %4, %5)"_s.arg( QgsExpression::quotedString( category.label() ) )
                           .arg( color.redF() )
                           .arg( color.greenF() )
                           .arg( color.blueF() )
                           .arg( color.alphaF() );
    }
  }
  else if ( const QgsGraduatedSymbolRenderer *graduatedRenderer = dynamic_cast<const QgsGraduatedSymbolRenderer *>( renderer ) )
  {
    const QgsRangeList ranges = graduatedRenderer->ranges();
    for ( const QgsRendererRange &range : ranges )
    {
      const QColor color = range.symbol() ? range.symbol()->color() : QColor( 90, 90, 90 );
      expressionCases << u"WHEN @chart_category = %1 THEN color_rgbf(%2, %3, %4, %5)"_s.arg( QgsExpression::quotedString( range.label() ) )
                           .arg( color.redF() )
                           .arg( color.greenF() )
                           .arg( color.blueF() )
                           .arg( color.alphaF() );
    }
  }
  else if ( const QgsRuleBasedRenderer *ruleBasedRenderer = dynamic_cast<const QgsRuleBasedRenderer *>( renderer ) )
  {
    bool proceed = true;
    bool hasElse = false;
    QColor elseColor;

    const QList< QgsRuleBasedRenderer::Rule * > rules = const_cast< QgsRuleBasedRenderer * >( ruleBasedRenderer )->rootRule()->children();
    for ( QgsRuleBasedRenderer::Rule *rule : rules )
    {
      if ( rule->hasActiveChildren() )
      {
        // We do not support multi-level rules configuration
        proceed = false;
        break;
      }

      if ( rule->isElse() )
      {
        hasElse = true;
        elseColor = rule->symbol() ? rule->symbol()->color() : QColor( 90, 90, 90 );
        continue;
      }

      const QColor color = rule->symbol() ? rule->symbol()->color() : QColor( 90, 90, 90 );
      expressionCases << u"WHEN @chart_category = %1 THEN color_rgbf(%2, %3, %4, %5)"_s.arg( QgsExpression::quotedString( rule->label() ) )
                           .arg( color.redF() )
                           .arg( color.greenF() )
                           .arg( color.blueF() )
                           .arg( color.alphaF() );
    }

    if ( proceed )
    {
      if ( hasElse )
      {
        expressionCases << u"ELSE color_rgbf(%1, %2, %3, %4)"_s.arg( elseColor.redF() ).arg( elseColor.greenF() ).arg( elseColor.blueF() ).arg( elseColor.alphaF() );
      }
    }
    else
    {
      expressionCases.clear();
    }
  }

  if ( !expressionCases.isEmpty() )
  {
    const QString rendererColorExpression = u"CASE %1 END"_s.arg( expressionCases.join( ' ' ) );
    if ( QgsBarChartPlot *barChartPlot = dynamic_cast<QgsBarChartPlot *>( plot ) )
    {
      for ( int idx = 0; idx < barChartPlot->fillSymbolCount(); idx++ )
      {
        QgsFillSymbol *fillSymbol = barChartPlot->fillSymbolAt( idx );
        for ( QgsSymbolLayer *symbolLayer : fillSymbol->symbolLayers() )
        {
          symbolLayer->setDataDefinedProperty( QgsSymbolLayer::Property::FillColor, QgsProperty::fromExpression( rendererColorExpression, true ) );
        }
      }
    }
    else if ( QgsLineChartPlot *lineChartPlot = dynamic_cast<QgsLineChartPlot *>( plot ) )
    {
      for ( int idx = 0; idx < lineChartPlot->markerSymbolCount(); idx++ )
      {
        QgsMarkerSymbol *markerSymbol = lineChartPlot->markerSymbolAt( idx );
        for ( QgsSymbolLayer *symbolLayer : markerSymbol->symbolLayers() )
        {
          symbolLayer->setDataDefinedProperty( QgsSymbolLayer::Property::FillColor, QgsProperty::fromExpression( rendererColorExpression, true ) );
        }
      }
    }
    else if ( QgsPieChartPlot *pieChartPlot = dynamic_cast<QgsPieChartPlot *>( plot ) )
    {
      for ( int idx = 0; idx < pieChartPlot->fillSymbolCount(); idx++ )
      {
        QgsFillSymbol *fillSymbol = pieChartPlot->fillSymbolAt( idx );
        for ( QgsSymbolLayer *symbolLayer : fillSymbol->symbolLayers() )
        {
          symbolLayer->setDataDefinedProperty( QgsSymbolLayer::Property::FillColor, QgsProperty::fromExpression( rendererColorExpression, true ) );
        }
      }
    }
  }
}
