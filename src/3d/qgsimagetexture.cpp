/***************************************************************************
  qgs3dsceneexporter.h
  --------------------------------------
  Date                 : July 2020
  Copyright            : (C) 2020 by Belgacem Nedjima
  Email                : gb underscore nedjima at esi dot dz
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsimagetexture.h"
#include "moc_qgsimagetexture.cpp"


QgsImageTexture::QgsImageTexture( const QImage &image, Qt3DCore::QNode *parent )
  : Qt3DRender::QPaintedTextureImage( parent )
  , mImage( image )
{
  setSize( mImage.size() );
}
