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

#include "qgs3dutils.h"
#include "qgshighlightmaterial.h"
#include "qgsphongmaterial.h"
#include "qgsphongmaterialsettings.h"

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
    case Qgis::MaterialRenderingTechnique::Triangles:
    case Qgis::MaterialRenderingTechnique::InstancedPoints:
    case Qgis::MaterialRenderingTechnique::Points:
    case Qgis::MaterialRenderingTechnique::TrianglesDataDefined:
    case Qgis::MaterialRenderingTechnique::TrianglesWithFixedTexture:
    case Qgis::MaterialRenderingTechnique::TrianglesFromModel:
    {
      if ( context.isHighlighted() )
      {
        return new QgsHighlightMaterial( technique );
      }

      return buildMaterial( settings, context );
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

void QgsPhongMaterial3DHandler::addParametersToEffect( Qt3DRender::QEffect *effect, const QgsAbstractMaterialSettings *settings, const QgsMaterialContext &materialContext ) const
{
  const QgsPhongMaterialSettings *phongSettings = dynamic_cast< const QgsPhongMaterialSettings * >( settings );
  Q_ASSERT( phongSettings );

  const QColor ambient = Qgs3DUtils::srgbToLinear( materialContext.isSelected() ? materialContext.selectionColor().darker() : phongSettings->ambient() );
  const QColor diffuse = Qgs3DUtils::srgbToLinear( materialContext.isSelected() ? materialContext.selectionColor() : phongSettings->diffuse() );

  Qt3DRender::QParameter *ambientParameter = new Qt3DRender::QParameter(
    u"ambientColor"_s,
    QColor::fromRgbF(
      static_cast< float >( ambient.redF() * phongSettings->ambientCoefficient() ),
      static_cast< float >( ambient.greenF() * phongSettings->ambientCoefficient() ),
      static_cast< float >( ambient.blueF() * phongSettings->ambientCoefficient() )
    )
  );
  Qt3DRender::QParameter *diffuseParameter = new Qt3DRender::QParameter(
    u"diffuseColor"_s,
    QColor::fromRgbF(
      static_cast<float >( diffuse.redF() * phongSettings->diffuseCoefficient() ),
      static_cast< float >( diffuse.greenF() * phongSettings->diffuseCoefficient() ),
      static_cast< float >( diffuse.blueF() * phongSettings->diffuseCoefficient() )
    )
  );

  const QColor specular = Qgs3DUtils::srgbToLinear( phongSettings->specular() );
  Qt3DRender::QParameter *specularParameter = new Qt3DRender::QParameter(
    u"specularColor"_s,
    QColor::fromRgbF(
      static_cast< float >( specular.redF() * phongSettings->specularCoefficient() ),
      static_cast< float >( specular.greenF() * phongSettings->specularCoefficient() ),
      static_cast< float >( specular.blueF() * phongSettings->specularCoefficient() )
    )
  );
  Qt3DRender::QParameter *shininessParameter = new Qt3DRender::QParameter( u"shininess"_s, static_cast<float>( phongSettings->shininess() ) );
  Qt3DRender::QParameter *opacityParameter = new Qt3DRender::QParameter( u"opacity"_s, static_cast<float>( phongSettings->opacity() ) );

  effect->addParameter( ambientParameter );
  effect->addParameter( diffuseParameter );
  effect->addParameter( specularParameter );
  effect->addParameter( shininessParameter );
  effect->addParameter( opacityParameter );
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

int QgsPhongMaterial3DHandler::dataDefinedByteStride( const QgsAbstractMaterialSettings * ) const
{
  return 9 * sizeof( unsigned char );
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

QgsMaterial *QgsPhongMaterial3DHandler::buildMaterial( const QgsAbstractMaterialSettings *settings, const QgsMaterialContext &context ) const
{
  const QgsPhongMaterialSettings *phongSettings = dynamic_cast< const QgsPhongMaterialSettings * >( settings );
  Q_ASSERT( phongSettings );

  QgsPhongMaterial *material = new QgsPhongMaterial;
  material->setObjectName( u"phongMaterial"_s );

  material->setDataDefinedEnabled( phongSettings->dataDefinedProperties().hasActiveProperties() );

  if ( !phongSettings->dataDefinedProperties().hasActiveProperties() )
  {
    const QColor ambient = context.isSelected() ? context.selectionColor().darker() : phongSettings->ambient();
    const QColor diffuse = context.isSelected() ? context.selectionColor() : phongSettings->diffuse();
    const QColor specular = phongSettings->specular();

    material->setAmbient( ambient, static_cast<float>( phongSettings->ambientCoefficient() ) );
    material->setDiffuse( diffuse, static_cast<float>( phongSettings->diffuseCoefficient() ) );
    material->setSpecular( specular, static_cast<float>( phongSettings->specularCoefficient() ) );
  }

  material->setShininess( static_cast<float>( phongSettings->shininess() ) );
  material->setOpacity( static_cast<float>( phongSettings->opacity() ) );

  return material;
}
