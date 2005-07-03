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

#include <limits>

#include "qgis.h"
#include "qgsgeometry.h"


// Set up static GEOS geometry factory
geos::GeometryFactory* QgsGeometry::geosGeometryFactory = new geos::GeometryFactory();


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
  if (mDirtyWkb)
  {
    // see which geometry representation to convert from
    if (mDirtyWkt)
    {
      // convert from GEOS
      exportGeosToWkb();
    }
    else
    {
      // TODO
    }
  }

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

void QgsGeometry::setGeos(geos::Geometry* geos)
{
  // TODO - make this more heap-friendly

  if (mGeos)
  {
    delete mGeos;
    mGeos = 0;
  }

  mGeos = geos;

#ifdef QGISDEBUG
  std::cout << "QgsGeometry::setGeos: setting Geos = '" << mGeos->toString() << "'." << std::endl;
#endif


  mDirtyWkb   = TRUE;
  mDirtyGeos  = FALSE;
  mDirtyWkt   = TRUE;

}

QgsPoint QgsGeometry::closestVertex(const QgsPoint& point) const
{
    if(mGeometry)
    {
	int wkbType;
	double actdist = std::numeric_limits<double>::max();

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
                                     int beforeVertex,
                                     const geos::CoordinateSequence*  old_sequence,
                                           geos::CoordinateSequence** new_sequence)

{
  // Bounds checking
  if (beforeVertex < 0)
  {
    (*new_sequence) = 0;
    return FALSE;
  }

  int numPoints = old_sequence->getSize();

  // Copy to the new sequence, including the new vertex
  (*new_sequence) = new geos::DefaultCoordinateSequence();

  bool inserted = FALSE;
  for (int i = 0; i < numPoints; i++)
  {
    // Do we insert the new vertex here?
    if (beforeVertex == i)
    {
      (*new_sequence)->add( geos::Coordinate(x, y) );
      inserted = TRUE;
    }
    (*new_sequence)->add( old_sequence->getAt(i) );
  }

  if (!inserted)
  {
    // The beforeVertex is greater than the actual number of vertices
    // in the geometry - append it.
    (*new_sequence)->add( geos::Coordinate(x, y) );
  }
// TODO: Check that the sequence is still simple, e.g. with geos::Geometry->isSimple()
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

  // Do just-in-time conversion to GEOS
  // TODO: This should be more of a "import to GEOS" since GEOS will be the home geometry.
  exportWkbToGeos();

  if (mGeos)
  {
    switch (mGeos->getGeometryTypeId())
    {
      case geos::GEOS_POINT:                 // a point
      {
        // Cannot insert a vertex to a point!
        return FALSE;

      } // case geos::GEOS_POINT

      case geos::GEOS_LINESTRING:            // a linestring
      {
        // Get the embedded GEOS Coordinate Sequence
        geos::LineString* geosls = static_cast<geos::LineString*>(mGeos);
        const geos::CoordinateSequence* old_sequence = geosls->getCoordinatesRO();
              geos::CoordinateSequence* new_sequence;

        if ( insertVertexBefore(x, y, beforeVertex.back(), old_sequence, (&new_sequence) ) )
        {
          // Put in the new GEOS geometry
          setGeos( static_cast<geos::Geometry*>( geosGeometryFactory->createLineString(new_sequence) ) );
          return TRUE;
        }
        else
        {
          return FALSE;
        }

      } // case geos::GEOS_LINESTRING

      case geos::GEOS_LINEARRING:            // a linear ring (linestring with 1st point == last point)
      {
        // TODO
        break;
      } // case geos::GEOS_LINEARRING

      case geos::GEOS_POLYGON:               // a polygon
      {
        // TODO
        break;
      } // case geos::GEOS_POLYGON
  
      case geos::GEOS_MULTIPOINT:            // a collection of points
      {
        // TODO
        break;
      } // case geos::GEOS_MULTIPOINT
  
      case geos::GEOS_MULTILINESTRING:       // a collection of linestrings
      {
        // TODO
        break;
      } // case geos::GEOS_MULTILINESTRING
  
      case geos::GEOS_MULTIPOLYGON:          // a collection of polygons
      {
        // TODO
        break;
      } // case geos::GEOS_MULTIPOLYGON
  
      case geos::GEOS_GEOMETRYCOLLECTION:    // a collection of heterogeneus geometries
      {
        // TODO
        break;
      } // case geos::GEOS_GEOMETRYCOLLECTION
  
    } // switch (mGeos->getGeometryTypeId())

  } // if (mGeos)

  return FALSE;


}


bool QgsGeometry::moveVertexAt(double x, double y,
                               int atVertex,
                               const geos::CoordinateSequence*  old_sequence,
                                     geos::CoordinateSequence** new_sequence)
{
  int numPoints = old_sequence->getSize();

  // Bounds checking
  if (
      (atVertex <  0)         ||
      (atVertex >= numPoints)
     )
  {
    (*new_sequence) = 0;
    return FALSE;
  }

  // Copy to the new sequence, including the moved vertex
  (*new_sequence) = new geos::DefaultCoordinateSequence();

  for (int i = 0; i < numPoints; i++)
  {
    // Do we move the vertex here?
    if (atVertex == i)
    {
      (*new_sequence)->add( geos::Coordinate(x, y) );
    }
    else
    {
      (*new_sequence)->add( old_sequence->getAt(i) );
    }
  }
// TODO: Check that the sequence is still simple, e.g. with geos::Geometry->isSimple()
}


bool QgsGeometry::moveVertexAt(double x, double y, 
                               QgsGeometryVertexIndex atVertex)
{

#ifdef QGISDEBUG
      std::cout << "QgsGeometry::moveVertexAt: Entered with "
         << "x " << x << ", y " << y 
//         << "beforeVertex " << beforeVertex << ", atRing " << atRing << ", atItem"
//         << " " << atItem            
                << "." << std::endl;
#endif

  // Do just-in-time conversion to GEOS
  // TODO: This should be more of a "import to GEOS" since GEOS will be the home geometry.
  exportWkbToGeos();

  if (mGeos)
  {
    switch (mGeos->getGeometryTypeId())
    {
      case geos::GEOS_POINT:                 // a point
      {
        // Cannot move an arbitrary vertex to a point!
        return FALSE;

      } // case geos::GEOS_POINT

      case geos::GEOS_LINESTRING:            // a linestring
      {
        // Get the embedded GEOS Coordinate Sequence
        geos::LineString* geosls = static_cast<geos::LineString*>(mGeos);
        const geos::CoordinateSequence* old_sequence = geosls->getCoordinatesRO();
              geos::CoordinateSequence* new_sequence;

        if ( moveVertexAt(x, y, atVertex.back(), old_sequence, (&new_sequence) ) )
        {
          // Put in the new GEOS geometry
          setGeos( static_cast<geos::Geometry*>( geosGeometryFactory->createLineString(new_sequence) ) );
          return TRUE;
        }
        else
        {
          return FALSE;
        }

      } // case geos::GEOS_LINESTRING

      case geos::GEOS_LINEARRING:            // a linear ring (linestring with 1st point == last point)
      {
        // TODO
        break;
      } // case geos::GEOS_LINEARRING
  
      case geos::GEOS_POLYGON:               // a polygon
      {
        // TODO
        break;
      } // case geos::GEOS_POLYGON
  
      case geos::GEOS_MULTIPOINT:            // a collection of points
      {
        // TODO
        break;
      } // case geos::GEOS_MULTIPOINT
  
      case geos::GEOS_MULTILINESTRING:       // a collection of linestrings
      {
        // TODO
        break;
      } // case geos::GEOS_MULTILINESTRING
  
      case geos::GEOS_MULTIPOLYGON:          // a collection of polygons
      {
        // TODO
        break;
      } // case geos::GEOS_MULTIPOLYGON
  
      case geos::GEOS_GEOMETRYCOLLECTION:    // a collection of heterogeneus geometries
      {
        // TODO
        break;
      } // case geos::GEOS_GEOMETRYCOLLECTION
  
    } // switch (mGeos->getGeometryTypeId())

  } // if (mGeos)

  return FALSE;

}


bool QgsGeometry::deleteVertexAt(int atVertex,
                                 const geos::CoordinateSequence*  old_sequence,
                                       geos::CoordinateSequence** new_sequence)
{
  int numPoints = old_sequence->getSize();

  // Bounds checking
  if (
      (atVertex <  0)         ||
      (atVertex >= numPoints) ||
      (numPoints <= 2)            // guard against collapsing to a point
     )
  {
    (*new_sequence) = 0;
    return FALSE;
  }

  // Copy to the new sequence, excepting the deleted vertex
  (*new_sequence) = new geos::DefaultCoordinateSequence();

  for (int i = 0; i < numPoints; i++)
  {
    // Do we delete (omit) the vertex here?
    if (atVertex != i)
    {
      (*new_sequence)->add( old_sequence->getAt(i) );
    }
  }

  // TODO: Check that the sequence is still simple, e.g. with geos::Geometry->isSimple()
}


bool QgsGeometry::deleteVertexAt(QgsGeometryVertexIndex atVertex)
{

#ifdef QGISDEBUG
      std::cout << "QgsGeometry::deleteVertexAt: Entered"
                << "." << std::endl;
#endif

  // Do just-in-time conversion to GEOS
  // TODO: This should be more of a "import to GEOS" since GEOS will be the home geometry.
  exportWkbToGeos();

  if (mGeos)
  {
    switch (mGeos->getGeometryTypeId())
    {
      case geos::GEOS_POINT:                 // a point
      {
        // Cannot delete an arbitrary vertex of a point!
        return FALSE;

      } // case geos::GEOS_POINT

      case geos::GEOS_LINESTRING:            // a linestring
      {
         // Get the embedded GEOS Coordinate Sequence
        geos::LineString* geosls = static_cast<geos::LineString*>(mGeos);
        const geos::CoordinateSequence* old_sequence = geosls->getCoordinatesRO();
              geos::CoordinateSequence* new_sequence;

        if ( deleteVertexAt(atVertex.back(), old_sequence, (&new_sequence) ) )
        {
          // Put in the new GEOS geometry
          setGeos( static_cast<geos::Geometry*>( geosGeometryFactory->createLineString(new_sequence) ) );
          return TRUE;
        }
        else
        {
          return FALSE;
        }

      } // case geos::GEOS_LINESTRING
  
      case geos::GEOS_LINEARRING:            // a linear ring (linestring with 1st point == last point)
      {
        // TODO
        break;
      } // case geos::GEOS_LINEARRING
  
      case geos::GEOS_POLYGON:               // a polygon
      {
        // TODO
        break;
      } // case geos::GEOS_POLYGON
  
      case geos::GEOS_MULTIPOINT:            // a collection of points
      {
        // TODO
        break;
      } // case geos::GEOS_MULTIPOINT
  
      case geos::GEOS_MULTILINESTRING:       // a collection of linestrings
      {
        // TODO
        break;
      } // case geos::GEOS_MULTILINESTRING
  
      case geos::GEOS_MULTIPOLYGON:          // a collection of polygons
      {
        // TODO
        break;
      } // case geos::GEOS_MULTIPOLYGON
  
      case geos::GEOS_GEOMETRYCOLLECTION:    // a collection of heterogeneus geometries
      {
        // TODO
        break;
      } // case geos::GEOS_GEOMETRYCOLLECTION
  
    } // switch (mGeos->getGeometryTypeId())

  } // if (mGeos)

  return FALSE;

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
                // return error if underflow
                if (0 > atVertex.back())
                {
                  return FALSE;
                }
                // return error if overflow
                if (*nPoints <= atVertex.back())
                {
                  return FALSE;
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


QgsPoint QgsGeometry::closestVertexWithContext(QgsPoint& point,
                                               QgsGeometryVertexIndex& atVertex,
                                               double& sqrDist)
// TODO
{

#ifdef QGISDEBUG
      std::cout << "QgsGeometry::closestVertexWithContext: Entered"
                << "." << std::endl;
#endif

  QgsPoint minDistPoint;

  // Initialise some stuff
  atVertex.clear();
  sqrDist   = std::numeric_limits<double>::max();
  int closestVertexIndex = 0;

  // set up the GEOS geometry
  exportWkbToGeos();

  if (mGeos)
  {
    switch (mGeos->getGeometryTypeId())
    {
      case geos::GEOS_POINT:                 // a point
      {
        atVertex.push_back(closestVertexIndex);

        minDistPoint = QgsPoint(
                                 mGeos->getCoordinate()->x,
                                 mGeos->getCoordinate()->y
                               );

        break;
      } // case geos::GEOS_POINT

      case geos::GEOS_LINESTRING:            // a linestring
      {
        int numPoints = mGeos->getNumPoints();

        geos::LineString* geosls = static_cast<geos::LineString*>(mGeos);
        const geos::CoordinateSequence* sequence = geosls->getCoordinatesRO();

        // go through points
        for (int n = 0; n < numPoints; n++)
        {
          double testDist = point.sqrDist(
                                           sequence->getAt(n).x,
                                           sequence->getAt(n).y
                                         );

          if (testDist < sqrDist)
          {
            closestVertexIndex = n;
            sqrDist = testDist;
          }
        }

        atVertex.push_back(closestVertexIndex);

        break;
      } // case geos::GEOS_LINESTRING
  
      case geos::GEOS_LINEARRING:            // a linear ring (linestring with 1st point == last point)
      {
        // TODO
        break;
      } // case geos::GEOS_LINEARRING
  
      case geos::GEOS_POLYGON:               // a polygon
      {
        // TODO
        break;
      } // case geos::GEOS_POLYGON
  
      case geos::GEOS_MULTIPOINT:            // a collection of points
      {
        // TODO
        break;
      } // case geos::GEOS_MULTIPOINT
  
      case geos::GEOS_MULTILINESTRING:       // a collection of linestrings
      {
        // TODO
        break;
      } // case geos::GEOS_MULTILINESTRING
  
      case geos::GEOS_MULTIPOLYGON:          // a collection of polygons
      {
        // TODO
        break;
      } // case geos::GEOS_MULTIPOLYGON
  
      case geos::GEOS_GEOMETRYCOLLECTION:    // a collection of heterogeneus geometries
      {
        // TODO
        break;
      } // case geos::GEOS_GEOMETRYCOLLECTION
    }

  } // if (mGeos)

  return minDistPoint;

}


QgsPoint QgsGeometry::closestSegmentWithContext(QgsPoint& point,
                                                QgsGeometryVertexIndex& beforeVertex,
                                                double& sqrDist)
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
  sqrDist   = std::numeric_limits<double>::max();

// TODO: Convert this to a GEOS comparison not a WKB comparison.
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
      int closestSegmentIndex = 0;
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
                                                          minDistPoint)  // TODO: save minDistPoint into something meaningful.
               )
               < sqrDist )
          {
#ifdef QGISDEBUG
//      std::cout << "QgsGeometry::closestSegment: testDist "
//                << testdist << ", sqrDist"
//                << sqrDist
//                << "." << std::endl;
#endif
            closestSegmentIndex = index;
            sqrDist = testdist;
          }
        }
        
        ptr += sizeof(double);
      }

      beforeVertex.push_back(closestSegmentIndex);

      break;

      // TODO: Other geometry types
      
    } // switch (wkbType)
    
  } // if (mGeometry)

