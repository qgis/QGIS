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

class QgsGeoPdfRenderedFeatureHandler: public QgsRenderedFeatureHandlerInterface
{
  public:
    void handleRenderedFeature( const QgsFeature &feature, const QgsGeometry &renderedBounds, const QgsRenderedFeatureHandlerInterface::RenderedFeatureContext &context ) override
    {
      QgsDebugMsg( QStringLiteral( "%1: %2" ).arg( context.renderContext->expressionContext().variable( QStringLiteral( "layer_id" ) ).toString() ).arg( feature.attribute( 0 ).toString() ) );
    }

    QSet<QString> usedAttributes( QgsVectorLayer *, const QgsRenderContext & ) const override
    {
      return QSet< QString >() << QgsFeatureRequest::ALL_ATTRIBUTES;
    }
};


QgsLayoutGeoPdfExporter::QgsLayoutGeoPdfExporter( QgsLayout *layout )
  : mLayout( layout )
  , mHandler( qgis::make_unique< QgsGeoPdfRenderedFeatureHandler >() )
{
  // on construction, we install a rendered feature handler on layout item maps
  mLayout->layoutItems( mMaps );
  for ( QgsLayoutItemMap *map : qgis::as_const( mMaps ) )
  {
    map->addRenderedFeatureHandler( mHandler.get() );
  }
}

QgsLayoutGeoPdfExporter::~QgsLayoutGeoPdfExporter()
{
  // cleanup - remove rendered feature handler from all maps
  for ( QgsLayoutItemMap *map : qgis::as_const( mMaps ) )
  {
    map->removeRenderedFeatureHandler( mHandler.get() );
  }
}

