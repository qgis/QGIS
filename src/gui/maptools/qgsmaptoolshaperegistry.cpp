/***************************************************************************
                         qgsmaptoolshaperegistry.cpp
                         ----------------------
    begin                : January 2022
    copyright            : (C) 2022 by Denis Rouzaud
    email                : denis@opengis.ch
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsmaptoolshaperegistry.h"


QgsMapToolShapeRegistry::QgsMapToolShapeRegistry()
{
}

QgsMapToolShapeRegistry::~QgsMapToolShapeRegistry()
{
  qDeleteAll( mMapTools );
  mMapTools.clear();
}

void QgsMapToolShapeRegistry::addMapTool( QgsMapToolShapeMetadata *mapTool )
{
  if ( !mapTool )
    return;

  if ( mapToolMetadata( mapTool->id() ) )
    return;

  mMapTools.append( mapTool );
}

void QgsMapToolShapeRegistry::removeMapTool( const QString &id )
{
  QList<QgsMapToolShapeMetadata *>::iterator it = mMapTools.begin();
  while ( it != mMapTools.end() )
  {
    if ( ( *it )->id() == id )
    {
      delete *it;
      it = mMapTools.erase( it );
    }
    else
    {
      ++it;
    }
  }
}

QgsMapToolShapeMetadata *QgsMapToolShapeRegistry::mapToolMetadata( const QString &id ) const
{
  for ( QgsMapToolShapeMetadata *md : std::as_const( mMapTools ) )
  {
    if ( md->id() == id )
      return md;
  }

  return nullptr;
}

QgsMapToolShapeAbstract *QgsMapToolShapeRegistry::mapTool( const QString &id, QgsMapToolCapture *parentTool ) const
{
  QgsMapToolShapeMetadata *md = mapToolMetadata( id );
  if ( !md )
    return nullptr;

  return md->factory( parentTool );
}
