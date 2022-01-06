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
      mMapViewsDom.insert( mapName, elem3DMap );

      elem3DMap = elem3DMap.nextSiblingElement( QStringLiteral( "view" ) );
    }
  }

  return true;
}

QDomElement QgsMapViewsManager::writeXml( QDomDocument &doc ) const
{
  QDomElement dom = doc.createElement( "mapViewDocks3D" );
  for ( QDomElement d : mMapViewsDom.values() )
    dom.appendChild( d );
  return dom;
}

void QgsMapViewsManager::clear()
{
  mMapViewsDom.clear();
  emit viewsListChanged();
}

QDomElement QgsMapViewsManager::getViewSettings( const QString &name ) const
{
  return mMapViewsDom.value( name, QDomElement() );
}

QList<QDomElement> QgsMapViewsManager::getViews() const
{
  return mMapViewsDom.values();
}

void QgsMapViewsManager::registerViewSettings( const QString &name, const QDomElement &dom )
{
  mMapViewsDom.insert( name, dom );
  emit viewsListChanged();
}

QStringList QgsMapViewsManager::getViewsNames() const
{
  return mMapViewsDom.keys();
}

void QgsMapViewsManager::removeView( const QString &name )
{
  mMapViewsDom.remove( name );
  emit viewsListChanged();
}

void QgsMapViewsManager::renameView( const QString &oldTitle, const QString &newTitle )
{
  QDomElement elem = mMapViewsDom.value( oldTitle );
  mMapViewsDom.remove( oldTitle );
  mMapViewsDom[ newTitle ] = elem;
  mMapViewsDom[ newTitle ].setAttribute( QStringLiteral( "name" ), newTitle );
  emit viewsListChanged();
}

void QgsMapViewsManager::setViewInitiallyVisible( const QString &name, bool visible )
{
  if ( mMapViewsDom.contains( name ) )
  {
    mMapViewsDom[ name ].setAttribute( QStringLiteral( "isOpen" ), visible );
  }
}

bool QgsMapViewsManager::isViewOpen( const QString &name )
{
  return mMapViewsDom.value( name, QDomElement() ).attribute( QStringLiteral( "isOpen" ), QStringLiteral( "1" ) ).toInt() == 1;
}
