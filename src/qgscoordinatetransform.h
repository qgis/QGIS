/***************************************************************************
               QgsCoordinateTransform.h  - Coordinate Transforms
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

//qt includes
#include <qobject.h>

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

extern "C"{
#include <proj_api.h>
}
class QString;

/*! \class QgsCoordinateTransform
* \brief Class for doing transforms between two map coordinate systems.
*
* This class can convert map coordinates to a different spatial reference system.
* It is normally associated with a map layer and is used to transform between the
* layer's coordinate system and the coordinate system of the map canvas, although
* it can be used in a more general sense to transform coordinates.
*
* All references to source and destination coordinate systems refer to 
* layer and map canvas respectively. All operations are from the perspective 
* of the layer. For example, a forward transformation transforms coordinates from the
* layers coordinate system to the map canvas.
*/
class QgsCoordinateTransform: public QObject
{
  Q_OBJECT
 public:
    /*!
     * Constructs a QgsCoordinateTransform using the Well Known Text representation
     * of the layer and map canvas coordinate systems
     * @param theSourceWKT WKT of the layer's coordinate system
     * @param theSourceWKT WKT of the map canvas coordinate system
     */
    QgsCoordinateTransform(QString theSourceWKT, QString theDestWKT  );
     //! destructor
    ~QgsCoordinateTransform();
    //! Enum used to indicate the direction (forward or inverse) of the transform
    enum TransformDirection{
      FORWARD,
      INVERSE
    };
    
    /*! 
     * Set the source (layer) WKT
     * @param theWKT WKT representation of the layer's coordinate system
     */
    void setSourceWKT(QString theWKT);
    /*!
     * Get the WKT representation of the layer's coordinate system
     * @return WKT of the layer's coordinate system
     */
    QString sourceWKT() const {return mSourceWKT;};
    /*! 
     * Get the WKT representation of the map canvas coordinate system
     * @return WKT of the map canvas coordinate system
     */
    QString destWKT() const {return mDestWKT;};    
  /*! 
   * Flag to indicate whether the coordinate systems have been initialised
   * @return true if initialised, otherwise false
   */
   bool isInitialised() {return mInitialisedFlag;};
   
       
    /*! Transform the point from Source Coordinate System to Destination Coordinate System
    * If the direction is FORWARD then coordinates are transformed from layer CS --> map canvas CS,
    * otherwise points are transformed from map canvas CS to layerCS.
    * @param p Point to transform
    * @param direction TransformDirection (defaults to FORWARD)
    * @return QgsPoint in Destination Coordinate System
     */    
   QgsPoint transform(const QgsPoint p,TransformDirection direction=FORWARD) const;
    
    /*! Transform the point specified by x,y from Source Coordinate System to Destination Coordinate System
    * If the direction is FORWARD then coordinates are transformed from layer CS --> map canvas CS,
    * otherwise points are transformed from map canvas CS to layerCS.
    * @param x x cordinate of point to transform
    * @param y y coordinate of point to transform
    * @param direction TransformDirection (defaults to FORWARD)
    * @return QgsPoint in Destination Coordinate System
     */
   QgsPoint transform(const double x, const double y,TransformDirection direction=FORWARD) const ;

   // Same as for the other transform() functions, but alters the x
   // and y variables in place
   void transformInPlace(double& x, double& y, TransformDirection direction = FORWARD) const;

    /*! Transform a QgsRect to the dest Coordinate system 
    * If the direction is FORWARD then coordinates are transformed from layer CS --> map canvas CS,
    * otherwise points are transformed from map canvas CS to layerCS.
    * @param QgsRect rect to transform
    * @param direction TransformDirection (defaults to FORWARD)
    * @return QgsRect in Destination Coordinate System
     */        
   QgsRect transform(const QgsRect theRect,TransformDirection direction=FORWARD) const;
    
    /*! Transform an array of coordinates to a different Coordinate System
    * If the direction is FORWARD then coordinates are transformed from layer CS --> map canvas CS,
    * otherwise points are transformed from map canvas CS to layerCS.
    * @param x x cordinate of point to transform
    * @param y y coordinate of point to transform     
    * @param direction TransformDirection (defaults to FORWARD)
    * @return QgsRect in Destination Coordinate System
     */        
   void transformCoords( const int &numPoint, double &x, double &y, double &z,TransformDirection direction=FORWARD) const;

 public slots:
    /*! 
     * Mutator for dest WKT - This slot will usually be called if the
     * project properties change and a different coordinate system is 
     * selected.
     * @param WKT of the destination coordinate system
     */
    void setDestWKT(QString theWKT);    
    
 private:
    //!initialise is used to actually create the Transformer instance
    void initialise();
    //! flag to show whether the transform is properly initialised or not
    bool mInitialisedFlag;
    /*! 
     * WKT of the source (layer) coordinate system 
     */
    QString mSourceWKT;
    /*! 
     * WKT of the destination (map canvas) coordinate system 
     */
    QString mDestWKT;
    /** Dunno if we need this - XXX Delete if unused */
    bool mInputIsDegrees;
    /*! 
     * Flag to indicate that the source and destination coordinate systems are
     * equal and not transformation needs to be done
     */
    bool mShortCircuit;
    OGRSpatialReference mSourceOgrSpatialRef;
    OGRSpatialReference mDestOgrSpatialRef;
    OGRCoordinateTransformation *forwardTransform;
    OGRCoordinateTransformation *inverseTransform;
    /*!
     * Proj4 parameters for the source (layer) coordinate system
     */
    QString mProj4SrcParms;
    /*!
     * Proj4 parameters for the destination (map canvas) coordinate system
     */
    QString mProj4DestParms;
    /*!
     * Proj4 data structure of the source projection (layer coordinate system)
     */
    projPJ mSourceProjection;
    /*!
     * Proj4 data structure of the destination projection (map canvas coordinate system)
     */
    projPJ mDestinationProjection;
};


#endif // QGSCOORDINATETRANSFORM_H
