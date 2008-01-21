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
#include "qgsapplication.h"
#include "qgslogger.h"
#include "qgspoint.h"
#include "qgsrect.h"
//#include "geos/operation/polygonize/Polygonizer.h"
//todo: adapt to geos3
#include "geos/opPolygonize.h"
#include "geos/opLinemerge.h"

// Set up static GEOS geometry factory
static GEOS_GEOM::GeometryFactory* geosGeometryFactory = new GEOS_GEOM::GeometryFactory();


QgsGeometry::QgsGeometry()
  : mGeometry(0),
  mGeometrySize(0),
  mGeos(0),

  mDirtyWkb(FALSE),
mDirtyGeos(FALSE)
{
  // NOOP
}    


QgsGeometry::QgsGeometry( QgsGeometry const & rhs )
  : mGeometry(0),
  mGeometrySize( rhs.mGeometrySize ),

  mDirtyWkb( rhs.mDirtyWkb ),
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
    if(rhs.mGeos->getGeometryTypeId() == GEOS_GEOM::GEOS_MULTIPOLYGON)//MH:problems with cloning for multipolygons in geos 2
      {
	GEOS_GEOM::MultiPolygon* multiPoly = dynamic_cast<GEOS_GEOM::MultiPolygon*>(rhs.mGeos);
	if(multiPoly)
	  {
	    std::vector<GEOS_GEOM::Geometry*> polygonVector;
	    for(GEOS_SIZE_T i = 0; i < multiPoly->getNumGeometries(); ++i)
	      {
		polygonVector.push_back((GEOS_GEOM::Geometry*)(multiPoly->getGeometryN(i)));
	      }
	    mGeos = geosGeometryFactory->createMultiPolygon(polygonVector);
	  }
      }
    else if(rhs.mGeos->getGeometryTypeId() == GEOS_GEOM::GEOS_MULTILINESTRING) //MH: and also for cloning multilines
      {
	GEOS_GEOM::MultiLineString* multiLine = dynamic_cast<GEOS_GEOM::MultiLineString*>(rhs.mGeos);
	if(multiLine)
	  {
	    std::vector<GEOS_GEOM::Geometry*> lineVector;
	    for(GEOS_SIZE_T i = 0; i < multiLine->getNumGeometries(); ++i)
	      {
		lineVector.push_back((GEOS_GEOM::Geometry*)(multiLine->getGeometryN(i)));
	      }
	    mGeos = geosGeometryFactory->createMultiLineString(lineVector);
	  }
      }
    else
      {
	mGeos = rhs.mGeos->clone();
      }
  }
  else
  {
    mGeos = 0;
  }    
}

QgsGeometry* QgsGeometry::fromWkt(QString wkt)
{
  GEOS_IO::WKTReader reader(geosGeometryFactory);
  GEOS_GEOM::Geometry* geom = reader.read(wkt.toLocal8Bit().data());
  QgsGeometry* g = new QgsGeometry;
  g->setGeos(geom);
  return g;
}

QgsGeometry* QgsGeometry::fromPoint(const QgsPoint& point)
{
  GEOS_GEOM::Coordinate coord = GEOS_GEOM::Coordinate(point.x(), point.y());
  GEOS_GEOM::Geometry* geom = 0;
  try
    {
      geom = geosGeometryFactory->createPoint(coord);
    }
  catch(GEOS_UTIL::GEOSException* e)
    {
      delete e; return 0;
    }
  QgsGeometry* g = new QgsGeometry;
  g->setGeos(geom);
  return g;
}

QgsGeometry* QgsGeometry::fromPolyline(const QgsPolyline& polyline)
{
  const GEOS_GEOM::CoordinateSequenceFactory* seqFactory = GEOS_GEOM::COORD_SEQ_FACTORY::instance();
  GEOS_GEOM::CoordinateSequence* seq = seqFactory->create(polyline.count(), 2);

  QgsPolyline::const_iterator it;
  int i = 0;
  for (it = polyline.begin(); it != polyline.end(); ++it)
  {
    seq->setAt(GEOS_GEOM::Coordinate(it->x(), it->y()), i++);
  }

  // new geometry takes ownership of the sequence
  GEOS_GEOM::Geometry* geom = 0;
  try
    {
      geom = geosGeometryFactory->createLineString(seq);
    }
  catch(GEOS_UTIL::GEOSException* e)
    {
      delete e; delete seq;
      return 0;
    }
  QgsGeometry* g = new QgsGeometry;
  g->setGeos(geom);
  return g;
}

QgsGeometry* QgsGeometry::fromMultiPolyline(const QgsMultiPolyline& multiline)
{
  const GEOS_GEOM::CoordinateSequenceFactory* seqFactory = GEOS_GEOM::COORD_SEQ_FACTORY::instance();
  std::vector<GEOS_GEOM::Geometry*>* lineVector = new std::vector<GEOS_GEOM::Geometry*>(multiline.count());
  GEOS_GEOM::LineString* currentLineString = 0;
  
  for(int i = 0; i < multiline.count(); ++i)
    {
      QgsPolyline currentLine = multiline.at(i);
      GEOS_GEOM::CoordinateSequence* seq = seqFactory->create(currentLine.count(), 2);
      for(int j = 0; j < currentLine.count(); ++j)
	{
	  seq->setAt(GEOS_GEOM::Coordinate(currentLine.at(j).x(), currentLine.at(j).y()), j);
	}
      try
	{
	  currentLineString = geosGeometryFactory->createLineString(seq);
	}
      catch(GEOS_UTIL::GEOSException* e)
	{
	  delete lineVector; delete seq; delete e;
	  return 0;
	}
      (*lineVector)[i] = currentLineString;
    }
  
  GEOS_GEOM::Geometry* geom = 0;
  try
    {
      geom = geosGeometryFactory->createMultiLineString(lineVector);
    }
  catch(GEOS_UTIL::GEOSException* e)
    {
      delete e; return 0;
    }
  QgsGeometry* g = new QgsGeometry;
  g->setGeos(geom);
  return g; 
}

static GEOS_GEOM::LinearRing* _createGeosLinearRing(const QgsPolyline& ring)
{
  // LinearRing in GEOS must have the first point the same as the last one
  bool needRepeatLastPnt = (ring[0] != ring[ring.count()-1]);

  const GEOS_GEOM::CoordinateSequenceFactory* seqFactory = GEOS_GEOM::COORD_SEQ_FACTORY::instance();

  GEOS_GEOM::CoordinateSequence* seq = seqFactory->create(ring.count() + (needRepeatLastPnt ? 1 : 0), 2);
  QgsPolyline::const_iterator it;
  int i = 0;
  for (it = ring.begin(); it != ring.end(); ++it)
  {
    seq->setAt(GEOS_GEOM::Coordinate(it->x(), it->y()), i++);
  }
  
  // add the first point to close the ring if needed
  if (needRepeatLastPnt)
    seq->setAt(GEOS_GEOM::Coordinate(ring[0].x(), ring[0].y()), ring.count());
  
  // ring takes ownership of the sequence
  GEOS_GEOM::LinearRing* linRing = 0;
  try
    {
      linRing = geosGeometryFactory->createLinearRing(seq);
    }
  catch(GEOS_UTIL::GEOSException* e)
    {
      delete e; return 0;
    }
  
  return linRing;
}

QgsGeometry* QgsGeometry::fromPolygon(const QgsPolygon& polygon)
{
  if (polygon.count() == 0)
    return NULL;
  
  const QgsPolyline& ring0 = polygon[0];
  
  // outer ring
  GEOS_GEOM::LinearRing* outerRing = _createGeosLinearRing(ring0);
  
  // holes
  std::vector<GEOS_GEOM::Geometry*>* holes = new std::vector<GEOS_GEOM::Geometry*> (polygon.count()-1);
  for (int i = 1; i < polygon.count(); i++)
  {
    (*holes)[i-1] = _createGeosLinearRing(polygon[i]);
  }

  // new geometry takes ownership of outerRing and vector of holes
  GEOS_GEOM::Geometry* geom = 0;
  try
    {
      geom = geosGeometryFactory->createPolygon(outerRing, holes);
    }
  catch(GEOS_UTIL::GEOSException* e)
    {
      delete e; return 0;
    }
  QgsGeometry* g = new QgsGeometry;
  g->setGeos(geom);
  return g;
}

QgsGeometry* QgsGeometry::fromMultiPolygon(const QgsMultiPolygon& multipoly)
{
  if(multipoly.count() == 0)
    {
      return 0;
    }

  std::vector<GEOS_GEOM::Geometry*>* polygons = new std::vector<GEOS_GEOM::Geometry*>(multipoly.count());
  GEOS_GEOM::Polygon* currentPolygon = 0;
  GEOS_GEOM::LinearRing* currentOuterRing = 0;
  std::vector<GEOS_GEOM::Geometry*>* currentHoles = 0;

  for(int i = 0; i < multipoly.count(); ++i)
    {
      currentOuterRing = _createGeosLinearRing(multipoly[i].at(0));
      currentHoles = new std::vector<GEOS_GEOM::Geometry*>(multipoly[i].count() - 1);
      for(int j = 1; j < multipoly[i].count(); ++j)
	{
	  (*currentHoles)[j-1] =  _createGeosLinearRing(multipoly[i].at(j));
	}
      try
	{
	  currentPolygon = geosGeometryFactory->createPolygon(currentOuterRing, currentHoles);
	}
      catch(GEOS_UTIL::GEOSException* e)
	{
	  delete e; delete polygons; return 0;
	}
      (*polygons)[i] = currentPolygon;
    }

  GEOS_GEOM::Geometry* geom = 0;
  try
    {
      geom = geosGeometryFactory->createMultiPolygon(polygons);
    }
  catch(GEOS_UTIL::GEOSException* e)
    {
      delete e; return 0;
    }
  QgsGeometry* g = new QgsGeometry;
  g->setGeos(geom);
  return g;
}

QgsGeometry* QgsGeometry::fromRect(const QgsRect& rect)
{
  QgsPolyline ring;
  ring.append(QgsPoint(rect.xMin(), rect.yMin()));
  ring.append(QgsPoint(rect.xMax(), rect.yMin()));
  ring.append(QgsPoint(rect.xMax(), rect.yMax()));
  ring.append(QgsPoint(rect.xMin(), rect.yMax()));
  ring.append(QgsPoint(rect.xMin(), rect.yMin()));

  QgsPolygon polygon;
  polygon.append(ring);

  return fromPolygon(polygon);
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

  // deep-copy the GEOS Geometry if appropriate
  if (rhs.mGeos)
  {  
    if(rhs.mGeos->getGeometryTypeId() == GEOS_GEOM::GEOS_MULTIPOLYGON)//MH:problems with cloning for multipolygons in geos 2
      {
	GEOS_GEOM::MultiPolygon* multiPoly = dynamic_cast<GEOS_GEOM::MultiPolygon*>(rhs.mGeos);
	if(multiPoly)
	  {
	    std::vector<GEOS_GEOM::Geometry*> polygonVector;
	    for(GEOS_SIZE_T i = 0; i < multiPoly->getNumGeometries(); ++i)
	      {
		polygonVector.push_back((GEOS_GEOM::Geometry*)(multiPoly->getGeometryN(i)));
	      }
	    mGeos = geosGeometryFactory->createMultiPolygon(polygonVector);
	  }
      }
    else if(rhs.mGeos->getGeometryTypeId() == GEOS_GEOM::GEOS_MULTILINESTRING) //MH: and also for cloning multilines
      {
	GEOS_GEOM::MultiLineString* multiLine = dynamic_cast<GEOS_GEOM::MultiLineString*>(rhs.mGeos);
	if(multiLine)
	  {
	    std::vector<GEOS_GEOM::Geometry*> lineVector;
	    for(GEOS_SIZE_T i = 0; i < multiLine->getNumGeometries(); ++i)
	      {
		lineVector.push_back((GEOS_GEOM::Geometry*)(multiLine->getGeometryN(i)));
	      }
	    mGeos = geosGeometryFactory->createMultiLineString(lineVector);
	  }
      }
    else
      {
	mGeos = rhs.mGeos->clone();
      }
  }
  else
  {
    mGeos = 0;
  }    

  mDirtyGeos = rhs.mDirtyGeos;
  mDirtyWkb  = rhs.mDirtyWkb;

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
  if (mGeos)
  {
    delete mGeos;
    mGeos = 0;
  }

  mGeometry = wkb;
  mGeometrySize = length;

  mDirtyWkb   = FALSE;
  mDirtyGeos  = TRUE;

}

unsigned char * QgsGeometry::wkbBuffer()
{
  if (mDirtyWkb)
  {
    // convert from GEOS
    exportGeosToWkb();
  }

  return mGeometry;
}

size_t QgsGeometry::wkbSize()
{
  if (mDirtyWkb)
  {
    exportGeosToWkb();
  }

  return mGeometrySize;
}


