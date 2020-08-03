/***************************************************************************
  qgsgoochmaterialsettings.cpp
  --------------------------------------
  Date                 : July 2020
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

#include "qgsgoochmaterialsettings.h"

#include "qgssymbollayerutils.h"
#include "qgslinematerial_p.h"
#include <Qt3DExtras/QGoochMaterial>

QString QgsGoochMaterialSettings::type() const
{
  return QStringLiteral( "gooch" );
}

QgsAbstractMaterialSettings *QgsGoochMaterialSettings::create()
{
  return new QgsGoochMaterialSettings();
}

bool QgsGoochMaterialSettings::supportsTechnique( QgsMaterialSettingsRenderingTechnique technique )
{
  switch ( technique )
  {
    case QgsMaterialSettingsRenderingTechnique::Triangles:
    case QgsMaterialSettingsRenderingTechnique::TrianglesWithFixedTexture:
      return true;

    case QgsMaterialSettingsRenderingTechnique::Lines:
    case QgsMaterialSettingsRenderingTechnique::InstancedPoints:
    case QgsMaterialSettingsRenderingTechnique::Points:
      return false;
  }
  return false;
}

QgsGoochMaterialSettings *QgsGoochMaterialSettings::clone() const
{
  return new QgsGoochMaterialSettings( *this );
}

void QgsGoochMaterialSettings::readXml( const QDomElement &elem, const QgsReadWriteContext & )
{
  mWarm = QgsSymbolLayerUtils::decodeColor( elem.attribute( QStringLiteral( "warm" ), QStringLiteral( "107,0,107" ) ) );
  mCool = QgsSymbolLayerUtils::decodeColor( elem.attribute( QStringLiteral( "cool" ), QStringLiteral( "255,130,0" ) ) );
  mDiffuse = QgsSymbolLayerUtils::decodeColor( elem.attribute( QStringLiteral( "diffuse" ), QStringLiteral( "178,178,178" ) ) );
  mSpecular = QgsSymbolLayerUtils::decodeColor( elem.attribute( QStringLiteral( "specular" ) ) );
  mShininess = elem.attribute( QStringLiteral( "shininess2" ), QStringLiteral( "100" ) ).toFloat();
  mAlpha = elem.attribute( QStringLiteral( "alpha" ), QStringLiteral( "0.25" ) ).toFloat();
  mBeta = elem.attribute( QStringLiteral( "beta" ), QStringLiteral( "0.5" ) ).toFloat();
}

void QgsGoochMaterialSettings::writeXml( QDomElement &elem, const QgsReadWriteContext & ) const
{
  elem.setAttribute( QStringLiteral( "warm" ), QgsSymbolLayerUtils::encodeColor( mWarm ) );
  elem.setAttribute( QStringLiteral( "cool" ), QgsSymbolLayerUtils::encodeColor( mCool ) );
  elem.setAttribute( QStringLiteral( "diffuse" ), QgsSymbolLayerUtils::encodeColor( mDiffuse ) );
  elem.setAttribute( QStringLiteral( "specular" ), QgsSymbolLayerUtils::encodeColor( mSpecular ) );
  elem.setAttribute( QStringLiteral( "shininess2" ), mShininess );
  elem.setAttribute( QStringLiteral( "alpha" ), mAlpha );
  elem.setAttribute( QStringLiteral( "beta" ), mBeta );
}

QMap<QString, QString> QgsGoochMaterialSettings::toExportParameters() const
{
  return QMap<QString, QString>();
}

Qt3DRender::QMaterial *QgsGoochMaterialSettings::toMaterial( QgsMaterialSettingsRenderingTechnique technique, const QgsMaterialContext &context ) const
{
  switch ( technique )
  {
    case QgsMaterialSettingsRenderingTechnique::Triangles:
    case QgsMaterialSettingsRenderingTechnique::TrianglesWithFixedTexture:
    {
      Qt3DExtras::QGoochMaterial *material  = new Qt3DExtras::QGoochMaterial;
      material->setDiffuse( mDiffuse );
      material->setWarm( mWarm );
      material->setCool( mCool );

      material->setSpecular( mSpecular );
      material->setShininess( mShininess );
      material->setAlpha( mAlpha );
      material->setBeta( mBeta );

      if ( context.isSelected() )
      {
        // update the material with selection colors
        material->setDiffuse( context.selectionColor() );
      }
      return material;
    }

    case QgsMaterialSettingsRenderingTechnique::Lines:
    case QgsMaterialSettingsRenderingTechnique::InstancedPoints:
    case QgsMaterialSettingsRenderingTechnique::Points:
      return nullptr;
  }
  return nullptr;
}

void QgsGoochMaterialSettings::addParametersToEffect( Qt3DRender::QEffect * ) const
{
}
