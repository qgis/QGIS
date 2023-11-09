/***************************************************************************
    qgsmapviewsmanager.cpp
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

#include "qgsmapviewsmanager.h"
#include "qgsproject.h"

QgsMapViewsManager::QgsMapViewsManager( QgsProject *project )
  : QObject( project )
{

}

bool QgsMapViewsManager::readXml( const QDomElement &element, const QDomDocument &doc )
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

QDomElement QgsMapViewsManager::writeXml( QDomDocument &doc ) const
{
  QDomElement dom = doc.createElement( "mapViewDocks3D" );
  for ( QDomElement d : m3DMapViewsDom )
    dom.appendChild( d );
  return dom;
}

void QgsMapViewsManager::clear()
{
  m3DMapViewsDom.clear();
  emit views3DListChanged();
}

QDomElement QgsMapViewsManager::get3DViewSettings( const QString &name ) const
{
  return m3DMapViewsDom.value( name, QDomElement() );
}

QList<QDomElement> QgsMapViewsManager::get3DViews() const
{
  return m3DMapViewsDom.values();
}

void QgsMapViewsManager::register3DViewSettings( const QString &name, const QDomElement &dom )
{
  m3DMapViewsDom.insert( name, dom );
  emit views3DListChanged();
}

QStringList QgsMapViewsManager::get3DViewsNames() const
{
  return m3DMapViewsDom.keys();
}

void QgsMapViewsManager::remove3DView( const QString &name )
{
  m3DMapViewsDom.remove( name );
  emit views3DListChanged();
}

void QgsMapViewsManager::rename3DView( const QString &oldTitle, const QString &newTitle )
{
  QDomElement elem = m3DMapViewsDom.value( oldTitle );
  m3DMapViewsDom.remove( oldTitle );
  m3DMapViewsDom[ newTitle ] = elem;
  m3DMapViewsDom[ newTitle ].setAttribute( QStringLiteral( "name" ), newTitle );
  emit views3DListChanged();
}

void QgsMapViewsManager::set3DViewInitiallyVisible( const QString &name, bool visible )
{
  if ( m3DMapViewsDom.contains( name ) )
  {
    m3DMapViewsDom[ name ].setAttribute( QStringLiteral( "isOpen" ), visible );
  }
}

bool QgsMapViewsManager::is3DViewOpen( const QString &name )
{
  return m3DMapViewsDom.value( name, QDomElement() ).attribute( QStringLiteral( "isOpen" ), QStringLiteral( "1" ) ).toInt() == 1;
}
