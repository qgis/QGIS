/***************************************************************************
                         qgsrasterrendererregistry.cpp
                         -----------------------------
    begin                : January 2012
    copyright            : (C) 2012 by Marco Hugentobler
    email                : marco at sourcepole dot ch
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsrasterrendererregistry.h"

QgsRasterRendererRegistry* QgsRasterRendererRegistry::mInstance = 0;

QgsRasterRendererRegistry* QgsRasterRendererRegistry::instance()
{
  if( !mInstance )
  {
    mInstance = new QgsRasterRendererRegistry();
  }
  return mInstance;
}

QgsRasterRendererRegistry::QgsRasterRendererRegistry()
{
}

QgsRasterRendererRegistry::~QgsRasterRendererRegistry()
{
}

void QgsRasterRendererRegistry::insert( QgsRasterRendererRegistryEntry entry )
{
  mEntries.insert( entry.name, entry );
}

bool QgsRasterRendererRegistry::rendererData( const QString& rendererName, QgsRasterRendererRegistryEntry& data ) const
{
  QHash< QString, QgsRasterRendererRegistryEntry >::const_iterator it = mEntries.find( rendererName );
  if( it == mEntries.constEnd() )
  {
    return false;
  }
  data = it.value();
  return true;
}


