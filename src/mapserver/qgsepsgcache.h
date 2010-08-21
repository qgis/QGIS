/***************************************************************************
                              qgsepsgcache.h
                              --------------
  begin                : June 9th, 2010
  copyright            : (C) 2010 by Marco Hugentobler
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

#ifndef QGSEPSGCACHE_H
#define QGSEPSGCACHE_H

#include "qgscoordinatereferencesystem.h"
#include <QHash>

/**A class that cashes QgsCoordinateReferenceSystem instances and allows fast searching by epsg numbers*/
class QgsEPSGCache
{
  public:
    static QgsEPSGCache* instance();
    ~QgsEPSGCache();
    /**Returns the CRS for an epsg number (or an invalid CRS in case of error)*/
    const QgsCoordinateReferenceSystem& searchCRS( long epsg );

  protected:
    QgsEPSGCache();

  private:
    static QgsEPSGCache* mInstance;
    QHash< long, QgsCoordinateReferenceSystem > mCRS;
    /**CRS that is not initialised (is returned in case of error)*/
    QgsCoordinateReferenceSystem mInvalidCRS;
};

#endif // QGSEPSGCACHE_H
