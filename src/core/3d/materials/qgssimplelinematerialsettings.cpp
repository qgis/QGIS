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

#include "qgscolorutils.h"

#include <QMap>
#include <QString>

using namespace Qt::StringLiterals;

QString QgsSimpleLineMaterialSettings::type() const
{
  return u"simpleline"_s;
}

bool QgsSimpleLineMaterialSettings::supportsTechnique( Qgis::MaterialRenderingTechnique technique )
{
  switch ( technique )
  {
    case Qgis::MaterialRenderingTechnique::Lines:
      return true;

    case Qgis::MaterialRenderingTechnique::Triangles:
    case Qgis::MaterialRenderingTechnique::InstancedPoints:
    case Qgis::MaterialRenderingTechnique::Points:
    case Qgis::MaterialRenderingTechnique::TrianglesWithFixedTexture:
    case Qgis::MaterialRenderingTechnique::TrianglesFromModel:
    case Qgis::MaterialRenderingTechnique::TrianglesDataDefined:
    case Qgis::MaterialRenderingTechnique::Billboards:
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

bool QgsSimpleLineMaterialSettings::equals( const QgsAbstractMaterialSettings *other ) const
{
  const QgsSimpleLineMaterialSettings *otherLine = dynamic_cast<const QgsSimpleLineMaterialSettings *>( other );
  if ( !otherLine )
    return false;

  return *this == *otherLine;
}

void QgsSimpleLineMaterialSettings::readXml( const QDomElement &elem, const QgsReadWriteContext &context )
{
  mAmbient = QgsColorUtils::colorFromString( elem.attribute( u"ambient"_s, u"25,25,25"_s ) );

  QgsAbstractMaterialSettings::readXml( elem, context );
}

void QgsSimpleLineMaterialSettings::writeXml( QDomElement &elem, const QgsReadWriteContext &context ) const
{
  elem.setAttribute( u"ambient"_s, QgsColorUtils::colorToString( mAmbient ) );

  QgsAbstractMaterialSettings::writeXml( elem, context );
}

QColor QgsSimpleLineMaterialSettings::averageColor() const
{
  return ambient();
}

void QgsSimpleLineMaterialSettings::setColorsFromBase( const QColor &baseColor )
{
  setAmbient( baseColor );
}
