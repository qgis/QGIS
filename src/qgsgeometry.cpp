/***************************************************************************
  qgsgeometry.cpp - Geometry (stored as Open Geospatial Consortium WKB)
  -------------------------------------------------------------------
Date                 : 02 May 2005
Copyright            : (C) 2005 by Brendan Morley
email                : morb at ozemail dot com dot au
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
/* $Id$ */

#include <cfloat>

#include "qgis.h"
#include "qgsgeometry.h"

QgsGeometry::QgsGeometry()
  : mGeometry(0),
    mGeometrySize(0),
    mWkt(0),
    mGeos(0),
    
    mDirtyGeos(FALSE),
    mDirtyWkb(FALSE),
    mDirtyWkt(FALSE)
{
  // NOOP
}    


QgsGeometry::QgsGeometry( QgsGeometry const & rhs )
    : mGeometry(0),
      mGeometrySize( rhs.mGeometrySize ),
      mWkt( rhs.mWkt ),
      
      mDirtyGeos( rhs.mDirtyGeos ),
      mDirtyWkb( rhs.mDirtyWkb ),
      mDirtyWkt( rhs.mDirtyWkt )
{      
  
  
  if ( mGeometrySize && rhs.mGeometry )
  {
    mGeometry = new unsigned char[mGeometrySize];
    memcpy( mGeometry, rhs.mGeometry, mGeometrySize );
  }

  // deep-copy the GEOS Geometry if appropriate
  if (rhs.mGeos)
  {  
    mGeos = rhs.mGeos->clone();
  }
  else
  {
    mGeos = 0;
  }    
}


QgsGeometry & QgsGeometry::operator=( QgsGeometry const & rhs )
{
  if ( &rhs == this )
  { 
    return *this; 
  }

  // remove old geometry if it exists
  if ( mGeometry )
  {
      delete [] mGeometry;
      mGeometry = 0;
  }
  
  mGeometrySize    = rhs.mGeometrySize;
  mWkt             = rhs.mWkt;

  // deep-copy the GEOS Geometry if appropriate
  if (rhs.mGeos)
  {  
    mGeos = rhs.mGeos->clone();
  }
  else
  {
    mGeos = 0;
  }    

  mDirtyGeos = rhs.mDirtyGeos;
  mDirtyWkb  = rhs.mDirtyWkb;
  mDirtyWkt  = rhs.mDirtyWkt;

  if ( mGeometrySize && rhs.mGeometry )
  {
      mGeometry = new unsigned char[mGeometrySize];
      memcpy( mGeometry, rhs.mGeometry, mGeometrySize );
  }
  
  return *this;
} // QgsGeometry::operator=( QgsGeometry const & rhs )


//! Destructor
QgsGeometry::~QgsGeometry()
{
#ifdef QGISDEBUG
//      std::cout << "QgsGeometry::~QgsGeometry: deleting with mGeometry " << mGeometry << " and mGeos " << mGeos
//                << "." << std::endl;
#endif
  if (mGeometry)
  {
    delete [] mGeometry;
  }
  
  if (mGeos)
  {
    delete mGeos;
  }
}


void QgsGeometry::setWkbAndOwnership(unsigned char * wkb, size_t length)
{
  // delete any existing WKB geometry before assigning new one
  if ( mGeometry )
  {
    delete [] mGeometry;
    mGeometry = 0;
  }

  // TODO: What about ownership?
  
  mGeometry = wkb;
  mGeometrySize = length;

  mDirtyWkb   = FALSE;  
  mDirtyGeos  = TRUE;
  mDirtyWkt   = TRUE;

}
    
unsigned char * QgsGeometry::wkbBuffer() const
{
  return mGeometry;
}
    
size_t QgsGeometry::wkbSize() const
{
  return mGeometrySize;
}


QString const& QgsGeometry::wkt() const
{
  if (mDirtyWkt)
  {
    // see which geometry representation to convert from
    if (mDirtyGeos)
    {
      // convert from WKB
      exportToWkt();
    }
    else
    {
      // TODO
    }
  
  }

  return mWkt;
}


