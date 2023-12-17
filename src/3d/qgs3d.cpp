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
#include "qgspointcloudlayer3drenderer.h"
#include "qgstiledscenelayer3drenderer.h"
#include "qgs3dsymbolregistry.h"
#include "qgspoint3dsymbol.h"
#include "qgsline3dsymbol.h"
#include "qgspolygon3dsymbol.h"
#include "qgsmaterialregistry.h"

#include "qgspolygon3dsymbol_p.h"
#include "qgspoint3dsymbol_p.h"
#include "qgsline3dsymbol_p.h"
#include "qgsgoochmaterialsettings.h"
#include "qgssimplelinematerialsettings.h"
#include "qgsphongtexturedmaterialsettings.h"
#include "qgsnullmaterialsettings.h"

#include "qgsstyle.h"

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
  QgsApplication::renderer3DRegistry()->addRenderer( new QgsPointCloudLayer3DRendererMetadata );
  QgsApplication::renderer3DRegistry()->addRenderer( new QgsTiledSceneLayer3DRendererMetadata );

  QgsApplication::symbol3DRegistry()->addSymbolType( new Qgs3DSymbolMetadata( QStringLiteral( "point" ), QObject::tr( "Point" ),
      &QgsPoint3DSymbol::create, nullptr, Qgs3DSymbolImpl::handlerForPoint3DSymbol ) );
  QgsApplication::symbol3DRegistry()->addSymbolType( new Qgs3DSymbolMetadata( QStringLiteral( "line" ), QObject::tr( "Line" ),
      &QgsLine3DSymbol::create, nullptr, Qgs3DSymbolImpl::handlerForLine3DSymbol ) );
  QgsApplication::symbol3DRegistry()->addSymbolType( new Qgs3DSymbolMetadata( QStringLiteral( "polygon" ), QObject::tr( "Polygon" ),
      &QgsPolygon3DSymbol::create, nullptr, Qgs3DSymbolImpl::handlerForPolygon3DSymbol ) );

  Qgs3D::materialRegistry()->addMaterialSettingsType( new QgsMaterialSettingsMetadata( QStringLiteral( "null" ), QObject::tr( "Embedded Textures" ),
      QgsNullMaterialSettings::create, QgsNullMaterialSettings::supportsTechnique, nullptr, QgsApplication::getThemeIcon( QStringLiteral( "/mIconPhongTexturedMaterial.svg" ) ) ) );
  Qgs3D::materialRegistry()->addMaterialSettingsType( new QgsMaterialSettingsMetadata( QStringLiteral( "phong" ), QObject::tr( "Realistic (Phong)" ),
      QgsPhongMaterialSettings::create, QgsPhongMaterialSettings::supportsTechnique, nullptr, QgsApplication::getThemeIcon( QStringLiteral( "/mIconPhongMaterial.svg" ) ) ) );
  Qgs3D::materialRegistry()->addMaterialSettingsType( new QgsMaterialSettingsMetadata( QStringLiteral( "phongtextured" ), QObject::tr( "Realistic Textured (Phong)" ),
      QgsPhongTexturedMaterialSettings::create, QgsPhongTexturedMaterialSettings::supportsTechnique, nullptr, QgsApplication::getThemeIcon( QStringLiteral( "/mIconPhongTexturedMaterial.svg" ) ) ) );
  Qgs3D::materialRegistry()->addMaterialSettingsType( new QgsMaterialSettingsMetadata( QStringLiteral( "simpleline" ), QObject::tr( "Single Color (Unlit)" ),
      QgsSimpleLineMaterialSettings::create, QgsSimpleLineMaterialSettings::supportsTechnique, nullptr, QgsApplication::getThemeIcon( QStringLiteral( "/mIconSimpleLineMaterial.svg" ) ) ) );
  Qgs3D::materialRegistry()->addMaterialSettingsType( new QgsMaterialSettingsMetadata( QStringLiteral( "gooch" ), QObject::tr( "CAD (Gooch)" ),
      QgsGoochMaterialSettings::create, QgsGoochMaterialSettings::supportsTechnique, nullptr, QgsApplication::getThemeIcon( QStringLiteral( "/mIconGoochMaterial.svg" ) ) ) );

  // because we are usually populating the 3d registry AFTER QgsApplication initialization, we need to defer creation
  // of 3d symbols in the default style until now
  QgsStyle::defaultStyle()->handleDeferred3DSymbolCreation();
}

QgsMaterialRegistry *Qgs3D::materialRegistry()
{
  return instance()->mMaterialRegistry;
}

Qgs3D::Qgs3D()
{
  mMaterialRegistry = new QgsMaterialRegistry();
}
