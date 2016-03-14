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
#include "qgsfeature_p.h"
#include "qgsfield.h"
#include "qgsgeometry.h"
#include "qgsrectangle.h"

#include "qgsmessagelog.h"

#include <QDataStream>

/***************************************************************************
 * This class is considered CRITICAL and any change MUST be accompanied with
 * full unit tests in testqgsfeature.cpp.
 * See details in QEP #17
 ****************************************************************************/

QgsFeature::QgsFeature( QgsFeatureId id )
{
  d = new QgsFeaturePrivate( id );
}

QgsFeature::QgsFeature( const QgsFields &fields, QgsFeatureId id )
{
  d = new QgsFeaturePrivate( id );
  d->fields = fields;
  initAttributes( d->fields.count() );
}

QgsFeature::QgsFeature( QgsFeature const & rhs )
    : d( rhs.d )
{
}

QgsFeature & QgsFeature::operator=( QgsFeature const & rhs )
{
  d = rhs.d;
  return *this;
}

QgsFeature::~QgsFeature()
{
}

/***************************************************************************
 * This class is considered CRITICAL and any change MUST be accompanied with
 * full unit tests in testqgsfeature.cpp.
 * See details in QEP #17
 ****************************************************************************/

QgsFeatureId QgsFeature::id() const
{
  return d->fid;
}

void QgsFeature::deleteAttribute( int field )
{
  d.detach();
  d->attributes.remove( field );
}

QgsGeometry *QgsFeature::geometry()
{
  d.detach();
  return d->geometry;
}

const QgsGeometry* QgsFeature::constGeometry() const
{
  return d->geometry;
}

QgsGeometry *QgsFeature::geometryAndOwnership()
{
  d.detach();
  d->ownsGeometry = false;

  return d->geometry;
}

/***************************************************************************
 * This class is considered CRITICAL and any change MUST be accompanied with
 * full unit tests in testqgsfeature.cpp.
 * See details in QEP #17
 ****************************************************************************/

void QgsFeature::setFeatureId( QgsFeatureId id )
{
  if ( id == d->fid )
    return;

  d.detach();
  d->fid = id;
}

QgsAttributes QgsFeature::attributes() const
{
  return d->attributes;
}

void QgsFeature::setAttributes( const QgsAttributes &attrs )
{
  if ( attrs == d->attributes )
    return;

  d.detach();
  d->attributes = attrs;
}

void QgsFeature::setGeometry( const QgsGeometry& geom )
{
  setGeometry( new QgsGeometry( geom ) );
}

void QgsFeature::setGeometry( QgsGeometry* geom )
{
  // we do a little bit of trickery here to avoid an unnecessary deep copy
  // of the existing geometry by the detach function
  // (since we are replacing the geometry anyway)

  //first, store the old ownsGeometry status
  QgsFeaturePrivate* old_d = d.data();
  bool ownedGeom = d->ownsGeometry;

  //then set owns geometry to false before the detach, so that the deep copy
  //is not made
  d->ownsGeometry = false;
  d.detach();

  //restore ownsGeometry setting if a detach was made
  if ( old_d != d.data() )
  {
    old_d->ownsGeometry = ownedGeom;
  }
  else if ( ownedGeom )
  {
    delete d->geometry;
  }

  d->geometry = geom;
  d->ownsGeometry = true;
}

/***************************************************************************
 * This class is considered CRITICAL and any change MUST be accompanied with
 * full unit tests in testqgsfeature.cpp.
 * See details in QEP #17
 ****************************************************************************/

/** Set the pointer to the feature geometry
*/
void QgsFeature::setGeometryAndOwnership( unsigned char *geom, int length )
{
  QgsGeometry *g = new QgsGeometry();
  g->fromWkb( geom, length );
  setGeometry( g );
}

void QgsFeature::setFields( const QgsFields* fields, bool init )
{
  setFields( *fields, init );
}