QgsPoint QgsGeometry::closestVertex(const QgsPoint& point) const
{
    if(mGeometry)
    {
	int wkbType;
	double actdist=DBL_MAX;
	double x,y;
	double *tempx,*tempy;
	memcpy(&wkbType, (mGeometry+1), sizeof(int));
	switch (wkbType)
	{
	    case QGis::WKBPoint:
		x = *((double *) (mGeometry + 5));
		y = *((double *) (mGeometry + 5 + sizeof(double)));
		break;

	    case QGis::WKBLineString:
	    {
		unsigned char* ptr=mGeometry+5;
		int* npoints=(int*)ptr;
		ptr+=sizeof(int);
		for(int index=0;index<*npoints;++index)
		{
		    tempx = (double*)ptr;
		    ptr+=sizeof(double);
		    tempy = (double*)ptr;
		    if(point.sqrDist(*tempx,*tempy)<actdist)
		    {
			x=*tempx;
			y=*tempy;
			actdist=point.sqrDist(*tempx,*tempy);
		    }
		    ptr+=sizeof(double);
		}
		break;
	    }
	    case QGis::WKBPolygon:
	    {
		int* nrings=(int*)(mGeometry+5);
		int* npoints;
		unsigned char* ptr=mGeometry+9;
		for(int index=0;index<*nrings;++index)
		{
		    npoints=(int*)ptr;
		    ptr+=sizeof(int);
		    for(int index2=0;index2<*npoints;++index2)
		    {
			tempx=(double*)ptr;
			ptr+=sizeof(double);
			tempy=(double*)ptr;
			if(point.sqrDist(*tempx,*tempy)<actdist)
			{
			    x=*tempx;
			    y=*tempy;
			    actdist=point.sqrDist(*tempx,*tempy);
			}
			ptr+=sizeof(double);
		    }
		}
	    }
		break;

	    case QGis::WKBMultiPoint:
	    {
		unsigned char* ptr=mGeometry+5;
		int* npoints=(int*)ptr;
		ptr+=sizeof(int);
		for(int index=0;index<*npoints;++index)
		{
		    tempx=(double*)ptr;
		    tempy=(double*)(ptr+sizeof(double));
		    if(point.sqrDist(*tempx,*tempy)<actdist)
			{
			    x=*tempx;
			    y=*tempy;
			    actdist=point.sqrDist(*tempx,*tempy);
			}
		    ptr+=(2*sizeof(double));
		}
	    }
		break;

	    case QGis::WKBMultiLineString:
	    {
		unsigned char* ptr=mGeometry+5;
		int* nlines=(int*)ptr;
		int* npoints;
		ptr+=sizeof(int);
		for(int index=0;index<*nlines;++index)
		{
		    npoints=(int*)ptr;
		    ptr+=sizeof(int);
		    for(int index2=0;index2<*npoints;++index2)
		    {
			tempx=(double*)ptr;
			ptr+=sizeof(double);
			tempy=(double*)ptr;
			if(point.sqrDist(*tempx,*tempy)<actdist)
			{
			    x=*tempx;
			    y=*tempy;
			    actdist=point.sqrDist(*tempx,*tempy);
			}
			ptr+=sizeof(double);
			
		    }
		}
	    }
		break;

	    case QGis::WKBMultiPolygon:
	    {
		unsigned char* ptr=mGeometry+5;
		int* npolys=(int*)ptr;
		int* nrings;
		int* npoints;
		ptr+=sizeof(int);
		for(int index=0;index<*npolys;++index)
		{
		    nrings=(int*)ptr;
		    ptr+=sizeof(int);
		    for(int index2=0;index2<*nrings;++index2)
		    {
			npoints=(int*)ptr;
			ptr+=sizeof(int);
			for(int index3=0;index3<*npoints;++index3)
			{
			   tempx=(double*)ptr;
			   ptr+=sizeof(double);
			   tempy=(double*)ptr;
			   if(point.sqrDist(*tempx,*tempy)<actdist)
			   {
			       x=*tempx;
			       y=*tempy;
			       actdist=point.sqrDist(*tempx,*tempy);
			   }
			   ptr+=sizeof(double); 
			}
		    }
		}
	    }
		break;

	    default:
		break;
	}
	return QgsPoint(x,y);    
    }
    return QgsPoint(0,0);
}




