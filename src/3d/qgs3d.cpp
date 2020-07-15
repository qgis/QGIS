/***************************************************************************
                         qgs3d.cpp
                         ----------
    begin                : July 2020
    copyright            : (C) 2020 by Nyall Dawson
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

#include "qgs3d.h"

#include "qgsapplication.h"
#include "qgs3drendererregistry.h"

#include "qgsabstract3drenderer.h"
#include "qgs3drendererregistry.h"
#include "qgsrulebased3drenderer.h"
#include "qgsvectorlayer3drenderer.h"
#include "qgsmeshlayer3drenderer.h"
#include "qgs3dsymbolregistry.h"
#include "qgspoint3dsymbol.h"
#include "qgsline3dsymbol.h"
#include "qgspolygon3dsymbol.h"

Qgs3D *Qgs3D::instance()
{
  static Qgs3D *sInstance( new Qgs3D() );
  return sInstance;
}

Qgs3D::~Qgs3D()
{
}

void Qgs3D::initialize()
{
  if ( instance()->mInitialized )
    return;

  instance()->mInitialized = true;

  QgsApplication::renderer3DRegistry()->addRenderer( new QgsVectorLayer3DRendererMetadata );
  QgsApplication::renderer3DRegistry()->addRenderer( new QgsRuleBased3DRendererMetadata );
  QgsApplication::renderer3DRegistry()->addRenderer( new QgsMeshLayer3DRendererMetadata );

  QgsApplication::symbol3DRegistry()->addSymbolType( new Qgs3DSymbolMetadata( QStringLiteral( "point" ), QObject::tr( "Point" ),
      &QgsPoint3DSymbol::create ) );
  QgsApplication::symbol3DRegistry()->addSymbolType( new Qgs3DSymbolMetadata( QStringLiteral( "line" ), QObject::tr( "Line" ),
      &QgsLine3DSymbol::create ) );
  QgsApplication::symbol3DRegistry()->addSymbolType( new Qgs3DSymbolMetadata( QStringLiteral( "polygon" ), QObject::tr( "Polygon" ),
      &QgsPolygon3DSymbol::create ) );

}

Qgs3D::Qgs3D()
{
}
