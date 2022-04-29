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

QStringList QgsProfilePlotRenderer::sourceIds() const
{
  QStringList res;
  res.reserve( mGenerators.size() );
  for ( const auto &it : mGenerators )
  {
    res.append( it->sourceId() );
  }
  return res;
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
    std::unique_ptr< ProfileJob > job = std::make_unique< ProfileJob >();
    job->generator = it.get();
    job->context = mContext;
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

  for ( const auto &job : mJobs )
  {
    if ( job->generator )
    {
      if ( QgsFeedback *feedback = job->generator->feedback() )
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

  for ( const auto &job : mJobs )
  {
    if ( job->generator )
    {
      if ( QgsFeedback *feedback = job->generator->feedback() )
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

void QgsProfilePlotRenderer::setContext( const QgsProfileGenerationContext &context )
{
  if ( mContext == context )
    return;

  const double maxErrorChanged = !qgsDoubleNear( context.maximumErrorMapUnits(), mContext.maximumErrorMapUnits() );
  const double distanceRangeChanged = context.distanceRange() != mContext.distanceRange();
  const double elevationRangeChanged = context.elevationRange() != mContext.elevationRange();
  mContext = context;

  for ( auto &job : mJobs )
  {
    // regenerate only those results which are refinable
    const bool jobNeedsRegeneration = ( maxErrorChanged && ( job->generator->flags() & Qgis::ProfileGeneratorFlag::RespectsMaximumErrorMapUnit ) )
                                      || ( distanceRangeChanged && ( job->generator->flags() & Qgis::ProfileGeneratorFlag::RespectsDistanceRange ) )
                                      || ( elevationRangeChanged && ( job->generator->flags() & Qgis::ProfileGeneratorFlag::RespectsElevationRange ) );
    if ( !jobNeedsRegeneration )
      continue;

    job->mutex.lock();
    job->context = mContext;
    if ( job->results && job->complete )
      job->invalidatedResults = std::move( job->results );
    job->results.reset();
    job->complete = false;
    job->mutex.unlock();
  }
}

void QgsProfilePlotRenderer::invalidateAllRefinableSources()
{
  for ( auto &job : mJobs )
  {
    // regenerate only those results which are refinable
    const bool jobNeedsRegeneration = ( job->generator->flags() & Qgis::ProfileGeneratorFlag::RespectsMaximumErrorMapUnit )
                                      || ( job->generator->flags() & Qgis::ProfileGeneratorFlag::RespectsDistanceRange )
                                      || ( job->generator->flags() & Qgis::ProfileGeneratorFlag::RespectsElevationRange );
    if ( !jobNeedsRegeneration )
      continue;

    job->mutex.lock();
    job->context = mContext;
    if ( job->results && job->complete )
      job->invalidatedResults = std::move( job->results );
    job->results.reset();
    job->complete = false;
    job->mutex.unlock();
  }
}

void QgsProfilePlotRenderer::replaceSource( QgsAbstractProfileSource *source )
{
  replaceSourceInternal( source, false );
}

bool QgsProfilePlotRenderer::invalidateResults( QgsAbstractProfileSource *source )
{
  return replaceSourceInternal( source, true );
}

bool QgsProfilePlotRenderer::replaceSourceInternal( QgsAbstractProfileSource *source, bool clearPreviousResults )
{
  if ( !source )
    return false;

  std::unique_ptr< QgsAbstractProfileGenerator > generator{ source->createProfileGenerator( mRequest ) };
  if ( !generator )
    return false;

  QString sourceId = generator->sourceId();
  bool res = false;
  for ( auto &job : mJobs )
  {
    if ( job->generator && job->generator->sourceId() == sourceId )
    {
      job->mutex.lock();
      res = true;
      if ( clearPreviousResults )
      {
        job->results.reset();
        job->complete = false;
      }
      else if ( job->results )
      {
        job->results->copyPropertiesFromGenerator( generator.get() );
      }
      job->generator = generator.get();
      job->mutex.unlock();

      for ( auto it = mGenerators.begin(); it != mGenerators.end(); )
      {
        if ( ( *it )->sourceId() == sourceId )
          it = mGenerators.erase( it );
        else
          it++;
      }
      mGenerators.emplace_back( std::move( generator ) );
    }
  }
  return res;
}

void QgsProfilePlotRenderer::regenerateInvalidatedResults()
{
  if ( isActive() )
    return;

  mStatus = Generating;

  connect( &mFutureWatcher, &QFutureWatcher<void>::finished, this, &QgsProfilePlotRenderer::onGeneratingFinished );

  mFuture = QtConcurrent::map( mJobs, generateProfileStatic );
  mFutureWatcher.setFuture( mFuture );
}

QgsDoubleRange QgsProfilePlotRenderer::zRange() const
{
  double min = std::numeric_limits< double >::max();
  double max = std::numeric_limits< double >::lowest();
  for ( const auto &job : mJobs )
  {
    if ( job->complete && job->results )
    {
      const QgsDoubleRange jobRange = job->results->zRange();
      min = std::min( min, jobRange.lower() );
      max = std::max( max, jobRange.upper() );
    }
  }
  return QgsDoubleRange( min, max );
}

QImage QgsProfilePlotRenderer::renderToImage( int width, int height, double distanceMin, double distanceMax, double zMin, double zMax, const QString &sourceId )
{
  QImage res( width, height, QImage::Format_ARGB32_Premultiplied );
  res.fill( Qt::transparent );

  QPainter p( &res );

  QgsRenderContext context = QgsRenderContext::fromQPainter( &p );
  context.setFlag( Qgis::RenderContextFlag::Antialiasing, true );
  context.setPainterFlagsUsingContext( &p );
  render( context, width, height, distanceMin, distanceMax, zMin, zMax, sourceId );
  p.end();

  return res;
}

void QgsProfilePlotRenderer::render( QgsRenderContext &context, double width, double height, double distanceMin, double distanceMax, double zMin, double zMax, const QString &sourceId )
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

  for ( auto &job : mJobs )
  {
    if ( ( sourceId.isEmpty() || job->generator->sourceId() == sourceId ) )
    {
      job->mutex.lock();
      if ( job->complete && job->results )
      {
        job->results->renderResults( profileRenderContext );
      }
      else if ( !job->complete && job->invalidatedResults )
      {
        // draw the outdated results while we wait for refinement to complete
        job->invalidatedResults->renderResults( profileRenderContext );
      }
      job->mutex.unlock();
    }
  }
}

QgsProfileSnapResult QgsProfilePlotRenderer::snapPoint( const QgsProfilePoint &point, const QgsProfileSnapContext &context )
{
  QgsProfileSnapResult bestSnapResult;
  if ( !mRequest.profileCurve() )
    return bestSnapResult;

  double bestSnapDistance = std::numeric_limits< double >::max();

  for ( const auto &job : mJobs )
  {
    job->mutex.lock();
    if ( job->complete && job->results )
    {
      const QgsProfileSnapResult jobSnapResult = job->results->snapPoint( point, context );
      if ( jobSnapResult.isValid() )
      {
        const double snapDistance = std::pow( point.distance() - jobSnapResult.snappedPoint.distance(), 2 )
                                    + std::pow( ( point.elevation() - jobSnapResult.snappedPoint.elevation() ) / context.displayRatioElevationVsDistance, 2 );

        if ( snapDistance < bestSnapDistance )
        {
          bestSnapDistance = snapDistance;
          bestSnapResult = jobSnapResult;
        }
      }
    }
    job->mutex.unlock();
  }

  return bestSnapResult;
}

void QgsProfilePlotRenderer::onGeneratingFinished()
{
  mStatus = Idle;
  emit generationFinished();
}

void QgsProfilePlotRenderer::generateProfileStatic( std::unique_ptr< ProfileJob > &job )
{
  if ( job->results )
    return;

  Q_ASSERT( job->generator );

  job->generator->generateProfile( job->context );
  job->mutex.lock();
  job->results.reset( job->generator->takeResults() );
  job->complete = true;
  job->invalidatedResults.reset();
  job->mutex.unlock();
}

