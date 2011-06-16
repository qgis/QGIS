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

#include "qgsfeature.h"
#include "qgsgeometry.h"
#include "qgsrectangle.h"

/** \class QgsFeature
 * \brief Encapsulates a spatial feature with attributes
 */

QgsFeature::QgsFeature( QgsFeatureId id, QString typeName )
    : mFid( id )
    , mGeometry( 0 )
    , mOwnsGeometry( 0 )
    , mValid( false )
    , mDirty( 0 )
    , mTypeName( typeName )
{
  // NOOP
}

QgsFeature::QgsFeature( QgsFeature const & rhs )
    : mFid( rhs.mFid )
    , mAttributes( rhs.mAttributes )
    , mGeometry( 0 )
    , mOwnsGeometry( false )
    , mValid( rhs.mValid )
    , mDirty( rhs.mDirty )
    , mTypeName( rhs.mTypeName )
{

  // copy embedded geometry
  if ( rhs.mGeometry )
  {
    setGeometry( *rhs.mGeometry );
  }
}


QgsFeature & QgsFeature::operator=( QgsFeature const & rhs )
{
  if ( &rhs == this )
    return *this;

  mFid =  rhs.mFid;
  mDirty =  rhs.mDirty;
  mAttributes =  rhs.mAttributes;
  mValid =  rhs.mValid;
  mTypeName = rhs.mTypeName;

  // make sure to delete the old geometry (if exists)
  if ( mGeometry && mOwnsGeometry )
    delete mGeometry;

  mGeometry = 0;
  mOwnsGeometry = false;

  if ( rhs.mGeometry )
    setGeometry( *rhs.mGeometry );

  return *this;
} // QgsFeature::operator=( QgsFeature const & rhs )



//! Destructor
QgsFeature::~QgsFeature()
{
  // Destruct the attached geometry only if we still own it.
  if ( mOwnsGeometry && mGeometry )
    delete mGeometry;
}

/**
 * Get the feature id for this feature
 * @return Feature id
 */
QgsFeatureId QgsFeature::id() const
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
void QgsFeature::setAttributeMap( const QgsAttributeMap& attributes )
{
  mAttributes = attributes;
}

/**Clear attribute map for this feature*/
void QgsFeature::clearAttributeMap()
{
  mAttributes.clear();
}

/**
 * Add an attribute to the map
 */
void QgsFeature::addAttribute( int field, QVariant attr )
{
  mAttributes.insert( field, attr );
}

/**Deletes an attribute and its value*/
void QgsFeature::deleteAttribute( int field )
{
  mAttributes.remove( field );
}


void QgsFeature::changeAttribute( int field, QVariant attr )
{
  mAttributes[field] = attr;
}

QgsGeometry *QgsFeature::geometry()
{
  return mGeometry;
}

QgsGeometry *QgsFeature::geometryAndOwnership()
{
  mOwnsGeometry = false;

  return mGeometry;
}



/** Set the feature id
*/
void QgsFeature::setFeatureId( QgsFeatureId id )
{
  mFid = id;
}


QString QgsFeature::typeName() const
{
  return mTypeName;
} // QgsFeature::typeName



/** sets the feature's type name
 */
void QgsFeature::setTypeName( QString typeName )
{
  mTypeName = typeName;
} // QgsFeature::typeName


void QgsFeature::setGeometry( const QgsGeometry& geom )
{
  setGeometry( new QgsGeometry( geom ) );
}

void QgsFeature::setGeometry( QgsGeometry* geom )
{
  // Destruct the attached geometry only if we still own it, before assigning new one.
  if ( mOwnsGeometry && mGeometry )
  {
    delete mGeometry;
    mGeometry = 0;
  }

  mGeometry = geom;
  mOwnsGeometry = true;
}

/** Set the pointer to the feature geometry
*/
void QgsFeature::setGeometryAndOwnership( unsigned char *geom, size_t length )
{
  QgsGeometry *g = new QgsGeometry();
  g->fromWkb( geom, length );
  setGeometry( g );
}


bool QgsFeature::isValid() const
{
  return mValid;
}

void QgsFeature::setValid( bool validity )
{
  mValid = validity;
}

bool QgsFeature::isDirty() const
{
  return mDirty;
}

void QgsFeature::clean()
{
  mDirty = false;
}