QGis::WKBTYPE QgsGeometry::wkbType()
{
  unsigned char *geom = wkbBuffer(); // ensure that wkb representation exists
  if(geom)
    {
      unsigned int wkbType;
      memcpy(&wkbType, (geom+1), sizeof(wkbType));
      return (QGis::WKBTYPE) wkbType;
    }
  else
    {
      return QGis::WKBUnknown;
    }
}


QGis::VectorType QgsGeometry::vectorType()
{
  QGis::WKBTYPE type = wkbType();
  if (type == QGis::WKBPoint || type == QGis::WKBPoint25D ||
      type == QGis::WKBMultiPoint || type == QGis::WKBMultiPoint25D)
    return QGis::Point;
  if (type == QGis::WKBLineString || type == QGis::WKBLineString25D ||
      type == QGis::WKBMultiLineString || type == QGis::WKBMultiLineString25D)
    return QGis::Line;
  if (type == QGis::WKBPolygon || type == QGis::WKBPolygon25D ||
      type == QGis::WKBMultiPolygon || type == QGis::WKBMultiPolygon25D)
    return QGis::Polygon;

  return QGis::Unknown;
}

bool QgsGeometry::isMultipart()
{
  QGis::WKBTYPE type = wkbType();
  if (type == QGis::WKBMultiPoint ||
      type == QGis::WKBMultiPoint25D ||
      type == QGis::WKBMultiLineString ||
      type == QGis::WKBMultiLineString25D ||
      type == QGis::WKBMultiPolygon ||
      type == QGis::WKBMultiPolygon25D)
    return true;

  return false;
}


void QgsGeometry::setGeos(GEOS_GEOM::Geometry* geos)
{
  // TODO - make this more heap-friendly

  if (mGeos)
  {
    delete mGeos;
    mGeos = 0;
  }
  if (mGeometry)
  {
    delete mGeometry;
    mGeometry = 0;
  }

  mGeos = geos;

  mDirtyWkb   = TRUE;
  mDirtyGeos  = FALSE;
}

QgsPoint QgsGeometry::closestVertex(const QgsPoint& point, int& atVertex, int& beforeVertex, int& afterVertex, double& sqrDist)
{
  // TODO: implement with GEOS
  if(mDirtyWkb)
  {
    exportGeosToWkb();
  }

  if (!mGeometry)
  {
    QgsDebugMsg("WKB geometry not available!");
    return QgsPoint(0,0);
  }

  int vertexnr;
  int vertexcounter = 0;
  QGis::WKBTYPE wkbType;
  double actdist = std::numeric_limits<double>::max();
  double x=0;
  double y=0;
  double *tempx,*tempy;
  memcpy(&wkbType, (mGeometry+1), sizeof(int));
  beforeVertex = -1;
  afterVertex = -1;
  bool hasZValue = false;

  switch (wkbType)
  {
    case QGis::WKBPoint25D:
    case QGis::WKBPoint:
      {
        x = *((double *) (mGeometry + 5));
        y = *((double *) (mGeometry + 5 + sizeof(double)));
        actdist = point.sqrDist(x, y);
        vertexnr = 0;
        break;
      }
    case QGis::WKBLineString25D:
      hasZValue = true;
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
          if(hasZValue) //skip z-coordinate for 25D geometries
          {
            ptr += sizeof(double);
          }
        }
        break;
      }
    case QGis::WKBPolygon25D:
      hasZValue = true;
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
            if(hasZValue) //skip z-coordinate for 25D geometries
            {
              ptr += sizeof(double);
            }
            ++vertexcounter;
          }
        }
        break; 
      }
    case QGis::WKBMultiPoint25D:
      hasZValue = true;
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
          if(hasZValue) //skip z-coordinate for 25D geometries
          {
            ptr += sizeof(double);
          }
        }
        break;
      }
    case QGis::WKBMultiLineString25D:
      hasZValue = true;
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
            if(hasZValue) //skip z-coordinate for 25D geometries
            {
              ptr += sizeof(double);
            }
            ++vertexcounter;
          }
        }
        break;
      }
    case QGis::WKBMultiPolygon25D:
      hasZValue = true;
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
              if(hasZValue) //skip z-coordinate for 25D geometries
              {
                ptr += sizeof(double);
              }
              ++vertexcounter;
            }
          }
        }
        break;
      }
    default:
      break;
  }
  sqrDist = actdist;
  atVertex = vertexnr;
  return QgsPoint(x,y);    
}


void QgsGeometry::adjacentVerticies(int atVertex, int& beforeVertex, int& afterVertex)
{
  // TODO: implement with GEOS
  if(mDirtyWkb)
  {
    exportGeosToWkb();
  }

  beforeVertex = -1;
  afterVertex = -1;

  if (!mGeometry)
  {
    QgsDebugMsg("WKB geometry not available!");
    return;
  }

  int vertexcounter = 0;

  QGis::WKBTYPE wkbType;
  bool hasZValue = false;

  memcpy(&wkbType, (mGeometry+1), sizeof(int));

  switch (wkbType)
  {
    case QGis::WKBPoint:
      {
        // NOOP - Points do not have adjacent verticies
        break;
      }
    case QGis::WKBLineString25D:
    case QGis::WKBLineString:
      {
        unsigned char* ptr = mGeometry+5;
        int* npoints = (int*) ptr;

        const int index = atVertex;

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
    case QGis::WKBPolygon25D:
      hasZValue = true;
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
            if(hasZValue) //skip z-coordinate for 25D geometries
            {
              ptr += sizeof(double);
            }
            if (vertexcounter == atVertex)
            { 
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
    case QGis::WKBMultiPoint25D:
    case QGis::WKBMultiPoint:
      {
        // NOOP - Points do not have adjacent verticies
        break;
      }

    case QGis::WKBMultiLineString25D:
      hasZValue = true;
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
            if(hasZValue) //skip z-coordinate for 25D geometries
            {
              ptr += sizeof(double);
            }

            if (vertexcounter == atVertex)
            {
              // Found the vertex of the linestring we were looking for.
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
    case QGis::WKBMultiPolygon25D:
      hasZValue = true;
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
              if(hasZValue) //skip z-coordinate for 25D geometries
              {
                ptr += sizeof(double);
              }
              if (vertexcounter == atVertex)
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

bool QgsGeometry::moveVertexAt(double x, double y, int atVertex)
{
  int vertexnr = atVertex;

  // TODO: implement with GEOS
  if(mDirtyWkb)
  {
    exportGeosToWkb();
  }

  if (!mGeometry)
  {
    QgsDebugMsg("WKB geometry not available!");
    return FALSE;
  }

  QGis::WKBTYPE wkbType;
  bool hasZValue = false;
  unsigned char* ptr = mGeometry+1;
  memcpy(&wkbType, ptr, sizeof(wkbType));
  ptr += sizeof(wkbType);

  switch(wkbType)
  {
    case QGis::WKBPoint25D:
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
    case QGis::WKBMultiPoint25D:
      hasZValue = true;
    case QGis::WKBMultiPoint:
      {
        int* nrPoints = (int*)ptr;
        if(vertexnr > *nrPoints || vertexnr < 0)
        {
          return false;
        }
        ptr += sizeof(int);
        if(hasZValue)
        {
          ptr += (3*sizeof(double)+1+sizeof(int))*vertexnr;
        }
        else
        {
          ptr += (2*sizeof(double)+1+sizeof(int))*vertexnr;
        }
        ptr += (1+sizeof(int));
        memcpy(ptr, &x, sizeof(double));
        ptr += sizeof(double);
        memcpy(ptr, &y, sizeof(double));
        mDirtyGeos = true;
        return true;
      }
    case QGis::WKBLineString25D:
      hasZValue = true;
    case QGis::WKBLineString:
      {
        int* nrPoints = (int*)ptr;
        if(vertexnr > *nrPoints || vertexnr < 0)
        {
          return false;
        }
        ptr += sizeof(int);
        ptr += 2*sizeof(double)*vertexnr;
        if(hasZValue)
        {
          ptr += sizeof(double) * vertexnr;
        }
        memcpy(ptr, &x, sizeof(double));
        ptr += sizeof(double);
        memcpy(ptr, &y, sizeof(double));
        mDirtyGeos = true;
        return true;
      }
    case QGis::WKBMultiLineString25D:
      hasZValue = true;
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
            if(hasZValue)
            {
              ptr += (vertexnr-pointindex)*sizeof(double);
            }
            memcpy(ptr, &x, sizeof(double));
            memcpy(ptr+sizeof(double), &y, sizeof(double));
            mDirtyGeos = true;
            return true;
          }
          pointindex += (*nrPoints);
          ptr += 2*sizeof(double)*(*nrPoints);
          if(hasZValue)
          {
            ptr += sizeof(double)*(*nrPoints);
          }
        }
        return false;
      }
    case QGis::WKBPolygon25D:
      hasZValue = true;
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
            if(hasZValue)
            {
              memcpy(ptr+3*sizeof(double)*(*nrPoints-1), &x, sizeof(double));
            }
            else
            {
              memcpy(ptr+2*sizeof(double)*(*nrPoints-1), &x, sizeof(double));
            }
            if(hasZValue)
            {
              memcpy(ptr+sizeof(double)+3*sizeof(double)*(*nrPoints-1), &y, sizeof(double));
            }
            else
            {
              memcpy(ptr+sizeof(double)+2*sizeof(double)*(*nrPoints-1), &y, sizeof(double));
            }
            mDirtyGeos = true;
            return true;
          }
          else if(vertexnr > pointindex && vertexnr < pointindex + (*nrPoints-1))//move only one point
          {
            ptr += 2*sizeof(double)*(vertexnr - pointindex);
            if(hasZValue)
            {
              ptr += sizeof(double)*(vertexnr - pointindex);
            }
            memcpy(ptr, &x, sizeof(double));
            ptr += sizeof(double);
            memcpy(ptr, &y, sizeof(double));
            mDirtyGeos = true;
            return true;
          }
          ptr += 2*sizeof(double)*(*nrPoints);
          if(hasZValue)
          {
            ptr += sizeof(double)*(*nrPoints);
          }
          pointindex += *nrPoints;
        }
        return false;
      }
    case QGis::WKBMultiPolygon25D:
      hasZValue = true;
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
              if(hasZValue)
              {
                memcpy(ptr+3*sizeof(double)*(*nrPoints-1), &x, sizeof(double));
              }
              else
              {
                memcpy(ptr+2*sizeof(double)*(*nrPoints-1), &x, sizeof(double));
              }
              if(hasZValue)
              {
                memcpy(ptr+sizeof(double)+3*sizeof(double)*(*nrPoints-1), &y, sizeof(double));
              }
              else
              {
                memcpy(ptr+sizeof(double)+2*sizeof(double)*(*nrPoints-1), &y, sizeof(double));
              }
              mDirtyGeos = true;
              return true;
            }
            else if(vertexnr > pointindex && vertexnr < pointindex + (*nrPoints-1))//move only one point
            {
              ptr += 2*sizeof(double)*(vertexnr - pointindex);
              if(hasZValue)
              {
                ptr += 3*sizeof(double)*(vertexnr - pointindex);
              }
              memcpy(ptr, &x, sizeof(double));
              ptr += sizeof(double);
              memcpy(ptr, &y, sizeof(double));
              mDirtyGeos = true;
              return true;
            }
            ptr += 2*sizeof(double)*(*nrPoints);
            if(hasZValue)
            {
              ptr += sizeof(double)*(*nrPoints);
            }
            pointindex += *nrPoints;
          }
        }
        return false;
      }

    default:
      return false;
  }
}

