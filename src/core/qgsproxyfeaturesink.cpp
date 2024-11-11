/***************************************************************************
                         qgsproxyfeaturesink.cpp
                         ----------------------
    begin                : April 2020
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

#include "qgsproxyfeaturesink.h"


QgsProxyFeatureSink::QgsProxyFeatureSink( QgsFeatureSink *sink )
  : mSink( sink )
{}

bool QgsProxyFeatureSink::addFeature( QgsFeature &feature, Flags flags )
{
  if ( !mSink )
    return false;

  return mSink->addFeature( feature, flags );
}

bool QgsProxyFeatureSink::addFeatures( QgsFeatureList &features, Flags flags )
{
  if ( !mSink )
    return false;

  return mSink->addFeatures( features, flags );
}

bool QgsProxyFeatureSink::addFeatures( QgsFeatureIterator &iterator, Flags flags )
{
  if ( !mSink )
    return false;

  return mSink->addFeatures( iterator, flags );
}

QString QgsProxyFeatureSink::lastError() const
{
  return mSink ? mSink->lastError() : QString();
}

bool QgsProxyFeatureSink::flushBuffer()
{
  if ( !mSink )
    return false;

  return mSink->flushBuffer();
}

void QgsProxyFeatureSink::finalize()
{
  if ( mSink )
  {
    mSink->finalize();
  }
}
