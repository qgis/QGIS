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
#include "qgsfield.h"
#include "qgsgeometry.h"
#include "qgsrectangle.h"

#include "qgsmessagelog.h"

/** \class QgsFeature
 * \brief Encapsulates a spatial feature with attributes
 */

QgsFeature::QgsFeature( QgsFeatureId id )
    : mFid( id )
    , mGeometry( 0 )
    , mOwnsGeometry( 0 )
    , mValid( false )
    , mFields( 0 )
{
  // NOOP
}

QgsFeature::QgsFeature( const QgsFields &fields, QgsFeatureId id )
    : mFid( id )
    , mGeometry( 0 )
    , mOwnsGeometry( 0 )
    , mValid( false )
    , mFields( &fields )
{
  initAttributes( fields.count() );
}

QgsFeature::QgsFeature( QgsFeature const & rhs )
    : mFid( rhs.mFid )
    , mAttributes( rhs.mAttributes )
    , mGeometry( 0 )
    , mOwnsGeometry( false )
    , mValid( rhs.mValid )
    , mFields( rhs.mFields )
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
  mAttributes =  rhs.mAttributes;
  mValid =  rhs.mValid;
  mFields = rhs.mFields;

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

/**Deletes an attribute and its value*/
void QgsFeature::deleteAttribute( int field )
{
  mAttributes.remove( field );
}


QgsGeometry *QgsFeature::geometry() const
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

void QgsFeature::setFields( const QgsFields* fields, bool initAttributes )
{
  mFields = fields;
  if ( initAttributes )
  {
    this->initAttributes( fields->count() );
  }
}


bool QgsFeature::isValid() const
{
  return mValid;
}

void QgsFeature::setValid( bool validity )
{
  mValid = validity;
}

void QgsFeature::initAttributes( int fieldCount )
{
  mAttributes.resize( fieldCount );
  QVariant* ptr = mAttributes.data();
  for ( int i = 0; i < fieldCount; ++i, ++ptr )
    ptr->clear();
}


bool QgsFeature::setAttribute( int idx, const QVariant &value )
{
  if ( idx < 0 || idx >= mAttributes.size() )
  {
    QgsMessageLog::logMessage( QObject::tr( "Attribute index %1 out of bounds [0;%2[" ).arg( idx ).arg( mAttributes.size() ), QString::null, QgsMessageLog::WARNING );
    return false;
  }

  mAttributes[idx] = value;
  return true;
}

bool QgsFeature::setAttribute( const QString& name, QVariant value )
{
  int fieldIdx = fieldNameIndex( name );
  if ( fieldIdx == -1 )
    return false;

  mAttributes[fieldIdx] = value;
  return true;
}

bool QgsFeature::deleteAttribute( const QString& name )
{
  int fieldIdx = fieldNameIndex( name );
  if ( fieldIdx == -1 )
    return false;

  mAttributes[fieldIdx].clear();
  return true;
}

QVariant QgsFeature::attribute( int fieldIdx ) const
{
  if ( fieldIdx < 0 || fieldIdx >= mAttributes.count() )
    return QVariant();
  return mAttributes[fieldIdx];
}


QVariant QgsFeature::attribute( const QString& name ) const
{
  int fieldIdx = fieldNameIndex( name );
  if ( fieldIdx == -1 )
    return QVariant();

  return mAttributes[fieldIdx];
}

int QgsFeature::fieldNameIndex( const QString& fieldName ) const
{
  if ( !mFields )
    return -1;

  for ( int i = 0; i < mFields->count(); ++i )
  {
    if ( QString::compare( mFields->at( i ).name(), fieldName, Qt::CaseInsensitive ) == 0 )
    {
      return i;
    }
  }
  return -1;
}
