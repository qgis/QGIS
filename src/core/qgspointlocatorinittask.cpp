/***************************************************************************
  qgspointlocatorinittask.cpp
  --------------------------------------
  Date                 : September 2019
  Copyright            : (C) 2019 by Julien Cabieces
  Email                : julien dot cabieces at oslandia dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgspointlocatorinittask.h"
#include "qgspointlocator.h"
#include "qgsvectorlayer.h"

/// @cond PRIVATE

QgsPointLocatorInitTask::QgsPointLocatorInitTask( QgsPointLocator *loc )
  : QgsTask( tr( "Indexing %1" ).arg( loc->layer()->id() ), QgsTask::Silent )
  , mLoc( loc )
{}

bool QgsPointLocatorInitTask::isBuildOK() const
{
  return mBuildOK;
}

bool QgsPointLocatorInitTask::run()
{
  mBuildOK = mLoc->rebuildIndex();
  return true;
}

/// @endcond
