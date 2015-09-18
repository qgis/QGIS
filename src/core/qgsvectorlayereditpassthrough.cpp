/***************************************************************************
    qgsvectorlayereditpassthrough.cpp
    ---------------------
    begin                : Jan 12 2015
    copyright            : (C) 2015 by Sandro Mani
    email                : manisandro at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsvectorlayereditpassthrough.h"
#include "qgsvectorlayer.h"
#include "qgsvectordataprovider.h"

bool QgsVectorLayerEditPassthrough::addFeature( QgsFeature& f )
{

  if ( L->dataProvider()->addFeatures( QgsFeatureList() << f ) )
  {
    emit featureAdded( f.id() );
    return true;
  }
  return false;
}

bool QgsVectorLayerEditPassthrough::addFeatures( QgsFeatureList& features )
{
  if ( L->dataProvider()->addFeatures( features ) )
  {
    Q_FOREACH ( const QgsFeature& f, features )
    {
      emit featureAdded( f.id() );
    }
    return true;
  }
  return false;
}

bool QgsVectorLayerEditPassthrough::deleteFeature( QgsFeatureId fid )
{
  if ( L->dataProvider()->deleteFeatures( QgsFeatureIds() << fid ) )
  {
    emit featureDeleted( fid );
    return true;
  }
  return false;
}

bool QgsVectorLayerEditPassthrough::changeGeometry( QgsFeatureId fid, QgsGeometry* geom )
{
  QgsGeometryMap geomMap;
  geomMap.insert( fid, *geom );
  if ( L->dataProvider()->changeGeometryValues( geomMap ) )
  {
    emit geometryChanged( fid, *geom );
    return true;
  }
  return false;
}

bool QgsVectorLayerEditPassthrough::changeAttributeValue( QgsFeatureId fid, int field, const QVariant &newValue, const QVariant &/*oldValue*/ )
{
  QgsAttributeMap map;
  map.insert( field, newValue );
  QgsChangedAttributesMap attribMap;
  attribMap.insert( fid, map );
  if ( L->dataProvider()->changeAttributeValues( attribMap ) )
  {
    emit attributeValueChanged( fid, field, newValue );
    return true;
  }
  return false;
}

bool QgsVectorLayerEditPassthrough::addAttribute( const QgsField &field )
{
  if ( L->dataProvider()->addAttributes( QList<QgsField>() << field ) )
  {
    emit attributeAdded( L->dataProvider()->fieldNameIndex( field.name() ) );
    return true;
  }
  return false;
}

bool QgsVectorLayerEditPassthrough::deleteAttribute( int attr )
{
  if ( L->dataProvider()->deleteAttributes( QgsAttributeIds() << attr ) )
  {
    emit attributeDeleted( attr );
    return true;
  }
  return false;
}

bool QgsVectorLayerEditPassthrough::commitChanges( QStringList& /*commitErrors*/ )
{
  return true;
}

void QgsVectorLayerEditPassthrough::rollBack()
{
}
