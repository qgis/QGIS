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

#include "memoryprovider.h"

#include "qgsfeature.h"
#include "qgsfield.h"
#include "qgsgeometry.h"
#include "qgslogger.h"
#include "qgsspatialrefsys.h"


static const QString TEXT_PROVIDER_KEY = "memory";
static const QString TEXT_PROVIDER_DESCRIPTION = "Memory provider";

QgsMemoryProvider::QgsMemoryProvider(QString uri)
  : QgsVectorDataProvider(uri)
{
  if (uri == "Point")
    mWkbType = QGis::WKBPoint;
  else if (uri == "LineString")
    mWkbType = QGis::WKBLineString;
  else if (uri == "Polygon")
    mWkbType = QGis::WKBPolygon;
  else if (uri == "MultiPoint")
    mWkbType = QGis::WKBMultiPoint;
  else if (uri == "MultiLineString")
    mWkbType = QGis::WKBMultiLineString;
  else if (uri == "MultiPolygon")
    mWkbType = QGis::WKBMultiPolygon;
  else
    mWkbType = QGis::WKBUnknown;
    
  mNextFeatureId = 1;
}

QgsMemoryProvider::~QgsMemoryProvider()
{
}

QString QgsMemoryProvider::storageType() const
{
  return "Memory storage";
}

bool QgsMemoryProvider::getNextFeature(QgsFeature& feature)
{
  bool hasFeature = FALSE;
  while (mSelectIterator != mFeatures.end())
  {
    if (mSelectRect.isEmpty())
    {
      hasFeature = TRUE;
      break;
    }
    else
    {
      // TODO: could use some less accurate test when not using mSelectUseIntersect (e.g. spatial index)
      if (feature.geometry()->intersects(mSelectRect))
      {
        hasFeature = TRUE;
        break;
      }
    }
    
    mSelectIterator++;
  }
    
  // copy feature
  if (hasFeature)
  {
    feature = mSelectIterator.value();
    mSelectIterator++;
  }
  
  return hasFeature;
}

bool QgsMemoryProvider::getFeatureAtId(int featureId,
                              QgsFeature& feature,
                              bool fetchGeometry,
                              QgsAttributeList fetchAttributes)
{
  QgsFeatureMap::iterator it = mFeatures.find(featureId);
  
  if (it == mFeatures.end())
    return FALSE;
  
  feature = *it;
  return TRUE;
}


void QgsMemoryProvider::select(QgsAttributeList fetchAttributes,
                                      QgsRect rect,
                                      bool fetchGeometry,
                                      bool useIntersect)
{
  mSelectAttrs = fetchAttributes;
  mSelectRect = rect;
  mSelectGeometry = fetchGeometry;
  mSelectUseIntersect = useIntersect;
  
  reset();
}

void QgsMemoryProvider::reset()
{
  mSelectIterator = mFeatures.begin();
}


QgsRect QgsMemoryProvider::extent()
{
  return mExtent;
}

QGis::WKBTYPE QgsMemoryProvider::geometryType() const
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
  return (mWkbType != QGis::WKBUnknown);
}

QgsSpatialRefSys QgsMemoryProvider::getSRS()
{
  // TODO: make provider projection-aware
  return QgsSpatialRefSys(); // return default SRS
}


bool QgsMemoryProvider::addFeatures(QgsFeatureList & flist)
{
  // TODO: sanity checks of fields and geometries
  for (QgsFeatureList::iterator it = flist.begin(); it != flist.end(); ++it)
  {
    mFeatures[mNextFeatureId] = *it;
    mFeatures[mNextFeatureId].setFeatureId(mNextFeatureId);
    mNextFeatureId++;
  }
  
  updateExtent();
  
  return TRUE;
}

bool QgsMemoryProvider::deleteFeatures(const QgsFeatureIds & id)
{
  for (QgsFeatureIds::const_iterator it = id.begin(); it != id.end(); ++it)
    mFeatures.remove(*it);
  
  updateExtent();
  
  return TRUE;
}

bool QgsMemoryProvider::addAttributes(const QgsNewAttributesMap & attributes)
{
  for (QgsNewAttributesMap::const_iterator it = attributes.begin(); it != attributes.end(); ++it)
  {
    QString name = it.key();
    QString typeName = it.value();
    QVariant::Type type;
    if (typeName == "int")
      type = QVariant::Int;
    else if (typeName == "double")
      type = QVariant::Double;
    else if (typeName == "string")
      type = QVariant::String;
    else
    {
      QgsDebugMsg("Field type not supported: "+type);
      continue;
    }
    
    // add new field as a last one
    int nextId = -1;
    for (QgsFieldMap::iterator it2 = mFields.begin(); it2 != mFields.end(); ++it2)
      if (it2.key() > nextId) nextId = it2.key();
    mFields[nextId+1] = QgsField(name, type, typeName);
  }
  return TRUE;
}

bool QgsMemoryProvider::deleteAttributes(const QgsAttributeIds& attributes)
{
  for (QgsAttributeIds::const_iterator it = attributes.begin(); it != attributes.end(); ++it)
    mFields.remove(*it);
  return TRUE;
}

bool QgsMemoryProvider::changeAttributeValues(const QgsChangedAttributesMap & attr_map)
{
  // TODO: change attribute values
  return FALSE;
}

bool QgsMemoryProvider::changeGeometryValues(QgsGeometryMap & geometry_map)
{
  // TODO: change geometries
  
  updateExtent();
  
  return FALSE;
}

int QgsMemoryProvider::capabilities() const
{
  return AddFeatures | DeleteFeatures | ChangeGeometries |
      ChangeAttributeValues | AddAttributes | DeleteAttributes |
      SelectAtId | SelectGeometryAtId | RandomSelectGeometryAtId | SequentialSelectGeometryAtId;
}


void QgsMemoryProvider::updateExtent()
{
  if (mFeatures.count() == 0)
  {
    mExtent = QgsRect();
  }
  else
  {
    mExtent = mFeatures.begin().value().geometry()->boundingBox();
    for (QgsFeatureMap::iterator it = mFeatures.begin(); it != mFeatures.end(); ++it)
      mExtent.unionRect(it.value().geometry()->boundingBox());
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
QGISEXTERN QgsMemoryProvider *classFactory(const QString *uri)
{
  return new QgsMemoryProvider(*uri);
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
