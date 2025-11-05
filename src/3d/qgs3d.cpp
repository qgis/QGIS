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

#include "qgs3drendererregistry.h"
#include "qgs3dsymbolregistry.h"
#include "qgs3dterrainregistry.h"
#include "qgsannotationlayer3drenderer.h"
#include "qgsapplication.h"
#include "qgscategorized3drenderer.h"
#include "qgsgoochmaterial3dhandler.h"
#include "qgsline3dsymbol.h"
#include "qgsline3dsymbol_p.h"
#include "qgsmaterialregistry.h"
#include "qgsmeshlayer3drenderer.h"
#include "qgsmetalroughmaterial3dhandler.h"
#include "qgsmetalroughtexturedmaterial3dhandler.h"
#include "qgsnullmaterial3dhandler.h"
#include "qgsphongmaterial3dhandler.h"
#include "qgsphongtexturedmaterial3dhandler.h"
#include "qgspoint3dsymbol.h"
#include "qgspoint3dsymbol_p.h"
#include "qgspointcloudlayer3drenderer.h"
#include "qgspolygon3dsymbol.h"
#include "qgspolygon3dsymbol_p.h"
#include "qgsrulebased3drenderer.h"
#include "qgssettingsentryenumflag.h"
#include "qgssettingsentryimpl.h"
#include "qgssettingstree.h"
#include "qgssimplelinematerial3dhandler.h"
#include "qgsstyle.h"
#include "qgstiledscenelayer3drenderer.h"
#include "qgsvectorlayer3drenderer.h"

#include <QString>

using namespace Qt::StringLiterals;

const QgsSettingsEntryBool *Qgs3D::settingMsaaEnabled = new QgsSettingsEntryBool( u"msaa-enabled"_s, QgsSettingsTree::sTree3DMap, false, u"Whether MSAA is enabled for 3D map rendering"_s );
const QgsSettingsEntryEnumFlag<Qgis::TextureFilterQuality> *Qgs3D::settingTextureFilterQuality
  = new QgsSettingsEntryEnumFlag<Qgis::TextureFilterQuality>( u"texture-filter"_s, QgsSettingsTree::sTree3DMap, Qgis::TextureFilterQuality::Anisotropic16x, u"Texture filter quality"_s );

Qgs3D *Qgs3D::instance()
{
  static Qgs3D *sInstance( new Qgs3D() );
  return sInstance;
}

Qgs3D::~Qgs3D()
{
  QgsMaterialRegistry *materialRegistry = QgsApplication::materialRegistry();
  qgis::down_cast< QgsMaterialSettingsMetadata * >( materialRegistry->materialSettingsMetadata( u"null"_s ) )->setHandler( nullptr );
  qgis::down_cast< QgsMaterialSettingsMetadata * >( materialRegistry->materialSettingsMetadata( u"phong"_s ) )->setHandler( nullptr );
  qgis::down_cast< QgsMaterialSettingsMetadata * >( materialRegistry->materialSettingsMetadata( u"phongtextured"_s ) )->setHandler( nullptr );
  qgis::down_cast< QgsMaterialSettingsMetadata * >( materialRegistry->materialSettingsMetadata( u"simpleline"_s ) )->setHandler( nullptr );
  qgis::down_cast< QgsMaterialSettingsMetadata * >( materialRegistry->materialSettingsMetadata( u"gooch"_s ) )->setHandler( nullptr );
  qgis::down_cast< QgsMaterialSettingsMetadata * >( materialRegistry->materialSettingsMetadata( u"metalrough"_s ) )->setHandler( nullptr );
}

