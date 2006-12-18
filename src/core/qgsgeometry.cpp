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
#include "qgslogger.h"


// Set up static GEOS geometry factory
GEOS_GEOM::GeometryFactory* QgsGeometry::geosGeometryFactory = new GEOS_GEOM::GeometryFactory();


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

void QgsGeometry::setGeos(GEOS_GEOM::Geometry* geos)
{
  // TODO - make this more heap-friendly

  if (mGeos)
  {
    delete mGeos;
    mGeos = 0;
  }

  mGeos = geos;

#ifdef QGISDEBUG
  //std::cout << "QgsGeometry::setGeos: setting Geos = '" << mGeos->toString() << "'." << std::endl;
#endif


  mDirtyWkb   = TRUE;
  mDirtyGeos  = FALSE;
  mDirtyWkt   = TRUE;

}

QgsPoint QgsGeometry::closestVertex(const QgsPoint& point, QgsGeometryVertexIndex& atVertex, int& beforeVertex, int& afterVertex, double& sqrDist) const
{
  if(mDirtyWkb)
    {
      exportGeosToWkb();
    }

    if(mGeometry)
    {
      int vertexnr;
      int vertexcounter = 0;
	int wkbType;
	double actdist = std::numeric_limits<double>::max();
	double x,y;
	double *tempx,*tempy;
	memcpy(&wkbType, (mGeometry+1), sizeof(int));
	beforeVertex = -1;
	afterVertex = -1;

	switch (wkbType)
	{
	    case QGis::WKBPoint:
		x = *((double *) (mGeometry + 5));
		y = *((double *) (mGeometry + 5 + sizeof(double)));
		actdist = point.sqrDist(x, y);
		vertexnr = 0;
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
			vertexnr = index;
			if(index == 0)//assign the rubber band indices
			  {
			    beforeVertex = -1;
			  }
			else
			  {
			    beforeVertex = index-1;
			  }
			if(index == (*npoints - 1))
			  {
			    afterVertex = -1;
			  }
			else
			  {
			    afterVertex = index+1;
			  }
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
			    vertexnr = vertexcounter;
			    //assign the rubber band indices
			    if(index2 == 0)
			      {
				beforeVertex = vertexcounter+(*npoints-2);
				afterVertex = vertexcounter+1;
			      }
			    else if(index2 == (*npoints-1))
			      {
				beforeVertex = vertexcounter-1;
				afterVertex = vertexcounter - (*npoints-2);
			      }
			    else
			      {
				beforeVertex = vertexcounter-1;
				afterVertex = vertexcounter+1;
			      }
			}
			ptr+=sizeof(double);
			++vertexcounter;
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
		  ptr += (1+sizeof(int)); //skip endian and point type
		    tempx=(double*)ptr;
		    tempy=(double*)(ptr+sizeof(double));
		    if(point.sqrDist(*tempx,*tempy)<actdist)
			{
			    x=*tempx;
			    y=*tempy;
			    actdist=point.sqrDist(*tempx,*tempy);
			    vertexnr = index;
			}
		    ptr+=(2*sizeof(double));
		}
	    }
		break;

	    case QGis::WKBMultiLineString:
	    {
		unsigned char* ptr=mGeometry+5;
		int* nlines=(int*)ptr;
		int* npoints = 0;
		ptr+=sizeof(int);
		for(int index=0;index<*nlines;++index)
		{
		  ptr += (sizeof(int) + 1);
		  npoints=(int*)ptr;
		  ptr+=sizeof(int);
		  for(int index2=0;index2<*npoints;++index2)
		    {
		      tempx=(double*)ptr;
		      ptr+=sizeof(double);
		      tempy=(double*)ptr;
		      ptr+=sizeof(double);
		      if(point.sqrDist(*tempx,*tempy)<actdist)
			{
			  x=*tempx;
			  y=*tempy;
			  actdist=point.sqrDist(*tempx,*tempy);
			  vertexnr = vertexcounter;

			  if(index2 == 0)//assign the rubber band indices
			    {
			      beforeVertex = -1;
			    }
			  else
			    {
			      beforeVertex = vertexnr-1;
			    }
			  if(index2 == (*npoints)-1)
			    {
			      afterVertex = -1;
			    }
			  else
			    {
			      afterVertex = vertexnr+1;
			    }
			}
		      ++vertexcounter;
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
		  ptr += (1 + sizeof(int)); //skip endian and polygon type
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
			       vertexnr = vertexcounter;
			       
			       //assign the rubber band indices
			       if(index3 == 0)
				 {
				   beforeVertex = vertexcounter+(*npoints-2);
				   afterVertex = vertexcounter+1;
				 }
			       else if(index3 == (*npoints-1))
				 {
				   beforeVertex = vertexcounter-1;
				   afterVertex = vertexcounter - (*npoints-2);
				 }
			       else
				 {
				   beforeVertex = vertexcounter-1;
				   afterVertex = vertexcounter+1;
				 }
			   }
			   ptr+=sizeof(double); 
			   ++vertexcounter;
			}
		    }
		}
	    }
		break;

	    default:
		break;
	}
	sqrDist = actdist;
	atVertex.clear();
	atVertex.push_back(vertexnr);
	return QgsPoint(x,y);    
    }
    return QgsPoint(0,0);
}


