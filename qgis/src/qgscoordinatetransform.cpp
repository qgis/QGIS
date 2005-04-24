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

QgsCoordinateTransform::QgsCoordinateTransform( QString theSourceWKT, QString theDestWKT ) : QObject(),
  mSourceWKT(theSourceWKT), mDestWKT(theDestWKT)
{
  // initialize the coordinate system data structures
  //XXX Who spells initialize initialise?
  //XXX A: Its the queen's english....
  //XXX  : Long live the queen! Lets get on with the initialisation...
  initialise();
}


QgsCoordinateTransform::~QgsCoordinateTransform()
{
  // free the proj objects
  pj_free(mSourceProjection);
  pj_free(mDestinationProjection);
}

void QgsCoordinateTransform::setSourceWKT(QString theWKT)
{
  mSourceWKT = theWKT;
  initialise();
}
void QgsCoordinateTransform::setDestWKT(QString theWKT)
{
#ifdef QGISDEBUG
  std::cout << "QgsCoordinateTransform::setDestWKT called" << std::endl;
#endif
  mDestWKT = theWKT;
  initialise();
}
// XXX This whole function is full of multiple return statements!!!
void QgsCoordinateTransform::initialise()
{
  mInitialisedFlag=false; //guilty until proven innocent...
  // Default to geo / wgs84 for now .... 
  // XXX Later we will make this user configurable
  //
  // XXX Do we need this singleton anymore?
  // SRID 4326 is geographic wgs84 - use the SRS singleton to fetch
  // the WKT for the coordinate system
  QString defaultWkt =  "GEOGCS[\"WGS 84\", "
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
  //default input projection to geo wgs84  
  // XXX Warning - multiple return paths in this block!!
  if (mSourceWKT.isEmpty())
  {
    //mSourceWKT = defaultWkt;
    // Pass through with no projection since we have no idea what the layer
    // coordinates are and projecting them may not be appropriate
    mShortCircuit = true;
    return;
  }

  if (mDestWKT.isEmpty())
  {
    //No destination projection is set so we set the default output projection to
    //be the same as input proj. This only happens on the first layer loaded
    //whatever that may be...
    mDestWKT = mSourceWKT;
  }  

  if (mSourceWKT == mDestWKT)
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

  //this is really ugly but we need to get a QString to a char**
  char *mySourceCharArrayPointer = (char *)mSourceWKT.ascii();
  char *myDestCharArrayPointer = (char *)mDestWKT.ascii();

  /* Here are the possible OGR error codes :
     typedef int OGRErr;

#define OGRERR_NONE                0
#define OGRERR_NOT_ENOUGH_DATA     1    --> not enough data to deserialize 
#define OGRERR_NOT_ENOUGH_MEMORY   2
#define OGRERR_UNSUPPORTED_GEOMETRY_TYPE 3
#define OGRERR_UNSUPPORTED_OPERATION 4
#define OGRERR_CORRUPT_DATA        5
#define OGRERR_FAILURE             6
#define OGRERR_UNSUPPORTED_SRS     7 */

  OGRErr myInputResult = mSourceOgrSpatialRef.importFromWkt( & mySourceCharArrayPointer );
  if (myInputResult != OGRERR_NONE)
  {
    std::cout << "vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv"<< std::endl;
    std::cout << "The source projection for this layer could *** NOT *** be set " << std::endl;
    std::cout << "INPUT: " << std::endl << mSourceWKT << std::endl;
    std::cout << "^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^" << std::endl;
    return;
  }
  // always morph from esri as it doesn't hurt anything
  mSourceOgrSpatialRef.morphFromESRI();
  // create the proj4 structs needed for transforming 
  char *proj4src;
  mSourceOgrSpatialRef.exportToProj4(&proj4src);
  // store the src proj parms in a QString because the pointer populated by exportToProj4
  // seems to get corrupted prior to its use in the transform
  mProj4SrcParms = proj4src;
  std::cout << "[[[[[[ Set source (layer) projection parms to: " << mProj4SrcParms << " ]]]]]]" << std::endl; 

  OGRErr myOutputResult = mDestOgrSpatialRef.importFromWkt( & myDestCharArrayPointer );
  if (myOutputResult != OGRERR_NONE)
  {
    std::cout << "vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv"<< std::endl;
    std::cout << "The dest projection for this layer could *** NOT *** be set " << std::endl;
    std::cout << "OUTPUT: " << std::endl << mDestWKT << std::endl;
    std::cout << "^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^" << std::endl;
    return;
  }  
  // initialize the proj4 structure for the source projection
    // always morph from esri as it doesn't hurt anything
    mDestOgrSpatialRef.morphFromESRI();
 // get the proj parms for dest cs
  char *proj4dest;
  mDestOgrSpatialRef.exportToProj4(&proj4dest);
  // store the dest proj parms in a QString because the pointer populated by exportToProj4
  // seems to get corrupted prior to its use in the transform
  mProj4DestParms = proj4dest;
  std::cout << "[[[[[[ Set destination (layer) projection parms to: " << mProj4DestParms << " ]]]]]]" << std::endl; 

 // init the projections (destination and source)
  mDestinationProjection = pj_init_plus((const char *)mProj4DestParms);
  mSourceProjection = pj_init_plus((const char *)mProj4SrcParms);

#ifdef QGISDEBUG 
  //OGRErr sourceValid = mSourceOgrSpatialRef.Validate();
  //OGRErr destValid = mDestOgrSpatialRef.Validate();
#endif     
  // One last test to see if they SRS are the same, despite slightly different
  // WKT specs
  // XXX This doesn't seem to work very well -- which means we are going to be
  // XXX attempting to transform coordinates that are in the same SRS. 
  // XXX What to do? What to do?....
  if( mSourceOgrSpatialRef.IsSame(&mDestOgrSpatialRef))
  {
    mShortCircuit = true;
  }
  // Validate the spaital reference systems  
  if ( (mSourceOgrSpatialRef.Validate() != OGRERR_NONE)  || (mDestOgrSpatialRef.Validate() != OGRERR_NONE))
  {
    std::cout << "vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv"<< std::endl;
    std::cout << "The OGR Coordinate transformation for this layer could *** NOT *** be set "
        << std::endl;
    std::cout << "INPUT: " << std::endl << mSourceWKT << std::endl;
    std::cout << "OUTPUT: " << std::endl << mDestWKT  << std::endl;
    std::cout << "^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^" << std::endl;
    return;
  }
  else
  {
    mInitialisedFlag = true;
    // Create the coordinate transform objects so we don't have to 
    // create them each pass through when projecting points
    forwardTransform = OGRCreateCoordinateTransformation( &mSourceOgrSpatialRef, &mDestOgrSpatialRef);
    inverseTransform = OGRCreateCoordinateTransformation( &mDestOgrSpatialRef, &mSourceOgrSpatialRef );


    std::cout << "vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv"<< std::endl;
    std::cout << "The OGR Coordinate transformation for this layer was set to" << std::endl;
    std::cout << "INPUT: " << std::endl << mSourceWKT << std::endl;
    //    std::cout << "PROJ4: " << std::endl << mProj4SrcParms << std::endl;  
    std::cout << "OUTPUT: " << std::endl << mDestWKT  << std::endl;
    //   std::cout << "PROJ4: " << std::endl << mProj4DestParms << std::endl;  
    std::cout << "^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^" << std::endl;
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
  std::cout << "Using transform in place " << __FILE__ << " " << __LINE__ << std::endl; 
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

  std::cout << "vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv"<< std::endl;
  std::cout << "Rect  projection..." << std::endl;
  std::cout << "INPUT: " << std::endl << mSourceWKT << std::endl;
  //std::cout << "PROJ4: " << std::endl << mProj4SrcParms << std::endl;  
  std::cout << "OUTPUT: " << std::endl << mDestWKT  << std::endl;
  //std::cout << "PROJ4: " << std::endl << mProj4DestParms << std::endl;  
  std::cout << "INPUT RECT: " << std::endl << x1 << "," << y1 << ":" << x2 << "," << y2 << std::endl;
  std::cout << "^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^" << std::endl;
#endif    
  // Number of points to reproject------+
  //                                    | 
  //                                    V 
  try{
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
/*
void QgsCoordinateTransform::transformCoords( 
     const int& numPoints, double* x, double* y, double* z,
     TransformDirection direction) const
{
  // use OGR to do the transform
  if(direction == INVERSE)
  {
    // transform from destination (map canvas/project) to layer CS
    inverseTransform->Transform(numPoints, x, y);
  }
  else
  {
    // transform from source layer CS to destination (map canvas/project) 
    forwardTransform->Transform(numPoints, x, y);

  }
}
*/
/* XXX THIS IS BASED ON DIRECT USE OF PROJ4 
 * XXX preserved for future use if we need it 
 */

void QgsCoordinateTransform::transformCoords( const int& numPoints, double *x, double *y, double *z,TransformDirection direction) const
{
  assert(mProj4DestParms.length() > 0);
  assert(mProj4SrcParms.length() > 0);
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

    pjErr << tr("Failed") << " " << dir << " " << tr("transform of") << x << ", " <<  y
      << pj_strerrno(projResult) << "\n";
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
