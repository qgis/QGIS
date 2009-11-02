/***************************************************************************
                          Triangulation.h  -  description
                             -------------------
    copyright            : (C) 2004 by Marco Hugentobler
    email                : mhugent@geo.unizh.ch
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef TRIANGULATION_H
#define TRIANGULATION_H

#include <QList>
#include "Line3D.h"
#include "Vector3D.h"
#include <qpainter.h>
#include "Line3D.h"
#include <TriangleInterpolator.h>

/**Interface for Triangulation classes*/
class ANALYSIS_EXPORT Triangulation
{
  public:
    /**Enumeration describing the behaviour, if two forced lines cross. SnappingType_VERTICE means, that the second inserted forced line is snapped to the closest vertice of the first inserted forced line. DELETE_FIRST means, that the status of the first inserted forced line is reset to that of a normal edge (so that the second inserted forced line remain and the first not*/
    enum forcedCrossBehaviour {SnappingType_VERTICE, DELETE_FIRST, INSERT_VERTICE};
    virtual ~Triangulation();
    /**Adds a line (e.g. a break-, structure- or an isoline) to the triangulation. The class takes ownership of the line object and its points*/
    virtual void addLine( Line3D* line, bool breakline ) = 0;
    /**Adds a point to the triangulation*/
    virtual int addPoint( Point3D* p ) = 0;
    /**Calculates the normal at a point on the surface and assigns it to 'result'. Returns true in case of success and flase in case of failure*/
    virtual bool calcNormal( double x, double y, Vector3D* result ) = 0;
    /**Performs a consistency check, remove this later*/
    virtual void performConsistencyTest() = 0;
    /**Calculates x-, y and z-value of the point on the surface and assigns it to 'result'. Returns true in case of success and flase in case of failure*/
    virtual bool calcPoint( double x, double y, Point3D* result ) = 0;
    /**Returns a pointer to the point with number i. Any virtual points must have the number -1*/
    virtual Point3D* getPoint( unsigned int i ) const = 0;
    /**Finds out, in which triangle the point with coordinates x and y is and assigns the numbers of the vertices to 'n1', 'n2' and 'n3' and the vertices to 'p1', 'p2' and 'p3'*/
    virtual bool getTriangle( double x, double y, Point3D* p1, int* n1, Point3D* p2, int* n2, Point3D* p3, int* n3 ) = 0;
    /**Finds out, in which triangle the point with coordinates x and y is and assigns the  points at the vertices to 'p1', 'p2' and 'p3*/
    virtual bool getTriangle( double x, double y, Point3D* p1, Point3D* p2, Point3D* p3 ) = 0;
    /**Returns the number of the point opposite to the triangle points p1, p2 (which have to be on a halfedge)*/
    virtual int getOppositePoint( int p1, int p2 ) = 0;
    /**Returns the largest x-coordinate value of the bounding box*/
    virtual double getXMax() const = 0;
    /**Returns the smallest x-coordinate value of the bounding box*/
    virtual double getXMin() const = 0;
    /**Returns the largest y-coordinate value of the bounding box*/
    virtual double getYMax() const = 0;
    /**Returns the smallest x-coordinate value of the bounding box*/
    virtual double getYMin() const = 0;
    /**Returns the number of points*/
    virtual int getNumberOfPoints() const = 0;
    /**Returns a pointer to a value list with the information of the triangles surrounding (counterclockwise) a point. Four integer values describe a triangle, the first three are the number of the half edges of the triangle and the fourth is -10, if the third (and most counterclockwise) edge is a breakline, and -20 otherwise. The value list has to be deleted by the code which called the method. Any virtual point needs to have the number -1*/
    virtual QList<int>* getSurroundingTriangles( int pointno ) = 0;
    /**Returns a value list with the numbers of the four points, which would be affected by an edge swap. This function is e.g. needed by NormVecDecorator to know the points, for which the normals have to be recalculated. The list has to be deleted by the code which calls this method*/
    virtual QList<int>* getPointsAroundEdge( double x, double y ) = 0;
    /**draws the points, edges and the forced lines*/
    //virtual void draw(QPainter* p, double xlowleft, double ylowleft, double xupright, double yupright, double width, double height) const=0;
    /**Sets the behaviour of the triangulation in case of crossing forced lines*/
    virtual void setForcedCrossBehaviour( Triangulation::forcedCrossBehaviour b ) = 0;
    /**Sets the color of the normal edges*/
    virtual void setEdgeColor( int r, int g, int b ) = 0;
    /**Sets the color of the forced edges*/
    virtual void setForcedEdgeColor( int r, int g, int b ) = 0;
    /**Sets the color of the breaklines*/
    virtual void setBreakEdgeColor( int r, int g, int b ) = 0;
    /**Sets an interpolator object*/
    virtual void setTriangleInterpolator( TriangleInterpolator* interpolator ) = 0;
    /**Eliminates the horizontal triangles by swapping*/
    virtual void eliminateHorizontalTriangles() = 0;
    /**Adds points to make the triangles better shaped (algorithm of ruppert)
     \param tin the triangulation or decorator which interpolates the elevation*/
    virtual void ruppertRefinement() = 0;
    /**Returns true, if the point with coordinates x and y is inside the convex hull and false otherwise*/
    virtual bool pointInside( double x, double y ) = 0;
    /**Reads the content of a taff-file*/
    //virtual bool readFromTAFF(QString fileName)=0;
    /**Saves the content to a taff file*/
    //virtual bool saveToTAFF(QString fileName) const=0;
    /**Swaps the edge which is closest to the point with x and y coordinates (if this is possible)*/
    virtual bool swapEdge( double x, double y ) = 0;
    /**Saves the triangulation as a (line) shapefile
    @return true in case of success*/
    virtual bool saveAsShapefile( const QString& fileName ) const = 0;
};

inline Triangulation::~Triangulation()
{

}

#endif
