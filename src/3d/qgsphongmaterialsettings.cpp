/***************************************************************************
  qgsphongmaterialsettings.cpp
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

#include "qgsphongmaterialsettings.h"

#include "qgssymbollayerutils.h"


void QgsPhongMaterialSettings::readXml( const QDomElement &elem )
{
  mAmbient = QgsSymbolLayerUtils::decodeColor( elem.attribute( "ambient" ) );
  mDiffuse = QgsSymbolLayerUtils::decodeColor( elem.attribute( "diffuse" ) );
  mSpecular = QgsSymbolLayerUtils::decodeColor( elem.attribute( "specular" ) );
  mShininess = elem.attribute( "shininess" ).toFloat();
}

void QgsPhongMaterialSettings::writeXml( QDomElement &elem ) const
{
  elem.setAttribute( "ambient", QgsSymbolLayerUtils::encodeColor( mAmbient ) );
  elem.setAttribute( "diffuse", QgsSymbolLayerUtils::encodeColor( mDiffuse ) );
  elem.setAttribute( "specular", QgsSymbolLayerUtils::encodeColor( mSpecular ) );
  elem.setAttribute( "shininess", mShininess );
}
