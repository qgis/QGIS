/***************************************************************************
                qgsfeature.cpp - Spatial Feature Implementation
                     --------------------------------------
Date                 : 09-Sep-2003
Copyright            : (C) 2003 by Gary E.Sherman
email                : sherman at mrcc.com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
/* $Id$ */

#include "qgsfeature.h"
#include "qgsattributedialog.h"
#include "qgsrect.h"
#include <geos.h>
#include <iostream>
#include <cfloat>

#include <cstring>

/** \class QgsFeature
 * \brief Encapsulates a spatial feature with attributes
 */
//! Constructor
QgsFeature::QgsFeature()
    : mFid(0), geometry(0), geometrySize(0)
{
}


QgsFeature::QgsFeature(int id, QString const & typeName )
    : mFid(id), geometry(0), geometrySize(0), mTypeName(typeName)
{
}


QgsFeature::QgsFeature( QgsFeature const & feature )
    : mFid( feature.mFid ), 
      attributes( feature.attributes ),
      fieldNames( feature.fieldNames ),
      mWKT( feature.mWKT ),
      mValid( feature.mValid ),
      geometrySize( feature.geometrySize ),
      mTypeName( feature.mTypeName )
{
    geometry = 0;

    if ( geometrySize && feature.geometry )
    {
        geometry = new unsigned char[geometrySize];

        memcpy( geometry, feature.geometry, geometrySize );
    }
}


QgsFeature & QgsFeature::operator=( QgsFeature const & feature )
{
    if ( &feature == this )
    { return *this; }

    mFid =  feature.mFid ; 
    attributes =  feature.attributes ;
    fieldNames =  feature.fieldNames ;
    mWKT =  feature.mWKT ;
    mValid =  feature.mValid ;
    geometrySize =  feature.geometrySize;
    mTypeName = feature.mTypeName;

    if ( geometry )
    {
        delete [] geometry;
    }

    geometry = 0;

    if ( geometrySize && feature.geometry )
    {
        geometry = new unsigned char[geometrySize];

        memcpy( geometry, feature.geometry, geometrySize );
    }

    return *this;
} // QgsFeature::operator=( QgsFeature const & rhs )



//! Destructor
QgsFeature::~QgsFeature()
{
#ifdef QGISDEBUG
  // disabled this - it causes too much output!!
//  std::cerr << "In QgsFeature destructor" << std::endl;
#endif
  if (geometry)
  {
      delete [] geometry;
  }
}

/**
 * Get the feature id for this feature
 * @return Feature id
 */
int QgsFeature::featureId() const
{
  return mFid;
}

/**
 * Get the attributes for this feature.
 * @return A std::map containing the field name/value mapping
 */
const std::vector < QgsFeatureAttribute > &QgsFeature::attributeMap()
{
  return attributes;
}

/**
 * Add an attribute to the map
 */
void QgsFeature::addAttribute(QString const&  field, QString const & value)
{
  attributes.push_back(QgsFeatureAttribute(field, value));
}

/**
 * Get the fields for this feature
 * @return A std::map containing field position (index) and field name
 */
const std::map < int, QString > &QgsFeature::fields()
{
  return fieldNames;
}

/**
 * Get the pointer to the feature geometry
 */
unsigned char * QgsFeature::getGeometry() const
{
  return geometry;
}


size_t QgsFeature::getGeometrySize() const
{
    return geometrySize;
} //  QgsFeauture::getGeometrySize() const


/**
 * Return well known text representation of this feature
 */
QString const & QgsFeature::wellKnownText() const
{
    if(mWKT.isNull())
    {
	exportToWKT();
    }
    return mWKT;
}

/** Set the feature id
*/
void QgsFeature::setFeatureId(int id)
{
  mFid = id;

}


QString const & QgsFeature::typeName() const
{
  return mTypeName;
} // QgsFeature::typeName



/** sets the feature's type name
 */
void QgsFeature::typeName( QString const & typeName )
{
    mTypeName = typeName;
} // QgsFeature::typeName



