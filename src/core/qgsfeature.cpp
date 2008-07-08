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
#include "qgsgeometry.h"
#include "qgsrect.h"

#include <iostream>
#include <cfloat>
#ifdef WIN32
#include <limits>
#endif
#include <cstring>
#include <assert.h>

/** \class QgsFeature
 * \brief Encapsulates a spatial feature with attributes
 */

QgsFeature::QgsFeature(int id, QString typeName)
    : mFid(id), 
      mGeometry(0),
      mOwnsGeometry(0),
      mValid(false),
      mDirty(0),
      mTypeName(typeName)
{
  // NOOP
}

QgsFeature::QgsFeature( QgsFeature const & rhs,
                        const QgsChangedAttributesMap & changedAttributes,
                        const QgsGeometryMap & changedGeometries )
  : mFid( rhs.mFid ), 
    mValid( rhs.mValid ),
    mDirty( rhs.mDirty ),
    mTypeName( rhs.mTypeName )

{
  // copy attributes from rhs feature
  mAttributes = rhs.mAttributes;
  
  if (changedAttributes.contains(mFid))
  {
    // get map of changed attributes
    const QgsAttributeMap& changed = changedAttributes[mFid];
  
    // changet the attributes which were provided in the attribute map
    for (QgsAttributeMap::const_iterator it = changed.begin(); it != changed.end(); ++it)
    {
      changeAttribute(it.key(), it.value());
    }
  }
  
  if (changedGeometries.contains(mFid))
  {
    // deep-copy geometry purely from changedGeometries
    mGeometry     = new QgsGeometry(changedGeometries[mFid]);
    mOwnsGeometry = TRUE;
  }
  else
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

}                        


QgsFeature::QgsFeature( QgsFeature const & rhs )
    : mFid( rhs.mFid ), 
      mAttributes( rhs.mAttributes ),
      mValid( rhs.mValid ),
      mDirty( rhs.mDirty ),
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
  mAttributes =  rhs.mAttributes ;
  mValid =  rhs.mValid ;
  mTypeName = rhs.mTypeName;

  // copy embedded geometry
  delete mGeometry;
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
const QgsAttributeMap& QgsFeature::attributeMap() const
{
  return mAttributes;
}

/**Sets the attributes for this feature*/
void QgsFeature::setAttributeMap(const QgsAttributeMap& attributes)
{
  mAttributes = attributes;
}

/**
 * Add an attribute to the map
 */
void QgsFeature::addAttribute(int field, QVariant attr)
{
  mAttributes.insert(field, attr);
}

/**Deletes an attribute and its value*/
void QgsFeature::deleteAttribute(int field)
{
  mAttributes.remove(field);
}


void QgsFeature::changeAttribute(int field, QVariant attr)
{
  mAttributes[field] = attr;
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



/** Set the feature id
*/
void QgsFeature::setFeatureId(int id)
{
  mFid = id;

}


QString QgsFeature::typeName() const
{
  return mTypeName;
} // QgsFeature::typeName



/** sets the feature's type name
 */
void QgsFeature::setTypeName(QString typeName)
{
    mTypeName = typeName;
} // QgsFeature::typeName


void QgsFeature::setGeometry(const QgsGeometry& geom)
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

void QgsFeature::setGeometry(QgsGeometry* geom)
{
  // Destruct the attached geometry only if we still own it, before assigning new one.
  if ( (mOwnsGeometry) && (mGeometry) )
  {
    delete mGeometry;
    mGeometry = 0;
  }
  
  mGeometry = geom;
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
