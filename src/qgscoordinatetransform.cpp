/***************************************************************************
               QgsCoordinateTransform.cpp  - Coordinate Transforms
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
#include <cassert>
#include "qgscoordinatetransform.h"

//qt includes
#include <qdom.h>

// Qt4-only includes to go here

QgsCoordinateTransform::QgsCoordinateTransform( ) : QObject()

{
}

QgsCoordinateTransform::QgsCoordinateTransform(const QgsSpatialRefSys& source, 
                                               const QgsSpatialRefSys& dest)
{
  mSourceSRS = source;
  mDestSRS = dest;
  initialise();
}


QgsCoordinateTransform::QgsCoordinateTransform( QString theSourceSRS, QString theDestSRS ) : QObject()

{
  mSourceSRS.createFromWkt(theSourceSRS);
  mDestSRS.createFromWkt(theDestSRS);
  // initialize the coordinate system data structures
  //XXX Who spells initialize initialise?
  //XXX A: Its the queen's english....
  //XXX  : Long live the queen! Lets get on with the initialisation...
  initialise();
}

QgsCoordinateTransform::QgsCoordinateTransform(long theSourceSrid,
    QString theDestWKT,
    QgsSpatialRefSys::SRS_TYPE theSourceSRSType): QObject()
{

  mSourceSRS.createFromId(theSourceSrid, theSourceSRSType);
  mDestSRS.createFromWkt(theDestWKT);
  // initialize the coordinate system data structures
  //XXX Who spells initialize initialise?
  //XXX A: Its the queen's english....
  //XXX  : Long live the queen! Lets get on with the initialisation...
  initialise();
}

QgsCoordinateTransform::~QgsCoordinateTransform()
{
  // free the proj objects
  if (mSourceProjection!=0) 
  {
    pj_free(mSourceProjection);
  }
  if (mDestinationProjection!=0)
  {
    pj_free(mDestinationProjection);
  }
}

void QgsCoordinateTransform::setSourceSRS(const QgsSpatialRefSys& theSRS)
{
  mSourceSRS = theSRS;
  initialise();
}
void QgsCoordinateTransform::setDestSRS(const QgsSpatialRefSys& theSRS)
{
#ifdef QGISDEBUG
  std::cout << "QgsCoordinateTransform::setDestSRS called" << std::endl;
#endif
  mDestSRS = theSRS;
  initialise();
}


void QgsCoordinateTransform::setDestSRSID (long theSRSID)
{
  //!todo Add some logic here to determine if the srsid is a system or user one
#ifdef QGISDEBUG
  std::cout << "QgsCoordinateTransform::setDestSRSID slot called" << std::endl;
#endif
  mDestSRS.createFromSrsId(theSRSID);
  initialise();
}

// XXX This whole function is full of multiple return statements!!!
void QgsCoordinateTransform::initialise()
{

  mInitialisedFlag=false; //guilty until proven innocent...

  // XXX Warning - multiple return paths in this block!!
  if (!mSourceSRS.isValid())
  {
    //mSourceSRS = defaultWkt;
    // Pass through with no projection since we have no idea what the layer
    // coordinates are and projecting them may not be appropriate
    mShortCircuit = true;
    return;
  }

  if (!mDestSRS.isValid())
  {
    //No destination projection is set so we set the default output projection to
    //be the same as input proj. This only happens on the first layer loaded
    //whatever that may be...
    mDestSRS.createFromProj4(mSourceSRS.proj4String());
  }

  //XXX todo overload == operator for QgsSpatialRefSys
  //at the moment srs.parameters contains the whole proj def...soon it wont...
  //if (mSourceSRS->proj4String() == mDestSRS->proj4String())
  if (mSourceSRS == mDestSRS)
  {
    // If the source and destination projection are the same, set the short
    // circuit flag (no transform takes place)
    mShortCircuit=true;
    return;
  }
  else
  {
    // Transform must take place
    mShortCircuit=false;
  }

  // init the projections (destination and source)
  mDestinationProjection = pj_init_plus(mDestSRS.proj4String());
  mSourceProjection = pj_init_plus(mSourceSRS.proj4String());

  mInitialisedFlag = true;
  if ( mDestinationProjection == NULL )
  {
    mInitialisedFlag = false;
  }
  if ( mSourceProjection == NULL )
  {
    mInitialisedFlag = false;
  }

  if (mInitialisedFlag)
  {

    std::cout << "------------------------------------------------------------"<< std::endl;
    std::cout << "QgsCoordinateTransform::initialise()" << std::endl;
    std::cout << "The OGR Coordinate transformation for this layer was set to" << std::endl;
    // note overloaded << operator on qgsspatialrefsys cant be used on pointers -
    // so we dereference them like this (*mSourceSRS) (Thanks Lars for pointing that out)
    std::cout << "INPUT: " << std::endl << mSourceSRS << std::endl;
    std::cout << "OUTPUT: " << std::endl << mDestSRS  << std::endl;
    std::cout << "------------------------------------------------------------" << std::endl;
  }
  else
  {
    std::cout << "------------------------------------------------------------"<< std::endl;
    std::cout << "QgsCoordinateTransform::initialise()" << std::endl;
    std::cout << "The OGR Coordinate transformation FAILED TO INITIALISE!" << std::endl;
    std::cout << "------------------------------------------------------------"<< std::endl;
  }
}

//
//
// TRANSFORMERS BELOW THIS POINT .........
//
//
//


QgsPoint QgsCoordinateTransform::transform(const QgsPoint thePoint,TransformDirection direction) const
{
  if (mShortCircuit || !mInitialisedFlag) return thePoint;
  // transform x
  double x = thePoint.x();
  double y = thePoint.y();
  double z = 0.0;
  try
  {

    transformCoords(1, &x, &y, &z, direction );
  }
  catch(QgsCsException &cse)
  {
    //something bad happened....
    // rethrow the exception
    throw cse;
  }
#ifdef QGISDEBUG
  //std::cout << "Point projection...X : " << thePoint.x() << "-->" << x << ", Y: " << thePoint.y() << " -->" << y << std::endl;
#endif
  return QgsPoint(x, y);
}


QgsPoint QgsCoordinateTransform::transform(const double theX, const double theY=0,TransformDirection direction) const
{
  return transform(QgsPoint(theX, theY), direction);
}

void QgsCoordinateTransform::transformInPlace(double& x, double& y, double& z,
    TransformDirection direction) const
{
  if (mShortCircuit || !mInitialisedFlag)
    return;
#ifdef QGISDEBUG
  //std::cout << "Using transform in place " << __FILE__ << " " << __LINE__ << std::endl;
#endif
  // transform x
  transformCoords(1, &x, &y, &z, direction );
}

void QgsCoordinateTransform::transformInPlace(std::vector<double>& x,
    std::vector<double>& y, std::vector<double>& z,
    TransformDirection direction) const
{
  if (mShortCircuit || !mInitialisedFlag)
    return;

  assert(x.size() == y.size());

  // Apparently, if one has a std::vector, it is valid to use the
  // address of the first element in the vector as a pointer to an
  // array of the vectors data, and hence easily interface with code
  // that wants C-style arrays.

  transformCoords(x.size(), &x[0], &y[0], &z[0], direction);
}

QgsRect QgsCoordinateTransform::transform(const QgsRect theRect,TransformDirection direction) const
{
  if (mShortCircuit || !mInitialisedFlag) return theRect;
  // transform x
  double x1 = theRect.xMin();
  double y1 = theRect.yMin();
  double x2 = theRect.xMax();
  double y2 = theRect.yMax();

#ifdef QGISDEBUG
  std::cout << this;
#endif
  // Number of points to reproject------+
  //                                    |
  //                                    V
  try
  {
    double z = 0.0;
    transformCoords(1, &x1, &y1, &z, direction);
    transformCoords(1, &x2, &y2, &z, direction);

  }
  catch(QgsCsException &cse)
  {
    // rethrow the exception
    throw cse;
  }

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


void QgsCoordinateTransform::transformCoords( const int& numPoints, double *x, double *y, double *z,TransformDirection direction) const
{
  assert(mSourceSRS.isValid());
  assert(mDestSRS.isValid());
#ifdef QGISDEBUG
  //double xorg = x;
  //double yorg = y;
  //std::cout << "[[[[[[Number of points to transform: " << numPoints << "]]]]]]" << std::endl;
#endif
  // use proj4 to do the transform
  QString dir;
  // if the source/destination projection is lat/long, convert the points to radians
  // prior to transforming
  if((pj_is_latlong(mDestinationProjection) && (direction == INVERSE))
      || (pj_is_latlong(mSourceProjection) && (direction == FORWARD)))
  {
    for (int i = 0; i < numPoints; ++i)
    {
      x[i] *= DEG_TO_RAD;
      y[i] *= DEG_TO_RAD;
      z[i] *= DEG_TO_RAD;
    }

  }
  int projResult;
  if(direction == INVERSE)
  {
    /*
    std::cout << "!!!! INVERSE PROJ4 TRANSFORM !!!!" << std::endl; 
    std::cout << "     numPoint: " << numPoints << std::endl; 
    std::cout << "     x       : " << x << std::endl; 
    std::cout << "     y       : " << y << std::endl; 
    */
    projResult = pj_transform(mDestinationProjection, mSourceProjection , numPoints, 0, x, y, z);
    dir = "inverse";
  }
  else
  {
    /*
    std::cout << "!!!! FORWARD PROJ4 TRANSFORM !!!!" << std::endl; 
    std::cout << "     numPoint: " << numPoints << std::endl; 
    std::cout << "     x       : " << x << std::endl; 
    std::cout << "     y       : " << y << std::endl; 
    std::cout << "     z       : " << z << std::endl; 
    */
    assert(mSourceProjection != 0);
    assert(mDestinationProjection !=0);
    projResult = pj_transform(mSourceProjection, mDestinationProjection, numPoints, 0, x, y, z);
    dir = "forward";
  }

  if (projResult != 0)
  {
    //something bad happened....
    QString msg;
    QTextOStream pjErr(&msg);

    pjErr << tr("Failed") << " " << dir << " " << tr("transform of") << '\n';
    for (int i = 0; i < numPoints; ++i)
      pjErr << "(" << x[i] << ", " << y[i] << ")\n";
    pjErr << tr("with error: ") << pj_strerrno(projResult) << '\n';
    throw  QgsCsException(msg);
  }
  // if the result is lat/long, convert the results from radians back
  // to degrees
  if((pj_is_latlong(mDestinationProjection) && (direction == FORWARD))
      || (pj_is_latlong(mSourceProjection) && (direction == INVERSE)))
  {
    for (int i = 0; i < numPoints; ++i)
    {
      x[i] *= RAD_TO_DEG;
      y[i] *= RAD_TO_DEG;
      z[i] *= RAD_TO_DEG;
    }
  }
