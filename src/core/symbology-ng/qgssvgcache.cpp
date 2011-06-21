/***************************************************************************
                              qgssvgcache.h
                            ------------------------------
  begin                :  2011
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

#include "qgssvgcache.h"

QgsSvgCache* QgsSvgCache::mInstance = 0;

QgsSvgCache* QgsSvgCache::instance()
{
  if ( !mInstance )
  {
    mInstance = new QgsSvgCache();
  }
  return mInstance;
}

QgsSvgCache::QgsSvgCache()
{
}

QgsSvgCache::~QgsSvgCache()
{
}


const QImage& QgsSvgCache::svgAsImage( const QString& file, double size, const QColor& fill, const QColor& outline, double outlineWidth ) const
{
}

const QPicture& QgsSvgCache::svgAsPicture( const QString& file, double size, const QColor& fill, const QColor& outline, double outlineWidth ) const
{
}

void QgsSvgCache::insertSVG( const QString& file, double size, const QColor& fill, const QColor& outline, double outlineWidth )
{
}

void QgsSvgCache::cacheImage( QgsSvgCacheEntry )
{
}

void QgsSvgCache::cachePicture( QgsSvgCacheEntry )
{
}