bool QgsGeometry::deleteVertexAt(int atVertex)
{
  int vertexnr = atVertex;
  bool success = false;

  // TODO: implement with GEOS
  if(mDirtyWkb)
  {
    exportGeosToWkb();
  }

  if (!mGeometry)
  {
    QgsDebugMsg("WKB geometry not available!");
    return FALSE;
  }

  //create a new geometry buffer for the modified geometry
  unsigned char* newbuffer;
  int pointindex = 0;
  QGis::WKBTYPE wkbType;
  bool hasZValue = false;
  unsigned char* ptr = mGeometry+1;
  memcpy(&wkbType, ptr, sizeof(wkbType));

  switch(wkbType)
  {
    case QGis::WKBPoint25D:
    case QGis::WKBLineString25D:
    case QGis::WKBPolygon25D:
    case QGis::WKBMultiPoint25D:
    case QGis::WKBMultiLineString25D:
    case QGis::WKBMultiPolygon25D:
      newbuffer = new unsigned char[mGeometrySize-3*sizeof(double)];
      break;
    default:
      newbuffer = new unsigned char[mGeometrySize-2*sizeof(double)];
  }

  memcpy(newbuffer, mGeometry, 1+sizeof(int)); //endian and type are the same

  ptr += sizeof(wkbType);
  unsigned char* newBufferPtr = newbuffer+1+sizeof(int);

  switch(wkbType)
  {
    case QGis::WKBPoint25D:
    case QGis::WKBPoint:
      {
        break; //cannot remove the only point vertex
      }
    case QGis::WKBMultiPoint25D:
      hasZValue = true;
    case QGis::WKBMultiPoint:
      {
        //todo
      }
    case QGis::WKBLineString25D:
      hasZValue = true;
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
            if(hasZValue)
            {
              newBufferPtr += sizeof(double);
            }
          }
          else
          {
            success = true;
          }
          ptr += 2*sizeof(double);
          if(hasZValue)
          {
            ptr += sizeof(double);
          }
          ++pointindex;
        }
        break;
      }
    case QGis::WKBMultiLineString25D:
      hasZValue = true;
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
              if(hasZValue)
              {
                newBufferPtr += sizeof(double);
              }
            }
            else
            {
              success = true;
            }
            ptr += 2*sizeof(double);
            if(hasZValue)
            {
              ptr += sizeof(double);
            }
            ++pointindex;
          }
        }
        break;
      }
    case QGis::WKBPolygon25D:
      hasZValue = true;
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
              if(hasZValue)
              {
                newBufferPtr += sizeof(double);
              }
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
                memcpy(newBufferPtr-(*nPoints-1)*2*sizeof(double), ptr-2*sizeof(double), sizeof(double));
                memcpy(newBufferPtr-(*nPoints-1)*2*sizeof(double)+sizeof(double), ptr-sizeof(double), sizeof(double));
              }
              success = true;
            }
            ptr += 2*sizeof(double);
            if(hasZValue)
            {
              ptr += sizeof(double);
            }
            ++pointindex;
          }
        }
        break;
      }
    case QGis::WKBMultiPolygon25D:
      hasZValue = true;
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
                if(hasZValue)
                {
                  newBufferPtr += sizeof(double);
                }
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
                  memcpy(newBufferPtr-(*nPoints-1)*2*sizeof(double), ptr-2*sizeof(double), sizeof(double));
                  memcpy(newBufferPtr-(*nPoints-1)*2*sizeof(double)+sizeof(double), ptr-sizeof(double), sizeof(double));
                }
                success = true;
              }
              ptr += 2*sizeof(double);
              if(hasZValue)
              {
                ptr += sizeof(double);
              }
              ++pointindex;
            }
          } 
        }
        break;
      }
    case QGis::WKBUnknown:
      break;
  }
  if(success)
  {
    delete mGeometry;
    mGeometry = newbuffer;
    mGeometrySize -= (2*sizeof(double));
    if(hasZValue)
    {
      mGeometrySize -= sizeof(double);
    }
    mDirtyGeos = true;
    return true;
  }
  else
  {
    delete[] newbuffer;
    return false;
  }
}

bool QgsGeometry::insertVertexBefore(double x, double y, int beforeVertex)
{
  int vertexnr = beforeVertex;
  bool success = false;

  // TODO: implement with GEOS
  if(mDirtyWkb)
  {
    exportGeosToWkb();
  }

  if (!mGeometry)
  {
    QgsDebugMsg("WKB geometry not available!");
    return FALSE;
  }

  //create a new geometry buffer for the modified geometry
  unsigned char* newbuffer;

  int pointindex = 0;
  QGis::WKBTYPE wkbType;
  bool hasZValue = false;

  unsigned char* ptr = mGeometry+1;
  memcpy(&wkbType, ptr, sizeof(wkbType));

  switch(wkbType)
  {
    case QGis::WKBPoint25D:
    case QGis::WKBLineString25D:
    case QGis::WKBPolygon25D:
    case QGis::WKBMultiPoint25D:
    case QGis::WKBMultiLineString25D:
    case QGis::WKBMultiPolygon25D:
      newbuffer = new unsigned char[mGeometrySize+3*sizeof(double)];
      break;
    default:
      newbuffer = new unsigned char[mGeometrySize+2*sizeof(double)];
  }
  memcpy(newbuffer, mGeometry, 1+sizeof(int)); //endian and type are the same

  ptr += sizeof(wkbType);
  unsigned char* newBufferPtr = newbuffer+1+sizeof(int);

  switch(wkbType)
  {
    case QGis::WKBPoint25D:
    case QGis::WKBPoint://cannot insert a vertex before another one on point types
      {
        delete newbuffer;
        return false;
      }
    case QGis::WKBMultiPoint25D:
      hasZValue = true;
    case QGis::WKBMultiPoint:
      {
        //todo
        break;
      }
    case QGis::WKBLineString25D:
      hasZValue = true;
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
          if(hasZValue)
          {
            ptr += sizeof(double);
          }
          newBufferPtr += 2*sizeof(double);
          if(hasZValue)
          {
            newBufferPtr += sizeof(double);
          }
          ++pointindex;
          if(pointindex == vertexnr)
          {
            memcpy(newBufferPtr, &x, sizeof(double));
            memcpy(newBufferPtr+sizeof(double), &y, sizeof(double));
            newBufferPtr += 2*sizeof(double);
            if(hasZValue)
            {
              newBufferPtr += sizeof(double);
            }
            success = true;
          }
        }
        break;
      }
    case QGis::WKBMultiLineString25D:
      hasZValue = true;
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
            if(hasZValue)
            {
              ptr += sizeof(double);
              newBufferPtr += sizeof(double);
            }
            ++pointindex;
            if(pointindex == vertexnr)
            {
              memcpy(newBufferPtr, &x, sizeof(double));
              memcpy(newBufferPtr+sizeof(double), &y, sizeof(double));
              newBufferPtr += 2*sizeof(double);
              if(hasZValue)
              {
                newBufferPtr += sizeof(double);
              }
              success = true;
            }
          }
        }
        break;
      }
    case QGis::WKBPolygon25D:
      hasZValue = true;
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
            if(hasZValue)
            {
              ptr += sizeof(double);
              newBufferPtr += sizeof(double);
            }
            ++pointindex;
            if(pointindex == vertexnr)
            {
              memcpy(newBufferPtr, &x, sizeof(double));
              memcpy(newBufferPtr+sizeof(double), &y, sizeof(double));
              newBufferPtr += 2*sizeof(double);
              if(hasZValue)
              {
                newBufferPtr += sizeof(double);
              }
              success = true;
            }
          }
        }
        break;
      }
    case QGis::WKBMultiPolygon25D:
      hasZValue = true;
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
              if(hasZValue)
              {
                ptr += sizeof(double);
                newBufferPtr += sizeof(double);
              }
              ++pointindex;
              if(pointindex == vertexnr)
              {
                memcpy(newBufferPtr, &x, sizeof(double));
                memcpy(newBufferPtr+sizeof(double), &y, sizeof(double));
                newBufferPtr += 2*sizeof(double);
                if(hasZValue)
                {
                  newBufferPtr += sizeof(double);
                }
                success = true;
              }
            }
          }

        }
        break;
      }
    case QGis::WKBUnknown:
      break;
  }

  if(success)
  {
    delete mGeometry;
    mGeometry = newbuffer;
    mGeometrySize += 2*sizeof(double);
    if(hasZValue)
    {
      mGeometrySize += sizeof(double);
    }
    mDirtyGeos = true;
    return true;
  }
  else
  {
    delete newbuffer;
    return false;
  }
}

QgsPoint QgsGeometry::vertexAt(int atVertex)
{
  double x,y;

  if (mGeos) //try to find the vertex from the Geos geometry (it present)
  {
    GEOS_GEOM::CoordinateSequence* cs = mGeos->getCoordinates();
    if(cs)
    {
      const GEOS_GEOM::Coordinate& coord = cs->getAt(atVertex);
      x = coord.x;
      y = coord.y;
      delete cs;
      return QgsPoint(x,y);
    }
  }
  else if(mGeometry)
  {
    QGis::WKBTYPE wkbType;
    bool hasZValue = false;
    unsigned char* ptr;

    memcpy(&wkbType, (mGeometry+1), sizeof(int));
    switch (wkbType)
    {
      case QGis::WKBPoint25D:
      case QGis::WKBPoint:
        {
          if(atVertex == 0)
          {
            ptr = mGeometry+1+sizeof(int);
            memcpy(&x, ptr, sizeof(double));
            ptr += sizeof(double);
            memcpy(&y, ptr, sizeof(double));
            return QgsPoint(x,y);
          }
          else
          {
            return QgsPoint(0,0);
          }
        }
      case QGis::WKBLineString25D:
        hasZValue = true;
      case QGis::WKBLineString:
        {
          int *nPoints;
          // get number of points in the line
          ptr = mGeometry + 1 + sizeof(int);     // now at mGeometry.numPoints
          nPoints = (int *) ptr;

          // return error if underflow
          if (0 > atVertex || *nPoints <= atVertex)
          {
            return QgsPoint(0,0);
          }

          // copy the vertex coordinates 
          if(hasZValue)
          {
            ptr = mGeometry + 9 + (atVertex * 3*sizeof(double));
          }
          else
          {
            ptr = mGeometry + 9 + (atVertex * 2*sizeof(double));
          }
          memcpy(&x, ptr, sizeof(double));
          ptr += sizeof(double);
          memcpy(&y, ptr, sizeof(double));
          return QgsPoint(x,y);
        }
      case QGis::WKBPolygon25D:
        hasZValue = true;
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
              if(pointindex == atVertex)
              {
                memcpy(&x, ptr, sizeof(double));
                ptr += sizeof(double);
                memcpy(&y, ptr, sizeof(double));
                return QgsPoint(x,y);
              }
              ptr += 2*sizeof(double);
              if(hasZValue)
              {
                ptr += sizeof(double);
              }
              ++pointindex;
            }
          }
          return QgsPoint(0,0);
        }
      case QGis::WKBMultiPoint25D:
        hasZValue = true;
      case QGis::WKBMultiPoint:
        {
          ptr = mGeometry+1+sizeof(int);
          int* nPoints = (int*)ptr;
          if(atVertex < 0 || atVertex >= *nPoints)
          {
            return QgsPoint(0,0);
          }
          if(hasZValue)
          {
            ptr += atVertex*(3*sizeof(double)+1+sizeof(int));
          }
          else
          {
            ptr += atVertex*(2*sizeof(double)+1+sizeof(int));
          }
          ptr += 1+sizeof(int);
          memcpy(&x, ptr, sizeof(double));
          ptr += sizeof(double);
          memcpy(&y, ptr, sizeof(double));
          return QgsPoint(x,y);
        }
      case QGis::WKBMultiLineString25D:
        hasZValue = true;
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
              if(pointindex == atVertex)
              {
                memcpy(&x, ptr, sizeof(double));
                ptr += sizeof(double);
                memcpy(&y, ptr, sizeof(double));
                return QgsPoint(x,y);
              }
              ptr += 2*sizeof(double);
              if(hasZValue)
              {
                ptr += sizeof(double);
              }
              ++pointindex;
            }
          }
          return QgsPoint(0,0);
        }
      case QGis::WKBMultiPolygon25D:
        hasZValue = true;
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
                if(pointindex == atVertex)
                {
                  memcpy(&x, ptr, sizeof(double));
                  ptr += sizeof(double);
                  memcpy(&y, ptr, sizeof(double));
                  return QgsPoint(x,y);
                }
                ++pointindex;
                ptr += 2*sizeof(double);
                if(hasZValue)
                {
                  ptr += sizeof(double);
                }
              }
            }
          }
          return QgsPoint(0,0);
        }
      default:
        QgsDebugMsg("error: mGeometry type not recognized");
        return QgsPoint(0,0);
    }
  }
  else
  {
    QgsDebugMsg("error: no mGeometry pointer");
  }     

  return QgsPoint(0,0);
}


double QgsGeometry::sqrDistToVertexAt(QgsPoint& point, int atVertex)
{
  QgsPoint pnt = vertexAt(atVertex);
  if (pnt != QgsPoint(0,0))
  {
    QgsDebugMsg("Exiting with distance to " + pnt.stringRep());
    return point.sqrDist(pnt);
  }
  else
  {
    QgsDebugMsg("Exiting with std::numeric_limits<double>::max().");
    // probably safest to bail out with a very large number
    return std::numeric_limits<double>::max();
  }
}


double QgsGeometry::closestVertexWithContext(const QgsPoint& point, int& atVertex)
{
  // Initialise some stuff
  double sqrDist = std::numeric_limits<double>::max();
  int closestVertexIndex = 0;

  // set up the GEOS geometry
  exportWkbToGeos();

  if (!mGeos)
  {
    QgsDebugMsg("GEOS geometry not available!");
    return -1;
  }

  GEOS_GEOM::CoordinateSequence* sequence = mGeos->getCoordinates();
  if(sequence)
  {
    for(GEOS_SIZE_T i = 0; i < sequence->getSize(); ++i)
    {
      double testDist = point.sqrDist(sequence->getAt(i).x, sequence->getAt(i).y);
      if(testDist < sqrDist)
      {
        closestVertexIndex = i;
        sqrDist = testDist;
      }
    }
  }
  atVertex = closestVertexIndex;

  return sqrDist;
}


