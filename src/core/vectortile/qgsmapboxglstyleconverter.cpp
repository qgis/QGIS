/***************************************************************************
  qgsmapboxglstyleconverter.cpp
  --------------------------------------
  Date                 : September 2020
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

#include "qgsmapboxglstyleconverter.h"
#include "qgsvectortilebasicrenderer.h"
#include "qgsvectortilebasiclabeling.h"

QgsMapBoxGlStyleConverter::QgsMapBoxGlStyleConverter( const QVariantMap &style )
  : mStyle( style )
{
  if ( mStyle.contains( QStringLiteral( "layers" ) ) )
  {
    parseLayers( mStyle.value( QStringLiteral( "layers" ) ).toList() );
  }
  else
  {
    mError = QObject::tr( "Could not find layers list in JSON" );
  }
}

QgsMapBoxGlStyleConverter::~QgsMapBoxGlStyleConverter() = default;

void QgsMapBoxGlStyleConverter::parseLayers( const QVariantList &layers )
{

}

QgsVectorTileRenderer *QgsMapBoxGlStyleConverter::renderer() const
{
  return mRenderer ? mRenderer->clone() : nullptr;
}

QgsVectorTileLabeling *QgsMapBoxGlStyleConverter::labeling() const
{
  return mLabeling ? mLabeling->clone() : nullptr;
}
