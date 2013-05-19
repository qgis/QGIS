/***************************************************************************
    qgslogicalfilter.cpp
    ---------------------
    begin                : August 2010
    copyright            : (C) 2010 by Marco Hugentobler
    email                : marco dot hugentobler at sourcepole dot ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "qgslogicalfilter.h"

QgsLogicalFilter::QgsLogicalFilter(): QgsFilter(), mFilter1( 0 ), mFilter2( 0 ), mFilterType( UNKNOWN )
{
}

QgsLogicalFilter::QgsLogicalFilter( FILTER_TYPE t, QgsFilter* filter1, QgsFilter* filter2 ): QgsFilter(), mFilter1( filter1 ), mFilter2( filter2 ), mFilterType( t )
{
}

QgsLogicalFilter::~QgsLogicalFilter()
{
  delete mFilter1;
  delete mFilter2;
}

void QgsLogicalFilter::addFilter1( QgsFilter* filter )
{
  delete mFilter1;
  mFilter1 = filter;
}

void QgsLogicalFilter::addFilter2( QgsFilter* filter )
{
  delete mFilter2;
  mFilter2 = filter;
}

bool QgsLogicalFilter::evaluate( const QgsFeature& f ) const
{
  if ( mFilterType == NOT )
  {
    if ( mFilter1 )
    {
      return !( mFilter1->evaluate( f ) );
    }
    return false;
  }

  //we need both filters for AND/OR
  if ( mFilter1 && mFilter2 )
  {
    if ( mFilterType == AND )
    {
      return ( mFilter1->evaluate( f ) && mFilter2->evaluate( f ) );
    }
    else if ( mFilterType == OR )
    {
      return ( mFilter1->evaluate( f ) || mFilter2->evaluate( f ) );
    }
  }
  return false;
}

QList<int> QgsLogicalFilter::attributeIndices() const
{
  QList<int> resultList;
  if ( mFilter1 )
  {
    resultList << mFilter1->attributeIndices();
  }
  if ( mFilter2 )
  {
    resultList << mFilter2->attributeIndices();
  }
  return resultList;
}