#ifdef QGISDEBUG
  // std::cout << "[[[[[[ Projected " << xorg << ", " << yorg << " to "  << x << ", " << y << " ]]]]]]"<< std::endl;
#endif
}

bool QgsCoordinateTransform::readXML( QDomNode & theNode )
{
#ifdef QGISDEBUG
  std::cout << "Reading Coordinate Transform from xml ------------------------!" << std::endl;
#endif
  QDomNode mySrcNodeParent = theNode.namedItem("sourcesrs");
  QDomNode mySrcNode = mySrcNodeParent.namedItem("spatialrefsys");
  mSourceSRS.readXML(mySrcNode);
  QDomNode myDestNodeParent = theNode.namedItem("destinationsrs");
  QDomNode myDestNode = myDestNodeParent.namedItem("spatialrefsys");
  mDestSRS.readXML(myDestNode);
  initialise();
}

bool QgsCoordinateTransform::writeXML( QDomNode & theNode, QDomDocument & theDoc )
{
  
  QDomElement myNodeElement = theNode.toElement();
  QDomElement myTransformElement  = theDoc.createElement( "coordinatetransform" );
  
  QDomElement mySourceElement  = theDoc.createElement( "sourcesrs" );
  mSourceSRS.writeXML(mySourceElement, theDoc);
  myTransformElement.appendChild(mySourceElement);
  
  QDomElement myDestElement  = theDoc.createElement( "destinationsrs" );
  mDestSRS.writeXML(myDestElement, theDoc);
  myTransformElement.appendChild(myDestElement);
  
  myNodeElement.appendChild(myTransformElement);

}
