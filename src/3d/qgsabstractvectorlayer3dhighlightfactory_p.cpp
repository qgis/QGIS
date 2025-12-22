/***************************************************************************
  qgsabstractvectorlayer3dhighlightfactory_p.cpp
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

#include "qgsabstractvectorlayer3dhighlightfactory_p.h"

#include "qgs3dmapsettings.h"
#include "qgsabstract3dengine.h"
#include "qgsgeotransform.h"
#include "qgsline3dsymbol.h"
#include "qgslinematerial_p.h"
#include "qgsphongmaterialsettings.h"
#include "qgspoint3dsymbol.h"
#include "qgspolygon3dsymbol.h"
#include "qgsvectorlayer.h"

///@cond PRIVATE

QgsAbstractVectorLayer3DHighlightFactory::QgsAbstractVectorLayer3DHighlightFactory( Qgs3DMapSettings *mapSettings, QgsVectorLayer *vLayer )
  : mMapSettings( mapSettings )
  , mLayer( vLayer )
{
}

std::unique_ptr<QgsAbstract3DSymbol> QgsAbstractVectorLayer3DHighlightFactory::prepareSymbol( const QgsAbstract3DSymbol *symbol, const QColor &fillColor, const QColor &edgeColor )
{
  if ( !symbol )
  {
    return nullptr;
  }

  std::unique_ptr<QgsPhongMaterialSettings> phong( static_cast<QgsPhongMaterialSettings *>( QgsPhongMaterialSettings::create() ) );
  phong->setAmbient( fillColor );
  phong->setDiffuse( Qt::darkGray );
  phong->setOpacity( 1.0f );

  std::unique_ptr<QgsAbstract3DSymbol> clonedSymbol( symbol->clone() );
  if ( QgsPolygon3DSymbol *polygonSymbol = dynamic_cast<QgsPolygon3DSymbol *>( clonedSymbol.get() ) )
  {
    polygonSymbol->setMaterialSettings( phong.release() );
    polygonSymbol->setAddBackFaces( false );
    polygonSymbol->setCullingMode( Qgs3DTypes::CullingMode::NoCulling );
    polygonSymbol->setEdgesEnabled( true );
    polygonSymbol->setEdgeColor( edgeColor );
    polygonSymbol->setEdgeWidth( 2.0f );
  }
  else if ( QgsPoint3DSymbol *pointSymbol = dynamic_cast<QgsPoint3DSymbol *>( clonedSymbol.get() ) )
  {
    pointSymbol->setMaterialSettings( phong.release() );
  }
  else if ( QgsLine3DSymbol *lineSymbol = dynamic_cast<QgsLine3DSymbol *>( clonedSymbol.get() ) )
  {
    lineSymbol->setMaterialSettings( phong.release() );
  }

  return clonedSymbol;
}

void QgsAbstractVectorLayer3DHighlightFactory::finalizeEntities( QList<Qt3DCore::QEntity *> newEntities, QgsAbstract3DEngine *engine, const QgsVector3D &dataOrigin )
{
  for ( auto *childEntity : newEntities )
  {
    for ( QgsGeoTransform *transform : childEntity->findChildren<QgsGeoTransform *>() )
    {
      transform->setGeoTranslation( dataOrigin );
      transform->setOrigin( mMapSettings->origin() );
    }

    for ( QgsLineMaterial *lm : childEntity->findChildren<QgsLineMaterial *>() )
    {
      QObject::connect( engine, &QgsAbstract3DEngine::sizeChanged, lm, [lm, engine] {
        lm->setViewportSize( engine->size() );
      } );

      lm->setViewportSize( engine->size() );
    }
  }
}

/// @endcond
