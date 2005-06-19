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
#include <iostream>
#include <cfloat>

#include <cstring>

/** \class QgsFeature
 * \brief Encapsulates a spatial feature with attributes
 */
//! Constructor
QgsFeature::QgsFeature()
    : mFid(0), 
      mDirty(0),
      mGeometry(0),
      mOwnsGeometry(0)
{
  // NOOP
}


QgsFeature::QgsFeature(int id, QString const & typeName )
    : mFid(id), 
      mDirty(0),
      mGeometry(0),
      mOwnsGeometry(0),
      mTypeName(typeName)
{
  // NOOP
}

QgsFeature::QgsFeature( QgsFeature const & rhs,
                        std::map<int,std::map<QString,QString> > & changedAttributes,
                        std::map<int, QgsGeometry> & changedGeometries )
    : mFid( rhs.mFid ), 
      mDirty( rhs.mDirty ),
      fieldNames( rhs.fieldNames ),
      mValid( rhs.mValid ),
      mTypeName( rhs.mTypeName )

{

  // Copy the attributes over.
  if ( changedAttributes.find(mFid) == changedAttributes.end() )
  {
    // copy attributes purely from rhs feature
    attributes = rhs.attributes;
  }
  else
  {
    attributes.clear();
  
    // TODO copy attributes from rhs feature with override from changedAttributes
    for (std::vector<QgsFeatureAttribute>::iterator iter  = attributes.begin();
                                                    iter != attributes.end();
                                                  ++iter)
    {
  
      // See if we have a changed attribute for this field
      if ( 
           changedAttributes[mFid].find( iter->fieldName() ) == 
           changedAttributes[mFid].end() 
         )
      {
        // No, copy from rhs feature
        attributes.push_back(changedAttributes[mFid][ iter->fieldName() ]);
      }
      else
      {
        // Yes, copy from changedAttributes
        attributes.push_back(QgsFeatureAttribute(iter->fieldName(),
                                                 changedAttributes[mFid][ iter->fieldName() ]
                            ));
      }  
      
    } 
  }
  

  // Copy the geometry over
  if ( changedGeometries.find(mFid) == changedGeometries.end() )
  {
    // copy geometry purely from rhs feature
    if ( rhs.mGeometry )
    {
      mGeometry = new QgsGeometry( *(rhs.mGeometry) );
      mOwnsGeometry = TRUE;
    }
    else
    {
      mGeometry = 0;
      mOwnsGeometry = FALSE;
    }
  }
  else
  {
    // deep-copy geometry purely from changedGeometries
    mGeometry     = new QgsGeometry(changedGeometries[mFid]);
    mOwnsGeometry = TRUE;
  }

}                        


QgsFeature::QgsFeature( QgsFeature const & rhs )
    : mFid( rhs.mFid ), 
      mDirty( rhs.mDirty ),
      attributes( rhs.attributes ),
      fieldNames( rhs.fieldNames ),
      mValid( rhs.mValid ),
      mTypeName( rhs.mTypeName )
{

  // copy embedded geometry
  if ( rhs.mGeometry )
  {
    mGeometry = new QgsGeometry( *(rhs.mGeometry) );
    mOwnsGeometry = TRUE;
  }
  else
  {
    mGeometry = 0;
    mOwnsGeometry = FALSE;
  }

}


QgsFeature & QgsFeature::operator=( QgsFeature const & rhs )
{
  if ( &rhs == this )
  { return *this; }

  mFid =  rhs.mFid ; 
  mDirty =  rhs.mDirty ; 
  attributes =  rhs.attributes ;
  fieldNames =  rhs.fieldNames ;
  mValid =  rhs.mValid ;
  mTypeName = rhs.mTypeName;

  // copy embedded geometry
  if ( rhs.mGeometry )
  {
    mGeometry = new QgsGeometry( *(rhs.mGeometry) );
    mOwnsGeometry = TRUE;
  }
  else
  {
    mGeometry = 0;
    mOwnsGeometry = FALSE;
  }
    
  return *this;
} // QgsFeature::operator=( QgsFeature const & rhs )



