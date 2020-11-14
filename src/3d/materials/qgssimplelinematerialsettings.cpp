/***************************************************************************
  qgssimplelinematerialsettings.cpp
  --------------------------------------
  Date                 : August 2020
  Copyright            : (C) 2020 by Nyall Dawson
  Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgssimplelinematerialsettings.h"
#include "qgssymbollayerutils.h"
#include "qgslinematerial_p.h"
#include <Qt3DRender/QTexture>
#include <Qt3DRender/QParameter>
#include <Qt3DRender/QEffect>
#include <QMap>


QString QgsSimpleLineMaterialSettings::type() const
{
  return QStringLiteral( "simpleline" );
}

bool QgsSimpleLineMaterialSettings::supportsTechnique( QgsMaterialSettingsRenderingTechnique technique )
{
  switch ( technique )
  {
    case QgsMaterialSettingsRenderingTechnique::Lines:
      return true;

    case QgsMaterialSettingsRenderingTechnique::Triangles:
    case QgsMaterialSettingsRenderingTechnique::InstancedPoints:
    case QgsMaterialSettingsRenderingTechnique::Points:
    case QgsMaterialSettingsRenderingTechnique::TrianglesWithFixedTexture:
    case QgsMaterialSettingsRenderingTechnique::TrianglesFromModel:
    case QgsMaterialSettingsRenderingTechnique::TrianglesDataDefined:
      return false;
  }
  return false;
}

QgsAbstractMaterialSettings *QgsSimpleLineMaterialSettings::create()
{
  return new QgsSimpleLineMaterialSettings();
}

QgsSimpleLineMaterialSettings *QgsSimpleLineMaterialSettings::clone() const
{
  return new QgsSimpleLineMaterialSettings( *this );
}

void QgsSimpleLineMaterialSettings::readXml( const QDomElement &elem, const QgsReadWriteContext & )
{
  mAmbient = QgsSymbolLayerUtils::decodeColor( elem.attribute( QStringLiteral( "ambient" ), QStringLiteral( "25,25,25" ) ) );
}

void QgsSimpleLineMaterialSettings::writeXml( QDomElement &elem, const QgsReadWriteContext & ) const
{
  elem.setAttribute( QStringLiteral( "ambient" ), QgsSymbolLayerUtils::encodeColor( mAmbient ) );
}

Qt3DRender::QMaterial *QgsSimpleLineMaterialSettings::toMaterial( QgsMaterialSettingsRenderingTechnique technique, const QgsMaterialContext &context ) const
{
  switch ( technique )
  {
    case QgsMaterialSettingsRenderingTechnique::Triangles:
    case QgsMaterialSettingsRenderingTechnique::InstancedPoints:
    case QgsMaterialSettingsRenderingTechnique::Points:
    case QgsMaterialSettingsRenderingTechnique::TrianglesWithFixedTexture:
    case QgsMaterialSettingsRenderingTechnique::TrianglesFromModel:
    case QgsMaterialSettingsRenderingTechnique::TrianglesDataDefined:
      return nullptr;

    case QgsMaterialSettingsRenderingTechnique::Lines:
    {
      QgsLineMaterial *mat = new QgsLineMaterial;
      mat->setLineColor( mAmbient );
      if ( context.isSelected() )
      {
        // update the material with selection colors
        mat->setLineColor( context.selectionColor() );
      }
      return mat;
    }
  }
  return nullptr;
}

QMap<QString, QString> QgsSimpleLineMaterialSettings::toExportParameters() const
{
  QMap<QString, QString> parameters;
  parameters[ QStringLiteral( "Ka" ) ] = QStringLiteral( "%1 %2 %3" ).arg( mAmbient.redF() ).arg( mAmbient.greenF() ).arg( mAmbient.blueF() );
  return parameters;
}

void QgsSimpleLineMaterialSettings::addParametersToEffect( Qt3DRender::QEffect *effect ) const
{
  Qt3DRender::QParameter *ambientParameter = new Qt3DRender::QParameter( QStringLiteral( "ka" ), QColor::fromRgbF( 0.05f, 0.05f, 0.05f, 1.0f ) );
  ambientParameter->setValue( mAmbient );
  effect->addParameter( ambientParameter );
}
