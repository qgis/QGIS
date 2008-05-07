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
#include "qgslogger.h"

//qt includes
#include <QDomNode>
#include <QDomElement>
#include <QTextOStream>
#include <QApplication>

extern "C" {
#include <proj_api.h>
}

// if defined shows all information about transform to stdout
#undef COORDINATE_TRANSFORM_VERBOSE



QgsCoordinateTransform::QgsCoordinateTransform( ) : QObject(), mSourceSRS(), mDestSRS()

{
    setFinder();
}

QgsCoordinateTransform::QgsCoordinateTransform(const QgsSpatialRefSys& source, 
                                               const QgsSpatialRefSys& dest)
{
    setFinder();
  mSourceSRS = source;
  mDestSRS = dest;
  initialise();
}

QgsCoordinateTransform::QgsCoordinateTransform(long theSourceSrsId, long theDestSrsId)
  : mSourceSRS(theSourceSrsId, QgsSpatialRefSys::QGIS_SRSID),
    mDestSRS(theDestSrsId, QgsSpatialRefSys::QGIS_SRSID)
{
  initialise();
}

QgsCoordinateTransform::QgsCoordinateTransform( QString theSourceSRS, QString theDestSRS ) : QObject()

{
    setFinder();
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
    setFinder();

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
  QgsDebugMsg("QgsCoordinateTransform::setDestSRS called");
  mDestSRS = theSRS;
  initialise();
}


void QgsCoordinateTransform::setDestSRSID (long theSRSID)
{
  //!todo Add some logic here to determine if the srsid is a system or user one
  QgsDebugMsg("QgsCoordinateTransform::setDestSRSID slot called");
  mDestSRS.createFromSrsId(theSRSID);
  initialise();
}

// XXX This whole function is full of multiple return statements!!!
// And probably shouldn't be a void
void QgsCoordinateTransform::initialise()
{

  mInitialisedFlag=false; //guilty until proven innocent...
  mSourceProjection = NULL;
  mDestinationProjection = NULL;

  // XXX Warning - multiple return paths in this block!!
  if (!mSourceSRS.isValid())
  {
    //mSourceSRS = defaultWkt;
    // Pass through with no projection since we have no idea what the layer
    // coordinates are and projecting them may not be appropriate
    mShortCircuit = true;
    QgsDebugMsg("SourceSRS seemed invalid!");
    return;
  }

  if (!mDestSRS.isValid())
  {
    //No destination projection is set so we set the default output projection to
    //be the same as input proj. This only happens on the first layer loaded
    //whatever that may be...
    mDestSRS.createFromProj4(mSourceSRS.proj4String());
  }

  // init the projections (destination and source)
  mDestinationProjection = pj_init_plus(mDestSRS.proj4String().toUtf8());
  mSourceProjection = pj_init_plus(mSourceSRS.proj4String().toUtf8());

  mInitialisedFlag = true;
  if ( mDestinationProjection == NULL )
  {
    mInitialisedFlag = false;
  }
  if ( mSourceProjection == NULL )
  {
    mInitialisedFlag = false;
  }
#ifdef COORDINATE_TRANSFORM_VERBOSE
  if (mInitialisedFlag)
  {
    QgsDebugMsg("------------------------------------------------------------");
    QgsDebugMsg("QgsCoordinateTransform::initialise()");
    QgsDebugMsg("The OGR Coordinate transformation for this layer was set to");
    QgsLogger::debug<QgsSpatialRefSys>("Input", mSourceSRS, __FILE__, __FUNCTION__, __LINE__);
    QgsLogger::debug<QgsSpatialRefSys>("Output", mDestSRS, __FILE__, __FUNCTION__, __LINE__);
    QgsDebugMsg("------------------------------------------------------------");
  }
  else
  {
    QgsDebugMsg("------------------------------------------------------------");
    QgsDebugMsg("The OGR Coordinate transformation FAILED TO INITIALISE!");
    QgsDebugMsg("------------------------------------------------------------");
  }
#else
  if (!mInitialisedFlag)
  {
    QgsDebugMsg("Coordinate transformation failed to initialize!");
  }
#endif

  //XXX todo overload == operator for QgsSpatialRefSys
  //at the moment srs.parameters contains the whole proj def...soon it wont...
  //if (mSourceSRS->proj4String() == mDestSRS->proj4String())
  if (mSourceSRS == mDestSRS)
  {
    // If the source and destination projection are the same, set the short
    // circuit flag (no transform takes place)
    mShortCircuit=true;
    QgsDebugMsg("Source/Dest SRS equal, shortcircuit is set.");
  }
  else
  {
    // Transform must take place
    mShortCircuit=false;
  }

}

