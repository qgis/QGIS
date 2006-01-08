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

//#include <geos.h>
#include <qstring.h>
#include <map>
#include <vector>

#include "qgis.h"

#include "qgsfeatureattribute.h"
#include "qgsgeometry.h"
//#include "qgspoint.h"

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

    /** create a copy of this feature in its uncommitted state.
        To do this, you also pass in a reference to the feature's
        layer's uncommitted attribute and geometry changes.
        The resulting feature will have those changes applied.
        
        This is useful in the cut/copy routine, where you'd
        want a copy of the "current" feature, not the on-disk feature.
     */
    QgsFeature( QgsFeature const & rhs,
                std::map<int,std::map<QString,QString> > & changedAttributes,
                std::map<int, QgsGeometry> & changedGeometries );

    /** copy ctor needed due to internal pointer */
    QgsFeature( QgsFeature const & rhs );

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
    void addAttribute(QString const & field, QString const & value = "", bool numeric = false);

    /**Deletes an attribute and its value*/
    void deleteAttribute(const QString& name);

    /**Changes an existing attribute value
       @param name attribute name
       @param newval new value*/
    void changeAttributeValue(const QString& name, const QString& newval);

    /**Changes an existing attribute name.  The value is unchanged.
       @param name attribute name
       @param newname new name*/
    void changeAttributeName(const QString& name, const QString& newname);

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
    
    /**
     * Return the dirty state of this feature.
     * Dirty is set if (e.g.) the feature's geometry has been modified in-memory.
     */
    bool isDirty() const;

    /** 
     * Reset the dirtiness of the feature.  (i.e. make clean)
     * You would normally do this after it's saved to permanent storage (e.g. disk, an ACID-compliant database)
     */
    void resetDirty();
    
    /**
     * Get the geometry object associated with this feature
     */
    QgsGeometry * geometry();
    
    /**
     * Get the geometry object associated with this feature
     * The caller assumes responsibility for the QgsGeometry*'s destruction.
     */
    QgsGeometry * geometryAndOwnership();
    
    /** gets the most recent in-memory version of the geometry (deprecated function in favour of geometry()) */
    unsigned char * getGeometry() const;

//     /** gets only the committed version of the geometry */
//     unsigned char * getCommittedGeometry() const;
//     
//     /** gets the most recent in-memory version of the geometry only
//         if it has been modified since committed (isDirty() == TRUE) */
//     unsigned char * getModifiedGeometry() const;

    size_t getGeometrySize() const;

/*    size_t getCommittedGeometrySize() const;

    size_t getModifiedGeometrySize() const;*/
    
    QString const& wellKnownText() const; 

    /** Set this feature's geometry from another QgsGeometry object (deep copy)
     */
    void setGeometry(QgsGeometry& geom);
    
    /** 
     * Set this feature's geometry from WKB
     *
     * This feature assumes responsibility for destroying geom.
     */
    void setGeometryAndOwnership(unsigned char * geom, size_t length);
    
    /** Set bulk-modified WKB geometry 
        \note   this function assumes the Geometry is not committed. 
     */
/*    void setModifiedGeometry(unsigned char * geom, size_t length);*/
    
    /** Insert a new vertex before the given vertex number,
     *  ring and item (first number is index 0)
     *  Not meaningful for Point geometries
     */
//     bool insertVertexBefore(double x, double y, int beforeVertex = 0, int atRing = 0, int atItem = 0);

    /** Moves the vertex at the given position number,
     *  ring and item (first number is index 0)
     *  to the given coordinates
     */
/*    bool moveVertexAt(double x, double y, int atVertex = 0, int atRing = 0, int atItem = 0);*/
    
    /**
     *  Modifies x and y to indicate the location of
     *  the vertex at the given position number,
     *  ring and item (first number is index 0)
     *  to the given coordinates
     */
/*    bool vertexAt(double &x, double &y, int atVertex = 0, int atRing = 0, int atItem = 0) const;*/
    
    /**Shows a popup dialog to change attribute values
     @return true if dialog is accepted, false if rejected*/
    bool attributeDialog();

//     /**Test for intersection with a rectangle (uses GEOS)*/
//     bool intersects(QgsRect* r) const;

    /**Returns the Vertex closest to a given point*/
//     QgsPoint closestVertex(const QgsPoint& point) const;

    /** Returns the line segment closest to the given point in beforeVertex, atRing and atItem
        Returns the SQUARE of the closest distance in minDist.
        Returns the closest point on the line segment to the given point
        
                
        TODO: point handling
        TODO: const correctness
     */
//    QgsPoint closestSegment(QgsPoint& point, 
//                            QgsPoint& segStart, QgsPoint& segStop,
//                            double& minSqrDist);

//     QgsPoint QgsFeature::closestSegmentWithContext(QgsPoint& point, 
//                                                    int& beforeVertex, int& atRing, int& atItem,
//                                                    double& minSqrDist);
//                             
//                             
     /**Returns the bounding box of this feature*/
     QgsRect boundingBox() const;
// 
     /** Creates a geos geometry from this features geometry. Note, that the returned object needs to be deleted.
         @note  This function is deprecated - use geometry()->geosGeometry() instead.
      */
     geos::Geometry* geosGeometry() const;

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
    QgsGeometry* mGeometry;
    
    /** Indicator if the mGeometry is owned by this QgsFeature.
        If so, this QgsFeature takes responsibility for the mGeometry's destruction.
     */ 
    bool mOwnsGeometry;   

//     /** pointer to modified (dirty / uncommitted) geometry in binary WKB format
//         This is only valid if isDirty().
//      */
//     unsigned char * modifiedGeometry;
//     
//     /** size of geometry */
//     size_t geometrySize;
// 
//     /** size of modified geometry */
//     size_t modifiedGeometrySize;

    //! Flag to indicate if this feature is valid
    bool mValid;

    //! Flag to indicate if this feature is dirty (e.g. geometry has been modified in-memory)
    bool mDirty;

    /// feature type name
    QString mTypeName;

//     /**WKT representation of the geometry*/
//     mutable QString mWKT;
// 
//     /**Exports the current WKB to mWKT
//      @return true in case of success and false else*/
//     bool exportToWKT(unsigned char * geom) const;
//     bool exportToWKT() const;
// 
//     /** Squared distance from point to the given line segment 
//      *  TODO: Perhaps move this to QgsPoint
//      */
//     double distanceSquaredPointToSegment(QgsPoint& point,
//                                          double *x1, double *y1,
//                                          double *x2, double *y2,
//                                          QgsPoint& minDistPoint);

}; // class QgsFeature

#endif
