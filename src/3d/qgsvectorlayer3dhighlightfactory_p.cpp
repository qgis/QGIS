/***************************************************************************
  qgsvectorlayer3dhighlightfactory_p.cpp
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

#include "qgsvectorlayer3dhighlightfactory_p.h"

#include <memory>

#include "qgs3drendercontext.h"
#include "qgs3dsymbolregistry.h"
#include "qgsabstract3dengine.h"
#include "qgsabstract3dsymbol.h"
#include "qgsapplication.h"
#include "qgsfeature3dhandler_p.h"

///@cond PRIVATE

QgsVectorLayer3DHighlightFactory::QgsVectorLayer3DHighlightFactory( Qgs3DMapSettings *mapSettings, QgsVectorLayer *vLayer, QgsAbstract3DSymbol *symbol )
  : QgsAbstractVectorLayer3DHighlightFactory( mapSettings, vLayer )
  , mSymbol( symbol )
{
}

QList<Qt3DCore::QEntity *> QgsVectorLayer3DHighlightFactory::create( const QgsFeature &feature, QgsAbstract3DEngine *engine, Qt3DCore::QEntity *parent, const QColor &fillColor, const QColor &edgeColor )
{
  const QgsBox3D box = feature.geometry().constGet()->boundingBox3D();
  QgsVector3D dataOrigin = box.center();
  if ( std::isnan( dataOrigin.z() ) )
  {
    dataOrigin.setZ( 0.0 );
  }

  std::unique_ptr<QgsAbstract3DSymbol> highlightSymbol = prepareSymbol( mSymbol, fillColor, edgeColor );
  std::unique_ptr<QgsFeature3DHandler> feat3DHandler( QgsApplication::symbol3DRegistry()->createHandlerForSymbol( mLayer, highlightSymbol.get() ) );

  const Qgs3DRenderContext renderContext = Qgs3DRenderContext::fromMapSettings( mMapSettings );
  QSet<QString> attributeNames;
  feat3DHandler->prepare( renderContext, attributeNames, dataOrigin );
  feat3DHandler->processFeature( feature, renderContext );
  QList<Qt3DCore::QEntity *> highlightEntities = feat3DHandler->finalize( parent, renderContext );

  finalizeEntities( highlightEntities, engine, dataOrigin );
  return highlightEntities;
}

/// @endcond
