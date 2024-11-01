/***************************************************************************
                          qgshistogram.cpp
                          ----------------
    begin                : May 2015
    copyright            : (C) 2015 by Nyall Dawson
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

#include "qgshistogram.h"

#include "qgsstatisticalsummary.h"
#include "qgsvectorlayer.h"
#include "qgsvectorlayerutils.h"

void QgsHistogram::prepareValues()
{
  std::sort( mValues.begin(), mValues.end() );

  QgsStatisticalSummary s;
  s.setStatistics( Qgis::Statistic::Max | Qgis::Statistic::Min | Qgis::Statistic::InterQuartileRange );
  s.calculate( mValues );
  mMin = s.min();
  mMax = s.max();
  mIQR = s.interQuartileRange();
}

void QgsHistogram::setValues( const QList<double> &values )
{
  mValues = values;
  prepareValues();
}

bool QgsHistogram::setValues( const QgsVectorLayer *layer, const QString &fieldOrExpression, QgsFeedback *feedback )
{
  mValues.clear();
  if ( !layer )
    return false;

  bool ok;
  mValues = QgsVectorLayerUtils::getDoubleValues( layer, fieldOrExpression, ok, false, nullptr, feedback );
  if ( !ok )
    return false;

  prepareValues();
  return true;
}

double QgsHistogram::optimalBinWidth() const
{
  //Freedman-Diaconis rule
  return 2.0 * mIQR * std::pow( mValues.count(), -1 / 3.0 );
}

int QgsHistogram::optimalNumberBins() const
{
  return std::ceil( ( mMax - mMin ) / optimalBinWidth() );
}

QList<double> QgsHistogram::binEdges( int bins ) const
{
  const double binWidth = ( mMax - mMin ) / bins;

  QList<double> edges;
  edges.reserve( bins + 1 );
  edges << mMin;
  double current = mMin;
  for ( int i = 0; i < bins; ++i )
  {
    current += binWidth;
    edges << current;
  }
  return edges;
}

QList<int> QgsHistogram::counts( int bins ) const
{
  const QList<double> edges = binEdges( bins );

  QList<int> binCounts;
  binCounts.reserve( bins );
  int currentValueIndex = 0;
  for ( int i = 0; i < bins; ++i )
  {
    int count = 0;
    while ( currentValueIndex < mValues.count() && mValues.at( currentValueIndex ) < edges.at( i + 1 ) )
    {
      count++;
      currentValueIndex++;
      if ( currentValueIndex >= mValues.count() )
        break;
    }
    binCounts << count;
  }

  if ( currentValueIndex < mValues.count() )
  {
    //last value needs to be added
    binCounts[bins - 1] = binCounts.last() + 1;
  }

  return binCounts;
}
