/***************************************************************************
                             qgsacetatelines.h
                    A collection of lines that can be drawn 
                     on the acetate layer of a QgsMapCanvas
                              -------------------
  begin                : 2004-10-24
  copyright            : (C) 2004 by Gary E.Sherman
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

#ifndef QGSACETATELINES_H
#define QGSACETATELINES_H

class QgsPoint;
class QgsLine;
class QPainter;
class QgsMapToPixel;

#include <vector>
//#include "qgsline.h"
#include "qgsacetateobject.h"

/** \class QgsAcetateLines
 * \brief A collection of lines drawn on the acetate layer of a map canvas
 *
 * An acetate object is a graphic or text object that is drawn on top of the map canvas 
 * after rendering of all map elements is completed. Acetate objects can be drawn in
 * device coordinates or map coordinates. Drawing in map coordinates requires passing
 * a QgsMapToPixel object to the draw function.
 *
 */ 
class QgsAcetateLines : public QgsAcetateObject {
  public:
    /**
     * Constructor
     */
    QgsAcetateLines();
    /** 
     * Destructor
     */
    ~QgsAcetateLines();
    /**
     * Draw the collection of lines using the Qpainter and applying a
     * coordinate transform if specified.  
     * @param painter Painter to use for drawing 
     * @param cXf Coordinate transform to use in drawing map coordinate
     * on the device. If this parameter is not specified, coordinates are
     * assumed to be device coordinates
     * rather than map coordinates.
     */
    void  draw (QPainter * painter, QgsMapToPixel * cXf=0);
    /** 
     * Set the origin point
     * @param value Point of origin
     */
    void setOrigin (QgsPoint value );
    /**
     * Returns the point of origin
     */
    QgsPoint origin();
    /**
     * Returns the line collection
     */
    std::vector<QgsLine> * lines();
    /** Sets the line collection
     * @param col The line collection (vector)
     */
    void setLines(std::vector<QgsLine> *lineCol);
    /** Add a line to the line collection
     * @param QgsLine to add to the collection
     */
    void add(QgsLine &line);
  private:
    //! Origin of the object in device or map coordinates 
    QgsPoint mOrigin;
    //! Lines collection
    std::vector<QgsLine> *mLineCollection;
};
#endif //QGSACETATELINES_H

