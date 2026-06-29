/***************************************************************************
  qgsphongmaterial3dhandler.cpp
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

#include "qgsphongmaterial3dhandler.h"

#include "qgs3d.h"
#include "qgs3dutils.h"
#include "qgsphongmaterial.h"
#include "qgsphongmaterialsettings.h"
#include "qgsunlitmaterial.h"

#include <QMap>
#include <QString>
#include <Qt3DCore/QAttribute>
#include <Qt3DCore/QBuffer>
#include <Qt3DCore/QGeometry>
#include <Qt3DRender/QEffect>
#include <Qt3DRender/QGraphicsApiFilter>
#include <Qt3DRender/QParameter>
#include <Qt3DRender/QTechnique>

using namespace Qt::StringLiterals;


QgsMaterial *QgsPhongMaterial3DHandler::toMaterial( const QgsAbstractMaterialSettings *settings, Qgis::MaterialRenderingTechnique technique, const QgsMaterialContext &context ) const
{
  switch ( technique )
  {
    case Qgis::MaterialRenderingTechnique::InstancedPoints:
    {
      Q_ASSERT( false );
      return nullptr;
    }
    case Qgis::MaterialRenderingTechnique::Triangles:
    case Qgis::MaterialRenderingTechnique::Points:
    case Qgis::MaterialRenderingTechnique::TrianglesDataDefined:
    case Qgis::MaterialRenderingTechnique::TrianglesWithFixedTexture:
    case Qgis::MaterialRenderingTechnique::TrianglesFromModel:
    {
      if ( context.isHighlighted() )
      {
        return Qgs3D::createHighlightMaterial();
      }

      const QgsPhongMaterialSettings *phongSettings = dynamic_cast< const QgsPhongMaterialSettings * >( settings );
      Q_ASSERT( phongSettings );

      QgsPhongMaterial *material = new QgsPhongMaterial();
      material->setObjectName( u"phongMaterial"_s );

      const QgsPropertyCollection &dataDefinedProperties = phongSettings->dataDefinedProperties();
      const bool dataDefined = dataDefinedProperties.isActive( QgsAbstractMaterialSettings::Property::Ambient )
                               || dataDefinedProperties.isActive( QgsAbstractMaterialSettings::Property::Diffuse )
                               || dataDefinedProperties.isActive( QgsAbstractMaterialSettings::Property::Specular );
      if ( !dataDefined )
      {
        const QColor ambient = context.isSelected() ? context.selectionColor().darker() : phongSettings->ambient();
        const QColor diffuse = context.isSelected() ? context.selectionColor() : phongSettings->diffuse();
        material->setAmbient( ambient, static_cast<float>( phongSettings->ambientCoefficient() ) );
        material->setDiffuse( diffuse, static_cast<float>( phongSettings->diffuseCoefficient() ) );
        material->setSpecular( phongSettings->specular(), static_cast<float>( phongSettings->specularCoefficient() ) );
      }
      material->setShininess( static_cast<float>( phongSettings->shininess() ) );
      material->setOpacity( static_cast<float>( phongSettings->opacity() ) );
      material->setDataDefinedEnabled( dataDefined );

      return material;
    }

    case Qgis::MaterialRenderingTechnique::Lines:
    case Qgis::MaterialRenderingTechnique::Billboards:
      return nullptr;
  }
  return nullptr;
}

QMap<QString, QString> QgsPhongMaterial3DHandler::toExportParameters( const QgsAbstractMaterialSettings *settings ) const
{
  const QgsPhongMaterialSettings *phongSettings = dynamic_cast< const QgsPhongMaterialSettings * >( settings );
  Q_ASSERT( phongSettings );

  QMap<QString, QString> parameters;
  parameters[u"Kd"_s] = u"%1 %2 %3"_s.arg( phongSettings->diffuse().redF() ).arg( phongSettings->diffuse().greenF() ).arg( phongSettings->diffuse().blueF() );
  parameters[u"Ka"_s] = u"%1 %2 %3"_s.arg( phongSettings->ambient().redF() ).arg( phongSettings->ambient().greenF() ).arg( phongSettings->ambient().blueF() );
  parameters[u"Ks"_s] = u"%1 %2 %3"_s.arg( phongSettings->specular().redF() ).arg( phongSettings->specular().greenF() ).arg( phongSettings->specular().blueF() );
  parameters[u"Ns"_s] = QString::number( phongSettings->shininess() );
  return parameters;
}

QByteArray QgsPhongMaterial3DHandler::dataDefinedVertexColorsAsByte( const QgsAbstractMaterialSettings *settings, const QgsExpressionContext &expressionContext ) const
{
  const QgsPhongMaterialSettings *phongSettings = dynamic_cast< const QgsPhongMaterialSettings * >( settings );
  Q_ASSERT( phongSettings );

  const QColor ambient = Qgs3DUtils::srgbToLinear( phongSettings->dataDefinedProperties().valueAsColor( QgsAbstractMaterialSettings::Property::Ambient, expressionContext, phongSettings->ambient() ) );
  const QColor diffuse = Qgs3DUtils::srgbToLinear( phongSettings->dataDefinedProperties().valueAsColor( QgsAbstractMaterialSettings::Property::Diffuse, expressionContext, phongSettings->diffuse() ) );
  const QColor specular = Qgs3DUtils::srgbToLinear( phongSettings->dataDefinedProperties().valueAsColor( QgsAbstractMaterialSettings::Property::Specular, expressionContext, phongSettings->specular() ) );

  const double diffuseCoefficient = phongSettings->diffuseCoefficient();
  const double ambientCoefficient = phongSettings->ambientCoefficient();
  const double specularCoefficient = phongSettings->specularCoefficient();

  QByteArray array;
  if ( diffuseCoefficient < 1 || ambientCoefficient < 1 || specularCoefficient < 1 )
  {
    // use floats if we are adjusting color component strength, bytes don't
    // give us enough precision
    array.resize( sizeof( float ) * 9 );
    float *fptr = reinterpret_cast<float *>( array.data() );

    *fptr++ = static_cast<float>( diffuse.redF() * diffuseCoefficient );
    *fptr++ = static_cast<float>( diffuse.greenF() * diffuseCoefficient );
    *fptr++ = static_cast<float>( diffuse.blueF() * diffuseCoefficient );

    *fptr++ = static_cast<float>( ambient.redF() * ambientCoefficient );
    *fptr++ = static_cast<float>( ambient.greenF() * ambientCoefficient );
    *fptr++ = static_cast<float>( ambient.blueF() * ambientCoefficient );

    *fptr++ = static_cast<float>( specular.redF() * specularCoefficient );
    *fptr++ = static_cast<float>( specular.greenF() * specularCoefficient );
    *fptr++ = static_cast<float>( specular.blueF() * specularCoefficient );
  }
  else
  {
    array.resize( sizeof( unsigned char ) * 9 );
    unsigned char *ptr = reinterpret_cast<unsigned char *>( array.data() );

    *ptr++ = static_cast<unsigned char>( diffuse.red() );
    *ptr++ = static_cast<unsigned char>( diffuse.green() );
    *ptr++ = static_cast<unsigned char>( diffuse.blue() );

    *ptr++ = static_cast<unsigned char>( ambient.red() );
    *ptr++ = static_cast<unsigned char>( ambient.green() );
    *ptr++ = static_cast<unsigned char>( ambient.blue() );

    *ptr++ = static_cast<unsigned char>( specular.red() );
    *ptr++ = static_cast<unsigned char>( specular.green() );
    *ptr++ = static_cast<unsigned char>( specular.blue() );
  }

  return array;
}

void QgsPhongMaterial3DHandler::applyDataDefinedToGeometry( const QgsAbstractMaterialSettings *settings, Qt3DCore::QGeometry *geometry, int vertexCount, const QByteArray &data ) const
{
  const QgsPhongMaterialSettings *phongSettings = dynamic_cast< const QgsPhongMaterialSettings * >( settings );
  Q_ASSERT( phongSettings );

  Qt3DCore::QBuffer *dataBuffer = new Qt3DCore::QBuffer( geometry );

  // use floats if we are adjusting color component strength, bytes don't
  // give us enough precision
  const bool useFloats = phongSettings->diffuseCoefficient() < 1 || phongSettings->ambientCoefficient() < 1 || phongSettings->specularCoefficient() < 1;

  Qt3DCore::QAttribute *diffuseAttribute = new Qt3DCore::QAttribute( geometry );
  diffuseAttribute->setName( u"dataDefinedDiffuseColor"_s );
  diffuseAttribute->setVertexBaseType( useFloats ? Qt3DCore::QAttribute::Float : Qt3DCore::QAttribute::UnsignedByte );
  diffuseAttribute->setVertexSize( 3 );
  diffuseAttribute->setAttributeType( Qt3DCore::QAttribute::VertexAttribute );
  diffuseAttribute->setBuffer( dataBuffer );
  diffuseAttribute->setByteStride( 9 * ( useFloats ? sizeof( float ) : sizeof( unsigned char ) ) );
  diffuseAttribute->setByteOffset( 0 );
  diffuseAttribute->setCount( vertexCount );
  geometry->addAttribute( diffuseAttribute );

  Qt3DCore::QAttribute *ambientAttribute = new Qt3DCore::QAttribute( geometry );
  ambientAttribute->setName( u"dataDefinedAmbiantColor"_s );
  ambientAttribute->setVertexBaseType( useFloats ? Qt3DCore::QAttribute::Float : Qt3DCore::QAttribute::UnsignedByte );
  ambientAttribute->setVertexSize( 3 );
  ambientAttribute->setAttributeType( Qt3DCore::QAttribute::VertexAttribute );
  ambientAttribute->setBuffer( dataBuffer );
  ambientAttribute->setByteStride( 9 * ( useFloats ? sizeof( float ) : sizeof( unsigned char ) ) );
  ambientAttribute->setByteOffset( 3 * ( useFloats ? sizeof( float ) : sizeof( unsigned char ) ) );
  ambientAttribute->setCount( vertexCount );
  geometry->addAttribute( ambientAttribute );

  Qt3DCore::QAttribute *specularAttribute = new Qt3DCore::QAttribute( geometry );
  specularAttribute->setName( u"dataDefinedSpecularColor"_s );
  specularAttribute->setVertexBaseType( useFloats ? Qt3DCore::QAttribute::Float : Qt3DCore::QAttribute::UnsignedByte );
  specularAttribute->setVertexSize( 3 );
  specularAttribute->setAttributeType( Qt3DCore::QAttribute::VertexAttribute );
  specularAttribute->setBuffer( dataBuffer );
  specularAttribute->setByteStride( 9 * ( useFloats ? sizeof( float ) : sizeof( unsigned char ) ) );
  specularAttribute->setByteOffset( 6 * ( useFloats ? sizeof( float ) : sizeof( unsigned char ) ) );
  specularAttribute->setCount( vertexCount );
  geometry->addAttribute( specularAttribute );

  dataBuffer->setData( data );
}

bool QgsPhongMaterial3DHandler::updatePreviewScene( Qt3DCore::QEntity *sceneRoot, const QgsAbstractMaterialSettings *settings, const QgsMaterialContext & ) const
{
  const QgsPhongMaterialSettings *phongSettings = qgis::down_cast< const QgsPhongMaterialSettings * >( settings );

  QgsPhongMaterial *material = sceneRoot->findChild<QgsPhongMaterial *>();
  if ( !material || material->objectName() != "phongMaterial"_L1 )
    return false;

  material->setAmbient( phongSettings->ambient(), static_cast<float>( phongSettings->ambientCoefficient() ) );
  material->setDiffuse( phongSettings->diffuse(), static_cast<float>( phongSettings->diffuseCoefficient() ) );
  material->setSpecular( phongSettings->specular(), static_cast<float>( phongSettings->specularCoefficient() ) );
  material->setShininess( static_cast<float>( phongSettings->shininess() ) );
  material->setOpacity( static_cast<float>( phongSettings->opacity() ) );

  return true;
}

QgsMaterial *QgsPhongMaterial3DHandler::toInstancedMaterial(
  const QgsAbstractMaterialSettings *settings, const QgsMaterialContext &context, Qgis::InstancedMaterialFlags flags, const QMatrix4x4 &transform
) const
{
  const QgsPhongMaterialSettings *phongSettings = qgis::down_cast< const QgsPhongMaterialSettings * >( settings );

  QgsPhongMaterial *material = new QgsPhongMaterial();
  material->setInstancingEnabled( true, flags );
  material->setInstancingMeshTransform( transform );
  material->setObjectName( u"phongMaterial"_s );

  const QColor ambient = context.isSelected() ? context.selectionColor().darker() : phongSettings->ambient();
  const QColor diffuse = context.isSelected() ? context.selectionColor() : phongSettings->diffuse();
  material->setAmbient( ambient, static_cast<float>( phongSettings->ambientCoefficient() ) );
  material->setDiffuse( diffuse, static_cast<float>( phongSettings->diffuseCoefficient() ) );
  material->setSpecular( phongSettings->specular(), static_cast<float>( phongSettings->specularCoefficient() ) );
  material->setShininess( static_cast<float>( phongSettings->shininess() ) );
  material->setOpacity( static_cast<float>( phongSettings->opacity() ) );

  return material;
}
