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

QgsVectorLayerEditPassthrough::QgsVectorLayerEditPassthrough( QgsVectorLayer* layer )
    : mModified( false )
{
  L = layer;
}

bool QgsVectorLayerEditPassthrough::isModified() const
{
  return mModified;
}

bool QgsVectorLayerEditPassthrough::addFeature( QgsFeature& f )
{

  QgsFeatureList fl;
  fl << f;
  if ( L->dataProvider()->addFeatures( fl ) )
  {
    f = fl.first();
    emit featureAdded( f.id() );
    mModified = true;
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
    mModified = true;
    return true;
  }
  return false;
}

bool QgsVectorLayerEditPassthrough::deleteFeature( QgsFeatureId fid )
{
  if ( L->dataProvider()->deleteFeatures( QgsFeatureIds() << fid ) )
  {
    emit featureDeleted( fid );
    mModified = true;
    return true;
  }
  return false;
}

bool QgsVectorLayerEditPassthrough::deleteFeatures( const QgsFeatureIds& fids )
{
  if ( L->dataProvider()->deleteFeatures( fids ) )
  {
    Q_FOREACH ( QgsFeatureId fid, fids )
      emit featureDeleted( fid );

    mModified = true;
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
    mModified = true;
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
    mModified = true;
    return true;
  }
  return false;
}

bool QgsVectorLayerEditPassthrough::addAttribute( const QgsField &field )
{
  if ( L->dataProvider()->addAttributes( QList<QgsField>() << field ) )
  {
    emit attributeAdded( L->dataProvider()->fieldNameIndex( field.name() ) );
    mModified = true;
    return true;
  }
  return false;
}

bool QgsVectorLayerEditPassthrough::deleteAttribute( int attr )
{
  if ( L->dataProvider()->deleteAttributes( QgsAttributeIds() << attr ) )
  {
    mModified = true;
    L->editFormConfig()->removeWidgetConfig( attr );
    emit attributeDeleted( attr );
    mModified = true;
    return true;
  }
  return false;
}

bool QgsVectorLayerEditPassthrough::renameAttribute( int attr, const QString& newName )
{
  QgsFieldNameMap map;
  map[ attr ] = newName;
  if ( L->dataProvider()->renameAttributes( map ) )
  {
    mModified = true;
    emit attributeRenamed( attr, newName );
    return true;
  }
  return false;
}

bool QgsVectorLayerEditPassthrough::commitChanges( QStringList& /*commitErrors*/ )
{
  mModified = false;
  return true;
}

void QgsVectorLayerEditPassthrough::rollBack()
{
  mModified = false;
}