void QgsGeometry::adjacentVerticies(const QgsGeometryVertexIndex& atVertex, int& beforeVertex, int& afterVertex) const
{
  if(mDirtyWkb)
  {
    exportGeosToWkb();
  }

  beforeVertex = -1;
  afterVertex = -1;

  if (mGeometry)
  {
    int vertexcounter = 0;

    int wkbType;

    memcpy(&wkbType, (mGeometry+1), sizeof(int));

    switch (wkbType)
    {
      case QGis::WKBPoint:
      {
        // NOOP - Points do not have adjacent verticies
        break;
      }

      case QGis::WKBLineString:
      {
        unsigned char* ptr = mGeometry+5;
        int* npoints = (int*) ptr;

        const int index = atVertex.back();

        // assign the rubber band indices

        if(index == 0)
        {
          beforeVertex = -1;
        }
        else
        {
          beforeVertex = index-1;
        }

        if(index == (*npoints - 1))
        {
          afterVertex = -1;
        }
        else
        {
          afterVertex = index+1;
        }

        break;
      }
      case QGis::WKBPolygon:
      {
        int* nrings=(int*)(mGeometry+5);
        int* npoints;
        unsigned char* ptr=mGeometry+9;

        // Walk through the POLYGON WKB

        for (int index0 = 0; index0 < *nrings; ++index0)
        {
          npoints=(int*)ptr;
          ptr+=sizeof(int);

          for (int index1 = 0; index1 < *npoints; ++index1)
          {
            ptr += sizeof(double);
            ptr += sizeof(double);

            if (vertexcounter == atVertex.back())
            // TODO: The above is deprecated as it doesn't allow for multiple linear rings in the polygon.
            // replace with the below when the rest of QgsGeometry is migrated to a GEOS back-end.
            //if (
            //    (index0 == atVertex.get_at(0)) &&
            //    (index1 == atVertex.get_at(1))
            //   )
            {
              // Found the vertex of the linear-ring we were looking for.

              // assign the rubber band indices

              if(index1 == 0)
              {
                beforeVertex = vertexcounter+(*npoints-2);
                afterVertex = vertexcounter+1;
              }
              else if(index1 == (*npoints-1))
              {
                beforeVertex = vertexcounter-1;
                afterVertex = vertexcounter - (*npoints-2);
              }
              else
              {
                beforeVertex = vertexcounter-1;
                afterVertex = vertexcounter+1;
              }
            }

            ++vertexcounter;
          }
        }
        break;
      }

      case QGis::WKBMultiPoint:
      {
        // NOOP - Points do not have adjacent verticies
        break;
      }

      case QGis::WKBMultiLineString:
      {
        unsigned char* ptr=mGeometry+5;
        int* nlines=(int*)ptr;
        int* npoints = 0;
        ptr+=sizeof(int);

        for (int index0 = 0; index0 < *nlines; ++index0)
        {
          ptr += (sizeof(int) + 1);
          npoints = (int*)ptr;
          ptr += sizeof(int);

          for (int index1 = 0; index1 < *npoints; ++index1)
          {
            ptr+=sizeof(double);
            ptr+=sizeof(double);

            if (vertexcounter == atVertex.back())
            // TODO: The above is deprecated as it doesn't allow for account for all ends of lines.
            // replace with the below when the rest of QgsGeometry is migrated to a GEOS back-end.
            //if (
            //    (index0 == atVertex.get_at(0)) &&
            //    (index1 == atVertex.get_at(1))
            //   )
            {
              // Found the vertex of the linestring we were looking for.

              // assign the rubber band indices

              if(index1 == 0)
                {
                  beforeVertex = -1;
                }
              else
                {
                  beforeVertex = vertexcounter-1;
                }
              if(index1 == (*npoints)-1)
                {
                  afterVertex = -1;
                }
              else
                {
                  afterVertex = vertexcounter+1;
                }
              }
            ++vertexcounter;
          }
        }

        break;
      }

      case QGis::WKBMultiPolygon:
      {
        unsigned char* ptr=mGeometry+5;
        int* npolys=(int*)ptr;
        int* nrings;
        int* npoints;
        ptr+=sizeof(int);

        for (int index0 = 0; index0 < *npolys; ++index0)
        {
          ptr += (1 + sizeof(int)); //skip endian and polygon type
          nrings=(int*)ptr;
          ptr+=sizeof(int);

          for (int index1 = 0; index1 < *nrings; ++index1)
          {
            npoints=(int*)ptr;
            ptr+=sizeof(int);

            for (int index2 = 0; index2 < *npoints; ++index2)
            {
              ptr += sizeof(double);
              ptr += sizeof(double);

              if (vertexcounter == atVertex.back())
              // TODO: The above is deprecated as it doesn't allow for multiple linear rings in the polygon.
              // replace with the below when the rest of QgsGeometry is migrated to a GEOS back-end.
              //if (
              //    (index0 == atVertex.get_at(0)) &&
              //    (index1 == atVertex.get_at(1)) &&
              //    (index2 == atVertex.get_at(2))
              //    )
              {
                // Found the vertex of the linear-ring of the polygon we were looking for.

                // assign the rubber band indices

                if(index2 == 0)
                {
                  beforeVertex = vertexcounter+(*npoints-2);
                  afterVertex = vertexcounter+1;
                }
                else if(index2 == (*npoints-1))
                {
                  beforeVertex = vertexcounter-1;
                  afterVertex = vertexcounter - (*npoints-2);
                }
                else
                {
                  beforeVertex = vertexcounter-1;
                  afterVertex = vertexcounter+1;
                }
              }
              ++vertexcounter;
            }
          }
        }

        break;
      }

      default:
        break;
    } // switch (wkbType)

  } // if (mGeometry)
}



