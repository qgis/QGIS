/***************************************************************************
                         qgsvectordataprovidertemporalcapabilities.cpp
                         ---------------
    begin                : March 2020
    copyright            : (C) 2020 by Nyall Dawson
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

#include "qgsvectordataprovidertemporalcapabilities.h"

QgsVectorDataProviderTemporalCapabilities::QgsVectorDataProviderTemporalCapabilities( bool enabled )
  :  QgsDataProviderTemporalCapabilities( enabled )
{
}

void QgsVectorDataProviderTemporalCapabilities::setAvailableTemporalRange( const QgsDateTimeRange &dateTimeRange )
{
  if ( !hasTemporalCapabilities() )
    setHasTemporalCapabilities( true );

  mAvailableTemporalRange = dateTimeRange;
}

const QgsDateTimeRange &QgsVectorDataProviderTemporalCapabilities::availableTemporalRange() const
{
  return mAvailableTemporalRange;
}
