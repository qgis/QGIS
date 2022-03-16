/***************************************************************************
  qgspolygon3dsymbol.cpp
  --------------------------------------
  Date                 : July 2017
  Copyright            : (C) 2017 by Martin Dobias
  Email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgspolygon3dsymbol.h"

#include <Qt3DCore/QEntity>

#include "qgs3dutils.h"
#include "qgssymbollayerutils.h"
#include "qgs3d.h"
#include "qgsmaterialregistry.h"
#include "qgs3dsceneexporter.h"
#include "qgsvectorlayerelevationproperties.h"
#include "qgsvectorlayer.h"

QgsPolygon3DSymbol::QgsPolygon3DSymbol()
  : mMaterial( std::make_unique< QgsPhongMaterialSettings >() )
{

}

QgsPolygon3DSymbol::~QgsPolygon3DSymbol() = default;

QgsAbstract3DSymbol *QgsPolygon3DSymbol::clone() const
{
  std::unique_ptr< QgsPolygon3DSymbol > result = std::make_unique< QgsPolygon3DSymbol >();
  result->mAltClamping = mAltClamping;
  result->mAltBinding = mAltBinding;
  result->mHeight = mHeight;
  result->mExtrusionHeight = mExtrusionHeight;
  result->mMaterial.reset( mMaterial->clone() );
  result->mCullingMode = mCullingMode;
  result->mInvertNormals = mInvertNormals;
  result->mAddBackFaces = mAddBackFaces;
  result->mRenderedFacade = mRenderedFacade;
  result->mEdgesEnabled = mEdgesEnabled;
  result->mEdgeWidth = mEdgeWidth;
  result->mEdgeColor = mEdgeColor;
  copyBaseSettings( result.get() );
  return result.release();
}

void QgsPolygon3DSymbol::writeXml( QDomElement &elem, const QgsReadWriteContext &context ) const
{
  Q_UNUSED( context )

  QDomDocument doc = elem.ownerDocument();

  QDomElement elemDataProperties = doc.createElement( QStringLiteral( "data" ) );
  elemDataProperties.setAttribute( QStringLiteral( "alt-clamping" ), Qgs3DUtils::altClampingToString( mAltClamping ) );
  elemDataProperties.setAttribute( QStringLiteral( "alt-binding" ), Qgs3DUtils::altBindingToString( mAltBinding ) );
  elemDataProperties.setAttribute( QStringLiteral( "height" ), mHeight );
  elemDataProperties.setAttribute( QStringLiteral( "extrusion-height" ), mExtrusionHeight );
  elemDataProperties.setAttribute( QStringLiteral( "culling-mode" ), Qgs3DUtils::cullingModeToString( mCullingMode ) );
  elemDataProperties.setAttribute( QStringLiteral( "invert-normals" ), mInvertNormals ? QStringLiteral( "1" ) : QStringLiteral( "0" ) );
  elemDataProperties.setAttribute( QStringLiteral( "add-back-faces" ), mAddBackFaces ? QStringLiteral( "1" ) : QStringLiteral( "0" ) );
  elemDataProperties.setAttribute( QStringLiteral( "rendered-facade" ), mRenderedFacade );
  elem.appendChild( elemDataProperties );

  elem.setAttribute( QStringLiteral( "material_type" ), mMaterial->type() );
  QDomElement elemMaterial = doc.createElement( QStringLiteral( "material" ) );
  mMaterial->writeXml( elemMaterial, context );
  elem.appendChild( elemMaterial );

  QDomElement elemDDP = doc.createElement( QStringLiteral( "data-defined-properties" ) );
  mDataDefinedProperties.writeXml( elemDDP, propertyDefinitions() );
  elem.appendChild( elemDDP );

  QDomElement elemEdges = doc.createElement( QStringLiteral( "edges" ) );
  elemEdges.setAttribute( QStringLiteral( "enabled" ), mEdgesEnabled ? QStringLiteral( "1" ) : QStringLiteral( "0" ) );
  elemEdges.setAttribute( QStringLiteral( "width" ), mEdgeWidth );
  elemEdges.setAttribute( QStringLiteral( "color" ), QgsSymbolLayerUtils::encodeColor( mEdgeColor ) );
  elem.appendChild( elemEdges );
}

void QgsPolygon3DSymbol::readXml( const QDomElement &elem, const QgsReadWriteContext &context )
{
  Q_UNUSED( context )

  const QDomElement elemDataProperties = elem.firstChildElement( QStringLiteral( "data" ) );
  mAltClamping = Qgs3DUtils::altClampingFromString( elemDataProperties.attribute( QStringLiteral( "alt-clamping" ) ) );
  mAltBinding = Qgs3DUtils::altBindingFromString( elemDataProperties.attribute( QStringLiteral( "alt-binding" ) ) );
  mHeight = elemDataProperties.attribute( QStringLiteral( "height" ) ).toFloat();
  mExtrusionHeight = elemDataProperties.attribute( QStringLiteral( "extrusion-height" ) ).toFloat();
  mCullingMode = Qgs3DUtils::cullingModeFromString( elemDataProperties.attribute( QStringLiteral( "culling-mode" ) ) );
  mInvertNormals = elemDataProperties.attribute( QStringLiteral( "invert-normals" ) ).toInt();
  mAddBackFaces = elemDataProperties.attribute( QStringLiteral( "add-back-faces" ) ).toInt();
  mRenderedFacade = elemDataProperties.attribute( QStringLiteral( "rendered-facade" ), "3" ).toInt();

  const QDomElement elemMaterial = elem.firstChildElement( QStringLiteral( "material" ) );
  const QString materialType = elem.attribute( QStringLiteral( "material_type" ), QStringLiteral( "phong" ) );
  mMaterial.reset( Qgs3D::materialRegistry()->createMaterialSettings( materialType ) );
  if ( !mMaterial )
    mMaterial.reset( Qgs3D::materialRegistry()->createMaterialSettings( QStringLiteral( "phong" ) ) );
  mMaterial->readXml( elemMaterial, context );

  const QDomElement elemDDP = elem.firstChildElement( QStringLiteral( "data-defined-properties" ) );
  if ( !elemDDP.isNull() )
    mDataDefinedProperties.readXml( elemDDP, propertyDefinitions() );

  const QDomElement elemEdges = elem.firstChildElement( QStringLiteral( "edges" ) );
  if ( !elemEdges.isNull() )
  {
    mEdgesEnabled = elemEdges.attribute( QStringLiteral( "enabled" ) ).toInt();
    mEdgeWidth = elemEdges.attribute( QStringLiteral( "width" ) ).toFloat();
    mEdgeColor = QgsSymbolLayerUtils::decodeColor( elemEdges.attribute( QStringLiteral( "color" ) ) );
  }
}

QList<QgsWkbTypes::GeometryType> QgsPolygon3DSymbol::compatibleGeometryTypes() const
{
  return QList< QgsWkbTypes::GeometryType >() << QgsWkbTypes::PolygonGeometry;
}

void QgsPolygon3DSymbol::setDefaultPropertiesFromLayer( const QgsVectorLayer *layer )
{
  const QgsVectorLayerElevationProperties *props = qgis::down_cast< const QgsVectorLayerElevationProperties * >( const_cast< QgsVectorLayer *>( layer )->elevationProperties() );

  mAltClamping = props->clamping();
  mAltBinding = props->binding();
  mExtrusionHeight = props->extrusionEnabled() ? static_cast< float>( props->extrusionHeight() ) : 0.0f;
  mHeight = static_cast< float >( props->zOffset() );
}

QgsAbstract3DSymbol *QgsPolygon3DSymbol::create()
{
  return new QgsPolygon3DSymbol();
}

QgsAbstractMaterialSettings *QgsPolygon3DSymbol::material() const
{
  return mMaterial.get();
}

void QgsPolygon3DSymbol::setMaterial( QgsAbstractMaterialSettings *material )
{
  if ( material == mMaterial.get() )
    return;

  mMaterial.reset( material );
}

bool QgsPolygon3DSymbol::exportGeometries( Qgs3DSceneExporter *exporter, Qt3DCore::QEntity *entity, const QString &objectNamePrefix ) const
{
  const QList<Qt3DRender::QGeometryRenderer *> renderers = entity->findChildren<Qt3DRender::QGeometryRenderer *>();
  for ( Qt3DRender::QGeometryRenderer *r : renderers )
  {
    Qgs3DExportObject *object = exporter->processGeometryRenderer( r, objectNamePrefix );
    if ( object == nullptr ) continue;
    exporter->processEntityMaterial( entity, object );
    exporter->mObjects.push_back( object );
  }
  return renderers.size() != 0;
}
