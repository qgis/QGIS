/***************************************************************************
                      qgsfeature.h - Spatial Feature Class
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

#ifndef QGSFEATURE_H
#define QGSFEATURE_H

#include <qstring.h>
#include <map>
#include <vector>

#include "qgsfeatureattribute.h"
#include "qgis.h"
#include "qgspoint.h"

class QgsRect;

/** \class QgsFeature - Feature attribute class.
 * Encapsulates a single feature including id and field/value.
 *@author Gary E.Sherman
 */

class QgsFeature {

  public:

    //! Constructor
    QgsFeature();
    QgsFeature(int id, QString const & typeName = "" );


    /** copy ctor needed due to internal pointer */
    QgsFeature( QgsFeature const & );

    /** assignment operator needed due to internal pointer */
    QgsFeature & operator=( QgsFeature const & rhs );

    //! Destructor
    ~QgsFeature();

    /**
     * Get the feature id for this feature
     * @return Feature id
     */
    int featureId() const;

    /**
     * Set the feature id for this feature
     * @param id Feature id
     */
     void setFeatureId(int id);


    /** returns the feature's type name
     */
     QString const & typeName() const;


    /** sets the feature's type name
     */
     void typeName( QString const & typeName );

    /**
     * Get the attributes for this feature.
     * @return A std::map containing the field name/value mapping
     */
    const std::vector<QgsFeatureAttribute>& attributeMap();

    /** 
     * Add an attribute to the map
     */
    void addAttribute(QString const & field, QString const & value = "");

    /**Deletes an attribute and its value*/
    void deleteAttribute(const QString& name);

    /**Changes an existing attribute value
       @param name attribute name
       @param newval new value*/
    void changeAttributeValue(const QString& name, const QString& newval);

    /**
     * Get the fields for this feature
     * @return A std::map containing field position (index) and field name
     */
    const std::map<int, QString>& fields();

    /**
     * Return the validity of this feature. This is normally set by
     * the provider to indicate some problem that makes the feature
     * invalid or to indicate a null feature.
     */
    bool isValid() const;

    /** 
     * Set the validity of the feature.
     */
    void setValid(bool validity);

    unsigned char * getGeometry() const;

    size_t getGeometrySize() const;

    QString const& wellKnownText() const; 

    /** Set WKB geometry*/
    void setGeometry(unsigned char * geometry, size_t length);

    /**Shows a popup dialog to change attribute values*/
    void attributeDialog();

    /**Test for intersection with a rectangle (uses GEOS)*/
    bool intersects(QgsRect* r);

    /**Returns the Vertex closest to a given point*/
    QgsPoint closestVertex(const QgsPoint& point);

  private:

    //! feature id
    int mFid;

    //! std::map containing field name/value pairs
    std::vector<QgsFeatureAttribute> attributes;

    //! std::map containing the field index and name
    std::map<int, QString> fieldNames;

    /** pointer to geometry in binary WKB format

       This is usually set by a call to OGRGeometry::exportToWkb()
     */
    unsigned char * geometry;

    /** size of geometry */
    size_t geometrySize;

    //! Flag to indicate if this feature is valid
    bool mValid;

    /// feature type name
    QString mTypeName;

    /**WKT representation of the geometry*/
    mutable QString mWKT;

    /**Exports the current WKB to mWKT
     @return true in case of success and false else*/
    bool exportToWKT() const;

}; // class QgsFeature

#endif
