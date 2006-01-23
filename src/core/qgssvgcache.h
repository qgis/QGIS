/***************************************************************************
               qgssvgcache.h  -  SVG rendering and pixmap caching
                             -------------------
    begin                : Sat Jul 17 2004
    copyright            : (C) 2004 by Lars Luthman and Tim Sutton 2006
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
#include <QSvgRenderer>

/** This class is a singleton that does all the SVG rendering in QGIS.
    When another part of the program needs to render an SVG file, it
    calls the getPixmap() function to get a pixmap rendering of that
    SVG file in the wanted scale. This class caches the pixmaps to 
    speed things up.
*/
class QgsSVGCache {
 public:
  
  QgsSVGCache();
  ~QgsSVGCache();
  
  /** This function returns a pixmap containing a rendering of the SVG file
      @c filename scaled by the factor @c scaleFactor. The pixmaps background
      will be transparent, and if the file for some reason can't be rendered
      the pixmap will be completely transparent. */
  QPixmap getPixmap(QString filename, double scaleFactor);
  
  /** Returns SVG renderer given its filename */
  QSvgRenderer * getPicture(QString filename);
  
  /** This function clears the pixmap cache and forces all newly requested
      pixmaps to be rendered again. Can be useful if the oversampling factor
      has changed and you want to get rid of cached lower-quality pixmaps. */
  void clear();
  
  
  /** This function returns a reference to the singleton object. */
  static inline QgsSVGCache& instance();
  
 protected:
  
  typedef std::map<QString, QSvgRenderer * > PictureMap;
  PictureMap pictureMap;
  typedef std::map<std::pair<QString, double>,QPixmap> PixmapMap;
  PixmapMap pixmapMap;
  std::queue<std::pair<QString, double> > fifo;
  int pixelLimit;
  int totalPixels;
  
};


QgsSVGCache& QgsSVGCache::instance() {
  static QgsSVGCache obj;
  return obj;
}

#endif