bool QgsGeometry::insertVertexBefore(double x, double y, 
                                     QgsGeometryVertexIndex beforeVertex)
{

#ifdef QGISDEBUG
      std::cout << "QgsGeometry::insertVertexBefore: Entered with "
         << "x " << x << ", y " << y 
//         << "beforeVertex " << beforeVertex << ", atRing " << atRing << ", atItem"
//         << " " << atItem            
                << "." << std::endl;
#endif


    if (mGeometry)
    {
        int wkbType;
//        double *x,*y;

        memcpy(&wkbType, (mGeometry+1), sizeof(int));
        switch (wkbType)
        {
            case QGis::WKBPoint:
            {
                // Meaningless
                return FALSE;
                break;
            }
            case QGis::WKBLineString:
            {
                unsigned char *ptr;
                int *nPoints;
                int idx;

#ifdef QGISDEBUG
      std::cout << "QgsGeometry::insertVertexBefore: in QGis::WKBLineString." << std::endl;
#endif
                
                // get number of points in the line
                ptr = mGeometry + 5;     // now at geometry.numPoints
                nPoints = (int *) ptr;

                
#ifdef QGISDEBUG
      std::cout << "QgsGeometry::insertVertexBefore: old number of points is "
                << (*nPoints)
                << "." << std::endl;
#endif
                                
                // silently adjust any overflow "beforeVertex" values to mean
                // "append to end of string"
                if (*nPoints < beforeVertex.back())
                {
                  beforeVertex.assign_back(*nPoints); // beforeVertex is 0 based, *nPoints is 1 based
                }
                      
                // create new geometry expanded by 2 doubles
                unsigned char *newWKB = new unsigned char[mGeometrySize + 16];
                memset(newWKB, '\0', mGeometrySize + 16);  // just in case
                
#ifdef QGISDEBUG
      std::cout << "QgsGeometry::insertVertexBefore: newWKB at " << newWKB << "." << std::endl;
#endif
                

#ifdef QGISDEBUG
      std::cout << "QgsGeometry::insertVertexBefore: done initial newWKB of "
                << (mGeometrySize + 16)
                << " bytes." << std::endl;
#endif
                                
                // copy section before splice
                memcpy(newWKB, mGeometry, 9 + (16 * beforeVertex.back()) );

#ifdef QGISDEBUG
      std::cout << "QgsGeometry::insertVertexBefore: done pre memcpy of "
                << (9 + (16 * beforeVertex.back()))
                << " bytes." << std::endl;
#endif
#ifdef QGISDEBUG
      exportToWkt(newWKB);
      std::cout << "QgsGeometry::insertVertexBefore: newWKB: " << mWkt << "." << std::endl;
#endif

                // adjust number of points
                (*nPoints)++;
                ptr = newWKB + 5;  // now at newWKB.numPoints
                memcpy(ptr, nPoints, 4);

#ifdef QGISDEBUG
      std::cout << "QgsGeometry::insertVertexBefore: new number of points is "
                << (*nPoints)
                << "." << std::endl;
#endif
                                
                
                // copy vertex to be spliced
                ptr += 4;   // now at newWKB.points[]
#ifdef QGISDEBUG
     std::cout << "QgsGeometry::insertVertexBefore: ptr at " << ptr << "." << std::endl;
#endif
                ptr += (16 * beforeVertex.back()); 
#ifdef QGISDEBUG
     std::cout << "QgsGeometry::insertVertexBefore: going to add x " << x << "." << std::endl;
     std::cout << "QgsGeometry::insertVertexBefore: ptr at " << ptr << ", going to add x " << x << "." << std::endl;
#endif
                memcpy(ptr, &x, 8);
                ptr += 8;
#ifdef QGISDEBUG
     std::cout << "QgsGeometry::insertVertexBefore: going to add y " << y << "." << std::endl;
     std::cout << "QgsGeometry::insertVertexBefore: ptr at " << ptr << ", going to add y " << y << "." << std::endl;
#endif
                memcpy(ptr, &y, 8);
                ptr += 8;

                
#ifdef QGISDEBUG
      exportToWkt(newWKB);
      std::cout << "QgsGeometry::insertVertexBefore: newWKB: " << mWkt << "." << std::endl;
#endif

                                
                // copy section after splice
                memcpy(ptr, mGeometry + 9 + (16 * beforeVertex.back()), (16 * (*nPoints - beforeVertex.back())) );

                
#ifdef QGISDEBUG
      std::cout << "QgsGeometry::insertVertexBefore: done post memcpy of "
                << (16 * (*nPoints - beforeVertex.back()))
                << " bytes." << std::endl;
#endif

#ifdef QGISDEBUG
      exportToWkt(newWKB);
      std::cout << "QgsGeometry::insertVertexBefore: newWKB: " << mWkt << "." << std::endl;
#endif
                                
                // set new geomtry to this object
                setWkbAndOwnership(newWKB, mGeometrySize + 16);
      
                break;
            }
            case QGis::WKBPolygon:
            {
                // TODO
                return false;
                break;
            }
            case QGis::WKBMultiPoint:
            {
                // TODO
                return false;
                break;
            }

            case QGis::WKBMultiLineString:
            {
                // TODO
                return false;
                break;
            }

            case QGis::WKBMultiPolygon:
            {
                 // TODO
                return false;
                break;
             }
            default:
#ifdef QGISDEBUG
                qWarning("error: mGeometry type not recognized in QgsGeometry::insertVertexBefore");
#endif
                return false;
                break;
        }

#ifdef QGISDEBUG
      std::cout << "QgsGeometry::insertVertexBefore: Exiting successfully." << std::endl;
#endif
        
        return true;
        
    }
    else
    {
#ifdef QGISDEBUG
        qWarning("error: no geometry pointer in QgsGeometry::insertVertexBefore");
#endif
        return false;
    }
    


}


bool QgsGeometry::moveVertexAt(double x, double y, 
                               QgsGeometryVertexIndex atVertex)
{
}


bool QgsGeometry::vertexAt(double &x, double &y, 
                           QgsGeometryVertexIndex atVertex) const
{
#ifdef QGISDEBUG
      std::cout << "QgsGeometry::vertexAt: Entered with "
//         << "atVertex " << atVertex << ", atRing " << atRing << ", atItem"
//         << " " << atItem            
                << "." << std::endl;
#endif

    if(mGeometry)
    {
        int wkbType;

        memcpy(&wkbType, (mGeometry+1), sizeof(int));
        switch (wkbType)
        {
            case QGis::WKBPoint:
            {
                // TODO
                return FALSE;
                break;
            }
            case QGis::WKBLineString:
            {
                unsigned char *ptr;
                int *nPoints;
                int idx;

                // get number of points in the line
                ptr = mGeometry + 5;     // now at mGeometry.numPoints
                nPoints = (int *) ptr;

#ifdef QGISDEBUG
      std::cout << "QgsGeometry::vertexAt: Number of points in WKBLineString is " << *nPoints
                << "." << std::endl;
#endif
                                
                // silently adjust any overflow "atVertex" values to mean
                // "append to end of string"
                if (*nPoints <= atVertex.back())
                {
                  atVertex.assign_back(*nPoints);
                  atVertex.decrement_back();  // atVertex is 0 based, *nPoints is 1 based
                }

                // copy the vertex coordinates                      
                ptr = mGeometry + 9 + (atVertex.back() * 16);
                memcpy(&x, ptr, 8);
                ptr += 8;
                memcpy(&y, ptr, 8);

#ifdef QGISDEBUG
      std::cout << "QgsGeometry::vertexAt: Point is (" << x << ", " << y << ")"
                << "." << std::endl;
#endif
                      
                break;
            }
            case QGis::WKBPolygon:
            {
                // TODO
                return false;
                break;
            }
            case QGis::WKBMultiPoint:
            {
                // TODO
                return false;
                break;
            }

            case QGis::WKBMultiLineString:
            {
                // TODO
                return false;
                break;
            }

            case QGis::WKBMultiPolygon:
            {
                 // TODO
                return false;
                break;
             }
            default:
#ifdef QGISDEBUG
                qWarning("error: mGeometry type not recognized in QgsGeometry::vertexAt");
#endif
                return false;
                break;
        }
        
#ifdef QGISDEBUG
      std::cout << "QgsGeometry::vertexAt: Exiting TRUE." << std::endl;
#endif
        
        return true;
        
    }
    else
    {
#ifdef QGISDEBUG
        qWarning("error: no mGeometry pointer in QgsGeometry::vertexAt");
#endif
        return false;
    }

    
}


