/***************************************************************************
    qgsadvanceddigitizingtoolsregistry.cpp
                             -------------------
    begin                : July 27 2024
    copyright            : (C) 2024 by Mathieu Pellerin
    email                : mathieu at opengis dot ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsadvanceddigitizingtoolsregistry.h"
#include "qgsadvanceddigitizingtools.h"
#include "qgsapplication.h"

QgsAdvancedDigitizingTool *QgsAdvancedDigitizingToolAbstractMetadata::createTool( QgsMapCanvas *, QgsAdvancedDigitizingDockWidget * )
{
  return nullptr;
}

QgsAdvancedDigitizingTool *QgsAdvancedDigitizingToolMetadata::createTool( QgsMapCanvas *canvas, QgsAdvancedDigitizingDockWidget *cadDockWidget )
{
  return mToolFunc ? mToolFunc( canvas, cadDockWidget ) : QgsAdvancedDigitizingToolAbstractMetadata::createTool( canvas, cadDockWidget );
}

QgsAdvancedDigitizingToolsRegistry::~QgsAdvancedDigitizingToolsRegistry()
{
  qDeleteAll( mTools );
}

void QgsAdvancedDigitizingToolsRegistry::addDefaultTools()
{
  addTool( new QgsAdvancedDigitizingToolMetadata( QStringLiteral( "circlesintersection" ), QObject::tr( "2-Circle Point Intersection" ), QgsApplication::getThemeIcon( QStringLiteral( "/cadtools/circlesintersection.svg" ) ), [=]( QgsMapCanvas *canvas, QgsAdvancedDigitizingDockWidget *cadDockWidget ) -> QgsAdvancedDigitizingTool * {
    return new QgsAdvancedDigitizingCirclesIntersectionTool( canvas, cadDockWidget );
  } ) );
}

bool QgsAdvancedDigitizingToolsRegistry::addTool( QgsAdvancedDigitizingToolAbstractMetadata *toolMetaData )
{
  if ( mTools.contains( toolMetaData->name() ) )
    return false;

  mTools[toolMetaData->name()] = toolMetaData;

  return true;
}


bool QgsAdvancedDigitizingToolsRegistry::removeTool( const QString &name )
{
  if ( !mTools.contains( name ) )
    return false;

  delete mTools.take( name );

  return true;
}

QgsAdvancedDigitizingToolAbstractMetadata *QgsAdvancedDigitizingToolsRegistry::toolMetadata( const QString &name )
{
  if ( !mTools.contains( name ) )
    return nullptr;

  return mTools[name];
}

const QStringList QgsAdvancedDigitizingToolsRegistry::toolMetadataNames() const
{
  return mTools.keys();
}
