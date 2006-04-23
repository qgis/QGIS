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
    
    mDirtyWkb(FALSE),
    mDirtyWkt(FALSE),
    mDirtyGeos(FALSE)
{
  // NOOP
}    


QgsGeometry::QgsGeometry( QgsGeometry const & rhs )
    : mGeometry(0),
      mGeometrySize( rhs.mGeometrySize ),
      mWkt( rhs.mWkt ),
      
      mDirtyWkb( rhs.mDirtyWkb ),
      mDirtyWkt( rhs.mDirtyWkt ),
      mDirtyGeos( rhs.mDirtyGeos )
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
  if(mDirtyWkb)
    {
      exportGeosToWkb();
    }

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

  return inserted;
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
	  mDirtyWkb = true;
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
        if(insertVertexToPolygon(beforeVertex.back(), x, y))
	  {
	    mDirtyWkb = true;
	    return true;
	  }
	else
	  {
	    return false;
	  }
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

bool QgsGeometry::moveVertexAt(double x, double y, QgsGeometryVertexIndex atVertex)
{
  int vertexnr = atVertex.back();

  if(mDirtyWkb)
    {
      exportGeosToWkb();
    }

  unsigned int wkbType;
  double *xPtr, *yPtr;
  unsigned char* ptr = mGeometry+1;
  memcpy(&wkbType, ptr, sizeof(wkbType));
  ptr += sizeof(wkbType);

  switch(wkbType)
    {
    case QGis::WKBPoint:
      {
	if(vertexnr == 0)
	  {
	    memcpy(ptr, &x, sizeof(double));
	    ptr += sizeof(double);
	    memcpy(ptr, &y, sizeof(double));
	    return true;
	  }
	else
	  {
	    return false;
	  }
      }
    case QGis::WKBMultiPoint:
    case QGis::WKBLineString:
      {
	int* nrPoints = (int*)ptr;
	if(vertexnr > *nrPoints || vertexnr < 0)
	  {
	    return false;
	  }
	ptr += sizeof(int);
	ptr += 2*sizeof(double)*vertexnr;
	memcpy(ptr, &x, sizeof(double));
	ptr += sizeof(double);
	memcpy(ptr, &y, sizeof(double));
	return true;
      }
    case QGis::WKBMultiLineString:
      {
	int* nrLines = (int*)ptr;
	ptr += sizeof(int);
	int* nrPoints = 0; //numer of points in a line
	int pointindex = 0;
	for(int linenr = 0; linenr < *nrLines; ++linenr)
	  {
	    nrPoints = (int*)ptr;
	    ptr += sizeof(int);
	    if(vertexnr >= pointindex && vertexnr < pointindex + (*nrPoints))
	      {
		ptr += (vertexnr-pointindex)*2*sizeof(double);
		memcpy(ptr, &x, sizeof(double));
		memcpy(ptr+sizeof(double), &y, sizeof(double));
		return true;
	      }
	    pointindex += (*nrPoints);
	    ptr += 2*sizeof(double)*(*nrPoints);
	  }
	return false;
      }
    case QGis::WKBPolygon:
      {
	int* nrRings = (int*)ptr;
	ptr += sizeof(int);
	int* nrPoints = 0; //numer of points in a ring
	int pointindex = 0;

	for(int ringnr = 0; ringnr < *nrRings; ++ringnr)
	  {
	    nrPoints = (int*)ptr;
	    ptr += sizeof(int);
	    if(vertexnr == pointindex || vertexnr == pointindex + (*nrPoints-1))//move two points
	      {
		memcpy(ptr, &x, sizeof(double));
		memcpy(ptr+sizeof(double), &y, sizeof(double));
		memcpy(ptr+2*sizeof(double)*(*nrPoints-1), &x, sizeof(double));
		memcpy(ptr+sizeof(double)+2*sizeof(double)*(*nrPoints-1), &y, sizeof(double));
		return true;
	      }
	    else if(vertexnr > pointindex && vertexnr < pointindex + (*nrPoints-1))//move only one point
	      {
		ptr += 2*sizeof(double)*(vertexnr - pointindex);
		memcpy(ptr, &x, sizeof(double));
		ptr += sizeof(double);
		memcpy(ptr, &y, sizeof(double));
		return true;
	      }
	    ptr += 2*sizeof(double)*(*nrPoints);
	    pointindex += *nrPoints;
	  }
	return false;
      }
    case QGis::WKBMultiPolygon:
      {
	int* nrPolygons = (int*)ptr;
	ptr += sizeof(int);
	int* nrRings = 0; //number of rings in a polygon
	int* nrPoints = 0; //number of points in a ring
	int pointindex = 0;

	for(int polynr = 0; polynr < *nrPolygons; ++polynr)
	  {
	    nrRings = (int*)ptr;
	    ptr += sizeof(int);
	    for(int ringnr = 0; ringnr< *nrRings; ++ringnr)
	      {
		nrPoints = (int*)ptr;
		ptr += sizeof(int);
		if(vertexnr == pointindex || vertexnr == pointindex + (*nrPoints-1))//move two points
		  {
		    memcpy(ptr, &x, sizeof(double));
		    memcpy(ptr+sizeof(double), &y, sizeof(double));
		    memcpy(ptr+2*sizeof(double)*(*nrPoints-1), &x, sizeof(double));
		    memcpy(ptr+sizeof(double)+2*sizeof(double)*(*nrPoints-1), &y, sizeof(double));
		    return true;
		  }
		else if(vertexnr > pointindex && vertexnr < pointindex + (*nrPoints-1))//move only one point
		  {
		    ptr += 2*sizeof(double)*(vertexnr - pointindex);
		    memcpy(ptr, &x, sizeof(double));
		    ptr += sizeof(double);
		    memcpy(ptr, &y, sizeof(double));
		    return true;
		  }
		ptr += 2*sizeof(double)*(*nrPoints);
		pointindex += *nrPoints;
	      }
	  }
	return false;
      }
    }
}



