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
  mShininess = elem.attribute( QStringLiteral( "shininess" ) ).toFloat();
}

void QgsGoochMaterialSettings::writeXml( QDomElement &elem, const QgsReadWriteContext & ) const
{
  elem.setAttribute( QStringLiteral( "warm" ), QgsSymbolLayerUtils::encodeColor( mWarm ) );
  elem.setAttribute( QStringLiteral( "cool" ), QgsSymbolLayerUtils::encodeColor( mCool ) );
  elem.setAttribute( QStringLiteral( "diffuse" ), QgsSymbolLayerUtils::encodeColor( mDiffuse ) );
  elem.setAttribute( QStringLiteral( "specular" ), QgsSymbolLayerUtils::encodeColor( mSpecular ) );
  elem.setAttribute( QStringLiteral( "shininess" ), mShininess );
}

Qt3DRender::QMaterial *QgsGoochMaterialSettings::toMaterial( const QgsMaterialContext &context ) const
{
  Qt3DExtras::QGoochMaterial *material  = new Qt3DExtras::QGoochMaterial;
  material->setDiffuse( mDiffuse );
  material->setWarm( mWarm );
  material->setCool( mCool );

  material->setSpecular( mSpecular );
  material->setShininess( mShininess );

  if ( context.isSelected() )
  {
    // update the material with selection colors
    material->setDiffuse( context.selectionColor() );
  }
  return material;
}

QgsLineMaterial *QgsGoochMaterialSettings::toLineMaterial( const QgsMaterialContext &context ) const
{
  QgsLineMaterial *mat = new QgsLineMaterial;
  mat->setLineColor( mDiffuse );
  if ( context.isSelected() )
  {
    // update the material with selection colors
    mat->setLineColor( context.selectionColor() );
  }
  return mat;
}

void QgsGoochMaterialSettings::addParametersToEffect( Qt3DRender::QEffect * ) const
{
}