/** Set the pointer to the feature geometry
*/
void QgsFeature::setGeometry(unsigned char *geom, size_t size)
{
    // delete any existing binary WKT geometry before assigning new one
    if ( geometry )
    {
        delete [] geometry;
    }

    geometry = geom;
    geometrySize = size;
}

bool QgsFeature::isValid() const
{
  return mValid;
}

void QgsFeature::setValid(bool validity)
{
  mValid = validity;
}

void QgsFeature::attributeDialog()
{
    QgsAttributeDialog attdialog(&attributes);
    if(attdialog.exec()==QDialog::Accepted)
    {
	for(int i=0;i<attributes.size();++i)
	{
	    attributes[i].setFieldValue(attdialog.value(i));
	}
    }
}

bool QgsFeature::intersects(QgsRect* r)
{
    bool returnval=false;

    geos::GeometryFactory *gf = new geos::GeometryFactory();
    geos::WKTReader *wktReader = new geos::WKTReader(gf);
    geos::Geometry *geosGeom = wktReader->read( qstrdup(wellKnownText()) );

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

bool QgsFeature::exportToWKT() const
{
    if(geometry)
    {
	int wkbType;
	double *x,*y;

	mWKT="";
	memcpy(&wkbType, (geometry+1), sizeof(int));
	switch (wkbType)
	{
	    case QGis::WKBPoint:
	    {
		mWKT+="POINT(";
		x = (double *) (geometry + 5);
		mWKT+=QString::number(*x,'f',3);
		mWKT+=" ";
		y = (double *) (geometry + 5 + sizeof(double));
		mWKT+=QString::number(*y,'f',3);
		mWKT+=")";
		break;
	    }
	    case QGis::WKBLineString:
	    {
		unsigned char *ptr;
		int *nPoints;
		int idx;

		mWKT+="LINESTRING(";
		// get number of points in the line
		ptr = geometry + 5;
		nPoints = (int *) ptr;
		ptr = geometry + 1 + 2 * sizeof(int);
		for (idx = 0; idx < *nPoints; ++idx)
		{
		    if(idx!=0)
		    {
			mWKT+=", ";
		    }
		    x = (double *) ptr;
		    mWKT+=QString::number(*x,'f',3);
		    mWKT+=" ";
		    ptr += sizeof(double);
		    y = (double *) ptr;
		    mWKT+=QString::number(*y,'f',3);
		    ptr += sizeof(double);
		}
		mWKT+=")";
		break;
	    }
	    case QGis::WKBPolygon:
	    {
		unsigned char *ptr;
		int idx, jdx;
		int *numRings, *nPoints;

		mWKT+="POLYGON(";
		// get number of rings in the polygon
		numRings = (int *)(geometry + 1 + sizeof(int));
		if (!(*numRings))  // sanity check for zero rings in polygon
		{
		    break;
		}
		int *ringStart; // index of first point for each ring
		int *ringNumPoints; // number of points in each ring
		ringStart = new int[*numRings];
		ringNumPoints = new int[*numRings];
		ptr = geometry+1+2*sizeof(int); // set pointer to the first ring
		for (idx = 0; idx < *numRings; idx++)
		{
		    if(idx!=0)
		    {
			mWKT+=",";
		    }
		    mWKT+="(";
		    // get number of points in the ring
		    nPoints = (int *) ptr;
		    ringNumPoints[idx] = *nPoints;
		    ptr += 4;
		    
		    for(jdx=0;jdx<*nPoints;jdx++)
		    {
			if(jdx!=0)
			{
			    mWKT+=",";
			}
			x = (double *) ptr;
			mWKT+=QString::number(*x,'f',3);
			mWKT+=" ";
			ptr += sizeof(double);
			y = (double *) ptr;
			mWKT+=QString::number(*y,'f',3);
			ptr += sizeof(double);
		    }
		    mWKT+=")";
		}
		mWKT+=")";
		delete [] ringStart;
		delete [] ringNumPoints;
		break;
	    }
	    case QGis::WKBMultiPoint:
	    {
		unsigned char *ptr;
		int idx;
		int *nPoints;

		mWKT+="MULTIPOINT(";
		nPoints=(int*)(geometry+5);
		ptr=geometry+5+sizeof(int);
		for(idx=0;idx<*nPoints;++idx)
		{
		    if(idx!=0)
		    {
			mWKT+=", ";
		    }
		    x = (double *) (ptr);
		    mWKT+=QString::number(*x,'f',3);
		    mWKT+=" ";
		    ptr+=sizeof(double);
		    y= (double *) (ptr);
		    mWKT+=QString::number(*y,'f',3);
		    ptr+=sizeof(double);
		}
		mWKT+=")";
		break;
	    }

	    case QGis::WKBMultiLineString:
	    {
		unsigned char *ptr;
		int idx, jdx, numLineStrings;
		int *nPoints;

		mWKT+="MULTILINESTRING(";
		numLineStrings = (int) (geometry[5]);
		ptr = geometry + 9;
		for (jdx = 0; jdx < numLineStrings; jdx++)
		{
		    if(jdx!=0)
		    {
			mWKT+=", ";
		    }
		    mWKT+="(";
		    ptr += 5; // skip type since we know its 2
		    nPoints = (int *) ptr;
		    ptr += sizeof(int);
		    for (idx = 0; idx < *nPoints; idx++)
		    {
			if(idx!=0)
			{
			    mWKT+=", ";
			}
			x = (double *) ptr;
			mWKT+=QString::number(*x,'f',3);
			ptr += sizeof(double);
			mWKT+=" ";
			y = (double *) ptr;
			mWKT+=QString::number(*y,'f',3);
			ptr += sizeof(double);
		    }
		    mWKT+=")";
		}
		mWKT+=")";
		break;
	    }

	    case QGis::WKBMultiPolygon:
	    {
		unsigned char *ptr;
		int idx, jdx, kdx;
		int *numPolygons, *numRings, *nPoints;
		
		mWKT+="MULTIPOLYGON(";
		ptr = geometry + 5;
		numPolygons = (int *) ptr;
		ptr = geometry + 9;
		for (kdx = 0; kdx < *numPolygons; kdx++)
		{
		    if(kdx!=0)
		    {
			mWKT+=",";
		    }
		    mWKT+="(";
		    ptr+=5;
		    numRings = (int *) ptr;
		    ptr += 4;
		    for (idx = 0; idx < *numRings; idx++)
		    {
			if(idx!=0)
			{
			    mWKT+=",";
			}
			mWKT+="(";
			nPoints = (int *) ptr;
			ptr += 4;
			for (jdx = 0; jdx < *nPoints; jdx++)
			{
			    x = (double *) ptr;
			    mWKT+=QString::number(*x,'f',3);
			    ptr += sizeof(double);
			    mWKT+=" ";
			    y = (double *) ptr;
			    mWKT+=QString::number(*y,'f',3);
			    ptr += sizeof(double);
			}
			mWKT+=")";
		    }
		    mWKT+=")";
		}
		mWKT+=")";
		break;
	    }
	    default:
#ifdef QGISDEBUG
		qWarning("error: feature type not recognized in QgsFeature::exportToWKT");
#endif
		return false;
		break;
	}
	return true;
	
    }
    else
    {
#ifdef QGISDEBUG
	qWarning("error: no geometry pointer in QgsFeature::exportToWKT");
#endif
	return false;
    }
    
}

QgsPoint QgsFeature::closestVertex(const QgsPoint& point)
{
    if(geometry)
    {
	int wkbType;
	double actdist=DBL_MAX;
	double x,y;
	double *tempx,*tempy;
	memcpy(&wkbType, (geometry+1), sizeof(int));
	switch (wkbType)
	{
	    case QGis::WKBPoint:
		x = *((double *) (geometry + 5));
		y = *((double *) (geometry + 5 + sizeof(double)));
		break;

	    case QGis::WKBLineString:
	    {
		unsigned char* ptr=geometry+5;
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
		int* nrings=(int*)(geometry+5);
		int* npoints;
		unsigned char* ptr=geometry+9;
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
		unsigned char* ptr=geometry+5;
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
		unsigned char* ptr=geometry+5;
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
		unsigned char* ptr=geometry+5;
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