//! Destructor
QgsFeature::~QgsFeature()
{

  // Destruct the attached geometry only if we still own it.
  if ( (mOwnsGeometry) && (mGeometry) )
  {
    delete mGeometry;
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
#ifdef QGISDEBUG
      std::cout << "QgsFeature::attributeMap: Returning attributes"
                << "." << std::endl;
#endif
  
  return attributes;
}

/**
 * Add an attribute to the map
 */
void QgsFeature::addAttribute(QString const&  field, QString const & value)
{
  attributes.push_back(QgsFeatureAttribute(field, value));
}

/**Deletes an attribute and its value*/
void QgsFeature::deleteAttribute(const QString& name)
{
    for(std::vector<QgsFeatureAttribute>::iterator iter=attributes.begin();iter!=attributes.end();++iter)
    {
	if(iter->fieldName()==name)
	{
	    attributes.erase(iter);
	    break;
	}
    }
}

/**Changes an existing attribute value
   @param name attribute name
   @param newval new value*/
void QgsFeature::changeAttributeValue(const QString& name, const QString& newval)
{
   for(std::vector<QgsFeatureAttribute>::iterator iter=attributes.begin();iter!=attributes.end();++iter)
    {
	if(iter->fieldName()==name)
	{
	    iter->setFieldValue(newval);
	    break;
	}
    } 
}

/**
 * Get the fields for this feature
 * @return A std::map containing field position (index) and field name
 */
const std::map < int, QString > &QgsFeature::fields()
{
  return fieldNames;
}


QgsGeometry * QgsFeature::geometry()
{
  return mGeometry;
}


QgsGeometry * QgsFeature::geometryAndOwnership()
{
  mOwnsGeometry = FALSE;
  
  return mGeometry;
}



unsigned char * QgsFeature::getGeometry() const
{

  return mGeometry->wkbBuffer();
  
/*#ifdef QGISDEBUG
      std::cout << "QgsFeature::getGeometry: mDirty is " << mDirty 
                << ", feature address:" << this << "." << std::endl;
#endif
  
  if (mDirty)
  {
#ifdef QGISDEBUG
      std::cout << "QgsFeature::getGeometry: getting modified geometry for " << featureId() << "." << std::endl;
#endif
    return modifiedGeometry;
  }
  else
  {
#ifdef QGISDEBUG
      std::cout << "QgsFeature::getGeometry: getting committed geometry for " << featureId() << "." << std::endl;
#endif
    return geometry;
  }  */
}

/*
unsigned char * QgsFeature::getCommittedGeometry() const
{
  // TODO: Store modifiedGeometry per-edit.
  // Otherwise we may have memory problems with very large geometries
  // as we currently have to keep a full copy of both geometries in memory
#ifdef QGISDEBUG
      std::cout << "QgsFeature::getCommittedGeometry: doing for " << featureId() << "." << std::endl;
#endif
  return geometry;
}


unsigned char * QgsFeature::getModifiedGeometry() const
{
  // TODO: Store modifiedGeometry per-edit.
  // Otherwise we may have memory problems with very large geometries
  // as we currently have to keep a full copy of both geometries in memory
#ifdef QGISDEBUG
      std::cout << "QgsFeature::getCommittedGeometry: doing for " << featureId() << "." << std::endl;
#endif
  return modifiedGeometry;
}*/


size_t QgsFeature::getGeometrySize() const
{
  if (mGeometry)
  {
    return mGeometry->wkbSize();
  }
  else
  {
    return 0;
  }    
} 


/*
size_t QgsFeature::getCommittedGeometrySize() const
{
    return geometrySize;
}


size_t QgsFeature::getModifiedGeometrySize() const
{
    return modifiedGeometrySize;
}
*/


/**
 * Return well known text representation of this feature
 */
QString const & QgsFeature::wellKnownText() const
{
  if (mGeometry)
  {
    return mGeometry->wkt();
  }
  else
  {
    return QString::null;   // TODO: Test for mGeometry in all functions of this class
  }  
/*  
    if(mWKT.isNull())
    {
	exportToWKT();
    }
    return mWKT;*/
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


void QgsFeature::setGeometry(QgsGeometry& geom)
{
  // Destruct the attached geometry only if we still own it, before assigning new one.
  if ( (mOwnsGeometry) && (mGeometry) )
  {
    delete mGeometry;
    mGeometry = 0;
  }
  
  mGeometry = new QgsGeometry(geom);
  mOwnsGeometry = TRUE;
}


/** Set the pointer to the feature geometry
*/
void QgsFeature::setGeometryAndOwnership(unsigned char *geom, size_t length)
{
  // Destruct the attached geometry only if we still own it, before assigning new one.
  if ( (mOwnsGeometry) && (mGeometry) )
  {
    delete mGeometry;
    mGeometry = 0;
  }
  
  mGeometry = new QgsGeometry();
  mGeometry->setWkbAndOwnership(geom, length);
  mOwnsGeometry = TRUE;

}


// /** Set the pointer to the modified feature geometry
// */
// void QgsFeature::setModifiedGeometry(unsigned char *geom, size_t length)
// {
//     // delete any existing WKB geometry before assigning new one
//     if ( modifiedGeometry )
//     {
//         delete [] modifiedGeometry;
//     }
// 
// #ifdef QGISDEBUG
//       std::cout << "QgsFeature::setModifiedGeometry: setting modified geometry for " << featureId() << "." << std::endl;
// #endif
//     
//     modifiedGeometry = geom;
//     modifiedGeometrySize = length;
//     mDirty = TRUE;
// 
// #ifdef QGISDEBUG
//       std::cout << "QgsFeature::setModifiedGeometry: modifiedGeometrySize = " << modifiedGeometrySize 
//                 << ", mDirty = " << mDirty << "." << std::endl;
// #endif
// #ifdef QGISDEBUG
//       exportToWKT(geometry);
//       std::cout << "QgsFeature::setModifiedGeometry: Original: " << mWKT << "." << std::endl;
// #endif
// #ifdef QGISDEBUG
//       exportToWKT(modifiedGeometry);
//       std::cout << "QgsFeature::setModifiedGeometry: Modified: " << mWKT << "." << std::endl;
// #endif
// 
// }


// bool QgsFeature::insertVertexBefore(double x, double y, int beforeVertex, int atRing, int atItem)
// {
// 
// #ifdef QGISDEBUG
//       std::cout << "QgsFeature::insertVertexBefore: Entered with "
//          << "x " << x << ", y " << y 
//          << "beforeVertex " << beforeVertex << ", atRing " << atRing << ", atItem"
//          << " " << atItem            
//                 << "." << std::endl;
// #endif
// 
//   // make sure we're using the latest version
//   unsigned char * localGeometry     = getGeometry();
//   size_t          localGeometrySize = getGeometrySize();
// 
//     if (localGeometry)
//     {
//         int wkbType;
// //        double *x,*y;
// 
//         memcpy(&wkbType, (localGeometry+1), sizeof(int));
//         switch (wkbType)
//         {
//             case QGis::WKBPoint:
//             {
//                 // Meaningless
//                 return FALSE;
//                 break;
//             }
//             case QGis::WKBLineString:
//             {
//                 unsigned char *ptr;
//                 int *nPoints;
//                 int idx;
// 
// #ifdef QGISDEBUG
//       std::cout << "QgsFeature::insertVertexBefore: in QGis::WKBLineString." << std::endl;
// #endif
//                 
//                 // get number of points in the line
//                 ptr = localGeometry + 5;     // now at localGeometry.numPoints
//                 nPoints = (int *) ptr;
// 
// /*                
// #ifdef QGISDEBUG
//       std::cout << "QgsFeature::insertVertexBefore: old number of points is "
//                 << (*nPoints)
//                 << "." << std::endl;
// #endif
// */                                
//                 // silently adjust any overflow "beforeVertex" values to mean
//                 // "append to end of string"
//                 if (*nPoints < beforeVertex)
//                 {
//                   beforeVertex = *nPoints;  // beforeVertex is 0 based, *nPoints is 1 based
//                 }
//                       
//                 // create new geometry expanded by 2 doubles
//                 unsigned char *newWKB = new unsigned char[localGeometrySize + 16];
//                 memset(newWKB, '\0', localGeometrySize + 16);  // just in case
// /*                
// #ifdef QGISDEBUG
//       std::cout << "QgsFeature::insertVertexBefore: newWKB at " << newWKB << "." << std::endl;
// #endif
//                 
// 
// #ifdef QGISDEBUG
//       std::cout << "QgsFeature::insertVertexBefore: done initial newWKB of "
//                 << (localGeometrySize + 16)
//                 << " bytes." << std::endl;
// #endif
// */                                
//                 // copy section before splice
//                 memcpy(newWKB, localGeometry, 9 + (16 * beforeVertex) );
// /*
// #ifdef QGISDEBUG
//       std::cout << "QgsFeature::insertVertexBefore: done pre memcpy of "
//                 << (9 + (16 * beforeVertex))
//                 << " bytes." << std::endl;
// #endif
// #ifdef QGISDEBUG
//       exportToWKT(newWKB);
//       std::cout << "QgsFeature::insertVertexBefore: newWKB: " << mWKT << "." << std::endl;
// #endif
// */
//                 // adjust number of points
//                 (*nPoints)++;
//                 ptr = newWKB + 5;  // now at newWKB.numPoints
//                 memcpy(ptr, nPoints, 4);
// /*
// #ifdef QGISDEBUG
//       std::cout << "QgsFeature::insertVertexBefore: new number of points is "
//                 << (*nPoints)
//                 << "." << std::endl;
// #endif
// */                                
//                 
//                 // copy vertex to be spliced
//                 ptr += 4;   // now at newWKB.points[]
// #ifdef QGISDEBUG
// //      std::cout << "QgsFeature::insertVertexBefore: ptr at " << ptr << "." << std::endl;
// #endif
//                 ptr += (16 * beforeVertex); 
// #ifdef QGISDEBUG
// //      std::cout << "QgsFeature::insertVertexBefore: going to add x " << x << "." << std::endl;
// //      std::cout << "QgsFeature::insertVertexBefore: ptr at " << ptr << ", going to add x " << x << "." << std::endl;
// #endif
//                 memcpy(ptr, &x, 8);
// //                memcpy(ptr, x, 8);
//                 ptr += 8;
// #ifdef QGISDEBUG
// //      std::cout << "QgsFeature::insertVertexBefore: going to add y " << y << "." << std::endl;
// //      std::cout << "QgsFeature::insertVertexBefore: ptr at " << ptr << ", going to add y " << y << "." << std::endl;
// #endif
//                 memcpy(ptr, &y, 8);
// //                memcpy(ptr, y, 8);
//                 ptr += 8;
// 
// /*                
// #ifdef QGISDEBUG
//       exportToWKT(newWKB);
//       std::cout << "QgsFeature::insertVertexBefore: newWKB: " << mWKT << "." << std::endl;
// #endif
// */
//                                 
//                 // copy section after splice
//                 memcpy(ptr, localGeometry + 9 + (16 * beforeVertex), (16 * (*nPoints - beforeVertex)) );
// 
// /*                
// #ifdef QGISDEBUG
//       std::cout << "QgsFeature::insertVertexBefore: done post memcpy of "
//                 << (16 * (*nPoints - beforeVertex))
//                 << " bytes." << std::endl;
// #endif
// */
// #ifdef QGISDEBUG
//       exportToWKT(newWKB);
//       std::cout << "QgsFeature::insertVertexBefore: newWKB: " << mWKT << "." << std::endl;
// #endif
//                                 
//                 // set new geomtry to this object
//                 setModifiedGeometry(newWKB, localGeometrySize + 16);
//       
//                 break;
//             }
//             case QGis::WKBPolygon:
//             {
//                 // TODO
//                 return false;
//                 break;
//             }
//             case QGis::WKBMultiPoint:
//             {
//                 // TODO
//                 return false;
//                 break;
//             }
// 
//             case QGis::WKBMultiLineString:
//             {
//                 // TODO
//                 return false;
//                 break;
//             }
// 
//             case QGis::WKBMultiPolygon:
//             {
//                  // TODO
//                 return false;
//                 break;
//              }
//             default:
// #ifdef QGISDEBUG
//                 qWarning("error: feature type not recognized in QgsFeature::insertVertexBefore");
// #endif
//                 return false;
//                 break;
//         }
// 
// #ifdef QGISDEBUG
//       std::cout << "QgsFeature::insertVertexBefore: Exiting successfully." << std::endl;
// #endif
//         
//         return true;
//         
//     }
//     else
//     {
// #ifdef QGISDEBUG
//         qWarning("error: no geometry pointer in QgsFeature::insertVertexBefore");
// #endif
//         return false;
//     }
//     
// 
// 
// }
// 
// 
// bool QgsFeature::moveVertexAt(double x, double y, int atVertex, int atRing, int atItem)
// {
// }
// 
// 
// bool QgsFeature::vertexAt(double &x, double &y, int atVertex, int atRing, int atItem) const
// {
// #ifdef QGISDEBUG
//       std::cout << "QgsFeature::vertexAt: Entered with "
//          << "atVertex " << atVertex << ", atRing " << atRing << ", atItem"
//          << " " << atItem            
//                 << "." << std::endl;
// #endif
// 
//     if(geometry)
//     {
//         int wkbType;
// 
//         memcpy(&wkbType, (geometry+1), sizeof(int));
//         switch (wkbType)
//         {
//             case QGis::WKBPoint:
//             {
//                 // TODO
//                 return FALSE;
//                 break;
//             }
//             case QGis::WKBLineString:
//             {
//                 unsigned char *ptr;
//                 int *nPoints;
//                 int idx;
// 
//                 // get number of points in the line
//                 ptr = geometry + 5;     // now at geometry.numPoints
//                 nPoints = (int *) ptr;
// 
// #ifdef QGISDEBUG
//       std::cout << "QgsFeature::vertexAt: Number of points in WKBLineString is " << *nPoints
//                 << "." << std::endl;
// #endif
//                                 
//                 // silently adjust any overflow "atVertex" values to mean
//                 // "append to end of string"
//                 if (*nPoints <= atVertex)
//                 {
//                   atVertex = (*nPoints - 1);  // atVertex is 0 based, *nPoints is 1 based
//                 }
// 
//                 // copy the vertex coordinates                      
//                 ptr = geometry + 9 + (atVertex * 16);
//                 memcpy(&x, ptr, 8);
//                 ptr += 8;
//                 memcpy(&y, ptr, 8);
// 
// #ifdef QGISDEBUG
//       std::cout << "QgsFeature::vertexAt: Point is (" << x << ", " << y << ")"
//                 << "." << std::endl;
// #endif
//                       
//                 break;
//             }
//             case QGis::WKBPolygon:
//             {
//                 // TODO
//                 return false;
//                 break;
//             }
//             case QGis::WKBMultiPoint:
//             {
//                 // TODO
//                 return false;
//                 break;
//             }
// 
//             case QGis::WKBMultiLineString:
//             {
//                 // TODO
//                 return false;
//                 break;
//             }
// 
//             case QGis::WKBMultiPolygon:
//             {
//                  // TODO
//                 return false;
//                 break;
//              }
//             default:
// #ifdef QGISDEBUG
//                 qWarning("error: feature type not recognized in QgsFeature::vertexAt");
// #endif
//                 return false;
//                 break;
//         }
//         
// #ifdef QGISDEBUG
//       std::cout << "QgsFeature::vertexAt: Exiting TRUE." << std::endl;
// #endif
//         
//         return true;
//         
//     }
//     else
//     {
// #ifdef QGISDEBUG
//         qWarning("error: no geometry pointer in QgsFeature::vertexAt");
// #endif
//         return false;
//     }
// 
//     
// }


bool QgsFeature::isValid() const
{
  return mValid;
}

void QgsFeature::setValid(bool validity)
{
  mValid = validity;
}

bool QgsFeature::isDirty() const
{
  return mDirty;
}

void QgsFeature::resetDirty()
{
  mDirty = FALSE;
}


bool QgsFeature::attributeDialog()
{
    QgsAttributeDialog attdialog(&attributes);
    if(attdialog.exec()==QDialog::Accepted)
    {
	for(int i=0;i<attributes.size();++i)
	{
	    attributes[i].setFieldValue(attdialog.value(i));
	}
	return true;
    }
    else
    {
	return false;
    }
}

// bool QgsFeature::intersects(QgsRect* r) const
// {
//     bool returnval=false;
// 
//     geos::GeometryFactory *gf = new geos::GeometryFactory();
//     geos::WKTReader *wktReader = new geos::WKTReader(gf);
//     geos::Geometry *geosGeom = wktReader->read( qstrdup(wellKnownText()) );
// 
//     //write the selection rectangle to wkt by hand
//     QString rectwkt="POLYGON((";
//     rectwkt+=QString::number(r->xMin(),'f',3);
//     rectwkt+=" ";
//     rectwkt+=QString::number(r->yMin(),'f',3);
//     rectwkt+=",";
//     rectwkt+=QString::number(r->xMax(),'f',3);
//     rectwkt+=" ";
//     rectwkt+=QString::number(r->yMin(),'f',3);
//     rectwkt+=",";
//     rectwkt+=QString::number(r->xMax(),'f',3);
//     rectwkt+=" ";
//     rectwkt+=QString::number(r->yMax(),'f',3);
//     rectwkt+=",";
//     rectwkt+=QString::number(r->xMin(),'f',3);
//     rectwkt+=" ";
//     rectwkt+=QString::number(r->yMax(),'f',3);
//     rectwkt+=",";
//     rectwkt+=QString::number(r->xMin(),'f',3);
//     rectwkt+=" ";
//     rectwkt+=QString::number(r->yMin(),'f',3);
//     rectwkt+="))";
//     
//     geos::Geometry *geosRect = wktReader->read( qstrdup(rectwkt) );
//     if(geosGeom->intersects(geosRect))
//     {
// 	returnval=true;
//     }
//  
//     delete geosGeom;
//     delete geosRect;
//     delete gf;
//     delete wktReader;
//     return returnval;
// }
// 
// 
// bool QgsFeature::exportToWKT(unsigned char * geom) const
// {
//     if(geom)
//     {
// 	int wkbType;
// 	double *x,*y;
// 
// 	mWKT="";
// 	memcpy(&wkbType, (geom+1), sizeof(int));
// 	switch (wkbType)
// 	{
// 	    case QGis::WKBPoint:
// 	    {
// 		mWKT+="POINT(";
// 		x = (double *) (geom + 5);
// 		mWKT+=QString::number(*x,'f',3);
// 		mWKT+=" ";
// 		y = (double *) (geom + 5 + sizeof(double));
// 		mWKT+=QString::number(*y,'f',3);
// 		mWKT+=")";
// 		break;
// 	    }
// 	    case QGis::WKBLineString:
// 	    {
// 		unsigned char *ptr;
// 		int *nPoints;
// 		int idx;
// 
// 		mWKT+="LINESTRING(";
// 		// get number of points in the line
// 		ptr = geom + 5;
// 		nPoints = (int *) ptr;
// 		ptr = geom + 1 + 2 * sizeof(int);
// 		for (idx = 0; idx < *nPoints; ++idx)
// 		{
// 		    if(idx!=0)
// 		    {
// 			mWKT+=", ";
// 		    }
// 		    x = (double *) ptr;
// 		    mWKT+=QString::number(*x,'f',3);
// 		    mWKT+=" ";
// 		    ptr += sizeof(double);
// 		    y = (double *) ptr;
// 		    mWKT+=QString::number(*y,'f',3);
// 		    ptr += sizeof(double);
// 		}
// 		mWKT+=")";
// 		break;
// 	    }
// 	    case QGis::WKBPolygon:
// 	    {
// 		unsigned char *ptr;
// 		int idx, jdx;
// 		int *numRings, *nPoints;
// 
// 		mWKT+="POLYGON(";
// 		// get number of rings in the polygon
// 		numRings = (int *)(geom + 1 + sizeof(int));
// 		if (!(*numRings))  // sanity check for zero rings in polygon
// 		{
// 		    break;
// 		}
// 		int *ringStart; // index of first point for each ring
// 		int *ringNumPoints; // number of points in each ring
// 		ringStart = new int[*numRings];
// 		ringNumPoints = new int[*numRings];
// 		ptr = geom+1+2*sizeof(int); // set pointer to the first ring
// 		for (idx = 0; idx < *numRings; idx++)
// 		{
// 		    if(idx!=0)
// 		    {
// 			mWKT+=",";
// 		    }
// 		    mWKT+="(";
// 		    // get number of points in the ring
// 		    nPoints = (int *) ptr;
// 		    ringNumPoints[idx] = *nPoints;
// 		    ptr += 4;
// 		    
// 		    for(jdx=0;jdx<*nPoints;jdx++)
// 		    {
// 			if(jdx!=0)
// 			{
// 			    mWKT+=",";
// 			}
// 			x = (double *) ptr;
// 			mWKT+=QString::number(*x,'f',3);
// 			mWKT+=" ";
// 			ptr += sizeof(double);
// 			y = (double *) ptr;
// 			mWKT+=QString::number(*y,'f',3);
// 			ptr += sizeof(double);
// 		    }
// 		    mWKT+=")";
// 		}
// 		mWKT+=")";
// 		delete [] ringStart;
// 		delete [] ringNumPoints;
// 		break;
// 	    }
// 	    case QGis::WKBMultiPoint:
// 	    {
// 		unsigned char *ptr;
// 		int idx;
// 		int *nPoints;
// 
// 		mWKT+="MULTIPOINT(";
// 		nPoints=(int*)(geom+5);
// 		ptr=geom+5+sizeof(int);
// 		for(idx=0;idx<*nPoints;++idx)
// 		{
// 		    if(idx!=0)
// 		    {
// 			mWKT+=", ";
// 		    }
// 		    x = (double *) (ptr);
// 		    mWKT+=QString::number(*x,'f',3);
// 		    mWKT+=" ";
// 		    ptr+=sizeof(double);
// 		    y= (double *) (ptr);
// 		    mWKT+=QString::number(*y,'f',3);
// 		    ptr+=sizeof(double);
// 		}
// 		mWKT+=")";
// 		break;
// 	    }
// 
// 	    case QGis::WKBMultiLineString:
// 	    {
// 		unsigned char *ptr;
// 		int idx, jdx, numLineStrings;
// 		int *nPoints;
// 
// 		mWKT+="MULTILINESTRING(";
// 		numLineStrings = (int) (geom[5]);
// 		ptr = geom + 9;
// 		for (jdx = 0; jdx < numLineStrings; jdx++)
// 		{
// 		    if(jdx!=0)
// 		    {
// 			mWKT+=", ";
// 		    }
// 		    mWKT+="(";
// 		    ptr += 5; // skip type since we know its 2
// 		    nPoints = (int *) ptr;
// 		    ptr += sizeof(int);
// 		    for (idx = 0; idx < *nPoints; idx++)
// 		    {
// 			if(idx!=0)
// 			{
// 			    mWKT+=", ";
// 			}
// 			x = (double *) ptr;
// 			mWKT+=QString::number(*x,'f',3);
// 			ptr += sizeof(double);
// 			mWKT+=" ";
// 			y = (double *) ptr;
// 			mWKT+=QString::number(*y,'f',3);
// 			ptr += sizeof(double);
// 		    }
// 		    mWKT+=")";
// 		}
// 		mWKT+=")";
// 		break;
// 	    }
// 
// 	    case QGis::WKBMultiPolygon:
// 	    {
// 		unsigned char *ptr;
// 		int idx, jdx, kdx;
// 		int *numPolygons, *numRings, *nPoints;
// 		
// 		mWKT+="MULTIPOLYGON(";
// 		ptr = geom + 5;
// 		numPolygons = (int *) ptr;
// 		ptr = geom + 9;
// 		for (kdx = 0; kdx < *numPolygons; kdx++)
// 		{
// 		    if(kdx!=0)
// 		    {
// 			mWKT+=",";
// 		    }
// 		    mWKT+="(";
// 		    ptr+=5;
// 		    numRings = (int *) ptr;
// 		    ptr += 4;
// 		    for (idx = 0; idx < *numRings; idx++)
// 		    {
// 			if(idx!=0)
// 			{
// 			    mWKT+=",";
// 			}
// 			mWKT+="(";
// 			nPoints = (int *) ptr;
// 			ptr += 4;
// 			for (jdx = 0; jdx < *nPoints; jdx++)
// 			{
// 			    x = (double *) ptr;
// 			    mWKT+=QString::number(*x,'f',3);
// 			    ptr += sizeof(double);
// 			    mWKT+=" ";
// 			    y = (double *) ptr;
// 			    mWKT+=QString::number(*y,'f',3);
// 			    ptr += sizeof(double);
// 			}
// 			mWKT+=")";
// 		    }
// 		    mWKT+=")";
// 		}
// 		mWKT+=")";
// 		break;
// 	    }
// 	    default:
// #ifdef QGISDEBUG
// 		qWarning("error: feature type not recognized in QgsFeature::exportToWKT");
// #endif
// 		return false;
// 		break;
// 	}
// 	return true;
// 	
//     }
//     else
//     {
// #ifdef QGISDEBUG
// 	qWarning("error: no geom pointer in QgsFeature::exportToWKT");
// #endif
// 	return false;
//     }
//     
// }
// 
// 
// bool QgsFeature::exportToWKT() const
// {
//   return exportToWKT( mGeometry.wkbBuffer() );
// }
// 

/*
QgsPoint QgsFeature::closestVertex(const QgsPoint& point) const
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
*/

// double QgsFeature::distanceSquaredPointToSegment(QgsPoint& point,
//                                                  double *x1, double *y1,
//                                                  double *x2, double *y2,
//                                                  QgsPoint& minDistPoint)
// {
// 
//   double d;
//   
//   // Proportion along segment (0 to 1) the perpendicular of the point falls upon
//   double t;
//   
//   
//   // Projection of point on the segment
//   double xn;
//   double yn;
// 
//   double segmentsqrdist = ( *x2 - *x1 ) * ( *x2 - *x1 ) + 
//                           ( *y2 - *y1 ) * ( *y2 - *y1 );
// 
// #ifdef QGISDEBUG
// //      std::cout << "QgsFeature::distanceSquaredPointToSegment: Entered with "
// //         << "(" << *x1 << ", " << *y1 << ") (" << *x2 << ", " << *y2 << ")"
// //         << " " << segmentsqrdist            
// //                << "." << std::endl;
// #endif
//                           
//                                                     
//   if ( segmentsqrdist == 0.0 )
//   {
//     // segment is a point 
//     xn = *x1;
//     yn = *y1;
//   }
//   else
//   {
// 
//     d =
//         ( point.x() - *x1 ) * ( *x2 - *x1 )
//       + ( point.y() - *y1 ) * ( *y2 - *y1 );
// 
//     t = d / segmentsqrdist;
//     
//     // Do not go beyond end of line
//     // (otherwise this would be a comparison to an infinite line)
//     if (t < 0.0)
//     { 
//       xn = *x1; 
//       yn = *y1;
//     }
//     else if (t > 1.0)
//     {
//       xn = *x2; 
//       yn = *y2;
//     }
//     else
//     {
//       xn = *x1 + t * ( *x2 - *x1 );
//       yn = *y1 + t * ( *y2 - *y1 );
//     }  
// 
//   }
//   
//   minDistPoint.set(xn, yn);
// 
//   return (
//           ( xn - point.x() ) * ( xn - point.x() ) + 
//           ( yn - point.y() ) * ( yn - point.y() )
//          );
// 
// }
//                    


// TODO: Wrap WKB processing into its own class

// QgsPoint QgsFeature::closestSegmentWithContext(QgsPoint& point, 
//                                     int& beforeVertex, int& atRing, int& atItem,
//                                     double& minSqrDist)
// //QgsPoint QgsFeature::closestSegment(QgsPoint& point, 
// //                                    QgsPoint& segStart, QgsPoint& segStop,
// //                                    double& minSqrDist)
// {
// 
// #ifdef QGISDEBUG
// //      std::cout << "QgsFeature::closestSegment: Entered"
// //                << "." << std::endl;
// #endif
// 
//   QgsPoint minDistPoint;
// 
//     int wkbType;
//     double x,y;
//     double *thisx,*thisy;
//     double *prevx,*prevy;
//     double testdist;
//   
//   // Initialise some stuff
//   beforeVertex = -1;
//   atRing       = -1;
//   atItem       = -1;
//   minSqrDist   = DBL_MAX;
// 
//   if (mGeometry)
//   {
// //    int wkbType;
// //    double x,y;
// //    double *thisx,*thisy;
// //    double *prevx,*prevy;
// //    double testdist;
//     
//     memcpy(&wkbType, (mGeometry+1), sizeof(int));
//     
//     switch (wkbType)
//     {
//     case QGis::WKBPoint:
//       // Points have no lines
//       return QgsPoint(0,0);
//      
//     case QGis::WKBLineString:
//       unsigned char* ptr = mGeometry + 5;
//       int* npoints = (int*) ptr;
//       ptr += sizeof(int);
//       for (int index=0; index < *npoints; ++index)
//       {
//         if (index > 0)
//         {
//           prevx = thisx;  
//           prevy = thisy;  
//         }
//         
//         thisx = (double*) ptr;
//         ptr += sizeof(double);
//         thisy = (double*) ptr;
//         
//         if (index > 0)
//         {
//           if ( 
//                ( 
//                  testdist = distanceSquaredPointToSegment(point,
//                                                           prevx, prevy,
//                                                           thisx, thisy,
//                                                           minDistPoint)
//                )                                           
//                < minSqrDist )                              
//           {
// #ifdef QGISDEBUG
// //      std::cout << "QgsFeature::closestSegment: testDist "
// //                << testdist << ", minSqrDist"
// //                << minSqrDist
// //                << "." << std::endl;
// #endif
//             
//             beforeVertex = index;
//             atRing       = 0;
//             atItem       = 0;
//             
//             minSqrDist = testdist;
//           }
//         }
//         
//         ptr += sizeof(double);
//       }
//       break;
// 
//       // TODO: Other geometry types
//       
//     } // switch (wkbType)
//     
//   } // if (mGeometry)
// 
// #ifdef QGISDEBUG
//       std::cout << "QgsFeature::closestSegment: Exiting on feature " << mFid << " with beforeVertex "
//                 << beforeVertex << ", minSqrDist from "
//                 << point.stringRep() << " is "
//                 << minSqrDist
//                 << "." << std::endl;
// #endif
//       
//   return minDistPoint;
// 
// }                 
// 
// 
QgsRect QgsFeature::boundingBox() const
{
  return mGeometry->boundingBox();
}
// QgsRect QgsFeature::boundingBox() const
// {
//     double xmin=DBL_MAX;
//     double ymin=DBL_MAX;
//     double xmax=-DBL_MAX;
//     double ymax=-DBL_MAX;
// 
//     double *x;
//     double *y;
//     int *nPoints;
//     int *numRings;
//     int *numPolygons;
//     int numPoints;
//     int numLineStrings;
//     int idx, jdx, kdx;
//     unsigned char *ptr;
//     char lsb;
//     QgsPoint pt;
//     QPointArray *pa;
//     int wkbType;
//     unsigned char *feature;
// 
//     feature = this->getGeometry();
//     if(feature)
//     {
// 	wkbType=(int) feature[1];
// 	switch (wkbType)
//       {
//       case QGis::WKBPoint:
//         x = (double *) (feature + 5);
//         y = (double *) (feature + 5 + sizeof(double));
//         if (*x < xmin)
//         {
//           xmin=*x;
//         }
//         if (*x > xmax)
//         {
//           xmax=*x;
//         }
//         if (*y < ymin)
//         {
//           ymin=*y;
//         }
//         if (*y > ymax)
//         {
//           ymax=*y;
//         }
//         break;
// 
//       case QGis::WKBLineString:
//         // get number of points in the line
//         ptr = feature + 5;
//         nPoints = (int *) ptr;
//         ptr = feature + 1 + 2 * sizeof(int);
//         for (idx = 0; idx < *nPoints; idx++)
//         {
//           x = (double *) ptr;
//           ptr += sizeof(double);
//           y = (double *) ptr;
//           ptr += sizeof(double);
//           if (*x < xmin)
//           {
//             xmin=*x;
//           }
//           if (*x > xmax)
//           {
// 	      xmax=*x;
//           }
//           if (*y < ymin)
//           {
//             ymin=*y;
//           }
//           if (*y > ymax)
//           {
//             ymax=*y;
//           }
//         }
//         break;
// 
//       case QGis::WKBMultiLineString:
//         numLineStrings = (int) (feature[5]);
//         ptr = feature + 9;
//         for (jdx = 0; jdx < numLineStrings; jdx++)
//         {
//           // each of these is a wbklinestring so must handle as such
//           lsb = *ptr;
//           ptr += 5;   // skip type since we know its 2
//           nPoints = (int *) ptr;
//           ptr += sizeof(int);
//           for (idx = 0; idx < *nPoints; idx++)
//           {
//             x = (double *) ptr;
//             ptr += sizeof(double);
//             y = (double *) ptr;
//             ptr += sizeof(double);
//             if (*x < xmin)
//             {
//               xmin=*x;
//             }
//             if (*x > xmax)
//             {
//               xmax=*x;
//             }
//             if (*y < ymin)
//             {
//               ymin=*y;
//             }
//             if (*y > ymax)
//             {
//               ymax=*y;
//             }
//           }
//         }
//         break;
// 
//       case QGis::WKBPolygon:
//         // get number of rings in the polygon
//         numRings = (int *) (feature + 1 + sizeof(int));
//         ptr = feature + 1 + 2 * sizeof(int);
//         for (idx = 0; idx < *numRings; idx++)
//         {
//           // get number of points in the ring
//           nPoints = (int *) ptr;
//           ptr += 4;
//           for (jdx = 0; jdx < *nPoints; jdx++)
//           {
//             // add points to a point array for drawing the polygon
//             x = (double *) ptr;
//             ptr += sizeof(double);
//             y = (double *) ptr;
//             ptr += sizeof(double);
//             if (*x < xmin)
//             {
//               xmin=*x;
//             }
//             if (*x > xmax)
//             {
//               xmax=*x;
//             }
//             if (*y < ymin)
//             {
//               ymin=*y;
//             }
//             if (*y > ymax)
//             {
//               ymax=*y;
//             }
//           }
//         }
//         break;
// 
// 	case QGis::WKBMultiPolygon:
//         // get the number of polygons
//         ptr = feature + 5;
//         numPolygons = (int *) ptr;
//         for (kdx = 0; kdx < *numPolygons; kdx++)
//         {
//           //skip the endian and feature type info and
//           // get number of rings in the polygon
//           ptr = feature + 14;
//           numRings = (int *) ptr;
//           ptr += 4;
//           for (idx = 0; idx < *numRings; idx++)
//           {
//             // get number of points in the ring
//             nPoints = (int *) ptr;
//             ptr += 4;
//             for (jdx = 0; jdx < *nPoints; jdx++)
//             {
//               // add points to a point array for drawing the polygon
//               x = (double *) ptr;
//               ptr += sizeof(double);
//               y = (double *) ptr;
//               ptr += sizeof(double);
//               if (*x < xmin)
//               {
//                 xmin=*x;
//               }
//               if (*x > xmax)
//               {
//                 xmax=*x;
//               }
//               if (*y < ymin)
//               {
//                 ymin=*y;
//               }
//               if (*y > ymax)
//               {
//                 ymax=*y;
//               }
//             }
//           }
//         }
//         break;
// 
//       default:
// 	  #ifdef QGISDEBUG
//         std::cout << "UNKNOWN WKBTYPE ENCOUNTERED\n";
// #endif
//         break;
// 
//       }
//       return QgsRect(xmin,ymin,xmax,ymax);
//     }
//     else
//     {
// 	return QgsRect(0,0,0,0);
//     }
// }
// 

geos::Geometry* QgsFeature::geosGeometry() const
{
  return mGeometry->geosGeometry();
}

