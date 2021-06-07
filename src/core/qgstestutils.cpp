/***************************************************************************
                                  qgstestutils.cpp
                              --------------------
    begin                : January 2018
    copyright            : (C) 2018 by Nyall Dawson
    email                : nyall.dawson@gmail.com
 ***************************************************************************
 *                                                                         *
 *   *
 *  
 *        *
 *                                     *
 *                                                                         *
 ***************************************************************************/

#include "qgstestutils.h"
#include "qgsvectordataprovider.h"
#include "qgsconnectionpool.h"
#include <QtConcurrentMap>
///@cond PRIVATE
///

static void getFeaturesForProvider( const QPair< QgsVectorDataProvider *, QgsFeatureRequest > &pair )
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
  QList< QPair< QgsVectorDataProvider *, QgsFeatureRequest > > jobs;
  jobs.reserve( JOBS_TO_RUN );
  for ( int i = 0; i < JOBS_TO_RUN; ++i )
  {
    jobs.append( qMakePair( provider, request ) );
  }

  //freaking hammer the provider with a ton of concurrent requests.
  //thread unsafe providers... you better be ready!!!!
  QtConcurrent::blockingMap( jobs, getFeaturesForProvider );

  return true;
}


///@endcond
