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

#include "qgs3d.h"
#include "qgs3dsceneexporter.h"
#include "qgs3dutils.h"
#include "qgscolorutils.h"
#include "qgsmaterialregistry.h"
#include "qgstessellatedpolygongeometry.h"
#include "qgsvectorlayer.h"
#include "qgsvectorlayerelevationproperties.h"

#include <Qt3DCore/QEntity>

QgsPolygon3DSymbol::QgsPolygon3DSymbol()
  : mMaterialSettings( std::make_unique<QgsPhongMaterialSettings>() )
{
}

QgsPolygon3DSymbol::~QgsPolygon3DSymbol() = default;

QgsAbstract3DSymbol *QgsPolygon3DSymbol::clone() const
{
  auto result = std::make_unique<QgsPolygon3DSymbol>();
  result->mAltClamping = mAltClamping;
  result->mAltBinding = mAltBinding;
  result->mOffset = mOffset;
  result->mExtrusionHeight = mExtrusionHeight;
  result->mMaterialSettings.reset( mMaterialSettings->clone() );
  result->mCullingMode = mCullingMode;
  result->mInvertNormals = mInvertNormals;
  result->mAddBackFaces = mAddBackFaces;
  result->mExtrusionFaces = mExtrusionFaces;
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

  QDomElement elemDataProperties = doc.createElement( u"data"_s );
  elemDataProperties.setAttribute( u"alt-clamping"_s, Qgs3DUtils::altClampingToString( mAltClamping ) );
  elemDataProperties.setAttribute( u"alt-binding"_s, Qgs3DUtils::altBindingToString( mAltBinding ) );
  elemDataProperties.setAttribute( u"offset"_s, mOffset );
  elemDataProperties.setAttribute( u"extrusion-height"_s, mExtrusionHeight );
  elemDataProperties.setAttribute( u"culling-mode"_s, Qgs3DUtils::cullingModeToString( mCullingMode ) );
  elemDataProperties.setAttribute( u"invert-normals"_s, mInvertNormals ? u"1"_s : u"0"_s );
  elemDataProperties.setAttribute( u"add-back-faces"_s, mAddBackFaces ? u"1"_s : u"0"_s );
  elemDataProperties.setAttribute( u"rendered-facade"_s, qgsEnumValueToKey( mExtrusionFaces ) );
  elem.appendChild( elemDataProperties );

  elem.setAttribute( u"material_type"_s, mMaterialSettings->type() );
  QDomElement elemMaterial = doc.createElement( u"material"_s );
  mMaterialSettings->writeXml( elemMaterial, context );
  elem.appendChild( elemMaterial );

  QDomElement elemDDP = doc.createElement( u"data-defined-properties"_s );
  mDataDefinedProperties.writeXml( elemDDP, propertyDefinitions() );
  elem.appendChild( elemDDP );

  QDomElement elemEdges = doc.createElement( u"edges"_s );
  elemEdges.setAttribute( u"enabled"_s, mEdgesEnabled ? u"1"_s : u"0"_s );
  elemEdges.setAttribute( u"width"_s, mEdgeWidth );
  elemEdges.setAttribute( u"color"_s, QgsColorUtils::colorToString( mEdgeColor ) );
  elem.appendChild( elemEdges );
}

void QgsPolygon3DSymbol::readXml( const QDomElement &elem, const QgsReadWriteContext &context )
{
  Q_UNUSED( context )

  const QDomElement elemDataProperties = elem.firstChildElement( u"data"_s );
  mAltClamping = Qgs3DUtils::altClampingFromString( elemDataProperties.attribute( u"alt-clamping"_s ) );
  mAltBinding = Qgs3DUtils::altBindingFromString( elemDataProperties.attribute( u"alt-binding"_s ) );
  mOffset = elemDataProperties.attribute( u"offset"_s ).toFloat();
  mExtrusionHeight = elemDataProperties.attribute( u"extrusion-height"_s ).toFloat();
  mCullingMode = Qgs3DUtils::cullingModeFromString( elemDataProperties.attribute( u"culling-mode"_s ) );
  mInvertNormals = elemDataProperties.attribute( u"invert-normals"_s ).toInt();
  mAddBackFaces = elemDataProperties.attribute( u"add-back-faces"_s ).toInt();
  mExtrusionFaces = qgsEnumKeyToValue( elemDataProperties.attribute( u"rendered-facade"_s ), Qgis::ExtrusionFace::Walls | Qgis::ExtrusionFace::Roof );

  const QDomElement elemMaterial = elem.firstChildElement( u"material"_s );
  const QString materialType = elem.attribute( u"material_type"_s, u"phong"_s );
  mMaterialSettings.reset( Qgs3D::materialRegistry()->createMaterialSettings( materialType ) );
  if ( !mMaterialSettings )
    mMaterialSettings.reset( Qgs3D::materialRegistry()->createMaterialSettings( u"phong"_s ) );
  mMaterialSettings->readXml( elemMaterial, context );

  const QDomElement elemDDP = elem.firstChildElement( u"data-defined-properties"_s );
  if ( !elemDDP.isNull() )
    mDataDefinedProperties.readXml( elemDDP, propertyDefinitions() );

  const QDomElement elemEdges = elem.firstChildElement( u"edges"_s );
  if ( !elemEdges.isNull() )
  {
    mEdgesEnabled = elemEdges.attribute( u"enabled"_s ).toInt();
    mEdgeWidth = elemEdges.attribute( u"width"_s ).toFloat();
    mEdgeColor = QgsColorUtils::colorFromString( elemEdges.attribute( u"color"_s ) );
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

void QgsPolygon3DSymbol::setRenderedFacade( int side )
{
  switch ( side )
  {
    case 0:
      mExtrusionFaces = Qgis::ExtrusionFace::NoFace;
      break;
    case 1:
      mExtrusionFaces = Qgis::ExtrusionFace::Walls;
      break;
    case 2:
      mExtrusionFaces = Qgis::ExtrusionFace::Roof;
      break;
    case 3:
      mExtrusionFaces = Qgis::ExtrusionFace::Walls | Qgis::ExtrusionFace::Roof;
      break;
    case 7:
      mExtrusionFaces = Qgis::ExtrusionFace::Walls | Qgis::ExtrusionFace::Roof | Qgis::ExtrusionFace::Floor;
      break;
    default:
      break;
  }
}

int QgsPolygon3DSymbol::renderedFacade()
{
  const int flags = static_cast<int>( mExtrusionFaces );

  if ( flags == static_cast<int>( Qgis::ExtrusionFace::Walls | Qgis::ExtrusionFace::Roof | Qgis::ExtrusionFace::Floor ) )
    return 7;
  else if ( flags == static_cast<int>( Qgis::ExtrusionFace::Walls | Qgis::ExtrusionFace::Roof ) )
    return 3;
  else if ( flags == static_cast<int>( Qgis::ExtrusionFace::Roof ) )
    return 2;
  else if ( flags == static_cast<int>( Qgis::ExtrusionFace::Walls ) )
    return 1;
  else
    return 0;
}
