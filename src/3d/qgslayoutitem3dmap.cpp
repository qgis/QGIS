/***************************************************************************
  qgslayoutitem3dmap.cpp
  --------------------------------------
  Date                 : August 2018
  Copyright            : (C) 2018 by Martin Dobias
  Email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgslayoutitem3dmap.h"

#include "qgs3dmapscene.h"
#include "qgs3dutils.h"
#include "qgscameracontroller.h"
#include "qgslayout.h"
#include "qgslayoutitemregistry.h"
#include "qgsoffscreen3dengine.h"


QgsLayoutItem3DMap::QgsLayoutItem3DMap( QgsLayout *layout )
  : QgsLayoutItem( layout )
{

}

QgsLayoutItem3DMap *QgsLayoutItem3DMap::create( QgsLayout *layout )
{
  return new QgsLayoutItem3DMap( layout );
}

int QgsLayoutItem3DMap::type() const
{
  return QgsLayoutItemRegistry::Layout3DMap;
}

void QgsLayoutItem3DMap::draw( QgsLayoutItemRenderContext &context )
{
  QgsOffscreen3DEngine engine;
  QSizeF sizePixels = mLayout->renderContext().measurementConverter().convert( sizeWithUnits(), QgsUnitTypes::LayoutPixels ).toQSizeF();
  engine.setSize( QSize( static_cast<int>( std::ceil( sizePixels.width() ) ),
                         static_cast<int>( std::ceil( sizePixels.height() ) ) ) );

  Qgs3DMapScene *scene = new Qgs3DMapScene( *mSettings, &engine );
  engine.setRootEntity( scene );

  scene->cameraController()->setCameraPose( mCameraPose );

  // XXX this should not be needed, but without it the scene often
  // is not completely ready (e.g. a missing terrain tile).
  // leaving it here until a more robust solution is found...
  Qgs3DUtils::captureSceneImage( engine, scene );

  QImage img = Qgs3DUtils::captureSceneImage( engine, scene );

  QgsRenderContext &ctx = context.renderContext();
  ctx.painter()->drawImage( 0, 0, img );
}

void QgsLayoutItem3DMap::setMapSettings( Qgs3DMapSettings *settings )
{
  mSettings.reset( settings );
}
