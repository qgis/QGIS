/***************************************************************************
    qgsapplegendinterface.cpp
     --------------------------------------
    Date                 : 19-Nov-2009
    Copyright            : (C) 2009 by Andres Manz
    Email                : manz dot andres at gmail dot com
****************************************************************************/
/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
/* $Id$ */

#include "qgsapplegendinterface.h"

#include "qgslegend.h"


QgsAppLegendInterface::QgsAppLegendInterface( QgsLegend * legend )
    : mLegend( legend )
{
  connect( legend, SIGNAL( itemMoved( QModelIndex, QModelIndex ) ), this, SLOT( updateIndex( QModelIndex, QModelIndex ) ) );
}

QgsAppLegendInterface::~QgsAppLegendInterface()
{
}

int QgsAppLegendInterface::addGroup( QString name, bool expand )
{
  return mLegend->addGroup( name, expand );
}

void QgsAppLegendInterface::removeGroup( int groupIndex )
{
  mLegend->removeGroup( groupIndex );
}

void QgsAppLegendInterface::moveLayer( QgsMapLayer * ml, int groupIndex )
{
  mLegend->moveLayer( ml, groupIndex );
}

void QgsAppLegendInterface::updateIndex( QModelIndex oldIndex, QModelIndex newIndex )
{
  if ( mLegend->isLegendGroup( newIndex ) )
  {
    emit groupIndexChanged( oldIndex.row(), newIndex.row() );
  }
}

QStringList QgsAppLegendInterface::groups()
{
  return mLegend->groups();
}
