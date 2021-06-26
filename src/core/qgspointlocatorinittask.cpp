/***************************************************************************
  qgspointlocatorinittask.cpp
  --------------------------------------
  Date                 : September 2019
  Copyright            : (C) 2019 by Julien Cabieces
  Email                : julien dot cabieces at oslandia dot com
 ***************************************************************************
 *                                                                         *
 *   *
 *  
 *        *
 *                                     *
 *                                                                         *
 ***************************************************************************/

#include "qgspointlocatorinittask.h"
#include "qgspointlocator.h"
#include "qgsvectorlayer.h"

/// @cond PRIVATE

QgsPointLocatorInitTask::QgsPointLocatorInitTask( QgsPointLocator *loc )
  : QgsTask( tr( "Indexing %1" ).arg( loc->layer()->id() ), QgsTask::Flags() )
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