void QgsFeature::setFields( const QgsFields &fields, bool init )
{
  d.detach();
  d->fields = fields;
  if ( init )
  {
    initAttributes( d->fields.count() );
  }
}

const QgsFields *QgsFeature::fields() const
{
  return &( d->fields );
}

/***************************************************************************
 * This class is considered CRITICAL and any change MUST be accompanied with
 * full unit tests in testqgsfeature.cpp.
 * See details in QEP #17
 ****************************************************************************/

bool QgsFeature::isValid() const
{
  return d->valid;
}

void QgsFeature::setValid( bool validity )
{
  if ( d->valid == validity )
    return;

  d.detach();
  d->valid = validity;
}

void QgsFeature::initAttributes( int fieldCount )
{
  d.detach();
  d->attributes.resize( fieldCount );
  QVariant* ptr = d->attributes.data();
  for ( int i = 0; i < fieldCount; ++i, ++ptr )
    ptr->clear();
}

bool QgsFeature::setAttribute( int idx, const QVariant &value )
{
  if ( idx < 0 || idx >= d->attributes.size() )
  {
    QgsMessageLog::logMessage( QObject::tr( "Attribute index %1 out of bounds [0;%2]" ).arg( idx ).arg( d->attributes.size() ), QString::null, QgsMessageLog::WARNING );
    return false;
  }

  d.detach();
  d->attributes[idx] = value;
  return true;
}

/***************************************************************************
 * This class is considered CRITICAL and any change MUST be accompanied with
 * full unit tests in testqgsfeature.cpp.
 * See details in QEP #17
 ****************************************************************************/

bool QgsFeature::setAttribute( const QString& name, const QVariant& value )
{
  int fieldIdx = fieldNameIndex( name );
  if ( fieldIdx == -1 )
    return false;

  d.detach();
  d->attributes[fieldIdx] = value;
  return true;
}

bool QgsFeature::deleteAttribute( const QString& name )
{
  int fieldIdx = fieldNameIndex( name );
  if ( fieldIdx == -1 )
    return false;

  d.detach();
  d->attributes[fieldIdx].clear();
  return true;
}

QVariant QgsFeature::attribute( int fieldIdx ) const
{
  if ( fieldIdx < 0 || fieldIdx >= d->attributes.count() )
    return QVariant();

  return d->attributes.at( fieldIdx );
}

QVariant QgsFeature::attribute( const QString& name ) const
{
  int fieldIdx = fieldNameIndex( name );
  if ( fieldIdx == -1 )
    return QVariant();

  return d->attributes.at( fieldIdx );
}

/***************************************************************************
 * This class is considered CRITICAL and any change MUST be accompanied with
 * full unit tests in testqgsfeature.cpp.
 * See details in QEP #17
 ****************************************************************************/

int QgsFeature::fieldNameIndex( const QString& fieldName ) const
{
  return d->fields.fieldNameIndex( fieldName );
}

/***************************************************************************
 * This class is considered CRITICAL and any change MUST be accompanied with
 * full unit tests in testqgsfeature.cpp.
 * See details in QEP #17
 ****************************************************************************/

QDataStream& operator<<( QDataStream& out, const QgsFeature& feature )
{
  out << feature.id();
  out << feature.attributes();
  if ( feature.constGeometry() )
  {
    out << *( feature.constGeometry() );
  }
  else
  {
    QgsGeometry geometry;
    out << geometry;
  }
  out << feature.isValid();
  return out;
}

QDataStream& operator>>( QDataStream& in, QgsFeature& feature )
{
  QgsFeatureId id;
  QgsGeometry* geometry = new QgsGeometry();
  bool valid;
  QgsAttributes attr;
  in >> id >> attr >> *geometry >> valid;
  feature.setFeatureId( id );
  feature.setGeometry( geometry );
  feature.setAttributes( attr );
  feature.setValid( valid );
  return in;
}
