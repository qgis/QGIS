/***************************************************************************
    qgselevationprofilemanager.cpp
    ------------------
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
#include "qgselevationprofilemanager.h"
#include "moc_qgselevationprofilemanager.cpp"
#include "qgselevationprofile.h"
#include "qgsproject.h"
#include "qgsreadwritecontext.h"
#include "qgsruntimeprofiler.h"

QgsElevationProfileManager::QgsElevationProfileManager( QgsProject *project )
  : QgsAbstractProjectStoredObjectManager( project )
{
  // proxy base class generic signals to specific profile signals
  connect( this, &QgsProjectStoredObjectManagerBase::objectAboutToBeAdded, this, &QgsElevationProfileManager::profileAboutToBeAdded );
  connect( this, &QgsProjectStoredObjectManagerBase::objectAdded, this, &QgsElevationProfileManager::profileAdded );
  connect( this, &QgsProjectStoredObjectManagerBase::objectRemoved, this, &QgsElevationProfileManager::profileRemoved );
  connect( this, &QgsProjectStoredObjectManagerBase::objectAboutToBeRemoved, this, &QgsElevationProfileManager::profileAboutToBeRemoved );
}

QgsElevationProfileManager::~QgsElevationProfileManager() = default;

bool QgsElevationProfileManager::addProfile( QgsElevationProfile *profile )
{
  return addObject( profile );
}

bool QgsElevationProfileManager::removeProfile( QgsElevationProfile *profile )
{
  return removeObject( profile );
}

void QgsElevationProfileManager::clear()
{
  clearObjects();
}

QList<QgsElevationProfile *> QgsElevationProfileManager::profiles() const
{
  return mObjects;
}

QgsElevationProfile *QgsElevationProfileManager::profileByName( const QString &name ) const
{
  return objectByName( name );
}

bool QgsElevationProfileManager::readXml( const QDomElement &element, const QDomDocument &doc, const QgsReadWriteContext &context )
{
  clear();

  QDomElement profilesElem = element;
  if ( element.tagName() != QLatin1String( "ElevationProfiles" ) )
  {
    profilesElem = element.firstChildElement( QStringLiteral( "ElevationProfiles" ) );
  }

  QgsScopedRuntimeProfile runtimeProfile( tr( "Creating elevation profiles" ), QStringLiteral( "projectload" ) );

  // restore profiles
  const QDomNodeList profileNodes = profilesElem.childNodes();
  bool result = true;
  for ( int i = 0; i < profileNodes.size(); ++i )
  {
    if ( profileNodes.at( i ).nodeName() != QLatin1String( "ElevationProfile" ) )
      continue;

    const QString profileName = profileNodes.at( i ).toElement().attribute( QStringLiteral( "name" ) );
    QgsScopedRuntimeProfile profile( profileName, QStringLiteral( "projectload" ) );

    auto l = std::make_unique< QgsElevationProfile>( mProject );
    if ( !l->readXml( profileNodes.at( i ).toElement(), doc, context ) )
    {
      result = false;
      continue;
    }
    if ( !addProfile( l.release() ) )
    {
      result = false;
    }
  }

  return result;
}

QDomElement QgsElevationProfileManager::writeXml( QDomDocument &doc, const QgsReadWriteContext &context ) const
{
  QDomElement profilesElem = doc.createElement( QStringLiteral( "ElevationProfiles" ) );
  for ( QgsElevationProfile *l : mObjects )
  {
    QDomElement profileElem = l->writeXml( doc, context );
    profilesElem.appendChild( profileElem );
  }
  return profilesElem;
}

QString QgsElevationProfileManager::generateUniqueTitle() const
{
  QStringList names;
  names.reserve( mObjects.size() );
  for ( QgsElevationProfile *l : mObjects )
  {
    names << l->name();
  }
  QString name;
  int id = 1;
  while ( name.isEmpty() || names.contains( name ) )
  {
    name = tr( "Elevation Profile %1" ).arg( id );
    id++;
  }
  return name;
}

void QgsElevationProfileManager::setupObjectConnections( QgsElevationProfile *profile )
{
  if ( profile )
  {
    connect( profile, &QgsElevationProfile::nameChanged, this, [this, profile]( const QString & newName )
    {
      emit profileRenamed( profile, newName );
    } );
  }
}
