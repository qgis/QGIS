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
class QString;

/*! \class QgsCoordinateTransform
* \brief Class for doing transforms between two map coordinate systems.
*
* This class can convert map coordinates to a different spatial reference system.
*/
class QgsCoordinateTransform{
 public:

    QgsCoordinateTransform(QString theSourceWKT, QString theDestWKT  );
     //! destructor
    ~QgsCoordinateTransform();
    
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
    OGRCoordinateTransformation * mSourceToDestXForm;
    bool mInputIsDegrees;
    //set to true if src cs  == dest cs
    bool mShortCircuit;
};

inline QgsCoordinateTransform::QgsCoordinateTransform( QString theSourceWKT, QString theDestWKT )
{
  //default to geo / wgs84 for now .... later we will make this user configurable
  QString myGeoWKT =    "GEOGCS[\"WGS 84\", "
    "  DATUM[\"WGS_1984\", "
    "    SPHEROID[\"WGS 84\",6378137,298.257223563, "
    "      AUTHORITY[\"EPSG\",7030]], "
    "    TOWGS84[0,0,0,0,0,0,0], "
    "    AUTHORITY[\"EPSG\",6326]], "
    "  PRIMEM[\"Greenwich\",0,AUTHORITY[\"EPSG\",8901]], "
    "  UNIT[\"DMSH\",0.0174532925199433,AUTHORITY[\"EPSG\",9108]], "
    "  AXIS[\"Lat\",NORTH], "
    "  AXIS[\"Long\",EAST], "
    "  AUTHORITY[\"EPSG\",4326]]";
  if (theSourceWKT.isEmpty())
  {
   theSourceWKT = myGeoWKT;
  }
  if (theDestWKT.isEmpty())
  {
    theDestWKT = myGeoWKT;
  }  
  
  if (theSourceWKT == theDestWKT)
  {
    mShortCircuit=true;
    return;
  }
  else
  {
    mShortCircuit=false;
  }
  
  OGRSpatialReference myInputSpatialRefSys, myOutputSpatialRefSys;
  //this is really ugly but we need to get a QString to a char**
  const char * mySourceCharArray =  theSourceWKT.ascii();
  char *mySourceCharArrayPointer = (char *)mySourceCharArray;
  const char * myDestCharArray =  theDestWKT.ascii();
  char *myDestCharArrayPointer = (char *)myDestCharArray;
  if ( myInputSpatialRefSys.importFromWkt( & mySourceCharArrayPointer ) != OGRERR_NONE ||
       myOutputSpatialRefSys.importFromWkt( & myDestCharArrayPointer ) != OGRERR_NONE )
    {
      printf( "QgsCoordinateTransform - invalid projection:\n myInputSpatialRefSys: (%s)\n myOutputSpatialRefSys: (%s)\n",
       theSourceWKT.ascii(), theDestWKT.ascii() );
    }

  mSourceToDestXForm = OGRCreateCoordinateTransformation( &myInputSpatialRefSys, &myOutputSpatialRefSys );

  if ( ! mSourceToDestXForm )
    {
      printf(  "QgsCoordinateTransform - invalid projection:\n myInputSpatialRefSys: (%s)\n myOutputSpatialRefSys: (%s)\n",
       theSourceWKT.ascii(), theDestWKT.ascii() );
    }

  // Deactivate GDAL error messages.
  //CPLSetErrorHandler( errorHandler );

  // Guess if the source o dest CS is in degrees.
  //Searchf for this phrase in each wkt:  "unit[\"degree\"" 
}

inline QgsCoordinateTransform::~QgsCoordinateTransform()
{
  delete mSourceToDestXForm;
}


inline QgsPoint QgsCoordinateTransform::transform(QgsPoint thePoint)
{
  if (mShortCircuit) return thePoint;
  // transform x
  double x = thePoint.x(); 
  double y = thePoint.y();
  if ( ! mSourceToDestXForm->Transform( 1, &x, &y ) )
  {
    //something bad happened....
    throw QgsCsException(QString("Coordinate transform failed"));
  }
  else
  {
    return QgsPoint(x, y);
  } 
}
inline QgsPoint QgsCoordinateTransform::transform(double theX, double theY)
{
  if (mShortCircuit) return QgsPoint(theX,theY);
  // transform x
  double x = theX; 
  double y = theY;
  if ( ! mSourceToDestXForm->Transform( 1, &x, &y ) )
  {
    //something bad happened....
    throw QgsCsException(QString("Coordinate transform failed"));
  }
  else
  {
    return QgsPoint(x, y);
  } 
}
#endif // QGSCSTRANSFORM_H