QgsPoint QgsGeometry::closestSegmentWithContext(QgsPoint& point, 
                                    QgsGeometryVertexIndex& beforeVertex,
                                    double& minSqrDist)
//QgsPoint QgsGeometry::closestSegment(QgsPoint& point, 
//                                    QgsPoint& segStart, QgsPoint& segStop,
//                                    double& minSqrDist)
{

#ifdef QGISDEBUG
//      std::cout << "QgsGeometry::closestSegment: Entered"
//                << "." << std::endl;
#endif

  QgsPoint minDistPoint;

    int wkbType;
    double x,y;
    double *thisx,*thisy;
    double *prevx,*prevy;
    double testdist;
  
  // Initialise some stuff
  beforeVertex.clear();
  minSqrDist   = DBL_MAX;

  if (mGeometry)
  {
//    int wkbType;
//    double x,y;
//    double *thisx,*thisy;
//    double *prevx,*prevy;
//    double testdist;
    
    memcpy(&wkbType, (mGeometry+1), sizeof(int));
    
    switch (wkbType)
    {
    case QGis::WKBPoint:
      // Points have no lines
      return QgsPoint(0,0);
     
    case QGis::WKBLineString:
      unsigned char* ptr = mGeometry + 5;
      int* npoints = (int*) ptr;
      ptr += sizeof(int);
      for (int index=0; index < *npoints; ++index)
      {
        if (index > 0)
        {
          prevx = thisx;  
          prevy = thisy;  
        }
        
        thisx = (double*) ptr;
        ptr += sizeof(double);
        thisy = (double*) ptr;
        
        if (index > 0)
        {
          if ( 
               ( 
                 testdist = distanceSquaredPointToSegment(point,
                                                          prevx, prevy,
                                                          thisx, thisy,
                                                          minDistPoint)
               )                                           
               < minSqrDist )                              
          {
#ifdef QGISDEBUG
//      std::cout << "QgsGeometry::closestSegment: testDist "
//                << testdist << ", minSqrDist"
//                << minSqrDist
//                << "." << std::endl;
#endif
            
            beforeVertex.push_back(index);
            
            minSqrDist = testdist;
          }
        }
        
        ptr += sizeof(double);
      }
      break;

      // TODO: Other geometry types
      
    } // switch (wkbType)
    
  } // if (mGeometry)

#ifdef QGISDEBUG
      std::cout << "QgsGeometry::closestSegment: Exiting with beforeVertex "
//                << beforeVertex << ", minSqrDist from "
                << point.stringRep() << " is "
                << minSqrDist
                << "." << std::endl;
#endif
      
  return minDistPoint;

}                 


