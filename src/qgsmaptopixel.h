/***************************************************************************
                          qgsmaptopixel.h  -  description
                             -------------------
    begin                : Sat Jun 22 2002
    copyright            : (C) 2002 by Gary E.Sherman
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
#ifndef QGSMAPTOPIXEL
#define QGSMAPTOPIXEL

#include "qgspoint.h"
#include <vector>

#include <cassert>

class QgsPoint;
class QPoint;

/*! \class QgsMapToPixel
* \brief Class for doing transforms between map coordinates and device coordinates.
*
* This class can convert device coordinates to map coordinates and vice versa.
*/
class QgsMapToPixel{
 public:
 /* Constructor
 * @param mupp Map units per pixel
 * @param ymax Maximum y value of the map canvas
 * @param ymin Minimum y value of the map canvas
 * @param xmin Minimum x value of the map canvas
 */
    QgsMapToPixel(double mupp=0, double ymax = 0, double ymin=0,
			   double xmin = 0);
		 //! destructor
    ~QgsMapToPixel();
    /*! Transform the point from map (world) coordinates to device coordinates
    * @param p Point to transform
    * @return QgsPoint in device coordinates
    */
    QgsPoint transform(const QgsPoint& p);
    void transform(QgsPoint* p);
    /*! Transform the point specified by x,y from map (world) 
     * coordinates to device coordinates
     * @param x x cordinate o point to transform
     * @param y y coordinate of point to transform
     * @return QgsPoint in device coordinates
    */
    QgsPoint transform(double x, double y);
       /*! Tranform device coordinates to map (world)  coordinates
    * @param x x coordinate of point to be converted to map cooordinates
    * @param y y coordinate of point to be converted to map cooordinates
    * @return QgsPoint in map coordinates
    */

    /* Transform device coordinates to map coordinates. Modifies the
       given coordinates in place. Intended as a fast way to do the
       transform. */ 
    void transformInPlace(double& x, double& y);

    /* Transform device coordinates to map coordinates. Modifies the
       given coordinates in place. Intended as a fast way to do the
       transform. */ 
    void transformInPlace(std::vector<double>& x, 
			  std::vector<double>& y);

    QgsPoint toMapCoordinates(int x, int y);
     /*! Tranform device coordinates to map (world)  coordinates
    * @param p Point to be converted to map cooordinates
    * @return QgsPoint in map coorndiates
    */
    QgsPoint toMapCoordinates(QPoint p);
    
    QgsPoint toMapPoint(int x, int y);
    /*! Set map units per pixel
    * @param mupp Map units per pixel
    */
    void setMapUnitsPerPixel(double mupp);
    
    //! Return current map units per pixel
    double mapUnitsPerPixel();

    //! Set maximum y value
    void setYmax(double ymax);
    //! Set minimum y value
    void setYmin(double ymin);
    //! set minimum x value
    void setXmin(double xmin);
    /*! Set parameters for use in tranfsorming coordinates
    * @param mupp Map units per pixel
    * @param xmin Minimum x value
    * @param ymin Minimum y value
    * @param ymax Maximum y value
    */
    void setParameters(double mupp, double xmin, double ymin, double ymax);
    //! String representation of the parameters used in the transform
    QString showParameters();

 private:
    double mMapUnitsPerPixel;
    double yMax;
    double yMin;
    double xMin;
    double xMax;

};

inline QgsMapToPixel::QgsMapToPixel(double mupp, 
				    double ymax,
				    double ymin, 
				    double xmin)
  : mMapUnitsPerPixel(mupp), 
     yMax(ymax), 
     yMin(ymin), 
     xMin(xmin),
     xMax(0)                   // XXX wasn't originally specified?  Why?
{
}

inline QgsMapToPixel::~QgsMapToPixel()
{
}

inline QgsPoint QgsMapToPixel::transform(double x, double y)
{
  transformInPlace(x,y);
  return QgsPoint(x,y);
}

inline QgsPoint QgsMapToPixel::transform(const QgsPoint& p)
{
  double dx = p.x();
  double dy = p.y();
  transformInPlace(dx, dy);

  //std::cerr << "Point to pixel...X : " << p.x() << "-->" << dx << ", Y: " << p.y() << " -->" << dy << std::endl;
  return QgsPoint(dx, dy);
}

inline void QgsMapToPixel::transform(QgsPoint* p)
{   
  double x = p->x();
  double y = p->y();
  transformInPlace(x, y);

#ifdef QGISDEBUG 
    //std::cerr << "Point to pixel...X : " << p->x() << "-->" << x << ", Y: " << p->y() << " -->" << y << std::endl;
#endif     
  p->set(x,y);
}

inline void QgsMapToPixel::transformInPlace(double& x, double& y)
{
  x = (x - xMin) / mMapUnitsPerPixel;
  y = yMax - (y - yMin) / mMapUnitsPerPixel;
}

inline void QgsMapToPixel::transformInPlace(std::vector<double>& x, 
					    std::vector<double>& y)
{
  assert(x.size() == y.size());
  for (int i = 0; i < x.size(); ++i)
    transformInPlace(x[i], y[i]);
}
#endif // QGSMAPTOPIXEL