//
//
// TRANSFORMERS BELOW THIS POINT .........
//
//
//


QgsPoint QgsCoordinateTransform::transform(const QgsPoint thePoint,TransformDirection direction)
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
    // rethrow the exception
    QgsLogger::warning("Throwing exception " + QString(__FILE__) + QString::number(__LINE__));
    throw cse;
  }

#ifdef QGISDEBUG
  //std::cout << "Point projection...X : " << thePoint.x() << "-->" << x << ", Y: " << thePoint.y() << " -->" << y << std::endl;
#endif
  return QgsPoint(x, y);
}


QgsPoint QgsCoordinateTransform::transform(const double theX, const double theY=0,TransformDirection direction)
{
  try
  {
    return transform(QgsPoint(theX, theY), direction);
  }
  catch(QgsCsException &cse)
  {
    // rethrow the exception
    QgsLogger::warning("Throwing exception " + QString(__FILE__) + QString::number(__LINE__));
    throw cse;
  }
}

QgsRect QgsCoordinateTransform::transform(const QgsRect theRect,TransformDirection direction)
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
  try
  {
    double z = 0.0;
    transformCoords(1, &x1, &y1, &z, direction);
    transformCoords(1, &x2, &y2, &z, direction);
  }
  catch(QgsCsException &cse)
  {
    // rethrow the exception
    QgsLogger::warning("Throwing exception " + QString(__FILE__) + QString::number(__LINE__));
    throw cse;
  }

#ifdef QGISDEBUG
  QgsDebugMsg("Rect projection...");
  QgsLogger::debug("Xmin : ", theRect.xMin(), 1, __FILE__, __FUNCTION__, __LINE__);
  QgsLogger::debug("-->", x1, 1, __FILE__, __FUNCTION__, __LINE__);
  QgsLogger::debug("Ymin : ", theRect.yMin(), 1, __FILE__, __FUNCTION__, __LINE__);
  QgsLogger::debug("-->", y1, 1, __FILE__, __FUNCTION__, __LINE__);
  QgsLogger::debug("Xmax : ", theRect.xMax(), 1, __FILE__, __FUNCTION__, __LINE__);
  QgsLogger::debug("-->", x2, 1, __FILE__, __FUNCTION__, __LINE__);
  QgsLogger::debug("Ymax : ", theRect.yMax(), 1, __FILE__, __FUNCTION__, __LINE__);
  QgsLogger::debug("-->", y2, 1, __FILE__, __FUNCTION__, __LINE__);
#endif
  return QgsRect(x1, y1, x2 , y2);
}

void QgsCoordinateTransform::transformInPlace(double& x, double& y, double& z,
    TransformDirection direction)
{
  if (mShortCircuit || !mInitialisedFlag)
    return;
#ifdef QGISDEBUG
  //std::cout << "Using transform in place " << __FILE__ << " " << __LINE__ << std::endl;
#endif
  // transform x
  try
  {
    transformCoords(1, &x, &y, &z, direction );
  }
  catch(QgsCsException &cse)
  {
    // rethrow the exception 
    QgsLogger::warning("Throwing exception " + QString(__FILE__) + QString::number(__LINE__));
    throw cse;
  }
}

void QgsCoordinateTransform::transformInPlace(std::vector<double>& x,
    std::vector<double>& y, std::vector<double>& z,
    TransformDirection direction)
{
  if (mShortCircuit || !mInitialisedFlag)
    return;

  assert(x.size() == y.size());

  // Apparently, if one has a std::vector, it is valid to use the
  // address of the first element in the vector as a pointer to an
  // array of the vectors data, and hence easily interface with code
  // that wants C-style arrays.

  try
  {
    transformCoords(x.size(), &x[0], &y[0], &z[0], direction);
  }
  catch(QgsCsException &cse)
  {
    // rethrow the exception
    QgsLogger::warning("Throwing exception " + QString(__FILE__) + QString::number(__LINE__));
    throw cse;
  }
}


