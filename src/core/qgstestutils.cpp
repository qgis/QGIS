/***************************************************************************
                                  qgstestutils.cpp
                              --------------------
    begin                : January 2018
    copyright            : (C) 2018 by Nyall Dawson
    email                : nyall.dawson@gmail.com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgstestutils.h"
#include "qgsvectordataprovider.h"
#include "qgsconnectionpool.h"
#include <QtConcurrentMap>
///@cond PRIVATE
///

static void getFeaturesForProvider( const QPair< std::shared_ptr< QgsAbstractFeatureSource >, QgsFeatureRequest > &pair )
{
  QgsFeatureIterator it = pair.first->getFeatures( pair.second );
  QgsFeature f;
  while ( it.nextFeature( f ) )
  {

  }
}

bool QgsTestUtils::testProviderIteratorThreadSafety( QgsVectorDataProvider *provider, const QgsFeatureRequest &request )
{
  constexpr int JOBS_TO_RUN = 100;
  QList< QPair< std::shared_ptr< QgsAbstractFeatureSource >, QgsFeatureRequest > > jobs;
  jobs.reserve( JOBS_TO_RUN );
  for ( int i = 0; i < JOBS_TO_RUN; ++i )
  {
    jobs.append( qMakePair( std::shared_ptr< QgsAbstractFeatureSource >( provider->featureSource() ), request ) );
  }

  //freaking hammer the provider with a ton of concurrent requests.
  //thread unsafe providers... you better be ready!!!!
  QtConcurrent::blockingMap( jobs, getFeaturesForProvider );

  return true;
}


///@endcond
