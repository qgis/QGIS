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

#ifndef QGSSVGCACHE_H
#define QGSSVGCACHE_H

#include <QColor>
#include <QDateTime>
#include <QMap>
#include <QMultiHash>
#include <QString>

class QDomElement;
class QImage;
class QPicture;

struct QgsSvgCacheEntry
{
  QgsSvgCacheEntry();
  QgsSvgCacheEntry( const QString& file, double size, double outlineWidth, double widthScaleFactor, double rasterScaleFctor, const QColor& fill, const QColor& outline );
  ~QgsSvgCacheEntry();

  QString file;
  double size;
  double outlineWidth;
  double widthScaleFactor;
  double rasterScaleFactor;
  QColor fill;
  QColor outline;
  QImage* image;
  QPicture* picture;
  QDateTime lastUsed;
  //content (with params replaced)
  QByteArray svgContent;

  /**Don't consider image, picture, last used timestamp for comparison*/
  bool operator==( const QgsSvgCacheEntry& other ) const;
};

/**A cache for images / pictures derived from svg files. This class supports parameter replacement in svg files
according to the svg params specification (http://www.w3.org/TR/2009/WD-SVGParamPrimer-20090616/). Supported are
the parameters 'fill-color', 'pen-color', 'outline-width', 'stroke-width'. E.g. <circle fill="param(fill-color red)" stroke="param(pen-color black)" stroke-width="param(outline-width 1)"*/
class QgsSvgCache
{
  public:

    static QgsSvgCache* instance();
    ~QgsSvgCache();

    const QImage& svgAsImage( const QString& file, double size, const QColor& fill, const QColor& outline, double outlineWidth,
                              double widthScaleFactor, double rasterScaleFactor );
    const QPicture& svgAsPicture( const QString& file, double size, const QColor& fill, const QColor& outline, double outlineWidth,
                                  double widthScaleFactor, double rasterScaleFactor );

    /**Tests if an svg file contains parameters for fill, outline color, outline width*/
    void containsParams( const QString& path, bool& hasFillParam, bool& hasOutlineParam, bool& hasOutlineWidthParam ) const;

  protected:
    QgsSvgCache();

    /**Creates new cache entry and returns pointer to it*/
    QgsSvgCacheEntry* insertSVG( const QString& file, double size, const QColor& fill, const QColor& outline, double outlineWidth,
                                 double widthScaleFactor, double rasterScaleFactor );

    void replaceParamsAndCacheSvg( QgsSvgCacheEntry* entry );
    void cacheImage( QgsSvgCacheEntry* entry );
    void cachePicture( QgsSvgCacheEntry* entry );
    /**Returns entry from cache or creates a new entry if it does not exist already*/
    QgsSvgCacheEntry* cacheEntry( const QString& file, double size, const QColor& fill, const QColor& outline, double outlineWidth,
                                 double widthScaleFactor, double rasterScaleFactor );

  private:
    static QgsSvgCache* mInstance;

    /**Entry pointers accessible by file name*/
    QMultiHash< QString, QgsSvgCacheEntry* > mEntryLookup;
    /**Estimated total size of all images, pictures and svgContent*/
    long mTotalSize;
    /**Replaces parameters in elements of a dom node and calls method for all child nodes*/
    void replaceElemParams( QDomElement& elem, const QColor& fill, const QColor& outline, double outlineWidth );

    /**Release memory and remove cache entry from mEntryLookup*/
    void removeCacheEntry( QString s, QgsSvgCacheEntry* entry );
};

#endif // QGSSVGCACHE_H
