/***************************************************************************
                         qgsprofilerenderer.h
                         ---------------
    begin                : March 2022
    copyright            : (C) 2022 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "qgsprofilerenderer.h"
#include "qgsabstractprofilesource.h"
#include "qgsabstractprofilegenerator.h"
#include "qgscurve.h"
#include "qgsgeos.h"
#include "qgsprofilesnapping.h"

#include <QtConcurrentMap>
#include <QtConcurrentRun>

QgsProfilePlotRenderer::QgsProfilePlotRenderer( const QList< QgsAbstractProfileSource * > &sources,
    const QgsProfileRequest &request )
  : mRequest( request )
{
  for ( QgsAbstractProfileSource *source : sources )
  {
    if ( source )
    {
      if ( std::unique_ptr< QgsAbstractProfileGenerator > generator{ source->createProfileGenerator( mRequest ) } )
        mGenerators.emplace_back( std::move( generator ) );
    }
  }
}

QgsProfilePlotRenderer::~QgsProfilePlotRenderer()
{
  if ( isActive() )
  {
    cancelGeneration();
  }
}

void QgsProfilePlotRenderer::startGeneration()
{
  if ( isActive() )
    return;

  mStatus = Generating;

  Q_ASSERT( mJobs.empty() );

  mJobs.reserve( mGenerators.size() );
  for ( const auto &it : mGenerators )
  {
    ProfileJob job;
    job.generator = it.get();
    mJobs.emplace_back( std::move( job ) );
  }

  connect( &mFutureWatcher, &QFutureWatcher<void>::finished, this, &QgsProfilePlotRenderer::onGeneratingFinished );

  mFuture = QtConcurrent::map( mJobs, generateProfileStatic );
  mFutureWatcher.setFuture( mFuture );
}

void QgsProfilePlotRenderer::cancelGeneration()
{
  if ( !isActive() )
    return;

  disconnect( &mFutureWatcher, &QFutureWatcher<void>::finished, this, &QgsProfilePlotRenderer::onGeneratingFinished );

  for ( const ProfileJob &job : mJobs )
  {
    if ( job.generator )
    {
      if ( QgsFeedback *feedback = job.generator->feedback() )
      {
        feedback->cancel();
      }
    }
  }

  mFutureWatcher.waitForFinished();

  onGeneratingFinished();
}

void QgsProfilePlotRenderer::cancelGenerationWithoutBlocking()
{
  if ( !isActive() )
    return;

  for ( const ProfileJob &job : mJobs )
  {
    if ( job.generator )
    {
      if ( QgsFeedback *feedback = job.generator->feedback() )
      {
        feedback->cancel();
      }
    }
  }
}

void QgsProfilePlotRenderer::waitForFinished()
{
  if ( !isActive() )
    return;

  disconnect( &mFutureWatcher, &QFutureWatcher<void>::finished, this, &QgsProfilePlotRenderer::onGeneratingFinished );
  mFutureWatcher.waitForFinished();

  onGeneratingFinished();
}

bool QgsProfilePlotRenderer::isActive() const
{
  return mStatus != Idle;
}

QgsDoubleRange QgsProfilePlotRenderer::zRange() const
{
  double min = std::numeric_limits< double >::max();
  double max = std::numeric_limits< double >::lowest();
  for ( const ProfileJob &job : mJobs )
  {
    if ( job.complete && job.results )
    {
      const QgsDoubleRange jobRange = job.results->zRange();
      min = std::min( min, jobRange.lower() );
      max = std::max( max, jobRange.upper() );
    }
  }
  return QgsDoubleRange( min, max );
}

QImage QgsProfilePlotRenderer::renderToImage( int width, int height, double distanceMin, double distanceMax, double zMin, double zMax )
{
  QImage res( width, height, QImage::Format_ARGB32_Premultiplied );
  res.fill( Qt::transparent );

  QPainter p( &res );

  QgsRenderContext context = QgsRenderContext::fromQPainter( &p );
  context.setFlag( Qgis::RenderContextFlag::Antialiasing, true );
  context.setPainterFlagsUsingContext( &p );
  render( context, width, height, distanceMin, distanceMax, zMin, zMax );
  p.end();

  return res;
}

void QgsProfilePlotRenderer::render( QgsRenderContext &context, double width, double height, double distanceMin, double distanceMax, double zMin, double zMax )
{
  QPainter *painter = context.painter();
  if ( !painter )
    return;

  QgsProfileRenderContext profileRenderContext( context );

  QTransform transform;
  transform.translate( 0, height );
  transform.scale( width / ( distanceMax - distanceMin ), -height / ( zMax - zMin ) );
  transform.translate( -distanceMin, -zMin );
  profileRenderContext.setWorldTransform( transform );

  profileRenderContext.setDistanceRange( QgsDoubleRange( distanceMin, distanceMax ) );
  profileRenderContext.setElevationRange( QgsDoubleRange( zMin, zMax ) );

  for ( const ProfileJob &job : mJobs )
  {
    if ( job.complete && job.results )
      job.results->renderResults( profileRenderContext );
  }
}

QgsProfileSnapResult QgsProfilePlotRenderer::snapPoint( const QgsProfilePoint &point, double maximumCurveDelta, double maximumHeightDelta )
{
  QgsProfileSnapResult bestSnapResult;
  if ( !mRequest.profileCurve() )
    return bestSnapResult;

  double bestSnapDistance = std::numeric_limits< double >::max();

  for ( const ProfileJob &job : mJobs )
  {
    if ( job.complete && job.results )
    {
      const QgsProfileSnapResult jobSnapResult = job.results->snapPoint( point, maximumCurveDelta, maximumHeightDelta );
      if ( jobSnapResult.isValid() )
      {
        const double snapDistance = std::pow( point.distance() - jobSnapResult.snappedPoint.distance(), 2 )
                                    + std::pow( point.elevation() - jobSnapResult.snappedPoint.elevation(), 2 );

        if ( snapDistance < bestSnapDistance )
        {
          bestSnapDistance = snapDistance;
          bestSnapResult = jobSnapResult;
        }
      }
    }
  }

  return bestSnapResult;
}

void QgsProfilePlotRenderer::onGeneratingFinished()
{
  mStatus = Idle;
  emit generationFinished();
}

void QgsProfilePlotRenderer::generateProfileStatic( ProfileJob &job )
{
  Q_ASSERT( job.generator );
  job.generator->generateProfile();
  job.results.reset( job.generator->takeResults() );
  job.complete = true;
}