bool QgsGeometry::insertVertexBefore(double x, double y,
                                     int beforeVertex,
                                     const GEOS_GEOM::CoordinateSequence*  old_sequence,
                                           GEOS_GEOM::CoordinateSequence** new_sequence)

{
  // Bounds checking
  if (beforeVertex < 0)
  {
    (*new_sequence) = 0;
    return FALSE;
  }

  int numPoints = old_sequence->getSize();

  // Copy to the new sequence, including the new vertex
  (*new_sequence) = new GEOS_GEOM::DefaultCoordinateSequence();

  bool inserted = FALSE;
  for (int i = 0; i < numPoints; i++)
  {
    // Do we insert the new vertex here?
    if (beforeVertex == i)
    {
      (*new_sequence)->add( GEOS_GEOM::Coordinate(x, y) );
      inserted = TRUE;
    }
    (*new_sequence)->add( old_sequence->getAt(i) );
  }

  if (!inserted)
  {
    // The beforeVertex is greater than the actual number of vertices
    // in the geometry - append it.
    (*new_sequence)->add( GEOS_GEOM::Coordinate(x, y) );
  }
// TODO: Check that the sequence is still simple, e.g. with GEOS_GEOM::Geometry->isSimple()

  return inserted;
}

bool QgsGeometry::moveVertexAt(double x, double y, QgsGeometryVertexIndex atVertex)
{
  int vertexnr = atVertex.back();

  if(mDirtyWkb)
    {
      exportGeosToWkb();
    }

  unsigned int wkbType;
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
	    mDirtyGeos = true;
	    return true;
	  }
	else
	  {
	    return false;
	  }
      }
    case QGis::WKBMultiPoint:
      {
	int* nrPoints = (int*)ptr;
	if(vertexnr > *nrPoints || vertexnr < 0)
	  {
	    return false;
	  }
	ptr += sizeof(int);
	ptr += (2*sizeof(double)+1+sizeof(int))*vertexnr;
	ptr += (1+sizeof(int));
	memcpy(ptr, &x, sizeof(double));
	ptr += sizeof(double);
	memcpy(ptr, &y, sizeof(double));
	mDirtyGeos = true;
	return true;
      }
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
	mDirtyGeos = true;
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
	    ptr += sizeof(int) + 1;
	    nrPoints = (int*)ptr;
	    ptr += sizeof(int);
	    if(vertexnr >= pointindex && vertexnr < pointindex + (*nrPoints))
	      {
		ptr += (vertexnr-pointindex)*2*sizeof(double);
		memcpy(ptr, &x, sizeof(double));
		memcpy(ptr+sizeof(double), &y, sizeof(double));
		mDirtyGeos = true;
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
		mDirtyGeos = true;
		return true;
	      }
	    else if(vertexnr > pointindex && vertexnr < pointindex + (*nrPoints-1))//move only one point
	      {
		ptr += 2*sizeof(double)*(vertexnr - pointindex);
		memcpy(ptr, &x, sizeof(double));
		ptr += sizeof(double);
		memcpy(ptr, &y, sizeof(double));
		mDirtyGeos = true;
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
	    ptr += (1 + sizeof(int)); //skip endian and polygon type
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
		    mDirtyGeos = true;
		    return true;
		  }
		else if(vertexnr > pointindex && vertexnr < pointindex + (*nrPoints-1))//move only one point
		  {
		    ptr += 2*sizeof(double)*(vertexnr - pointindex);
		    memcpy(ptr, &x, sizeof(double));
		    ptr += sizeof(double);
		    memcpy(ptr, &y, sizeof(double));
		    mDirtyGeos = true;
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

bool QgsGeometry::deleteVertexAt(QgsGeometryVertexIndex atVertex)
{
  int vertexnr = atVertex.back();
  bool success = false;

  if(mDirtyWkb)
    {
      exportGeosToWkb();
    }

  //create a new geometry buffer for the modified geometry
  unsigned char* newbuffer = new unsigned char[mGeometrySize-2*sizeof(double)];
  memcpy(newbuffer, mGeometry, 1+sizeof(int)); //endian and type are the same

  int pointindex = 0;
  unsigned int wkbType;
  unsigned char* ptr = mGeometry+1;
  memcpy(&wkbType, ptr, sizeof(wkbType));
  ptr += sizeof(wkbType);
  unsigned char* newBufferPtr = newbuffer+1+sizeof(int);
  
  switch(wkbType)
    {
    case QGis::WKBPoint:
      {
	break; //cannot remove the only point vertex
      }
    case QGis::WKBMultiPoint:
      {
	//todo
      }
    case QGis::WKBLineString:
      {
	int* nPoints = (int*)ptr;
	if((*nPoints) < 3 || vertexnr > (*nPoints)-1 || vertexnr < 0)
	  {
	    break;
	  }
	int newNPoints = (*nPoints)-1; //new number of points
	memcpy(newBufferPtr, &newNPoints, sizeof(int));
	ptr += sizeof(int);
	newBufferPtr += sizeof(int);
	for(int pointnr = 0; pointnr < *nPoints; ++pointnr)
	  {
	    if(vertexnr != pointindex)
	      {
		memcpy(newBufferPtr, ptr, sizeof(double));
		memcpy(newBufferPtr+sizeof(double), ptr+sizeof(double), sizeof(double));
		newBufferPtr += 2*sizeof(double);
	      }
	    else
	      {
		success = true;
	      }
	    ptr += 2*sizeof(double);
	    ++pointindex;
	  }
	break;
      }
    case QGis::WKBMultiLineString:
      {
	int* nLines = (int*)ptr;
	memcpy(newBufferPtr, nLines, sizeof(int));
	newBufferPtr += sizeof(int);
	ptr += sizeof(int);
	int* nPoints = 0; //number of points in a line
	int pointindex = 0;
	for(int linenr = 0; linenr < *nLines; ++linenr)
	  {
	    memcpy(newBufferPtr, ptr, sizeof(int) + 1);
	    ptr += (sizeof(int) + 1);
	    newBufferPtr += (sizeof(int) + 1);
	    nPoints = (int*)ptr;
	    ptr += sizeof(int);
	    int newNPoint;

	    //find out if the vertex is in this line
	    if(vertexnr >= pointindex && vertexnr < pointindex + (*nPoints))
	      {
		if(*nPoints < 3)
		  {
		    break;
		  }
		newNPoint = (*nPoints)-1;
	      }
	    else
	      {
		newNPoint = *nPoints;
	      }
	    memcpy(newBufferPtr, &newNPoint, sizeof(int));
	    newBufferPtr += sizeof(int);

	    for(int pointnr = 0; pointnr < *nPoints; ++pointnr)
	      {
		if(vertexnr != pointindex)
		  {
		    memcpy(newBufferPtr, ptr, sizeof(double));//x
		    memcpy(newBufferPtr+sizeof(double), ptr+sizeof(double), sizeof(double));//y
		    newBufferPtr += 2*sizeof(double);
		  }
		else
		  {
		    success = true;
		  }
		ptr += 2*sizeof(double);
		++pointindex;
	      }
	  }
	break;
      }
    case QGis::WKBPolygon:
      {
	int* nRings = (int*)ptr;
	memcpy(newBufferPtr, nRings, sizeof(int));
	ptr += sizeof(int);
	newBufferPtr += sizeof(int);
	int* nPoints = 0; //number of points in a ring
	int pointindex = 0;
      
	for(int ringnr = 0; ringnr < *nRings; ++ringnr)
	  {
	    nPoints = (int*)ptr;
	    ptr += sizeof(int);
	    int newNPoints;
	    if(vertexnr >= pointindex && vertexnr < pointindex + *nPoints)//vertex to delete is in this ring
	      {
		if(*nPoints < 5) //a ring has at least 3 points
		  {
		    break;
		  }
		newNPoints = *nPoints - 1;
	      }
	    else
	      {
		newNPoints = *nPoints;
	      }
	    memcpy(newBufferPtr, &newNPoints, sizeof(int));
	    newBufferPtr += sizeof(int);
	    
	    for(int pointnr = 0; pointnr < *nPoints; ++pointnr)
	      {
		if(vertexnr != pointindex)
		  {
		    memcpy(newBufferPtr, ptr, sizeof(double));//x
		    memcpy(newBufferPtr+sizeof(double), ptr+sizeof(double), sizeof(double));//y
		    newBufferPtr += 2*sizeof(double);
		  }
		else
		  {
		    if(pointnr == 0)//adjust the last point of the ring
		      {
			memcpy(ptr+(*nPoints-1)*2*sizeof(double), ptr+2*sizeof(double), sizeof(double));
			memcpy(ptr+sizeof(double)+(*nPoints-1)*2*sizeof(double), ptr+3*sizeof(double), sizeof(double));
		      }
		    if(pointnr == (*nPoints)-1)
		      {
			memcpy(newBufferPtr-(*nPoints-2)*2*sizeof(double), ptr-2*sizeof(double), sizeof(double));
			memcpy(newBufferPtr-(*nPoints-2)*2*sizeof(double)+sizeof(double), ptr-sizeof(double), sizeof(double));
		      }
		    success = true;
		  }
		ptr += 2*sizeof(double);
		++pointindex;
	      }
	  }
	break;
      }
    case QGis::WKBMultiPolygon:
      {
	int* nPolys = (int*)ptr;
	memcpy(newBufferPtr, nPolys, sizeof(int));
	newBufferPtr += sizeof(int);
	ptr +=sizeof(int);
	int* nRings = 0;
	int* nPoints = 0;
	int pointindex = 0;

	for(int polynr = 0; polynr < *nPolys; ++polynr)
	  {
	    memcpy(newBufferPtr, ptr, (1 + sizeof(int))); 
	    ptr += (1 + sizeof(int)); //skip endian and polygon type
	    newBufferPtr += (1 + sizeof(int));
	    nRings = (int*)ptr;
	    memcpy(newBufferPtr, nRings, sizeof(int));
	    newBufferPtr += sizeof(int);
	    ptr += sizeof(int);
	    for(int ringnr = 0; ringnr < *nRings; ++ringnr)
	      {
		nPoints = (int*)ptr;
		ptr += sizeof(int);
		int newNPoints;
		if(vertexnr >= pointindex && vertexnr < pointindex + *nPoints)//vertex to delete is in this ring
		  {
		    if(*nPoints < 5) //a ring has at least 3 points
		      {
			break;
		      }
		    newNPoints = *nPoints - 1;
		  }
		else
		  {
		    newNPoints = *nPoints;
		  }
		memcpy(newBufferPtr, &newNPoints, sizeof(int));
		newBufferPtr += sizeof(int);
	    
		for(int pointnr = 0; pointnr < *nPoints; ++pointnr)
		  {
		    if(vertexnr != pointindex)
		      {
			memcpy(newBufferPtr, ptr, sizeof(double));//x
			memcpy(newBufferPtr+sizeof(double), ptr+sizeof(double), sizeof(double));//y
			newBufferPtr += 2*sizeof(double);
		      }
		    else
		      {
			if(pointnr == 0)//adjust the last point of the ring
			  {
			    memcpy(ptr+(*nPoints-1)*2*sizeof(double), ptr+2*sizeof(double), sizeof(double));
			    memcpy(ptr+sizeof(double)+(*nPoints-1)*2*sizeof(double), ptr+3*sizeof(double), sizeof(double));
			  }
			if(pointnr == (*nPoints)-1)
			  {
			    memcpy(newBufferPtr-(*nPoints-2)*2*sizeof(double), ptr-2*sizeof(double), sizeof(double));
			    memcpy(newBufferPtr-(*nPoints-2)*2*sizeof(double)+sizeof(double), ptr-sizeof(double), sizeof(double));
			  }
			success = true;
		      }
		    ptr += 2*sizeof(double);
		    ++pointindex;
		  }
	      } 
	  }
	break;
      }
    }
  if(success)
    {
      delete mGeometry;
      mGeometry = newbuffer;
      mGeometrySize -= (2*sizeof(double));
      mDirtyGeos = true;
      return true;
    }
  else
    {
      delete[] newbuffer;
      return false;
    }
}

bool QgsGeometry::insertVertexBefore(double x, double y, QgsGeometryVertexIndex beforeVertex)
{
  int vertexnr = beforeVertex.back();
  bool success = false;

  if(mDirtyWkb)
    {
      exportGeosToWkb();
    }

  //create a new geometry buffer for the modified geometry
  unsigned char* newbuffer = new unsigned char[mGeometrySize+2*sizeof(double)];
  memcpy(newbuffer, mGeometry, 1+sizeof(int)); //endian and type are the same
  
  int pointindex = 0;
  unsigned int wkbType;
  unsigned char* ptr = mGeometry+1;
  memcpy(&wkbType, ptr, sizeof(wkbType));
  ptr += sizeof(wkbType);
  unsigned char* newBufferPtr = newbuffer+1+sizeof(int);

  switch(wkbType)
    {
    case QGis::WKBPoint://cannot insert a vertex before another one on point types
      {
	delete newbuffer;
	return false;
      }
    case QGis::WKBMultiPoint:
      {
	//todo
	break;
      }
    case QGis::WKBLineString:
      {
	int* nPoints = (int*)ptr;
	if(vertexnr > *nPoints || vertexnr < 0)
	  {
	    break;
	  }
	int newNPoints = (*nPoints)+1;
	memcpy(newBufferPtr, &newNPoints, sizeof(int));
	newBufferPtr += sizeof(int);
	ptr += sizeof(int);

	for(int pointnr = 0; pointnr < *nPoints; ++pointnr)
	  {
	    memcpy(newBufferPtr, ptr, sizeof(double));//x
	    memcpy(newBufferPtr+sizeof(double), ptr+sizeof(double), sizeof(double));//x
	    ptr += 2*sizeof(double);
	    newBufferPtr += 2*sizeof(double);
	    ++pointindex;
	    if(pointindex == vertexnr)
	      {
		memcpy(newBufferPtr, &x, sizeof(double));
		memcpy(newBufferPtr+sizeof(double), &y, sizeof(double));
		newBufferPtr += 2*sizeof(double);
		success = true;
	      }
	  }
	break;
      }
    case QGis::WKBMultiLineString:
      {
	int* nLines = (int*)ptr;
	int* nPoints = 0; //number of points in a line
	ptr += sizeof(int);
	memcpy(newBufferPtr, nLines, sizeof(int));
	newBufferPtr += sizeof(int);
	int pointindex = 0;

	for(int linenr = 0; linenr < *nLines; ++linenr)
	  {
	    memcpy(newBufferPtr, ptr, sizeof(int) + 1);
	    ptr += (sizeof(int) + 1);
	    newBufferPtr += (sizeof(int) + 1);
	    nPoints = (int*)ptr;
	    int newNPoints;
	    if(vertexnr >= pointindex && vertexnr < (pointindex + (*nPoints)))//point is in this ring
	      {
		newNPoints = (*nPoints)+1;
	      }
	    else
	      {
		newNPoints = *nPoints;
	      }
	    memcpy(newBufferPtr, &newNPoints, sizeof(double));
	    newBufferPtr += sizeof(int);
	    ptr += sizeof(int);

	    for(int pointnr = 0; pointnr < *nPoints; ++pointnr)
	      {
		memcpy(newBufferPtr, ptr, sizeof(double));//x
		memcpy(newBufferPtr+sizeof(double), ptr+sizeof(double), sizeof(double));//y
		ptr += 2*sizeof(double);
		newBufferPtr += 2*sizeof(double);
		++pointindex;
		if(pointindex == vertexnr)
		  {
		    memcpy(newBufferPtr, &x, sizeof(double));
		    memcpy(newBufferPtr+sizeof(double), &y, sizeof(double));
		    newBufferPtr += 2*sizeof(double);
		    success = true;
		  }
	      }
	  }
	break;
      }
    case QGis::WKBPolygon:
      {
	int* nRings = (int*)ptr;
	int* nPoints = 0; //number of points in a ring
	ptr += sizeof(int);
	memcpy(newBufferPtr, nRings, sizeof(int));
	newBufferPtr += sizeof(int);
	int pointindex = 0;

	for(int ringnr = 0; ringnr < *nRings; ++ringnr)
	  {
	    nPoints = (int*)ptr;
	    int newNPoints;
	    if(vertexnr >= pointindex && vertexnr < (pointindex + (*nPoints)))//point is in this ring
	      {
		newNPoints = (*nPoints)+1;
	      }
	    else
	      {
		newNPoints = *nPoints;
	      }
	    memcpy(newBufferPtr, &newNPoints, sizeof(double));
	    newBufferPtr += sizeof(int);
	    ptr += sizeof(int);

	    for(int pointnr = 0; pointnr < *nPoints; ++pointnr)
	      {
		memcpy(newBufferPtr, ptr, sizeof(double));//x
		memcpy(newBufferPtr+sizeof(double), ptr+sizeof(double), sizeof(double));//y
		ptr += 2*sizeof(double);
		newBufferPtr += 2*sizeof(double);
		++pointindex;
		if(pointindex == vertexnr)
		  {
		    memcpy(newBufferPtr, &x, sizeof(double));
		    memcpy(newBufferPtr+sizeof(double), &y, sizeof(double));
		    newBufferPtr += 2*sizeof(double);
		    success = true;
		  }
	      }
	  }
	break;
      }
    case QGis::WKBMultiPolygon:
      {
	int* nPolys = (int*)ptr;
	int* nRings = 0; //number of rings in a polygon
	int* nPoints = 0; //number of points in a ring
	memcpy(newBufferPtr, nPolys, sizeof(int));
	ptr += sizeof(int);
	newBufferPtr += sizeof(int);
	int pointindex = 0;

	for(int polynr = 0; polynr < *nPolys; ++polynr)
	  {
	    memcpy(newBufferPtr, ptr, (1 + sizeof(int)));
	    ptr += (1 + sizeof(int));//skip endian and polygon type
	    newBufferPtr += (1 + sizeof(int));
	    nRings = (int*)ptr;
	    ptr += sizeof(int);
	    memcpy(newBufferPtr, nRings, sizeof(int));
	    newBufferPtr += sizeof(int);

	    for(int ringnr = 0; ringnr < *nRings; ++ringnr)
	      {
		nPoints = (int*)ptr;
		int newNPoints;
		if(vertexnr >= pointindex && vertexnr < (pointindex + (*nPoints)))//point is in this ring
		  {
		    newNPoints = (*nPoints)+1;
		  }
		else
		  {
		    newNPoints = *nPoints;
		  }
		memcpy(newBufferPtr, &newNPoints, sizeof(double));
		newBufferPtr += sizeof(int);
		ptr += sizeof(int);
		
		for(int pointnr = 0; pointnr < *nPoints; ++pointnr)
		  {
		    memcpy(newBufferPtr, ptr, sizeof(double));//x
		    memcpy(newBufferPtr+sizeof(double), ptr+sizeof(double), sizeof(double));//y
		    ptr += 2*sizeof(double);
		    newBufferPtr += 2*sizeof(double);
		    ++pointindex;
		    if(pointindex == vertexnr)
		      {
			memcpy(newBufferPtr, &x, sizeof(double));
			memcpy(newBufferPtr+sizeof(double), &y, sizeof(double));
			newBufferPtr += 2*sizeof(double);
			success = true;
		      }
		  }
	      }
	    
	  }
	break;
      }
    }
  
  if(success)
    {
      delete mGeometry;
      mGeometry = newbuffer;
      mGeometrySize += 2*sizeof(double);
      mDirtyGeos = true;
      return true;
    }
  else
    {
      delete newbuffer;
      return false;
    }
}

bool QgsGeometry::vertexAt(double &x, double &y, 
                           QgsGeometryVertexIndex atVertex) const
{
      if(mGeos)//try to find the vertex from the Geos geometry (it present)
	{
	  GEOS_GEOM::CoordinateSequence* cs = mGeos->getCoordinates();
	  if(cs)
	    {
	      const GEOS_GEOM::Coordinate& coord = cs->getAt(atVertex.back());
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
		    return true;
		  }
		else
		  {
		    return FALSE;
		  }
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
                return true;
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
			    return true;
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
		ptr += atVertex.back()*(2*sizeof(double)+1+sizeof(int));
		ptr += 1+sizeof(int);
		memcpy(&x, ptr, sizeof(double));
		ptr += sizeof(double);
		memcpy(&y, ptr, sizeof(double));
                return true;
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
		    ptr += sizeof(int) + 1;
		    nPoints = (int*)ptr;
		    ptr += sizeof(int);
		    for(int pointnr = 0; pointnr < *nPoints; ++pointnr)
		      {
			if(pointindex == atVertex.back())
			  {
			    memcpy(&x, ptr, sizeof(double));
			    ptr += sizeof(double);
			    memcpy(&y, ptr, sizeof(double));
			    return true;
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
		    ptr += (1 + sizeof(int)); //skip endian and polygon type
		    nRings = (int*)ptr;
		    ptr += sizeof(int);
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
				return true;
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
	}
      else
	{
#ifdef QGISDEBUG
	  qWarning("error: no mGeometry pointer in QgsGeometry::vertexAt");
#endif
	  return false;
	}     
}


double QgsGeometry::sqrDistToVertexAt(QgsPoint& point,
                                      QgsGeometryVertexIndex& atVertex) const
{
  double x;
  double y;

  if (vertexAt(x, y, atVertex))
  {
#ifdef QGISDEBUG
    std::cout << "QgsGeometry::sqrDistToVertexAt: Exiting with distance to " << x << " " << y << "." << std::endl;
#endif
    return point.sqrDist(x, y);
  }
  else
  {
#ifdef QGISDEBUG
    std::cout << "QgsGeometry::sqrDistToVertexAt: Exiting with std::numeric_limits<double>::max()." << std::endl;
#endif
    // probably safest to bail out with a very large number
    return std::numeric_limits<double>::max();
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
    GEOS_GEOM::CoordinateSequence* sequence = mGeos->getCoordinates();
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
	    ptr += sizeof(int) + 1;
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
	    ptr += (1 + sizeof(int));
	    nRings = (int*)ptr;
	    ptr += sizeof(int);
	    for(int ringnr = 0; ringnr < *nRings; ++ringnr)
	      {
		nPoints = (int*)ptr;
		ptr += sizeof(int);
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
      case QGis::WKBMultiPoint:
	{
	  ptr = mGeometry + 1 + sizeof(int);
	  nPoints = (int *) ptr;
	  for (idx = 0; idx < *nPoints; idx++)
	    {
	      ptr += (1+sizeof(int));
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
	}
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

    GEOS_GEOM::GeometryFactory *gf = new GEOS_GEOM::GeometryFactory();
    GEOS_IO::WKTReader *wktReader = new GEOS_IO::WKTReader(gf);
    GEOS_GEOM::Geometry *geosGeom = wktReader->read( qstrdup(wkt()) );

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
    
    GEOS_GEOM::Geometry *geosRect = wktReader->read( qstrdup(rectwkt) );
    try // geos might throw exception on error
    {
      if(geosGeom->intersects(geosRect))
      {
        returnval=true;
      }
    }
    catch (GEOS_UTIL::TopologyException* e)
    {
#if GEOS_VERSION_MAJOR < 3
      QString error = e->toString().c_str();
#else
      QString error = e->what();
#endif
      QgsLogger::warning("GEOS: " + error);
    }
 
    delete geosGeom;
    delete geosRect;
    delete gf;
    delete wktReader;
    return returnval;
}

bool QgsGeometry::fast_intersects(const QgsRect* r) const
{
  bool returnval=false;
  
  //use the geos export of QgsGeometry
  GEOS_GEOM::Geometry *geosGeom = geosGeometry();

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
    GEOS_GEOM::GeometryFactory *gf = new GEOS_GEOM::GeometryFactory();
    GEOS_IO::WKTReader *wktReader = new GEOS_IO::WKTReader(gf);
    GEOS_GEOM::Geometry *geosRect = wktReader->read( qstrdup(rectwkt) );
    
    try // geos might throw exception on error
    {
      if(geosGeom->intersects(geosRect))
      {
        returnval=true;
      }
    }
    catch (GEOS_UTIL::TopologyException* e)
    {
#if GEOS_VERSION_MAJOR < 3
      QString error = e->toString().c_str();
#else
      QString error = e->what();
#endif
      QgsLogger::warning("GEOS: " + error);
    }
      
    delete geosGeom;
    delete geosRect;
    delete gf;
    delete wktReader;
    return returnval;
}


bool QgsGeometry::contains(QgsPoint* p) const
{
    bool returnval = false;

    exportWkbToGeos();

    GEOS_GEOM::Point* geosPoint = geosGeometryFactory->createPoint(GEOS_GEOM::Coordinate(p->x(), p->y()));

    returnval = mGeos->contains(geosPoint);

    delete geosPoint;

    return returnval;
}


bool QgsGeometry::exportToWkt(unsigned char * geom) const
{
  QgsDebugMsg("QgsGeometry::exportToWkt: entered");

    if(geom)
    {
	int wkbType;
	double *x,*y;

	mWkt="";
        // Will this really work when geom[0] == 0 ???? I (gavin) think not.
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
	      QgsDebugMsg("QgsGeometry::exportToWkt: LINESTRING found");
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
	      QgsDebugMsg("QgsGeometry::exportToWkt: POLYGON found");
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
		  ptr += (1+sizeof(int));
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
	      QgsDebugMsg("QgsGeometry::exportToWkt: MULTILINESTRING found");
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
	      QgsDebugMsg("QgsGeometry::exportToWkt: MULTIPOLYGON found");
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
			  if(jdx!=0)
			    {
			      mWkt+=",";
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



GEOS_GEOM::Geometry* QgsGeometry::geosGeometry() const
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
//    GEOS_GEOM::GeometryFactory* geometryFactory = new GEOS_GEOM::GeometryFactory();
    
    wkbtype = (mGeometry[0] == 1) ? mGeometry[1] : mGeometry[4];
    switch(wkbtype)
    {
	case QGis::WKBPoint:
	{
	   x = (double *) (mGeometry + 5);
	   y = (double *) (mGeometry + 5 + sizeof(double));
           
           mDirtyGeos = FALSE;
	   return geosGeometryFactory->createPoint(GEOS_GEOM::Coordinate(*x,*y));
	}
	case QGis::WKBMultiPoint:
	{
	    std::vector<GEOS_GEOM::Geometry*>* points=new std::vector<GEOS_GEOM::Geometry*>;
	    ptr = mGeometry + 5;
	    nPoints = (int *) ptr;
	    ptr = mGeometry + 1 + 2 * sizeof(int);
	    for (idx = 0; idx < *nPoints; idx++)
	    {
	      ptr += (1 + sizeof(int));
		x = (double *) ptr;
		ptr += sizeof(double);
		y = (double *) ptr;
		ptr += sizeof(double);
		points->push_back(geosGeometryFactory->createPoint(GEOS_GEOM::Coordinate(*x,*y)));
	    }
	    return geosGeometryFactory->createMultiPoint(points);
	}
	case QGis::WKBLineString:
	{
#ifdef QGISDEBUG
    qWarning("QgsGeometry::geosGeometry: Linestring found");
#endif
	    GEOS_GEOM::DefaultCoordinateSequence* sequence=new GEOS_GEOM::DefaultCoordinateSequence();
	    ptr = mGeometry + 5;
	    nPoints = (int *) ptr;
	    ptr = mGeometry + 1 + 2 * sizeof(int);
	    for (idx = 0; idx < *nPoints; idx++)
	    {
		x = (double *) ptr;
		ptr += sizeof(double);
		y = (double *) ptr;
		ptr += sizeof(double);
		sequence->add(GEOS_GEOM::Coordinate(*x,*y));
	    }
	    return geosGeometryFactory->createLineString(sequence); 
	}
	case QGis::WKBMultiLineString:
	{
	    std::vector<GEOS_GEOM::Geometry*>* lines=new std::vector<GEOS_GEOM::Geometry*>;
	    numLineStrings = (int) (mGeometry[5]);
	    ptr = mGeometry + 9;
	    for (jdx = 0; jdx < numLineStrings; jdx++)
	    {
		GEOS_GEOM::DefaultCoordinateSequence* sequence=new GEOS_GEOM::DefaultCoordinateSequence();
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
		    sequence->add(GEOS_GEOM::Coordinate(*x,*y));
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
	    
	    GEOS_GEOM::LinearRing* outer=0;
	    std::vector<GEOS_GEOM::Geometry*>* inner=new std::vector<GEOS_GEOM::Geometry*>;

	    for (idx = 0; idx < *numRings; idx++)
	    {
#ifdef QGISDEBUG
    qWarning("Ring nr: "+QString::number(idx));
#endif		
		GEOS_GEOM::DefaultCoordinateSequence* sequence=new GEOS_GEOM::DefaultCoordinateSequence();
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
		    sequence->add(GEOS_GEOM::Coordinate(*x,*y));
		}
		GEOS_GEOM::LinearRing* ring=geosGeometryFactory->createLinearRing(sequence);
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
	    std::vector<GEOS_GEOM::Geometry *> *polygons=new std::vector<GEOS_GEOM::Geometry *>;
	    // get the number of polygons
	    ptr = mGeometry + 5;
	    numPolygons = (int *) ptr;
	    ptr = mGeometry +9;
	    for (kdx = 0; kdx < *numPolygons; kdx++)
	    {
#ifdef QGISDEBUG
		//qWarning("Polygon nr: "+QString::number(kdx));
#endif
		GEOS_GEOM::LinearRing* outer=0;
		std::vector<GEOS_GEOM::Geometry*>* inner=new std::vector<GEOS_GEOM::Geometry*>;

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
		    GEOS_GEOM::DefaultCoordinateSequence* sequence=new GEOS_GEOM::DefaultCoordinateSequence();
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
			sequence->add(GEOS_GEOM::Coordinate(*x,*y));
		    }
		    GEOS_GEOM::LinearRing* ring=geosGeometryFactory->createLinearRing(sequence);
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
    case GEOS_GEOM::GEOS_POINT:                 // a point
    {
      // TODO
      break;
    } // case GEOS_GEOM::GEOS_POINT

    case GEOS_GEOM::GEOS_LINESTRING:            // a linestring
    {
#ifdef QGISDEBUG
  std::cout << "QgsGeometry::exportGeosToWkb: Got a GEOS_GEOM::GEOS_LINESTRING." << std::endl;
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

      GEOS_GEOM::LineString* geosls = static_cast<GEOS_GEOM::LineString*>(mGeos);
      const GEOS_GEOM::CoordinateSequence* sequence = geosls->getCoordinatesRO();

      // assign points
      for (int n = 0; n < numPoints; n++)
      {
#ifdef QGISDEBUG
	//std::cout << "QgsGeometry::exportGeosToWkb: Adding " 
	//  << sequence->getAt(n).x << ", " 
	//  << sequence->getAt(n).y << "." << std::endl;
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
    } // case GEOS_GEOM::GEOS_LINESTRING

    case GEOS_GEOM::GEOS_LINEARRING:            // a linear ring (linestring with 1st point == last point)
    {
      // TODO
      break;
    } // case GEOS_GEOM::GEOS_LINEARRING

    case GEOS_GEOM::GEOS_POLYGON:               // a polygon
    {
      int geometrySize;
      double x, y; //point coordinates
      GEOS_GEOM::Polygon* thePolygon = dynamic_cast<GEOS_GEOM::Polygon*>(mGeos);
      const GEOS_GEOM::LineString* theRing = 0;
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
		const GEOS_GEOM::CoordinateSequence* ringSequence = theRing->getCoordinatesRO();
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
    } // case GEOS_GEOM::GEOS_POLYGON

    case GEOS_GEOM::GEOS_MULTIPOINT:            // a collection of points
    {
      // TODO
      break;
    } // case GEOS_GEOM::GEOS_MULTIPOINT

    case GEOS_GEOM::GEOS_MULTILINESTRING:       // a collection of linestrings
    {
      // TODO
      break;
    } // case GEOS_GEOM::GEOS_MULTILINESTRING

    case GEOS_GEOM::GEOS_MULTIPOLYGON:          // a collection of polygons
    {
      // TODO
      break;
    } // case GEOS_GEOM::GEOS_MULTIPOLYGON

    case GEOS_GEOM::GEOS_GEOMETRYCOLLECTION:    // a collection of heterogeneus geometries
    {
      // TODO
      break;
    } // case GEOS_GEOM::GEOS_GEOMETRYCOLLECTION

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
  