QgsRect QgsGeometry::boundingBox() const
{
    double xmin=DBL_MAX;
    double ymin=DBL_MAX;
    double xmax=-DBL_MAX;
    double ymax=-DBL_MAX;

    double *x;
    double *y;
    int *nPoints;
    int *numRings;
    int *numPolygons;
    int numPoints;
    int numLineStrings;
    int idx, jdx, kdx;
    unsigned char *ptr;
    char lsb;
    QgsPoint pt;
    QPointArray *pa;
    int wkbType;
    unsigned char *mGeometry;

    if(mGeometry)
    {
	wkbType=(int) mGeometry[1];
	switch (wkbType)
      {
      case QGis::WKBPoint:
        x = (double *) (mGeometry + 5);
        y = (double *) (mGeometry + 5 + sizeof(double));
        if (*x < xmin)
        {
          xmin=*x;
        }
        if (*x > xmax)
        {
          xmax=*x;
        }
        if (*y < ymin)
        {
          ymin=*y;
        }
        if (*y > ymax)
        {
          ymax=*y;
        }
        break;

      case QGis::WKBLineString:
        // get number of points in the line
        ptr = mGeometry + 5;
        nPoints = (int *) ptr;
        ptr = mGeometry + 1 + 2 * sizeof(int);
        for (idx = 0; idx < *nPoints; idx++)
        {
          x = (double *) ptr;
          ptr += sizeof(double);
          y = (double *) ptr;
          ptr += sizeof(double);
          if (*x < xmin)
          {
            xmin=*x;
          }
          if (*x > xmax)
          {
	      xmax=*x;
          }
          if (*y < ymin)
          {
            ymin=*y;
          }
          if (*y > ymax)
          {
            ymax=*y;
          }
        }
        break;

      case QGis::WKBMultiLineString:
        numLineStrings = (int) (mGeometry[5]);
        ptr = mGeometry + 9;
        for (jdx = 0; jdx < numLineStrings; jdx++)
        {
          // each of these is a wbklinestring so must handle as such
          lsb = *ptr;
          ptr += 5;   // skip type since we know its 2
          nPoints = (int *) ptr;
          ptr += sizeof(int);
          for (idx = 0; idx < *nPoints; idx++)
          {
            x = (double *) ptr;
            ptr += sizeof(double);
            y = (double *) ptr;
            ptr += sizeof(double);
            if (*x < xmin)
            {
              xmin=*x;
            }
            if (*x > xmax)
            {
              xmax=*x;
            }
            if (*y < ymin)
            {
              ymin=*y;
            }
            if (*y > ymax)
            {
              ymax=*y;
            }
          }
        }
        break;

      case QGis::WKBPolygon:
        // get number of rings in the polygon
        numRings = (int *) (mGeometry + 1 + sizeof(int));
        ptr = mGeometry + 1 + 2 * sizeof(int);
        for (idx = 0; idx < *numRings; idx++)
        {
          // get number of points in the ring
          nPoints = (int *) ptr;
          ptr += 4;
          for (jdx = 0; jdx < *nPoints; jdx++)
          {
            // add points to a point array for drawing the polygon
            x = (double *) ptr;
            ptr += sizeof(double);
            y = (double *) ptr;
            ptr += sizeof(double);
            if (*x < xmin)
            {
              xmin=*x;
            }
            if (*x > xmax)
            {
              xmax=*x;
            }
            if (*y < ymin)
            {
              ymin=*y;
            }
            if (*y > ymax)
            {
              ymax=*y;
            }
          }
        }
        break;

	case QGis::WKBMultiPolygon:
        // get the number of polygons
        ptr = mGeometry + 5;
        numPolygons = (int *) ptr;
        for (kdx = 0; kdx < *numPolygons; kdx++)
        {
          //skip the endian and mGeometry type info and
          // get number of rings in the polygon
          ptr = mGeometry + 14;
          numRings = (int *) ptr;
          ptr += 4;
          for (idx = 0; idx < *numRings; idx++)
          {
            // get number of points in the ring
            nPoints = (int *) ptr;
            ptr += 4;
            for (jdx = 0; jdx < *nPoints; jdx++)
            {
              // add points to a point array for drawing the polygon
              x = (double *) ptr;
              ptr += sizeof(double);
              y = (double *) ptr;
              ptr += sizeof(double);
              if (*x < xmin)
              {
                xmin=*x;
              }
              if (*x > xmax)
              {
                xmax=*x;
              }
              if (*y < ymin)
              {
                ymin=*y;
              }
              if (*y > ymax)
              {
                ymax=*y;
              }
            }
          }
        }
        break;

      default:
	  #ifdef QGISDEBUG
        std::cout << "UNKNOWN WKBTYPE ENCOUNTERED\n";
#endif
        break;

      }
      return QgsRect(xmin,ymin,xmax,ymax);
    }
    else
    {
	return QgsRect(0,0,0,0);
    }
}


bool QgsGeometry::intersects(QgsRect* r) const
{
    bool returnval=false;

    geos::GeometryFactory *gf = new geos::GeometryFactory();
    geos::WKTReader *wktReader = new geos::WKTReader(gf);
    geos::Geometry *geosGeom = wktReader->read( qstrdup(wkt()) );

    //write the selection rectangle to wkt by hand
    QString rectwkt="POLYGON((";
    rectwkt+=QString::number(r->xMin(),'f',3);
    rectwkt+=" ";
    rectwkt+=QString::number(r->yMin(),'f',3);
    rectwkt+=",";
    rectwkt+=QString::number(r->xMax(),'f',3);
    rectwkt+=" ";
    rectwkt+=QString::number(r->yMin(),'f',3);
    rectwkt+=",";
    rectwkt+=QString::number(r->xMax(),'f',3);
    rectwkt+=" ";
    rectwkt+=QString::number(r->yMax(),'f',3);
    rectwkt+=",";
    rectwkt+=QString::number(r->xMin(),'f',3);
    rectwkt+=" ";
    rectwkt+=QString::number(r->yMax(),'f',3);
    rectwkt+=",";
    rectwkt+=QString::number(r->xMin(),'f',3);
    rectwkt+=" ";
    rectwkt+=QString::number(r->yMin(),'f',3);
    rectwkt+="))";
    
    geos::Geometry *geosRect = wktReader->read( qstrdup(rectwkt) );
    if(geosGeom->intersects(geosRect))
    {
	returnval=true;
    }
 
    delete geosGeom;
    delete geosRect;
    delete gf;
    delete wktReader;
    return returnval;
}


