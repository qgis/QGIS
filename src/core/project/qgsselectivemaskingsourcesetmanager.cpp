/***************************************************************************
    qgsselectivemaskingsourcesetmanager.cpp
    ------------------
    Date                 : January 2026
    Copyright            : (C) 2026 Nyall Dawson
    Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "qgsselectivemaskingsourcesetmanager.h"

#include "qgsproject.h"
#include "qgsreadwritecontext.h"
#include "qgsruntimeprofiler.h"
#include "qgsselectivemaskingsourceset.h"

#include "moc_qgsselectivemaskingsourcesetmanager.cpp"

QgsSelectiveMaskingSourceSetManager::QgsSelectiveMaskingSourceSetManager( QgsProject *project )
  : QgsAbstractProjectStoredObjectManager( project )
{
  // proxy base class generic signals to specific profile signals
  connect( this, &QgsProjectStoredObjectManagerBase::objectAboutToBeAdded, this, &QgsSelectiveMaskingSourceSetManager::setAboutToBeAdded );
  connect( this, &QgsProjectStoredObjectManagerBase::objectAdded, this, &QgsSelectiveMaskingSourceSetManager::setAdded );
  connect( this, &QgsProjectStoredObjectManagerBase::objectAdded, this, &QgsSelectiveMaskingSourceSetManager::changed );
  connect( this, &QgsProjectStoredObjectManagerBase::objectRemoved, this, &QgsSelectiveMaskingSourceSetManager::setRemoved );
  connect( this, &QgsProjectStoredObjectManagerBase::objectRemoved, this, &QgsSelectiveMaskingSourceSetManager::changed );
  connect( this, &QgsProjectStoredObjectManagerBase::objectAboutToBeRemoved, this, &QgsSelectiveMaskingSourceSetManager::setAboutToBeRemoved );
}

QgsSelectiveMaskingSourceSetManager::~QgsSelectiveMaskingSourceSetManager()
{
  clearObjects();
}

bool QgsSelectiveMaskingSourceSetManager::addSet( const QgsSelectiveMaskingSourceSet &set )
{
  return addObject( new QgsSelectiveMaskingSourceSet( set ) );
}

bool QgsSelectiveMaskingSourceSetManager::updateSet( const QgsSelectiveMaskingSourceSet &set )
{
  if ( set.id().isEmpty() )
    return false;

  for ( QgsSelectiveMaskingSourceSet *l : mObjects )
  {
    if ( l->id() == set.id() )
    {
      l->setSources( set.sources() );
      emit changed();
      return true;
    }
  }
  return false;
}

bool QgsSelectiveMaskingSourceSetManager::removeSet( const QString &name )
{
  QgsSelectiveMaskingSourceSet *set = objectByName( name );
  if ( !set )
    return false;

  return removeObject( set );
}

void QgsSelectiveMaskingSourceSetManager::clear()
{
  clearObjects();
}

bool QgsSelectiveMaskingSourceSetManager::renameSet( const QString &oldName, const QString &newName )
{
  if ( objectByName( newName ) )
    return false;

  QgsSelectiveMaskingSourceSet *set = objectByName( oldName );
  if ( !set )
    return false;
  set->setName( newName );
  emit setRenamed( oldName, newName );
  emit changed();
  return true;
}

QVector<QgsSelectiveMaskingSourceSet> QgsSelectiveMaskingSourceSetManager::sets() const
{
  QVector<QgsSelectiveMaskingSourceSet> res;
  res.reserve( mObjects.size() );
  for ( const QgsSelectiveMaskingSourceSet *set : mObjects )
  {
    res.append( *set );
  }
  return res;
}

QgsSelectiveMaskingSourceSet QgsSelectiveMaskingSourceSetManager::setById( const QString &id ) const
{
  for ( QgsSelectiveMaskingSourceSet *l : mObjects )
  {
    if ( l->id() == id )
      return *l;
  }
  return QgsSelectiveMaskingSourceSet();
}

QgsSelectiveMaskingSourceSet QgsSelectiveMaskingSourceSetManager::setByName( const QString &name ) const
{
  QgsSelectiveMaskingSourceSet *set = objectByName( name );
  if ( set )
    return *set;

  return QgsSelectiveMaskingSourceSet();
}

bool QgsSelectiveMaskingSourceSetManager::readXml( const QDomElement &element, const QDomDocument &doc, const QgsReadWriteContext &context )
{
  clear();

  QDomElement setsElem = element;
  if ( element.tagName() != "SelectiveMaskingSourceSets"_L1 )
  {
    setsElem = element.firstChildElement( u"SelectiveMaskingSourceSets"_s );
  }

  QgsScopedRuntimeProfile runtimeProfile( tr( "Creating selective masking source sets" ), u"projectload"_s );

  // restore sets
  const QDomNodeList setsNodes = setsElem.childNodes();
  bool result = true;
  for ( int i = 0; i < setsNodes.size(); ++i )
  {
    if ( setsNodes.at( i ).nodeName() != "selectiveMaskingSourceSet"_L1 )
      continue;

    QgsSelectiveMaskingSourceSet set;
    if ( !set.readXml( setsNodes.at( i ).toElement(), doc, context ) )
    {
      result = false;
      continue;
    }
    if ( !addSet( set ) )
    {
      result = false;
    }
  }

  emit changed();

  return result;
}

QDomElement QgsSelectiveMaskingSourceSetManager::writeXml( QDomDocument &doc, const QgsReadWriteContext &context ) const
{
  QDomElement setsElem = doc.createElement( u"SelectiveMaskingSourceSets"_s );
  for ( const QgsSelectiveMaskingSourceSet *l : mObjects )
  {
    QDomElement setElem = l->writeXml( doc, context );
    setsElem.appendChild( setElem );
  }
  return setsElem;
}

QString QgsSelectiveMaskingSourceSetManager::generateUniqueTitle() const
{
  QStringList names;
  names.reserve( mObjects.size() );
  for ( QgsSelectiveMaskingSourceSet *l : mObjects )
  {
    names << l->name();
  }
  QString name;
  int id = 1;
  while ( name.isEmpty() || names.contains( name ) )
  {
    name = tr( "Masking Source Set %1" ).arg( id );
    id++;
  }
  return name;
}
