/***************************************************************************
                          qgscoordinatetransform.h  -  description
                             -------------------
    begin                : Sat Jun 22 2002
    copyright            : (C) 2002 by Gary E.Sherman
    email                : sherman@mrcc.com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSCOORDINATETRANSFORM_H
#define QGSCOORDINATETRANSFORM_H
class QgsPoint;
class QPoint;
/* \class QgsCoordinateTransform
* \brief Class for doing transforms between map coordinates and device coordinates.
*
* This class can convert device coordinates to map coordinates and vice versa.
*/
class QgsCoordinateTransform{
 public:
    QgsCoordinateTransform(double mupp=0, double ymax = 0, double ymin=0,
			   double xmin = 0);
    ~QgsCoordinateTransform();
    /*! Transform the point from map (world) coordinates to device coordinates
    *@ param p Point to transform
    * @return QgsPoint in device coordinates
    */
    QgsPoint transform(QgsPoint p);
    /*! Transform the point specified by x,y from map (world) coordinates to device coordinates
    *@ param x x cordinate o point to transform
	*@param y y coordinate of point to transform
    * @return QgsPoint in device coordinates
    */
    QgsPoint transform(double x, double y);
       /*! Tranform device coordinates to map (world)  coordinates
    * @param x x coordinate of point to be converted to map cooordinates
    * @param y y coordinate of point to be converted to map cooordinates
    * @return QgsPoint in map coordinates
    */
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
    double mapUnitsPerPixel;
    double yMax;
    double yMin;
    double xMin;
    double xMax;

};
#endif // QGSCOORDINATETRANSFORM_H
