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
	* Get the first feature resulting from a select operation
	* @return QgsFeature
	*/
	virtual QgsFeature * QgsDataProvider::getFirstFeature(bool fetchAttributes=false)=0;
	/** 
	* Get the next feature resutling from a select operation
	* @return QgsFeature
	*/
	virtual QgsFeature * QgsDataProvider::getNextFeature(bool fetchAttributes=false)=0;

	/** Get feature type.
	* Gets the feature type as defined in WKBTYPE (qgis.h).
	* @return int representing the feature type
	*/
	virtual int geometryType()=0; 
    /**
    * Number of features in the layer
    * @return long containing number of features
    */
    virtual long featureCount()=0;
    /**
    * Number of attribute fields for a feature in the layer
    */
  virtual int fieldCount()=0;
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
	virtual std::vector<QgsFeature>& QgsDataProvider::identify(QgsRect *rect)=0;

   /**
   * Return the endian of this layer.
   * @return 0 for NDR (little endian), 1 for XDR (big endian
   */
	virtual int QgsDataProvider::endian()=0;

  /**
  * Return a list of field names for this layer
  * @return vector of field names
  */
  virtual std::vector<QgsField>& fields()=0;
  
/** 
* Reset the layer to clear any spatial filtering or other contstraints that
* would prevent the entire record set from being traversed by call to 
* getNextFeature(). Some data stores may not require any special action to
* reset the layer. In this case, the provider should simply implement an empty
* function body.
*/
  virtual void reset()=0;

  /**Returns the minimum value of an attributs
     @param position the number of the attribute*/
  virtual QString minValue(int position)=0;

  /**Returns the maximum value of an attributs
     @param position the number of the attribute*/
  virtual QString maxValue(int position)=0;

/**
* Returns true if this is a valid layer. It is up to individual providers
* to determine what constitutes a valid layer
*/
virtual bool isValid()=0;
};
#endif