#ifdef QGISDEBUG
      std::cout << "QgsGeometry::closestSegment: Exiting with beforeVertex "
//                << beforeVertex << ", sqrDist from "
                << point.stringRep() << " is "
                << sqrDist
                << "." << std::endl;
#endif
      
  return minDistPoint;  // TODO: Is this meaningful?

}                 


QgsRect QgsGeometry::boundingBox() const
{
    double xmin =  std::numeric_limits<double>::max();
    double ymin =  std::numeric_limits<double>::max();
    double xmax = -std::numeric_limits<double>::max();
    double ymax = -std::numeric_limits<double>::max();

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
      // consider endian when fetching feature type
      wkbType = (mGeometry[0] == 1) ? mGeometry[1] : mGeometry[4];
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
#ifdef QGISDEBUG
  std::cout << "QgsGeometry::exportToWkt: entered." << std::endl;
#endif

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
  std::cout << "QgsGeometry::geosGeometry: entered." << std::endl;
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

//    // TODO: Make this a static member - save generating for every geometry
//    geos::GeometryFactory* geometryFactory = new geos::GeometryFactory();
    
    wkbtype=(int) mGeometry[1];
    switch(wkbtype)
    {
	case QGis::WKBPoint:
	{
	   x = (double *) (mGeometry + 5);
	   y = (double *) (mGeometry + 5 + sizeof(double));
           
           mDirtyGeos = FALSE;
	   return geosGeometryFactory->createPoint(geos::Coordinate(*x,*y));
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
		points->push_back(geosGeometryFactory->createPoint(geos::Coordinate(*x,*y)));
	    }
	    return geosGeometryFactory->createMultiPoint(points);
	}
	case QGis::WKBLineString:
	{
#ifdef QGISDEBUG
    qWarning("QgsGeometry::geosGeometry: Linestring found");
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
		qWarning("QgsGeometry::geosGeometry: adding coordinate pair "+QString::number(*x)+"//"+QString::number(*y));
#endif
		sequence->add(geos::Coordinate(*x,*y));
	    }
	    return geosGeometryFactory->createLineString(sequence); 
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
		lines->push_back(geosGeometryFactory->createLineString(sequence));
	    }
	    return geosGeometryFactory->createMultiLineString(lines);
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
		geos::LinearRing* ring=geosGeometryFactory->createLinearRing(sequence);
		if(idx==0)
		{
		    outer=ring;
		}
		else
		{
		    inner->push_back(ring);
		}
	    }
	    return geosGeometryFactory->createPolygon(outer,inner);
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
		    geos::LinearRing* ring=geosGeometryFactory->createLinearRing(sequence);
		    if(idx==0)
		    {
			outer=ring;
		    }
		    else
		    {
			inner->push_back(ring);
		    }
		}
	    
		polygons->push_back(geosGeometryFactory->createPolygon(outer,inner));
	    }
	    return (geosGeometryFactory->createMultiPolygon(polygons));
	}
	default:
	    return 0;
    }
    
}