double QgsGeometry::closestSegmentWithContext(const QgsPoint& point,
    QgsPoint& minDistPoint,
    int& beforeVertex)
{
  QgsPoint distPoint;

  QGis::WKBTYPE wkbType;
  bool hasZValue = false;
  double *thisx = NULL;
  double *thisy = NULL;
  double *prevx = NULL;
  double *prevy = NULL;
  double testdist;
  int closestSegmentIndex = 0;

  // Initialise some stuff
  double sqrDist = std::numeric_limits<double>::max();

  // TODO: implement with GEOS
  if(mDirtyWkb) //convert latest geos to mGeometry
  {
    exportGeosToWkb();
  }

  if (!mGeometry)
  { 
    QgsDebugMsg("WKB geometry not available!");
    return -1;
  }

  memcpy(&wkbType, (mGeometry+1), sizeof(int));

  switch (wkbType)
  {
    case QGis::WKBPoint25D:
    case QGis::WKBPoint:
    case QGis::WKBMultiPoint25D:
    case QGis::WKBMultiPoint:  
      {
        // Points have no lines
        return -1;
      }
    case QGis::WKBLineString25D:
      hasZValue = true;
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
            if((testdist = distanceSquaredPointToSegment(point, prevx, prevy, thisx, thisy, distPoint)) < sqrDist )
            {
              closestSegmentIndex = index;
              sqrDist = testdist;
              minDistPoint = distPoint;
            }
          }
          ptr += sizeof(double);
          if(hasZValue)
          {
            ptr += sizeof(double);
          }
        }
        beforeVertex = closestSegmentIndex;
        break;
      }
    case QGis::WKBMultiLineString25D:
      hasZValue = true;
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
            if(hasZValue)
            {
              ptr += sizeof(double);
            }
            if(prevx && prevy)
            {
              if((testdist = distanceSquaredPointToSegment(point, prevx, prevy, thisx, thisy, distPoint)) < sqrDist )
              {
                closestSegmentIndex = pointindex;
                sqrDist = testdist;
                minDistPoint = distPoint;
              }
            }
            prevx = thisx;
            prevy = thisy;
            ++pointindex;
          }
        }
        beforeVertex = closestSegmentIndex;
        break;
      }
    case QGis::WKBPolygon25D:
      hasZValue = true;
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
            if(hasZValue)
            {
              ptr += sizeof(double);
            }
            if(prevx && prevy)
            {
              if((testdist = distanceSquaredPointToSegment(point, prevx, prevy, thisx, thisy, distPoint)) < sqrDist )
              {
                closestSegmentIndex = index;
                sqrDist = testdist;
                minDistPoint = distPoint;
              }
            }
            prevx = thisx;
            prevy = thisy;
            ++index;
          }
        }
        beforeVertex = closestSegmentIndex;
        break;
      }
    case QGis::WKBMultiPolygon25D:
      hasZValue = true;
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
              if(hasZValue)
              {
                ptr += sizeof(double);
              }
              if(prevx && prevy)
              {
                if((testdist = distanceSquaredPointToSegment(point, prevx, prevy, thisx, thisy, distPoint)) < sqrDist )
                {
                  closestSegmentIndex = pointindex;
                  sqrDist = testdist;
                  minDistPoint = distPoint;
                }
              }
              prevx = thisx;
              prevy = thisy;
              ++pointindex;
            }
          }
        }
        beforeVertex = closestSegmentIndex;
        break;
      }
    case QGis::WKBUnknown:
    default:
      return -1;
      break;
  } // switch (wkbType)


  QgsDebugMsg("Exiting with nearest point " + point.stringRep() +
      ", dist: " + QString::number(sqrDist) + ".");

  return sqrDist;
}                 

int QgsGeometry::addRing(const QList<QgsPoint>& ring)
{
  //bail out if this geometry is not polygon/multipolygon
  if(vectorType() != QGis::Polygon)
    {
      return 1;
    }

  //test for invalid geometries
  if(ring.size() < 4)
    {
      return 3;
    }

  //ring must be closed
  if(ring.first() != ring.last())
    {
      return 2;
    }
  
  //create geos geometry from wkb if not already there
  if(!mGeos || mDirtyGeos)
    {
      exportWkbToGeos();
    }
  
  //Fill GEOS Polygons of the feature into list
  std::list<GEOS_GEOM::Polygon*> polygonList; //list of polygon pointers (only one for polygon geometries)
  GEOS_GEOM::Polygon* thisPolygon = 0;
  GEOS_GEOM::MultiPolygon* thisMultiPolygon = 0;

  if(this->wkbType() == QGis::WKBPolygon)
    {
      thisPolygon = dynamic_cast<GEOS_GEOM::Polygon*>(mGeos);
      if(!thisPolygon)
	{
	  return 1;
	}
      polygonList.push_back(thisPolygon);
    }
  else if(this->wkbType() == QGis::WKBMultiPolygon)
    {
      thisMultiPolygon = dynamic_cast<GEOS_GEOM::MultiPolygon*>(mGeos);
      if(!thisMultiPolygon)
	{
	  return 1;
	}
      int numPolys = thisMultiPolygon->getNumGeometries();
      for(int i = 0; i < numPolys; ++i)
	{
	  polygonList.push_back((GEOS_GEOM::Polygon*)(thisMultiPolygon->getGeometryN(i)));
	}
    }

  //create new ring
  GEOS_GEOM::DefaultCoordinateSequence* newSequence=new GEOS_GEOM::DefaultCoordinateSequence();
  for(QList<QgsPoint>::const_iterator it = ring.begin(); it != ring.end(); ++it)
    {
      newSequence->add(GEOS_GEOM::Coordinate(it->x(),it->y()));
    }
  
 
  //create new ring
  GEOS_GEOM::LinearRing* newRing = 0;
  try
    {
      newRing = geosGeometryFactory->createLinearRing(newSequence);
    }
  catch(GEOS_UTIL::IllegalArgumentException* e)
    {
      delete newSequence;
      delete e;
      return 3;
    }
  std::vector<GEOS_GEOM::Geometry*> dummyVector;

  //create polygon from new ring because there is a problem with geos operations and linear rings
  GEOS_GEOM::Polygon* newRingPolygon = geosGeometryFactory->createPolygon(*newRing, dummyVector);
  if(!newRing || !newRingPolygon || !newRing->isValid() || !newRingPolygon->isValid())
    {
      QgsDebugMsg("ring is not valid");
      delete newRing;
      delete newRingPolygon;
      return 3;
    }
  
  GEOS_GEOM::LinearRing* outerRing = 0; //outer ring of already existing feature
  std::vector<GEOS_GEOM::Geometry*>* inner = 0; //vector of inner rings. The existing rings and the new one will be added
  int numberOfPolyContainingRing = 0; //for multipolygons: store index of the polygon where the ring is
  bool foundPoly = false; //set to true as soon we found a polygon containing the ring

  for(std::list<GEOS_GEOM::Polygon*>::const_iterator it = polygonList.begin(); it != polygonList.end(); ++it)
    {
      /***********inner rings*****************************************/
      //consider already existing rings
      inner=new std::vector<GEOS_GEOM::Geometry*>();
      int numExistingRings = (*it)->getNumInteriorRing();
      inner->resize(numExistingRings + 1);
      for(int i = 0; i < numExistingRings; ++i)
	{
	  GEOS_GEOM::LinearRing* existingRing = geosGeometryFactory->createLinearRing((*it)->getInteriorRingN(i)->getCoordinates());
	  //create polygon from new ring because there is a problem with geos operations and linear rings
	  GEOS_GEOM::Polygon* existingRingPolygon = geosGeometryFactory->createPolygon(*existingRing, dummyVector);
	 
      //check, if the new ring intersects the existing one and bail out if yes
	  //if(existingRing->disjoint(newRing))
	  if(!existingRingPolygon->disjoint(newRingPolygon)) //does only work with polygons, not linear rings
	    {
	      QgsDebugMsg("new ring not disjoint with existing ring");
	      //delete objects wich are no longer needed
	      delete existingRing;
	      delete existingRingPolygon;
	      for(std::vector<GEOS_GEOM::Geometry*>::iterator it = inner->begin(); it != inner->end(); ++it)
		{
		  delete *it;
		}
	      delete inner;
	      delete newRing;
	      delete newRingPolygon;
	      return 4; //error: ring not disjoint with existing rings
	    }
	  (*inner)[i] = existingRing;
	  delete existingRingPolygon; //delete since this polygon has only be created for disjoint() test
	}
      
      //also add new ring to the vector
      if(newRing)
	{
	  (*inner)[numExistingRings] = newRing;
	}

      /*****************outer ring****************/
      outerRing = geosGeometryFactory->createLinearRing((*it)->getExteriorRing()->getCoordinates());
      //create polygon from new ring because there is a problem with geos operations and linear rings
      GEOS_GEOM::Polygon* outerRingPolygon = geosGeometryFactory->createPolygon(*outerRing, dummyVector);

      //check if the new ring is within the outer shell of the polygon and bail out if not
      //if(newRing->within(outerRing))
      if(newRingPolygon->within(outerRingPolygon)) //does only work for polygons, not linear rings
	{
	  QgsDebugMsg("new ring within outer ring");
	  foundPoly = true;
	  delete outerRingPolygon;
	  break; //ring is in geometry and does not intersect existing rings -> proceed with adding ring to feature
	}

      //we need to search in other polygons...
      for(std::vector<GEOS_GEOM::Geometry*>::iterator it = inner->begin(); it != inner->end(); ++it)
      {
	if( (*it) != newRing) //we need newRing for later polygons
	  {
	    delete *it;
	  }
      }
      delete inner;
      delete outerRing;
      delete outerRingPolygon;
      
      ++numberOfPolyContainingRing;
    }

  delete newRingPolygon;
 
  if(foundPoly)
    {
      GEOS_GEOM::Polygon* newPolygon = geosGeometryFactory->createPolygon(outerRing,inner);
      if(this->wkbType() == QGis::WKBPolygon)
	{
	  delete mGeos;
	  mGeos = newPolygon;
	}
      else if(this->wkbType() == QGis::WKBMultiPolygon)
	{
	  //remember in which polygon the ring is and replace only this polygon
	  std::vector<GEOS_GEOM::Geometry*>* polygons = new std::vector<GEOS_GEOM::Geometry*>();
	  int numPolys = thisMultiPolygon->getNumGeometries();
	    for(int i = 0; i < numPolys; ++i)
	      {
		if(i == numberOfPolyContainingRing)
		  {
		    polygons->push_back(newPolygon);
		  }
		else
		  {
		    GEOS_GEOM::Polygon* p = (GEOS_GEOM::Polygon*)(thisMultiPolygon->getGeometryN(i)->clone());
		    polygons->push_back(p);
		  }
	      }
	  delete mGeos;
	  mGeos = geosGeometryFactory->createMultiPolygon(polygons);
	}
      mDirtyWkb = true;
      mDirtyGeos = false;
      return 0;
    }
  else
    {
      delete newRing;
      return 5;
    }
}

int QgsGeometry::addIsland(const QList<QgsPoint>& ring)
{
  //Ring needs to have at least three points and must be closed
  if(ring.size() < 4)
    {
      return 2;
    }

  //ring must be closed
  if(ring.first() != ring.last())
    {
      return 2;
    }

  if(wkbType() == QGis::WKBPolygon || wkbType() == QGis::WKBPolygon25D)
    {
      if(!convertToMultiType())
	{
	  return 1;
	}
    }

  //bail out if wkbtype is not multipolygon
  if(wkbType() != QGis::WKBMultiPolygon && wkbType() != QGis::WKBMultiPolygon25D)
    {
      return 1;
    }

  //create geos geometry from wkb if not already there
  if(!mGeos || mDirtyGeos)
    {
      exportWkbToGeos();
    }

  //this multipolygon
  GEOS_GEOM::MultiPolygon* thisMultiPolygon = dynamic_cast<GEOS_GEOM::MultiPolygon*>(mGeos);
  if(!thisMultiPolygon)
    {
      return 1;
    }

  //create new polygon from ring

  //coordinate sequence first
  GEOS_GEOM::DefaultCoordinateSequence* newSequence=new GEOS_GEOM::DefaultCoordinateSequence();
  for(QList<QgsPoint>::const_iterator it = ring.begin(); it != ring.end(); ++it)
    {
      newSequence->add(GEOS_GEOM::Coordinate(it->x(),it->y()));
    }
  
  //then linear ring
  GEOS_GEOM::LinearRing* newRing = 0;
  try
    {
      newRing = geosGeometryFactory->createLinearRing(newSequence);
    }
  catch(GEOS_UTIL::IllegalArgumentException* e)
    {
      delete e;
      delete newSequence;
      return 2;
    }

  //finally the polygon
  std::vector<GEOS_GEOM::Geometry*> dummyVector;
  GEOS_GEOM::Polygon* newPolygon = geosGeometryFactory->createPolygon(*newRing, dummyVector);
  delete newRing;

  if(!newPolygon || !newPolygon->isValid())
    {
      delete newPolygon;
      return 2;
    }

  //create new multipolygon
  std::vector<GEOS_GEOM::Geometry*>* newMultiPolygonVector = new std::vector<GEOS_GEOM::Geometry*>();
  for(GEOS_SIZE_T i = 0; i < thisMultiPolygon->getNumGeometries(); ++i)
    {
      const GEOS_GEOM::Geometry* polygonN = thisMultiPolygon->getGeometryN(i);
      
      //bail out if new polygon is not disjoint with existing ones
      if(!polygonN->disjoint(newPolygon))
	{
	  delete newPolygon;
	  for(std::vector<GEOS_GEOM::Geometry*>::iterator it =newMultiPolygonVector->begin(); it != newMultiPolygonVector->end(); ++it)
	    {
	      delete *it;
	    }
	  delete newMultiPolygonVector;
	  return 3;
	}
      newMultiPolygonVector->push_back(polygonN->clone());
    }
  newMultiPolygonVector->push_back(newPolygon);
  GEOS_GEOM::MultiPolygon* newMultiPolygon = geosGeometryFactory->createMultiPolygon(newMultiPolygonVector);
  
  delete mGeos;
  mGeos = newMultiPolygon;

  mDirtyWkb = true;
  mDirtyGeos = false;
  return 0;
}

