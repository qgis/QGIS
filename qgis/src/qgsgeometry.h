/***************************************************************************
  qgsgeometry.h - Geometry (stored as Open Geospatial Consortium WKB)
  -------------------------------------------------------------------
Date                 : 02 May 2005
Copyright            : (C) 2005 by Brendan Morley
email                : morb at ozemail dot com dot au
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
/* $Id$ */

#ifndef QGSGEOMETRY_H
#define QGSGEOMETRY_H

#include <qstring.h>

#include <geos.h>

#include "qgsgeometryvertexindex.h"
#include "qgspoint.h"
#include "qgsrect.h"


/** 
 * Represents a geometry with input and output in formats specified by 
 * (at least) the Open Geospatial Consortium (WKB / WKT), and containing
 * various functions for geoprocessing of the geometry.
 *
 * The geometry is represented internally by the OGC WKB format, though
 * perhaps this will be migrated to GEOS geometry in future.
 *
 * @author Brendan Morley
 */

class QgsGeometry {

  public:
  

    //! Constructor
    QgsGeometry();
    
    /** copy constructor will prompt a deep copy of the object */
    QgsGeometry( QgsGeometry const & );
    
    /** assignments will prompt a deep copy of the object */
    QgsGeometry & operator=( QgsGeometry const & rhs );

    //! Destructor
    ~QgsGeometry();

    
    /** 
       Set the geometry, feeding in the buffer containing OGC Well-Known Binary and the buffer's length.
       This class will take ownership of the buffer
    */
    void setFromWkb(unsigned char * wkb, size_t length);
    
    /** 
       Returns the buffer containing this geometry in WKB format.
       You may wish to use in conjunction with wkbSize().
    */
    unsigned char * wkbBuffer() const;
    
    /** 
       Returns the size of the WKB in wkbBuffer().
    */
    size_t wkbSize() const;
    
    /** 
       Returns the QString containing this geometry in WKT format.
    */
    QString const& wkt() const; 
    
    /**
       Returns the vertex closest to the given point
    */
    QgsPoint closestVertex(const QgsPoint& point) const;
    
    /** Insert a new vertex before the given vertex index,
     *  ring and item (first number is index 0)
     *  Not meaningful for Point geometries
     */
    bool insertVertexBefore(double x, double y, QgsGeometryVertexIndex beforeVertex);

    /** Moves the vertex at the given position number,
     *  ring and item (first number is index 0)
     *  to the given coordinates
     */
    bool moveVertexAt(double x, double y, QgsGeometryVertexIndex atVertex);
    
    /**
     *  Modifies x and y to indicate the location of
     *  the vertex at the given position number,
     *  ring and item (first number is index 0)
     *  to the given coordinates
     */
    bool vertexAt(double &x, double &y, QgsGeometryVertexIndex atVertex) const;
    
    QgsPoint closestSegmentWithContext(QgsPoint& point, 
                                       QgsGeometryVertexIndex& beforeVertex,
                                       double& minSqrDist);
                            
    /**Returns the bounding box of this feature*/
    QgsRect boundingBox() const;

    /**Test for intersection with a rectangle (uses GEOS)*/
    bool intersects(QgsRect* r) const;

    /**Creates a geos geometry from this features geometry. Note, that the returned object needs to be deleted*/
    geos::Geometry* geosGeometry() const;




  private:

    /** pointer to geometry in binary WKB format
        This is the class' native implementation
     */
    unsigned char * mGeometry;

    
    /** size of geometry */
    size_t mGeometrySize;

        
    /** cached WKT version of this geometry */
    mutable QString mWkt;

    
    /** Squared distance from point to the given line segment 
     *  TODO: Perhaps move this to QgsPoint
     */
    double distanceSquaredPointToSegment(QgsPoint& point,
                                         double *x1, double *y1,
                                         double *x2, double *y2,
                                         QgsPoint& minDistPoint);
                                         
    /**Exports the current WKB to mWkt
     @return true in case of success and false else*/
    bool exportToWkt(unsigned char * geom) const;
    bool exportToWkt() const;


}; // class QgsGeometry

#endif
