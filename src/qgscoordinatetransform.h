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


//qgis includes
#include "qgspoint.h"
#include "qgsrect.h"
#include "qgscsexception.h"

//gdal and ogr includes
#include <ogr_api.h>
#include <ogr_spatialref.h>
#include <cpl_error.h>

//non qt includes
#include <iostream>

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

    /*! Transform a QgsRect to the dest Coordinate system 
    * @param QgsRect rect to transform
    * @return QgsRect in Destination Coordinate System
    */        
    QgsRect transform(QgsRect theRect);
    
    /*! Inverse Transform the point from Dest Coordinate System to Source Coordinate System
    * @param p Point to transform (in destination coord system)
    * @return QgsPoint in Source Coordinate System
    */    
    QgsPoint inverseTransform(QgsPoint p);
    
    /*! Inverse Transform the point specified by x,y from Dest Coordinate System to Source Coordinate System
    * @param x x cordinate of point to transform (in dest coord sys)
    * @param y y coordinate of point to transform (in dest coord sys)
    * @return QgsPoint in Source Coordinate System
    */
    QgsPoint inverseTransform(double x, double y);

    /*! Inverse Transform a QgsRect to the source Coordinate system 
    * @param QgsRect rect to transform (in dest coord sys)
    * @return QgsRect in Source Coordinate System
    */        
    QgsRect inverseTransform(QgsRect theRect); 
       
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
    /** Used for forward transform */
    OGRCoordinateTransformation * mSourceToDestXForm;
    /** Used for reverse transform */
    OGRCoordinateTransformation * mDestToSourceXForm;
    /** Transform definitionsin WKT format */
    QString mSourceWKT,mDestWKT;
    /** Dunno if we need this - XXXXX Delete if unused */
    bool mInputIsDegrees;
    //set to true if src cs  == dest cs
    bool mShortCircuit;
};


//--------------------------------------------------------
// Inlined method implementations for best performance
//--------------------------------------------------------


// ------------------------------------------------------------------
//
//
// --------------- FORWARD PROJECTIONS ------------------------------
//
//
// ------------------------------------------------------------------
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
#ifdef QGISDEBUG 
    std::cout << "Point projection...X : " << thePoint.x() << "-->" << x << ", Y: " << thePoint.y() << " -->" << y << std::endl;
#endif        
    return QgsPoint(x, y);
  } 
}

inline QgsRect QgsCoordinateTransform::transform(QgsRect theRect)
{
  if (mShortCircuit || !mInitialisedFlag) return theRect;
  // transform x
  double x1 = theRect.xMin(); 
  double y1 = theRect.yMin();
  double x2 = theRect.xMax(); 
  double y2 = theRect.yMax();  
  // Number of points to reproject------+
  //                                    | 
  //                                    V 
  if ( ! mSourceToDestXForm->Transform( 1, &x1, &y1 ) || ! mSourceToDestXForm->Transform( 1, &x2, &y2 ) )
  {
    //something bad happened....
    throw QgsCsException(QString("Coordinate transform failed"));
  }
  else
  {
#ifdef QGISDEBUG 
    std::cout << "Rect projection..." 
              << "Xmin : " 
              << theRect.xMin() 
	      << "-->" << x1 
	      << ", Ymin: " 
	      << theRect.yMin() 
	      << " -->" << y1
              << "Xmax : " 
              << theRect.xMax() 
	      << "-->" << x2 
	      << ", Ymax: " 
	      << theRect.yMax() 
	      << " -->" << y2	      
	      << std::endl;
#endif        
    return QgsRect(x1, y1, x2 , y2);
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
#ifdef QGISDEBUG 
    std::cout << "Point projection...X : " << theX << "-->" << x << ", Y: " << theY << " -->" << y << std::endl;
#endif    
    return QgsPoint(x, y);
  } 
}

// ------------------------------------------------------------------
//
//
// --------------- INVERSE PROJECTIONS ------------------------------
//
//
// ------------------------------------------------------------------


inline QgsPoint QgsCoordinateTransform::inverseTransform(QgsPoint thePoint)
{
  if (mShortCircuit || !mInitialisedFlag) return thePoint;
  // transform x
  double x = thePoint.x(); 
  double y = thePoint.y();
  // Number of points to reproject------+
  //                                    | 
  //                                    V 
  if ( ! mDestToSourceXForm->Transform( 1, &x, &y ) )
  {
    //something bad happened....
    throw QgsCsException(QString("Coordinate inverse transform failed"));
  }
  else
  {
#ifdef QGISDEBUG 
    std::cout << "Point inverse projection...X : " << thePoint.x() << "-->" << x << ", Y: " << thePoint.y() << " -->" << y << std::endl;
#endif        
    return QgsPoint(x, y);
  } 
}

inline QgsRect QgsCoordinateTransform::inverseTransform(QgsRect theRect)
{
  if (mShortCircuit || !mInitialisedFlag) return theRect;
  // transform x
  double x1 = theRect.xMin(); 
  double y1 = theRect.yMin();
  double x2 = theRect.xMax(); 
  double y2 = theRect.yMax();  
  // Number of points to reproject------+
  //                                    | 
  //                                    V 
  if ( ! mDestToSourceXForm->Transform( 1, &x1, &y1 ) || ! mDestToSourceXForm->Transform( 1, &x2, &y2 ) )
  {
    //something bad happened....
    throw QgsCsException(QString("Coordinate inverse transform failed"));
  }
  else
  {
#ifdef QGISDEBUG 
    std::cout << "Rect inverse projection..." 
              << "Xmin : " 
              << theRect.xMin() 
	      << "-->" << x1 
	      << ", Ymin: " 
	      << theRect.yMin() 
	      << " -->" << y1
              << "Xmax : " 
              << theRect.xMax() 
	      << "-->" << x2 
	      << ", Ymax: " 
	      << theRect.yMax() 
	      << " -->" << y2	      
	      << std::endl;
#endif        
    return QgsRect(x1, y1, x2 , y2);
  } 
}


inline QgsPoint QgsCoordinateTransform::inverseTransform(double theX, double theY)
{
  if (mShortCircuit || !mInitialisedFlag) return QgsPoint(theX,theY);
  // transform x
  double x = theX; 
  double y = theY;
  if ( ! mDestToSourceXForm->Transform( 1, &x, &y ) )
  {
    //something bad happened....
    throw QgsCsException(QString("Coordinate inverseTransform failed"));
  }
  else
  {
#ifdef QGISDEBUG 
    std::cout << "Point inverse projection...X : " << theX << "-->" << x << ", Y: " << theY << " -->" << y << std::endl;
#endif    
    return QgsPoint(x, y);
  } 
}



#endif // QGSCOORDINATETRANSFORM_H
