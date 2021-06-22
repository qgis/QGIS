/***************************************************************************
                             qgsprovidersublayerdetails.cpp
                             ----------------------------
    begin                : May 2021
    copyright            : (C) 2021 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************/
/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsprovidersublayerdetails.h"
#include "qgsmaplayerfactory.h"

QgsProviderSublayerDetails::LayerOptions::LayerOptions( const QgsCoordinateTransformContext &transformContext )
  : transformContext( transformContext )
{}

QgsMapLayer *QgsProviderSublayerDetails::toLayer( const LayerOptions &options ) const
{
  return QgsMapLayerFactory::createLayer( mUri, mName, mType, mProviderKey, options.transformContext );
}
