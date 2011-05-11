/***************************************************************************
                              qgscapabilitiescache.h
                              ----------------------
  begin                : May 11th, 2011
  copyright            : (C) 2011 by Marco Hugentobler
  email                : marco dot hugentobler at sourcepole dot ch
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgscapabilitiescache.h"

QgsCapabilitiesCache::QgsCapabilitiesCache()
{
}

QgsCapabilitiesCache::~QgsCapabilitiesCache()
{
}

const QDomDocument* QgsCapabilitiesCache::searchCapabilitiesDocument( const QString& configFilePath ) const
{
  QHash< QString, QDomDocument >::const_iterator it = mCachedCapabilities.find( configFilePath );
  if( it == mCachedCapabilities.constEnd() )
  {
    return 0;
  }
  else
  {
    return &(it.value());
  }
}

void QgsCapabilitiesCache::insertCapabilitiesDocument( const QString& configFilePath, const QDomDocument* doc )
{
  mCachedCapabilities.insert( configFilePath, doc->cloneNode().toDocument() );
}