void Qgs3D::initialize()
{
  if ( instance()->mInitialized )
    return;

  instance()->mInitialized = true;

  QgsMaterialRegistry *materialRegistry = QgsApplication::materialRegistry();

  instance()->mNullMaterialHandler = std::make_unique< QgsNullMaterial3DHandler >();
  qgis::down_cast< QgsMaterialSettingsMetadata * >( materialRegistry->materialSettingsMetadata( u"null"_s ) )->setHandler( instance()->mNullMaterialHandler.get() );

  instance()->mPhongMaterialHandler = std::make_unique< QgsPhongMaterial3DHandler >();
  qgis::down_cast< QgsMaterialSettingsMetadata * >( materialRegistry->materialSettingsMetadata( u"phong"_s ) )->setHandler( instance()->mPhongMaterialHandler.get() );

  instance()->mPhongTexturedMaterialHandler = std::make_unique< QgsPhongTexturedMaterial3DHandler >();
  qgis::down_cast< QgsMaterialSettingsMetadata * >( materialRegistry->materialSettingsMetadata( u"phongtextured"_s ) )->setHandler( instance()->mPhongTexturedMaterialHandler.get() );

  instance()->mSimpleLineMaterialHandler = std::make_unique< QgsSimpleLineMaterial3DHandler >();
  qgis::down_cast< QgsMaterialSettingsMetadata * >( materialRegistry->materialSettingsMetadata( u"simpleline"_s ) )->setHandler( instance()->mSimpleLineMaterialHandler.get() );

  instance()->mGoochMaterialHandler = std::make_unique< QgsGoochMaterial3DHandler >();
  qgis::down_cast< QgsMaterialSettingsMetadata * >( materialRegistry->materialSettingsMetadata( u"gooch"_s ) )->setHandler( instance()->mGoochMaterialHandler.get() );

  instance()->mMetalRoughMaterialHandler = std::make_unique< QgsMetalRoughMaterial3DHandler >();
  qgis::down_cast< QgsMaterialSettingsMetadata * >( materialRegistry->materialSettingsMetadata( u"metalrough"_s ) )->setHandler( instance()->mMetalRoughMaterialHandler.get() );

  instance()->mMetalRoughTexturedMaterialHandler = std::make_unique< QgsMetalRoughTexturedMaterial3DHandler >();
  qgis::down_cast< QgsMaterialSettingsMetadata * >( materialRegistry->materialSettingsMetadata( u"metalroughtextured"_s ) )->setHandler( instance()->mMetalRoughTexturedMaterialHandler.get() );

  QgsApplication::renderer3DRegistry()->addRenderer( new QgsVectorLayer3DRendererMetadata );
  QgsApplication::renderer3DRegistry()->addRenderer( new QgsRuleBased3DRendererMetadata );
  QgsApplication::renderer3DRegistry()->addRenderer( new QgsCategorized3DRendererMetadata );
  QgsApplication::renderer3DRegistry()->addRenderer( new QgsMeshLayer3DRendererMetadata );
  QgsApplication::renderer3DRegistry()->addRenderer( new QgsPointCloudLayer3DRendererMetadata );
  QgsApplication::renderer3DRegistry()->addRenderer( new QgsTiledSceneLayer3DRendererMetadata );
  QgsApplication::renderer3DRegistry()->addRenderer( new QgsAnnotationLayer3DRendererMetadata );

  QgsApplication::symbol3DRegistry()->addSymbolType( new Qgs3DSymbolMetadata( u"point"_s, QObject::tr( "Point" ), &QgsPoint3DSymbol::create, nullptr, Qgs3DSymbolImpl::handlerForPoint3DSymbol ) );
  QgsApplication::symbol3DRegistry()->addSymbolType( new Qgs3DSymbolMetadata( u"line"_s, QObject::tr( "Line" ), &QgsLine3DSymbol::create, nullptr, Qgs3DSymbolImpl::handlerForLine3DSymbol ) );
  QgsApplication::symbol3DRegistry()->addSymbolType( new Qgs3DSymbolMetadata( u"polygon"_s, QObject::tr( "Polygon" ), &QgsPolygon3DSymbol::create, nullptr, Qgs3DSymbolImpl::handlerForPolygon3DSymbol ) );


  // because we are usually populating the 3d registry AFTER QgsApplication initialization, we need to defer creation
  // of 3d symbols in the default style until now
  QgsStyle::defaultStyle()->handleDeferred3DSymbolCreation();
}

QgsMaterialRegistry *Qgs3D::materialRegistry()
{
  return QgsApplication::materialRegistry();
}

Qgs3DTerrainRegistry *Qgs3D::terrainRegistry()
{
  return instance()->mTerrainRegistry;
}

const QgsAbstractMaterial3DHandler *Qgs3D::handlerForMaterialSettings( const QgsAbstractMaterialSettings *settings )
{
  if ( !settings )
    return nullptr;

  const QgsAbstractMaterial3DHandler *handler = nullptr;
  if ( const QgsMaterialSettingsMetadata *metadata = dynamic_cast< const QgsMaterialSettingsMetadata * >( QgsApplication::materialRegistry()->materialSettingsMetadata( settings->type() ) ) )
  {
    handler = metadata->handler();
  }
  if ( !handler )
  {
    QgsDebugError( u"No handler registered for %1"_s.arg( settings->type() ) );
  }
  return handler;
}

QgsMaterial *Qgs3D::toMaterial( const QgsAbstractMaterialSettings *settings, Qgis::MaterialRenderingTechnique technique, const QgsMaterialContext &context )
{
  if ( const QgsAbstractMaterial3DHandler *handler = handlerForMaterialSettings( settings ) )
  {
    return handler->toMaterial( settings, technique, context );
  }
  return nullptr;
}

QMap<QString, QString> Qgs3D::toMaterialExportParameters( const QgsAbstractMaterialSettings *settings )
{
  if ( const QgsAbstractMaterial3DHandler *handler = handlerForMaterialSettings( settings ) )
  {
    return handler->toExportParameters( settings );
  }
  return {};
}

void Qgs3D::addMaterialParametersToEffect( Qt3DRender::QEffect *effect, const QgsAbstractMaterialSettings *settings, const QgsMaterialContext &materialContext )
{
  if ( const QgsAbstractMaterial3DHandler *handler = handlerForMaterialSettings( settings ) )
  {
    handler->addParametersToEffect( effect, settings, materialContext );
  }
}

void Qgs3D::applyMaterialDataDefinedToGeometry( const QgsAbstractMaterialSettings *settings, Qt3DCore::QGeometry *geometry, int vertexCount, const QByteArray &dataDefinedBytes )
{
  if ( const QgsAbstractMaterial3DHandler *handler = handlerForMaterialSettings( settings ) )
  {
    handler->applyDataDefinedToGeometry( settings, geometry, vertexCount, dataDefinedBytes );
  }
}

QByteArray Qgs3D::materialDataDefinedVertexColorsAsByte( const QgsAbstractMaterialSettings *settings, const QgsExpressionContext &expressionContext )
{
  if ( const QgsAbstractMaterial3DHandler *handler = handlerForMaterialSettings( settings ) )
  {
    return handler->dataDefinedVertexColorsAsByte( settings, expressionContext );
  }
  return QByteArray();
}

int Qgs3D::materialDataDefinedByteStride( const QgsAbstractMaterialSettings *settings )
{
  if ( const QgsAbstractMaterial3DHandler *handler = handlerForMaterialSettings( settings ) )
  {
    return handler->dataDefinedByteStride( settings );
  }
  return 0;
}

Qgs3D::Qgs3D()
{
  mTerrainRegistry = new Qgs3DTerrainRegistry();
}
