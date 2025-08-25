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


QgsVectorLayerXyPlotDataGatherer::QgsVectorLayerXyPlotDataGatherer( const QgsFeatureIterator &iterator, const QgsExpressionContext &expressionContext, const QList<QgsVectorLayerXyPlotDataGatherer::XySeriesDetails> &seriesDetails, Qgis::PlotAxisType xAxisType, const QStringList &predefinedCategories )
  : mIterator( iterator )
  , mExpressionContext( expressionContext )
  , mXAxisType( xAxisType )
  , mSeriesDetails( seriesDetails )
  , mPredefinedCategories( predefinedCategories )
{
}

bool QgsVectorLayerXyPlotDataGatherer::run()
{
  QStringList gatheredCategories;
  QList<QMap<QString, double>> gatheredSeriesCategoriesSum;
  QList<QgsXyPlotSeries *> gatheredSeries;
  for ( int i = 0; i < mSeriesDetails.size(); i++ )
  {
    gatheredSeries << new QgsXyPlotSeries();
    gatheredSeriesCategoriesSum << QMap<QString, double>();
  }

  QMap<QString, QgsExpression> expressions;
  QgsFeature feature;
  while ( mIterator.nextFeature( feature ) )
  {
    int seriesIndex = 0;
    for ( const XySeriesDetails &seriesDetails : mSeriesDetails )
    {
      mExpressionContext.setFeature( feature );

      if ( !seriesDetails.filterExpression.isEmpty() )
      {
        if ( !expressions.contains( seriesDetails.filterExpression ) )
        {
          expressions[seriesDetails.filterExpression] = QgsExpression( seriesDetails.filterExpression );
          expressions[seriesDetails.filterExpression].prepare( &mExpressionContext );
        }
        if ( !expressions[seriesDetails.filterExpression].evaluate( &mExpressionContext ).toBool() )
        {
          continue;
        }
      }

      if ( !expressions.contains( seriesDetails.xExpression ) )
      {
        expressions[seriesDetails.xExpression] = QgsExpression( seriesDetails.xExpression );
        expressions[seriesDetails.xExpression].prepare( &mExpressionContext );
      }
      if ( !expressions.contains( seriesDetails.yExpression ) )
      {
        expressions[seriesDetails.yExpression] = QgsExpression( seriesDetails.yExpression );
        expressions[seriesDetails.yExpression].prepare( &mExpressionContext );
      }

      bool ok = false;
      const double y = expressions[seriesDetails.yExpression].evaluate( &mExpressionContext ).toDouble( &ok );
      if ( !ok )
      {
        continue;
      }
      switch ( mXAxisType )
      {
        case Qgis::PlotAxisType::Interval:
        {
          const double x = expressions[seriesDetails.xExpression].evaluate( &mExpressionContext ).toDouble( &ok );
          if ( !ok )
          {
            continue;
          }

          gatheredSeries[seriesIndex]->append( x, y );
          break;
        }

        case Qgis::PlotAxisType::Categorical:
        {
          const QString category = expressions[seriesDetails.xExpression].evaluate( &mExpressionContext ).toString();
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

          if ( !gatheredSeriesCategoriesSum[seriesIndex].contains( category ) )
          {
            gatheredSeriesCategoriesSum[seriesIndex][category] = y;
          }
          else
          {
            gatheredSeriesCategoriesSum[seriesIndex][category] += y;
          }
        }
      }

      seriesIndex++;
      if ( isCanceled() )
        return false;
    }
  }

  if ( mXAxisType == Qgis::PlotAxisType::Categorical )
  {
    int seriesIndex = 0;
    for ( QMap<QString, double> &gatheredCategoriesSum : gatheredSeriesCategoriesSum )
    {
      if ( !mPredefinedCategories.isEmpty() )
      {
        for ( int i = 0; i < mPredefinedCategories.size(); i++ )
        {
          if ( gatheredCategoriesSum.contains( mPredefinedCategories[i] ) )
          {
            gatheredSeries[seriesIndex]->append( i, gatheredCategoriesSum[mPredefinedCategories[i]] );
          }
        }
      }
      else if ( !gatheredCategories.isEmpty() )
      {
        for ( int i = 0; i < gatheredCategories.size(); i++ )
        {
          if ( gatheredCategoriesSum.contains( gatheredCategories[i] ) )
          {
            gatheredSeries[seriesIndex]->append( i, gatheredCategoriesSum[gatheredCategories[i]] );
          }
        }
      }
      seriesIndex++;
    }
  }

  for ( QgsXyPlotSeries *series : gatheredSeries )
  {
    mData.addSeries( series ); // Ownership transferred
  }

  if ( mXAxisType == Qgis::PlotAxisType::Categorical )
  {
    mData.setCategories( !mPredefinedCategories.isEmpty() ? mPredefinedCategories : gatheredCategories );
  }

  return true;
}

QgsPlotData QgsVectorLayerXyPlotDataGatherer::data() const
{
  return mData;
}
