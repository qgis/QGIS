/***************************************************************************
                          QgsCoordinateTransform.cpp  -  description
                             -------------------
    begin                : Dec 2004
    copyright            : (C) 2004 Tim Sutton
    email                : tim at linfiniti.com
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
#ifndef QGSCOORDINATETRANSFORM_H
#define QGSCOORDINATETRANSFORM_H

#include "qgspoint.h"
#include "qgscsexception.h"
#include <ogr_api.h>
#include <ogr_spatialref.h>
#include <cpl_error.h>

class QgsPoint;
class QPoint;
class QString;

/*! \class QgsCoordinateTransform
* \brief Class for doing transforms between two map coordinate systems.
*
* This class can convert map coordinates to a different spatial reference system.
*/
class QgsCoordinateTransform{
 public:

    QgsMapToPixel(QString theSourceWKT, QString theDestWKT  );
		 //! destructor
    ~QgsMapToPixel();
    
    /*! Transform the point from Source Coordinate System to Destination Coordinate System
    * @param p Point to transform
    * @return QgsPoint in Destination Coordinate System
    */    
    QgsPoint transform(QgsPoint p);
    
    /*! Transform the point specified by x,y from Source Coordinate System to Destination Coordinate System
    * @param x x cordinate o point to transform
    * @param y y coordinate of point to transform
    * @return QgsPoint in Destination Coordinate System
    */
    QgsPoint transform(double x, double y);

    QString showParameters();
 private:
    OGRCreateCoordinateTransformation gSourceToDestXForm;
    bool gInputIsDegrees;
};

inline QgsCoordinateTransform::QgsCoordinateTransform( QString theSourceWKT, QString theDestWKT )
{
  OGRSpatialReference myInputSpatialRefSys, myOutputSpatialRefSys;
 
  if ( myInputSpatialRefSys.importFromWkt( &theSourceWKT ) != OGRERR_NONE ||
       myOutputSpatialRefSys.importFromWkt( &theDestWKT ) != OGRERR_NONE )
    {
      printf( 1, "QgsCoordinateTransform - invalid projection:\n myInputSpatialRefSys: (%s)\n myOutputSpatialRefSys: (%s)\n",
		   theSourceWKT, theDestWKT );
    }

  gSourceToDestXForm = OGRCreateCoordinateTransformation( &myInputSpatialRefSys, &myOutputSpatialRefSys );

  if ( ! gSourceToDestXForm )
    {
      printf( 1, "QgsCoordinateTransform - invalid projection:\n myInputSpatialRefSys: (%s)\n myOutputSpatialRefSys: (%s)\n",
		   theSourceWKT, theDestWKT );
    }

  // Deactivate GDAL error messages.
  //CPLSetErrorHandler( errorHandler );

  // Guess if the source o dest CS is in degrees.
  //Searchf for this phrase in each wkt:  "unit[\"degree\"" 
}

inline QgsMapToPixel::~QgsMapToPixel()
{
  delete gSourceToDestXForm;
}
inline QgsPoint QgsMapToPixel::transform(double x, double y)
{
	return (transform(QgsPoint(x, y)));
}

inline QgsPoint QgsMapToPixel::transform(QgsPoint thePoint) throws QgsCsException
{
	// transform x
	double x = thePoint.x(); 
	double y = thePoint.y();
	if ( ! gSourceToDestXForm->Transform( 1, &x, &y ) )
	{
	  //something bad happened....
	  throw QgsCsException("Coordinate transform failed";
	}
	else
	{
	  return QgsPoint(x, y);
	}	
}
inline QgsPoint QgsMapToPixel::transform(double theX, double theY)
{
	// transform x
	double x = theX; 
	double y = theY;
	if ( ! gSourceToDestXForm->Transform( 1, &x, &y ) )
	{
	  //something bad happened....
	  return null;
	}
	else
	{
	  return QgsPoint(x, y);
	}	
}
#endif // QGSCSTRANSFORM_H