bool QgsGeometry::exportToWkt(unsigned char * geom) const
{
    if(geom)
    {
	int wkbType;
	double *x,*y;

	mWkt="";
	memcpy(&wkbType, (geom+1), sizeof(int));
	switch (wkbType)
	{
	    case QGis::WKBPoint:
	    {
		mWkt+="POINT(";
		x = (double *) (geom + 5);
		mWkt+=QString::number(*x,'f',6);
		mWkt+=" ";
		y = (double *) (geom + 5 + sizeof(double));
		mWkt+=QString::number(*y,'f',6);
		mWkt+=")";
		break;
	    }
	    case QGis::WKBLineString:
	    {
		unsigned char *ptr;
		int *nPoints;
		int idx;

		mWkt+="LINESTRING(";
		// get number of points in the line
		ptr = geom + 5;
		nPoints = (int *) ptr;
		ptr = geom + 1 + 2 * sizeof(int);
		for (idx = 0; idx < *nPoints; ++idx)
		{
		    if(idx!=0)
		    {
			mWkt+=", ";
		    }
		    x = (double *) ptr;
		    mWkt+=QString::number(*x,'f',6);
		    mWkt+=" ";
		    ptr += sizeof(double);
		    y = (double *) ptr;
		    mWkt+=QString::number(*y,'f',6);
		    ptr += sizeof(double);
		}
		mWkt+=")";
		break;
	    }
	    case QGis::WKBPolygon:
	    {
		unsigned char *ptr;
		int idx, jdx;
		int *numRings, *nPoints;

		mWkt+="POLYGON(";
		// get number of rings in the polygon
		numRings = (int *)(geom + 1 + sizeof(int));
		if (!(*numRings))  // sanity check for zero rings in polygon
		{
		    break;
		}
		int *ringStart; // index of first point for each ring
		int *ringNumPoints; // number of points in each ring
		ringStart = new int[*numRings];
		ringNumPoints = new int[*numRings];
		ptr = geom+1+2*sizeof(int); // set pointer to the first ring
		for (idx = 0; idx < *numRings; idx++)
		{
		    if(idx!=0)
		    {
			mWkt+=",";
		    }
		    mWkt+="(";
		    // get number of points in the ring
		    nPoints = (int *) ptr;
		    ringNumPoints[idx] = *nPoints;
		    ptr += 4;
		    
		    for(jdx=0;jdx<*nPoints;jdx++)
		    {
			if(jdx!=0)
			{
			    mWkt+=",";
			}
			x = (double *) ptr;
			mWkt+=QString::number(*x,'f',6);
			mWkt+=" ";
			ptr += sizeof(double);
			y = (double *) ptr;
			mWkt+=QString::number(*y,'f',6);
			ptr += sizeof(double);
		    }
		    mWkt+=")";
		}
		mWkt+=")";
		delete [] ringStart;
		delete [] ringNumPoints;
		break;
	    }
	    case QGis::WKBMultiPoint:
	    {
		unsigned char *ptr;
		int idx;
		int *nPoints;

		mWkt+="MULTIPOINT(";
		nPoints=(int*)(geom+5);
		ptr=geom+5+sizeof(int);
		for(idx=0;idx<*nPoints;++idx)
		{
		    if(idx!=0)
		    {
			mWkt+=", ";
		    }
		    x = (double *) (ptr);
		    mWkt+=QString::number(*x,'f',6);
		    mWkt+=" ";
		    ptr+=sizeof(double);
		    y= (double *) (ptr);
		    mWkt+=QString::number(*y,'f',6);
		    ptr+=sizeof(double);
		}
		mWkt+=")";
		break;
	    }

	    case QGis::WKBMultiLineString:
	    {
		unsigned char *ptr;
		int idx, jdx, numLineStrings;
		int *nPoints;

		mWkt+="MULTILINESTRING(";
		numLineStrings = (int) (geom[5]);
		ptr = geom + 9;
		for (jdx = 0; jdx < numLineStrings; jdx++)
		{
		    if(jdx!=0)
		    {
			mWkt+=", ";
		    }
		    mWkt+="(";
		    ptr += 5; // skip type since we know its 2
		    nPoints = (int *) ptr;
		    ptr += sizeof(int);
		    for (idx = 0; idx < *nPoints; idx++)
		    {
			if(idx!=0)
			{
			    mWkt+=", ";
			}
			x = (double *) ptr;
			mWkt+=QString::number(*x,'f',6);
			ptr += sizeof(double);
			mWkt+=" ";
			y = (double *) ptr;
			mWkt+=QString::number(*y,'f',6);
			ptr += sizeof(double);
		    }
		    mWkt+=")";
		}
		mWkt+=")";
		break;
	    }

	    case QGis::WKBMultiPolygon:
	    {
		unsigned char *ptr;
		int idx, jdx, kdx;
		int *numPolygons, *numRings, *nPoints;
		
		mWkt+="MULTIPOLYGON(";
		ptr = geom + 5;
		numPolygons = (int *) ptr;
		ptr = geom + 9;
		for (kdx = 0; kdx < *numPolygons; kdx++)
		{
		    if(kdx!=0)
		    {
			mWkt+=",";
		    }
		    mWkt+="(";
		    ptr+=5;
		    numRings = (int *) ptr;
		    ptr += 4;
		    for (idx = 0; idx < *numRings; idx++)
		    {
			if(idx!=0)
			{
			    mWkt+=",";
			}
			mWkt+="(";
			nPoints = (int *) ptr;
			ptr += 4;
			for (jdx = 0; jdx < *nPoints; jdx++)
			{
			    x = (double *) ptr;
			    mWkt+=QString::number(*x,'f',6);
			    ptr += sizeof(double);
			    mWkt+=" ";
			    y = (double *) ptr;
			    mWkt+=QString::number(*y,'f',6);
			    ptr += sizeof(double);
			}
			mWkt+=")";
		    }
		    mWkt+=")";
		}
		mWkt+=")";
		break;
	    }
	    default:
#ifdef QGISDEBUG
		qWarning("error: mGeometry type not recognized in QgsGeometry::exportToWkt");
#endif
		return false;
		break;
	}
	return true;
	
    }
    else
    {
#ifdef QGISDEBUG
	qWarning("error: no geom pointer in QgsGeometry::exportToWkt");
#endif
	return false;
    }
    
}


