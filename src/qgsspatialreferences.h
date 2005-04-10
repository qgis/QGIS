/***************************************************************************
                    qgsspatialreferencesystems.h  -  Singleton class for
                    Spatial reference systems
                             -------------------
    begin                : 2005-01-22
    copyright            : (C) 2005 by Gary E.Sherman
    email                : sherman at mrcc.com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
 /* $Id$ */
 
#ifndef QGSSPATIALREFERENCESYSTEMS_H
#define QGSSPATIALREFERENCESYSTEMS_H
#include <qmap.h>
#include "qgsspatialrefsys.h"
class QString;
//! Typedef for map containing spatial reference system objects
typedef QMap<QString,QgsSpatialRefSys*>  projectionWKTMap_t ; //wkt = well known text (see gdal/ogr)
/*!
 * \class QgsSpatialReferences
 * \brief Singleton class to manage spatial reference systems
 */
class QgsSpatialReferences
{
public:
  /*!
   * Function to return the instance pointer to the singleton QgsSpatialReferences
   * class. On first call to instance(), the object is constructed and the spatial
   * references read from the share/qgis/resources/spatial_ref_sys.txt
   * @return Pointer to the one and only QgsSpatialReferences instance
   */
 static QgsSpatialReferences* instance();
 /*!
  * Get a spatial reference system using its spatial reference id (SRID)
  * @return Pointer to a spatial reference system matching the specified SRID
  */
 QgsSpatialRefSys * getSrsBySrid(QString srid);
 /*!
  * Get a spatial reference system using its well known text (WKT) specification.
  * @return Pointer to a spatial reference system matching the specified WKT
  */
 QgsSpatialRefSys * getSrsByWkt(QString &wkt);
 /*!
  * Get the map containing the collection of spatial reference systems
  * @return The map of spatial reference systems
  */
 projectionWKTMap_t getMap();

protected:
 //! Protected constructor
 QgsSpatialReferences();
private:
 //! Instance member pointer
 static QgsSpatialReferences* _instance;
 //! Map containing the spatial reference systems read from the spatial_ref_sys file
projectionWKTMap_t mSpatialReferences; 
};
#endif // QGSSPATIALREFERENCESYSTEMS_H

