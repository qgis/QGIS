/***************************************************************************
  qgsline3dsymbol.cpp
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

#include "qgsline3dsymbol.h"

#include "qgs3d.h"
#include "qgs3dexportobject.h"
#include "qgs3dsceneexporter.h"
#include "qgs3dutils.h"
#include "qgsmaterialregistry.h"
#include "qgsvectorlayer.h"
#include "qgsvectorlayerelevationproperties.h"

QgsLine3DSymbol::QgsLine3DSymbol()
  : mMaterialSettings( std::make_unique<QgsPhongMaterialSettings>() )
{
}

QgsLine3DSymbol::~QgsLine3DSymbol() = default;

QgsAbstract3DSymbol *QgsLine3DSymbol::clone() const
{
  auto result = std::make_unique<QgsLine3DSymbol>();
  result->mAltClamping = mAltClamping;
  result->mAltBinding = mAltBinding;
  result->mWidth = mWidth;
  result->mOffset = mOffset;
  result->mExtrusionHeight = mExtrusionHeight;
  result->mRenderAsSimpleLines = mRenderAsSimpleLines;
  result->mMaterialSettings.reset( mMaterialSettings->clone() );
  copyBaseSettings( result.get() );
  return result.release();
}

void QgsLine3DSymbol::writeXml( QDomElement &elem, const QgsReadWriteContext &context ) const
{
  Q_UNUSED( context )

  QDomDocument doc = elem.ownerDocument();

  QDomElement elemDataProperties = doc.createElement( u"data"_s );
  elemDataProperties.setAttribute( u"alt-clamping"_s, Qgs3DUtils::altClampingToString( mAltClamping ) );
  elemDataProperties.setAttribute( u"alt-binding"_s, Qgs3DUtils::altBindingToString( mAltBinding ) );
  elemDataProperties.setAttribute( u"offset"_s, mOffset );
  elemDataProperties.setAttribute( u"extrusion-height"_s, mExtrusionHeight );
  elemDataProperties.setAttribute( u"simple-lines"_s, mRenderAsSimpleLines ? u"1"_s : u"0"_s );
  elemDataProperties.setAttribute( u"width"_s, mWidth );
  elem.appendChild( elemDataProperties );

  elem.setAttribute( u"material_type"_s, mMaterialSettings->type() );
  QDomElement elemMaterial = doc.createElement( u"material"_s );
  mMaterialSettings->writeXml( elemMaterial, context );
  elem.appendChild( elemMaterial );
}

void QgsLine3DSymbol::readXml( const QDomElement &elem, const QgsReadWriteContext &context )
{
  Q_UNUSED( context )

  const QDomElement elemDataProperties = elem.firstChildElement( u"data"_s );
  mAltClamping = Qgs3DUtils::altClampingFromString( elemDataProperties.attribute( u"alt-clamping"_s ) );
  mAltBinding = Qgs3DUtils::altBindingFromString( elemDataProperties.attribute( u"alt-binding"_s ) );
  mOffset = elemDataProperties.attribute( u"offset"_s ).toFloat();
  mExtrusionHeight = elemDataProperties.attribute( u"extrusion-height"_s ).toFloat();
  mWidth = elemDataProperties.attribute( u"width"_s ).toFloat();
  mRenderAsSimpleLines = elemDataProperties.attribute( u"simple-lines"_s, u"0"_s ).toInt();

  const QDomElement elemMaterial = elem.firstChildElement( u"material"_s );
  const QString materialType = elem.attribute( u"material_type"_s, u"phong"_s );
  mMaterialSettings.reset( Qgs3D::materialRegistry()->createMaterialSettings( materialType ) );
  if ( !mMaterialSettings )
    mMaterialSettings.reset( Qgs3D::materialRegistry()->createMaterialSettings( u"phong"_s ) );
  mMaterialSettings->readXml( elemMaterial, context );
}

QgsAbstractMaterialSettings *QgsLine3DSymbol::materialSettings() const
{
  return mMaterialSettings.get();
}

void QgsLine3DSymbol::setMaterialSettings( QgsAbstractMaterialSettings *materialSettings )
{
  if ( materialSettings == mMaterialSettings.get() )
    return;

  mMaterialSettings.reset( materialSettings );
}

QList<Qgis::GeometryType> QgsLine3DSymbol::compatibleGeometryTypes() const
{
  return QList<Qgis::GeometryType>() << Qgis::GeometryType::Line;
}

void QgsLine3DSymbol::setDefaultPropertiesFromLayer( const QgsVectorLayer *layer )
{
  const QgsVectorLayerElevationProperties *props = qgis::down_cast<const QgsVectorLayerElevationProperties *>( const_cast<QgsVectorLayer *>( layer )->elevationProperties() );

  mAltClamping = props->clamping();
  mAltBinding = props->binding();
  mExtrusionHeight = props->extrusionEnabled() ? static_cast<float>( props->extrusionHeight() ) : 0.0f;
  mOffset = static_cast<float>( props->zOffset() );
}

QgsAbstract3DSymbol *QgsLine3DSymbol::create()
{
  return new QgsLine3DSymbol();
}

bool QgsLine3DSymbol::exportGeometries( Qgs3DSceneExporter *exporter, Qt3DCore::QEntity *entity, const QString &objectNamePrefix ) const
{
  if ( renderAsSimpleLines() )
  {
    const QVector<Qgs3DExportObject *> objs = exporter->processLines( entity, objectNamePrefix );
    exporter->mObjects << objs;
    return objs.size() != 0;
  }
  else
  {
    const QList<Qt3DRender::QGeometryRenderer *> renderers = entity->findChildren<Qt3DRender::QGeometryRenderer *>();
    for ( Qt3DRender::QGeometryRenderer *r : renderers )
    {
      Qgs3DExportObject *object = exporter->processGeometryRenderer( r, objectNamePrefix );
      if ( !object )
        continue;
      object->setupMaterial( materialSettings() );
      exporter->mObjects.push_back( object );
    }
    return renderers.size() != 0;
  }
}
