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
    
    //! Accessor and mutator for source WKT
    void setSourceWKT(QString theWKT);
    QString sourceWKT() const {return mSourceWKT;};
    //! Accessor and mutator for dest WKT
    void setDestWKT(QString theWKT);
    QString destWKT() const {return mDestWKT;};    
    //! Accessor for whether this transoform is properly initialised
    bool isInitialised() {return mInitialisedFlag;};
 private:
    //!initialise is used to actually create the Transformer instance
    void initialise();
    //! flag to show whether the transform is properly initialised or not
    bool mInitialisedFlag;
    OGRCoordinateTransformation * mSourceToDestXForm;
    QString mSourceWKT,mDestWKT;
    bool mInputIsDegrees;
    //set to true if src cs  == dest cs
    bool mShortCircuit;
};




inline QgsPoint QgsCoordinateTransform::transform(QgsPoint thePoint)
{
  if (mShortCircuit || !mInitialisedFlag) return thePoint;
  // transform x
  double x = thePoint.x(); 
  double y = thePoint.y();
  // Number of points to reproject------+
  //                                    | 
  //                                    V 
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
  if (mShortCircuit || !mInitialisedFlag) return QgsPoint(theX,theY);
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
#endif // QGSCOORDINATETRANSFORM_H
