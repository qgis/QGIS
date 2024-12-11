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
#include "qgs3d.h"
#include "qgscolorutils.h"
#include "qgsmaterialregistry.h"
#include "qgs3dsceneexporter.h"
#include "qgsvectorlayerelevationproperties.h"
#include "qgsvectorlayer.h"
#include "qgstessellatedpolygongeometry.h"

QgsPolygon3DSymbol::QgsPolygon3DSymbol()
  : mMaterialSettings( std::make_unique<QgsPhongMaterialSettings>() )
{
}

QgsPolygon3DSymbol::~QgsPolygon3DSymbol() = default;

QgsAbstract3DSymbol *QgsPolygon3DSymbol::clone() const
{
  std::unique_ptr<QgsPolygon3DSymbol> result = std::make_unique<QgsPolygon3DSymbol>();
  result->mAltClamping = mAltClamping;
  result->mAltBinding = mAltBinding;
  result->mOffset = mOffset;
  result->mExtrusionHeight = mExtrusionHeight;
  result->mMaterialSettings.reset( mMaterialSettings->clone() );
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
  elemDataProperties.setAttribute( QStringLiteral( "offset" ), mOffset );
  elemDataProperties.setAttribute( QStringLiteral( "extrusion-height" ), mExtrusionHeight );
  elemDataProperties.setAttribute( QStringLiteral( "culling-mode" ), Qgs3DUtils::cullingModeToString( mCullingMode ) );
  elemDataProperties.setAttribute( QStringLiteral( "invert-normals" ), mInvertNormals ? QStringLiteral( "1" ) : QStringLiteral( "0" ) );
  elemDataProperties.setAttribute( QStringLiteral( "add-back-faces" ), mAddBackFaces ? QStringLiteral( "1" ) : QStringLiteral( "0" ) );
  elemDataProperties.setAttribute( QStringLiteral( "rendered-facade" ), mRenderedFacade );
  elem.appendChild( elemDataProperties );

  elem.setAttribute( QStringLiteral( "material_type" ), mMaterialSettings->type() );
  QDomElement elemMaterial = doc.createElement( QStringLiteral( "material" ) );
  mMaterialSettings->writeXml( elemMaterial, context );
  elem.appendChild( elemMaterial );

  QDomElement elemDDP = doc.createElement( QStringLiteral( "data-defined-properties" ) );
  mDataDefinedProperties.writeXml( elemDDP, propertyDefinitions() );
  elem.appendChild( elemDDP );

  QDomElement elemEdges = doc.createElement( QStringLiteral( "edges" ) );
  elemEdges.setAttribute( QStringLiteral( "enabled" ), mEdgesEnabled ? QStringLiteral( "1" ) : QStringLiteral( "0" ) );
  elemEdges.setAttribute( QStringLiteral( "width" ), mEdgeWidth );
  elemEdges.setAttribute( QStringLiteral( "color" ), QgsColorUtils::colorToString( mEdgeColor ) );
  elem.appendChild( elemEdges );
}

void QgsPolygon3DSymbol::readXml( const QDomElement &elem, const QgsReadWriteContext &context )
{
  Q_UNUSED( context )

  const QDomElement elemDataProperties = elem.firstChildElement( QStringLiteral( "data" ) );
  mAltClamping = Qgs3DUtils::altClampingFromString( elemDataProperties.attribute( QStringLiteral( "alt-clamping" ) ) );
  mAltBinding = Qgs3DUtils::altBindingFromString( elemDataProperties.attribute( QStringLiteral( "alt-binding" ) ) );
  mOffset = elemDataProperties.attribute( QStringLiteral( "offset" ) ).toFloat();
  mExtrusionHeight = elemDataProperties.attribute( QStringLiteral( "extrusion-height" ) ).toFloat();
  mCullingMode = Qgs3DUtils::cullingModeFromString( elemDataProperties.attribute( QStringLiteral( "culling-mode" ) ) );
  mInvertNormals = elemDataProperties.attribute( QStringLiteral( "invert-normals" ) ).toInt();
  mAddBackFaces = elemDataProperties.attribute( QStringLiteral( "add-back-faces" ) ).toInt();
  mRenderedFacade = elemDataProperties.attribute( QStringLiteral( "rendered-facade" ), "3" ).toInt();

  const QDomElement elemMaterial = elem.firstChildElement( QStringLiteral( "material" ) );
  const QString materialType = elem.attribute( QStringLiteral( "material_type" ), QStringLiteral( "phong" ) );
  mMaterialSettings.reset( Qgs3D::materialRegistry()->createMaterialSettings( materialType ) );
  if ( !mMaterialSettings )
    mMaterialSettings.reset( Qgs3D::materialRegistry()->createMaterialSettings( QStringLiteral( "phong" ) ) );
  mMaterialSettings->readXml( elemMaterial, context );

  const QDomElement elemDDP = elem.firstChildElement( QStringLiteral( "data-defined-properties" ) );
  if ( !elemDDP.isNull() )
    mDataDefinedProperties.readXml( elemDDP, propertyDefinitions() );

  const QDomElement elemEdges = elem.firstChildElement( QStringLiteral( "edges" ) );
  if ( !elemEdges.isNull() )
  {
    mEdgesEnabled = elemEdges.attribute( QStringLiteral( "enabled" ) ).toInt();
    mEdgeWidth = elemEdges.attribute( QStringLiteral( "width" ) ).toFloat();
    mEdgeColor = QgsColorUtils::colorFromString( elemEdges.attribute( QStringLiteral( "color" ) ) );
  }
}

