/***************************************************************************
  qgsclothmaterial.cpp
  --------------------------------------
  Date                 : April 2026
  Copyright            : (C) 2026 by Nyall Dawson
  Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsclothmaterial.h"

#include "qgs3dutils.h"

#include <QString>
#include <Qt3DRender/QAbstractTexture>
#include <Qt3DRender/QGraphicsApiFilter>
#include <Qt3DRender/QParameter>
#include <Qt3DRender/QRenderPass>
#include <Qt3DRender/QShaderProgramBuilder>
#include <Qt3DRender/QTechnique>
#include <Qt3DRender/QTexture>

#include "moc_qgsclothmaterial.cpp"

using namespace Qt::StringLiterals;

///@cond PRIVATE
QgsClothMaterial::QgsClothMaterial( QNode *parent )
  : QgsPBRMaterial( parent )
  , mSheenParameter( new Qt3DRender::QParameter( u"sheenColor"_s, Qgs3DUtils::srgbToLinear( QColor( "red" ) ), this ) )
{
  init();
}

QgsClothMaterial::~QgsClothMaterial() = default;

void QgsClothMaterial::setSheenColor( const QColor &sheenColor )
{
  mSheenParameter->setValue( Qgs3DUtils::srgbToLinear( sheenColor ) );
}

QStringList QgsClothMaterial::fragmentShaderDefines() const
{
  QStringList defines = QgsPBRMaterial::fragmentShaderDefines();
  defines << u"CLOTH_MATERIAL"_s;
  return defines;
}

void QgsClothMaterial::init()
{
  initMaterial();
  mSheenParameter->setParent( mEffect );
  mEffect->addParameter( mSheenParameter );
}


///@endcond PRIVATE
