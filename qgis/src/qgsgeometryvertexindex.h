/***************************************************************************
             qgsgeometry.h - Vertex Index into a QgsGeometry
             -----------------------------------------------
Date                 : 08 May 2005
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

#ifndef QGSGEOMETRYVERTEXINDEX_H
#define QGSGEOMETRYVERTEXINDEX_H

#include <vector>

#include <qstring.h>


 /**
  *  A QgsVertexIndex identifies a particular vertex in a given OGC Geometry.
  *  The left hand int (counting from 0) refers to the vertex number in
  *  the "innermost" line-string or linear-ring.
  *  The next int is the n'th line-string or linear-ring in a multi-line-string
  *  or polygon, and so on to the multi-geometry level if applicable
  *
  * @author Brendan Morley
  */

class QgsGeometryVertexIndex {

  public:
  
    //! Constructor
    QgsGeometryVertexIndex();
    
    /** copy constructor will prompt a deep copy of the object */
    QgsGeometryVertexIndex( QgsGeometryVertexIndex const & rhs );
    
    /** assignments will prompt a deep copy of the object */
    QgsGeometryVertexIndex & operator=( QgsGeometryVertexIndex const & rhs );

    //! Destructor
    ~QgsGeometryVertexIndex();
    
    /** Pushes an int onto the last (rightmost) element of the index */
    void push_back(int& i);
    
    /** Gets the last (rightmost) element of the index */
    int back();
    
    /** Resets the index */
    void clear();

    /** Decrements the last (rightmost) element of the index */
    int decrement_back();

    /** assign i to the last (rightmost) element of the index */
    void assign_back(int& i);

    /** Returns this index as a string - useful for "printf debugging" */
    QString toString();


  private:

    std::vector<int> mIndex;


}; // class QgsGeometryVertexIndex

#endif
