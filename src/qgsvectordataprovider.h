#ifndef QGSVECTORDATAPROVIDER_H
#define QGSVECTORDATAPROVIDER_H

/***************************************************************************
    qgsvectordataprovider.h - DataProvider Interface for vector layers
     --------------------------------------
    Date                 : 23-Sep-2004
    Copyright            : (C) 200 by Marco Hugentobler
    email                : marco.hugentobler@autoform.ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsdataprovider.h"

/**Interface class for vector data providers*/
class QgsVectorDataProvider: public QgsDataProvider
{
 public:
    virtual ~QgsVectorDataProvider() {};

    /** 
     * Get the first feature resulting from a select operation
     * @return QgsFeature
     */
    virtual QgsFeature * getFirstFeature(bool fetchAttributes=false)=0;

    /** 
     * Get the next feature resutling from a select operation
     * @return QgsFeature
     */
    virtual QgsFeature * getNextFeature(bool fetchAttributes=false)=0;

    /**Get the next feature resulting from a select operation.
     *@param attlist a list containing the indexes of the attribute fields to copy
     */
    virtual QgsFeature * getNextFeature(std::list<int>& attlist)=0;

    /**
     * Get the next feature using new method
     * TODO - make this pure virtual once it works and change existing providers
     *        to use this method of fetching features
     */

    virtual bool getNextFeature(QgsFeature &feature, bool fetchAttributes=false)=0;

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

    /**Adds a feature (but does not commit it)
       @return true in case of success and false in case of failure*/
    virtual bool addFeature(QgsFeature* f)=0;

    /**Deletes a feature
       @param id the number of the feature
       @return true in case of success and false in case of failure*/
    virtual bool deleteFeature(int id)=0;

    /**
     * Identify features within the search radius specified by rect
     * @param rect Bounding rectangle of search radius
     * @return std::vector containing QgsFeature objects that intersect rect
     */
    virtual std::vector<QgsFeature>& identify(QgsRect *rect)=0;
 protected:

    /**Flag indicating wheter the provider is in editing mode or not*/
    bool mEditable;
};

#endif
