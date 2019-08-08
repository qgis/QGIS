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
#include <QMutex>
#include <QMutexLocker>

class QgsGeoPdfRenderedFeatureHandler: public QgsRenderedFeatureHandlerInterface
{
  public:

    QgsGeoPdfRenderedFeatureHandler( QgsLayoutItemMap *map )
      : mMap( map )
      , mMapToLayoutTransform( mMap->transform() )
    {
    }

    void handleRenderedFeature( const QgsFeature &feature, const QgsGeometry &renderedBounds, const QgsRenderedFeatureHandlerInterface::RenderedFeatureContext &context ) override
    {
      // is it a hack retrieving the layer ID from an expression context like this? possibly... BUT
      // the alternative is adding a layer ID member to QgsRenderContext, and that's just asking for people to abuse it
      // and use it to retrieve QgsMapLayers mid-way through a render operation. Lesser of two evils it is!
      const QString layerId = context.renderContext->expressionContext().variable( QStringLiteral( "layer_id" ) ).toString();

      // transform from pixels to map item coordinates
      QTransform pixelToMapItemTransform = QTransform::fromScale( 1.0 / context.renderContext->scaleFactor(), 1.0 / context.renderContext->scaleFactor() );
      QgsGeometry transformed = renderedBounds;
      transformed.transform( pixelToMapItemTransform );
      // transform from map item coordinates to page coordinates
      transformed.transform( mMapToLayoutTransform );

      // we (currently) don't REALLY need a mutex here, because layout maps are always rendered using a single threaded operation.
      // but we'll play it safe, just in case this changes in future.
      QMutexLocker locker( &mMutex );
      renderedFeatures[ layerId ].append( QgsLayoutGeoPdfExporter::RenderedFeature( feature, transformed ) );
    }

    QSet<QString> usedAttributes( QgsVectorLayer *, const QgsRenderContext & ) const override
    {
      return QSet< QString >() << QgsFeatureRequest::ALL_ATTRIBUTES;
    }

    QMap< QString, QVector< QgsLayoutGeoPdfExporter::RenderedFeature > > renderedFeatures;

  private:
    QgsLayoutItemMap *mMap = nullptr;
    QTransform mMapToLayoutTransform;
    QMutex mMutex;
};


QgsLayoutGeoPdfExporter::QgsLayoutGeoPdfExporter( QgsLayout *layout )
  : mLayout( layout )
{
  // on construction, we install a rendered feature handler on layout item maps
  QList< QgsLayoutItemMap * > maps;
  mLayout->layoutItems( maps );
  for ( QgsLayoutItemMap *map : qgis::as_const( maps ) )
  {
    QgsGeoPdfRenderedFeatureHandler *handler = new QgsGeoPdfRenderedFeatureHandler( map );
    mMapHandlers.insert( map, handler );
    map->addRenderedFeatureHandler( handler );
  }
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

QMap<QString, QVector<QgsLayoutGeoPdfExporter::RenderedFeature> > QgsLayoutGeoPdfExporter::renderedFeatures( QgsLayoutItemMap *map ) const
{
  return mMapHandlers.value( map )->renderedFeatures;
}

