/***************************************************************************
               qgssvgcache.h  -  SVG rendering and pixmap caching
                             -------------------
    begin                : Sat Jul 17 2004
    copyright            : (C) 2004 by Lars Luthman
    email                : larsl at users dot sourceforge dot org
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
/*  $Id$ */

#ifndef QGSSVGCACHE_H
#define QGSSVGCACHE_H

#include <map>
#include <queue>

#include <qpixmap.h>
#include <qpicture.h>


/** This class is a singleton that does all the SVG rendering in QGIS.
    When another part of the program needs to render an SVG file, it
    calls the getPixmap() function to get a pixmap rendering of that
    SVG file in the wanted scale. This class also does some extra 
    tricks to make Qt's SVG renderings look a little bit better, and
    it caches the pixmaps to speed things up.
*/
class QgsSVGCache {
 public:
  
  QgsSVGCache();
  
  /** This function returns a pixmap containing a rendering of the SVG file
      @c filename scaled by the factor @c scaleFactor. The pixmaps background
      will be transparent, and if the file for some reason can't be rendered
      the pixmap will be completely transparent. */
  QPixmap getPixmap(QString filename, double scaleFactor);
  
  /** Returns SVG picture in original size */
  QPicture getPicture(QString filename);
  
  /** This function clears the pixmap cache and forces all newly requested
      pixmaps to be rendered again. Can be useful if the oversampling factor
      has changed and you want to get rid of cached lower-quality pixmaps. */
  void clear();
  
  /** When a pixmap is requested and it is not in the cache, it will be 
      loaded using a QPicture and rendered to a QPixmap. Since Qt does not do
      any kind of anti-aliasing or smoothing, the QPicture is first rendered
      to a pixmap that is larger than the requested size, and then it's scaled
      down using QImage::smoothScale() to smooth out sharp edges and lines.
      The oversampling factor determins how much larger the first pixmap is -
      the larger this factor is the smoother the final pixmap will look, but
      it will also take longer time to render and scale the image. This
      function sets the oversampling factor. It should be larger than or equal
      to 1. 
  */
  void setOversampling(int oversamplingFactor);
  
  /** This function returns the oversampling factor.
      @see setOversampling()
  */
  int getOversampling() const;
  
  /** This function returns a reference to the singleton object. */
  static inline QgsSVGCache& instance();
  
 protected:
  
  typedef std::map<QString, QPicture> PictureMap;
  PictureMap pictureMap;

  typedef std::map<std::pair<QString, double>, QPixmap> PixmapMap;
  PixmapMap pixmapMap;
  int oversampling;
  std::queue<PixmapMap::key_type> fifo;
  int pixelLimit;
  int totalPixels;
  
};


QgsSVGCache& QgsSVGCache::instance() {
  static QgsSVGCache obj;
  return obj;
}

#endif