bool QgsGeometry::exportToWkt() const
{
  if (mDirtyWkt)
  {
    return (mDirtyWkt = exportToWkt( mGeometry ));
  }
  else
  {
    // Already have a fresh copy of Wkt available.
    return TRUE;
  }  
}



geos::Geometry* QgsGeometry::geosGeometry() const
{

    if (!mDirtyGeos)
    {
      // No need to convert again
      return mGeos;
      
      // TODO: make mGeos useful - assign to it and clear mDirty before we return out of this function
    }

    if(!mGeometry)
    {
	return 0;
    }

#ifdef QGISDEBUG
    qWarning("In QgsGeometry::geosGeometry()");
#endif
	
    double *x;
    double *y;
    int *nPoints;
    int *numRings;
    int *numPolygons;
    int numPoints;
    int numLineStrings;
    int idx, jdx, kdx;
    unsigned char *ptr;
    char lsb;
    QgsPoint pt;
    QPointArray *pa;
    int wkbtype;

    // TODO: Make this a static member - save generating for every geometry
    geos::GeometryFactory* geometryFactory = new geos::GeometryFactory();
    
    wkbtype=(int) mGeometry[1];
    switch(wkbtype)
    {
	case QGis::WKBPoint:
	{
	   x = (double *) (mGeometry + 5);
	   y = (double *) (mGeometry + 5 + sizeof(double));
           
           mDirtyGeos = FALSE;
	   return geometryFactory->createPoint(geos::Coordinate(*x,*y));
	}
	case QGis::WKBMultiPoint:
	{
	    std::vector<geos::Geometry*>* points=new std::vector<geos::Geometry*>;
	    ptr = mGeometry + 5;
	    nPoints = (int *) ptr;
	    ptr = mGeometry + 1 + 2 * sizeof(int);
	    for (idx = 0; idx < *nPoints; idx++)
	    {
		x = (double *) ptr;
		ptr += sizeof(double);
		y = (double *) ptr;
		ptr += sizeof(double);
		points->push_back(geometryFactory->createPoint(geos::Coordinate(*x,*y)));
	    }
	    return geometryFactory->createMultiPoint(points);
	}
	case QGis::WKBLineString:
	{
#ifdef QGISDEBUG
    qWarning("Linestring found");
#endif
	    geos::DefaultCoordinateSequence* sequence=new geos::DefaultCoordinateSequence();
	    ptr = mGeometry + 5;
	    nPoints = (int *) ptr;
	    ptr = mGeometry + 1 + 2 * sizeof(int);
	    for (idx = 0; idx < *nPoints; idx++)
	    {
		x = (double *) ptr;
		ptr += sizeof(double);
		y = (double *) ptr;
		ptr += sizeof(double);
#ifdef QGISDEBUG
			qWarning("adding coordinate pair "+QString::number(*x)+"//"+QString::number(*y));
#endif
		sequence->add(geos::Coordinate(*x,*y));
	    }
	    return geometryFactory->createLineString(sequence); 
	}
	case QGis::WKBMultiLineString:
	{
	    std::vector<geos::Geometry*>* lines=new std::vector<geos::Geometry*>;
	    numLineStrings = (int) (mGeometry[5]);
	    ptr = mGeometry + 9;
	    for (jdx = 0; jdx < numLineStrings; jdx++)
	    {
		geos::DefaultCoordinateSequence* sequence=new geos::DefaultCoordinateSequence();
		// each of these is a wbklinestring so must handle as such
		lsb = *ptr;
		ptr += 5;   // skip type since we know its 2
		nPoints = (int *) ptr;
		ptr += sizeof(int);
		for (idx = 0; idx < *nPoints; idx++)
		{
		    x = (double *) ptr;
		    ptr += sizeof(double);
		    y = (double *) ptr;
		    ptr += sizeof(double);
		    sequence->add(geos::Coordinate(*x,*y));
		}
		lines->push_back(geometryFactory->createLineString(sequence));
	    }
	    return geometryFactory->createMultiLineString(lines);
	}
	case QGis::WKBPolygon: 
	{
#ifdef QGISDEBUG
    qWarning("Polygon found");
#endif
	    // get number of rings in the polygon
	    numRings = (int *) (mGeometry + 1 + sizeof(int));
	    ptr = mGeometry + 1 + 2 * sizeof(int);
	    
	    geos::LinearRing* outer=0;
	    std::vector<geos::Geometry*>* inner=new std::vector<geos::Geometry*>;

	    for (idx = 0; idx < *numRings; idx++)
	    {
#ifdef QGISDEBUG
    qWarning("Ring nr: "+QString::number(idx));
#endif		
		geos::DefaultCoordinateSequence* sequence=new geos::DefaultCoordinateSequence();
		// get number of points in the ring
		nPoints = (int *) ptr;
		ptr += 4;
		for (jdx = 0; jdx < *nPoints; jdx++)
		{
		    // add points to a point array for drawing the polygon
		    x = (double *) ptr;
		    ptr += sizeof(double);
		    y = (double *) ptr;
		    ptr += sizeof(double);
		    sequence->add(geos::Coordinate(*x,*y));
		}
		geos::LinearRing* ring=geometryFactory->createLinearRing(sequence);
		if(idx==0)
		{
		    outer=ring;
		}
		else
		{
		    inner->push_back(ring);
		}
	    }
	    return geometryFactory->createPolygon(outer,inner);
	}
	
	case QGis::WKBMultiPolygon:
	{
#ifdef QGISDEBUG
	    qWarning("Multipolygon found");
#endif
	    std::vector<geos::Geometry *> *polygons=new std::vector<geos::Geometry *>;
	    // get the number of polygons
	    ptr = mGeometry + 5;
	    numPolygons = (int *) ptr;
	    ptr = mGeometry +9;
	    for (kdx = 0; kdx < *numPolygons; kdx++)
	    {
#ifdef QGISDEBUG
		//qWarning("Polygon nr: "+QString::number(kdx));
#endif
		geos::LinearRing* outer=0;
		std::vector<geos::Geometry*>* inner=new std::vector<geos::Geometry*>;

		//skip the endian and mGeometry type info and
		// get number of rings in the polygon
		ptr += 5;
		numRings = (int *) ptr;
		ptr += 4;
		for (idx = 0; idx < *numRings; idx++)
		{
#ifdef QGISDEBUG
		    //qWarning("Ring nr: "+QString::number(idx));
#endif
		    geos::DefaultCoordinateSequence* sequence=new geos::DefaultCoordinateSequence();
		    // get number of points in the ring
		    nPoints = (int *) ptr;
		    ptr += 4;
		    for (jdx = 0; jdx < *nPoints; jdx++)
		    {
			// add points to a point array for drawing the polygon
			x = (double *) ptr;
			ptr += sizeof(double);
			y = (double *) ptr;
			ptr += sizeof(double);
#ifdef QGISDEBUG
			//qWarning("adding coordinate pair "+QString::number(*x)+"//"+QString::number(*y));
#endif
			sequence->add(geos::Coordinate(*x,*y));
		    }
		    geos::LinearRing* ring=geometryFactory->createLinearRing(sequence);
		    if(idx==0)
		    {
			outer=ring;
		    }
		    else
		    {
			inner->push_back(ring);
		    }
		}
	    
		polygons->push_back(geometryFactory->createPolygon(outer,inner));
	    }
	    return (geometryFactory->createMultiPolygon(polygons));
	}
	default:
	    return 0;
    }
    
}


