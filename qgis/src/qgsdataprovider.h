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
class QgsDataSourceURI;
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
  virtual void select(QgsRect *mbr, bool useIntersect=false)=0;
  /** 
    * Set the data source specification. This may be a path or database
  * connection string
  * @param data source specification
  */
  virtual void setDataSourceUri(QString uri) = 0;
  
    /** 
  * Get the data source specification. This may be a path or database
  * connection string
  * @return data source specification
  */
  virtual QString getDataSourceUri() = 0;

  virtual QgsDataSourceURI * getURI()=0;
  /**
  * Get the extent of the layer
  * @return QgsRect containing the extent of the layer
  */
  virtual QgsRect * extent() = 0;
    
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
  virtual int endian()=0;

  /**
   * Returns true if this is a valid layer. It is up to individual providers
   * to determine what constitutes a valid layer
   */
  virtual bool isValid()=0;

  /* Reset the layer - for an OGRLayer, this means clearing the
   * spatial filter and calling ResetReading
   */
  virtual void reset()
  { 
     // NOP by default 
  }
  /**
   * Update the feature count based on current spatial filter. If not
   * overridden in the data provider this function returns -1
   */
  virtual long updateFeatureCount()
  {
    return -1;
  }
  /**
   * Update the extents of the layer. Not implemented by default
   */
  virtual void updateExtents()
  {
    // NOP by default
  }
  /**
   * Set the subset string used to create a subset of features in
   * the layer. This may be a sql where clause or any other string
   * that can be used by the data provider to create a subset.
   * Must be implemented in the dataprovider.
   */
  virtual void setSubsetString(QString subset)
  {
    // NOP by default
  }
/**
 * Returns the subset definition string (typically sql) currently in
 * use by the layer and used by the provider to limit the feature set.
 * Must be overridden in the dataprovider, otherwise returns a null
 * QString.
 */
  virtual QString subsetString()
  {
    return QString::null;
  }
    
};


#endif

