/***************************************************************************
    memoryprovider.cpp - provider with storage in memory
    ------------------
    begin                : June 2008
    copyright            : (C) 2008 by Martin Dobias
    email                : wonder.sk at gmail.com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsmemoryprovider.h"

#include "qgsfeature.h"
#include "qgsfield.h"
#include "qgsgeometry.h"
#include "qgslogger.h"
#include "qgsspatialindex.h"
#include "qgscoordinatereferencesystem.h"


static const QString TEXT_PROVIDER_KEY = "memory";
static const QString TEXT_PROVIDER_DESCRIPTION = "Memory provider";

QgsMemoryProvider::QgsMemoryProvider( QString uri )
    : QgsVectorDataProvider( uri ),
    mSelectRectGeom( NULL ),
    mSpatialIndex( NULL )
{
  if ( uri == "Point" )
    mWkbType = QGis::WKBPoint;
  else if ( uri == "LineString" )
    mWkbType = QGis::WKBLineString;
  else if ( uri == "Polygon" )
    mWkbType = QGis::WKBPolygon;
  else if ( uri == "MultiPoint" )
    mWkbType = QGis::WKBMultiPoint;
  else if ( uri == "MultiLineString" )
    mWkbType = QGis::WKBMultiLineString;
  else if ( uri == "MultiPolygon" )
    mWkbType = QGis::WKBMultiPolygon;
  else
    mWkbType = QGis::WKBUnknown;

  mNextFeatureId = 1;
}

QgsMemoryProvider::~QgsMemoryProvider()
{
  delete mSpatialIndex;
  delete mSelectRectGeom;
}

QString QgsMemoryProvider::storageType() const
{
  return "Memory storage";
}

bool QgsMemoryProvider::nextFeature( QgsFeature& feature )
{
  feature.setValid( false );
  bool hasFeature = FALSE;

  // option 1: using spatial index
  if ( mSelectUsingSpatialIndex )
  {
    while ( mSelectSI_Iterator != mSelectSI_Features.end() )
    {
      // do exact check in case we're doing intersection
      if ( mSelectUseIntersect )
      {
        if ( mFeatures[*mSelectSI_Iterator].geometry()->intersects( mSelectRectGeom ) )
          hasFeature = TRUE;
      }
      else
        hasFeature = TRUE;

      if ( hasFeature )
        break;

      mSelectSI_Iterator++;
    }

    // copy feature
    if ( hasFeature )
    {
      feature = mFeatures[*mSelectSI_Iterator];
      mSelectSI_Iterator++;
    }
    return hasFeature;
  }

  // option 2: not using spatial index
  while ( mSelectIterator != mFeatures.end() )
  {
    if ( mSelectRect.isEmpty() )
    {
      // selection rect empty => using all features
      hasFeature = TRUE;
    }
    else
    {
      if ( mSelectUseIntersect )
      {
        // using exact test when checking for intersection
        if ( mSelectIterator->geometry()->intersects( mSelectRectGeom ) )
          hasFeature = TRUE;
      }
      else
      {
        // check just bounding box against rect when not using intersection
        if ( mSelectIterator->geometry()->boundingBox().intersects( mSelectRect ) )
          hasFeature = TRUE;
      }
    }

    if ( hasFeature )
      break;

    mSelectIterator++;
  }

  // copy feature
  if ( hasFeature )
  {
    feature = mSelectIterator.value();
    mSelectIterator++;
    feature.setValid( true );
  }

  return hasFeature;
}


bool QgsMemoryProvider::featureAtId( int featureId,
                                     QgsFeature& feature,
                                     bool fetchGeometry,
                                     QgsAttributeList fetchAttributes )
{
  QgsFeatureMap::iterator it = mFeatures.find( featureId );

  if ( it == mFeatures.end() )
    return FALSE;

  feature = *it;
  return TRUE;
}


void QgsMemoryProvider::select( QgsAttributeList fetchAttributes,
                                QgsRectangle rect,
                                bool fetchGeometry,
                                bool useIntersect )
{
  mSelectAttrs = fetchAttributes;
  mSelectRect = rect;
  delete mSelectRectGeom;
  mSelectRectGeom = QgsGeometry::fromRect( rect );
  mSelectGeometry = fetchGeometry;
  mSelectUseIntersect = useIntersect;

  // if there's spatial index, use it!
  // (but don't use it when selection rect is not specified)
  if ( mSpatialIndex && !mSelectRect.isEmpty() )
  {
    mSelectUsingSpatialIndex = TRUE;
    mSelectSI_Features = mSpatialIndex->intersects( rect );
    QgsDebugMsg( "Features returned by spatial index: " + QString::number( mSelectSI_Features.count() ) );
  }
  else
  {
    mSelectUsingSpatialIndex = FALSE;
    mSelectSI_Features.clear();
  }

  rewind();
}

void QgsMemoryProvider::rewind()
{
  if ( mSelectUsingSpatialIndex )
    mSelectSI_Iterator = mSelectSI_Features.begin();
  else
    mSelectIterator = mFeatures.begin();
}


QgsRectangle QgsMemoryProvider::extent()
{
  return mExtent;
}

QGis::WkbType QgsMemoryProvider::geometryType() const
{
  return mWkbType;
}

long QgsMemoryProvider::featureCount() const
{
  return mFeatures.count();
}

uint QgsMemoryProvider::fieldCount() const
{
  return mFields.count();
}


const QgsFieldMap & QgsMemoryProvider::fields() const
{
  return mFields;
}

bool QgsMemoryProvider::isValid()
{
  return ( mWkbType != QGis::WKBUnknown );
}

QgsCoordinateReferenceSystem QgsMemoryProvider::crs()
{
  // TODO: make provider projection-aware
  return QgsCoordinateReferenceSystem(); // return default CRS
}


bool QgsMemoryProvider::addFeatures( QgsFeatureList & flist )
{
  // TODO: sanity checks of fields and geometries
  for ( QgsFeatureList::iterator it = flist.begin(); it != flist.end(); ++it )
  {
    mFeatures[mNextFeatureId] = *it;
    QgsFeature& newfeat = mFeatures[mNextFeatureId];
    newfeat.setFeatureId( mNextFeatureId );

    // update spatial index
    if ( mSpatialIndex )
      mSpatialIndex->insertFeature( newfeat );

    mNextFeatureId++;
  }


  updateExtent();

  return TRUE;
}

bool QgsMemoryProvider::deleteFeatures( const QgsFeatureIds & id )
{
  for ( QgsFeatureIds::const_iterator it = id.begin(); it != id.end(); ++it )
  {
    QgsFeatureMap::iterator fit = mFeatures.find( *it );

    // check whether such feature exists
    if ( fit == mFeatures.end() )
      continue;

    // update spatial index
    if ( mSpatialIndex )
      mSpatialIndex->deleteFeature( *fit );

    mFeatures.erase( fit );
  }

  updateExtent();

  return TRUE;
}

bool QgsMemoryProvider::addAttributes( const QList<QgsField> &attributes )
{
  for ( QList<QgsField>::const_iterator it = attributes.begin(); it != attributes.end(); ++it )
  {
    switch ( it->type() )
    {
      case QVariant::Int:
      case QVariant::Double:
      case QVariant::String:
        break;
      default:
        QgsDebugMsg( "Field type not supported: " + it->typeName() );
        continue;
    }

    // add new field as a last one
    int nextId = -1;
    for ( QgsFieldMap::iterator it2 = mFields.begin(); it2 != mFields.end(); ++it2 )
      if ( it2.key() > nextId ) nextId = it2.key();
    mFields[nextId+1] = *it;
  }
  return TRUE;
}

bool QgsMemoryProvider::deleteAttributes( const QgsAttributeIds& attributes )
{
  for ( QgsAttributeIds::const_iterator it = attributes.begin(); it != attributes.end(); ++it )
    mFields.remove( *it );
  return TRUE;
}

bool QgsMemoryProvider::changeAttributeValues( const QgsChangedAttributesMap & attr_map )
{
  for ( QgsChangedAttributesMap::const_iterator it = attr_map.begin(); it != attr_map.end(); ++it )
  {
    QgsFeatureMap::iterator fit = mFeatures.find( it.key() );
    if ( fit == mFeatures.end() )
      continue;

    const QgsAttributeMap& attrs = it.value();
    for ( QgsAttributeMap::const_iterator it2 = attrs.begin(); it2 != attrs.end(); ++it2 )
      fit->changeAttribute( it2.key(), it2.value() );
  }
  return TRUE;
}

bool QgsMemoryProvider::changeGeometryValues( QgsGeometryMap & geometry_map )
{
  for ( QgsGeometryMap::const_iterator it = geometry_map.begin(); it != geometry_map.end(); ++it )
  {
    QgsFeatureMap::iterator fit = mFeatures.find( it.key() );
    if ( fit == mFeatures.end() )
      continue;

    // update spatial index
    if ( mSpatialIndex )
      mSpatialIndex->deleteFeature( *fit );

    fit->setGeometry( it.value() );

    // update spatial index
    if ( mSpatialIndex )
      mSpatialIndex->insertFeature( *fit );
  }

  updateExtent();

  return TRUE;
}

bool QgsMemoryProvider::createSpatialIndex()
{
  if ( !mSpatialIndex )
  {
    mSpatialIndex = new QgsSpatialIndex();

    // add existing features to index
    for ( QgsFeatureMap::iterator it = mFeatures.begin(); it != mFeatures.end(); ++it )
    {
      mSpatialIndex->insertFeature( *it );
    }
  }
  return TRUE;
}

int QgsMemoryProvider::capabilities() const
{
  return AddFeatures | DeleteFeatures | ChangeGeometries |
         ChangeAttributeValues | AddAttributes | DeleteAttributes | CreateSpatialIndex |
         SelectAtId | SelectGeometryAtId;
}


void QgsMemoryProvider::updateExtent()
{
  if ( mFeatures.count() == 0 )
  {
    mExtent = QgsRectangle();
  }
  else
  {
    mExtent = mFeatures.begin().value().geometry()->boundingBox();
    for ( QgsFeatureMap::iterator it = mFeatures.begin(); it != mFeatures.end(); ++it )
      mExtent.unionRect( it.value().geometry()->boundingBox() );
  }
}



// --------------------------------

QString  QgsMemoryProvider::name() const
{
  return TEXT_PROVIDER_KEY;
}

QString  QgsMemoryProvider::description() const
{
  return TEXT_PROVIDER_DESCRIPTION;
}

// --------------------------------


/**
 * Class factory to return a pointer to a newly created
 * QgsMemoryProvider object
 */
QGISEXTERN QgsMemoryProvider *classFactory( const QString *uri )
{
  return new QgsMemoryProvider( *uri );
}

/** Required key function (used to map the plugin to a data store type)
 */
QGISEXTERN QString providerKey()
{
  return TEXT_PROVIDER_KEY;
}

/**
 * Required description function
 */
QGISEXTERN QString description()
{
  return TEXT_PROVIDER_DESCRIPTION;
}

/**
 * Required isProvider function. Used to determine if this shared library
 * is a data provider plugin
 */
QGISEXTERN bool isProvider()
{
  return true;
}