int QgsGeometry::translate(double dx, double dy)
{
  if(mDirtyWkb)
  {
    exportGeosToWkb();
  }

  if (!mGeometry)
  {
    QgsDebugMsg("WKB geometry not available!");
    return 1;
  }

  QGis::WKBTYPE wkbType;
  memcpy(&wkbType, &(mGeometry[1]), sizeof(int));
  bool hasZValue = false;
  int wkbPosition = 5;

  switch (wkbType)
  {
    case QGis::WKBPoint25D:
    case QGis::WKBPoint:
      {
	translateVertex(wkbPosition, dx, dy, hasZValue);
      }
      break;

    case QGis::WKBLineString25D:
      hasZValue = true;
    case QGis::WKBLineString:
      {
        int* npoints=(int*)(&mGeometry[wkbPosition]);
        wkbPosition+=sizeof(int);
        for(int index=0;index<*npoints;++index)
        {
	  translateVertex(wkbPosition, dx, dy, hasZValue);
        }
        break;
      }

    case QGis::WKBPolygon25D:
      hasZValue = true;
    case QGis::WKBPolygon:
      {
        int* nrings = (int*)(&(mGeometry[wkbPosition]));
	wkbPosition += sizeof(int);
        int* npoints;

        for(int index=0;index<*nrings;++index)
        {
          npoints=(int*)(&(mGeometry[wkbPosition]));
          wkbPosition+=sizeof(int);
          for(int index2=0;index2<*npoints;++index2)
          {
	    translateVertex(wkbPosition, dx, dy, hasZValue);
          }
        }
        break; 
      }

  case QGis::WKBMultiPoint25D:
    hasZValue = true;
  case QGis::WKBMultiPoint:
    {
        int* npoints=(int*)(&(mGeometry[wkbPosition]));
        wkbPosition+=sizeof(int);
        for(int index=0;index<*npoints;++index)
	  {
	    wkbPosition += (sizeof(int) + 1);
	    translateVertex(wkbPosition, dx, dy, hasZValue);
	  }
	break;
    }

  case QGis::WKBMultiLineString25D:
    hasZValue = true;
  case QGis::WKBMultiLineString:
    {
      int* nlines=(int*)(&(mGeometry[wkbPosition]));
      int* npoints = 0;
      wkbPosition+=sizeof(int);
      for(int index=0;index<*nlines;++index)
        {
          wkbPosition += (sizeof(int) + 1);
	  npoints = (int*)(&(mGeometry[wkbPosition]));
	  wkbPosition += sizeof(int);
	  for(int index2 = 0; index2 < *npoints; ++index2)
	    {
	      translateVertex(wkbPosition, dx, dy, hasZValue);
	    }
	}
      break;
    }

  case QGis::WKBMultiPolygon25D:
    hasZValue = true;
  case QGis::WKBMultiPolygon:
      {
        int* npolys=(int*)(&(mGeometry[wkbPosition]));
        int* nrings;
        int* npoints;
        wkbPosition+=sizeof(int);
        for(int index=0;index<*npolys;++index)
        {
          wkbPosition += (1 + sizeof(int)); //skip endian and polygon type
          nrings=(int*)(&(mGeometry[wkbPosition]));
          wkbPosition+=sizeof(int);
          for(int index2=0;index2<*nrings;++index2)
          {
            npoints=(int*)(&(mGeometry[wkbPosition]));
            wkbPosition+=sizeof(int);
            for(int index3=0;index3<*npoints;++index3)
            { 
	      translateVertex(wkbPosition, dx, dy, hasZValue);
	    }
	  }
	}
      }

      default:
	break;
      }
  mDirtyGeos = true;
  return 0;   
}

int QgsGeometry::splitGeometry(const QList<QgsPoint>& splitLine, QList<QgsGeometry*>& newGeometries)
{
  int returnCode = 0;

  //return if this type is point/multipoint
  if(vectorType() == QGis::Point)
    {
      return 1; //cannot split points
    }

  //make sure, mGeos and mWkb are there and up-to-date
  if(mDirtyWkb)
    {
      exportGeosToWkb();
    }
  if(!mGeos || mDirtyGeos)
    {
      exportWkbToGeos();
    }

  //make sure splitLine is valid
  if(splitLine.size() < 2)
    {
      return 1;
    }

  newGeometries.clear();

  try
    {
      GEOS_GEOM::DefaultCoordinateSequence* splitLineCoords = new GEOS_GEOM::DefaultCoordinateSequence();
      QList<QgsPoint>::const_iterator lineIt;
      for(lineIt = splitLine.constBegin(); lineIt != splitLine.constEnd(); ++lineIt)
	{
	  splitLineCoords->add(GEOS_GEOM::Coordinate(lineIt->x(),lineIt->y()));
	}
      GEOS_GEOM::LineString* splitLineGeos = geosGeometryFactory->createLineString(splitLineCoords);
      if(!splitLineGeos)
	{
	  delete splitLineCoords;
	  return 1;
	}
      
      if(!splitLineGeos->isValid() || !splitLineGeos->isSimple())
	{
	  delete splitLineGeos;
	  return 1;
	}
      
      //for line/multiline: call splitLinearGeometry
      if(vectorType() == QGis::Line)
	{
	  returnCode = splitLinearGeometry(splitLineGeos, newGeometries);
	  delete splitLineGeos;
	}
      else if(vectorType() == QGis::Polygon)
	{
	  returnCode = splitPolygonGeometry(splitLineGeos, newGeometries);
	  delete splitLineGeos;
	}
      else
	{
	  return 1;
	}
    }
  catch(GEOS_UTIL::GEOSException* e)
    {
      delete e; return 2;
    }
  return returnCode;
}

int QgsGeometry::makeDifference(QgsGeometry* other)
{
  //make sure geos geometry is up to date
  if(!mGeos || mDirtyGeos)
    {
      exportWkbToGeos();
    }

  if(!mGeos)
    {
      return 1;
    }

  if(!mGeos->isValid())
    {
      return 2;
    }

  if(!mGeos->isSimple())
    {
      return 3;
    }

  //convert other geometry to geos
  if(!other->mGeos || other->mDirtyGeos)
    {
      other->exportWkbToGeos();
    }
  
  if(!other->mGeos)
    {
      return 4;
    }

  //make geometry::difference
  try
    {
      if(mGeos->intersects(other->mGeos))
	{
	  mGeos = mGeos->difference(other->mGeos);
	}
      else
	{
	  return 0; //nothing to do
	}
    }
  catch(GEOS_UTIL::GEOSException* e)
    {
      delete e;
      return 5;
    }
  
  if(!mGeos)
    {
      mDirtyGeos = true;
      return 6;
    }

  //set wkb dirty to true
  mDirtyWkb = true;
  return 0;
}

QgsRect QgsGeometry::boundingBox()
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
  QGis::WKBTYPE wkbType;
  bool hasZValue = false;

  // TODO: implement with GEOS
  if(mDirtyWkb)
  {
    exportGeosToWkb();
  }

  if(!mGeometry)
  {
    QgsDebugMsg("WKB geometry not available!");
    return QgsRect(0,0,0,0);
  }
  // consider endian when fetching feature type
  //wkbType = (mGeometry[0] == 1) ? mGeometry[1] : mGeometry[4]; //MH: Does not work for 25D geometries
  memcpy(&wkbType, &(mGeometry[1]), sizeof(int));
  switch (wkbType)
  {
    case QGis::WKBPoint25D:
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
    case QGis::WKBMultiPoint25D:
      hasZValue = true;
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
          if(hasZValue)
          {
            ptr += sizeof(double);
          }
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
    case QGis::WKBLineString25D:
      hasZValue = true;
    case QGis::WKBLineString:
      {
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
          if(hasZValue)
          {
            ptr += sizeof(double);
          }
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
    case QGis::WKBMultiLineString25D:
      hasZValue = true;
    case QGis::WKBMultiLineString:
      {
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
            if(hasZValue)
            {
              ptr += sizeof(double);
            }
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
      }
    case QGis::WKBPolygon25D:
      hasZValue = true;
    case QGis::WKBPolygon:
      {
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
            if(hasZValue)
            {
              ptr += sizeof(double);
            }
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
      }
    case QGis::WKBMultiPolygon25D:
      hasZValue = true;
    case QGis::WKBMultiPolygon:
      {
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
              if(hasZValue)
              {
                ptr += sizeof(double);
              }
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
      }

    default:
      QgsDebugMsg("UNKNOWN WKBTYPE ENCOUNTERED");
      return QgsRect(0,0,0,0);
      break;

  }
  return QgsRect(xmin,ymin,xmax,ymax);
}

bool QgsGeometry::intersects(const QgsRect& r)
{
  QgsGeometry* g = fromRect(r);
  bool res = intersects(g);
  delete g;
  return res;
}

bool QgsGeometry::intersects(QgsGeometry* geometry)
{
  try // geos might throw exception on error
  {
    // ensure that both geometries have geos geometry
    exportWkbToGeos();
    geometry->exportWkbToGeos();

    if (!mGeos || !geometry->mGeos)
    {
      QgsDebugMsg("GEOS geometry not available!");
      return false;
    }

     return mGeos->intersects(geometry->mGeos);
  }
  catch (GEOS_UTIL::GEOSException &e)
  {
#if GEOS_VERSION_MAJOR < 3
    QString error = e.toString().c_str();
#else
    QString error = e.what();
#endif
    QgsLogger::warning("GEOS: " + error);
    return false;
  }
}


bool QgsGeometry::contains(QgsPoint* p)
{
  exportWkbToGeos();

  if (!mGeos)
  {
    QgsDebugMsg("GEOS geometry not available!");
    return false;
  }

  GEOS_GEOM::Point* geosPoint = geosGeometryFactory->createPoint(GEOS_GEOM::Coordinate(p->x(), p->y()));

  bool returnval = mGeos->contains(geosPoint);

  delete geosPoint;

  return returnval;
}


