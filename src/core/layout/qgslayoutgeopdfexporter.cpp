/***************************************************************************
                              qgslayoutgeopdfexporter.cpp
                             --------------------------
    begin                : August 2019
    copyright            : (C) 2019 by Nyall Dawson
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

#include "qgslayoutgeopdfexporter.h"
#include "qgsrenderedfeaturehandlerinterface.h"
#include "qgsfeaturerequest.h"
#include "qgslayout.h"
#include "qgslogger.h"
#include "qgsgeometry.h"
#include "qgsvectorlayer.h"
#include "qgsvectorfilewriter.h"
#include "qgslayertree.h"

#include <gdal.h>
#include "qgsgdalutils.h"
#include "cpl_string.h"
#include "qgslayoutpagecollection.h"

#include <QMutex>
#include <QMutexLocker>
#include <QDomDocument>
#include <QDomElement>

///@cond PRIVATE
class QgsGeoPdfRenderedFeatureHandler: public QgsRenderedFeatureHandlerInterface
{
  public:

    QgsGeoPdfRenderedFeatureHandler( QgsLayoutItemMap *map, QgsLayoutGeoPdfExporter *exporter, const QStringList &layerIds )
      : mExporter( exporter )
      , mMap( map )
      , mLayerIds( layerIds )
    {
      // get page size
      const QgsLayoutSize pageSize = map->layout()->pageCollection()->page( map->page() )->pageSize();
      QSizeF pageSizeLayoutUnits = map->layout()->convertToLayoutUnits( pageSize );
      const QgsLayoutSize pageSizeInches = map->layout()->renderContext().measurementConverter().convert( pageSize, QgsUnitTypes::LayoutInches );

      // PDF assumes 72 dpi -- this is hardcoded!!
      const double pageHeightPdfUnits = pageSizeInches.height() * 72;
      const double pageWidthPdfUnits = pageSizeInches.width() * 72;

      QTransform mapTransform;
      QPolygonF mapRectPoly = QPolygonF( QRectF( 0, 0, map->rect().width(), map->rect().height() ) );
      //workaround QT Bug #21329
      mapRectPoly.pop_back();

      QPolygonF mapRectInLayout = map->mapToScene( mapRectPoly );

      //create transform from layout coordinates to map coordinates
      QTransform::quadToQuad( mapRectPoly, mapRectInLayout, mMapToLayoutTransform );

      // and a transform to PDF coordinate space
      mLayoutToPdfTransform = QTransform::fromTranslate( 0, pageHeightPdfUnits ).scale( pageWidthPdfUnits / pageSizeLayoutUnits.width(),
                              -pageHeightPdfUnits / pageSizeLayoutUnits.height() );
    }

    void handleRenderedFeature( const QgsFeature &feature, const QgsGeometry &renderedBounds, const QgsRenderedFeatureHandlerInterface::RenderedFeatureContext &context ) override
    {
      // is it a hack retrieving the layer ID from an expression context like this? possibly... BUT
      // the alternative is adding a layer ID member to QgsRenderContext, and that's just asking for people to abuse it
      // and use it to retrieve QgsMapLayers mid-way through a render operation. Lesser of two evils it is!
      const QString layerId = context.renderContext.expressionContext().variable( QStringLiteral( "layer_id" ) ).toString();
      if ( !mLayerIds.contains( layerId ) )
        return;

      const QString theme = ( mMap->mExportThemes.isEmpty() || mMap->mExportThemeIt == mMap->mExportThemes.end() ) ? QString() : *mMap->mExportThemeIt;

      // transform from pixels to map item coordinates
      QTransform pixelToMapItemTransform = QTransform::fromScale( 1.0 / context.renderContext.scaleFactor(), 1.0 / context.renderContext.scaleFactor() );
      QgsGeometry transformed = renderedBounds;
      transformed.transform( pixelToMapItemTransform );
      // transform from map item coordinates to page coordinates
      transformed.transform( mMapToLayoutTransform );
      // ...and then to PDF coordinate space
      transformed.transform( mLayoutToPdfTransform );

      // always convert to multitype, to make things consistent
      transformed.convertToMultiType();

      mExporter->pushRenderedFeature( layerId, QgsLayoutGeoPdfExporter::RenderedFeature( feature, transformed ), theme );
    }

    QSet<QString> usedAttributes( QgsVectorLayer *, const QgsRenderContext & ) const override
    {
      return QSet< QString >() << QgsFeatureRequest::ALL_ATTRIBUTES;
    }

  private:
    QTransform mMapToLayoutTransform;
    QTransform mLayoutToPdfTransform;
    QgsLayoutGeoPdfExporter *mExporter = nullptr;
    QgsLayoutItemMap *mMap = nullptr;
    QStringList mLayerIds;
};
///@endcond

QgsLayoutGeoPdfExporter::QgsLayoutGeoPdfExporter( QgsLayout *layout )
  : mLayout( layout )
{
  // build a list of exportable feature layers in advance
  QStringList exportableLayerIds;
  const QMap< QString, QgsMapLayer * > layers = mLayout->project()->mapLayers( true );
  for ( auto it = layers.constBegin(); it != layers.constEnd(); ++it )
  {
    if ( QgsMapLayer *ml = it.value() )
    {
      const QVariant visibility = ml->customProperty( QStringLiteral( "geopdf/initiallyVisible" ), true );
      mInitialLayerVisibility.insert( ml->id(), !visibility.isValid() ? true : visibility.toBool() );
      if ( ml->type() == QgsMapLayerType::VectorLayer )
      {
        const QVariant v = ml->customProperty( QStringLiteral( "geopdf/includeFeatures" ) );
        if ( !v.isValid() || v.toBool() )
        {
          exportableLayerIds << ml->id();
        }
      }

      const QString groupName = ml->customProperty( QStringLiteral( "geopdf/groupName" ) ).toString();
      if ( !groupName.isEmpty() )
        mCustomLayerTreeGroups.insert( ml->id(), groupName );
    }
  }

  // on construction, we install a rendered feature handler on layout item maps
  QList< QgsLayoutItemMap * > maps;
  mLayout->layoutItems( maps );
  for ( QgsLayoutItemMap *map : qgis::as_const( maps ) )
  {
    QgsGeoPdfRenderedFeatureHandler *handler = new QgsGeoPdfRenderedFeatureHandler( map, this, exportableLayerIds );
    mMapHandlers.insert( map, handler );
    map->addRenderedFeatureHandler( handler );
  }

  // start with project layer order, and then apply custom layer order if set
  QStringList geoPdfLayerOrder;
  const QString presetLayerOrder = mLayout->customProperty( QStringLiteral( "pdfLayerOrder" ) ).toString();
  if ( !presetLayerOrder.isEmpty() )
    geoPdfLayerOrder = presetLayerOrder.split( QStringLiteral( "~~~" ) );

  QList< QgsMapLayer * > layerOrder = mLayout->project()->layerTreeRoot()->layerOrder();
  for ( auto it = geoPdfLayerOrder.rbegin(); it != geoPdfLayerOrder.rend(); ++it )
  {
    for ( int i = 0; i < layerOrder.size(); ++i )
    {
      if ( layerOrder.at( i )->id() == *it )
      {
        layerOrder.move( i, 0 );
        break;
      }
    }
  }

  for ( const QgsMapLayer *layer : layerOrder )
    mLayerOrder << layer->id();
}

QgsLayoutGeoPdfExporter::~QgsLayoutGeoPdfExporter()
{
  // cleanup - remove rendered feature handler from all maps
  for ( auto it = mMapHandlers.constBegin(); it != mMapHandlers.constEnd(); ++it )
  {
    it.key()->removeRenderedFeatureHandler( it.value() );
    delete it.value();
  }
}

QgsAbstractGeoPdfExporter::VectorComponentDetail QgsLayoutGeoPdfExporter::componentDetailForLayerId( const QString &layerId )
{
  QgsProject *project = mLayout->project();
  VectorComponentDetail detail;
  const QgsMapLayer *layer = project->mapLayer( layerId );
  detail.name = layer ? layer->name() : layerId;
  detail.mapLayerId = layerId;
  if ( const QgsVectorLayer *vl = qobject_cast< const QgsVectorLayer * >( layer ) )
  {
    detail.displayAttribute = vl->displayField();
  }
  return detail;
}