bool QgsGeometry::exportWkbToGeos() const
{
#ifdef QGISDEBUG
  std::cout << "QgsGeometry::exportWkbToGeos: entered." << std::endl;
#endif

  if (mDirtyGeos)
  {
    // TODO: Clean up the nomenclature a bit
    mGeos = geosGeometry();
    mDirtyGeos = FALSE;
    return mDirtyGeos;
  }
  else
  {
    // Already have a fresh copy of Geos available.
    return TRUE;
  }
}


bool QgsGeometry::exportGeosToWkb() const
// TODO: Make this work
{
#ifdef QGISDEBUG
  std::cout << "QgsGeometry::exportGeosToWkb: entered." << std::endl;
#endif

  if (!mDirtyWkb)
  {
    // No need to convert again
    return TRUE;
  }

  // clear the WKB, ready to replace with the new one
  if (mGeometry)
  {
    delete [] mGeometry;
    mGeometry = 0;
  }

  if (!mGeos)
  {
    // GEOS is null, therefore WKB is null.
    return TRUE;
  }

  // set up byteOrder
  char byteOrder = 1;   // TODO

#ifdef QGISDEBUG
  std::cout << "QgsGeometry::exportGeosToWkb: Testing mGeos->getGeometryTypeId() = '"
            << mGeos->getGeometryTypeId() << "'." << std::endl;
#endif

  switch (mGeos->getGeometryTypeId())
  {
    case geos::GEOS_POINT:                 // a point
    {
      // TODO
      break;
    } // case geos::GEOS_POINT

    case geos::GEOS_LINESTRING:            // a linestring
    {
#ifdef QGISDEBUG
  std::cout << "QgsGeometry::exportGeosToWkb: Got a geos::GEOS_LINESTRING." << std::endl;
#endif

      // TODO
      int numPoints = mGeos->getNumPoints();

      // allocate some space for the WKB
      mGeometrySize = 1 +   // sizeof(byte)
                      4 +   // sizeof(uint32)
                      4 +   // sizeof(uint32)
                   ( (sizeof(double) +
                      sizeof(double)) * numPoints );

      mGeometry = new unsigned char[mGeometrySize];

      unsigned char* ptr = mGeometry;

      // assign byteOrder
      memcpy(ptr, &byteOrder, 1);
      ptr += 1;

      // assign wkbType
      int wkbType = QGis::WKBLineString;
      memcpy(ptr, &wkbType, 4);
      ptr += 4;

      // assign numPoints
      memcpy(ptr, &numPoints, 4);
      ptr += 4;

      geos::LineString* geosls = static_cast<geos::LineString*>(mGeos);
      const geos::CoordinateSequence* sequence = geosls->getCoordinatesRO();

      // assign points
      for (int n = 0; n < numPoints; n++)
      {
#ifdef QGISDEBUG
  std::cout << "QgsGeometry::exportGeosToWkb: Adding " 
            << sequence->getAt(n).x << ", " 
            << sequence->getAt(n).y << "." << std::endl;
#endif
        // assign x
        memcpy(ptr, &(sequence->getAt(n).x), sizeof(double));
        ptr += sizeof(double);

        // assign y
        memcpy(ptr, &(sequence->getAt(n).y), sizeof(double));
        ptr += sizeof(double);
      }

      mDirtyWkb = FALSE;

      // TODO: Deal with endian-ness

      break;
    } // case geos::GEOS_LINESTRING

    case geos::GEOS_LINEARRING:            // a linear ring (linestring with 1st point == last point)
    {
      // TODO
      break;
    } // case geos::GEOS_LINEARRING

    case geos::GEOS_POLYGON:               // a polygon
    {
      // TODO
      break;
    } // case geos::GEOS_POLYGON

    case geos::GEOS_MULTIPOINT:            // a collection of points
    {
      // TODO
      break;
    } // case geos::GEOS_MULTIPOINT

    case geos::GEOS_MULTILINESTRING:       // a collection of linestrings
    {
      // TODO
      break;
    } // case geos::GEOS_MULTILINESTRING

    case geos::GEOS_MULTIPOLYGON:          // a collection of polygons
    {
      // TODO
      break;
    } // case geos::GEOS_MULTIPOLYGON

    case geos::GEOS_GEOMETRYCOLLECTION:    // a collection of heterogeneus geometries
    {
      // TODO
      break;
    } // case geos::GEOS_GEOMETRYCOLLECTION

  } // switch (mGeos->getGeometryTypeId())

  return FALSE;
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
                   

