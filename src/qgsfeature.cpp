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

#include <iostream>

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
  std::cerr << "In QgsFeature destructor" << std::endl;
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

void QgsFeature::setWellKnownText(QString const & wkt)
{
  mWKT = wkt;
}

bool QgsFeature::isValid() const
{
  return mValid;
}

void QgsFeature::setValid(bool validity)
{
  mValid = validity;
}
