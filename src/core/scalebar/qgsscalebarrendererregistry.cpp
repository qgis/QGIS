/***************************************************************************
    qgsscalebarrendererregistry.cpp
    -----------------------
    begin                : March 2020
    copyright            : (C) 2020 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com

 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsscalebarrendererregistry.h"
#include "qgsscalebarrenderer.h"
#include "qgsdoubleboxscalebarrenderer.h"
#include "qgsnumericscalebarrenderer.h"
#include "qgssingleboxscalebarrenderer.h"
#include "qgsticksscalebarrenderer.h"
#include "qgssteppedlinescalebarrenderer.h"
#include "qgshollowscalebarrenderer.h"

QgsScaleBarRendererRegistry::QgsScaleBarRendererRegistry()
{
  addRenderer( new QgsDoubleBoxScaleBarRenderer() );
  addRenderer( new QgsNumericScaleBarRenderer() );
  addRenderer( new QgsSingleBoxScaleBarRenderer() );
  addRenderer( new QgsTicksScaleBarRenderer( QgsTicksScaleBarRenderer::TicksUp ) );
  addRenderer( new QgsTicksScaleBarRenderer( QgsTicksScaleBarRenderer::TicksDown ) );
  addRenderer( new QgsTicksScaleBarRenderer( QgsTicksScaleBarRenderer::TicksMiddle ) );
  addRenderer( new QgsSteppedLineScaleBarRenderer() );
  addRenderer( new QgsHollowScaleBarRenderer() );
}

QgsScaleBarRendererRegistry::~QgsScaleBarRendererRegistry()
{
  qDeleteAll( mRenderers );
}

QStringList QgsScaleBarRendererRegistry::renderers() const
{
  return mRenderers.keys();
}

QStringList QgsScaleBarRendererRegistry::sortedRendererList() const
{
  QStringList ids = mRenderers.keys();

  std::sort( ids.begin(), ids.end(), [ = ]( const QString & a, const QString & b )->bool
  {
    if ( sortKey( a ) < sortKey( b ) )
      return true;
    else if ( sortKey( a ) > sortKey( b ) )
      return false;
    else
    {
      const int res = QString::localeAwareCompare( visibleName( a ), visibleName( b ) );
      if ( res < 0 )
        return true;
      else if ( res > 0 )
        return false;
    }
    return false;
  } );
  return ids;
}

void QgsScaleBarRendererRegistry::addRenderer( QgsScaleBarRenderer *renderer )
{
  if ( !renderer )
    return;

  mRenderers.insert( renderer->id(), renderer );
}

void QgsScaleBarRendererRegistry::removeRenderer( const QString &id )
{
  if ( QgsScaleBarRenderer *renderer = mRenderers.take( id ) )
  {
    delete renderer;
  }
}

QgsScaleBarRenderer *QgsScaleBarRendererRegistry::renderer( const QString &id ) const
{
  if ( mRenderers.contains( id ) )
    return mRenderers.value( id )->clone();

  return nullptr;
}

QString QgsScaleBarRendererRegistry::visibleName( const QString &id ) const
{
  if ( mRenderers.contains( id ) )
    return mRenderers.value( id )->visibleName();

  return QString();
}

int QgsScaleBarRendererRegistry::sortKey( const QString &id ) const
{
  if ( mRenderers.contains( id ) )
    return mRenderers.value( id )->sortKey();

  return 0;
}
