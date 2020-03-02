/***************************************************************************
                             qgsmodelgraphicsscene.cpp
                             ----------------------------------
    Date                 : March 2020
    Copyright            : (C) 2020 Nyall Dawson
    Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsmodelgraphicsscene.h"

///@cond NOT_STABLE

QgsModelGraphicsScene::QgsModelGraphicsScene( QObject *parent )
  : QGraphicsScene( parent )
{

}

void QgsModelGraphicsScene::setFlag( QgsModelGraphicsScene::Flag flag, bool on )
{
  if ( on )
    mFlags |= flag;
  else
    mFlags &= ~flag;
}

///@endcond

