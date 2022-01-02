/***************************************************************************
    qgs3dviewsmanager.cpp
    ------------------
    Date                 : December 2021
    Copyright            : (C) 2021 Belgacem Nedjima
    Email                : gb underscore nedjima at esi dot dz
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgs3dviewsmanager.h"
#include "qgsproject.h"
#include "qgsstyleentityvisitor.h"

Qgs3DViewsManager::Qgs3DViewsManager( QgsProject *project )
  : QObject( project )
{

}

bool Qgs3DViewsManager::readXml( const QDomElement &element, const QDomDocument &doc )
{
  Q_UNUSED( doc );
  clear();

  QgsReadWriteContext readWriteContext;
  readWriteContext.setPathResolver( QgsProject::instance()->pathResolver() );
  QDomElement elem3DMaps = element.firstChildElement( QStringLiteral( "mapViewDocks3D" ) );
  if ( !elem3DMaps.isNull() )
  {
    QDomElement elem3DMap = elem3DMaps.firstChildElement( QStringLiteral( "view" ) );
    while ( !elem3DMap.isNull() )
    {
      QString mapName = elem3DMap.attribute( QStringLiteral( "name" ) );
      m3DMapViewsDom.insert( mapName, elem3DMap );

      elem3DMap = elem3DMap.nextSiblingElement( QStringLiteral( "view" ) );
    }
  }

  return true;
}

QDomElement Qgs3DViewsManager::writeXml( QDomDocument &doc ) const
{
  QDomElement dom = doc.createElement( "mapViewDocks3D" );
  for ( QDomElement d : m3DMapViewsDom.values() )
    dom.appendChild( d );
  return dom;
}

void Qgs3DViewsManager::clear()
{
  m3DMapViewsDom.clear();
  emit viewsListChanged();
}

QDomElement Qgs3DViewsManager::get3DViewSettings( const QString &name ) const
{
  return m3DMapViewsDom.value( name, QDomElement() );
}

QList<QDomElement> Qgs3DViewsManager::get3DViews() const
{
  return m3DMapViewsDom.values();
}

void Qgs3DViewsManager::register3DViewSettings( const QString &name, const QDomElement &dom )
{
  m3DMapViewsDom.insert( name, dom );
  emit viewsListChanged();
}

QStringList Qgs3DViewsManager::get3DViewsNames() const
{
  return m3DMapViewsDom.keys();
}

void Qgs3DViewsManager::remove3DView( const QString &name )
{
  m3DMapViewsDom.remove( name );
  emit viewsListChanged();
}

void Qgs3DViewsManager::rename3DView( const QString &oldTitle, const QString &newTitle )
{
  QDomElement elem = m3DMapViewsDom.value( oldTitle );
  m3DMapViewsDom.remove( oldTitle );
  m3DMapViewsDom[ newTitle ] = elem;
  emit viewsListChanged();
}

void Qgs3DViewsManager::viewClosed( const QString &name )
{
  if ( m3DMapViewsDom.contains( name ) )
    m3DMapViewsDom[ name ].setAttribute( "isOpen", 0 );
}

void Qgs3DViewsManager::viewOpened( const QString &name )
{
  if ( m3DMapViewsDom.contains( name ) )
    m3DMapViewsDom[ name ].setAttribute( "isOpen", 1 );
}
