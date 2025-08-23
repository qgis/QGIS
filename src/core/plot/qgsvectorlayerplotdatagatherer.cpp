/***************************************************************************
                         qgsvectorlayerplotdatagatherer.cpp
                         -----------------
    begin                : August 2025
    copyright            : (C) 2025 by Mathieu
    email                : mathieu at opengis dot ch
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "moc_qgsvectorlayerplotdatagatherer.cpp"
#include "qgsvectorlayerplotdatagatherer.h"
#include "qgsexpression.h"
#include "qgsexpressioncontextutils.h"


QgsVectorLayerXyPlotDataGatherer::QgsVectorLayerXyPlotDataGatherer( QgsVectorLayer *layer, const QList<XySeriesDetails> &seriesDetails, Qgis::PlotAxisType xAxisType, const QStringList &predefinedCategories )
  : mSource( new QgsVectorLayerFeatureSource( layer ) )
  , mExpressionContext( QgsExpressionContextUtils::globalProjectLayerScopes( layer ) )
  , mXAxisType( xAxisType )
  , mSeriesDetails( seriesDetails )
  , mPredefinedCategories( predefinedCategories )
{
}

void QgsVectorLayerXyPlotDataGatherer::run()
{
  mWasCanceled = false;

  QStringList gatheredCategories;
  for ( const XySeriesDetails &seriesDetails : mSeriesDetails )
  {
    std::unique_ptr<QgsXyPlotSeries> series = std::make_unique<QgsXyPlotSeries>();
    QMap<QString, double> gatheredCategoriesSum;

    QgsExpression xExpression( seriesDetails.xExpression );
    xExpression.prepare( &mExpressionContext );
    QgsExpression yExpression( seriesDetails.yExpression );
    yExpression.prepare( &mExpressionContext );

    QSet<QString> referencedColumns = xExpression.referencedColumns();
    referencedColumns.unite( yExpression.referencedColumns() );

    QgsFeatureRequest request;
    if ( !seriesDetails.filterExpression.isEmpty() )
    {
      request.setFilterExpression( seriesDetails.filterExpression );
    }
    if ( !seriesDetails.orderByExpression.isEmpty() )
    {
      request.addOrderBy( seriesDetails.orderByExpression );
    }
    request.setSubsetOfAttributes( referencedColumns, mSource->fields() );
    if ( !xExpression.needsGeometry() && !yExpression.needsGeometry() )
    {
      request.setFlags( Qgis::FeatureRequestFlag::NoGeometry );
    }

    QgsFeatureIterator iterator = mSource->getFeatures( request );
    QgsFeature feature;
    while ( iterator.nextFeature( feature ) )
    {
      mExpressionContext.setFeature( feature );

      bool ok = false;
      const double y = yExpression.evaluate( &mExpressionContext ).toDouble( &ok );
      if ( !ok )
      {
        continue;
      }
      switch ( mXAxisType )
      {
        case Qgis::PlotAxisType::Interval:
        {
          const double x = xExpression.evaluate( &mExpressionContext ).toDouble( &ok );
          if ( !ok )
          {
            continue;
          }

          series->append( x, y );
          break;
        }

        case Qgis::PlotAxisType::Categorical:
        {
          const QString category = xExpression.evaluate( &mExpressionContext ).toString();
          if ( category.isEmpty() )
          {
            continue;
          }

          double x = -1;
          if ( !mPredefinedCategories.isEmpty() )
          {
            x = mPredefinedCategories.indexOf( category );
          }
          else
          {
            if ( !gatheredCategories.contains( category ) )
            {
              gatheredCategories << category;
            }
            x = gatheredCategories.indexOf( category );
          }

          if ( x == -1 )
          {
            continue;
          }

          if ( !gatheredCategoriesSum.contains( category ) )
          {
            gatheredCategoriesSum[category] = y;
          }
          else
          {
            gatheredCategoriesSum[category] += y;
          }
        }
      }

      const QMutexLocker locker( &mCancelMutex );
      if ( mWasCanceled )
        return;
    }

    if ( mXAxisType == Qgis::PlotAxisType::Categorical )
    {
      if ( !mPredefinedCategories.isEmpty() )
      {
        for ( int i = 0; i < mPredefinedCategories.size(); i++ )
        {
          if ( gatheredCategoriesSum.contains( mPredefinedCategories[i] ) )
          {
            series->append( i, gatheredCategoriesSum[mPredefinedCategories[i]] );
          }
        }
      }
      else if ( !gatheredCategories.isEmpty() )
      {
        for ( int i = 0; i < gatheredCategories.size(); i++ )
        {
          if ( gatheredCategoriesSum.contains( gatheredCategories[i] ) )
          {
            series->append( i, gatheredCategoriesSum[gatheredCategories[i]] );
          }
        }
      }
    }

    mData.addSeries( series.release() );
  }

  if ( mXAxisType == Qgis::PlotAxisType::Categorical )
  {
    mData.setCategories( !mPredefinedCategories.isEmpty() ? mPredefinedCategories : gatheredCategories );
  }
}

void QgsVectorLayerXyPlotDataGatherer::stop()
{
  const QMutexLocker locker( &mCancelMutex );
  mWasCanceled = true;
}

bool QgsVectorLayerXyPlotDataGatherer::wasCanceled() const
{
  const QMutexLocker locker( &mCancelMutex );
  return mWasCanceled;
}

QgsPlotData QgsVectorLayerXyPlotDataGatherer::data() const
{
  return mData;
}
