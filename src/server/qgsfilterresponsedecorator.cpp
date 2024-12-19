/***************************************************************************
                          qgsfilterresponsedecorator.cpp

  Define response wrapper for fcgi response
  -------------------
  begin                : 2017-01-03
  copyright            : (C) 2017 by David Marteau
  email                : david dot marteau at 3liz dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsconfig.h"
#include "qgsfilterresponsedecorator.h"

QgsFilterResponseDecorator::QgsFilterResponseDecorator( QgsServerFiltersMap filters, QgsServerResponse &response )
  : mFilters( filters )
  , mResponse( response )
{
}

void QgsFilterResponseDecorator::start()
{
#ifdef HAVE_SERVER_PYTHON_PLUGINS
  QgsServerFiltersMap::const_iterator filtersIterator;
  for ( filtersIterator = mFilters.constBegin(); filtersIterator != mFilters.constEnd(); ++filtersIterator )
  {
    if ( ! filtersIterator.value()->onRequestReady() )
    {
      // stop propagation
      return;
    }
  }
#endif
}

void QgsFilterResponseDecorator::ready()
{
#ifdef HAVE_SERVER_PYTHON_PLUGINS
  QgsServerFiltersMap::const_iterator filtersIterator;
  for ( filtersIterator = mFilters.constBegin(); filtersIterator != mFilters.constEnd(); ++filtersIterator )
  {
    if ( ! filtersIterator.value()->onProjectReady() )
    {
      // stop propagation
      return;
    }
  }
#endif
}

void QgsFilterResponseDecorator::finish()
{
#ifdef HAVE_SERVER_PYTHON_PLUGINS
  QgsServerFiltersMap::const_iterator filtersIterator;
  for ( filtersIterator = mFilters.constBegin(); filtersIterator != mFilters.constEnd(); ++filtersIterator )
  {
    if ( ! filtersIterator.value()->onResponseComplete() )
    {
      // stop propagation, 'finish' must be called on the wrapped
      // response
      break;
    }
  }
#endif
  // Will call internal 'flush'
  mResponse.finish();
}


void QgsFilterResponseDecorator::flush()
{
#ifdef HAVE_SERVER_PYTHON_PLUGINS
  QgsServerFiltersMap::const_iterator filtersIterator;

  for ( filtersIterator = mFilters.constBegin(); filtersIterator != mFilters.constEnd(); ++filtersIterator )
  {
    if ( ! filtersIterator.value()->onSendResponse() )
    {
      // Stop propagation
      return;
    }
  }
#endif
  mResponse.flush();
}