void QgsGeometry::fromGeosGeometry()
// TODO: Make this work
{
  if(!mGeometry)
  {
//    return 0;
  }

}



double QgsGeometry::distanceSquaredPointToSegment(QgsPoint& point,
                                                  double *x1, double *y1,
                                                  double *x2, double *y2,
                                                  QgsPoint& minDistPoint)
{

  double d;
  
  // Proportion along segment (0 to 1) the perpendicular of the point falls upon
  double t;
  
  
  // Projection of point on the segment
  double xn;
  double yn;

  double segmentsqrdist = ( *x2 - *x1 ) * ( *x2 - *x1 ) + 
                          ( *y2 - *y1 ) * ( *y2 - *y1 );

#ifdef QGISDEBUG
//      std::cout << "QgsGeometry::distanceSquaredPointToSegment: Entered with "
//         << "(" << *x1 << ", " << *y1 << ") (" << *x2 << ", " << *y2 << ")"
//         << " " << segmentsqrdist            
//                << "." << std::endl;
#endif
                          
                                                    
  if ( segmentsqrdist == 0.0 )
  {
    // segment is a point 
    xn = *x1;
    yn = *y1;
  }
  else
  {

    d =
        ( point.x() - *x1 ) * ( *x2 - *x1 )
      + ( point.y() - *y1 ) * ( *y2 - *y1 );

    t = d / segmentsqrdist;
    
    // Do not go beyond end of line
    // (otherwise this would be a comparison to an infinite line)
    if (t < 0.0)
    { 
      xn = *x1; 
      yn = *y1;
    }
    else if (t > 1.0)
    {
      xn = *x2; 
      yn = *y2;
    }
    else
    {
      xn = *x1 + t * ( *x2 - *x1 );
      yn = *y1 + t * ( *y2 - *y1 );
    }  

  }
  
  minDistPoint.set(xn, yn);

  return (
          ( xn - point.x() ) * ( xn - point.x() ) + 
          ( yn - point.y() ) * ( yn - point.y() )
         );

}
                   

