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
#include "qgspointlocatorsource.h"

#include "moc_qgspointlocatorinittask.cpp"

/// @cond PRIVATE

QgsPointLocatorInitTask::QgsPointLocatorInitTask( QgsPointLocator *loc )
  // the source id (not loc->layer()->id(), which is null for a non-vector layer such as annotation)
  : QgsTask( tr( "Indexing %1" ).arg( loc->source() ? loc->source()->id() : QString() ), QgsTask::Silent )
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
