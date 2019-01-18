/***************************************************************************
  qgsfeature3dhandler_p.cpp
  --------------------------------------
  Date                 : January 2019
  Copyright            : (C) 2019 by Martin Dobias
  Email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/


#include "qgsfeature3dhandler_p.h"

#include "qgsfeaturerequest.h"
#include "qgsvectorlayer.h"

#include "qgs3dmapsettings.h"
#include "qgs3dutils.h"

/// @cond PRIVATE


namespace Qgs3DSymbolImpl
{

  Qt3DCore::QEntity *entityFromHandler( QgsFeature3DHandler *handler, const Qgs3DMapSettings &map, QgsVectorLayer *layer )
  {
    Qgs3DRenderContext context( map );

    QgsExpressionContext exprContext( Qgs3DUtils::globalProjectLayerExpressionContext( layer ) );
    exprContext.setFields( layer->fields() );
    context.setExpressionContext( exprContext );

    QSet<QString> attributeNames;
    if ( !handler->prepare( context, attributeNames ) )
      return nullptr;

    // build the feature request
    QgsFeatureRequest req;
    req.setDestinationCrs( map.crs(), map.transformContext() );
    req.setSubsetOfAttributes( attributeNames, layer->fields() );

    QgsFeature f;
    QgsFeatureIterator fi = layer->getFeatures( req );
    while ( fi.nextFeature( f ) )
    {
      context.expressionContext().setFeature( f );
      handler->processFeature( f, context );
    }

    Qt3DCore::QEntity *entity = new Qt3DCore::QEntity;
    handler->finalize( entity, context );
    return entity;
  }

}

/// @endcond