QString QgsGeometry::exportToWkt()
{
  QgsDebugMsg("QgsGeometry::exportToWkt: entered");

  // TODO: implement with GEOS
  if(mDirtyWkb)
  {
    exportGeosToWkb();
  }

  if (!mGeometry)
  {
    QgsDebugMsg("WKB geometry not available!");
    return false;
  }

  QGis::WKBTYPE wkbType;
  bool hasZValue = false;
  double *x,*y;

  QString mWkt; // TODO: rename

  // Will this really work when mGeometry[0] == 0 ???? I (gavin) think not.
  //wkbType = (mGeometry[0] == 1) ? mGeometry[1] : mGeometry[4];
  memcpy(&wkbType, &(mGeometry[1]), sizeof(int));

  switch (wkbType)
  {
    case QGis::WKBPoint25D:
    case QGis::WKBPoint:
      {
        mWkt+="POINT(";
        x = (double *) (mGeometry + 5);
        mWkt+=QString::number(*x,'f',6);
        mWkt+=" ";
        y = (double *) (mGeometry + 5 + sizeof(double));
        mWkt+=QString::number(*y,'f',6);
        mWkt+=")";
        return mWkt;
      }

    case QGis::WKBLineString25D:
      hasZValue = true;
    case QGis::WKBLineString:
      {
        QgsDebugMsg("QgsGeometry::exportToWkt: LINESTRING found");
        unsigned char *ptr;
        int *nPoints;
        int idx;

        mWkt+="LINESTRING(";
        // get number of points in the line
        ptr = mGeometry + 5;
        nPoints = (int *) ptr;
        ptr = mGeometry + 1 + 2 * sizeof(int);
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
          if(hasZValue)
          {
            ptr += sizeof(double);
          }
        }
        mWkt+=")";
        return mWkt;
      }

    case QGis::WKBPolygon25D:
      hasZValue = true;
    case QGis::WKBPolygon:
      {
        QgsDebugMsg("QgsGeometry::exportToWkt: POLYGON found");
        unsigned char *ptr;
        int idx, jdx;
        int *numRings, *nPoints;

        mWkt+="POLYGON(";
        // get number of rings in the polygon
        numRings = (int *)(mGeometry + 1 + sizeof(int));
        if (!(*numRings))  // sanity check for zero rings in polygon
        {
          return QString();
        }
        int *ringStart; // index of first point for each ring
        int *ringNumPoints; // number of points in each ring
        ringStart = new int[*numRings];
        ringNumPoints = new int[*numRings];
        ptr = mGeometry+1+2*sizeof(int); // set pointer to the first ring
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
            if(hasZValue)
            {
              ptr += sizeof(double);
            }
          }
          mWkt+=")";
        }
        mWkt+=")";
        delete [] ringStart;
        delete [] ringNumPoints;
        return mWkt;
      }

    case QGis::WKBMultiPoint25D:
      hasZValue = true;
    case QGis::WKBMultiPoint:
      {
        unsigned char *ptr;
        int idx;
        int *nPoints;

        mWkt+="MULTIPOINT(";
        nPoints=(int*)(mGeometry+5);
        ptr=mGeometry+5+sizeof(int);
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
          if(hasZValue)
          {
            ptr += sizeof(double);
          }
        }
        mWkt+=")";
        return mWkt;
      }

    case QGis::WKBMultiLineString25D:
      hasZValue = true;
    case QGis::WKBMultiLineString:
      {
        QgsDebugMsg("QgsGeometry::exportToWkt: MULTILINESTRING found");
        unsigned char *ptr;
        int idx, jdx, numLineStrings;
        int *nPoints;

        mWkt+="MULTILINESTRING(";
        numLineStrings = (int) (mGeometry[5]);
        ptr = mGeometry + 9;
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
            if(hasZValue)
            {
              ptr += sizeof(double);
            }
          }
          mWkt+=")";
        }
        mWkt+=")";
        return mWkt;
      }

    case QGis::WKBMultiPolygon25D:
      hasZValue = true;
    case QGis::WKBMultiPolygon:
      {
        QgsDebugMsg("QgsGeometry::exportToWkt: MULTIPOLYGON found");
        unsigned char *ptr;
        int idx, jdx, kdx;
        int *numPolygons, *numRings, *nPoints;

        mWkt+="MULTIPOLYGON(";
        ptr = mGeometry + 5;
        numPolygons = (int *) ptr;
        ptr = mGeometry + 9;
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
              if(hasZValue)
              {
                ptr += sizeof(double);
              }
            }
            mWkt+=")";
          }
          mWkt+=")";
        }
        mWkt+=")";
        return mWkt;
      }

    default:
      QgsDebugMsg("error: mGeometry type not recognized");
      return QString();
  }
}



bool QgsGeometry::exportWkbToGeos()
{
  QgsDebugMsg("QgsGeometry::exportWkbToGeos: entered.");

  if (!mDirtyGeos)
  {
    // No need to convert again
    return TRUE;
  }

  if (mGeos)
  {
    delete mGeos;
    mGeos = 0;
  }

  if (!mGeometry)
  {
    // no WKB => no GEOS
    mDirtyGeos = FALSE;
    return TRUE;
  }

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
  QGis::WKBTYPE wkbtype;
  bool hasZValue = false;

  //wkbtype = (mGeometry[0] == 1) ? mGeometry[1] : mGeometry[4];
  memcpy(&wkbtype, &(mGeometry[1]), sizeof(int));
  
  try{ //try-catch block for geos exceptions
    switch(wkbtype)
      {
      case QGis::WKBPoint25D:
      case QGis::WKBPoint:
	{
	  x = (double *) (mGeometry + 5);
	  y = (double *) (mGeometry + 5 + sizeof(double));

	  mGeos = geosGeometryFactory->createPoint(GEOS_GEOM::Coordinate(*x,*y));
	  mDirtyGeos = FALSE;
	  break;
	}
	
      case QGis::WKBMultiPoint25D:
	hasZValue = true;
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
	      if(hasZValue)
		{
		  ptr += sizeof(double);
		}
	      points->push_back(geosGeometryFactory->createPoint(GEOS_GEOM::Coordinate(*x,*y)));
	    }
	  mGeos = geosGeometryFactory->createMultiPoint(points);
	  mDirtyGeos = FALSE;
	  break;
	}
	
      case QGis::WKBLineString25D:
	hasZValue = true;
      case QGis::WKBLineString:
	{
	  QgsDebugMsg("QgsGeometry::geosGeometry: Linestring found");
	  
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
	      if(hasZValue)
		{
		  ptr += sizeof(double);
		}
	      sequence->add(GEOS_GEOM::Coordinate(*x,*y));
	    }
	  mDirtyGeos = FALSE;
	  mGeos = geosGeometryFactory->createLineString(sequence); 
	  break;
	}

      case QGis::WKBMultiLineString25D:
	hasZValue = true;
      case QGis::WKBMultiLineString:
	{
	  std::vector<GEOS_GEOM::Geometry*>* lines=new std::vector<GEOS_GEOM::Geometry*>;
	  numLineStrings = (int) (mGeometry[5]);
	  ptr = (mGeometry + 9);
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
		  if(hasZValue)
		    {
		      ptr += sizeof(double);
		    }
		  sequence->add(GEOS_GEOM::Coordinate(*x,*y));
		}
	      lines->push_back(geosGeometryFactory->createLineString(sequence));
	    }
	  mGeos = geosGeometryFactory->createMultiLineString(lines);
	  mDirtyGeos = FALSE;
	  break;
	}
	
      case QGis::WKBPolygon25D:
	hasZValue = true;
      case QGis::WKBPolygon: 
	{
	  QgsDebugMsg("Polygon found");
	  
	  // get number of rings in the polygon
	  numRings = (int *) (mGeometry + 1 + sizeof(int));
	  ptr = mGeometry + 1 + 2 * sizeof(int);
	  
	  GEOS_GEOM::LinearRing* outer=0;
	  std::vector<GEOS_GEOM::Geometry*>* inner=new std::vector<GEOS_GEOM::Geometry*>;
	  
	  for (idx = 0; idx < *numRings; idx++)
	    {
	      
	      //QgsDebugMsg("Ring nr: "+QString::number(idx));
	      
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
	      if(hasZValue)
		{
		  ptr += sizeof(double);
		}
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
	  mGeos = geosGeometryFactory->createPolygon(outer,inner);
	  mDirtyGeos = FALSE;
	  break;
	}
	
      case QGis::WKBMultiPolygon25D:
	hasZValue = true;
      case QGis::WKBMultiPolygon:
	{
	  QgsDebugMsg("Multipolygon found");
	  
	  std::vector<GEOS_GEOM::Geometry *> *polygons=new std::vector<GEOS_GEOM::Geometry *>;
	  // get the number of polygons
	  ptr = mGeometry + 5;
	  numPolygons = (int *) ptr;
	  ptr = mGeometry +9;
	  for (kdx = 0; kdx < *numPolygons; kdx++)
	    {
	      
	      //QgsDebugMsg("Polygon nr: "+QString::number(kdx));
	      
	      GEOS_GEOM::LinearRing* outer=0;
	      std::vector<GEOS_GEOM::Geometry*>* inner=new std::vector<GEOS_GEOM::Geometry*>;
	      
	      //skip the endian and mGeometry type info and
	      // get number of rings in the polygon
	      ptr += 5;
	      numRings = (int *) ptr;
	      ptr += 4;
	      for (idx = 0; idx < *numRings; idx++)
		{
		  //QgsDebugMsg("Ring nr: "+QString::number(idx));
		  
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
		      if(hasZValue)
			{
			  ptr += sizeof(double);
			}
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
	  mGeos = geosGeometryFactory->createMultiPolygon(polygons);
	  mDirtyGeos = FALSE;
	  break;
	}
	
      default:
	return FALSE;
      }
  }
  catch(GEOS_UTIL::GEOSException* e)
    {
      delete e;
      return FALSE;
    }
  return TRUE;
}


