/***************************************************************************
                               qgsacetateobject.h
  An object that can be drawn on the acetate layer of a QgsMapCanvas
                              -------------------
  begin                : June 10, 2004
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

#ifndef QGSACETATEOBJECT_H
#define QGSACETATEOBJECT_H
class QgsPoint;
class QPainter;
class QgsCoordinateTransform;

/*! \class QgsAcetateObject
* \brief Base class for all objects that are drawn on the acetate layer of a map canvas.
*
* An acetate object is a graphic or text object that is drawn on top of the map canvas 
* after rendering of all map elements is completed. Acetate objects can be drawn in
* device coordinates or map coordinates. Drawing in map coordinates requires passing
* a QgsCoordinateTransform object to the draw function.
*
* The draw function must be overridden in a subclass to provide the specfic logic for
* drawing the object.
*/ 
class QgsAcetateObject {
public:
  /**
   * Constructor. Constructs an object with the specified origin. If the object is
   * spatially referenced, the origin should be in map coordinates.
   */
  QgsAcetateObject(QgsPoint &origin);
	/** 
	 * Destructor
	 */
	~QgsAcetateObject();

  /**
   * Draw the object using the Qpainter and applying a coordinate transform if
   * specified.
   * @param painter Painter to use for drawing
   * @param cXf Coordinate transform to use in drawing map coordinate on the device. If 
   * this parameter is not specified, coordinates are assumed to be device coordinates
   * rather than map coordinates.
   */
  virtual void  draw (QPainter * painter, QgsCoordinateTransform * cXf=0)=0;
   /** 
    * Set the origin point
    * @param value Point of origin
    */
  void setOrigin (QgsPoint value );
 /**
  * Returns the point of origin
  */
  QgsPoint origin();

private:
  //! Origin of the object in device or map coordinates 
   QgsPoint mOrigin;
};
#endif //QGSACETATEOBJECT_H