QgsRect QgsCoordinateTransform::transformBoundingBox(const QgsRect rect, TransformDirection direction)
{
  // Calculate the bounding box of a QgsRect in the source SRS
  // when projected to the destination SRS (or the inverse).
  // This is done by looking at a number of points spread evenly
  // across the rectangle

  if (mShortCircuit || !mInitialisedFlag)
    return rect;

  static const int numP = 8;

  QgsRect bb_rect;
  bb_rect.setMinimal();

  // We're interfacing with C-style vectors in the
  // end, so let's do C-style vectors here too.
  
  double x[numP * numP];
  double y[numP * numP];
  double z[numP * numP];

  QgsDebugMsg("Entering transformBoundingBox...");
  
  // Populate the vectors

  double dx = rect.width()  / (double)(numP - 1);
  double dy = rect.height() / (double)(numP - 1);

  double pointY = rect.yMin();

  for (int i = 0; i < numP ; i++)
  {

    // Start at right edge
    double pointX = rect.xMin();

    for (int j = 0; j < numP; j++)
    {
      x[(i*numP) + j] = pointX;
      y[(i*numP) + j] = pointY;
      // and the height...
      z[(i*numP) + j] = 0.0;
//      std::cout << "BBox coord: (" << x[(i*numP) + j] << ", " << y[(i*numP) + j]
//                << ")" << std::endl;
      pointX += dx; 
    }
    pointY += dy;
  }

  // Do transformation. Any exception generated must
  // be handled in above layers.
  try
  {
    transformCoords(numP * numP, x, y, z, direction);
  }
  catch(QgsCsException &cse)
  {
  // rethrow the exception
    QgsLogger::warning("Throwing exception " + QString(__FILE__) + QString::number(__LINE__));
    throw cse;
  }

  // Calculate the bounding box and use that for the extent
        
  for (int i = 0; i < numP * numP; i++)
  {
    bb_rect.combineExtentWith(x[i], y[i]);
  }
  
  QgsDebugMsg("Projected extent: " + QString((bb_rect.stringRep()).toLocal8Bit().data()));
  
  return bb_rect;
}

void QgsCoordinateTransform::transformCoords( const int& numPoints, double *x, double *y, double *z,TransformDirection direction)
{
  // Refuse to transform the points if the srs's are invalid
  if (!mSourceSRS.isValid())
  {
    QgsLogger::critical( tr("The source spatial reference system (SRS) is not valid. ") +
			 tr("The coordinates can not be reprojected. The SRS is: ") +
			    mSourceSRS.proj4String() );
    return;
  }
  if (!mDestSRS.isValid())
  {
    QgsLogger::critical( tr("The destination spatial reference system (SRS) is not valid. ") +
			 tr("The coordinates can not be reprojected. The SRS is: ") +
			    mDestSRS.proj4String() );
    return;
  }

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
    {
      if(direction == FORWARD)
      {
        pjErr << "(" << x[i] << ", " << y[i] << ")\n";
      }
      else
      {
        pjErr << "(" << x[i] * RAD_TO_DEG << ", " << y[i] * RAD_TO_DEG << ")\n";
      }
    }

    pjErr << tr("with error: ") << pj_strerrno(projResult) << '\n';
  
    QgsDebugMsg("Projection failed emitting invalid transform signal: " + QString(msg.toLocal8Bit().data()));

    emit invalidTransformInput();
    
    QgsLogger::warning("Throwing exception " + QString(__FILE__) + QString::number(__LINE__));
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
  
  QgsDebugMsg("Reading Coordinate Transform from xml ------------------------!");
  
  QDomNode mySrcNode = theNode.namedItem("sourcesrs");
  mSourceSRS.readXML(mySrcNode);

  QDomNode myDestNode = theNode.namedItem("destinationsrs");
  mDestSRS.readXML(myDestNode);

  initialise();

  return true;
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

  return true;
}

const char *finder( const char *name )
{
    QString proj;
#ifdef WIN32
    proj = QApplication::applicationDirPath() 
           + "/share/proj/" + QString(name);
#endif
    return proj.toUtf8(); 
}

void QgsCoordinateTransform::setFinder()
{
#ifdef WIN32
    // Attention! It should be possible to set PROJ_LIB
    // but it can happen that it was previously set by installer
    // (version 0.7) and the old installation was deleted

    // Another problem: PROJ checks if pj_finder was set before 
    // PROJ_LIB enviroment variable. pj_finder is probably set in 
    // GRASS gproj library when plugin is loaded, consequently 
    // PROJ_LIB is ignored 

    pj_set_finder( finder );
#endif
}