#if 0
bool QgsGeometry::moveVertexAt(double x, double y, 
                               QgsGeometryVertexIndex atVertex)
{
  exportWkbToGeos();
  if (mGeos)
  {
    switch (mGeos->getGeometryTypeId())
    {
    case geos::GEOS_POINT:
      {
	return false;
      }  
    case geos::GEOS_LINESTRING:
      {
	geos::CoordinateSequence* sequence = mGeos->getCoordinates();
	sequence->setAt(geos::Coordinate(x, y), atVertex.back());
	setGeos( static_cast<geos::Geometry*>( geosGeometryFactory->createLineString(sequence) ) );
	mDirtyWkb = true;
	return true;
      } 
    case geos::GEOS_POLYGON:
      {
	if(movePolygonVertex(atVertex.back(), x, y))
	  {
	    mDirtyWkb = true;
	    return true;
	  }
	else
	  {
	    return false;
	  }
      }
    }
  }
		    
  return false;
}
#endif

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
        geos::CoordinateSequence* sequence = geosls->getCoordinates();
        sequence->deleteAt(atVertex.back());
	geos::LineString* newLineString = geosGeometryFactory->createLineString(sequence);
	if(newLineString)
	  {
	    setGeos(newLineString);
	    mDirtyWkb = true;
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
        if(deleteVertexFromPolygon(atVertex.back()))
	  {
	    mDirtyWkb = true;
	    return true;
	  }
	else
	  {
	    return false;
	  }
      }
  
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
      if(mGeos)//try to find the vertex from the Geos geometry (it present)
	{
	  geos::CoordinateSequence* cs = mGeos->getCoordinates();
	  if(cs)
	    {
	      const geos::Coordinate& coord = cs->getAt(atVertex.back());
	      x = coord.x;
	      y = coord.y;
	      delete cs;
	      return true;
	    }
	}

      else if(mGeometry)
	{
	  int wkbType;
	  unsigned char* ptr;

	  memcpy(&wkbType, (mGeometry+1), sizeof(int));
	  switch (wkbType)
	    {
            case QGis::WKBPoint:
	      {
                if(atVertex.back() == 0)
		  {
		    ptr = mGeometry+1+sizeof(int);
		    memcpy(&x, ptr, sizeof(double));
		    ptr += sizeof(double);
		    memcpy(&y, ptr, sizeof(double));
		  }
		else
		  {
		    return FALSE;
		  }
                break;
	      }
            case QGis::WKBLineString:
	      {
                int *nPoints;
                // get number of points in the line
                ptr = mGeometry + 1 + sizeof(int);     // now at mGeometry.numPoints
                nPoints = (int *) ptr;
		
                // return error if underflow
                if (0 > atVertex.back() || *nPoints <= atVertex.back())
		  {
		    return FALSE;
		  }
                // copy the vertex coordinates                      
                ptr = mGeometry + 9 + (atVertex.back() * 2*sizeof(double));
                memcpy(&x, ptr, sizeof(double));
                ptr += sizeof(double);
                memcpy(&y, ptr, sizeof(double));
                break;
	      }
            case QGis::WKBPolygon:
	      {
                int *nRings;
		int *nPoints = 0;
		ptr = mGeometry+1+sizeof(int);
		nRings = (int*)ptr;
		ptr += sizeof(int);
		int pointindex = 0;
		for(int ringnr = 0; ringnr < *nRings; ++ringnr)
		  {
		    nPoints = (int*)ptr;
		    ptr += sizeof(int);
		    for(int pointnr = 0; pointnr < *nPoints; ++pointnr)
		      {
			if(pointindex == atVertex.back())
			  {
			    memcpy(&x, ptr, sizeof(double));
			    ptr += sizeof(double);
			    memcpy(&y, ptr, sizeof(double));
			    break;
			  }
			ptr += 2*sizeof(double);
			++pointindex;
		      }
		  }
		return false;
	      }
            case QGis::WKBMultiPoint:
	      {
                ptr = mGeometry+1+sizeof(int);
		int* nPoints = (int*)ptr;
		if(atVertex.back() < 0 || atVertex.back() >= *nPoints)
		  {
		    return false;
		  }
		ptr += atVertex.back()*2*sizeof(double);
		memcpy(&x, ptr, sizeof(double));
		ptr += sizeof(double);
		memcpy(&y, ptr, sizeof(double));
                break;
	      }    
            case QGis::WKBMultiLineString:
	      {
                ptr = mGeometry+1+sizeof(int);
		int* nLines = (int*)ptr;
		int* nPoints = 0; //number of points in a line
		int pointindex = 0; //global point counter
		ptr += sizeof(int);
		for(int linenr = 0; linenr < *nLines; ++linenr)
		  {
		    nPoints = (int*)ptr;
		    ptr += sizeof(int);
		    for(int pointnr = 0; pointnr < *nPoints; ++pointnr)
		      {
			if(pointindex == atVertex.back())
			  {
			    memcpy(&x, ptr, sizeof(double));
			    ptr += sizeof(double);
			    memcpy(&y, ptr, sizeof(double));
			    break;
			  }
			ptr += 2*sizeof(double);
			++pointindex;
		      }
		  }
                return false;
	      }
            case QGis::WKBMultiPolygon:
	      {
		ptr = mGeometry+1+sizeof(int);
		int* nRings = 0;//number of rings in a polygon
		int* nPoints = 0;//number of points in a ring
		int pointindex = 0; //global point counter
		int* nPolygons = (int*)ptr;
		ptr += sizeof(int);
		for(int polynr = 0; polynr < *nPolygons; ++polynr)
		  {
		    nRings = (int*)ptr;
		    ptr += sizeof(int);
		    for(int ringnr = 0; ringnr < *nRings; ++ringnr)
		      {
			nPoints = (int*)ptr;
			for(int pointnr = 0; pointnr < *nPoints; ++pointnr)
			  {
			    if(pointindex == atVertex.back())
			      {
				memcpy(&x, ptr, sizeof(double));
				ptr += sizeof(double);
				memcpy(&y, ptr, sizeof(double));
				break;
			      }
			    ++pointindex;
			    ptr += 2*sizeof(double);
			  }
		      }
		  }
                return false;
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
{
  QgsPoint minDistPoint;

  // Initialise some stuff
  atVertex.clear();
  sqrDist   = std::numeric_limits<double>::max();
  int closestVertexIndex = 0;

  // set up the GEOS geometry
  exportWkbToGeos();

  if (mGeos)
  {
    geos::CoordinateSequence* sequence = mGeos->getCoordinates();
	if(sequence)
	  {
	    for(int i = 0; i < sequence->getSize(); ++i)
	      {
		double testDist = point.sqrDist(sequence->getAt(i).x, sequence->getAt(i).y);
		if(testDist < sqrDist)
		  {
		    closestVertexIndex = i;
		    sqrDist = testDist;
		  }
	      }
	  }
	atVertex.push_back(closestVertexIndex);
  } // if (mGeos)
  return minDistPoint;
}


QgsPoint QgsGeometry::closestSegmentWithContext(QgsPoint& point,
                                                QgsGeometryVertexIndex& beforeVertex,
                                                double& sqrDist)
{
  QgsPoint minDistPoint;

  int wkbType; 
  double *thisx,*thisy;
  double *prevx,*prevy;
  double testdist;
  int closestSegmentIndex = 0;

  // Initialise some stuff
  beforeVertex.clear();
  sqrDist   = std::numeric_limits<double>::max();

  if(mDirtyWkb) //convert latest geos to mGeometry
    {
      exportGeosToWkb();
    }

  if (mGeometry)
  { 
    memcpy(&wkbType, (mGeometry+1), sizeof(int));
    
    switch (wkbType)
    {
    case QGis::WKBPoint:
    case QGis::WKBMultiPoint:  
      {
	// Points have no lines
	return QgsPoint(0,0);
      }
    case QGis::WKBLineString:
      {
	unsigned char* ptr = mGeometry+1+sizeof(int);
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
		if((testdist = distanceSquaredPointToSegment(point, prevx, prevy, thisx, thisy, minDistPoint)) < sqrDist )
		  {
		    closestSegmentIndex = index;
		    sqrDist = testdist;
		  }
	      }
	    ptr += sizeof(double);
	  }
	beforeVertex.push_back(closestSegmentIndex);
	break;
      }
    case QGis::WKBMultiLineString:
      {
	unsigned char* ptr = mGeometry+1+sizeof(int);
	int* nLines = (int*)ptr;
	ptr += sizeof(int);
	int* nPoints = 0; //number of points in a line
	int pointindex = 0;//global pointindex
	for(int linenr = 0; linenr < *nLines; ++linenr)
	  {
	    nPoints = (int*)ptr;
	    ptr += sizeof(int);
	    prevx = 0;
	    prevy = 0;
	    for(int pointnr = 0; pointnr < *nPoints; ++pointnr)
	      {
		thisx = (double*) ptr;
		ptr += sizeof(double);
		thisy = (double*) ptr;
		ptr += sizeof(double);
		if(prevx && prevy)
		  {
		    if((testdist = distanceSquaredPointToSegment(point, prevx, prevy, thisx, thisy, minDistPoint)) < sqrDist )
		      {
			closestSegmentIndex = pointindex;
			sqrDist = testdist;
		      }
		  }
		prevx = thisx;
		prevy = thisy;
		++pointindex;
	      }
	  }
	beforeVertex.push_back(closestSegmentIndex);
	break;
      }
    case QGis::WKBPolygon:
      {
	int index = 0;
	unsigned char* ptr = mGeometry+1+sizeof(int);
	int* nrings = (int*)ptr;
	int* npoints = 0; //number of points in a ring
	ptr += sizeof(int);
	for(int ringnr = 0; ringnr < *nrings; ++ringnr)//loop over rings
	  {
	    npoints = (int*)ptr;
	    ptr += sizeof(int);
	    prevx = 0;
	    prevy = 0;
	    for(int pointnr = 0; pointnr < *npoints; ++pointnr)//loop over points in a ring
	      {
		thisx = (double*)ptr;
		ptr += sizeof(double);
		thisy = (double*)ptr;
		ptr += sizeof(double);
		if(prevx && prevy)
		  {
		    if((testdist = distanceSquaredPointToSegment(point, prevx, prevy, thisx, thisy, minDistPoint)) < sqrDist )
		      {
			closestSegmentIndex = index;
			sqrDist = testdist;
		      }
		  }
		prevx = thisx;
		prevy = thisy;
		++index;
	      }
	  }
	beforeVertex.push_back(closestSegmentIndex);
	break;
      }
    case QGis::WKBMultiPolygon:
      {
	unsigned char* ptr = mGeometry+1+sizeof(int);
	int* nRings = 0;
	int* nPoints = 0;
	int pointindex = 0;
	int* nPolygons = (int*)ptr;
	ptr += sizeof(int);
	for(int polynr = 0; polynr < *nPolygons; ++polynr)
	  {
	    nRings = (int*)ptr;
	    ptr += sizeof(int);
	    for(int ringnr = 0; ringnr < *nRings; ++ringnr)
	      {
		nPoints = (int*)ptr;
		prevx = 0;
		prevy = 0;
		for(int pointnr = 0; pointnr < *nPoints; ++pointnr)
		  {
		    thisx = (double*)ptr;
		    ptr += sizeof(double);
		    thisy = (double*)ptr;
		    ptr += sizeof(double);
		    if(prevx && prevy)
		      {
			if((testdist = distanceSquaredPointToSegment(point, prevx, prevy, thisx, thisy, minDistPoint)) < sqrDist )
			  {
			    closestSegmentIndex = pointindex;
			    sqrDist = testdist;
			  }
		      }
		    prevx = thisx;
		    prevy = thisy;
		    ++pointindex;
		  }
	      }
	  }
	beforeVertex.push_back(closestSegmentIndex);
	break;
      }
    } // switch (wkbType)
    
  } // if (mGeometry)

#ifdef QGISDEBUG
      std::cout << "QgsGeometry::closestSegment: Exiting with beforeVertex "
//                << beforeVertex << ", sqrDist from "
                << point.stringRep().toLocal8Bit().data() << " is "
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
    int numLineStrings;
    int idx, jdx, kdx;
    unsigned char *ptr;
    char lsb;
    QgsPoint pt;
    int wkbType;

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
	ptr += 4;

        for (kdx = 0; kdx < *numPolygons; kdx++)
        {
          //skip the endian and mGeometry type info and
          // get number of rings in the polygon
          ptr +=5;
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
	wkbType = (geom[0] == 1) ? geom[1] : geom[4];
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
    int numLineStrings;
    int idx, jdx, kdx;
    unsigned char *ptr;
    char lsb;
    QgsPoint pt;
    int wkbtype;

//    // TODO: Make this a static member - save generating for every geometry
//    geos::GeometryFactory* geometryFactory = new geos::GeometryFactory();
    
    wkbtype = (mGeometry[0] == 1) ? mGeometry[1] : mGeometry[4];
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
      return true;

      // TODO: Deal with endian-ness
    } // case geos::GEOS_LINESTRING

    case geos::GEOS_LINEARRING:            // a linear ring (linestring with 1st point == last point)
    {
      // TODO
      break;
    } // case geos::GEOS_LINEARRING

    case geos::GEOS_POLYGON:               // a polygon
    {
      int geometrySize;
      double x, y; //point coordinates
      geos::Polygon* thePolygon = dynamic_cast<geos::Polygon*>(mGeos);
      const geos::LineString* theRing = 0;
      int nPointsInRing = 0;

      if(thePolygon)
	{
	  //first calculate the geometry size
	  geometrySize = 1 + 2*sizeof(int); //endian, type, number of rings
	  theRing = thePolygon->getExteriorRing();
	  if(theRing)
	    {
	      geometrySize += sizeof(int);
	      geometrySize += theRing->getNumPoints()*2*sizeof(double);
	    }
	  for(int i = 0; i < thePolygon->getNumInteriorRing(); ++i)
	    {
	      geometrySize += sizeof(int); //number of points in ring
	      theRing = thePolygon->getInteriorRingN(i);
	      if(theRing)
		{
		  geometrySize += theRing->getNumPoints()*2*sizeof(double);
		}
	    }
    
	    mGeometry = new unsigned char[geometrySize];

	    //then fill the geometry itself into the wkb
	    int position = 0;
	    // assign byteOrder
	    memcpy(mGeometry, &byteOrder, 1);
	    position += 1;
	    //endian flag must be added by QgsVectorLayer
	    int wkbtype=QGis::WKBPolygon;
	    memcpy(&mGeometry[position],&wkbtype, sizeof(int));
	    position += sizeof(int);
	    int nRings = thePolygon->getNumInteriorRing()+1;
	    memcpy(&mGeometry[position], &nRings, sizeof(int));
	    position += sizeof(int);

	    //exterior ring first
	    theRing = thePolygon->getExteriorRing();
	    if(theRing)
	      {
		nPointsInRing = theRing->getNumPoints();
		memcpy(&mGeometry[position], &nPointsInRing, sizeof(int));
		position += sizeof(int);
		const geos::CoordinateSequence* ringSequence = theRing->getCoordinatesRO();
		//for(int j = 0; j < nPointsInRing; ++j)
		for(int j = 0; j <ringSequence->getSize(); ++j)
		      {
			//x = theRing->getPointN(j)->getX();
			x = ringSequence->getAt(j).x;
			memcpy(&mGeometry[position], &x, sizeof(double));
			position += sizeof(double);
			//y = theRing->getPointN(j)->getY();
			y = ringSequence->getAt(j).y;
			memcpy(&mGeometry[position], &y, sizeof(double));
			position += sizeof(double);
		      }
	      }

	    //interior rings after
	    for(int i = 0; i < thePolygon->getNumInteriorRing(); ++i)
	      {
		theRing = thePolygon->getInteriorRingN(i);
		if(theRing)
		  {
		    nPointsInRing = theRing->getNumPoints();
		    memcpy(&mGeometry[position], &nPointsInRing, sizeof(int));
		    position += sizeof(int);
		    for(int j = 0; j < nPointsInRing; ++j)
		      {
			x = theRing->getPointN(j)->getX();
			memcpy(&mGeometry[position], &x, sizeof(double));
			position += sizeof(double);
			y = theRing->getPointN(j)->getY();
			memcpy(&mGeometry[position], &y, sizeof(double));
			position += sizeof(double);
		      }
		  }
	      }
	    mDirtyWkb = FALSE;
	    return true;
	}
      else
	{
	  //error
	}
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
                   
bool QgsGeometry::movePolygonVertex(int atVertex, double x, double y)
{
  if(!mGeos)
    {
      return false;
    }

  geos::Polygon* originalpoly = dynamic_cast<geos::Polygon*>(mGeos);
  if(!originalpoly)
    {
      return false;
    }
  
  geos::CoordinateSequence* coordinates = originalpoly->getCoordinates();
  std::vector<int> rings(originalpoly->getNumInteriorRing() + 1); //a vector storing the number of points in each ring
  const geos::LineString* outerRing = originalpoly->getExteriorRing();
  int pointcounter = 0;

  if(atVertex == 0 || atVertex == outerRing->getNumPoints()-1)
    {
      coordinates->setAt(geos::Coordinate(x, y), 0);
      coordinates->setAt(geos::Coordinate(x, y), outerRing->getNumPoints()-1);
    }
  else if(atVertex < outerRing->getNumPoints())
    {
      coordinates->setAt(geos::Coordinate(x, y), atVertex);
    }
  rings[0] = outerRing->getNumPoints();
  pointcounter += outerRing->getNumPoints();

  for(int i = 0; i < originalpoly->getNumInteriorRing(); ++i)
    {
      const geos::LineString* innerRing = originalpoly->getInteriorRingN(i);
      if(atVertex == pointcounter || atVertex== pointcounter + innerRing->getNumPoints()-1)
	{
	  coordinates->setAt(geos::Coordinate(x, y), pointcounter);
	  coordinates->setAt(geos::Coordinate(x, y), pointcounter + innerRing->getNumPoints()-1);
	}
      else if(atVertex > pointcounter && atVertex < pointcounter + innerRing->getNumPoints()-1)
	{
	  coordinates->setAt(geos::Coordinate(x, y), atVertex);
	}
      rings[i+1] = innerRing->getNumPoints();
      pointcounter += innerRing->getNumPoints();
    }
  
  geos::Polygon* newPolygon = createPolygonFromCoordSequence(coordinates, rings);
  delete coordinates;
  if(newPolygon)
    {
      delete mGeos;
      mGeos = newPolygon;
      return true;
    }
  else
    {
      return false;
    }
}

bool QgsGeometry::deleteVertexFromPolygon(int atVertex)
{
  if(!mGeos)
    {
      return false;
    }

  geos::Polygon* originalpoly = dynamic_cast<geos::Polygon*>(mGeos);
  if(!originalpoly)
    {
      return false;
    }
  
  geos::CoordinateSequence* coordinates = originalpoly->getCoordinates();
  std::vector<int> rings(originalpoly->getNumInteriorRing() + 1); //a vector storing the number of points in each ring
  const geos::LineString* outerRing = originalpoly->getExteriorRing();
  int pointcounter = 0;

  if(atVertex == 0 || atVertex == outerRing->getNumPoints()-1)
    {
      coordinates->setAt(coordinates->getAt(1), outerRing->getNumPoints()-1);
      coordinates->deleteAt(0);
      rings[0] = outerRing->getNumPoints()-1;
      pointcounter += outerRing->getNumPoints()-1;
    }
  else if(atVertex < outerRing->getNumPoints())
    {
      coordinates->deleteAt(atVertex);
      rings[0] = outerRing->getNumPoints()-1;
      pointcounter += outerRing->getNumPoints()-1;
    }
  else
    {
      rings[0] = outerRing->getNumPoints();
      pointcounter += outerRing->getNumPoints();
    }

  for(int i = 0; i < originalpoly->getNumInteriorRing(); ++i)
    {
      const geos::LineString* innerRing = originalpoly->getInteriorRingN(i);
      if(atVertex == pointcounter || atVertex== pointcounter + innerRing->getNumPoints()-1)
	{
	  coordinates->setAt(coordinates->getAt(pointcounter+1), pointcounter + innerRing->getNumPoints()-1);
	  coordinates->deleteAt(pointcounter);
	  pointcounter += innerRing->getNumPoints()-1;
	  rings[i+1] = innerRing->getNumPoints()-1;
	}
      else if(atVertex > pointcounter && atVertex < pointcounter + innerRing->getNumPoints()-1)
	{
	  coordinates->deleteAt(atVertex);
	  rings[i+1] = innerRing->getNumPoints()-1;
	  pointcounter += innerRing->getNumPoints()-1;
	}
      else
	{
	  rings[i+1] = innerRing->getNumPoints();
	  pointcounter += innerRing->getNumPoints();
	}
    }
  
  geos::Polygon* newPolygon = createPolygonFromCoordSequence(coordinates, rings);
  delete coordinates;
  if(newPolygon)
    {
      delete mGeos;
      mGeos = newPolygon;
      return true;
    }
  else
    {
      return false;
    }
}

bool QgsGeometry::insertVertexToPolygon(int beforeVertex, double x, double y)
{
  if(!mGeos)
    {
      return false;
    }

  geos::Polygon* originalpoly = dynamic_cast<geos::Polygon*>(mGeos);
  if(!originalpoly)
    {
      return false;
    }
  
  geos::CoordinateSequence* coordinates = originalpoly->getCoordinates();
  //the sequence where the point will be inserted
  geos::CoordinateSequence* newSequence = new geos::DefaultCoordinateSequence();
  std::vector<int> rings(originalpoly->getNumInteriorRing() + 1); //a vector storing the number of points in each ring
  const geos::LineString* outerRing = originalpoly->getExteriorRing();
  
  int coordinateCounter = 0;
  int numRingPoints = 0;
  for(int i = 0; i < outerRing->getNumPoints(); ++i)
    {
      newSequence->add(coordinates->getAt(coordinateCounter), true);
      ++coordinateCounter;
      ++numRingPoints;
      if(coordinateCounter == beforeVertex)
	{
	  ++numRingPoints;
	  newSequence->add(geos::Coordinate(x, y), true);
	}
    }
  rings[0] = numRingPoints;

  for(int i = 0; i < originalpoly->getNumInteriorRing(); ++i)
    {
      numRingPoints = 0;
      const geos::LineString* innerRing = originalpoly->getInteriorRingN(i);
      for(int j = 0; j < originalpoly->getNumInteriorRing(); ++j)
	{
	  newSequence->add(coordinates->getAt(coordinateCounter), true);
	  ++numRingPoints;
	  ++coordinateCounter;
	  if(coordinateCounter == beforeVertex)
	    {
	      ++numRingPoints;
	      newSequence->add(geos::Coordinate(x, y), true);
	    }
	}
      rings[i+1] = numRingPoints;
    }
  geos::Polygon* newPolygon = createPolygonFromCoordSequence(newSequence, rings);
  delete coordinates;
  delete newSequence;
  if(newPolygon)
    {
      delete mGeos;
      mGeos = newPolygon;
      return true;
    }
  else
    {
      return false;
    }
}

geos::Polygon* QgsGeometry::createPolygonFromCoordSequence(const geos::CoordinateSequence* coords, const std::vector<int>& pointsInRings) const
{
  
  geos::CoordinateSequence* outerRingSequence = new geos::DefaultCoordinateSequence();
  int pointcounter = 0;
  for(int i = 0; i < pointsInRings[0]; ++i)
    {
      outerRingSequence->add(geos::Coordinate(coords->getAt(pointcounter)));
      ++pointcounter;
    }
  geos::LinearRing* newOuterRing = 0;
  try
    {
      newOuterRing = geosGeometryFactory->createLinearRing(outerRingSequence);
    }
  catch(geos::IllegalArgumentException* e)
    {
      return 0;
    }

  std::vector<geos::Geometry*>* newInnerRings = new std::vector<geos::Geometry*>(pointsInRings.size()-1);
  
  for(int i = 0; i < pointsInRings.size()-1; ++i)
    {
      geos::CoordinateSequence* innerRingSequence = new geos::DefaultCoordinateSequence();
      for(int j = 0; j < pointsInRings[i+1]; ++j)
	{
	  innerRingSequence->add(geos::Coordinate(coords->getAt(pointcounter)));
	  ++pointcounter;
	}
      geos::LinearRing* newInnerRing = 0;
      try
	{
	  newInnerRing = geosGeometryFactory->createLinearRing(innerRingSequence);
	}
      catch(geos::IllegalArgumentException* e)
	{
	  return 0;
	}
      (*newInnerRings)[i] = newInnerRing;
    }
  
  geos::Polygon* newPolygon = 0;
  try
    {
      newPolygon = geosGeometryFactory->createPolygon(newOuterRing, newInnerRings);
    }
  catch(geos::IllegalArgumentException* e)
    {
      return 0;
    }

  return newPolygon;
}