bool QgsGeometry::exportGeosToWkb()
{
  //QgsDebugMsg("QgsGeometry::exportGeosToWkb: entered.");

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
    mDirtyWkb = FALSE;
    return TRUE;
  }

  // set up byteOrder
  char byteOrder = QgsApplication::endian();

  switch (mGeos->getGeometryTypeId())
  {
    case GEOS_GEOM::GEOS_POINT:                 // a point
      {
        mGeometrySize = 1 +   // sizeof(byte)
          4 +   // sizeof(uint32)
          2*sizeof(double);
        mGeometry = new unsigned char[mGeometrySize];

        // assign byteOrder
        memcpy(mGeometry, &byteOrder, 1);

        // assign wkbType
        int wkbType = QGis::WKBPoint;
        memcpy(mGeometry+1, &wkbType, 4);

        GEOS_GEOM::Point* pt = static_cast<GEOS_GEOM::Point*>(mGeos);
        double x = pt->getX();
        double y = pt->getY();

        memcpy(mGeometry+5, &x, sizeof(double));
        memcpy(mGeometry+13, &y, sizeof(double));

        break;
      } // case GEOS_GEOM::GEOS_POINT

    case GEOS_GEOM::GEOS_LINESTRING:            // a linestring
      {
        //QgsDebugMsg("Got a geos::GEOS_LINESTRING.");

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
          for(GEOS_SIZE_T i = 0; i < thePolygon->getNumInteriorRing(); ++i)
          {
            geometrySize += sizeof(int); //number of points in ring
            theRing = thePolygon->getInteriorRingN(i);
            if(theRing)
            {
              geometrySize += theRing->getNumPoints()*2*sizeof(double);
            }
          }

          mGeometry = new unsigned char[geometrySize];
	  mGeometrySize = geometrySize;

          //then fill the geometry itself into the wkb
          int position = 0;
          // assign byteOrder
          memcpy(mGeometry, &byteOrder, 1);
          position += 1;
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
            for(GEOS_SIZE_T j = 0; j < ringSequence->getSize(); ++j)
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
          for(GEOS_SIZE_T i = 0; i < thePolygon->getNumInteriorRing(); ++i)
          {
            theRing = thePolygon->getInteriorRingN(i);
            if(theRing)
            {
	      const GEOS_GEOM::CoordinateSequence* ringSequence = theRing->getCoordinatesRO();
              nPointsInRing = ringSequence->getSize();
              memcpy(&mGeometry[position], &nPointsInRing, sizeof(int));
              position += sizeof(int);
              for(int j = 0; j < nPointsInRing; ++j)
              {
		x = ringSequence->getAt(j).x;
                memcpy(&mGeometry[position], &x, sizeof(double));
                position += sizeof(double);
		y = ringSequence->getAt(j).y;
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
	GEOS_GEOM::MultiLineString* theMultiLineString = dynamic_cast<GEOS_GEOM::MultiLineString*>(mGeos);
	if(!theMultiLineString)
	  {
	    return false;
	  }
	
	//find out size of geometry
	int geometrySize = 1 + 2 * sizeof(int);
	for(GEOS_SIZE_T i = 0; i < theMultiLineString->getNumGeometries(); ++i)
	  {
	    geometrySize += (1 + 2 * sizeof(int));
	    geometrySize += theMultiLineString->getGeometryN(i)->getNumPoints() * 2 * sizeof(double);
	  }

	mGeometry = new unsigned char[geometrySize];
	mGeometrySize = geometrySize;
	int wkbPosition = 0; //current position in the byte array
	
	memcpy(mGeometry, &byteOrder, 1);
	wkbPosition += 1;
	int wkbtype=QGis::WKBMultiLineString;
	memcpy(&mGeometry[wkbPosition],&wkbtype, sizeof(int));
	wkbPosition += sizeof(int);
	int numLines = theMultiLineString->getNumGeometries();
	memcpy(&mGeometry[wkbPosition], &numLines, sizeof(int));
	wkbPosition += sizeof(int);

	//loop over lines
	int lineType = QGis::WKBLineString;
	GEOS_GEOM::CoordinateSequence* lineCoordinates = 0;
	int lineSize;
	double x, y;

	for(GEOS_SIZE_T i = 0; i < theMultiLineString->getNumGeometries(); ++i)
	  {
	    //endian and type WKBLineString
	    memcpy(&mGeometry[wkbPosition], &byteOrder, 1);
	    wkbPosition += 1;
	    memcpy(&mGeometry[wkbPosition], &lineType, sizeof(int));
	    wkbPosition += sizeof(int);
	    lineCoordinates = theMultiLineString->getGeometryN(i)->getCoordinates();
	    
	    //line size
	    lineSize = lineCoordinates->getSize();
	    memcpy(&mGeometry[wkbPosition], &lineSize, sizeof(int));
	    wkbPosition += sizeof(int);
	    
	    //vertex coordinates
	    for(GEOS_SIZE_T j = 0; j < lineSize; ++j)
	      {
		GEOS_GEOM::Coordinate c = lineCoordinates->getAt(j);
		x = c.x; y = c.y;
		memcpy(&mGeometry[wkbPosition], &x, sizeof(double));
		wkbPosition += sizeof(double);
		memcpy(&mGeometry[wkbPosition], &y, sizeof(double));
		wkbPosition += sizeof(double);
	      }
	    delete lineCoordinates;
	  }
	mDirtyWkb = FALSE;
	return true;
      } // case GEOS_GEOM::GEOS_MULTILINESTRING

    case GEOS_GEOM::GEOS_MULTIPOLYGON:          // a collection of polygons
      {
	GEOS_GEOM::MultiPolygon* theMultiPolygon = dynamic_cast<GEOS_GEOM::MultiPolygon*>(mGeos);
	if(!theMultiPolygon)
	  {
	    return false;
	  }

	//first determine size of geometry
        int geometrySize = 1 + (2 * sizeof(int)); //endian, type, number of polygons
	for(GEOS_SIZE_T i = 0; i < theMultiPolygon->getNumGeometries(); ++i)
	  {
	    GEOS_GEOM::Polygon* thePoly = (GEOS_GEOM::Polygon*)(theMultiPolygon->getGeometryN(i));
	    geometrySize += (1 + (2 * sizeof(int))); //endian, type, number of rings
	    //exterior ring
	    geometrySize += sizeof(int); //number of points in exterior ring
	    const GEOS_GEOM::LineString* exRing = thePoly->getExteriorRing();
	    geometrySize += (2*sizeof(double)*exRing->getNumPoints());

	    const GEOS_GEOM::LineString* intRing = 0;
	    for(GEOS_SIZE_T j = 0; j < thePoly->getNumInteriorRing(); ++j)
	      {
		geometrySize += sizeof(int); //number of points in ring
		intRing = thePoly->getInteriorRingN(j);
		geometrySize += 2*sizeof(double)*intRing->getNumPoints();
	      }
	  }

	mGeometry = new unsigned char[geometrySize];
	mGeometrySize = geometrySize;
	int wkbPosition = 0; //current position in the byte array
        
	memcpy(mGeometry, &byteOrder, 1);
	wkbPosition += 1;
	int wkbtype=QGis::WKBMultiPolygon;
	memcpy(&mGeometry[wkbPosition],&wkbtype, sizeof(int));
	wkbPosition += sizeof(int);
	int numPolygons = theMultiPolygon->getNumGeometries();
	memcpy(&mGeometry[wkbPosition], &numPolygons, sizeof(int));
	wkbPosition += sizeof(int);

	//loop over polygons
	for(GEOS_SIZE_T i = 0; i < theMultiPolygon->getNumGeometries(); ++i)
	  {
	    GEOS_GEOM::Polygon* thePoly = (GEOS_GEOM::Polygon*)(theMultiPolygon->getGeometryN(i));
	    memcpy(&mGeometry[wkbPosition], &byteOrder, 1);
	    wkbPosition += 1;
	    int polygonType = QGis::WKBPolygon;
	    memcpy(&mGeometry[wkbPosition], &polygonType, sizeof(int));
	    wkbPosition += sizeof(int);
	    int numRings = thePoly->getNumInteriorRing() + 1;
	    memcpy(&mGeometry[wkbPosition], &numRings, sizeof(int));
	    wkbPosition += sizeof(int);

	    //exterior ring
	    const GEOS_GEOM::LineString* theRing = thePoly->getExteriorRing();
	    int nPointsInRing = theRing->getNumPoints();
	    memcpy(&mGeometry[wkbPosition], &nPointsInRing, sizeof(int));
	    wkbPosition += sizeof(int);
	    const GEOS_GEOM::CoordinateSequence* ringCoordinates = theRing->getCoordinatesRO();
	    
	    double x, y;
	    GEOS_GEOM::Coordinate coord;
	    for(int k = 0; k < nPointsInRing; ++k)
	      {
		coord = ringCoordinates->getAt(k);
		x = coord.x;
		y = coord.y;
		memcpy(&mGeometry[wkbPosition], &x, sizeof(double));
		wkbPosition += sizeof(double);
		memcpy(&mGeometry[wkbPosition], &y, sizeof(double));
		wkbPosition += sizeof(double);
	      }
	    
	    //interior rings
	    for(GEOS_SIZE_T j = 0; j < thePoly->getNumInteriorRing(); ++j)
	      {
		theRing = thePoly->getInteriorRingN(j);
		nPointsInRing = theRing->getNumPoints();
		memcpy(&mGeometry[wkbPosition], &nPointsInRing, sizeof(int));
		wkbPosition += sizeof(int);
		ringCoordinates = theRing->getCoordinatesRO();

		for(int k = 0; k < nPointsInRing; ++k)
		  {
		    coord = ringCoordinates->getAt(k);
		    x = coord.x;
		    y = coord.y;
		    memcpy(&mGeometry[wkbPosition], &x, sizeof(double));
		    wkbPosition += sizeof(double);
		    memcpy(&mGeometry[wkbPosition], &y, sizeof(double));
		    wkbPosition += sizeof(double);
		  }
	      }
	  }
	mDirtyWkb = FALSE;
	return true;	
      } // case GEOS_GEOM::GEOS_MULTIPOLYGON

    case GEOS_GEOM::GEOS_GEOMETRYCOLLECTION:    // a collection of heterogeneus geometries
      {
        // TODO
        break;
      } // case GEOS_GEOM::GEOS_GEOMETRYCOLLECTION

  } // switch (mGeos->getGeometryTypeId())

  return FALSE;
}




double QgsGeometry::distanceSquaredPointToSegment(const QgsPoint& point,
    double *x1, double *y1,
    double *x2, double *y2,
    QgsPoint& minDistPoint)
{

  double nx, ny; //normal vector

  nx = *y2 - *y1;
  ny = -(*x2 - *x1);

  double t;
  t = (point.x() * ny - point.y() * nx - *x1 * ny + *y1 * nx) / ((*x2 - *x1) * ny - (*y2 - *y1) * nx);

  if (t < 0.0)
    { 
      minDistPoint.setX(*x1); 
      minDistPoint.setY(*y1);
    }
  else if (t > 1.0)
    {
      minDistPoint.setX(*x2); 
      minDistPoint.setY(*y2);
    }
  else
    { 
      minDistPoint.setX(*x1 + t * ( *x2 - *x1 ));
      minDistPoint.setY(*y1 + t * ( *y2 - *y1 )); 
    }

  return (minDistPoint.sqrDist(point));
#if 0
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
#endif //0
}

bool QgsGeometry::convertToMultiType()
{
  if(!mGeometry)
    {
      return false;
    }

  QGis::WKBTYPE geomType = wkbType();
  
  if(geomType == QGis::WKBMultiPoint || geomType == QGis::WKBMultiPoint25D || \
     geomType == QGis::WKBMultiLineString || geomType == QGis::WKBMultiLineString25D || \
     geomType == QGis::WKBMultiPolygon || geomType == QGis::WKBMultiPolygon25D || geomType == QGis::WKBUnknown)
    {
      return false; //no need to convert
    }

  int newGeomSize = mGeometrySize + 1 + 2 * sizeof(int); //endian: 1, multitype: sizeof(int), number of geometries: sizeof(int) 
  unsigned char* newGeometry = new unsigned char[newGeomSize];

  int currentWkbPosition = 0;
  //copy endian
  char byteOrder = QgsApplication::endian();
  memcpy(&newGeometry[currentWkbPosition], &byteOrder, 1);
  currentWkbPosition += 1;

  //copy wkbtype
  //todo
  QGis::WKBTYPE newMultiType;
  switch(geomType)
    {
    case QGis::WKBPoint:
      newMultiType = QGis::WKBMultiPoint;
      break;
    case QGis::WKBPoint25D:
      newMultiType = QGis::WKBMultiPoint25D;
      break;
    case QGis::WKBLineString:
      newMultiType = QGis::WKBMultiLineString;
      break;
    case QGis::WKBLineString25D:
      newMultiType = QGis::WKBMultiLineString25D;
      break;
    case QGis::WKBPolygon:
      newMultiType = QGis::WKBMultiPolygon;
      break;
    case QGis::WKBPolygon25D:
      newMultiType = QGis::WKBMultiPolygon25D;
      break;
    default:
      delete newGeometry;
      return false;
    }
  memcpy(&newGeometry[currentWkbPosition], &newMultiType, sizeof(int));
  currentWkbPosition += sizeof(int);

  //copy number of geometries
  int nGeometries = 1;
  memcpy(&newGeometry[currentWkbPosition], &nGeometries, sizeof(int));
  currentWkbPosition += sizeof(int);

  //copy the existing single geometry
  memcpy(&newGeometry[currentWkbPosition], mGeometry, mGeometrySize);

  delete mGeometry;
  mGeometry = newGeometry;
  mGeometrySize = newGeomSize;
  mDirtyGeos = true;
  return true;
}

void QgsGeometry::translateVertex(int& wkbPosition, double dx, double dy, bool hasZValue)
{
  double x, y, translated_x, translated_y;

  //x-coordinate
  x = *((double *) (&(mGeometry[wkbPosition])));
  translated_x = x + dx;
  memcpy(&(mGeometry[wkbPosition]), &translated_x, sizeof(double)); 
  wkbPosition += sizeof(double);
  
  //y-coordinate
  y = *((double *) (&(mGeometry[wkbPosition])));
  translated_y = y + dy;
  memcpy(&(mGeometry[wkbPosition]), &translated_y, sizeof(double));
  wkbPosition += sizeof(double);

  if(hasZValue)
    {
      wkbPosition += sizeof(double);
    }
}

int QgsGeometry::splitLinearGeometry(GEOS_GEOM::LineString* splitLine, QList<QgsGeometry*>& newGeometries)
{
  if(!splitLine)
    {
      return 2;
    }

  if(!mGeos || mDirtyGeos)
    {
      exportWkbToGeos();
    }

 //first test if linestring intersects geometry. If not, return straight away
  if(!splitLine->intersects(mGeos))
    {
      return 1;
    }

  //todo: use the geos polygonizer
  GEOS_LINEMERGE::LineMerger lineMerger;
  
  //first union all the polygon rings together (to get them noded, see JTS developer guide)
  GEOS_GEOM::Geometry* nodedGeometry = nodeGeometries(splitLine, mGeos);
  if(!nodedGeometry)
    {
      return 3; //an error occured during noding
    } 

  lineMerger.add(nodedGeometry);

  std::vector<GEOS_GEOM::LineString*>* mergedLineStrings = lineMerger.getMergedLineStrings();
  if(!mergedLineStrings)
    {
      delete nodedGeometry;
      return 4;
    }

  QList<GEOS_GEOM::Geometry*> testedGeometries;
  GEOS_GEOM::Geometry* intersectGeom = 0;

  for(unsigned int i = 0; i < mergedLineStrings->size(); ++i)
    {
      intersectGeom = mGeos->intersection((*mergedLineStrings)[i]);
      if(intersectGeom->getLength() > 0.00000001)
	{
	  testedGeometries.push_back((*mergedLineStrings)[i]);
	}
      else
	{
	  delete (*mergedLineStrings)[i];
	}
      delete intersectGeom;
    }

  mergeGeometriesMultiTypeSplit(testedGeometries);

  if(testedGeometries.size() > 0)
    {
      delete mGeos;
      mGeos = testedGeometries.at(0);
      mDirtyWkb = true;
    }

  for(int i = 1; i < testedGeometries.size(); ++i)
    {
      QgsGeometry* newQgisGeometry = new QgsGeometry();
      newQgisGeometry->setGeos(testedGeometries[i]);
      newGeometries.push_back(newQgisGeometry);
    }

  delete nodedGeometry;
  delete mergedLineStrings;
  return 0;
}

int QgsGeometry::splitPolygonGeometry(GEOS_GEOM::LineString* splitLine, QList<QgsGeometry*>& newGeometries)
{
  if(!splitLine)
    {
      return 2;
    }

  if(!mGeos || mDirtyGeos)
    {
      exportWkbToGeos();
    }

  //first test if linestring intersects geometry. If not, return straight away
  if(!splitLine->intersects(mGeos))
    {
      return 1;
    }

  //todo: use the geos polygonizer
  GEOS_POLYGONIZE::Polygonizer polygonizer;
  
  //first union all the polygon rings together (to get them noded, see JTS developer guide)
  GEOS_GEOM::Geometry* nodedGeometry = nodeGeometries(splitLine, mGeos);
  if(!nodedGeometry)
    {
      return 2; //an error occured during noding
    }

  //add to polygonizer
  polygonizer.add(nodedGeometry);

  //test for cut edge and abort split if they are present
  std::vector<const GEOS_GEOM::LineString*>* cutEdges = polygonizer.getCutEdges();
  if(cutEdges && cutEdges->size() > 0)
    {
      return 3;
    }

  std::vector<GEOS_GEOM::Polygon*>* polygons = polygonizer.getPolygons();
  if(!polygons)
    {
      delete nodedGeometry;
      return 4;
    }

  //test every polygon if contained in original geometry
  //include in result if yes
  QList<GEOS_GEOM::Geometry*> testedGeometries;
  GEOS_GEOM::Geometry* intersectGeometry = 0;
  for(unsigned int i = 0; i < polygons->size(); ++i)
    {
      intersectGeometry = mGeos->intersection((*polygons)[i]);
      if(intersectGeometry->getArea() > 0.00000000001)
	{
	  testedGeometries.push_back((*polygons)[i]);
	}
      else
	{
	  delete (*polygons)[i];
	}
      delete intersectGeometry;
    }

  mergeGeometriesMultiTypeSplit(testedGeometries);

  if(testedGeometries.size() > 0)
    {
      delete mGeos;
      mGeos = testedGeometries.at(0);
      mDirtyWkb = true;
    }

  for(int i = 1; i < testedGeometries.size(); ++i)
    {
      QgsGeometry* newQgisGeometry = new QgsGeometry();
      newQgisGeometry->setGeos(testedGeometries[i]);
      newGeometries.push_back(newQgisGeometry);
    }
  
  delete nodedGeometry;
  delete polygons;
  return 0;
}

GEOS_GEOM::Geometry* QgsGeometry::nodeGeometries(const GEOS_GEOM::LineString* splitLine, const GEOS_GEOM::Geometry* geom) const
{
  if(!splitLine || !geom)
    {
      return 0;
    }

  const GEOS_GEOM::Geometry* geometryBoundary = 0;
  bool deleteBoundary = false;
  if(geom->getGeometryTypeId() == GEOS_GEOM::GEOS_POLYGON || geom->getGeometryTypeId() == GEOS_GEOM::GEOS_MULTIPOLYGON)
    {
      geometryBoundary = geom->getBoundary();
      deleteBoundary = true;
    }
  else
    {
      geometryBoundary = geom;
    }

  GEOS_GEOM::Geometry* splitLineClone = splitLine->clone();
  GEOS_GEOM::Geometry* unionGeometry = splitLineClone->Union(geometryBoundary);
 
  delete splitLineClone;
  if(deleteBoundary)
    {
      delete geometryBoundary;
    }
 
  return unionGeometry;
}

int QgsGeometry::mergeGeometriesMultiTypeSplit(QList<GEOS_GEOM::Geometry*>& splitResult)
{
  if(!mGeos || mDirtyGeos)
    {
      exportWkbToGeos();
    }

  //convert mGeos to geometry collection
  GEOS_GEOM::GeometryCollection* collection = dynamic_cast<GEOS_GEOM::GeometryCollection*>(mGeos);
  if(!collection)
    {
      return 0;
    }
  
  QList<GEOS_GEOM::Geometry*> copyList = splitResult;
  splitResult.clear();

  //collect all the geometries that belong to the initial multifeature
  std::vector<GEOS_GEOM::Geometry*>* unionGeom = new std::vector<GEOS_GEOM::Geometry*>();

  for(int i = 0; i < copyList.size(); ++i)
    {
      //is this geometry a part of the original multitype?
      bool isPart = false;
      for(int j = 0; j < collection->getNumGeometries(); ++j)
	{
	  if(copyList.at(i)->equals(collection->getGeometryN(j)))
	    {
	      isPart = true;
	      break;
	    }
	}

      if(isPart)
	{
	  unionGeom->push_back(copyList.at(i));
	}
      else
	{
	  std::vector<GEOS_GEOM::Geometry*>* geomVector = new std::vector<GEOS_GEOM::Geometry*>();
	  geomVector->push_back(copyList.at(i));
	  if(mGeos->getGeometryTypeId() == GEOS_GEOM::GEOS_MULTILINESTRING)
	    {
	      splitResult.push_back(geosGeometryFactory->createMultiLineString(geomVector));
	    }
	  else if(mGeos->getGeometryTypeId() == GEOS_GEOM::GEOS_MULTIPOLYGON)
	    {
	      splitResult.push_back(geosGeometryFactory->createMultiPolygon(geomVector));
	    }
	  else
	    {
	      delete copyList.at(i);
	      delete geomVector;
	    }
	}
    }

  //make multifeature out of unionGeom
  if(unionGeom->size() > 0)
    {
      if(mGeos->getGeometryTypeId() == GEOS_GEOM::GEOS_MULTILINESTRING)
	{
	  splitResult.push_front(geosGeometryFactory->createMultiLineString(unionGeom));
	}
      else if(mGeos->getGeometryTypeId() == GEOS_GEOM::GEOS_MULTIPOLYGON)
	{
	  splitResult.push_front(geosGeometryFactory->createMultiPolygon(unionGeom));
	}
    }
  else
    {
      delete unionGeom;
    }

  return 0;
}

QgsPoint QgsGeometry::asPoint(unsigned char*& ptr, bool hasZValue)
{
  ptr += 5;
  double* x = (double *) (ptr);
  double* y = (double *) (ptr + sizeof(double));
  ptr += 2 * sizeof(double);

  if (hasZValue)
    ptr += sizeof(double);

  return QgsPoint(*x,*y);
}


QgsPolyline QgsGeometry::asPolyline(unsigned char*& ptr, bool hasZValue)
{
  double x,y;
  ptr += 5;
  unsigned int nPoints = *((int*)ptr);
  ptr += 4;

  QgsPolyline line(nPoints);

  // Extract the points from the WKB format into the x and y vectors. 
  for (uint i = 0; i < nPoints; ++i)
  {
    x = *((double *) ptr);
    y = *((double *) (ptr + sizeof(double)));

    ptr += 2 * sizeof(double);

    line[i] = QgsPoint(x,y);

    if (hasZValue) // ignore Z value
      ptr += sizeof(double);
  }

  return line;
}


QgsPolygon QgsGeometry::asPolygon(unsigned char*& ptr, bool hasZValue)
{
  double x,y;

  ptr += 5;

  // get number of rings in the polygon
  unsigned int numRings = *((int*)ptr);
  ptr += 4;

  if ( numRings == 0 )  // sanity check for zero rings in polygon
    return QgsPolygon();

  QgsPolygon rings(numRings);

  for (uint idx = 0; idx < numRings; idx++)
  {
    uint nPoints = *((int*)ptr);
    ptr += 4;

    QgsPolyline ring(nPoints);

    for (uint jdx = 0; jdx < nPoints; jdx++)
    {
      x = *((double *) ptr);
      y = *((double *) (ptr + sizeof(double)));

      ptr += 2 * sizeof(double);

      if (hasZValue)
        ptr += sizeof(double);

      ring[jdx] = QgsPoint(x,y);
    }

    rings[idx] = ring;
  }

  return rings;
}


QgsPoint QgsGeometry::asPoint()
{
  QGis::WKBTYPE type = wkbType();
  if (type != QGis::WKBPoint && type != QGis::WKBPoint25D)
    return QgsPoint(0,0);

  unsigned char* ptr = mGeometry;
  return asPoint(ptr, type == QGis::WKBPoint25D);
}

QgsPolyline QgsGeometry::asPolyline()
{
  QGis::WKBTYPE type = wkbType();
  if (type != QGis::WKBLineString && type != QGis::WKBLineString25D)
    return QgsPolyline();

  unsigned char *ptr = mGeometry;
  return asPolyline(ptr, type == QGis::WKBLineString25D);
}

QgsPolygon QgsGeometry::asPolygon()
{
  QGis::WKBTYPE type = wkbType();
  if (type != QGis::WKBPolygon && type != QGis::WKBPolygon25D)
    return QgsPolygon();

  unsigned char *ptr = mGeometry;
  return asPolygon(ptr, type == QGis::WKBPolygon25D);
}

QgsMultiPoint QgsGeometry::asMultiPoint()
{
  QGis::WKBTYPE type = wkbType();
  if (type != QGis::WKBMultiPoint && type != QGis::WKBMultiPoint25D)
    return QgsMultiPoint();

  bool hasZValue = (type == QGis::WKBMultiPoint25D);

  unsigned char* ptr = mGeometry + 5;
  unsigned int nPoints = *((int*)ptr);
  ptr += 4;

  QgsMultiPoint points(nPoints);
  for (uint i = 0; i < nPoints; i++)
  {
    points[i] = asPoint(ptr, hasZValue);
  }

  return points;
}

QgsMultiPolyline QgsGeometry::asMultiPolyline()
{
  QGis::WKBTYPE type = wkbType();
  if (type != QGis::WKBMultiLineString && type != QGis::WKBMultiLineString25D)
    return QgsMultiPolyline();

  bool hasZValue = (type == QGis::WKBMultiLineString25D);

  unsigned char* ptr = mGeometry + 5;
  unsigned int numLineStrings = *((int*)ptr);
  ptr += 4;

  QgsMultiPolyline lines(numLineStrings);

  for (uint i = 0; i < numLineStrings; i++)
  {
    lines[i] = asPolyline(ptr, hasZValue);
  }

  return lines;
}

QgsMultiPolygon QgsGeometry::asMultiPolygon()
{
  QGis::WKBTYPE type = wkbType();
  if (type != QGis::WKBMultiPolygon && type != QGis::WKBMultiPolygon25D)
    return QgsMultiPolygon();

  bool hasZValue = (type == QGis::WKBMultiPolygon25D);

  unsigned char* ptr = mGeometry + 5;
  unsigned int numPolygons = *((int*)ptr);
  ptr += 4;

  QgsMultiPolygon polygons(numPolygons);

  for (uint i = 0; i < numPolygons; i++)
  {
    polygons[i] = asPolygon(ptr, hasZValue);
  }

  return polygons;
}


double QgsGeometry::distance(QgsGeometry& geom)
{
  if (mGeos == NULL)
  {
    exportWkbToGeos();
  }

  if (geom.mGeos == NULL)
  {
    geom.exportWkbToGeos();
  }

  return mGeos->distance(geom.mGeos);
}


QgsGeometry* QgsGeometry::buffer(double distance, int segments)
{
  if (mGeos == NULL)
    exportWkbToGeos();
  GEOS_GEOM::Geometry* geos = mGeos->buffer(distance, segments);
  QgsGeometry* g = new QgsGeometry;
  g->setGeos(geos);
  return g;
}

QgsGeometry* QgsGeometry::convexHull()
{
  if (mGeos == NULL)
    exportWkbToGeos();
  GEOS_GEOM::Geometry* geos = mGeos->convexHull();
  QgsGeometry* g = new QgsGeometry;
  g->setGeos(geos);
  return g;
}

QgsGeometry* QgsGeometry::intersection(QgsGeometry* geometry)
{
  if (geometry == NULL)
    return NULL;
  if (mGeos == NULL)
    exportWkbToGeos();
  if (geometry->mGeos == NULL)
    geometry->exportWkbToGeos();
  GEOS_GEOM::Geometry* geos = mGeos->intersection(geometry->mGeos);
  QgsGeometry* g = new QgsGeometry;
  g->setGeos(geos);
  return g;
}

QgsGeometry* QgsGeometry::Union(QgsGeometry* geometry)
{
  if (geometry == NULL)
    return NULL;
  if (mGeos == NULL)
    exportWkbToGeos();
  if (geometry->mGeos == NULL)
    geometry->exportWkbToGeos();
  GEOS_GEOM::Geometry* geos = mGeos->Union(geometry->mGeos);
  QgsGeometry* g = new QgsGeometry;
  g->setGeos(geos);
  return g;
}

QgsGeometry* QgsGeometry::difference(QgsGeometry* geometry)
{
  if (geometry == NULL)
    return NULL;
  if (mGeos == NULL)
    exportWkbToGeos();
  if (geometry->mGeos == NULL)
    geometry->exportWkbToGeos();
  GEOS_GEOM::Geometry* geos = mGeos->difference(geometry->mGeos);
  QgsGeometry* g = new QgsGeometry;
  g->setGeos(geos);
  return g;
}

QgsGeometry* QgsGeometry::symDifference(QgsGeometry* geometry)
{
  if (geometry == NULL)
    return NULL;
  if (mGeos == NULL)
    exportWkbToGeos();
  if (geometry->mGeos == NULL)
    geometry->exportWkbToGeos();
  GEOS_GEOM::Geometry* geos = mGeos->symDifference(geometry->mGeos);
  QgsGeometry* g = new QgsGeometry;
  g->setGeos(geos);
  return g;
}
