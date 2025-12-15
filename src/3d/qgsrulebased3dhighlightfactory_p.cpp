/***************************************************************************
  qgsrulebased3dhighlightfactory_p.cpp
  --------------------------------------
  Date                 : December 2025
  Copyright            : (C) 2025 by Jean Felder
  Email                : jean dot felder at oslandia dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsrulebased3dhighlightfactory_p.h"

#include "qgs3drendercontext.h"
#include "qgsfeature3dhandler_p.h"

///@cond PRIVATE

QgsRuleBased3DHighlightFactory::QgsRuleBased3DHighlightFactory( Qgs3DMapSettings *mapSettings, QgsVectorLayer *vLayer, QgsRuleBased3DRenderer::Rule *rootRule )
  : QgsAbstractVectorLayer3DHighlightFactory( mapSettings, vLayer )
  , mRootRule( rootRule->clone() )
{
}

QgsRuleBased3DHighlightFactory::~QgsRuleBased3DHighlightFactory()
{
  qDeleteAll( mHandlers );
  mHandlers.clear();
}

QList<Qt3DCore::QEntity *> QgsRuleBased3DHighlightFactory::create( const QgsFeature &feature, QgsAbstract3DEngine *engine, Qt3DCore::QEntity *parent, const QColor &fillColor, const QColor &edgeColor )
{
  const QgsBox3D box = feature.geometry().constGet()->boundingBox3D();
  QgsVector3D dataOrigin = box.center();
  if ( std::isnan( dataOrigin.z() ) )
  {
    dataOrigin.setZ( 0.0 );
  }

  Qgs3DRenderContext renderContext = Qgs3DRenderContext::fromMapSettings( mMapSettings );

  // replace the symbol of the rule associated with the feature
  for ( QgsRuleBased3DRenderer::Rule *rule : mRootRule->children() )
  {
    if ( rule->isFilterOK( feature, renderContext ) && rule->symbol() )
    {
      rule->setSymbol( prepareSymbol( rule->symbol(), fillColor, edgeColor ).release() );
      break;
    }
  }

  mRootRule->createHandlers( mLayer, mHandlers );

  QSet<QString> attributeNames;
  mRootRule->prepare( renderContext, attributeNames, dataOrigin, mHandlers );
  mRootRule->registerFeature( feature, renderContext, mHandlers );

  QList<Qt3DCore::QEntity *> highlightEntities;
  finalizeEntities( highlightEntities, engine, QgsVector3D() );
  for ( auto it = mHandlers.constBegin(); it != mHandlers.constEnd(); ++it )
  {
    QgsFeature3DHandler *handler = it.value();
    highlightEntities = handler->finalize( parent, renderContext );
    // feature handler found
    if ( !highlightEntities.empty() )
    {
      break;
    }
  }

  finalizeEntities( highlightEntities, engine, dataOrigin );
  return highlightEntities;
}

/// @endcond
