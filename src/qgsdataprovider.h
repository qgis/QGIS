/***************************************************************************
    qgsdataprovider.h - DataProvider Interface class
     --------------------------------------
    Date                 : 09-Sep-2003
    Copyright            : (C) 2003 by Gary E.Sherman
    email                : sherman at mrcc.com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
/* $Id$ */

#ifndef QQGSDATAPROVIDER_H
#define QQGSDATAPROVIDER_H
#include <qstring.h>
#include <vector>
#include <list>
class QgsRect;
class QgsFeature;
class QgsField;
/** \class QgsDataProvider
* \brief Abstract base class for spatial data provider implementations
  *@author Gary E.Sherman
  */

class QgsDataProvider {

public: 
  /**
  * We need this so the subclass destructors get called
  */
  virtual ~QgsDataProvider() {};

  /**
  * Select features based on a bounding rectangle. Features can be retrieved 
  * with calls to getFirstFeature and getNextFeature. Request for features 
  * for use in drawing the map canvas should set useIntersect to false.
  * @param mbr QgsRect containing the extent to use in selecting features
  * @param useIntersect If true, use the intersects function to select features
  * rather than the PostGIS && operator that selects based on bounding box
  * overlap.
  *
  */
  virtual void QgsDataProvider::select(QgsRect *mbr, bool useIntersect=false)=0;
  /** 
    * Set the data source specification. This may be a path or database
  * connection string
  * @param data source specification
  */
  virtual void QgsDataProvider::setDataSourceUri(QString uri) = 0;
  
    /** 
  * Get the data source specification. This may be a path or database
  * connection string
  * @return data source specification
  */
  virtual QString QgsDataProvider::getDataSourceUri() = 0;

  /**
  * Get the extent of the layer
  * @return QgsRect containing the extent of the layer
  */
  virtual QgsRect * QgsDataProvider::extent() = 0;
    
  /**
  * Identify features within the search radius specified by rect
  * @param rect Bounding rectangle of search radius
  * @return std::vector containing QgsFeature objects that intersect rect
  */
  //virtual std::vector<QgsFeature>& QgsDataProvider::identify(QgsRect *rect)=0;

   /**
   * Return the endian of this layer.
   * @return 0 for NDR (little endian), 1 for XDR (big endian
   */
  virtual int QgsDataProvider::endian()=0;

  /**
   * Returns true if this is a valid layer. It is up to individual providers
   * to determine what constitutes a valid layer
   */
  virtual bool isValid()=0;

  /**
     Enables editing capabilities of the provider (if supported)
     @return false in case of error or if the provider does not support editing
  */
  virtual bool startEditing()=0;

  /**
     Disables the editing capabilities of the provider
  */
  virtual void stopEditing()=0;

  /**
     Commits changes
     @return false in case of problems
  */
  virtual bool commitChanges()=0;

  /**
     Discards changes
     @return false in case of problems
  */
  virtual bool rollBack()=0;

  /**Returns true if the provider is in editing mode*/
  virtual bool isEditable() const=0;

  /**Returns true if the provider has been modified since the last commit*/
  virtual bool isModified() const=0;

};


#endif