QList<Qgis::GeometryType> QgsPolygon3DSymbol::compatibleGeometryTypes() const
{
  return QList<Qgis::GeometryType>() << Qgis::GeometryType::Polygon;
}

void QgsPolygon3DSymbol::setDefaultPropertiesFromLayer( const QgsVectorLayer *layer )
{
  const QgsVectorLayerElevationProperties *props = qgis::down_cast<const QgsVectorLayerElevationProperties *>( const_cast<QgsVectorLayer *>( layer )->elevationProperties() );

  mAltClamping = props->clamping();
  mAltBinding = props->binding();
  mExtrusionHeight = props->extrusionEnabled() ? static_cast<float>( props->extrusionHeight() ) : 0.0f;
  if ( props->dataDefinedProperties().isActive( QgsMapLayerElevationProperties::Property::ExtrusionHeight ) )
  {
    mDataDefinedProperties.setProperty( QgsAbstract3DSymbol::Property::ExtrusionHeight, props->dataDefinedProperties().property( QgsMapLayerElevationProperties::Property::ExtrusionHeight ) );
  }
  else
  {
    mDataDefinedProperties.setProperty( QgsAbstract3DSymbol::Property::ExtrusionHeight, QgsProperty() );
  }
  if ( props->dataDefinedProperties().isActive( QgsMapLayerElevationProperties::Property::ZOffset ) )
  {
    mDataDefinedProperties.setProperty( QgsAbstract3DSymbol::Property::Height, props->dataDefinedProperties().property( QgsMapLayerElevationProperties::Property::ZOffset ) );
  }
  else
  {
    mDataDefinedProperties.setProperty( QgsAbstract3DSymbol::Property::Height, QgsProperty() );
  }
  mOffset = static_cast<float>( props->zOffset() );
}

QgsAbstract3DSymbol *QgsPolygon3DSymbol::create()
{
  return new QgsPolygon3DSymbol();
}

QgsAbstractMaterialSettings *QgsPolygon3DSymbol::materialSettings() const
{
  return mMaterialSettings.get();
}

void QgsPolygon3DSymbol::setMaterialSettings( QgsAbstractMaterialSettings *materialSettings )
{
  if ( materialSettings == mMaterialSettings.get() )
    return;

  mMaterialSettings.reset( materialSettings );
}

bool QgsPolygon3DSymbol::exportGeometries( Qgs3DSceneExporter *exporter, Qt3DCore::QEntity *entity, const QString &objectNamePrefix ) const
{
  QList<Qt3DCore::QEntity *> subEntities = entity->findChildren<Qt3DCore::QEntity *>( QString(), Qt::FindDirectChildrenOnly );
  // sort geometries by their name in order to always export them in the same way:
  std::sort( subEntities.begin(), subEntities.end(), []( const Qt3DCore::QEntity *a, const Qt3DCore::QEntity *b ) {
    return a->objectName() < b->objectName();
  } );

  if ( subEntities.isEmpty() )
  {
    const QList<Qt3DRender::QGeometryRenderer *> renderers = entity->findChildren<Qt3DRender::QGeometryRenderer *>();
    const int startSize = exporter->mObjects.size();
    for ( Qt3DRender::QGeometryRenderer *renderer : renderers )
    {
      Qgs3DExportObject *object = exporter->processGeometryRenderer( renderer, objectNamePrefix );
      if ( object )
      {
        exporter->processEntityMaterial( entity, object );
        exporter->mObjects.push_back( object );
      }
    }
    return exporter->mObjects.size() > startSize;
  }
  else
  {
    bool out = false;
    QString prefix;
    for ( Qt3DCore::QEntity *e : subEntities )
    {
      if ( e->objectName().isEmpty() )
        prefix = objectNamePrefix;
      else
        prefix = e->objectName() + "_";

      out |= exportGeometries( exporter, e, prefix );
    }
    return out;
  }
}
