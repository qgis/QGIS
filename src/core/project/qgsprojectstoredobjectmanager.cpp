/***************************************************************************
    qgsprojectstoredobjectmanager.cpp
    --------------------
    Date                 : July 2025
    Copyright            : (C) 2025 Nyall Dawson
    Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsprojectstoredobjectmanager.h"
#include "moc_qgsprojectstoredobjectmanager.cpp"
#include "qgsproject.h"

//
// QgsProjectStoredObjectManagerBase
//

QgsProjectStoredObjectManagerBase::QgsProjectStoredObjectManagerBase( QgsProject *project )
  : QObject( project )
  , mProject( project )
{

}

void QgsProjectStoredObjectManagerBase::markProjectDirty()
{
  mProject->setDirty( true );
}

//
// QgsAbstractProjectStoredObjectManager
//

template<class T>
QgsAbstractProjectStoredObjectManager<T>::QgsAbstractProjectStoredObjectManager( QgsProject *project )
  : QgsProjectStoredObjectManagerBase( project )
{

}

template<class T>
QgsAbstractProjectStoredObjectManager<T>::~QgsAbstractProjectStoredObjectManager()
{
  clearObjects();
}

template<class T>
QList<T *> QgsAbstractProjectStoredObjectManager<T>::objects() const
{
  return mObjects;
}

template<class T>
T *QgsAbstractProjectStoredObjectManager<T>::objectByName( const QString &name ) const
{
  for ( T *l : mObjects )
  {
    if ( l->name() == name )
      return l;
  }
  return nullptr;
}

template<class T>
void QgsAbstractProjectStoredObjectManager<T>::clearObjects()
{
  const QList< T * > objects = mObjects;
  for ( T *l : objects )
  {
    removeObject( l );
  }
}

template<class T>
bool QgsAbstractProjectStoredObjectManager<T>::addObject( T *object )
{
  if ( !object || mObjects.contains( object ) )
    return false;

  // check for duplicate name
  const QList<T *> constObjects = mObjects;
  for ( T *l : constObjects )
  {
    if ( l->name() == object->name() )
    {
      delete object;
      return false;
    }
  }

  setupObjectConnections( object );

  emit objectAboutToBeAdded( object->name() );
  mObjects << object;
  emit objectAdded( object->name() );
  markProjectDirty();
  return true;
}

template<class T>
bool QgsAbstractProjectStoredObjectManager<T>::removeObject( T *object )
{
  if ( !object )
    return false;

  if ( !mObjects.contains( object ) )
    return false;

  QString name = object->name();
  emit objectAboutToBeRemoved( name );
  mObjects.removeAll( object );
  delete object;
  emit objectRemoved( name );
  markProjectDirty();
  return true;
}

template<class T>
void QgsAbstractProjectStoredObjectManager<T>::setupObjectConnections( T * )
{
}

#include "qgsmasterlayoutinterface.h"

template class QgsAbstractProjectStoredObjectManager<QgsMasterLayoutInterface>;
