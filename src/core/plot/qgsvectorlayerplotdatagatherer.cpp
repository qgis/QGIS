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


QgsVectorLayerXyPlotDataGatherer::QgsVectorLayerXyPlotDataGatherer( Qgis::PlotAxisType xAxisType )
  : mXAxisType( xAxisType )
{
}

void QgsVectorLayerXyPlotDataGatherer::setFeatureIterator( QgsFeatureIterator &iterator )
{
  mIterator = iterator;
}

void QgsVectorLayerXyPlotDataGatherer::setExpressionContext( const QgsExpressionContext &expressionContext )
{
  mExpressionContext = expressionContext;
}

void QgsVectorLayerXyPlotDataGatherer::setSeriesDetails( const QList<QgsVectorLayerXyPlotDataGatherer::XySeriesDetails> &seriesDetails )
{
  mSeriesDetails = seriesDetails;
}

void QgsVectorLayerXyPlotDataGatherer::setPredefinedCategories( const QStringList &predefinedCategories )
{
  mPredefinedCategories = predefinedCategories;
}

bool QgsVectorLayerXyPlotDataGatherer::run()
{
  QStringList gatheredCategories;
  QList<QMap<QString, double>> gatheredSeriesCategoriesSum;
  gatheredSeriesCategoriesSum.reserve( mSeriesDetails.size() );
  std::vector<std::unique_ptr<QgsXyPlotSeries>> gatheredSeries;
  gatheredSeries.reserve( mSeriesDetails.size() );
  for ( int i = 0; i < mSeriesDetails.size(); i++ )
  {
    std::unique_ptr<QgsXyPlotSeries> series = std::make_unique<QgsXyPlotSeries>();
    gatheredSeries.emplace_back( std::move( series ) );
    gatheredSeriesCategoriesSum << QMap<QString, double>();
  }

  QMap<QString, QgsExpression> preparedExpressions;
  QgsFeature feature;
  while ( mIterator.nextFeature( feature ) )
  {
    mExpressionContext.setFeature( feature );

    int seriesIndex = 0;
    for ( const XySeriesDetails &seriesDetails : mSeriesDetails )
    {
      if ( !seriesDetails.filterExpression.isEmpty() )
      {
        auto filterExpressionIt = preparedExpressions.find( seriesDetails.filterExpression );
        if ( filterExpressionIt == preparedExpressions.end() )
        {
          filterExpressionIt = preparedExpressions.insert( seriesDetails.filterExpression, QgsExpression( seriesDetails.filterExpression ) );
          filterExpressionIt->prepare( &mExpressionContext );
        }

        if ( !filterExpressionIt->evaluate( &mExpressionContext ).toBool() )
        {
          continue;
        }
      }

      auto xExpressionIt = preparedExpressions.find( seriesDetails.xExpression );
      if ( xExpressionIt == preparedExpressions.end() )
      {
        xExpressionIt = preparedExpressions.insert( seriesDetails.xExpression, QgsExpression( seriesDetails.xExpression ) );
        xExpressionIt->prepare( &mExpressionContext );
      }

      auto yExpressionIt = preparedExpressions.find( seriesDetails.yExpression );
      if ( yExpressionIt == preparedExpressions.end() )
      {
        yExpressionIt = preparedExpressions.insert( seriesDetails.yExpression, QgsExpression( seriesDetails.yExpression ) );
        yExpressionIt->prepare( &mExpressionContext );
      }

      bool ok = false;
      const double y = yExpressionIt->evaluate( &mExpressionContext ).toDouble( &ok );
      if ( !ok )
      {
        continue;
      }
      switch ( mXAxisType )
      {
        case Qgis::PlotAxisType::Interval:
        {
          const double x = xExpressionIt->evaluate( &mExpressionContext ).toDouble( &ok );
          if ( !ok )
          {
            continue;
          }

          gatheredSeries[seriesIndex]->append( x, y );
          break;
        }

        case Qgis::PlotAxisType::Categorical:
        {
          const QString category = xExpressionIt->evaluate( &mExpressionContext ).toString();
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

          auto gatheredSeriesCategoriesSumIt = gatheredSeriesCategoriesSum[seriesIndex].find( category );
          if ( gatheredSeriesCategoriesSumIt == gatheredSeriesCategoriesSum[seriesIndex].end() )
          {
            gatheredSeriesCategoriesSumIt = gatheredSeriesCategoriesSum[seriesIndex].insert( category, y );
            *gatheredSeriesCategoriesSumIt = y;
          }
          else
          {
            *gatheredSeriesCategoriesSumIt += y;
          }
        }
      }

      seriesIndex++;
      if ( isCanceled() )
        return false;
    }
  }

  switch ( mXAxisType )
  {
    case Qgis::PlotAxisType::Categorical:
    {
      int seriesIndex = 0;
      for ( QMap<QString, double> &gatheredCategoriesSum : gatheredSeriesCategoriesSum )
      {
        if ( !mPredefinedCategories.isEmpty() )
        {
          for ( int i = 0; i < mPredefinedCategories.size(); i++ )
          {
            auto gatheredCategoriesSumIt = gatheredCategoriesSum.find( mPredefinedCategories[i] );
            if ( gatheredCategoriesSumIt != gatheredCategoriesSum.end() )
            {
              gatheredSeries[seriesIndex]->append( i, *gatheredCategoriesSumIt );
            }
          }
        }
        else if ( !gatheredCategories.isEmpty() )
        {
          for ( int i = 0; i < gatheredCategories.size(); i++ )
          {
            auto gatheredCategoriesSumIt = gatheredCategoriesSum.find( gatheredCategories[i] );
            if ( gatheredCategoriesSumIt != gatheredCategoriesSum.end() )
            {
              gatheredSeries[seriesIndex]->append( i, *gatheredCategoriesSumIt );
            }
          }
        }
        seriesIndex++;
      }

      mData.setCategories( !mPredefinedCategories.isEmpty() ? mPredefinedCategories : gatheredCategories );
      break;
    }

    case Qgis::PlotAxisType::Interval:
      break;
  }

  for ( std::unique_ptr<QgsXyPlotSeries> &series : gatheredSeries )
  {
    mData.addSeries( series.release() );
  }

  return true;
}

QgsPlotData QgsVectorLayerXyPlotDataGatherer::data() const
{
  return mData;
}
